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
//     Networking module that tunnels an IPX connection to a
//     DOSbox IPX server.
//
// Hi! Are you looking at this file this because you're thinking of trying
// to add native IPX support to Chocolate Doom? Please don't do that.
// Instead, look into writing a DOSbox IPX VPN server - a server that
// speaks the DOSbox IPX protocol and bridges onto a physical IPX network.
// Then both Choco and DOSbox can connect to real networks without the
// complexity of having to deal with system-specific IPX APIs.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "doomtype.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_misc.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_sdl.h"
#include "net_vanilla.h"
#include "z_zone.h"

#define DOOM_IPX_SOCKET  0x869b
#define SETUP_TIME 0xffffffff

typedef byte ipx_addr_t[6];

typedef struct
{
    net_addr_t net_addr;
    ipx_addr_t ipx_addr;
} addrpair_t;

typedef struct
{
    unsigned int checksum;
    unsigned int length;
    unsigned int trans_control;
    unsigned int packet_type;

    unsigned int dest_net;
    ipx_addr_t dest_addr;
    unsigned int dest_socket;

    unsigned int src_net;
    ipx_addr_t src_addr;
    unsigned int src_socket;
} ipx_header_t;

typedef struct
{
    int game_id;
    int drone;
    int nodes_found;
    int nodes_wanted;
} setupdata_t;

static const ipx_addr_t ipx_broadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static net_context_t *ipx_context;
static net_addr_t *server_addr;

static ipx_addr_t local_addr;
static unsigned int local_net;
static unsigned int socket_id = DOOM_IPX_SOCKET;
static unsigned int packet_id = 0;

extern net_module_t net_dbipx_module;
static addrpair_t **addr_table;
static int addr_table_size = 0;

// Finds an address by searching the table.  If the address is not found,
// it is added to the table.
static net_addr_t *NET_DBIPX_FindAddress(ipx_addr_t addr)
{
    addrpair_t *new_entry;
    int empty_entry = -1;
    int i;

    for (i=0; i<addr_table_size; ++i)
    {
        if (addr_table[i] != NULL
         && !memcmp(addr_table[i]->ipx_addr, addr, sizeof(ipx_addr_t)))
        {
            return &addr_table[i]->net_addr;
        }

        if (empty_entry < 0 && addr_table[i] == NULL)
        {
            empty_entry = i;
        }
    }

    // Was not found in list.  We need to add it.

    if (empty_entry < 0)
    {
        addr_table = realloc(addr_table,
                             sizeof(addrpair_t *) * (addr_table_size + 1));
        empty_entry = addr_table_size;
        ++addr_table_size;
    }

    // Add a new entry

    new_entry = Z_Malloc(sizeof(addrpair_t), PU_STATIC, 0);

    memcpy(new_entry->ipx_addr, addr, sizeof(ipx_addr_t));
    new_entry->net_addr.handle = &new_entry->ipx_addr;
    new_entry->net_addr.module = &net_dbipx_module;
    new_entry->net_addr.refcount = 0;
    addr_table[empty_entry] = new_entry;

    return &new_entry->net_addr;
}

static void NET_DBIPX_FreeAddress(net_addr_t *addr)
{
    int i;

    for (i = 0; i < addr_table_size; ++i)
    {
        if (addr == &addr_table[i]->net_addr)
        {
            Z_Free(addr_table[i]);
            addr_table[i] = NULL;
            return;
        }
    }

    I_Error("NET_DBIPX_FreeAddress: Attempted to remove an unused address!");
}

static void WriteIPXAddress(net_packet_t *packet, ipx_addr_t addr)
{
    unsigned int i;

    for (i = 0; i < sizeof(ipx_addr_t); ++i)
    {
        NET_WriteInt8(packet, addr[i]);
    }
}

