//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Network client code
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "doomtype.h"
#include "deh_main.h"
#include "deh_str.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_fixed.h"
#include "m_config.h"
#include "m_misc.h"
#include "net_client.h"
#include "net_common.h"
#include "net_defs.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_server.h"
#include "net_structrw.h"
#include "w_checksum.h"
#include "w_wad.h"

extern void D_ReceiveTic(ticcmd_t *ticcmds, boolean *playeringame);

typedef enum
{
    // waiting for the game to launch

    CLIENT_STATE_WAITING_LAUNCH,

    // waiting for the game to start

    CLIENT_STATE_WAITING_START,

    // in game

    CLIENT_STATE_IN_GAME,

} net_clientstate_t;

// Type of structure used in the receive window

typedef struct
{
    // Whether this tic has been received yet

    boolean active;

    // Last time we sent a resend request for this tic

    unsigned int resend_time;

    // Tic data from server

    net_full_ticcmd_t cmd;

} net_server_recv_t;

// Type of structure used in the send window

typedef struct
{
    // Whether this slot is active yet

    boolean active;

    // The tic number

    unsigned int seq;

    // Time the command was generated

    unsigned int time;

    // Ticcmd diff

    net_ticdiff_t cmd;
} net_server_send_t;

extern fixed_t offsetms;

static net_connection_t client_connection;
static net_clientstate_t client_state;
static net_addr_t *server_addr;
static net_context_t *client_context;

// game settings, as received from the server when the game started

static net_gamesettings_t settings;

// true if the client code is in use

boolean net_client_connected;

// true if we have received waiting data from the server,
// and the wait data that was received.

boolean net_client_received_wait_data;
net_waitdata_t net_client_wait_data;

// Waiting at the initial wait screen for the game to be launched?

boolean net_waiting_for_launch = false;

// Name that we send to the server

char *net_player_name = NULL;

// Connected but not participating in the game (observer)

boolean drone = false;

// The last ticcmd constructed

static ticcmd_t last_ticcmd;

// Buffer of ticcmd diffs being sent to the server

static net_server_send_t send_queue[BACKUPTICS];

// Receive window

static ticcmd_t recvwindow_cmd_base[NET_MAXPLAYERS];
static int recvwindow_start;
static net_server_recv_t recvwindow[BACKUPTICS];

// Whether we need to send an acknowledgement and
// when gamedata was last received.

static boolean need_to_acknowledge;
static unsigned int gamedata_recv_time;

// Hash checksums of our wad directory and dehacked data.

sha1_digest_t net_local_wad_sha1sum;
sha1_digest_t net_local_deh_sha1sum;

// Are we playing with the freedoom IWAD?

unsigned int net_local_is_freedoom;

// Average time between sending our ticcmd and receiving from the server

static fixed_t average_latency;

#define NET_CL_ExpandTicNum(b) NET_ExpandTicNum(recvwindow_start, (b))

// Called when we become disconnected from the server

static void NET_CL_Disconnected(void)
{
    D_ReceiveTic(NULL, NULL);
}

// Expand a net_full_ticcmd_t, applying the diffs in cmd->cmds as
// patches against recvwindow_cmd_base.  Place the results into
// the d_net.c structures (netcmds/nettics) and save the new ticcmd
// back into recvwindow_cmd_base.

static void NET_CL_ExpandFullTiccmd(net_full_ticcmd_t *cmd, unsigned int seq,
                                    ticcmd_t *ticcmds)
{
    int latency;
    fixed_t adjustment;
    int i;

    // Update average_latency

    if (seq == send_queue[seq % BACKUPTICS].seq)
    {
        latency = I_GetTimeMS() - send_queue[seq % BACKUPTICS].time;
    }
    else if (seq > send_queue[seq % BACKUPTICS].seq)
    {
        // We have received the ticcmd from the server before we have
        // even sent ours

        latency = 0;
    }
    else
    {
        latency = -1;
    }

    if (latency >= 0)
    {
        if (seq <= 20)
        {
            average_latency = latency * FRACUNIT;
        }
        else
        {
            // Low level filter

            average_latency = (fixed_t)((average_latency * 0.9)
                            + (latency * FRACUNIT * 0.1));
        }
    }

    //printf("latency: %i\tremote:%i\n", average_latency / FRACUNIT, 
    //                                   cmd->latency);

    // Possibly adjust offsetms in d_net.c, try to make players all have
    // the same lag.  Don't adjust in the first few tics of play, as 
    // we don't have an accurate value for average_latency yet.

    if (seq > TICRATE)
    {
        adjustment = (cmd->latency * FRACUNIT) - average_latency;

        // Only adjust very slightly; the cumulative effect over 
        // multiple tics will sort it out.

        adjustment = adjustment / 100;

        offsetms += adjustment;
    }

    // Expand tic diffs for all players
    
    for (i=0; i<NET_MAXPLAYERS; ++i)
    {
        if (i == settings.consoleplayer && !drone)
        {
            continue;
        }
        
        if (cmd->playeringame[i])
        {
            net_ticdiff_t *diff;

            diff = &cmd->cmds[i];

            // Use the ticcmd diff to patch the previous ticcmd to
            // the new ticcmd

            NET_TiccmdPatch(&recvwindow_cmd_base[i], diff, &ticcmds[i]);

            // Store a copy for next time

            recvwindow_cmd_base[i] = ticcmds[i];
        }
    }
}

