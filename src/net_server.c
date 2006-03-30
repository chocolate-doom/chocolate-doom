// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_server.c 455 2006-03-30 19:08:37Z fraggle $
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
// Revision 1.35  2006/02/24 19:14:59  fraggle
// Fix -extratics
//
// Revision 1.34  2006/02/24 08:19:45  fraggle
// Only advance the receive window if we have received ticcmds from all
// connected players.
//
// Revision 1.33  2006/02/23 23:40:30  fraggle
// Free back packets sent to the server after parsing them
//
// Revision 1.32  2006/02/23 19:15:18  fraggle
// Fix crash when NOT recording lowres
//
// Revision 1.31  2006/02/23 19:12:02  fraggle
// Add lowres_turn to indicate whether we generate angleturns which are
// 8-bit as opposed to 16-bit.  This is used when recording demos without
// -longtics enabled.  Sync this option between clients in a netgame, so
// that if one player is recording a Vanilla demo, all clients record
// in lowres.
//
// Revision 1.30  2006/02/23 18:20:29  fraggle
// Fix bugs in resend code for server->client data
//
// Revision 1.29  2006/02/22 18:35:55  fraggle
// Packet resends for server->client gamedata
//
// Revision 1.28  2006/02/19 13:42:27  fraggle
// Move tic number expansion code to common code.  Parse game data packets
// received from the server.
// Strip down d_net.[ch] to work through the new networking code.  Remove
// game sync code.
// Remove i_net.[ch] as it is no longer needed.
// Working networking!
//
// Revision 1.27  2006/02/17 21:42:13  fraggle
// Remove debug code
//
// Revision 1.26  2006/02/17 21:40:52  fraggle
// Full working resends for client->server comms
//
// Revision 1.25  2006/02/17 20:15:16  fraggle
// Request resends for missed packets
//
// Revision 1.24  2006/02/16 01:12:28  fraggle
// Define a new type net_full_ticcmd_t, a structure containing all ticcmds
// for a given tic.  Store received game data in a receive window.  Add
// send queues for clients and add data from the receive window to
// generate complete sets of ticcmds.
//
// Revision 1.23  2006/01/22 22:29:42  fraggle
// Periodically request the time from clients to estimate their offset to
// the server time.
//
// Revision 1.22  2006/01/21 14:16:49  fraggle
// Add first game data sending code. Check the client version when connecting.
//
// Revision 1.21  2006/01/12 02:18:59  fraggle
// Only start new games when in the waiting-for-start state.
//
// Revision 1.20  2006/01/12 02:11:52  fraggle
// Game start packets
//
// Revision 1.19  2006/01/10 19:59:26  fraggle
// Reliable packet transport mechanism
//
// Revision 1.18  2006/01/09 02:03:39  fraggle
// Send clients their player number, and indicate on the waiting screen
// which client we are.
//
// Revision 1.17  2006/01/09 01:50:51  fraggle
// Deduce a sane player name by examining environment variables.  Add
// a "player_name" setting to chocolate-doom.cfg.  Transmit the name
// to the server and use the names players send in the waiting data list.
//
// Revision 1.16  2006/01/08 05:06:06  fraggle
// Reject new connections if the server is not in the waiting state.
//
// Revision 1.15  2006/01/08 04:52:26  fraggle
// Allow the server to reject clients
//
// Revision 1.14  2006/01/08 03:36:17  fraggle
// Fix packet send
//
// Revision 1.13  2006/01/08 02:53:05  fraggle
// Send keepalives if the connection is not doing anything else.
// Send all packets using a new NET_Conn_SendPacket to support this.
//
// Revision 1.12  2006/01/08 00:10:48  fraggle
// Move common connection code into net_common.c, shared by server
// and client code.
//
// Revision 1.11  2006/01/07 20:08:11  fraggle
// Send player name and address in the waiting data packets.  Display these
// on the waiting screen, and improve the waiting screen appearance.
//
// Revision 1.10  2006/01/02 21:48:37  fraggle
// fix client connected function
//
// Revision 1.9  2006/01/02 21:04:10  fraggle
// Create NET_SV_Shutdown function to shut down the server.  Call it
// when quitting the game.  Print the IP of the server correctly when
// connecting.
//
// Revision 1.8  2006/01/02 20:13:06  fraggle
// Refer to connected clients by their AddrToString() output rather than just
// the pointer to their struct.  Listen for IP connections as well as
// loopback connections.
//
// Revision 1.7  2006/01/02 17:24:40  fraggle
// Remove test code
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
// Network server code
//

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "i_timer.h"

