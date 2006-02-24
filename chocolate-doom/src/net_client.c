// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2005 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// $Log$
// Revision 1.32  2006/02/24 19:14:59  fraggle
// Fix -extratics
//
// Revision 1.31  2006/02/23 20:53:03  fraggle
// Detect when clients are disconnected from the server, recover cleanly
// and display a message.
//
// Revision 1.30  2006/02/23 20:31:09  fraggle
// Set ticdup from the command line with the -dup parameter.
//
// Revision 1.29  2006/02/23 19:12:01  fraggle
// Add lowres_turn to indicate whether we generate angleturns which are
// 8-bit as opposed to 16-bit.  This is used when recording demos without
// -longtics enabled.  Sync this option between clients in a netgame, so
// that if one player is recording a Vanilla demo, all clients record
// in lowres.
//
// Revision 1.28  2006/02/23 18:20:29  fraggle
// Fix bugs in resend code for server->client data
//
// Revision 1.27  2006/02/22 18:35:55  fraggle
// Packet resends for server->client gamedata
//
// Revision 1.26  2006/02/19 13:42:27  fraggle
// Move tic number expansion code to common code.  Parse game data packets
// received from the server.
// Strip down d_net.[ch] to work through the new networking code.  Remove
// game sync code.
// Remove i_net.[ch] as it is no longer needed.
// Working networking!
//
// Revision 1.25  2006/02/17 21:40:52  fraggle
// Full working resends for client->server comms
//
// Revision 1.24  2006/02/17 20:15:16  fraggle
// Request resends for missed packets
//
// Revision 1.23  2006/01/22 22:29:42  fraggle
// Periodically request the time from clients to estimate their offset to
// the server time.
//
// Revision 1.22  2006/01/21 14:16:49  fraggle
// Add first game data sending code. Check the client version when connecting.
//
// Revision 1.21  2006/01/14 02:06:48  fraggle
// Include the game version in the settings structure.
//
// Revision 1.20  2006/01/13 23:52:12  fraggle
// Fix game start packet parsing logic.
//
// Revision 1.19  2006/01/13 02:19:18  fraggle
// Only accept sane player values when starting a new game.
//
// Revision 1.18  2006/01/12 02:18:59  fraggle
// Only start new games when in the waiting-for-start state.
//
// Revision 1.17  2006/01/12 02:11:52  fraggle
// Game start packets
//
// Revision 1.16  2006/01/10 19:59:25  fraggle
// Reliable packet transport mechanism
//
// Revision 1.15  2006/01/09 02:03:39  fraggle
// Send clients their player number, and indicate on the waiting screen
// which client we are.
//
// Revision 1.14  2006/01/09 01:50:51  fraggle
// Deduce a sane player name by examining environment variables.  Add
// a "player_name" setting to chocolate-doom.cfg.  Transmit the name
// to the server and use the names players send in the waiting data list.
//
// Revision 1.13  2006/01/08 04:52:26  fraggle
// Allow the server to reject clients
//
// Revision 1.12  2006/01/08 03:36:40  fraggle
// Fix double free of addresses
//
// Revision 1.11  2006/01/08 02:53:31  fraggle
// Detect when client connection is disconnected.
//
// Revision 1.10  2006/01/08 00:10:47  fraggle
// Move common connection code into net_common.c, shared by server
// and client code.
//
// Revision 1.9  2006/01/07 20:08:11  fraggle
// Send player name and address in the waiting data packets.  Display these
// on the waiting screen, and improve the waiting screen appearance.
//
// Revision 1.8  2006/01/02 21:50:26  fraggle
// Restructure the waiting screen code.  Establish our own separate event
// loop while waiting for the game to start, to avoid affecting the original
// code too much.  Move some _gui variables to net_client.c.
//
// Revision 1.7  2006/01/02 20:14:07  fraggle
// Fix connect timeout and shutdown client properly if we fail to connect.
//
// Revision 1.6  2006/01/02 00:54:17  fraggle
// Fix packet not freed back after being sent.
// Code to disconnect clients from the server side.
//
// Revision 1.5  2006/01/02 00:00:08  fraggle
// Neater prefixes: NET_Client -> NET_CL_.  NET_Server -> NET_SV_.
//
// Revision 1.4  2006/01/01 23:54:31  fraggle
// Client disconnect code
//
// Revision 1.3  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//
// Revision 1.2  2005/12/29 21:29:55  fraggle
// Working client connect code
//
// Revision 1.1  2005/12/29 17:48:25  fraggle
// Add initial client/server connect code.  Reorganise sources list in
// Makefile.am.
//
//
// Network client code
//

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "doomdef.h"
#include "doomstat.h"
#include "deh_main.h"
#include "g_game.h"
#include "i_system.h"
#include "m_argv.h"
#include "net_client.h"
#include "net_common.h"
#include "net_defs.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_server.h"
#include "net_structrw.h"