// Advance the receive window

static void NET_CL_AdvanceWindow(void)
{
    ticcmd_t ticcmds[NET_MAXPLAYERS];

    while (recvwindow[0].active)
    {
        // Expand tic diff data into d_net.c structures

        NET_CL_ExpandFullTiccmd(&recvwindow[0].cmd, recvwindow_start,
                                ticcmds);
        D_ReceiveTic(ticcmds, recvwindow[0].cmd.playeringame);

        // Advance the window

        memmove(recvwindow, recvwindow + 1,
                sizeof(net_server_recv_t) * (BACKUPTICS - 1));
        memset(&recvwindow[BACKUPTICS-1], 0, sizeof(net_server_recv_t));

        ++recvwindow_start;

        //printf("CL: advanced to %i\n", recvwindow_start);
    }
}

// Shut down the client code, etc.  Invoked after a disconnect.

static void NET_CL_Shutdown(void)
{
    if (net_client_connected)
    {
        net_client_connected = false;

        NET_FreeAddress(server_addr);

        // Shut down network module, etc.  To do.
    }
}

void NET_CL_LaunchGame(void)
{
    NET_Conn_NewReliable(&client_connection, NET_PACKET_TYPE_LAUNCH);
}

void NET_CL_StartGame(net_gamesettings_t *settings)
{
    net_packet_t *packet;

    // Start from a ticcmd of all zeros

    memset(&last_ticcmd, 0, sizeof(ticcmd_t));
    
    // Send packet

    packet = NET_Conn_NewReliable(&client_connection, 
                                  NET_PACKET_TYPE_GAMESTART);

    NET_WriteSettings(packet, settings);
}

static void NET_CL_SendGameDataACK(void)
{
    net_packet_t *packet;

    packet = NET_NewPacket(10);

    NET_WriteInt16(packet, NET_PACKET_TYPE_GAMEDATA_ACK);
    NET_WriteInt8(packet, recvwindow_start & 0xff);

    NET_Conn_SendPacket(&client_connection, packet);

    NET_FreePacket(packet);

    need_to_acknowledge = false;
}

static void NET_CL_SendTics(int start, int end)
{
    net_packet_t *packet;
    int i;

    if (!net_client_connected)
    {
        // Disconnected from server

        return;
    }

    if (start < 0)
        start = 0;
    
    // Build a new packet to send to the server

    packet = NET_NewPacket(512);
    NET_WriteInt16(packet, NET_PACKET_TYPE_GAMEDATA);

    // Write the start tic and number of tics.  Send only the low byte
    // of start - it can be inferred by the server.

    NET_WriteInt8(packet, recvwindow_start & 0xff);
    NET_WriteInt8(packet, start & 0xff);
    NET_WriteInt8(packet, end - start + 1);

    // Add the tics.

    for (i=start; i<=end; ++i)
    {
        net_server_send_t *sendobj;

        sendobj = &send_queue[i % BACKUPTICS];

        NET_WriteInt16(packet, average_latency / FRACUNIT);

        NET_WriteTiccmdDiff(packet, &sendobj->cmd, settings.lowres_turn);
    }
    
    // Send the packet

    NET_Conn_SendPacket(&client_connection, packet);
    
    // All done!

    NET_FreePacket(packet);

    // Acknowledgement has been sent as part of the packet

    need_to_acknowledge = false;
}