static void WriteIPXHeader(net_packet_t *packet, ipx_header_t *header)
{
    NET_WriteInt16(packet,  header->checksum);
    NET_WriteInt16(packet,  header->length);
    NET_WriteInt8(packet,   header->trans_control);
    NET_WriteInt8(packet,   header->packet_type);

    NET_WriteInt32(packet,  header->dest_net);
    WriteIPXAddress(packet, header->dest_addr);
    NET_WriteInt16(packet,  header->dest_socket);

    NET_WriteInt32(packet,  header->src_net);
    WriteIPXAddress(packet, header->src_addr);
    NET_WriteInt16(packet,  header->src_socket);
}

static boolean ReadIPXAddress(net_packet_t *packet, ipx_addr_t addr)
{
    unsigned int i, n;

    for (i = 0; i < sizeof(ipx_addr_t); ++i)
    {
        if (!NET_ReadInt8(packet, &n))
        {
            return false;
        }
        addr[i] = n;
    }
    return true;
}

static boolean ReadIPXHeader(net_packet_t *packet, ipx_header_t *header)
{
    return NET_ReadInt16(packet,  &header->checksum)
        && NET_ReadInt16(packet,  &header->length)
        && NET_ReadInt8(packet,   &header->trans_control)
        && NET_ReadInt8(packet,   &header->packet_type)
        && NET_ReadInt32(packet,  &header->dest_net)
        && ReadIPXAddress(packet,  header->dest_addr)
        && NET_ReadInt16(packet,  &header->dest_socket)
        && NET_ReadInt32(packet,  &header->src_net)
        && ReadIPXAddress(packet,  header->src_addr)
        && NET_ReadInt16(packet,  &header->src_socket);
}

static void NET_DBIPX_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    ipx_header_t hdr;
    const byte *ipx_addr;

    if (addr == &net_broadcast_addr)
    {
        ipx_addr = (byte *) ipx_broadcast;
    }
    else
    {
        ipx_addr = (byte *) addr->handle;
    }

    hdr.checksum = 0xffff;
    hdr.length = 0x22 + packet->len + 0x4;
    hdr.trans_control = 0x00;
    hdr.packet_type = 0xff;

    hdr.dest_net = local_net;
    memcpy(hdr.dest_addr, ipx_addr, sizeof(ipx_addr_t));
    hdr.dest_socket = socket_id;

    hdr.src_net = local_net;
    memcpy(hdr.src_addr, local_addr, sizeof(ipx_addr_t));
    hdr.src_socket = socket_id;

    // Make a copy so we don't modify the original.
    packet = NET_PacketDup(packet);
    NET_SetPosition(packet, 0);
    WriteIPXHeader(packet, &hdr);

    // IPXSETUP includes a timestamp on every packet; not sure why but it's
    // important that it's there.
    NET_WriteInt32_LE(packet, packet_id);
    if (packet_id != SETUP_TIME)
    {
        ++packet_id;
    }

    NET_SetPosition(packet, packet->len);
    NET_WriteInt32(packet, 0x00000000);

    NET_SendPacket(server_addr, packet);

    NET_FreePacket(packet);
}

static boolean NET_DBIPX_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    ipx_header_t hdr;
    net_packet_t *got_packet;
    net_addr_t *got_addr;
    unsigned int unused;

    for (;;)
    {
        if (!NET_RecvPacket(ipx_context, &got_addr, &got_packet))
        {
            return false;
        }
        if (got_addr == server_addr
         && ReadIPXHeader(got_packet, &hdr)
         && NET_ReadInt32(got_packet, &unused)
         && got_packet->len >= 4)
        {
            NET_ReleaseAddress(got_addr);
            break;
        }
        NET_FreePacket(got_packet);
        NET_ReleaseAddress(got_addr);
    }

    *addr = NET_DBIPX_FindAddress(hdr.src_addr);
    *packet = got_packet;

    // Strip IPX footer
    got_packet->len -= 4;

    return true;
}

