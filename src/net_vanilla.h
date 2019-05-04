//
// Copyright(C) 2018 Simon Howard
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

#ifndef NET_VANILLA_H
#define NET_VANILLA_H

#include "net_defs.h"

typedef enum
{
    NET_VANILLA_PROTO_DOOM,
    NET_VANILLA_PROTO_HERETIC,
    NET_VANILLA_PROTO_HEXEN,
} net_vanilla_protocol_t;

typedef struct
{
    net_vanilla_protocol_t protocol;
    net_addr_t *addrs[MAXNETNODES];
    unsigned int num_nodes;

    unsigned int consoleplayer;
    unsigned int num_players;
    unsigned int player_class;
} net_vanilla_settings_t;

void NET_VanillaInit(net_context_t *context, net_vanilla_settings_t *settings);
boolean NET_VanillaSyncSettings(net_gamesettings_t *settings,
                                net_startup_callback_t callback);
void NET_VanillaSendTiccmd(ticcmd_t *ticcmd, int maketic);
void NET_VanillaQuit(void);
void NET_VanillaRun(void);

net_context_t *NET_DBIPX_Connect(char *address);
void NET_DBIPX_ArbitrateGame(net_vanilla_settings_t *settings, int want_nodes);

net_context_t *NET_Serial_Connect(char *address);
net_context_t *NET_Serial_Answer(void);
void NET_Serial_ArbitrateGame(net_context_t *context,
                              net_vanilla_settings_t *settings);

#endif /* #ifndef NET_CLIENT_H */

