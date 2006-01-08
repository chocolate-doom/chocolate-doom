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
// Revision 1.2  2006/01/08 02:53:05  fraggle
// Send keepalives if the connection is not doing anything else.
// Send all packets using a new NET_Conn_SendPacket to support this.
//
// Revision 1.1  2006/01/08 00:10:48  fraggle
// Move common connection code into net_common.c, shared by server
// and client code.
//
//
// Common code shared between the client and server
//

#include "doomdef.h"
#include "i_system.h"

#include "net_common.h"
#include "net_io.h"
#include "net_packet.h"

// connections time out after 10 seconds

#define CONNECTION_TIMEOUT_LEN 10

// maximum time between sending packets

#define KEEPALIVE_PERIOD 1

// Initialise as a client connection

void NET_Conn_InitClient(net_connection_t *conn, net_addr_t *addr)
{
    conn->state = NET_CONN_STATE_CONNECTING;
    conn->last_send_time = -1;
    conn->num_retries = 0;
    conn->addr = addr;
}

// Initialise as a server connection

void NET_Conn_InitServer(net_connection_t *conn, net_addr_t *addr)
{
    conn->state = NET_CONN_STATE_WAITING_ACK;
    conn->last_send_time = -1;
    conn->num_retries = 0;
    conn->addr = addr;
}

// Send a packet to a connection
// All packets should be sent through this interface, as it maintains the
// keepalive_send_time counter.

void NET_Conn_SendPacket(net_connection_t *conn, net_packet_t *packet)
{
    conn->keepalive_send_time = I_GetTimeMS();
    NET_SendPacket(conn->addr, packet);
}

// parse an ACK packet from a client

static void NET_Conn_ParseACK(net_connection_t *conn, net_packet_t *packet)
{
    net_packet_t *reply;

    if (conn->state == NET_CONN_STATE_CONNECTING)
    {
        // We are a client

        // received a response from the server to our SYN

        conn->state = NET_CONN_STATE_CONNECTED;

        // We must send an ACK reply to the server's ACK

        reply = NET_NewPacket(10);
        NET_WriteInt16(reply, NET_PACKET_TYPE_ACK);
        NET_Conn_SendPacket(conn, reply);
        NET_FreePacket(reply);
    }
    
    if (conn->state == NET_CONN_STATE_WAITING_ACK)
    {
        // We are a server

        // Client is connected
        
        conn->state = NET_CONN_STATE_CONNECTED;
    }
}

static void NET_Conn_ParseDisconnect(net_connection_t *conn, net_packet_t *packet)
{
    net_packet_t *reply;

    // Other end wants to disconnect
    // Send a DISCONNECT_ACK reply.
    
    reply = NET_NewPacket(10);
    NET_WriteInt16(reply, NET_PACKET_TYPE_DISCONNECT_ACK);
    NET_Conn_SendPacket(conn, reply);
    NET_FreePacket(reply);

    conn->last_send_time = I_GetTimeMS();
    
    conn->state = NET_CONN_STATE_DISCONNECTED_SLEEP;
}

// Parse a DISCONNECT_ACK packet

static void NET_Conn_ParseDisconnectACK(net_connection_t *conn,
                                        net_packet_t *packet)
{

    if (conn->state == NET_CONN_STATE_DISCONNECTING)
    {
        // We have received an acknowledgement to our disconnect
        // request. We have been disconnected successfully.
        
        conn->state = NET_CONN_STATE_DISCONNECTED;
        conn->last_send_time = -1;
    }
}

// Process a packet received by the server
//
// Returns true if eaten by common code

boolean NET_Conn_Packet(net_connection_t *conn, net_packet_t *packet, 
                        int packet_type)
{
    conn->keepalive_recv_time = I_GetTimeMS();
    
    switch (packet_type)
    {
        case NET_PACKET_TYPE_ACK:
            NET_Conn_ParseACK(conn, packet);
            break;
        case NET_PACKET_TYPE_DISCONNECT:
            NET_Conn_ParseDisconnect(conn, packet);
            break;
        case NET_PACKET_TYPE_DISCONNECT_ACK:
            NET_Conn_ParseDisconnectACK(conn, packet);
            break;
        case NET_PACKET_TYPE_KEEPALIVE:
            // No special action needed.
            break;
        default:
            // Not a common packet

            return false;
    }

    // We found a packet that we found interesting, and ate it.

    return true;
}