static void IPXAddrToString(byte *ipx_addr, char *buffer, int buffer_len)
{
    X_snprintf(buffer, buffer_len,
               "%02x:%02x:%02x:%02x:%02x:%02x",
               ipx_addr[0], ipx_addr[1], ipx_addr[2], ipx_addr[3],
               ipx_addr[4], ipx_addr[5]);
}

void NET_DBIPX_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    IPXAddrToString((byte *) addr->handle, buffer, buffer_len);
}

net_addr_t *NET_DBIPX_ResolveAddress(const char *address)
{
    unsigned int a, b, c, d, e, f;

    if (sscanf(address, "%02x:%02x:%02x:%02x:%02x:%02x",
               &a, &b, &c, &d, &e, &f) != 6)
    {
        return NULL;
    }

    {
        ipx_addr_t addr = {a, b, c, d, e, f};
        return NET_DBIPX_FindAddress(addr);
    }
}

// Complete module

net_module_t net_dbipx_module =
{
    NULL,
    NULL,
    NET_DBIPX_SendPacket,
    NET_DBIPX_RecvPacket,
    NET_DBIPX_AddrToString,
    NET_DBIPX_FreeAddress,
    NET_DBIPX_ResolveAddress,
};

static void SendRegistrationPacket(void)
{
    ipx_header_t hdr;
    net_packet_t *packet;

    memset(&hdr, 0, sizeof(hdr));

    hdr.checksum      = 0xffff;
    hdr.length        = 0x1e;
    hdr.trans_control = 0x00;
    hdr.packet_type   = 0xff;

    hdr.dest_socket = 2;   // 2 = registration
    hdr.src_socket = 2;

    packet = NET_NewPacket(32);
    WriteIPXHeader(packet, &hdr);
    NET_SendPacket(server_addr, packet);
    NET_FreePacket(packet);
}

static boolean ParseRegistrationResponse(net_packet_t *packet)
{
    ipx_header_t hdr;

    if (!ReadIPXHeader(packet, &hdr)
     || hdr.dest_socket != 2 || hdr.src_socket != 2)
    {
        return false;
    }

    memcpy(local_addr, hdr.dest_addr, sizeof(ipx_addr_t));
    local_net = hdr.dest_net;
    return true;
}

static boolean RegisterToServer(void)
{
    net_packet_t *packet;
    net_addr_t *addr;
    int i;

    for (i = 0; i < 5; ++i)
    {
        SendRegistrationPacket();

        while (NET_RecvPacket(ipx_context, &addr, &packet))
        {
            NET_ReleaseAddress(addr);
            if (addr == server_addr
             && ParseRegistrationResponse(packet))
            {
                return true;
            }
            NET_FreePacket(packet);
        }

        I_Sleep(1000);
    }

    return false;
}

net_context_t *NET_DBIPX_Connect(char *address)
{
    net_context_t *context;

    ipx_context = NET_NewContext();
    net_udp_module.InitClient();
    NET_AddModule(ipx_context, &net_udp_module);

    server_addr = NET_ResolveAddress(ipx_context, address);
    if (server_addr == NULL)
    {
        I_Error("NET_DBIPX_Connect: Failed to resolve address %s",
                address);
    }

    if (!RegisterToServer())
    {
        I_Error("NET_DBIPX_Connect: Failed to connect to DOSBox IPX "
                "server at address %s", NET_AddrToString(server_addr));
    }

    printf("NET_DBIPX_Connect: Connected to DOSBox IPX server at %s.\n",
           NET_AddrToString(server_addr));

    context = NET_NewContext();
    NET_AddModule(context, &net_dbipx_module);
    return context;
}

static net_packet_t *MakeSetupPacket(setupdata_t *data)
{
    net_packet_t *packet;

    packet = NET_NewPacket(16);
    NET_WriteInt16_LE(packet, data->game_id);
    NET_WriteInt16_LE(packet, data->drone);
    NET_WriteInt16_LE(packet, data->nodes_found);
    NET_WriteInt16_LE(packet, data->nodes_wanted);
    return packet;
}

