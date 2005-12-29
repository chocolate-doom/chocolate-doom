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
} net_clientstate_t;

static net_clientstate_t client_state;
static net_addr_t *server_addr;
static net_context_t *client_context;
static int last_send_time;

// parse a received packet

static void ClientParsePacket(net_packet_t *packet)
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

            if (client_state == CLIENT_STATE_CONNECTING)
            {
                client_state = CLIENT_STATE_WAITING_START;
            }
            break;

        case NET_PACKET_TYPE_GAMESTART:
            break;

        case NET_PACKET_TYPE_GAMEDATA:
            break;
        default:
            break;
    }
}

// called when we are in the "connecting" state

static void ClientConnecting(void)
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

// "Run" the client code: check for new packets, send packets as
// needed

void NET_ClientRun(void)
{
    net_addr_t *addr;
    net_packet_t *packet;
    
    while (NET_RecvPacket(client_context, &addr, &packet))
    {
        // only accept packets from the server

        if (addr == server_addr)
        {
            ClientParsePacket(packet);
        }

        NET_FreePacket(packet);
    }

    // send packets as needed

    switch (client_state)
    {
        case CLIENT_STATE_CONNECTING:
            ClientConnecting();
            break;
        case CLIENT_STATE_WAITING_START:
            break;
        case CLIENT_STATE_IN_GAME:
            break;
    }
}

// connect to a server

boolean NET_ClientConnect(net_addr_t *addr)
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

        NET_ClientRun();
        
        // run the server, just incase we are doing a loopback
        // connect

        NET_ServerRun();
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