typedef enum
{
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

static net_connection_t client_connection;
static net_clientstate_t client_state;
static net_addr_t *server_addr;
static net_context_t *client_context;

// TRUE if the client code is in use

boolean net_client_connected;

// if TRUE, this client is the controller of the game

boolean net_client_controller = false;

// Number of clients currently connected to the server

int net_clients_in_game;

// Names of all players

char net_player_addresses[MAXPLAYERS][MAXPLAYERNAME];
char net_player_names[MAXPLAYERS][MAXPLAYERNAME];

// Player number

int net_player_number;

// Waiting for the game to start?

boolean net_waiting_for_start = false;

// Name that we send to the server

char *net_player_name = NULL;

// The last ticcmd constructed

static ticcmd_t last_ticcmd;

// Buffer of ticcmd diffs being sent to the server

static net_server_send_t send_queue[NET_TICCMD_QUEUE_SIZE];

// Receive window

static ticcmd_t recvwindow_cmd_base[MAXPLAYERS];
static int recvwindow_start;
static net_server_recv_t recvwindow[BACKUPTICS];

#define NET_CL_ExpandTicNum(b) NET_ExpandTicNum(recvwindow_start, (b))

// Called when a player leaves the game

static void NET_CL_PlayerQuitGame(player_t *player)
{
    static char exitmsg[80];

    // Do this the same way as Vanilla Doom does, to allow dehacked
    // replacements of this message

    strcpy(exitmsg, DEH_String("Player 1 left the game"));

    exitmsg[7] += player - players;

    players[consoleplayer].message = exitmsg;

    // TODO: check if it is sensible to do this:

    if (demorecording) 
    {
        G_CheckDemoStatus ();
    }
}

// Called when we become disconnected from the server

static void NET_CL_Disconnected(void)
{
    int i;

    // disconnected from server

    players[consoleplayer].message = "Disconnected from server";

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (i != consoleplayer)
            playeringame[i] = false;
    }
}

// Expand a net_full_ticcmd_t, applying the diffs in cmd->cmds as
// patches against recvwindow_cmd_base.  Place the results into
// the d_net.c structures (netcmds/nettics) and save the new ticcmd
// back into recvwindow_cmd_base.

static void NET_CL_ExpandFullTiccmd(net_full_ticcmd_t *cmd)
{
    int i;

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (i == consoleplayer)
        {
            continue;
        }
        
        if (playeringame[i] && !cmd->playeringame[i])
        {
            NET_CL_PlayerQuitGame(&players[i]);
        }
        
        playeringame[i] = cmd->playeringame[i];

        if (playeringame[i])
        {
            net_ticdiff_t *diff;
            ticcmd_t ticcmd;

            diff = &cmd->cmds[i];

            // Use the ticcmd diff to patch the previous ticcmd to
            // the new ticcmd

            NET_TiccmdPatch(&recvwindow_cmd_base[i], diff, &ticcmd);

            // Save in d_net.c structures

            netcmds[i][nettics[i] % BACKUPTICS] = ticcmd;
            ++nettics[i];

            // Store a copy for next time

            recvwindow_cmd_base[i] = ticcmd;
        }
    }
}

// Advance the receive window