static boolean ReadSetupData(net_packet_t *packet, setupdata_t *data)
{
    boolean result;

    result = NET_ReadSInt16_LE(packet, &data->game_id)
          && NET_ReadSInt16_LE(packet, &data->drone)
          && NET_ReadSInt16_LE(packet, &data->nodes_found)
          && NET_ReadSInt16_LE(packet, &data->nodes_wanted);

    return result;
}

static int FindOrAddAddr(net_vanilla_settings_t *settings, net_addr_t *addr,
                         int want_nodes)
{
    int i;

    for (i = 0; i < settings->num_nodes; ++i)
    {
        if (addr == settings->addrs[i])
        {
            return i;
        }
    }

    if (settings->num_nodes >= MAXNETNODES)
    {
        I_Error("Too many nodes! %i > %i", settings->num_nodes + 1,
                MAXNETNODES);
    }
    settings->addrs[settings->num_nodes] = addr;
    NET_ReferenceAddress(addr);
    i = settings->num_nodes;
    ++settings->num_nodes;
    printf("Found node at %s (%i/%i)\n", NET_AddrToString(addr),
           settings->num_nodes + 1, want_nodes);
    return i;
}

static boolean AllNodesReady(setupdata_t *node_data, int num_nodes)
{
    int i;

    for (i = 0; i < num_nodes; ++i)
    {
        if (node_data[i].nodes_found < node_data[i].nodes_wanted)
        {
            return false;
        }
    }

    return true;
}

static void SetPlayerNumber(net_vanilla_settings_t *settings,
                            setupdata_t *node_data)
{
    byte *ipx_addr;
    int i;

    settings->consoleplayer = 0;
    settings->num_players = 1;

    for (i = 0; i < settings->num_nodes; ++i)
    {
        if (node_data[i].drone)
        {
            continue;
        }
        ++settings->num_players;

        ipx_addr = (byte *) settings->addrs[i]->handle;
        if (memcmp(ipx_addr, local_addr, sizeof(ipx_addr_t)) < 0)
        {
            ++settings->consoleplayer;
        }
    }
}

void NET_DBIPX_ArbitrateGame(net_vanilla_settings_t *settings,
                             int want_nodes)
{
    setupdata_t setup, tmp;
    setupdata_t node_data[MAXNETNODES];
    char buf[32];
    net_packet_t *packet;
    net_addr_t *addr;
    int node_num, game_id;

    // During the setup phase we must send packet IDs with the special
    // setup time value. This distinguishes them from game packets.
    packet_id = SETUP_TIME;

    game_id = 0;  // TODO?
    settings->num_nodes = 0;

    printf("Looking for %i nodes:\n", want_nodes);
    IPXAddrToString(local_addr, buf, sizeof(buf));
    printf("Local address %s (1/%i)\n", buf, want_nodes);

    // Keep looping until we reach the desired number of nodes. Note
    // that want_nodes includes us. We also don't start until all other
    // nodes have reached their targets.
    while (settings->num_nodes + 1 < want_nodes
        || !AllNodesReady(node_data, settings->num_nodes))
    {
        while (NET_DBIPX_RecvPacket(&addr, &packet))
        {
            NET_ReferenceAddress(addr);
            if (ReadSetupData(packet, &tmp))
            {
                node_num = FindOrAddAddr(settings, addr, want_nodes);
                node_data[node_num] = tmp;
            }
            NET_FreePacket(packet);
            NET_ReleaseAddress(addr);
        }

        setup.game_id = game_id;
        setup.drone = 0;  // TODO
        setup.nodes_found = settings->num_nodes + 1;
        setup.nodes_wanted = want_nodes;

        packet = MakeSetupPacket(&setup);
        NET_DBIPX_SendPacket(&net_broadcast_addr, packet);
        NET_FreePacket(packet);

        I_Sleep(1000);
    }

    SetPlayerNumber(settings, node_data);
    packet_id = 0;
}

