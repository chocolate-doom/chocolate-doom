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

#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
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

    // time query variables

    int last_time_req_time;
    int time_req_seq;
    signed int time_offset;

    // send queue: items to send to the client
    // this is a circular buffer

    int sendseq;
    net_full_ticcmd_t sendqueue[BACKUPTICS];

} net_client_t;

// structure used for the recv window

typedef struct 
{
    boolean active;
    unsigned int time;
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

static unsigned int NET_SV_ExpandTicNum(int i)
{
    int l, h;
    unsigned int result;

    h = recvwindow_start & ~0xff;
    l = recvwindow_start & 0xff;
    
    result = h | i;

    if (i - l > 0x80)
        result -= 0x100;
    else if (l - i > 0x80)
        result += 0x100;

    return result;
}

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
        memcpy(recvwindow, recvwindow + 1, sizeof(*recvwindow) * (BACKUPTICS - 1));
        memset(&recvwindow[BACKUPTICS-1], 0, sizeof(*recvwindow));
        ++recvwindow_start;
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
    client->last_time_req_time = -1;
    client->time_req_seq = 0;
    client->time_offset = 0;

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
     || !NET_ReadInt16(packet, &cl_gamemission))
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

    // Change server state

    server_state = SERVER_IN_GAME;
    sv_settings = settings;

    // Send start packets to each connected node

    NET_SV_AssignPlayers();

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

    memset(recvwindow, 0, sizeof(recvwindow));
    recvwindow_start = 0;
}

static void NET_SV_ParseTimeResponse(net_packet_t *packet, net_client_t *client)
{
    unsigned int seq;
    unsigned int remote_time;
    unsigned int rtt;
    unsigned int nowtime;
    signed int time_offset;

    if (!NET_ReadInt32(packet, &seq)
     || !NET_ReadInt32(packet, &remote_time))
    {
	return;
    }

    if (seq != client->time_req_seq)
    {
	// Not the time response we are expecting

	return;
    }

    // Calculate the round trip time

    nowtime = I_GetTimeMS();
    rtt = nowtime - client->last_time_req_time;

    // Adjust the remote time based on the round trip time

    remote_time += rtt / 2;

    // Calculate the offset to our own time

    time_offset = remote_time - nowtime;

    // Update the time offset

    if (client->time_req_seq == 1)
    {
	// This is the first reply, so this is the only sample we have
	// so far
	
	client->time_offset = time_offset;
    }
    else
    {
	// Apply a low level filter to the time offset adjustments
	
	client->time_offset = ((client->time_offset * 3) / 4)
	                    + (time_offset / 4);
    }

    printf("client %p time offset: %i(%i)->%i\n", client, time_offset, rtt, client->time_offset);
}

// Process game data from a client

static void NET_SV_ParseGameData(net_packet_t *packet, net_client_t *client)
{
    net_client_recv_t *recvobj;
    unsigned int seq;
    unsigned int num_tics;
    int i;
    int player;

    if (server_state != SERVER_IN_GAME)
    {
        return;
    }
    
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
        int index;

        if (!NET_ReadTiccmdDiff(packet, &diff, false))
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
    }
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
	    case NET_PACKET_TYPE_TIME_RESP:
		NET_SV_ParseTimeResponse(packet, client);
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

static void NET_SV_SendTimeRequest(net_client_t *client)
{
    net_packet_t *packet;

    ++client->time_req_seq;
    
    // Transmit the request packet

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_TIME_REQ);
    NET_WriteInt32(packet, client->time_req_seq);
    NET_Conn_SendPacket(&client->connection, packet);
    NET_FreePacket(packet);

    // Save the time we send the request

    client->last_time_req_time = I_GetTimeMS();
}

static void NET_SV_PumpSendQueue(net_client_t *client)
{
    net_full_ticcmd_t cmd;
    int recv_index;
    int i;

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

    //printf("have complete ticcmd for %i\n", client->sendseq);

    // We have all data we need to generate a command for this tic.
    
    cmd.seq = client->sendseq;

    // Add ticcmds from all players

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (sv_players[i] == NULL || !recvwindow[recv_index][i].active)
        {
            cmd.playeringame[i] = false;
            continue;
        }

        cmd.playeringame[i] = true;
        cmd.cmds[i] = recvwindow[recv_index][i].diff;
    }

    // Add into the queue

    client->sendqueue[client->sendseq % BACKUPTICS] = cmd;

    ++client->sendseq;
}

// Perform any needed action on a client

static void NET_SV_RunClient(net_client_t *client)
{
    // Run common code

    NET_Conn_Run(&client->connection);
    
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

    if (client->last_time_req_time < 0)
    {
	client->last_time_req_time = I_GetTimeMS() - 5000;
    }

    if (I_GetTimeMS() - client->last_time_req_time > 10000)
    {
	// Query the clients' times once every ten seconds.
	
	NET_SV_SendTimeRequest(client);
    }

    if (server_state == SERVER_IN_GAME)
    {
        NET_SV_PumpSendQueue(client);
    }
}

// Initialise server and wait for connections

void NET_SV_Init(void)
{
    int i;

    // initialise send/receive context, with loopback send/recv

    server_context = NET_NewContext();
    NET_AddModule(server_context, &net_loop_server_module);
    net_loop_server_module.InitServer();
    NET_AddModule(server_context, &net_sdl_module);
    net_sdl_module.InitServer();

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

        I_Sleep(10);
    }
}

