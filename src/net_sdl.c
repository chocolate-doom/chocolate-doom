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
// Revision 1.1  2005/10/30 19:56:15  fraggle
// Add foundation code for the new networking system
//
//
// DESCRIPTION:
//     Networking module which uses SDL_net
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "net_defs.h"
#include "net_packet.h"
#include "net_sdl.h"
#include "z_zone.h"

//
// NETWORKING
//

#include <SDL_net.h>

static int port = 2342;
static UDPsocket udpsocket;
static UDPpacket *recvpacket;

typedef struct
{
    net_addr_t net_addr;
    IPaddress sdl_addr;
} addrpair_t;

static addrpair_t **addr_table;
static int addr_table_size;

// Initialises the address table

static void NET_SDL_InitAddrTable(void)
{
    addr_table_size = 16;

    addr_table = Z_Malloc(sizeof(addrpair_t *) * addr_table_size,
                          PU_STATIC, 0);
    memset(addr_table, 0, sizeof(addrpair_t *) * addr_table_size);
}

static boolean AddressesEqual(IPaddress *a, IPaddress *b)
{
    return a->host == b->host
        && a->port == b->port;
}

// Finds an address by searching the table.  If the address is not found,
// it is added to the table.

static net_addr_t *NET_SDL_FindAddress(IPaddress *addr)
{
    addrpair_t *new_entry;
    int empty_entry = -1;
    int i;

    for (i=0; i<addr_table_size; ++i)
    {
        if (addr_table[i] != NULL
         && AddressesEqual(addr, &addr_table[i]->sdl_addr))
        {
            return &addr_table[i]->net_addr;
        }

        if (empty_entry < 0 && addr_table[i] == NULL)
            empty_entry = i;
    }

    // Was not found in list.  We need to add it.

    // Is there any space in the table? If not, increase the table size

    if (empty_entry < 0)
    {
        addrpair_t **new_addr_table;
        int new_addr_table_size;

        // after reallocing, we will add this in as the first entry
        // in the new block of memory

        empty_entry = addr_table_size;
        
        // allocate a new array twice the size, init to 0 and copy 
        // the existing table in.  replace the old table.

        new_addr_table_size = addr_table_size * 2;
        new_addr_table = Z_Malloc(sizeof(addrpair_t *) * new_addr_table_size,
                                  PU_STATIC, 0);
        memset(new_addr_table, 0, sizeof(addrpair_t *) * new_addr_table_size);
        memcpy(new_addr_table, addr_table, 
               sizeof(addrpair_t *) * addr_table_size);
        Z_Free(addr_table);
        addr_table = new_addr_table;
        addr_table_size = new_addr_table_size;
    }

    // Add a new entry
    
    new_entry = Z_Malloc(sizeof(addrpair_t), PU_STATIC, 0);

    new_entry->sdl_addr = *addr;
    new_entry->net_addr.handle = &new_entry->sdl_addr;
    new_entry->net_addr.module = &i_net_module;

    addr_table[empty_entry] = new_entry;

    return &new_entry->net_addr;
}

static void NET_SDL_FreeAddress(net_addr_t *addr)
{
    int i;
    
    for (i=0; i<addr_table_size; ++i)
    {
        if (addr == &addr_table[i]->net_addr)
        {
            Z_Free(addr_table[i]);
            addr_table[i] = NULL;
            return;
        }
    }

    I_Error("NET_SDL_FreeAddress: Attempted to remove an unused address!");
}

static boolean NET_SDL_InitClient(void)
{
    SDLNet_Init();

    udpsocket = SDLNet_UDP_Open(0);

    if (udpsocket == NULL)
    {
        I_Error("NET_SDL_InitClient: Unable to open a socket!");
    }
    
    NET_SDL_InitAddrTable();
    recvpacket = SDLNet_AllocPacket(1500);

    return true;
}

static boolean NET_SDL_InitServer(void)
{
    int p;
    
    p = M_CheckParm("-port");
    if (p > 0)
        port = atoi(myargv[p+1]);

    SDLNet_Init();

    udpsocket = SDLNet_UDP_Open(port);

    if (udpsocket == NULL)
    {
        I_Error("NET_SDL_InitServer: Unable to bind to port %i", port);
    }

    NET_SDL_InitAddrTable();
    recvpacket = SDLNet_AllocPacket(1500);

    return true;
}

static void NET_SDL_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    UDPpacket sdl_packet;
    IPaddress *ip = (IPaddress *) addr->handle;

    sdl_packet.channel = 0;
    sdl_packet.data = packet->data;
    sdl_packet.len = packet->len;
    sdl_packet.address = *ip;

    if (!SDLNet_UDP_Send(udpsocket, -1, &sdl_packet))
    {
        I_Error("NET_SDL_SendPacket: Error transmitting packet: %s",
                SDLNet_GetError());
    }
}

static boolean NET_SDL_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    int result;

    result = SDLNet_UDP_Recv(udpsocket, recvpacket);

    if (result < 0)
    {
        I_Error("NET_SDL_RecvPacket: Error receiving packet: %s",
                SDLNet_GetError());
    }

    // no packets received

    if (result == 0)
        return false;

    // Put the data into a new packet structure

    *packet = NET_NewPacket(recvpacket->len);
    memcpy((*packet)->data, recvpacket->data, recvpacket->len);
    (*packet)->len = recvpacket->len;

    // Address

    *addr = NET_SDL_FindAddress(&recvpacket->address);

    return true;
}

void NET_SDL_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    IPaddress *ip;

    ip = (IPaddress *) addr->handle;
    
    snprintf(buffer, buffer_len, 
             "%i.%i.%i.%i",
             (ip->host >> 24) & 0xff,
             (ip->host >> 16) & 0xff,
             (ip->host >> 8) & 0xff,
             ip->host & 0xff);
}

net_addr_t *NET_SDL_ResolveAddress(char *address)
{
    IPaddress ip;
    
    if (SDLNet_ResolveHost(&ip, address, port))
    {
        // unable to resolve

        return NULL;
    }

    return NET_SDL_FindAddress(&ip);
}

// Complete module

net_module_t i_net_module =
{
    NET_SDL_InitClient,
    NET_SDL_InitServer,
    NET_SDL_SendPacket,
    NET_SDL_RecvPacket,
    NET_SDL_AddrToString,
    NET_SDL_FreeAddress,
    NET_SDL_ResolveAddress,
};