// Add a new ticcmd to the send queue

void NET_CL_SendTiccmd(ticcmd_t *ticcmd, int maketic)
{
    net_ticdiff_t diff;
    net_server_send_t *sendobj;
    int starttic, endtic;
    
    // Calculate the difference to the last ticcmd

    NET_TiccmdDiff(&last_ticcmd, ticcmd, &diff);
    
    // Store in the send queue

    sendobj = &send_queue[maketic % BACKUPTICS];
    sendobj->active = true;
    sendobj->seq = maketic;
    sendobj->time = I_GetTimeMS();
    sendobj->cmd = diff;

    last_ticcmd = *ticcmd;

    // Send to server.

    starttic = maketic - settings.extratics;
    endtic = maketic;

    if (starttic < 0)
        starttic = 0;
    
    NET_CL_SendTics(starttic, endtic);
}

// Parse a SYN packet received back from the server indicating a successful
// connection attempt.
static void NET_CL_ParseSYN(net_packet_t *packet)
{
    net_protocol_t protocol;
    char *server_version;

    server_version = NET_ReadSafeString(packet);
    if (server_version == NULL)
    {
        return;
    }

    protocol = NET_ReadProtocol(packet);
    if (protocol == NET_PROTOCOL_UNKNOWN)
    {
        return;
    }

    // We are now successfully connected.
    client_connection.state = NET_CONN_STATE_CONNECTED;
    client_connection.protocol = protocol;

    // Even though we have negotiated a compatible protocol, the game may still
    // desync. Chocolate Doom's philosophy makes this unlikely, but if we're
    // playing with a forked version, or even against a different version that
    // fixes a compatibility issue, we may still have problems.
    if (strcmp(server_version, PACKAGE_STRING) != 0)
    {
        fprintf(stderr, "NET_CL_ParseSYN: This is '%s', but the server is "
                "'%s'. It is possible that this mismatch may cause the game "
                "to desync.\n", PACKAGE_STRING, server_version);
    }
}

// data received while we are waiting for the game to start

static void NET_CL_ParseWaitingData(net_packet_t *packet)
{
    net_waitdata_t wait_data;

    if (!NET_ReadWaitData(packet, &wait_data))
    {
        // Invalid packet?
        return;
    }

    if (wait_data.num_players > wait_data.max_players
     || wait_data.ready_players > wait_data.num_players
     || wait_data.max_players > NET_MAXPLAYERS)
    {
        // insane data

        return;
    }

    if ((wait_data.consoleplayer >= 0 && drone)
     || (wait_data.consoleplayer < 0 && !drone)
     || (wait_data.consoleplayer >= wait_data.num_players))
    {
        // Invalid player number

        return;
    }

    memcpy(&net_client_wait_data, &wait_data, sizeof(net_waitdata_t));
    net_client_received_wait_data = true;
}

static void NET_CL_ParseLaunch(net_packet_t *packet)
{
    unsigned int num_players;

    if (client_state != CLIENT_STATE_WAITING_LAUNCH)
    {
        return;
    }

    // The launch packet contains the number of players that will be
    // in the game when it starts, so that we can do the startup
    // progress indicator (the wait data is unreliable).

    if (!NET_ReadInt8(packet, &num_players))
    {
        return;
    }

    net_client_wait_data.num_players = num_players;
    client_state = CLIENT_STATE_WAITING_START;
}

static void NET_CL_ParseGameStart(net_packet_t *packet)
{
    if (!NET_ReadSettings(packet, &settings))
    {
        return;
    }

    if (client_state != CLIENT_STATE_WAITING_START)
    {
        return;
    }

    if (settings.num_players > NET_MAXPLAYERS
     || settings.consoleplayer >= (signed int) settings.num_players)
    {
        // insane values
        return;
    }

    if ((drone && settings.consoleplayer >= 0)
     || (!drone && settings.consoleplayer < 0))
    {
        // Invalid player number: must be positive for real players,
        // negative for drones

        return;
    }

    client_state = CLIENT_STATE_IN_GAME;

    // Clear the receive window

    memset(recvwindow, 0, sizeof(recvwindow));
    recvwindow_start = 0;
    memset(&recvwindow_cmd_base, 0, sizeof(recvwindow_cmd_base));

    // Clear the send queue

    memset(&send_queue, 0x00, sizeof(send_queue));
}

