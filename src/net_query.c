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

#include <stdarg.h>
#include <stdlib.h>

#include "i_system.h"
#include "i_timer.h"

#include "net_common.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_query.h"
#include "net_structrw.h"
#include "net_sdl.h"

typedef struct 
{
    net_addr_t *addr;
    net_querydata_t data;
} queryresponse_t;

static net_context_t *query_context;
static queryresponse_t *responders;
static int num_responses;

// Add a new address to the list of hosts that has responded

static queryresponse_t *AddResponder(net_addr_t *addr, 
                                     net_querydata_t *data)
{
    queryresponse_t *response;

    responders = realloc(responders, 
                         sizeof(queryresponse_t) * (num_responses + 1));

    response = &responders[num_responses];
    response->addr = addr;
    response->data = *data;
    ++num_responses;

    return response;
}

// Returns true if the reply is from a host that has not previously
// responded.

static boolean CheckResponder(net_addr_t *addr)
{
    int i;

    for (i=0; i<num_responses; ++i)
    {
        if (responders[i].addr == addr)
        {
            return false;
        }
    }

    return true;
}

// Transmit a query packet

static void NET_Query_SendQuery(net_addr_t *addr)
{
    net_packet_t *request;

    request = NET_NewPacket(10);
    NET_WriteInt16(request, NET_PACKET_TYPE_QUERY);

    if (addr == NULL)
    {
        NET_SendBroadcast(query_context, request);
    }
    else
    {
        NET_SendPacket(addr, request);
    }

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

static void PrintHeader(void)
{
    int i;

    formatted_printf(18, "Address");
    formatted_printf(8, "Players");
    puts("Description");

    for (i=0; i<70; ++i)
        putchar('=');
    putchar('\n');
}

static void PrintResponse(queryresponse_t *response)
{
    formatted_printf(18, "%s: ", NET_AddrToString(response->addr));
    formatted_printf(8, "%i/%i", response->data.num_players, 
                                 response->data.max_players);

    if (response->data.gamemode != indetermined)
    {
        printf("(%s) ", GameDescription(response->data.gamemode, 
                                        response->data.gamemission));
    }

    if (response->data.server_state)
    {
        printf("(game running) ");
    }

    NET_SafePuts(response->data.description);
}

static void NET_Query_ParsePacket(net_addr_t *addr, net_packet_t *packet)
{
    unsigned int packet_type;
    net_querydata_t querydata;
    queryresponse_t *response;

    // Have we already received a packet from this host?

    if (!CheckResponder(addr))
    {
        return;
    }

    // Read the header

    if (!NET_ReadInt16(packet, &packet_type)
     || packet_type != NET_PACKET_TYPE_QUERY_RESPONSE)
    {
        return;
    }

    // Read query data

    if (!NET_ReadQueryData(packet, &querydata))
    {
        return;
    }

    if (num_responses <= 0)
    {
        // If this is the first response, print the table header

        PrintHeader();
    }

    response = AddResponder(addr, &querydata);

    PrintResponse(response);
}

static void NET_Query_GetResponse(void)
{
    net_addr_t *addr;
    net_packet_t *packet;

    if (NET_RecvPacket(query_context, &addr, &packet))
    {
        NET_Query_ParsePacket(addr, packet);
        NET_FreePacket(packet);
    }
}

static net_addr_t *NET_Query_QueryLoop(net_addr_t *addr, 
                                       boolean find_one)
{
    int start_time;
    int last_send_time;

    last_send_time = -1;
    start_time = I_GetTimeMS();

    while (I_GetTimeMS() < start_time + 5000)
    {
        // Send a query once every second

        if (last_send_time < 0 || I_GetTimeMS() > last_send_time + 1000)
        {
            NET_Query_SendQuery(addr);
            last_send_time = I_GetTimeMS();
        }

        // Check for a response

        NET_Query_GetResponse();

        // Found a response?

        if (find_one && num_responses > 0)
            break;
        
        // Don't thrash the CPU
        
        I_Sleep(100);
    }

    if (num_responses > 0)
        return responders[0].addr;
    else
        return NULL;
}

void NET_Query_Init(void)
{
    query_context = NET_NewContext();
    NET_AddModule(query_context, &net_sdl_module);
    net_sdl_module.InitClient();

    responders = NULL;
    num_responses = 0;
}

void NET_QueryAddress(char *addr)
{
    net_addr_t *net_addr;
    
    NET_Query_Init();

    net_addr = NET_ResolveAddress(query_context, addr);

    if (net_addr == NULL)
    {
        I_Error("NET_QueryAddress: Host '%s' not found!", addr);
    }

    printf("\nQuerying '%s'...\n\n", addr);

    if (!NET_Query_QueryLoop(net_addr, true))
    {
        I_Error("No response from '%s'", addr);
    }

    exit(0);
}

net_addr_t *NET_FindLANServer(void)
{
    NET_Query_Init();

    return NET_Query_QueryLoop(NULL, true);
}

void NET_LANQuery(void)
{
    NET_Query_Init();

    printf("\nSearching for servers on local LAN ...\n\n");

    if (!NET_Query_QueryLoop(NULL, false))
    {
        I_Error("No servers found");
    }

    exit(0);
}