#include "net_client.h"
#include "net_common.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_loop.h"
#include "net_packet.h"
#include "net_server.h"
#include "net_sdl.h"
#include "net_structrw.h"

typedef enum
{
    // waiting for the game to start

    SERVER_WAITING_START,

    // in a game

    SERVER_IN_GAME,
} net_server_state_t;

typedef struct 
{
    boolean active;
    int player_number;
    net_addr_t *addr;
    net_connection_t connection;
    int last_send_time;
    char *name;

    // recording a demo without -longtics

    boolean recording_lowres;

    // send queue: items to send to the client
    // this is a circular buffer

    int sendseq;
    net_full_ticcmd_t sendqueue[BACKUPTICS];

} net_client_t;

// structure used for the recv window

typedef struct 
{
    // Whether this tic has been received yet

    boolean active;

    // Latency value received from the client

    signed int latency;

    // Last time we sent a resend request for this tic

    unsigned int resend_time;

    // Tic data itself

    net_ticdiff_t diff;
} net_client_recv_t;

static net_server_state_t server_state;
static boolean server_initialised = false;
static net_client_t clients[MAXNETNODES];
static net_client_t *sv_players[MAXPLAYERS];
static net_context_t *server_context;
static int sv_gamemode;
static int sv_gamemission;
static net_gamesettings_t sv_settings;

// receive window

static unsigned int recvwindow_start;
static net_client_recv_t recvwindow[BACKUPTICS][MAXPLAYERS];

#define NET_SV_ExpandTicNum(b) NET_ExpandTicNum(recvwindow_start, (b))

static void NET_SV_DisconnectClient(net_client_t *client)
{
    if (client->active)
    {
        NET_Conn_Disconnect(&client->connection);
    }
}

static boolean ClientConnected(net_client_t *client)
{
    // Check that the client is properly connected: ie. not in the 
    // process of connecting or disconnecting

    return client->active 
        && client->connection.state == NET_CONN_STATE_CONNECTED;
}

// Send a message to be displayed on a client's console

static void NET_SV_SendConsoleMessage(net_client_t *client, char *s, ...)
{
    char buf[1024];
    va_list args;
    net_packet_t *packet;

    va_start(args, s);
    vsnprintf(buf, sizeof(buf), s, args);
    va_end(args);
    
    packet = NET_Conn_NewReliable(&client->connection, 
                                  NET_PACKET_TYPE_CONSOLE_MESSAGE);

    NET_WriteString(packet, buf);
}

// Send a message to all clients

static void NET_SV_BroadcastMessage(char *s, ...)
{
    char buf[1024];
    va_list args;
    int i;

    va_start(args, s);
    vsnprintf(buf, sizeof(buf), s, args);
    va_end(args);
    
    for (i=0; i<MAXNETNODES; ++i)
    {
        if (ClientConnected(&clients[i]))
        {
            NET_SV_SendConsoleMessage(&clients[i], buf);
        }
    }

    NET_SafePuts(buf);
}


// Assign player numbers to connected clients

static void NET_SV_AssignPlayers(void)
{
    int i;
    int pl;

    pl = 0;

    for (i=0; i<MAXNETNODES; ++i)
    {
        if (ClientConnected(&clients[i]))
        {
            sv_players[pl] = &clients[i];
            sv_players[pl]->player_number = pl;
            ++pl;
        }
    }

    for (; pl<MAXPLAYERS; ++pl)
    {
        sv_players[pl] = NULL;
    }
}

// returns the number of clients connected

static int NET_SV_NumClients(void)
{
    int count;
    int i;

    count = 0;

    for (i=0; i<MAXNETNODES; ++i)
    {
        if (ClientConnected(&clients[i]))
        {
            ++count;
        }
    }

    return count;
}

// Returns the index of a particular client in the list of connected
// clients.

static int NET_SV_ClientIndex(net_client_t *client)
{
    int count;
    int i;

    count = 0;

    for (i=0; i<MAXNETNODES; ++i)
    {
        if (ClientConnected(&clients[i]))
        {
            if (client == &clients[i])
            {
                return count;
            }
            ++count;
        }
    }

    return -1;
}

