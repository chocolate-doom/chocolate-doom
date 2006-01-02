// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_client.c 242 2006-01-02 00:54:17Z fraggle $
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
#include "net_defs.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_server.h"

typedef enum
{
    // sent a syn, not received an ack yet

    CLIENT_STATE_CONNECTING,

    // waiting for the game to start

    CLIENT_STATE_WAITING_START,

    // in game

    CLIENT_STATE_IN_GAME,

    // in disconnect state: sent DISCONNECT, waiting for DISCONNECT_ACK reply
    
    CLIENT_STATE_DISCONNECTING,

    // successfully disconnected

    CLIENT_STATE_DISCONNECTED,
} net_clientstate_t;

static boolean client_initialised = false;

static net_clientstate_t client_state;
static net_addr_t *server_addr;
static net_context_t *client_context;
static int last_send_time;

// data received while we are waiting for the game to start

static void NET_CL_ParseWaitingData(net_packet_t *packet)
{
    unsigned int num_players;
    unsigned int is_controller;

    if (!NET_ReadInt8(packet, &num_players)
     || !NET_ReadInt8(packet, &is_controller))
    {
        // invalid packet

        return;
    }

    net_clients_in_game = num_players;
    net_client_controller = is_controller != 0;
}

// Received an ACK

static void NET_CL_ParseACK(net_packet_t *packet)
{
    net_packet_t *reply;

    // send an ACK back

    reply = NET_NewPacket(10);
    NET_WriteInt16(reply, NET_PACKET_TYPE_ACK);
    NET_SendPacket(server_addr, reply);
    NET_FreePacket(reply);

    // set the client state if we havent already
 
    if (client_state == CLIENT_STATE_CONNECTING)
    {
        client_state = CLIENT_STATE_WAITING_START;
    }
}

// parse a DISCONNECT packet

static void NET_CL_ParseDisconnect(net_packet_t *packet)
{
    net_packet_t *reply;

    // construct a DISCONNECT_ACK reply packet

    reply = NET_NewPacket(10);
    NET_WriteInt16(reply, NET_PACKET_TYPE_DISCONNECT_ACK);

    // send the reply several times, in case of packet loss

    NET_SendPacket(server_addr, reply);
    NET_SendPacket(server_addr, reply);
    NET_SendPacket(server_addr, reply);
    NET_FreePacket(reply);

    client_state = CLIENT_STATE_DISCONNECTED;

    //I_Error("Disconnected from server.\n");
    fprintf(stderr, "Disconnected from server.\n");

    // Now what?
}

// parse a DISCONNECT_ACK packet

static void NET_CL_ParseDisconnectACK(net_packet_t *packet)
{
    if (client_state == CLIENT_STATE_DISCONNECTING)
    {
        // successfully disconnected from the server.

        client_state = CLIENT_STATE_DISCONNECTED;

        // now what?
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

    switch (packet_type)
    {
        case NET_PACKET_TYPE_ACK:

            // received an acknowledgement to the SYN we sent

            NET_CL_ParseACK(packet);
            break;

        case NET_PACKET_TYPE_WAITING_DATA:

            NET_CL_ParseWaitingData(packet);
            break;

        case NET_PACKET_TYPE_GAMESTART:
            break;

        case NET_PACKET_TYPE_GAMEDATA:
            break;

        case NET_PACKET_TYPE_DISCONNECT:
            NET_CL_ParseDisconnect(packet);
            break;

        case NET_PACKET_TYPE_DISCONNECT_ACK:
            NET_CL_ParseDisconnectACK(packet);
            break;

        default:
            break;
    }
}

// called when we are in the "connecting" state

static void NET_CL_Connecting(void)
{
    net_packet_t *packet;

    // send a SYN packet every second

    if (last_send_time < 0 || I_GetTimeMS() - last_send_time > 1000)
    {
        // construct a SYN packet

        packet = NET_NewPacket(10);

        // packet type
     
        NET_WriteInt16(packet, NET_PACKET_TYPE_SYN);

        // magic number

        NET_WriteInt32(packet, NET_MAGIC_NUMBER);

        // send to the server

        NET_SendPacket(server_addr, packet);

        NET_FreePacket(packet);

        last_send_time = I_GetTimeMS();
    }
}

// Called when we are in the "disconnecting" state, disconnecting from
// the server.

static void NET_CL_Disconnecting(void)
{
    net_packet_t *packet;

    // send a DISCONNECT packet every second

    if (last_send_time < 0 || I_GetTimeMS() - last_send_time > 1000)
    {
        // construct packet

        packet = NET_NewPacket(10);

        // packet type
     
        NET_WriteInt16(packet, NET_PACKET_TYPE_DISCONNECT);

        // send to the server

        NET_SendPacket(server_addr, packet);

        NET_FreePacket(packet);

        last_send_time = I_GetTimeMS();
    }
}

// "Run" the client code: check for new packets, send packets as
// needed

void NET_CL_Run(void)
{
    net_addr_t *addr;
    net_packet_t *packet;
    
    if (!client_initialised)
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

        NET_FreePacket(packet);
    }

    // send packets as needed

    switch (client_state)
    {
        case CLIENT_STATE_CONNECTING:
            NET_CL_Connecting();
            break;
        case CLIENT_STATE_DISCONNECTING:
            NET_CL_Disconnecting();
            break;
        default:
            break;
    }
}

// connect to a server

boolean NET_CL_Connect(net_addr_t *addr)
{
    int start_time;

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

    client_initialised = true;

    // try to connect
 
    client_state = CLIENT_STATE_CONNECTING;
    last_send_time = -1;

    start_time = I_GetTimeMS();

    while (client_state == CLIENT_STATE_CONNECTING)
    {
        // time out after 5 seconds 

        if (I_GetTime() - start_time > 5000)
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

    if (client_state != CLIENT_STATE_CONNECTING)
    {
        // connected ok!

        return true;
    }
    else
    {
        // failed to connect
        
        return false;
    }
}

// disconnect from the server

void NET_CL_Disconnect(void)
{
    int start_time;

    if (!client_initialised)
    {
        return;
    }
    
    // set the client into the DISCONNECTING state

    if (client_state != CLIENT_STATE_DISCONNECTED)
    {
        client_state = CLIENT_STATE_DISCONNECTING;
        last_send_time = -1;
    }

    start_time = I_GetTimeMS();

    while (client_state != CLIENT_STATE_DISCONNECTED)
    {
        if (I_GetTimeMS() - start_time > 5000)
        {
            // time out after 5 seconds
            
            client_state = CLIENT_STATE_DISCONNECTED;

            fprintf(stderr, "NET_CL_Disconnect: Timeout while disconnecting from server\n");
            break;
        }

        NET_CL_Run();
        NET_SV_Run();

        I_Sleep(10);
    }

    // Finished sending disconnect packets, etc.

    // Shut down network module, etc.  To do.

    NET_FreeAddress(server_addr);

    client_initialised = false;
}

