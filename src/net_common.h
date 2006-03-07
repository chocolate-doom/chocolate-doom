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
// Revision 1.4  2006/02/19 13:42:27  fraggle
// Move tic number expansion code to common code.  Parse game data packets
// received from the server.
// Strip down d_net.[ch] to work through the new networking code.  Remove
// game sync code.
// Remove i_net.[ch] as it is no longer needed.
// Working networking!
//
// Revision 1.3  2006/01/10 19:59:26  fraggle
// Reliable packet transport mechanism
//
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

#ifndef NET_COMMON_H
#define NET_COMMON_H

#include "net_defs.h"
#include "net_packet.h"

typedef enum 
{
    // sending syn packets, waiting for an ACK reply 
    // (client side)

    NET_CONN_STATE_CONNECTING,

    // received a syn, sent an ack, waiting for an ack reply
    // (server side)

    NET_CONN_STATE_WAITING_ACK,
    
    // successfully connected

    NET_CONN_STATE_CONNECTED,

    // sent a DISCONNECT packet, waiting for a DISCONNECT_ACK reply

    NET_CONN_STATE_DISCONNECTING,

    // client successfully disconnected

    NET_CONN_STATE_DISCONNECTED,

    // We are disconnected, but in a sleep state, waiting for several
    // seconds.  This is in case the DISCONNECT_ACK we sent failed
    // to arrive, and we need to send another one.  We keep this as
    // a valid connection for a few seconds until we are sure that
    // the other end has successfully disconnected as well.

    NET_CONN_STATE_DISCONNECTED_SLEEP,

} net_connstate_t;

#define MAX_RETRIES 5

typedef struct net_reliable_packet_s net_reliable_packet_t;

typedef struct 
{
    net_connstate_t state;
    net_addr_t *addr;
    int last_send_time;
    int num_retries;
    int keepalive_send_time;
    int keepalive_recv_time;
    net_reliable_packet_t *reliable_packets;
    int reliable_send_seq;
    int reliable_recv_seq;
} net_connection_t;


void NET_Conn_SendPacket(net_connection_t *conn, net_packet_t *packet);
void NET_Conn_InitClient(net_connection_t *conn, net_addr_t *addr);
void NET_Conn_InitServer(net_connection_t *conn, net_addr_t *addr);
boolean NET_Conn_Packet(net_connection_t *conn, net_packet_t *packet,
                        unsigned int *packet_type);
void NET_Conn_Disconnect(net_connection_t *conn);
void NET_Conn_Run(net_connection_t *conn);
net_packet_t *NET_Conn_NewReliable(net_connection_t *conn, int packet_type);

// Other miscellaneous common functions

void NET_SafePuts(char *msg);
unsigned int NET_ExpandTicNum(unsigned int relative, unsigned int b);

#endif /* #ifndef NET_COMMON_H */