// Possibly advance the recv window if all connected clients have
// used the data in the window

static void NET_SV_AdvanceWindow(void)
{
    int i;
    int lowtic = -1;

    // Find the smallest value of player->sendseq for all connected
    // players

    for (i=0; i<MAXPLAYERS; ++i) 
    {
        if (sv_players[i] == NULL || !ClientConnected(sv_players[i]))
        {
            continue;
        }

        if (lowtic < 0 || sv_players[i]->sendseq < lowtic)
        {
            lowtic = sv_players[i]->sendseq;
        }
    }

    if (lowtic < 0)
    {
        return;
    }

    // Advance the recv window until it catches up with lowtic

    while (recvwindow_start < lowtic)
    {    
        boolean should_advance;

        // Check we have tics from all players for first tic in
        // the recv window
        
        should_advance = true;

        for (i=0; i<MAXPLAYERS; ++i)
        {
            if (sv_players[i] == NULL || !ClientConnected(sv_players[i]))
            {
                continue;
            }

            if (!recvwindow[0][i].active)
            {
                should_advance = false;
                break;
            }
        }

        if (!should_advance)
        {
            // The first tic is not complete: ie. we have not 
            // received tics from all connected players.  This can
            // happen if only one player is in the game.

            break;
        }
        
        // Advance the window

        memcpy(recvwindow, recvwindow + 1, sizeof(*recvwindow) * (BACKUPTICS - 1));
        memset(&recvwindow[BACKUPTICS-1], 0, sizeof(*recvwindow));
        ++recvwindow_start;

        //printf("SV: advanced to %i\n", recvwindow_start);
    }
}

// returns a pointer to the client which controls the server

static net_client_t *NET_SV_Controller(void)
{
    int i;

    // first client in the list is the controller

    for (i=0; i<MAXNETNODES; ++i)
    {
        if (ClientConnected(&clients[i]))
        {
            return &clients[i];
        }
    }

    return NULL;
}

// Given an address, find the corresponding client

static net_client_t *NET_SV_FindClient(net_addr_t *addr)
{
    int i;

    for (i=0; i<MAXNETNODES; ++i) 
    {
        if (clients[i].active && clients[i].addr == addr)
        {
            // found the client

            return &clients[i];
        }
    }

    return NULL;
}

// send a rejection packet to a client

static void NET_SV_SendReject(net_addr_t *addr, char *msg)
{
    net_packet_t *packet;

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_REJECTED);
    NET_WriteString(packet, msg);
    NET_SendPacket(addr, packet);
    NET_FreePacket(packet);
}

static void NET_SV_InitNewClient(net_client_t *client, 
                                 net_addr_t *addr,
                                 char *player_name)
{
    client->active = true;
    NET_Conn_InitServer(&client->connection, addr);
    client->addr = addr;
    client->last_send_time = -1;
    client->name = strdup(player_name);

    // init the ticcmd send queue

    client->sendseq = 0;
    memset(client->sendqueue, 0xff, sizeof(client->sendqueue));
}

// parse a SYN from a client(initiating a connection)

