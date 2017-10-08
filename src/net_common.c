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
// Common code shared between the client and server
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "d_mode.h"
#include "i_system.h"
#include "i_timer.h"

#include "net_common.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_structrw.h"

// connections time out after 30 seconds

#define CONNECTION_TIMEOUT_LEN 30

// maximum time between sending packets

#define KEEPALIVE_PERIOD 1

// reliable packet that is guaranteed to reach its destination

struct net_reliable_packet_s
{
    net_packet_t *packet;
    int last_send_time;
    int seq;
    net_reliable_packet_t *next;
};

static void NET_Conn_Init(net_connection_t *conn, net_addr_t *addr,
                          net_protocol_t protocol)
{
    conn->last_send_time = -1;
    conn->num_retries = 0;
    conn->addr = addr;
    conn->protocol = protocol;
    conn->reliable_packets = NULL;
    conn->reliable_send_seq = 0;
    conn->reliable_recv_seq = 0;
}

// Initialize as a client connection

void NET_Conn_InitClient(net_connection_t *conn, net_addr_t *addr,
                         net_protocol_t protocol)
{
    NET_Conn_Init(conn, addr, protocol);
    conn->state = NET_CONN_STATE_CONNECTING;
}

// Initialize as a server connection

void NET_Conn_InitServer(net_connection_t *conn, net_addr_t *addr,
                         net_protocol_t protocol)
{
    NET_Conn_Init(conn, addr, protocol);
    conn->state = NET_CONN_STATE_CONNECTED;
}

// Send a packet to a connection
// All packets should be sent through this interface, as it maintains the
// keepalive_send_time counter.

void NET_Conn_SendPacket(net_connection_t *conn, net_packet_t *packet)
{
    conn->keepalive_send_time = I_GetTimeMS();
    NET_SendPacket(conn->addr, packet);
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
    conn->disconnect_reason = NET_DISCONNECT_REMOTE;
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
        conn->disconnect_reason = NET_DISCONNECT_LOCAL;
        conn->last_send_time = -1;
    }
}

static void NET_Conn_ParseReject(net_connection_t *conn, net_packet_t *packet)
{
    char *msg;

    msg = NET_ReadSafeString(packet);

    if (msg == NULL)
    {
        return;
    }
    
    if (conn->state == NET_CONN_STATE_CONNECTING)
    {
        // rejected by server

        conn->state = NET_CONN_STATE_DISCONNECTED;
        conn->disconnect_reason = NET_DISCONNECT_REMOTE;

        printf("Rejected by server: %s\n", msg);
    }
}

static void NET_Conn_ParseReliableACK(net_connection_t *conn, net_packet_t *packet)
{
    unsigned int seq;

    if (!NET_ReadInt8(packet, &seq))
    {
        return;
    }

    if (conn->reliable_packets == NULL)
    {
        return;
    }
            
    // Is this an acknowledgement for the first packet in the list?

    if (seq == (unsigned int)((conn->reliable_packets->seq + 1) & 0xff))
    {
        net_reliable_packet_t *rp;

        // Discard it, then.
        // Unlink from the list.

        rp = conn->reliable_packets;
        conn->reliable_packets = rp->next;
        
        NET_FreePacket(rp->packet);
        free(rp);
    }
}

// Process the header of a reliable packet
//
// Returns true if the packet should be discarded (incorrect sequence)

static boolean NET_Conn_ReliablePacket(net_connection_t *conn, 
                                       net_packet_t *packet)
{
    unsigned int seq;
    net_packet_t *reply;
    boolean result;

    // Read the sequence number

    if (!NET_ReadInt8(packet, &seq))
    {
        return true;
    }

    if (seq != (unsigned int)(conn->reliable_recv_seq & 0xff))
    {
        // This is not the next expected packet in the sequence!
        //
        // Discard the packet.  If we were smart, we would use a proper
        // sliding window protocol to do this, but I'm lazy.

        result = true;
    }
    else
    {
        // Now we can receive the next packet in the sequence.

        conn->reliable_recv_seq = (conn->reliable_recv_seq + 1) & 0xff;
    
        result = false;
    }

    // Send an acknowledgement

    // Note: this is braindead.  It would be much more sensible to 
    // include this in the next packet, rather than the overhead of
    // sending a complete packet just for one byte of information.

    reply = NET_NewPacket(10);

    NET_WriteInt16(reply, NET_PACKET_TYPE_RELIABLE_ACK);
    NET_WriteInt8(reply, conn->reliable_recv_seq & 0xff);

    NET_Conn_SendPacket(conn, reply);

    NET_FreePacket(reply);

    return result;
}

