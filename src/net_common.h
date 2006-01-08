// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_common.h 263 2006-01-08 00:10:48Z fraggle $
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

typedef struct 
{
    net_connstate_t state;
    net_addr_t *addr;
    int last_send_time;
    int num_retries;
} net_connection_t;


void NET_Conn_InitClient(net_connection_t *conn, net_addr_t *addr);
void NET_Conn_InitServer(net_connection_t *conn, net_addr_t *addr);
boolean NET_Conn_Packet(net_connection_t *conn, net_packet_t *packet,
                        int packet_type);
void NET_Conn_Disconnect(net_connection_t *conn);
void NET_Conn_Run(net_connection_t *conn);

#endif /* #ifndef NET_COMMON_H */