static void NET_SV_ParseSYN(net_packet_t *packet, 
                            net_client_t *client,
                            net_addr_t *addr)
{
    unsigned int magic;
    unsigned int cl_gamemode, cl_gamemission;
    unsigned int cl_recording_lowres;
    char *player_name;
    char *client_version;
    int i;

    // read the magic number

    if (!NET_ReadInt32(packet, &magic))
    {
        return;
    }

    if (magic != NET_MAGIC_NUMBER)
    {
        // invalid magic number

        return;
    }

    // read the game mode and mission

    if (!NET_ReadInt16(packet, &cl_gamemode) 
     || !NET_ReadInt16(packet, &cl_gamemission)
     || !NET_ReadInt8(packet, &cl_recording_lowres))
    {
        return;
    }

    // read the player's name

    player_name = NET_ReadString(packet);

    if (player_name == NULL)
    {
        return;
    }

    client_version = NET_ReadString(packet);

    if (client_version == NULL)
    {
        return;
    }
    
    // received a valid SYN

    // not accepting new connections?
    
    if (server_state != SERVER_WAITING_START)
    {
        NET_SV_SendReject(addr, "Server is not currently accepting connections");
    }
    
    // allocate a client slot if there isn't one already

    if (client == NULL)
    {
        // find a slot, or return if none found

        for (i=0; i<MAXNETNODES; ++i)
        {
            if (!clients[i].active)
            {
                client = &clients[i];
                break;
            }
        }

        if (client == NULL)
        {
            return;
        }
    }
    else
    {
        // If this is a recently-disconnected client, deactivate
        // to allow immediate reconnection

        if (client->connection.state == NET_CONN_STATE_DISCONNECTED)
        {
            client->active = false;
        }
    }

    // New client?

    if (!client->active)
    {
        int num_clients;

        // Before accepting a new client, check that there is a slot
        // free

        num_clients = NET_SV_NumClients();

        if (num_clients >= MAXPLAYERS)
        {
            NET_SV_SendReject(addr, "Server is full!");
            return;
        }

        if (strcmp(client_version, PACKAGE_STRING) != 0)
        {
            NET_SV_SendReject(addr, "Different versions cannot play a network game!");
            return;
        }

        // TODO: Add server option to allow rejecting clients which
        // set lowres_turn.  This is potentially desirable as the 
        // presence of such clients affects turning resolution.

        // Adopt the game mode and mission of the first connecting client

        if (num_clients == 0)
        {
            sv_gamemode = cl_gamemode;
            sv_gamemission = cl_gamemission;
        }

        // Check the connecting client is playing the same game as all
        // the other clients

        if (cl_gamemode != sv_gamemode || cl_gamemission != sv_gamemission)
        {
            NET_SV_SendReject(addr, "You are playing the wrong game!");
            return;
        }
        
        // Activate, initialise connection

        NET_SV_InitNewClient(client, addr, player_name);

        client->recording_lowres = cl_recording_lowres;
    }

    if (client->connection.state == NET_CONN_STATE_WAITING_ACK)
    {
        // force an acknowledgement
        client->connection.last_send_time = -1;
    }
}

// Parse a game start packet

static void NET_SV_ParseGameStart(net_packet_t *packet, net_client_t *client)
{
    net_gamesettings_t settings;
    net_packet_t *startpacket;
    int i;
    
    if (client != NET_SV_Controller())
    {
        // Only the controller can start a new game

        return;
    }

    if (!NET_ReadSettings(packet, &settings))
    {
        // Malformed packet

        return;
    }

    if (server_state != SERVER_WAITING_START)
    {
        // Can only start a game if we are in the waiting start state.

        return;
    }

    // Assign player numbers

    NET_SV_AssignPlayers();

    // Check if anyone is recording a demo and set lowres_turn if so.

    settings.lowres_turn = false;

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (sv_players[i] != NULL && sv_players[i]->recording_lowres)
        {
            NET_SV_BroadcastMessage("Playing in low resolution turning mode, "
                                    "because player %i is recording a Vanilla demo.\n",
                                    i + 1);
                   
            settings.lowres_turn = true;
        }
    }

    // Send start packets to each connected node

    for (i=0; i<MAXPLAYERS; ++i) 
    {
        if (sv_players[i] == NULL)
            break;

        startpacket = NET_Conn_NewReliable(&sv_players[i]->connection,
                                           NET_PACKET_TYPE_GAMESTART);

        NET_WriteInt8(startpacket, NET_SV_NumClients());
        NET_WriteInt8(startpacket, sv_players[i]->player_number);
        NET_WriteSettings(startpacket, &settings);
    }

    // Change server state

    server_state = SERVER_IN_GAME;
    sv_settings = settings;

    memset(recvwindow, 0, sizeof(recvwindow));
    recvwindow_start = 0;
}

// Send a resend request to a client

static void NET_SV_SendResendRequest(net_client_t *client, int start, int end)
{
    net_packet_t *packet;
    net_client_recv_t *recvobj;
    int i;
    unsigned int nowtime;
    int index;

    //printf("SV: send resend for %i-%i\n", start, end);

    packet = NET_NewPacket(20);

    NET_WriteInt16(packet, NET_PACKET_TYPE_GAMEDATA_RESEND);
    NET_WriteInt32(packet, start);
    NET_WriteInt8(packet, end - start + 1);

    NET_Conn_SendPacket(&client->connection, packet);
    NET_FreePacket(packet);

    // Store the time we send the resend request

    nowtime = I_GetTimeMS();

    for (i=start; i<=end; ++i)
    {
        index = i - recvwindow_start;

        if (index >= BACKUPTICS)
        {
            // Outside the range

            continue;
        }
        
        recvobj = &recvwindow[index][client->player_number];

        recvobj->resend_time = nowtime;
    }
}