static void NET_CL_SendResendRequest(int start, int end)
{
    net_packet_t *packet;
    unsigned int nowtime;
    int i;

    //printf("CL: Send resend %i-%i\n", start, end);
    
    packet = NET_NewPacket(64);
    NET_WriteInt16(packet, NET_PACKET_TYPE_GAMEDATA_RESEND);
    NET_WriteInt32(packet, start);
    NET_WriteInt8(packet, end - start + 1);
    NET_Conn_SendPacket(&client_connection, packet);
    NET_FreePacket(packet);

    nowtime = I_GetTimeMS();

    // Save the time we sent the resend request

    for (i=start; i<=end; ++i)
    {
        int index;

        index = i - recvwindow_start;

        if (index < 0 || index >= BACKUPTICS)
            continue;

        recvwindow[index].resend_time = nowtime;
    }
}

// Check for expired resend requests

static void NET_CL_CheckResends(void)
{
    int i;
    int resend_start, resend_end;
    unsigned int nowtime;

    nowtime = I_GetTimeMS();

    resend_start = -1;
    resend_end = -1;

    for (i=0; i<BACKUPTICS; ++i)
    {
        net_server_recv_t *recvobj;
        boolean need_resend;

        recvobj = &recvwindow[i];

        // if need_resend is true, this tic needs another retransmit
        // request (300ms timeout)

        need_resend = !recvobj->active
                   && recvobj->resend_time != 0
                   && nowtime > recvobj->resend_time + 300;

        if (need_resend)
        {
            // Start a new run of resend tics?
 
            if (resend_start < 0)
            {
                resend_start = i;
            }
            
            resend_end = i;
        }
        else
        {
            if (resend_start >= 0)
            {
                // End of a run of resend tics

                //printf("CL: resend request timed out: %i-%i\n", resend_start, resend_end);
                NET_CL_SendResendRequest(recvwindow_start + resend_start,
                                         recvwindow_start + resend_end);

                resend_start = -1;
            }
        }
    }

    if (resend_start >= 0)
    {
        //printf("CL: resend request timed out: %i-%i\n", resend_start, resend_end);
        NET_CL_SendResendRequest(recvwindow_start + resend_start,
                                 recvwindow_start + resend_end);
    }

    // We have received some data from the server and not acknowledged
    // it yet.  Normally this gets acknowledged when we send our game
    // data, but if the client is a drone we need to do this.

    if (need_to_acknowledge && nowtime - gamedata_recv_time > 200)
    {
        NET_CL_SendGameDataACK();
    }
}


// Parsing of NET_PACKET_TYPE_GAMEDATA packets
// (packets containing the actual ticcmd data)

static void NET_CL_ParseGameData(net_packet_t *packet)
{
    net_server_recv_t *recvobj;
    unsigned int seq, num_tics;
    unsigned int nowtime;
    int resend_start, resend_end;
    size_t i;
    int index;
    
    // Read header
    
    if (!NET_ReadInt8(packet, &seq)
     || !NET_ReadInt8(packet, &num_tics))
    {
        return;
    }

    nowtime = I_GetTimeMS();

    // Whatever happens, we now need to send an acknowledgement of our
    // current receive point.

    if (!need_to_acknowledge)
    {
        need_to_acknowledge = true;
        gamedata_recv_time = nowtime;
    }

    // Expand byte value into the full tic number

    seq = NET_CL_ExpandTicNum(seq);

    for (i=0; i<num_tics; ++i)
    {
        net_full_ticcmd_t cmd;

        index = seq - recvwindow_start + i;

        if (!NET_ReadFullTiccmd(packet, &cmd, settings.lowres_turn))
        {
            return;
        }

        if (index < 0 || index >= BACKUPTICS)
        {
            // Out of range of the recv window

            continue;
        }

        // Store in the receive window
        
        recvobj = &recvwindow[index];

        recvobj->active = true;
        recvobj->cmd = cmd;
    }

    // Has this been received out of sequence, ie. have we not received
    // all tics before the first tic in this packet?  If so, send a 
    // resend request.

    //printf("CL: %p: %i\n", client, seq);

    resend_end = seq - recvwindow_start;

    if (resend_end <= 0)
        return;

    if (resend_end >= BACKUPTICS)
        resend_end = BACKUPTICS - 1;

    index = resend_end - 1;
    resend_start = resend_end;
    
    while (index >= 0)
    {
        recvobj = &recvwindow[index];

        if (recvobj->active)
        {
            // ended our run of unreceived tics

            break;
        }

        if (recvobj->resend_time != 0)
        {
            // Already sent a resend request for this tic

            break;
        }

        resend_start = index;
        --index;
    }

    // Possibly send a resend request

    if (resend_start < resend_end)
    {
        NET_CL_SendResendRequest(recvwindow_start + resend_start, 
                                 recvwindow_start + resend_end - 1);
    }
}

