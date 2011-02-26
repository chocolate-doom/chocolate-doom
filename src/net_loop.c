// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
// DESCRIPTION:
//      Loopback network module for server compiled into the client
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "doomdef.h"
#include "i_system.h"
#include "net_defs.h"
#include "net_loop.h"
#include "net_packet.h"

#define MAX_QUEUE_SIZE 16

typedef struct
{
    net_packet_t *packets[MAX_QUEUE_SIZE];
    int head, tail;
} packet_queue_t;

static packet_queue_t client_queue;
static packet_queue_t server_queue;
static net_addr_t client_addr;
static net_addr_t server_addr;

static void QueueInit(packet_queue_t *queue)
{
    queue->head = queue->tail = 0;
}

static void QueuePush(packet_queue_t *queue, net_packet_t *packet)
{
    int new_tail;

    new_tail = (queue->tail + 1) % MAX_QUEUE_SIZE;

    if (new_tail == queue->head)
    {
        // queue is full
        
        return;
    }

    queue->packets[queue->tail] = packet;
    queue->tail = new_tail;
}

static net_packet_t *QueuePop(packet_queue_t *queue)
{
    net_packet_t *packet;
    
    if (queue->tail == queue->head)
    {
        // queue empty

        return NULL;
    }

    packet = queue->packets[queue->head];
    queue->head = (queue->head + 1) % MAX_QUEUE_SIZE;

    return packet;
}

//-----------------------------------------------------------------------------
//
// Client end code
//
//-----------------------------------------------------------------------------

static boolean NET_CL_InitClient(void)
{
    QueueInit(&client_queue);

    return true;
}

static boolean NET_CL_InitServer(void)
{
    I_Error("NET_CL_InitServer: attempted to initialize client pipe end as a server!");
    return false;
}

static void NET_CL_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    QueuePush(&server_queue, NET_PacketDup(packet));
}

static boolean NET_CL_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    net_packet_t *popped;

    popped = QueuePop(&client_queue);

    if (popped != NULL)
    {
        *packet = popped;
        *addr = &client_addr;
        client_addr.module = &net_loop_client_module;
        
        return true;
    }

    return false;
}

static void NET_CL_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    snprintf(buffer, buffer_len, "local server");
}

static void NET_CL_FreeAddress(net_addr_t *addr)
{
}

static net_addr_t *NET_CL_ResolveAddress(char *address)
{
    if (address == NULL)
    {
        client_addr.module = &net_loop_client_module;

        return &client_addr;
    }
    else
    {
        return NULL;
    }
}

net_module_t net_loop_client_module =
{
    NET_CL_InitClient,
    NET_CL_InitServer,
    NET_CL_SendPacket,
    NET_CL_RecvPacket,
    NET_CL_AddrToString,
    NET_CL_FreeAddress,
    NET_CL_ResolveAddress,
};

//-----------------------------------------------------------------------------
//
// Server end code
//
//-----------------------------------------------------------------------------

static boolean NET_SV_InitClient(void)
{
    I_Error("NET_SV_InitClient: attempted to initialize server pipe end as a client!");
    return false;
}

static boolean NET_SV_InitServer(void)
{
    QueueInit(&server_queue);

    return true;
}

static void NET_SV_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    QueuePush(&client_queue, NET_PacketDup(packet));
}

static boolean NET_SV_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    net_packet_t *popped;

    popped = QueuePop(&server_queue);

    if (popped != NULL)
    {
        *packet = popped;
        *addr = &server_addr;
        server_addr.module = &net_loop_server_module;
        
        return true;
    }

    return false;
}

static void NET_SV_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    snprintf(buffer, buffer_len, "local client");
}

static void NET_SV_FreeAddress(net_addr_t *addr)
{
}

static net_addr_t *NET_SV_ResolveAddress(char *address)
{
    if (address == NULL)
    {
        server_addr.module = &net_loop_server_module;
        return &server_addr;
    }
    else
    {
        return NULL;
    }
}

net_module_t net_loop_server_module =
{
    NET_SV_InitClient,
    NET_SV_InitServer,
    NET_SV_SendPacket,
    NET_SV_RecvPacket,
    NET_SV_AddrToString,
    NET_SV_FreeAddress,
    NET_SV_ResolveAddress,
};