// Check for expired resend requests

static void NET_SV_CheckResends(net_client_t *client)
{
    int i;
    int player;
    int resend_start, resend_end;
    unsigned int nowtime;

    nowtime = I_GetTimeMS();

    player = client->player_number;
    resend_start = -1;
    resend_end = -1;

    for (i=0; i<BACKUPTICS; ++i)
    {
        net_client_recv_t *recvobj;
        boolean need_resend;

        recvobj = &recvwindow[i][player];

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

                //printf("SV: resend request timed out: %i-%i\n", resend_start, resend_end);
                NET_SV_SendResendRequest(client, 
                                         recvwindow_start + resend_start,
                                         recvwindow_start + resend_end);

                resend_start = -1;
            }
        }
    }

    if (resend_start >= 0)
    {
        NET_SV_SendResendRequest(client, 
                                 recvwindow_start + resend_start,
                                 recvwindow_start + resend_end);
    }
}

// Process game data from a client

static void NET_SV_ParseGameData(net_packet_t *packet, net_client_t *client)
{
    net_client_recv_t *recvobj;
    unsigned int seq;
    unsigned int num_tics;
    unsigned int nowtime;
    int i;
    int player;
    int resend_start, resend_end;
    int index;

    if (server_state != SERVER_IN_GAME)
    {
        return;
    }

    //if (rand() % 8 == 0)
    //    return;

    player = client->player_number;

    // Read header

    if (!NET_ReadInt8(packet, &seq)
     || !NET_ReadInt8(packet, &num_tics))
    {
        return;
    }

    // Expand 8-bit value to the full sequence number

    seq = NET_SV_ExpandTicNum(seq);

    // Sanity checks

    for (i=0; i<num_tics; ++i)
    {
        net_ticdiff_t diff;
        signed int latency;

        if (!NET_ReadSInt16(packet, &latency)
         || !NET_ReadTiccmdDiff(packet, &diff, sv_settings.lowres_turn))
        {
            return;
        }

        index = seq + i - recvwindow_start;

        if (index < 0 || index >= BACKUPTICS)
        {
            // Not in range of the recv window

            continue;
        }

        recvobj = &recvwindow[index][player];
        recvobj->active = true;
        recvobj->diff = diff;
        recvobj->latency = latency;
    }

    // Has this been received out of sequence, ie. have we not received
    // all tics before the first tic in this packet?  If so, send a 
    // resend request.

    //printf("SV: %p: %i\n", client, seq);

    resend_end = seq - recvwindow_start;

    if (resend_end <= 0)
        return;

    if (resend_end >= BACKUPTICS)
        resend_end = BACKUPTICS - 1;

    nowtime = I_GetTimeMS();

    index = resend_end - 1;
    resend_start = resend_end;
    
    while (index >= 0)
    {
        recvobj = &recvwindow[index][player];

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
        NET_SV_SendResendRequest(client, 
                                 recvwindow_start + resend_start, 
                                 recvwindow_start + resend_end - 1);
    }
}

static void NET_SV_SendTics(net_client_t *client, int start, int end)
{
    net_packet_t *packet;
    int i;

    packet = NET_NewPacket(500);

    NET_WriteInt16(packet, NET_PACKET_TYPE_GAMEDATA);

    // Send the start tic and number of tics

    NET_WriteInt8(packet, start & 0xff);
    NET_WriteInt8(packet, end-start + 1);

    // Write the tics

    for (i=start; i<=end; ++i)
    {
        net_full_ticcmd_t *cmd;

        cmd = &client->sendqueue[i % BACKUPTICS];

        if (i != cmd->seq)
        {
            I_Error("Wanted to send %i, but %i is in its place", i, cmd->seq);
        }

        // Add command
       
        NET_WriteFullTiccmd(packet, cmd, sv_settings.lowres_turn);
    }
    
    // Send packet

    NET_Conn_SendPacket(&client->connection, packet);
    
    NET_FreePacket(packet);
}

// Parse a retransmission request from a client

