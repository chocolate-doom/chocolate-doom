// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_packet.c 229 2005-10-30 19:56:15Z fraggle $
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
//      Network packet manipulation (net_packet_t)
//
//-----------------------------------------------------------------------------

#include <string.h>
#include "net_packet.h"
#include "z_zone.h"

net_packet_t *NET_NewPacket(int initial_size)
{
    net_packet_t *packet;

    packet = (net_packet_t *) Z_Malloc(sizeof(net_packet_t), PU_STATIC, 0);
    
    if (initial_size == 0)
        initial_size = 256;

    packet->alloced = initial_size;
    packet->data = Z_Malloc(initial_size, PU_STATIC, 0);
    packet->len = 0;
    packet->pos = 0;

    return packet;
}

// duplicates an existing packet

net_packet_t *NET_PacketDup(net_packet_t *packet)
{
    net_packet_t *newpacket;

    newpacket = NET_NewPacket(packet->len);
    memcpy(newpacket->data, packet->data, packet->len);
    newpacket->len = packet->len;

    return newpacket;
}

void NET_FreePacket(net_packet_t *packet)
{
    Z_Free(packet->data);
    Z_Free(packet);
}

// Read a byte from the packet, returning true if read
// successfully

boolean NET_ReadInt8(net_packet_t *packet, unsigned int *data)
{
    if (packet->pos + 1 > packet->len)
        return false;

    *data = packet->data[packet->pos];

    packet->pos += 1;

    return true;
}

// Read a 16-bit integer from the packet, returning true if read
// successfully

boolean NET_ReadInt16(net_packet_t *packet, unsigned int *data)
{
    byte *p;

    if (packet->pos + 2 > packet->len)
        return false;

    p = packet->data + packet->pos;

    *data = (p[0] << 8) | p[1];
    packet->pos += 2;

    return true;
}

// Read a 32-bit integer from the packet, returning true if read
// successfully

boolean NET_ReadInt32(net_packet_t *packet, unsigned int *data)
{
    byte *p;

    if (packet->pos + 4 > packet->len)
        return false;

    p = packet->data + packet->pos;

    *data = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    packet->pos += 4;
    
    return true;
}

// Dynamically increases the size of a packet

static void NET_IncreasePacket(net_packet_t *packet)
{
    byte *newdata;
   
    packet->alloced *= 2;

    newdata = Z_Malloc(packet->alloced, PU_STATIC, 0);

    memcpy(newdata, packet->data, packet->len);

    Z_Free(packet->data);
    packet->data = newdata;
}

// Write a single byte to the packet

void NET_WriteInt8(net_packet_t *packet, unsigned int i)
{
    if (packet->len + 1 > packet->alloced)
        NET_IncreasePacket(packet);

    packet->data[packet->len] = i;
    packet->len += 1;
}

// Write a 16-bit integer to the packet

void NET_WriteInt16(net_packet_t *packet, unsigned int i)
{
    byte *p;
    
    if (packet->len + 2 > packet->alloced)
        NET_IncreasePacket(packet);

    p = packet->data + packet->len;

    p[0] = (i >> 8) & 0xff;
    p[1] = i & 0xff;

    packet->len += 2;
}


// Write a single byte to the packet

void NET_WriteInt32(net_packet_t *packet, unsigned int i)
{
    byte *p;

    if (packet->len + 4 > packet->alloced)
        NET_IncreasePacket(packet);

    p = packet->data + packet->len;

    p[0] = (i >> 24) & 0xff;
    p[1] = (i >> 16) & 0xff;
    p[2] = (i >> 8) & 0xff;
    p[3] = i & 0xff;

    packet->len += 4;
}




