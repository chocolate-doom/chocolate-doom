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
// DESCRIPTION:
//     Definitions for use in networking code.
//

#ifndef NET_PACKET_H
#define NET_PACKET_H

#include "net_defs.h"

net_packet_t *NET_NewPacket(int initial_size);
net_packet_t *NET_PacketDup(net_packet_t *packet);
void NET_FreePacket(net_packet_t *packet);

boolean NET_ReadInt8(net_packet_t *packet, unsigned int *data);
boolean NET_ReadInt16(net_packet_t *packet, unsigned int *data);
boolean NET_ReadInt32(net_packet_t *packet, unsigned int *data);

boolean NET_ReadSInt8(net_packet_t *packet, signed int *data);
boolean NET_ReadSInt16(net_packet_t *packet, signed int *data);
boolean NET_ReadSInt32(net_packet_t *packet, signed int *data);

char *NET_ReadString(net_packet_t *packet);

void NET_WriteInt8(net_packet_t *packet, unsigned int i);
void NET_WriteInt16(net_packet_t *packet, unsigned int i);
void NET_WriteInt32(net_packet_t *packet, unsigned int i);

void NET_WriteString(net_packet_t *packet, char *string);

#endif /* #ifndef NET_PACKET_H */