static void NET_SV_ParseResendRequest(net_packet_t *packet, net_client_t *client)
{
    static unsigned int start;
    static unsigned int num_tics;

    // Read the starting tic and number of tics

    if (!NET_ReadInt32(packet, &start)
     || !NET_ReadInt8(packet, &num_tics))
    {
        return;
    }

    //printf("SV: %p: resend %i-%i\n", client, start, start+num_tics-1);

    // Resend those tics

    NET_SV_SendTics(client, start, start + num_tics - 1);
}


// Process a packet received by the server

static void NET_SV_Packet(net_packet_t *packet, net_addr_t *addr)
{
    net_client_t *client;
    unsigned int packet_type;

    // Find which client this packet came from

    client = NET_SV_FindClient(addr);

    // Read the packet type

    if (!NET_ReadInt16(packet, &packet_type))
    {
        // no packet type

        return;
    }

    if (packet_type == NET_PACKET_TYPE_SYN)
    {
        NET_SV_ParseSYN(packet, client, addr);
    }
    else if (client == NULL)
    {
        // Must come from a valid client; ignore otherwise
    }
    else if (NET_Conn_Packet(&client->connection, packet, &packet_type))
    {
        // Packet was eaten by the common connection code
    }
    else
    { 
        //printf("SV: %s: %i\n", NET_AddrToString(addr), packet_type);

        switch (packet_type)
        {
            case NET_PACKET_TYPE_GAMESTART:
                NET_SV_ParseGameStart(packet, client);
                break;
            case NET_PACKET_TYPE_GAMEDATA:
                NET_SV_ParseGameData(packet, client);
                break;
            case NET_PACKET_TYPE_GAMEDATA_RESEND:
                NET_SV_ParseResendRequest(packet, client);
                break;
            default:
                // unknown packet type

                break;
        }
    }

    // If this address is not in the list of clients, be sure to
    // free it back.

    if (NET_SV_FindClient(addr) == NULL)
    {
        NET_FreeAddress(addr);
    }
}


static void NET_SV_SendWaitingData(net_client_t *client)
{
    net_packet_t *packet;
    int num_clients;
    int i;

    num_clients = NET_SV_NumClients();

    // time to send the client another status packet

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_WAITING_DATA);

    // include the number of clients waiting

    NET_WriteInt8(packet, num_clients);

    // indicate whether the client is the controller

    NET_WriteInt8(packet, NET_SV_Controller() == client);

    // send the index of the client

    NET_WriteInt8(packet, NET_SV_ClientIndex(client));

    // send the address of all players

    for (i=0; i<num_clients; ++i)
    {
        char *addr;

        // name

        NET_WriteString(packet, clients[i].name);

        // address

        addr = NET_AddrToString(clients[i].addr);

        NET_WriteString(packet, addr);
    }
    
    // send packet to client and free

    NET_Conn_SendPacket(&client->connection, packet);
    NET_FreePacket(packet);
}

static void NET_SV_PumpSendQueue(net_client_t *client)
{
    net_full_ticcmd_t cmd;
    int recv_index;
    int i;
    int starttic, endtic;

    recv_index = client->sendseq - recvwindow_start;

    if (recv_index < 0 || recv_index >= BACKUPTICS)
    {
        return;
    }

    // Check if we can generate a new entry for the send queue
    // using the data in recvwindow.

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (sv_players[i] == client)
        {
            // Client does not rely on itself for data

            continue;
        }

        if (sv_players[i] == NULL || !ClientConnected(sv_players[i]))
        {
            continue;
        }

        if (!recvwindow[recv_index][i].active)
        {
            // We do not have this player's ticcmd, so we cannot
            // generate a complete command yet.

            return;
        }
    }

    //printf("SV: have complete ticcmd for %i\n", client->sendseq);

    // We have all data we need to generate a command for this tic.
    
    cmd.seq = client->sendseq;

    // Add ticcmds from all players

    cmd.latency = 0;

    for (i=0; i<MAXPLAYERS; ++i)
    {
        net_client_recv_t *recvobj;

        if (sv_players[i] == client)
        {
            // Not the player we are sending to

            cmd.playeringame[i] = false;
            continue;
        }
        
        if (sv_players[i] == NULL || !recvwindow[recv_index][i].active)
        {
            cmd.playeringame[i] = false;
            continue;
        }

        cmd.playeringame[i] = true;

        recvobj = &recvwindow[recv_index][i];

        cmd.cmds[i] = recvobj->diff;

        if (recvobj->latency > cmd.latency)
            cmd.latency = recvobj->latency;
    }

    //printf("SV: %i: latency %i\n", client->player_number, cmd.latency);

    // Add into the queue

    client->sendqueue[client->sendseq % BACKUPTICS] = cmd;

    // Transmit the new tic to the client

    starttic = client->sendseq - sv_settings.extratics;
    endtic = client->sendseq;

    if (starttic < 0)
        starttic = 0;

    NET_SV_SendTics(client, starttic, endtic);

    ++client->sendseq;
}

