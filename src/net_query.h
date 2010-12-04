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
//     Querying servers to find their current status.
//

#ifndef NET_QUERY_H
#define NET_QUERY_H

#include "net_defs.h"

typedef void (*net_query_callback_t)(net_addr_t *addr,
                                     net_querydata_t *querydata,
                                     unsigned int ping_time,
                                     void *user_data);

extern int NET_LANQuery(net_query_callback_t callback, void *user_data);
extern int NET_MasterQuery(net_query_callback_t callback, void *user_data);
extern void NET_QueryAddress(char *addr);
extern net_addr_t *NET_FindLANServer(void);

extern void NET_QueryPrintCallback(net_addr_t *addr, net_querydata_t *data,
                                   unsigned int ping_time, void *user_data);

extern net_addr_t *NET_Query_ResolveMaster(net_context_t *context);
extern void NET_Query_AddToMaster(net_addr_t *master_addr);
extern void NET_Query_MasterResponse(net_packet_t *packet);

#endif /* #ifndef NET_QUERY_H */

