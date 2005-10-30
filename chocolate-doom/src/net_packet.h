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
//     Definitions for use in networking code.
//
//-----------------------------------------------------------------------------

#ifndef NET_PACKET_H
#define NET_PACKET_H

#include "net_defs.h"

net_packet_t *NET_NewPacket(int initial_size);
net_packet_t *NET_PacketDup(net_packet_t *packet);
void NET_FreePacket(net_packet_t *packet);
boolean NET_ReadInt8(net_packet_t *packet, unsigned int *data);
boolean NET_ReadInt16(net_packet_t *packet, unsigned int *data);
boolean NET_ReadInt32(net_packet_t *packet, unsigned int *data);
void NET_WriteInt8(net_packet_t *packet, unsigned int i);
void NET_WriteInt16(net_packet_t *packet, unsigned int i);
void NET_WriteInt32(net_packet_t *packet, unsigned int i);

#endif /* #ifndef NET_PACKET_H */