// Parse a resend request from the server due to a dropped packet

static void NET_CL_ParseResendRequest(net_packet_t *packet)
{
    static unsigned int start;
    static unsigned int end;
    static unsigned int num_tics;

    if (drone)
    {
        // Drones don't send gamedata.

        return;
    }

    if (!NET_ReadInt32(packet, &start)
     || !NET_ReadInt8(packet, &num_tics))
    {
        return;
    }

    end = start + num_tics - 1;

    //printf("requested resend %i-%i .. ", start, end);

    // Check we have the tics being requested.  If not, reduce the 
    // window of tics to only what we have.

    while (start <= end
        && (!send_queue[start % BACKUPTICS].active
         || send_queue[start % BACKUPTICS].seq != start))
    {
        ++start;
    }
     
    while (start <= end
        && (!send_queue[end % BACKUPTICS].active
         || send_queue[end % BACKUPTICS].seq != end))
    {
        --end;
    }

    //printf("%i-%i\n", start, end);

    // Resend those tics

    if (start <= end)
    {
        //printf("CL: resend %i-%i\n", start, start+num_tics-1);

        NET_CL_SendTics(start, end);
    }
}

// Console message that the server wants the client to print

static void NET_CL_ParseConsoleMessage(net_packet_t *packet)
{
    char *msg;

    msg = NET_ReadSafeString(packet);

    if (msg == NULL)
    {
        return;
    }

    printf("Message from server:\n%s\n", msg);
}

// parse a received packet

static void NET_CL_ParsePacket(net_packet_t *packet)
{
    unsigned int packet_type;

    if (!NET_ReadInt16(packet, &packet_type))
    {
        return;
    }

    if (NET_Conn_Packet(&client_connection, packet, &packet_type))
    {
        // Packet eaten by the common connection code
    }
    else
    {
        switch (packet_type)
        {
            case NET_PACKET_TYPE_SYN:
                NET_CL_ParseSYN(packet);
                break;

            case NET_PACKET_TYPE_WAITING_DATA:
                NET_CL_ParseWaitingData(packet);
                break;

            case NET_PACKET_TYPE_LAUNCH:
                NET_CL_ParseLaunch(packet);
                break;

            case NET_PACKET_TYPE_GAMESTART:
                NET_CL_ParseGameStart(packet);
                break;

            case NET_PACKET_TYPE_GAMEDATA:
                NET_CL_ParseGameData(packet);
                break;

            case NET_PACKET_TYPE_GAMEDATA_RESEND:
                NET_CL_ParseResendRequest(packet);
                break;

            case NET_PACKET_TYPE_CONSOLE_MESSAGE:
                NET_CL_ParseConsoleMessage(packet);
                break;

            default:
                break;
        }
    }
}

// "Run" the client code: check for new packets, send packets as
// needed

void NET_CL_Run(void)
{
    net_addr_t *addr;
    net_packet_t *packet;
    
    if (!net_client_connected)
    {
        return;
    }
    
    while (NET_RecvPacket(client_context, &addr, &packet))
    {
        // only accept packets from the server

        if (addr == server_addr)
        {
            NET_CL_ParsePacket(packet);
        }
        else
        {
            NET_FreeAddress(addr);
        }

        NET_FreePacket(packet);
    }

    // Run the common connection code to send any packets as needed

    NET_Conn_Run(&client_connection);

    if (client_connection.state == NET_CONN_STATE_DISCONNECTED
     || client_connection.state == NET_CONN_STATE_DISCONNECTED_SLEEP)
    {
        NET_CL_Disconnected();

        NET_CL_Shutdown();
    }

    net_waiting_for_launch =
            client_connection.state == NET_CONN_STATE_CONNECTED
         && client_state == CLIENT_STATE_WAITING_LAUNCH;

    if (client_state == CLIENT_STATE_IN_GAME)
    {
        // Possibly advance the receive window

        NET_CL_AdvanceWindow();

        // Check if our resend requests have timed out

        NET_CL_CheckResends();
    }
}

