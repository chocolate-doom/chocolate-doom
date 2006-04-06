// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_client.c 462 2006-04-06 19:31:45Z fraggle $
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

#include <stdarg.h>
#include <stdlib.h>

#include "i_system.h"
#include "i_timer.h"

#include "net_common.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_query.h"
#include "net_sdl.h"

static net_context_t *query_context;
static int num_responses;

static void NET_Query_SendQuery(net_addr_t *addr)
{
    net_packet_t *request;

    request = NET_NewPacket(10);
    NET_WriteInt16(request, NET_PACKET_TYPE_QUERY);
    NET_SendPacket(addr, request);
    NET_FreePacket(request);
}

static void formatted_printf(int wide, char *s, ...)
{
    va_list args;
    int i;
    
    va_start(args, s);
    i = vprintf(s, args);
    va_end(args);

    while (i < wide) 
    {
        putchar(' ');
        ++i;
    } 
}

static char *GameDescription(GameMode_t mode, GameMission_t mission)
{
    switch (mode)
    {
        case shareware:
            return "shareware";
        case registered:
            return "registered";
        case retail:
            return "ultimate";
        case commercial:
            if (mission == doom2)
                return "doom2";
            else if (mission == pack_tnt)
                return "tnt";
            else if (mission == pack_plut)
                return "plutonia";
        default:
            return "unknown";
    }
}

static void NET_Query_ParsePacket(net_addr_t *addr, net_packet_t *packet)
{
    unsigned int packet_type;
    char *server_version;
    unsigned int in_game;
    unsigned int num_players, max_players;
    unsigned int servermode, servermission;
    char *server_description;
    int i;

    if (!NET_ReadInt16(packet, &packet_type)
     || !(server_version = NET_ReadString(packet))
     || !NET_ReadInt8(packet, &in_game)
     || !NET_ReadInt8(packet, &num_players)
     || !NET_ReadInt8(packet, &max_players)
     || !NET_ReadInt8(packet, &servermode)
     || !NET_ReadInt8(packet, &servermission)
     || !(server_description = NET_ReadString(packet)))
    {
        return;
    }

    if (num_responses <= 0)
    {
        // If this is the first response, print the table header

        formatted_printf(18, "Address");
        formatted_printf(8, "Players");
        puts("Description");

        for (i=0; i<70; ++i)
            putchar('=');
        putchar('\n');
    }

    formatted_printf(18, "%s: ", NET_AddrToString(addr));
    formatted_printf(8, "%i/%i", num_players, max_players);

    if (servermode != indetermined)
    {
        printf("(%s) ", GameDescription(servermode, servermission));
    }

    if (in_game)
    {
        printf("(game running) ");
    }

    NET_SafePuts(server_description);

    ++num_responses;
}

static void NET_Query_GetResponse(void)
{
    net_addr_t *addr;
    net_packet_t *packet;

    if (NET_RecvPacket(query_context, &addr, &packet))
    {
        NET_Query_ParsePacket(addr, packet);
    }
}

void NET_Query_Init(void)
{
    query_context = NET_NewContext();
    NET_AddModule(query_context, &net_sdl_module);
    net_sdl_module.InitClient();

    num_responses = 0;
}

void NET_QueryAddress(char *addr)
{
    int start_time;
    net_addr_t *net_addr;
    
    NET_Query_Init();

    net_addr = NET_ResolveAddress(query_context, addr);

    if (net_addr == NULL)
    {
        I_Error("NET_QueryAddress: Host '%s' not found!", addr);
    }

    printf("\nQuerying '%s'...\n\n", addr);

    NET_Query_SendQuery(net_addr);

    start_time = I_GetTimeMS();

    while (num_responses <= 0 && I_GetTimeMS() < start_time + 5000)
    {
        NET_Query_GetResponse();
        I_Sleep(100);
    }

    if (num_responses <= 0)
    {
        I_Error("No response from '%s'", addr);
    }

    exit(0);
}