static void NET_CL_AdvanceWindow(void)
{
    while (recvwindow[0].active)
    {
        // Expand tic diff data into d_net.c structures

        NET_CL_ExpandFullTiccmd(&recvwindow[0].cmd);

        // Advance the window

        memcpy(recvwindow, recvwindow + 1, 
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

void NET_CL_StartGame(void)
{
    net_packet_t *packet;
    net_gamesettings_t settings;
    int i;

    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings.deathmatch = deathmatch;
    settings.episode = startepisode;
    settings.map = startmap;
    settings.skill = startskill;
    settings.gameversion = gameversion;

    i = M_CheckParm("-extratics");

    if (i > 0)
        settings.extratics = atoi(myargv[i+1]);
    else
        settings.extratics = 1;

    i = M_CheckParm("-dup");

    if (i > 0)
        settings.ticdup = atoi(myargv[i+1]);
    else
        settings.ticdup = 1;

    // Start from a ticcmd of all zeros

    memset(&last_ticcmd, 0, sizeof(ticcmd_t));
    
    // Send packet

    packet = NET_Conn_NewReliable(&client_connection, 
                                  NET_PACKET_TYPE_GAMESTART);

    NET_WriteSettings(packet, &settings);
}

static void NET_CL_SendTics(int start, int end)
{
    net_packet_t *packet;
    int i;

    if (start < 0)
        start = 0;
    
    // Build a new packet to send to the server

    packet = NET_NewPacket(512);
    NET_WriteInt16(packet, NET_PACKET_TYPE_GAMEDATA);

    // Write the start tic and number of tics.  Send only the low byte
    // of start - it can be inferred by the server.

    NET_WriteInt8(packet, start & 0xff);
    NET_WriteInt8(packet, end - start + 1);

    // Add the tics.

    for (i=start; i<=end; ++i)
    {
        net_server_send_t *sendobj;

        sendobj = &send_queue[i % NET_TICCMD_QUEUE_SIZE];

        NET_WriteInt16(packet, sendobj->time);

        NET_WriteTiccmdDiff(packet, &sendobj->cmd, lowres_turn);
    }
    
    // Send the packet

    NET_Conn_SendPacket(&client_connection, packet);
    
    // All done!

    NET_FreePacket(packet);
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

    sendobj = &send_queue[maketic % NET_TICCMD_QUEUE_SIZE];
    sendobj->active = true;
    sendobj->seq = maketic;
    sendobj->time = I_GetTimeMS();
    sendobj->cmd = diff;

    last_ticcmd = *ticcmd;

    // Send to server.

    starttic = maketic - extratics;
    endtic = maketic;

    if (starttic < 0)
        starttic = 0;
    
    NET_CL_SendTics(starttic, endtic);
}

// data received while we are waiting for the game to start

static void NET_CL_ParseWaitingData(net_packet_t *packet)
{
    unsigned int num_players;
    unsigned int is_controller;
    unsigned int player_number;
    char *player_names[MAXPLAYERS];
    char *player_addr[MAXPLAYERS];
    int i;

    if (!NET_ReadInt8(packet, &num_players)
     || !NET_ReadInt8(packet, &is_controller)
     || !NET_ReadInt8(packet, &player_number))
    {
        // invalid packet

        return;
    }

    if (num_players > MAXPLAYERS 
     || player_number >= num_players)
    {
        // insane data

        return;
    }
 
    // Read the player names

    for (i=0; i<num_players; ++i)
    {
        player_names[i] = NET_ReadString(packet);
        player_addr[i] = NET_ReadString(packet);

        if (player_names[i] == NULL || player_addr[i] == NULL)
        {
            return;
        }
    }

    net_clients_in_game = num_players;
    net_client_controller = is_controller != 0;
    net_player_number = player_number;

    for (i=0; i<num_players; ++i)
    {
        strncpy(net_player_names[i], player_names[i], MAXPLAYERNAME);
        net_player_names[i][MAXPLAYERNAME-1] = '\0';
        strncpy(net_player_addresses[i], player_addr[i], MAXPLAYERNAME);
        net_player_addresses[i][MAXPLAYERNAME-1] = '\0';
    }
}

static void NET_CL_ParseGameStart(net_packet_t *packet)
{
    net_gamesettings_t settings;
    unsigned int player_number, num_players;
    int i;

    if (!NET_ReadInt8(packet, &num_players)
     || !NET_ReadInt8(packet, &player_number)
     || !NET_ReadSettings(packet, &settings))
    {
        return;
    }

    if (client_state != CLIENT_STATE_WAITING_START)
    {
        return;
    }

    if (num_players >= MAXPLAYERS || player_number >= num_players)
    {
        // insane values
        return;
    }

    // Start the game

    consoleplayer = player_number;
    
    for (i=0; i<MAXPLAYERS; ++i) 
    {
        playeringame[i] = i < num_players;
    }

    client_state = CLIENT_STATE_IN_GAME;

    deathmatch = settings.deathmatch;
    ticdup = settings.ticdup;
    extratics = settings.extratics;
    startepisode = settings.episode;
    startmap = settings.map;
    startskill = settings.skill;
    lowres_turn = settings.lowres_turn;

    memset(recvwindow, 0, sizeof(recvwindow));
    recvwindow_start = 0;
    memset(&recvwindow_cmd_base, 0, sizeof(recvwindow_cmd_base));

    netgame = true;
    autostart = true;
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
}


// Parsing of NET_PACKET_TYPE_GAMEDATA packets
// (packets containing the actual ticcmd data)

static void NET_CL_ParseGameData(net_packet_t *packet)
{
    net_server_recv_t *recvobj;
    unsigned int seq, num_tics;
    unsigned int nowtime;
    int resend_start, resend_end;
    int i;
    int index;
    
    // Read header
    
    if (!NET_ReadInt8(packet, &seq)
     || !NET_ReadInt8(packet, &num_tics))
    {
        return;
    }

    nowtime = I_GetTimeMS();

    // Expand byte value into the full tic number

    seq = NET_CL_ExpandTicNum(seq);

    for (i=0; i<num_tics; ++i)
    {
        net_full_ticcmd_t cmd;

        index = seq - recvwindow_start + i;

        if (!NET_ReadFullTiccmd(packet, &cmd, lowres_turn))
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

static void NET_CL_ParseTimeRequest(net_packet_t *packet)
{
    net_packet_t *reply;
    unsigned int seq;
    
    // Received a request from the server for our current time.

    if (!NET_ReadInt32(packet, &seq))
    {
	return;
    }
    
    // Send a response with our current time.

    reply = NET_NewPacket(10);
    NET_WriteInt16(reply, NET_PACKET_TYPE_TIME_RESP);
    NET_WriteInt32(reply, seq);
    NET_WriteInt32(reply, I_GetTimeMS());
    NET_Conn_SendPacket(&client_connection, reply);
    NET_FreePacket(reply);
}

// Parse a resend request from the server due to a dropped packet

static void NET_CL_ParseResendRequest(net_packet_t *packet)
{
    static unsigned int start;
    static unsigned int num_tics;

    if (!NET_ReadInt32(packet, &start)
     || !NET_ReadInt8(packet, &num_tics))
    {
        return;
    }

    // Resend those tics

    //printf("CL: resend %i-%i\n", start, start+num_tics-1);

    NET_CL_SendTics(start, start + num_tics - 1);
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
            case NET_PACKET_TYPE_WAITING_DATA:
                NET_CL_ParseWaitingData(packet);
                break;

            case NET_PACKET_TYPE_GAMESTART:
                NET_CL_ParseGameStart(packet);
                break;

            case NET_PACKET_TYPE_GAMEDATA:
                NET_CL_ParseGameData(packet);
                break;

	    case NET_PACKET_TYPE_TIME_REQ:
		NET_CL_ParseTimeRequest(packet);
		break;

            case NET_PACKET_TYPE_GAMEDATA_RESEND:
                NET_CL_ParseResendRequest(packet);
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
    
    net_waiting_for_start = client_connection.state == NET_CONN_STATE_CONNECTED
                         && client_state == CLIENT_STATE_WAITING_START;

    if (client_state == CLIENT_STATE_IN_GAME)
    {
        // Possibly advance the receive window

        NET_CL_AdvanceWindow();

        // Check if our resend requests have timed out

        NET_CL_CheckResends();
    }
}

static void NET_CL_SendSYN(void)
{
    net_packet_t *packet;

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_SYN);
    NET_WriteInt32(packet, NET_MAGIC_NUMBER);
    NET_WriteInt16(packet, gamemode);
    NET_WriteInt16(packet, gamemission);
    NET_WriteInt8(packet, lowres_turn);
    NET_WriteString(packet, net_player_name);
    NET_WriteString(packet, PACKAGE_STRING);
    NET_Conn_SendPacket(&client_connection, packet);
    NET_FreePacket(packet);
}

// connect to a server

boolean NET_CL_Connect(net_addr_t *addr)
{
    int start_time;
    int last_send_time;

    server_addr = addr;

    // Are we recording a demo? Possibly set lowres turn mode

    if (M_CheckParm("-record") > 0 && M_CheckParm("-longtics") == 0)
    {
        lowres_turn = true;
    }

    // create a new network I/O context and add just the
    // necessary module

    client_context = NET_NewContext();
    
    // initialise module for client mode

    if (!addr->module->InitClient())
    {
        return false;
    }

    NET_AddModule(client_context, addr->module);

    net_client_connected = true;

    // Initialise connection

    NET_Conn_InitClient(&client_connection, addr);

    // try to connect
 
    start_time = I_GetTimeMS();
    last_send_time = -1;

    while (client_connection.state == NET_CONN_STATE_CONNECTING)
    {
        int nowtime = I_GetTimeMS();

        // Send a SYN packet every second.

        if (nowtime - last_send_time > 1000 || last_send_time < 0)
        {
            NET_CL_SendSYN();
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

        I_Sleep(10);
    }

    if (client_connection.state == NET_CONN_STATE_CONNECTED)
    {
        // connected ok!

        client_state = CLIENT_STATE_WAITING_START;

        return true;
    }
    else
    {
        // failed to connect

        NET_CL_Shutdown();
        
        return false;
    }
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
            
            client_state = NET_CONN_STATE_DISCONNECTED;

            fprintf(stderr, "NET_CL_Disconnect: Timeout while disconnecting from server\n");
            break;
        }

        NET_CL_Run();
        NET_SV_Run();

        I_Sleep(10);
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
    if (net_player_name == NULL)
        net_player_name = "Player";
}

void NET_Init(void)
{
    NET_CL_Init();
}