static void NET_CL_SendSYN(net_connect_data_t *data)
{
    net_packet_t *packet;

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_SYN);
    NET_WriteInt32(packet, NET_MAGIC_NUMBER);
    NET_WriteString(packet, PACKAGE_STRING);
    NET_WriteProtocolList(packet);
    NET_WriteConnectData(packet, data);
    NET_WriteString(packet, net_player_name);
    NET_Conn_SendPacket(&client_connection, packet);
    NET_FreePacket(packet);
}

// Connect to a server
boolean NET_CL_Connect(net_addr_t *addr, net_connect_data_t *data)
{
    int start_time;
    int last_send_time;

    server_addr = addr;

    memcpy(net_local_wad_sha1sum, data->wad_sha1sum, sizeof(sha1_digest_t));
    memcpy(net_local_deh_sha1sum, data->deh_sha1sum, sizeof(sha1_digest_t));
    net_local_is_freedoom = data->is_freedoom;

    // create a new network I/O context and add just the
    // necessary module

    client_context = NET_NewContext();

    // initialize module for client mode

    if (!addr->module->InitClient())
    {
        return false;
    }

    NET_AddModule(client_context, addr->module);

    net_client_connected = true;
    net_client_received_wait_data = false;

    // Initialize connection

    NET_Conn_InitClient(&client_connection, addr, NET_PROTOCOL_UNKNOWN);

    // try to connect

    start_time = I_GetTimeMS();
    last_send_time = -1;

    while (client_connection.state == NET_CONN_STATE_CONNECTING)
    {
        int nowtime = I_GetTimeMS();

        // Send a SYN packet every second.

        if (nowtime - last_send_time > 1000 || last_send_time < 0)
        {
            NET_CL_SendSYN(data);
            last_send_time = nowtime;
        }

        // time out after 5 seconds

        if (nowtime - start_time > 5000)
        {
            break;
        }

        // run client code

        NET_CL_Run();

        // run the server, just incase we are doing a loopback
        // connect

        NET_SV_Run();

        // Don't hog the CPU

        I_Sleep(1);
    }

    if (client_connection.state == NET_CONN_STATE_CONNECTED)
    {
        // connected ok!

        client_state = CLIENT_STATE_WAITING_LAUNCH;
        drone = data->drone;

        return true;
    }
    else
    {
        // failed to connect

        NET_CL_Shutdown();

        return false;
    }
}

// read game settings received from server

boolean NET_CL_GetSettings(net_gamesettings_t *_settings)
{
    if (client_state != CLIENT_STATE_IN_GAME)
    {
        return false;
    }

    memcpy(_settings, &settings, sizeof(net_gamesettings_t));

    return true;
}

// disconnect from the server

void NET_CL_Disconnect(void)
{
    int start_time;

    if (!net_client_connected)
    {
        return;
    }
    
    NET_Conn_Disconnect(&client_connection);

    start_time = I_GetTimeMS();

    while (client_connection.state != NET_CONN_STATE_DISCONNECTED
        && client_connection.state != NET_CONN_STATE_DISCONNECTED_SLEEP)
    {
        if (I_GetTimeMS() - start_time > 5000)
        {
            // time out after 5 seconds

            client_state = CLIENT_STATE_WAITING_START;

            fprintf(stderr, "NET_CL_Disconnect: Timeout while disconnecting from server\n");
            break;
        }

        NET_CL_Run();
        NET_SV_Run();

        I_Sleep(1);
    }

    // Finished sending disconnect packets, etc.

    NET_CL_Shutdown();
}

void NET_CL_Init(void)
{
    // Try to set from the USER and USERNAME environment variables
    // Otherwise, fallback to "Player"

    if (net_player_name == NULL) 
        net_player_name = getenv("USER");
    if (net_player_name == NULL)
        net_player_name = getenv("USERNAME");

    // On Windows, environment variables are in OEM codepage
    // encoding, so convert to UTF8:

#ifdef _WIN32
    if (net_player_name != NULL)
    {
        net_player_name = M_OEMToUTF8(net_player_name);
    }
#endif

    if (net_player_name == NULL)
        net_player_name = "Player";
}

void NET_Init(void)
{
    NET_CL_Init();
}

void NET_BindVariables(void)
{
    M_BindStringVariable("player_name", &net_player_name);
}
