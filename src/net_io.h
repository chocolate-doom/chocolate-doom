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
//      Network packet manipulation (net_packet_t)
//
//-----------------------------------------------------------------------------

#ifndef NET_IO_H
#define NET_IO_H

#include "net_defs.h"

extern net_addr_t net_broadcast_addr;

net_context_t *NET_NewContext(void);
void NET_AddModule(net_context_t *context, net_module_t *module);
void NET_SendPacket(net_addr_t *addr, net_packet_t *packet);
void NET_SendBroadcast(net_context_t *context, net_packet_t *packet);
boolean NET_RecvPacket(net_context_t *context, net_addr_t **addr, 
                       net_packet_t **packet);
char *NET_AddrToString(net_addr_t *addr);
void NET_FreeAddress(net_addr_t *addr);
net_addr_t *NET_ResolveAddress(net_context_t *context, char *address);

#endif  /* #ifndef NET_IO_H */

