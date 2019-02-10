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
//      Network packet manipulation (net_packet_t)
//

#ifndef NET_IO_H
#define NET_IO_H

#include "net_defs.h"

extern net_addr_t net_broadcast_addr;

// Create a new network context.
net_context_t *NET_NewContext(void);

// Add a network module to a context.
void NET_AddModule(net_context_t *context, net_module_t *module);

// Send a packet to the given address.
void NET_SendPacket(net_addr_t *addr, net_packet_t *packet);

// Send a broadcast using all modules in the given context.
void NET_SendBroadcast(net_context_t *context, net_packet_t *packet);

// Check all modules in the given context and receive a packet, returning true
// if a packet was received. The result is stored in *packet and the source is
// stored in *addr, with an implicit reference added. The packet must be freed
// by the caller and the reference releasd.
boolean NET_RecvPacket(net_context_t *context, net_addr_t **addr,
                       net_packet_t **packet);

// Return a string representation of the given address. The result points to a
// static buffer and will become invalid with the next call.
char *NET_AddrToString(net_addr_t *addr);

// Add a reference to the given address.
void NET_ReferenceAddress(net_addr_t *addr);

// Release a reference to the given address. When there are no more references,
// the address will be freed.
void NET_ReleaseAddress(net_addr_t *addr);

// Resolve a string representation of an address. If successful, a net_addr_t
// pointer is received with an implicit reference that must be freed by the
// caller when it is no longer needed.
net_addr_t *NET_ResolveAddress(net_context_t *context, const char *address);

#endif  /* #ifndef NET_IO_H */

