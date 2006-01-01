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

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_loop.h"
#include "net_packet.h"
#include "net_server.h"

typedef enum 
{
    // received a syn, sent an ack, waiting for an ack reply

    CLIENT_STATE_WAITING_ACK,
    
    // waiting for a game to start

    CLIENT_STATE_WAITING_START,

    // in game

    CLIENT_STATE_IN_GAME,

    // sent a DISCONNECT packet, waiting for a DISCONNECT_ACK reply

    CLIENT_STATE_DISCONNECTING,

    // client successfully disconnected

    CLIENT_STATE_DISCONNECTED,
} net_clientstate_t;

#define MAX_RETRIES 5

typedef struct 
{
    boolean active;
    net_clientstate_t state;
    net_addr_t *addr;
    int last_send_time;
    int num_retries;
} net_client_t;

static boolean server_initialised = false;
static net_client_t clients[MAXNETNODES];
static net_context_t *server_context;

static boolean ClientConnected(net_client_t *client)
{
    // Check that the client is properly connected: ie. not in the 
    // process of connecting or disconnecting

    return clients->active
        && clients->state != CLIENT_STATE_DISCONNECTING
        && clients->state != CLIENT_STATE_DISCONNECTED
        && clients->state != CLIENT_STATE_WAITING_ACK;
}

// returns the number of clients connected

static int ServerNumClients(void)
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

// returns a pointer to the client which controls the server

static net_client_t *ServerController(void)
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

static net_client_t *ServerFindClient(net_addr_t *addr)
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

// parse a SYN from a client(initiating a connection)

static void ServerParseSYN(net_packet_t *packet, 
                           net_client_t *client,
                           net_addr_t *addr)
{
    unsigned int magic;
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

    // received a valid SYN

    // allocate a client slot if there isn't one already

    if (client == NULL)
    {
        // find a slot, or return if none found

        for (i=0; i<MAXNETNODES; ++i)
        {
            if (!clients[i].active)
            {
                clients[i].active = true;
                clients[i].addr = addr;
                clients[i].state = CLIENT_STATE_WAITING_ACK;
                clients[i].num_retries = 0;
                client = &clients[i];
                break;
            }
        }

        if (client == NULL)
        {
            return;
        }
    }

    if (client->state == CLIENT_STATE_WAITING_ACK)
    {
        // force an acknowledgement

        client->last_send_time = -1;
    }
}

// parse an ACK packet from a client

static void ServerParseACK(net_packet_t *packet, net_client_t *client)
{
    if (client == NULL)
    {
        return;
    }

    if (client->state == CLIENT_STATE_WAITING_ACK)
    {
        // now waiting for the game to start

        client->state = CLIENT_STATE_WAITING_START;

        // force a waiting data packet to be sent immediately

        client->last_send_time = -1;
    }
}

static void ServerParseDisconnect(net_packet_t *packet, net_client_t *client)
{
    net_packet_t *reply;

    // sanity check

    if (client == NULL)
    {
        return;
    }

    // This client wants to disconnect from the server.
    // Send a DISCONNECT_ACK reply.
    
    reply = NET_NewPacket(10);
    NET_WriteInt16(reply, NET_PACKET_TYPE_DISCONNECT_ACK);
    NET_SendPacket(client->addr, reply);
    NET_FreePacket(reply);

    client->last_send_time = I_GetTimeMS();
    
    // Do not set to inactive immediately.  Instead, set to the 
    // DISCONNECTED state.  This is in case our acknowledgement is
    // not received and another must be sent.
    //
    // After a few seconds, the client will get properly removed
    // and cleaned up from the clients list.

    client->state = CLIENT_STATE_DISCONNECTED;

    printf("client %i disconnected\n", client-clients);
}

// Process a packet received by the server

static void ServerPacket(net_packet_t *packet, net_addr_t *addr)
{
    net_client_t *client;
    unsigned int packet_type;

    // Find which client this packet came from

    client = ServerFindClient(addr);

    // Read the packet type

    if (!NET_ReadInt16(packet, &packet_type))
    {
        // no packet type

        return;
    }

    switch (packet_type)
    {
        case NET_PACKET_TYPE_SYN:
            ServerParseSYN(packet, client, addr);
            break;
        case NET_PACKET_TYPE_ACK:
            ServerParseACK(packet, client);
            break;
        case NET_PACKET_TYPE_GAMESTART:
            break;
        case NET_PACKET_TYPE_GAMEDATA:
            break;
        case NET_PACKET_TYPE_DISCONNECT:
            ServerParseDisconnect(packet, client);
            break;
        default:
            // unknown packet type

            break;
    }

    // If this address is not in the list of clients, be sure to
    // free it back.

    if (ServerFindClient(addr) == NULL)
    {
        NET_FreeAddress(addr);
    }
}


static void ServerSendWaitingData(net_client_t *client)
{
    net_packet_t *packet;

    // time to send the client another status packet

    packet = NET_NewPacket(10);
    NET_WriteInt16(packet, NET_PACKET_TYPE_WAITING_DATA);

    // include the number of clients waiting

    NET_WriteInt8(packet, ServerNumClients());

    // indicate whether the client is the controller

    NET_WriteInt8(packet, ServerController() == client);
    
    // send packet to client and free

    NET_SendPacket(client->addr, packet);
    NET_FreePacket(packet);
    
    // update time

    client->last_send_time = I_GetTimeMS();
}

// Perform any needed action on a client

static void ServerRunClient(net_client_t *client)
{
    net_packet_t *packet;

    if (client->state == CLIENT_STATE_WAITING_ACK)
    {
        if (client->last_send_time < 0
         || I_GetTime() - client->last_send_time > 35)
        {
            // it has been a second since the last ACK was sent, and 
            // still no reply.

            if (client->num_retries < MAX_RETRIES)
            {
                // send another ACK

                packet = NET_NewPacket(10);
                NET_WriteInt16(packet, NET_PACKET_TYPE_ACK);
                NET_SendPacket(client->addr, packet);
                NET_FreePacket(packet);
                client->last_send_time = I_GetTime();

                ++client->num_retries;
            }
            else 
            {
                // no more retries allowed.

                client->active = false;
                NET_FreeAddress(client->addr);
            }
        }
    }

    // waiting for the game to start

    if (client->state == CLIENT_STATE_WAITING_START)
    {
        // Send information once every second

        if (client->last_send_time < 0 
         || I_GetTimeMS() - client->last_send_time > 1000)
        {
            ServerSendWaitingData(client);
        }
    }

    // Client has disconnected.  
    //
    // See ServerParseDisconnect() above.

    if (client->state == CLIENT_STATE_DISCONNECTED)
    {
        // Remove from the list after five seconds

        if (I_GetTimeMS() - client->last_send_time > 5000)
        {
            client->active = false;
            NET_FreeAddress(client->addr);
        }
    }
}

// Initialise server and wait for connections

void NET_ServerInit(void)
{
    int i;

    // initialise send/receive context, with loopback send/recv

    server_context = NET_NewContext();
    NET_AddModule(server_context, &net_loop_server_module);
    net_loop_server_module.InitServer();

    // no clients yet
   
    for (i=0; i<MAXNETNODES; ++i) 
    {
        clients[i].active = false;
    }

    server_initialised = true;
}

// Run server code to check for new packets/send packets as the server
// requires

void NET_ServerRun(void)
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
        ServerPacket(packet, addr);
    }

    // "Run" any clients that may have things to do, independent of responses
    // to received packets

    for (i=0; i<MAXNETNODES; ++i)
    {
        if (clients[i].active)
        {
            ServerRunClient(&clients[i]);
        }
    }
}

