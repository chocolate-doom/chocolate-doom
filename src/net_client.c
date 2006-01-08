// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_client.c 268 2006-01-08 04:52:26Z fraggle $
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

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "net_client.h"
#include "net_common.h"
#include "net_defs.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_server.h"

typedef enum
{
    // waiting for the game to start

    CLIENT_STATE_WAITING_START,

    // in game

    CLIENT_STATE_IN_GAME,

} net_clientstate_t;

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

// Nmaes of all players

char net_player_addresses[MAXPLAYERS][MAXPLAYERNAME];
char net_player_names[MAXPLAYERS][MAXPLAYERNAME];

// Waiting for the game to start?

boolean net_waiting_for_start = false;

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

// data received while we are waiting for the game to start

static void NET_CL_ParseWaitingData(net_packet_t *packet)
{
    unsigned int num_players;
    unsigned int is_controller;
    char *player_names[MAXPLAYERS];
    char *player_addr[MAXPLAYERS];
    int i;

    if (!NET_ReadInt8(packet, &num_players)
     || !NET_ReadInt8(packet, &is_controller))
    {
        // invalid packet

        return;
    }

    if (num_players > MAXPLAYERS)
    {
        // Invalid number of players

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

    for (i=0; i<num_players; ++i)
    {
        strncpy(net_player_names[i], player_names[i], MAXPLAYERNAME);
        net_player_names[i][MAXPLAYERNAME-1] = '\0';
        strncpy(net_player_addresses[i], player_addr[i], MAXPLAYERNAME);
        net_player_addresses[i][MAXPLAYERNAME-1] = '\0';
    }
}

// parse a received packet

static void NET_CL_ParsePacket(net_packet_t *packet)
{
    unsigned int packet_type;

    if (!NET_ReadInt16(packet, &packet_type))
    {
        return;
    }

    if (NET_Conn_Packet(&client_connection, packet, packet_type))
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
                break;

            case NET_PACKET_TYPE_GAMEDATA:
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
        // disconnected from server

        NET_CL_Shutdown();
    }
    
    net_waiting_for_start = client_connection.state == NET_CONN_STATE_CONNECTED
                         && client_state == CLIENT_STATE_WAITING_START;
}

static void NET_CL_SendSYN(void)
{
    net_packet_t *packet;

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_SYN);
    NET_WriteInt32(packet, NET_MAGIC_NUMBER);
    NET_WriteInt16(packet, gamemode);
    NET_WriteInt16(packet, gamemission);
    NET_Conn_SendPacket(&client_connection, packet);
    NET_FreePacket(packet);
}

// connect to a server

boolean NET_CL_Connect(net_addr_t *addr)
{
    int start_time;
    int last_send_time;

    server_addr = addr;

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