// Perform any needed action on a client

static void NET_SV_RunClient(net_client_t *client)
{
    // Run common code

    NET_Conn_Run(&client->connection);
    
    if (client->connection.state == NET_CONN_STATE_DISCONNECTED
     && client->connection.disconnect_reason == NET_DISCONNECT_TIMEOUT)
    {
        NET_SV_BroadcastMessage("Client '%s' timed out and disconnected",
                                client->name);
    }
    
    // Is this client disconnected?

    if (client->connection.state == NET_CONN_STATE_DISCONNECTED)
    {
        // deactivate and free back 

        client->active = false;
        free(client->name);
        NET_FreeAddress(client->addr);
    }
    
    if (!ClientConnected(client))
    {
        // client has not yet finished connecting

        return;
    }

    if (server_state == SERVER_WAITING_START)
    {
        // Waiting for the game to start

        // Send information once every second

        if (client->last_send_time < 0 
         || I_GetTimeMS() - client->last_send_time > 1000)
        {
            NET_SV_SendWaitingData(client);
            client->last_send_time = I_GetTimeMS();
        }
    }

    if (server_state == SERVER_IN_GAME)
    {
        NET_SV_PumpSendQueue(client);
    }
}

// Add a network module to the server context

void NET_SV_AddModule(net_module_t *module)
{
    NET_AddModule(server_context, module);
    module->InitServer();
}

// Initialise server and wait for connections

void NET_SV_Init(void)
{
    int i;

    // initialise send/receive context

    server_context = NET_NewContext();

    // no clients yet
   
    for (i=0; i<MAXNETNODES; ++i) 
    {
        clients[i].active = false;
    }

    server_state = SERVER_WAITING_START;
    server_initialised = true;
}

// Run server code to check for new packets/send packets as the server
// requires

void NET_SV_Run(void)
{
    net_addr_t *addr;
    net_packet_t *packet;
    int i;

    if (!server_initialised)
    {
        return;
    }

    while (NET_RecvPacket(server_context, &addr, &packet)) 
    {
        NET_SV_Packet(packet, addr);
        NET_FreePacket(packet);
    }

    // "Run" any clients that may have things to do, independent of responses
    // to received packets

    for (i=0; i<MAXNETNODES; ++i)
    {
        if (clients[i].active)
        {
            NET_SV_RunClient(&clients[i]);
        }
    }

    if (server_state == SERVER_IN_GAME)
    {
        NET_SV_AdvanceWindow();

        for (i=0; i<MAXPLAYERS; ++i)
        {
            if (sv_players[i] != NULL)
            {
                NET_SV_CheckResends(sv_players[i]);
            }
        }
    }
}

void NET_SV_Shutdown(void)
{
    int i;
    boolean running;
    int start_time;

    if (!server_initialised)
    {
        return;
    }
    
    fprintf(stderr, "SV: Shutting down server...\n");

    // Disconnect all clients
    
    for (i=0; i<MAXNETNODES; ++i)
    {
        if (clients[i].active)
        {
            NET_SV_DisconnectClient(&clients[i]);
        }
    }

    // Wait for all clients to finish disconnecting

    start_time = I_GetTimeMS();
    running = true;

    while (running)
    {
        // Check if any clients are still not finished

        running = false;

        for (i=0; i<MAXNETNODES; ++i)
        {
            if (clients[i].active)
            {
                running = true;
            }
        }

        // Timed out?

        if (I_GetTimeMS() - start_time > 5000)
        {
            running = false;
            fprintf(stderr, "SV: Timed out waiting for clients to disconnect.\n");
        }

        // Run the client code in case this is a loopback client.

        NET_CL_Run();
        NET_SV_Run();

        // Don't hog the CPU

        I_Sleep(1);
    }
}