void NET_Conn_Disconnect(net_connection_t *conn)
{
    if (conn->state != NET_CONN_STATE_DISCONNECTED
     && conn->state != NET_CONN_STATE_DISCONNECTING
     && conn->state != NET_CONN_STATE_DISCONNECTED_SLEEP)
    {
        conn->state = NET_CONN_STATE_DISCONNECTING;
        conn->last_send_time = -1;
        conn->num_retries = 0;
    }
}

void NET_Conn_Run(net_connection_t *conn)
{
    net_packet_t *packet;
    unsigned int nowtime;

    nowtime = I_GetTimeMS();

    if (conn->state == NET_CONN_STATE_CONNECTED)
    {
        // Check the keepalive counters

        if (nowtime - conn->keepalive_recv_time > CONNECTION_TIMEOUT_LEN * 1000)
        {
            // Haven't received any packets from the other end in a long
            // time.  Assume disconnected.

            conn->state = NET_CONN_STATE_DISCONNECTED;
        }
        
        if (nowtime - conn->keepalive_send_time > KEEPALIVE_PERIOD * 1000)
        {
            // We have not sent anything in a long time.
            // Send a keepalive.

            packet = NET_NewPacket(10);
            NET_WriteInt16(packet, NET_PACKET_TYPE_KEEPALIVE);
            NET_Conn_SendPacket(conn, packet);
            NET_FreePacket(packet);
        }
    }
    else if (conn->state == NET_CONN_STATE_CONNECTING)
    {
        if (conn->last_send_time < 0
         || nowtime - conn->last_send_time > 1000)
        {
            // It has been a second since the last SYN was sent, and no
            // reply.
            
            if (conn->num_retries < MAX_RETRIES)
            {
                // send another SYN
                    
                packet = NET_NewPacket(10);
                NET_WriteInt16(packet, NET_PACKET_TYPE_SYN);
                NET_WriteInt32(packet, NET_MAGIC_NUMBER);
                NET_Conn_SendPacket(conn, packet);
                NET_FreePacket(packet);
                conn->last_send_time = nowtime;

                ++conn->num_retries;
            }
            else
            {
                conn->state = NET_CONN_STATE_DISCONNECTED;
            }
        }
    }
    else if (conn->state == NET_CONN_STATE_WAITING_ACK)
    {
        if (conn->last_send_time < 0
         || nowtime - conn->last_send_time > 1000)
        {
            // it has been a second since the last ACK was sent, and 
            // still no reply.

            if (conn->num_retries < MAX_RETRIES)
            {
                // send another ACK

                packet = NET_NewPacket(10);
                NET_WriteInt16(packet, NET_PACKET_TYPE_ACK);
                NET_Conn_SendPacket(conn, packet);
                NET_FreePacket(packet);
                conn->last_send_time = nowtime;

                ++conn->num_retries;
            }
            else 
            {
                // no more retries allowed.

                conn->state = NET_CONN_STATE_DISCONNECTED;
            }
        }
    }
    else if (conn->state == NET_CONN_STATE_DISCONNECTING)
    {
        // Waiting for a reply to our DISCONNECT request.

        if (conn->last_send_time < 0
         || nowtime - conn->last_send_time > 1000)
        {
            // it has been a second since the last disconnect packet 
            // was sent, and still no reply.

            if (conn->num_retries < MAX_RETRIES)
            {
                // send another disconnect

                packet = NET_NewPacket(10);
                NET_WriteInt16(packet, NET_PACKET_TYPE_DISCONNECT);
                NET_Conn_SendPacket(conn, packet);
                NET_FreePacket(packet);
                conn->last_send_time = nowtime;

                ++conn->num_retries;
            }
            else 
            {
                // No more retries allowed.
                // Force disconnect.

                conn->state = NET_CONN_STATE_DISCONNECTED;
            }
        }
    }
    else if (conn->state == NET_CONN_STATE_DISCONNECTED_SLEEP)
    {
        // We are disconnected, waiting in case we need to send
        // a DISCONNECT_ACK to the server again.

        if (nowtime - conn->last_send_time > 5000)
        {
            // Idle for 5 seconds, switch state

            conn->state = NET_CONN_STATE_DISCONNECTED;
        }
    }
}