// Process a packet received by the server
//
// Returns true if eaten by common code

boolean NET_Conn_Packet(net_connection_t *conn, net_packet_t *packet, 
                        unsigned int *packet_type)
{
    conn->keepalive_recv_time = I_GetTimeMS();

    // Is this a reliable packet?

    if (*packet_type & NET_RELIABLE_PACKET)
    {
        if (NET_Conn_ReliablePacket(conn, packet)) 
        {
            // Invalid packet: eat it.

            return true;
        }

        // Remove the reliable bit

        *packet_type &= ~NET_RELIABLE_PACKET;
    }
    
    switch (*packet_type)
    {
        case NET_PACKET_TYPE_DISCONNECT:
            NET_Conn_ParseDisconnect(conn, packet);
            break;
        case NET_PACKET_TYPE_DISCONNECT_ACK:
            NET_Conn_ParseDisconnectACK(conn, packet);
            break;
        case NET_PACKET_TYPE_KEEPALIVE:
            // No special action needed.
            break;
        case NET_PACKET_TYPE_REJECTED:
            NET_Conn_ParseReject(conn, packet);
            break;
        case NET_PACKET_TYPE_RELIABLE_ACK:
            NET_Conn_ParseReliableACK(conn, packet);
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
        conn->disconnect_reason = NET_DISCONNECT_LOCAL;
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
            conn->disconnect_reason = NET_DISCONNECT_TIMEOUT;
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

        // Check the reliable packet list. Has the first packet in the
        // list timed out?
        //
        // NB.  This is braindead, we have a fixed time of one second.

        if (conn->reliable_packets != NULL
         && (conn->reliable_packets->last_send_time < 0
          || nowtime - conn->reliable_packets->last_send_time > 1000))
        {
            // Packet timed out, time to resend

            NET_Conn_SendPacket(conn, conn->reliable_packets->packet);
            conn->reliable_packets->last_send_time = nowtime;
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
                conn->disconnect_reason = NET_DISCONNECT_LOCAL;
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
            conn->disconnect_reason = NET_DISCONNECT_REMOTE;
        }
    }
}

net_packet_t *NET_Conn_NewReliable(net_connection_t *conn, int packet_type)
{
    net_packet_t *packet;
    net_reliable_packet_t *rp;
    net_reliable_packet_t **listend;

    // Generate a packet with the right header

    packet = NET_NewPacket(100);

    NET_WriteInt16(packet, packet_type | NET_RELIABLE_PACKET);

    // write the low byte of the send sequence number
    
    NET_WriteInt8(packet, conn->reliable_send_seq & 0xff);

    // Add to the list of reliable packets

    rp = malloc(sizeof(net_reliable_packet_t));
    rp->packet = packet;
    rp->next = NULL;
    rp->seq = conn->reliable_send_seq;
    rp->last_send_time = -1;

    for (listend = &conn->reliable_packets; 
         *listend != NULL; 
         listend = &((*listend)->next));

    *listend = rp;

    // Count along the sequence

    conn->reliable_send_seq = (conn->reliable_send_seq + 1) & 0xff;

    // Finished
    
    return packet;
}

// Used to expand the least significant byte of a tic number into 
// the full tic number, from the current tic number

unsigned int NET_ExpandTicNum(unsigned int relative, unsigned int b)
{
    unsigned int l, h;
    unsigned int result;

    h = relative & ~0xff;
    l = relative & 0xff;

    result = h | b;

    if (l < 0x40 && b > 0xb0)
        result -= 0x100;
    if (l > 0xb0 && b < 0x40)
        result += 0x100;
    
    return result;
}

// Check that game settings are valid

boolean NET_ValidGameSettings(GameMode_t mode, GameMission_t mission,
                              net_gamesettings_t *settings)
{
    if (settings->ticdup <= 0)
        return false;

    if (settings->extratics < 0)
        return false;

    if (settings->deathmatch < 0 || settings->deathmatch > 2)
        return false;

    if (settings->skill < sk_noitems || settings->skill > sk_nightmare)
        return false;

    if (!D_ValidGameVersion(mission, settings->gameversion))
        return false;

    if (!D_ValidEpisodeMap(mission, mode, settings->episode, settings->map))
        return false;

    return true;
}

