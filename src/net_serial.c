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
// DESCRIPTION:
//     Networking code for sersetup.exe-compatible setup protocol.
//
// Hi! Are you looking at this file this because you're thinking of trying
// to add native serial support to Chocolate Doom? Please don't do that.
// Instead, just use a TCP-to-serial server. This avoids adding the
// complexity of having to deal with system-specific APIs.
//

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_misc.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_sdl.h"
#include "net_vanilla.h"

static net_addr_t *remote_addr;

net_context_t *NET_Serial_Connect(char *address)
{
    net_context_t *context;

    context = NET_NewContext();
    net_tcp_module.InitClient();
    NET_AddModule(context, &net_tcp_module);

    remote_addr = NET_ResolveAddress(context, address);
    if (remote_addr == NULL)
    {
        I_Error("NET_Serial_Connect: Failed to resolve address %s",
                address);
    }

    // TCP connection won't be opened until we send something.
    // So make sure we send at least one packet.
    {
        net_packet_t *packet;
        packet = NET_NewPacket(8);
        NET_WriteString(packet, "Hello!");
        NET_SendPacket(remote_addr, packet);
        NET_FreePacket(packet);
    }

    printf("NET_Serial_Connect: Connected to DOSBox modem server at %s.\n",
           NET_AddrToString(remote_addr));

    return context;
}

net_context_t *NET_Serial_Answer(void)
{
    net_context_t *context;
    net_packet_t *packet;
    net_addr_t *addr;

    context = NET_NewContext();
    net_tcp_module.InitServer();
    NET_AddModule(context, &net_tcp_module);

    // TODO: Include the port number in this message
    printf("NET_Serial_Answer: Listening for connection.\n");

    // Wait for a TCP connection - we want at least one packet to come
    // through.
    remote_addr = NULL;
    while (remote_addr == NULL)
    {
        if (NET_RecvPacket(context, &addr, &packet))
        {
            NET_ReleaseAddress(remote_addr);
            remote_addr = addr;
            NET_FreePacket(packet);
        }
        else
        {
            I_Sleep(100);
        }
    }

    printf("NET_Serial_Answer: Connection received from %s.\n",
           NET_AddrToString(remote_addr));

    return context;
}

static void SendSetupPacket(int id, int stage)
{
    net_packet_t *packet;
    char buf[32];
    int i;

    // TODO: This only works for the later version of sersetup.exe; the
    // protocol was changed in later versions. Maybe add support for the
    // older protocol in the future.
    X_snprintf(buf, sizeof(buf), "ID%06d_%d", id, stage);

    packet = NET_NewPacket(16);
    for (i = 0; i < strlen(buf); ++i)
    {
        NET_WriteInt8(packet, buf[i]);
    }
    NET_SendPacket(remote_addr, packet);
    NET_FreePacket(packet);
}

static boolean ProcessSetupPacket(net_packet_t *packet, int *id, int *stage)
{
    char buf[32];

    if (packet->len > sizeof(buf) - 1)
    {
        return false;
    }

    memcpy(buf, packet->data, packet->len);
    buf[packet->len] = '\0';

    return sscanf(buf, "ID%d_%d", id, stage) == 2;
}

static unsigned int HashAddr(net_addr_t *addr)
{
    unsigned int result = 5381;
    unsigned int i;
    char *addr_str;

    addr_str = NET_AddrToString(addr);
    // djb2 hash:
    for (i = 0; addr_str[i] != '\0'; ++i)
    {
        result = result * 33 + addr_str[i];
    }

    return result;
}

static int GenerateID(void)
{
    //!
    // @category vnet
    //
    // When used with -dbdial/-dbanswer, force this player to be player 1.
    //
    if (M_ParmExists("-player1"))
    {
        return 0;
    }

    //!
    // @category vnet
    //
    // When used with -dbdial/-dbanswer, force this player to be player 2.
    //
    if (M_ParmExists("-player2"))
    {
        return 999999;
    }

    // Otherwise generate the ID randomly, seeding from a hash of the remote
    // address. This allows games to be played locally with -dbdial/-dbanswer
    // in different Chocolate Doom instances without the possibility of the
    // two processes generating the same ID.
    srand(HashAddr(remote_addr));
    return rand() % 1000000;
}

void NET_Serial_ArbitrateGame(net_context_t *context,
                              net_vanilla_settings_t *settings)
{
    int id, stage, remote_id, remote_stage;
    net_packet_t *packet;
    net_addr_t *addr;

    id = GenerateID();
    stage = 0;
    remote_id = 0;
    remote_stage = 0;

    while (stage < 2)
    {
        while (NET_RecvPacket(context, &addr, &packet))
        {
            if (addr == remote_addr
             && ProcessSetupPacket(packet, &remote_id, &remote_stage))
            {
                stage = remote_stage + 1;
            }
            NET_FreePacket(packet);
            NET_ReleaseAddress(addr);
        }

        if (stage > 0 && id == remote_id)
        {
            I_Error("NET_Serial_ArbitrateGame: Duplicate ID strings, "
                    "ID #%06d. Check that both players are not providing "
                    "the same -player1 or -player2 command line argument.",
                    id);
        }

        SendSetupPacket(id, stage);
        I_Sleep(1000);
    }

    settings->addrs[0] = remote_addr;
    settings->num_nodes = 1;
    settings->num_players = 2;
    if (id < remote_id)
    {
        settings->consoleplayer = 0;
    }
    else
    {
        settings->consoleplayer = 1;
    }
}

