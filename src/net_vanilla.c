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
//
// Vanilla Doom networking protocol
//

#include <string.h>

#include "d_mode.h"

#include "i_system.h"
#include "i_timer.h"

#include "net_common.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_vanilla.h"

#define NCMD_EXIT        0x80000000
#define NCMD_RETRANSMIT  0x40000000
#define NCMD_SETUP       0x20000000
#define NCMD_KILL        0x10000000      // kill game
#define NCMD_CHECKSUM    0x0fffffff

#define PL_DRONE 0x80    // bit flag in doomdata_t.player

#define MAX_DOOMDATA_TICS 12

typedef struct
{
    unsigned int retransmitfrom;
    unsigned int starttic;
    unsigned int player;
    unsigned int numtics;
    ticcmd_t cmds[MAX_DOOMDATA_TICS];
} doomdata_t;

typedef struct
{
    boolean ingame;
    int player;
    boolean drone;
    // tics received from this player, as a circular buffer.
    ticcmd_t recvwindow[MAX_DOOMDATA_TICS];
    // We have received all tics up to this number:
    unsigned int latest;
    // If true, we received an out-of-order packet and the next
    // tics packet should include a resend request.
    boolean need_resend;
} node_t;

typedef struct
{
    unsigned int player;
    unsigned int player_class;
    unsigned int ready;
} hexen_class_packet_t;

extern void D_ReceiveTic(ticcmd_t *ticcmds, boolean *playeringame);

static node_t nodes[NET_MAXPLAYERS];

// Recently-generated tics, as a circular buffer:
static ticcmd_t sendwindow[MAX_DOOMDATA_TICS];

// Number of tics we have made and sent to other peers.
static int sent_tics;

// Number of tics we have full received from other peers and "run".
static int run_tics;

static net_context_t *vcontext;
static net_vanilla_settings_t vsettings;
static int extratics;

// Read and calculate the checksum for the given packet, starting from
// the current position.
static unsigned int CalculateChecksum(net_packet_t *packet)
{
    unsigned int c, i;
    unsigned int pos;
    unsigned int val;

    pos = NET_GetPosition(packet);

    c = 0x1234567;
    for (i = 0; NET_ReadInt32_LE(packet, &val); ++i)
    {
        c += val * (i + 1);
    }

    NET_SetPosition(packet, pos);

    return c;
}

// Add the checksum (including specified packet type) to the beginning of
// the specified packet.
static void AddChecksum(net_packet_t *packet, unsigned int packet_type)
{
    unsigned int csum;

    NET_SetPosition(packet, 0);
    csum = CalculateChecksum(packet);
    NET_WriteInt32_LE(packet, (csum & NCMD_CHECKSUM) | packet_type);
}

// Read the packet type and verify the checksum. If the checksum is
// invalid then NCMD_CHECKSUM is returned.
static unsigned int ReadPacketType(net_packet_t *packet)
{
    unsigned int csum, got_csum;
    boolean ok;

    ok = NET_ReadInt32_LE(packet, &csum);
    got_csum = CalculateChecksum(packet);

    if (!ok || (got_csum & NCMD_CHECKSUM) != (csum & NCMD_CHECKSUM))
    {
        return NCMD_CHECKSUM;
    }

    return csum & ~NCMD_CHECKSUM;
}

static net_packet_t *MakeSetupPacket(net_gamesettings_t *settings)
{
    net_packet_t *packet;

    packet = NET_NewPacket(8);

    NET_WriteInt8(packet, (settings->skill & 0x0f)
                        | ((settings->deathmatch & 0x03) << 6)
                        | (settings->nomonsters ? 0x20 : 0)
                        | (settings->respawn_monsters ? 0x10 : 0));
    switch (vsettings.protocol)
    {
        case NET_VANILLA_PROTO_DOOM:
            NET_WriteInt8(packet, (settings->episode << 6) | settings->map);
            break;
        case NET_VANILLA_PROTO_HERETIC:
            NET_WriteInt8(packet, (settings->episode << 4) | settings->map);
            break;
        case NET_VANILLA_PROTO_HEXEN:
            NET_WriteInt8(packet, settings->map & 0x3f);
            break;
    }
    NET_WriteInt8(packet, D_GameVersionCode(settings->gameversion));
    NET_WriteInt8(packet, 0); // numtics = 0

    AddChecksum(packet, NCMD_SETUP);
    return packet;
}

static boolean ParseSetupPacket(net_packet_t *packet,
                                net_gamesettings_t *settings)
{
    unsigned int val;
    int version_code;

    if (!NET_ReadInt8(packet, &val))
    {
        return false;
    }
    settings->skill = val & 0x0f;
    settings->deathmatch = (val >> 6) & 0x03;
    settings->nomonsters = (val & 0x20) != 0;
    settings->respawn_monsters = (val & 0x10) != 0;

    if (!NET_ReadInt8(packet, &val))
    {
        return false;
    }

    switch (vsettings.protocol)
    {
        case NET_VANILLA_PROTO_DOOM:
            settings->episode = val >> 6;
            settings->map = val & 0x1f;
            break;
        case NET_VANILLA_PROTO_HERETIC:
            settings->episode = val >> 4;
            settings->map = val & 0x0f;
            break;
        case NET_VANILLA_PROTO_HEXEN:
            // Ignore player-class setup packets.
            if (val >= 64)
            {
                return false;
            }
            settings->episode = 1;
            settings->map = val;
            break;
    }

    if (!NET_ReadInt8(packet, &val))
    {
        return false;
    }

    version_code = D_GameVersionCode(settings->gameversion);
    if (val != version_code)
    {
        I_Error("Different DOOM versions cannot play a net game! %d != %d",
                 val, version_code);
    }

    // Last byte is numtics but is unused
    return NET_ReadInt8(packet, &val);
}

static net_packet_t *MakeHexenClassPacket(hexen_class_packet_t *h)
{
    net_packet_t *packet;

    packet = NET_NewPacket(8);

    NET_WriteInt8(packet, h->ready);
    NET_WriteInt8(packet, h->player_class + 64);
    NET_WriteInt8(packet, h->player);
    NET_WriteInt8(packet, 0);
    AddChecksum(packet, NCMD_SETUP);
    return packet;
}

static boolean ReadHexenClassPacket(net_packet_t *packet,
                                    hexen_class_packet_t *h)
{
    unsigned int throwaway;
    boolean result;

    result = NET_ReadInt8(packet, &h->ready)
          && NET_ReadInt8(packet, &h->player_class)
          && NET_ReadInt8(packet, &h->player)
          && NET_ReadInt8(packet, &throwaway)
          && throwaway == 0
          && h->player_class >= 64 && h->player_class < 67;

    h->player_class -= 64;
    return result;
}

static void WriteTiccmd(net_packet_t *packet, ticcmd_t *ticcmd)
{
    NET_WriteInt8(packet, ticcmd->forwardmove);
    NET_WriteInt8(packet, ticcmd->sidemove);
    NET_WriteInt16_LE(packet, ticcmd->angleturn);
    NET_WriteInt16_LE(packet, ticcmd->consistancy);
    NET_WriteInt8(packet, ticcmd->chatchar);
    NET_WriteInt8(packet, ticcmd->buttons);

    switch (vsettings.protocol)
    {
        case NET_VANILLA_PROTO_DOOM:
            break;
        case NET_VANILLA_PROTO_HERETIC:
        case NET_VANILLA_PROTO_HEXEN:
            NET_WriteInt8(packet, ticcmd->lookfly);
            NET_WriteInt8(packet, ticcmd->arti);
            break;
    }
}

static net_packet_t *MakeTicsPacket(doomdata_t *data, unsigned int flags)
{
    net_packet_t *packet;
    unsigned int i;

    packet = NET_NewPacket(32);
    NET_WriteInt8(packet, data->retransmitfrom);
    NET_WriteInt8(packet, data->starttic);
    NET_WriteInt8(packet, data->player);
    NET_WriteInt8(packet, data->numtics);

    if (data->numtics > MAX_DOOMDATA_TICS)
    {
        data->numtics = MAX_DOOMDATA_TICS;
    }

    for (i = 0; i < data->numtics; ++i)
    {
        WriteTiccmd(packet, &data->cmds[i]);
    }

    AddChecksum(packet, flags);
    return packet;
}

static boolean ReadTiccmd(net_packet_t *packet, ticcmd_t *ticcmd)
{
    int forwardmove, sidemove, angleturn, consistency;
    unsigned int chatchar, buttons, lookfly, arti;

    if (!NET_ReadSInt8(packet, &forwardmove)
     || !NET_ReadSInt8(packet, &sidemove)
     || !NET_ReadSInt16_LE(packet, &angleturn)
     || !NET_ReadSInt16_LE(packet, &consistency)
     || !NET_ReadInt8(packet, &chatchar)
     || !NET_ReadInt8(packet, &buttons))
    {
        return false;
    }

    ticcmd->forwardmove = forwardmove;
    ticcmd->sidemove = sidemove;
    ticcmd->angleturn = angleturn;
    ticcmd->consistancy = consistency;
    ticcmd->chatchar = chatchar;
    ticcmd->buttons = buttons;

    switch (vsettings.protocol)
    {
        case NET_VANILLA_PROTO_DOOM:
            break;
        case NET_VANILLA_PROTO_HERETIC:
        case NET_VANILLA_PROTO_HEXEN:
            if (!NET_ReadInt8(packet, &lookfly)
             || !NET_ReadInt8(packet, &arti))
            {
                return false;
            }
            ticcmd->lookfly = lookfly;
            ticcmd->arti = arti;
            break;
    }

    return true;
}

static boolean ReadTicsPacket(net_packet_t *packet, doomdata_t *data)
{
    unsigned int i;

    if (!NET_ReadInt8(packet, &data->retransmitfrom)
     || !NET_ReadInt8(packet, &data->starttic)
     || !NET_ReadInt8(packet, &data->player)
     || !NET_ReadInt8(packet, &data->numtics)
     || data->numtics > MAX_DOOMDATA_TICS)
    {
        return false;
    }

    for (i = 0; i < data->numtics; ++i)
    {
        if (!ReadTiccmd(packet, &data->cmds[i]))
        {
            return false;
        }
    }

    return true;
}

void NET_VanillaInit(net_context_t *context, net_vanilla_settings_t *settings)
{
    unsigned int n;

    vcontext = context;
    vsettings = *settings;
    sent_tics = 0;
    run_tics = 0;

    for (n = 0; n < vsettings.num_nodes; ++n)
    {
        nodes[n].ingame = true;
        nodes[n].player = -1;
    }
}

static int CountBoolVector(boolean *values, int values_len)
{
    int result, i;

    for (i = 0, result = 0; i < values_len; ++i)
    {
        if (values[i])
        {
            ++result;
        }
    }

    return result;
}

// Synchronize player class types. Only used for Hexen protocol.
static boolean SyncPlayerClasses(net_gamesettings_t *settings,
                                 net_startup_callback_t callback)
{
    net_packet_t *packet;
    net_addr_t *addr;
    hexen_class_packet_t hpkt;
    boolean got_class[NET_MAXPLAYERS];
    boolean ready[NET_MAXPLAYERS];
    unsigned int i;

    for (i = 0; i < settings->num_players; ++i)
    {
        got_class[i] = false;
        ready[i] = false;
    }

    settings->player_classes[settings->consoleplayer] = vsettings.player_class;
    got_class[settings->consoleplayer] = true;

    while (!ready[settings->consoleplayer])
    {
        while (NET_RecvPacket(vcontext, &addr, &packet))
        {
            if (ReadPacketType(packet) == NCMD_SETUP
             && ReadHexenClassPacket(packet, &hpkt)
             && hpkt.player < settings->num_players)
            {
                settings->player_classes[hpkt.player] = hpkt.player_class;
                got_class[hpkt.player] = true;
                ready[hpkt.player] = hpkt.ready;
            }
            NET_ReleaseAddress(addr);
            NET_FreePacket(packet);
        }
        ready[settings->consoleplayer] =
            CountBoolVector(got_class, settings->num_players)
               == settings->num_players;

        hpkt.player = settings->consoleplayer;
        hpkt.player_class = vsettings.player_class;
        hpkt.ready = ready[settings->consoleplayer];

        packet = MakeHexenClassPacket(&hpkt);
        for (i = 0; i < vsettings.num_nodes; ++i)
        {
            NET_SendPacket(vsettings.addrs[i], packet);
        }
        NET_FreePacket(packet);

        // Wait a little bit before sending more packets.
        I_Sleep(250);

        if (callback != NULL
         && !callback(CountBoolVector(ready, settings->num_players),
                      settings->num_players))
        {
            return false;
        }
    }

    return true;
}

// Send game settings to other players and block until the other nodes
// are all ready.
static void SendGameSettings(net_gamesettings_t *settings)
{
    net_packet_t *packet;
    boolean all_started = false;
    int n;

    printf("sending network start info...\n");
    packet = MakeSetupPacket(settings);

    for (;;)
    {
        for (n = 0; n < vsettings.num_nodes; ++n)
        {
            NET_SendPacket(vsettings.addrs[n], packet);
        }

        // Check all other peers have started sending tics.
        NET_VanillaRun();
        all_started = true;
        for (n = 0; n < vsettings.num_nodes; ++n)
        {
            all_started = all_started
                       && (!nodes[n].ingame || nodes[n].latest > 0);
        }
        if (all_started)
        {
            break;
        }

        I_Sleep(1000);
    }
}

// Wait until player 1 sends game settings, and update the given struct.
static void RecvGameSettings(net_gamesettings_t *settings)
{
    net_addr_t *addr;
    net_packet_t *packet;

    printf("listening for network start info...\n");

    for (;;)
    {
        if (!NET_RecvPacket(vcontext, &addr, &packet))
        {
            I_Sleep(100);
            continue;
        }

        // TODO: Confirm sending address is key player
        NET_ReleaseAddress(addr);
        if (ReadPacketType(packet) != NCMD_SETUP)
        {
            continue;
        }
        if (ParseSetupPacket(packet, settings))
        {
            break;
        }

        NET_FreePacket(packet);
    }
}

boolean NET_VanillaSyncSettings(net_gamesettings_t *settings,
                                net_startup_callback_t callback)
{
    settings->new_sync = false;
    settings->consoleplayer = vsettings.consoleplayer;
    settings->num_players = vsettings.num_players;
    extratics = settings->extratics;

    if (vcontext == NULL)
    {
        return false;
    }

    if (vsettings.protocol == NET_VANILLA_PROTO_HEXEN
     && !SyncPlayerClasses(settings, callback))
    {
        return false;
    }

    if (vsettings.consoleplayer == 0)
    {
        SendGameSettings(settings);
    }
    else
    {
        RecvGameSettings(settings);
    }

    return true;
}

static void SendToNode(int node_num, unsigned int starttic,
                       unsigned int numtics)
{
    doomdata_t dd;
    net_packet_t *packet;
    unsigned int i, flags;

    if (numtics > MAX_DOOMDATA_TICS)
    {
        I_Error("SendToNode: tried to send %d > %d tics",
                numtics, MAX_DOOMDATA_TICS);
    }

    dd.starttic = starttic;
    dd.numtics = numtics;
    dd.player = vsettings.consoleplayer;

    for (i = 0; i < numtics; ++i)
    {
        dd.cmds[i] = sendwindow[(starttic + i) % MAX_DOOMDATA_TICS];
    }

    if (nodes[node_num].need_resend)
    {
        nodes[node_num].need_resend = false;
        flags = NCMD_RETRANSMIT;
    }
    else
    {
        flags = 0;
    }

    packet = MakeTicsPacket(&dd, 0);
    NET_SendPacket(vsettings.addrs[node_num], packet);
    NET_FreePacket(packet);
}

void NET_VanillaSendTiccmd(ticcmd_t *ticcmd, int maketic)
{
    unsigned int actual_extratics;
    unsigned int n;

    if (vcontext == NULL || maketic != sent_tics)
    {
        return;
    }

    sendwindow[maketic % MAX_DOOMDATA_TICS] = *ticcmd;
    ++sent_tics;

    // Figure out which tics to send. Usually this is just a single
    // tic (starttic=maketic; numtics=1) but we can include extra
    // tics as insurance against packet loss.
    actual_extratics = extratics;
    if (actual_extratics > maketic)
    {
        actual_extratics = maketic;
    }

    for (n = 0; n < vsettings.num_nodes; ++n)
    {
        if (nodes[n].ingame)
        {
            SendToNode(n, maketic - actual_extratics,
                       1 + actual_extratics);
        }
    }
}

void NET_VanillaQuit(void)
{
    net_packet_t *packet;
    int i, n;

    if (vcontext == NULL)
    {
        return;
    }

    packet = NET_NewPacket(8);
    NET_WriteInt8(packet, 0);
    NET_WriteInt8(packet, 0);
    NET_WriteInt8(packet, vsettings.consoleplayer);
    NET_WriteInt8(packet, 0);
    AddChecksum(packet, NCMD_EXIT);

    // When we quit we send lots of backup packets to make sure the
    // message gets through.
    for (i = 0; i < 4; ++i)
    {
        for (n = 0; n < vsettings.num_nodes; ++n)
        {
            if (nodes[n].ingame)
            {
                NET_SendPacket(vsettings.addrs[n], packet);
            }
        }
    }

    NET_FreePacket(packet);
    vcontext = NULL;
}

static int NodeForAddr(net_addr_t *addr)
{
    unsigned int i;

    for (i = 0; i < vsettings.num_players; ++i)
    {
        if (addr == vsettings.addrs[i])
        {
            return i;
        }
    }
    return -1;
}

static void RetransmitTics(unsigned int node_num,
                           unsigned int retransmitfrom)
{
    unsigned int numtics;

    retransmitfrom = NET_ExpandTicNum(sent_tics, retransmitfrom);
    numtics = sent_tics - retransmitfrom;

    SendToNode(node_num, retransmitfrom, numtics);
}

static void ProcessPacket(net_packet_t *packet, net_addr_t *addr)
{
    doomdata_t dd;
    int n, i, player;
    int packet_type;
    int starttic, tic;

    n = NodeForAddr(addr);
    if (n < 0)
    {
        return;
    }

    packet_type = ReadPacketType(packet);
    switch (packet_type)
    {
        case NCMD_KILL:
            I_Error("Killed by network driver");
            return;
        case NCMD_EXIT:
            nodes[n].ingame = false;
            return;
        case NCMD_SETUP:
            // Don't care at this level.
            return;
        case NCMD_CHECKSUM:
            // Invalid packet checksum. TODO: Log error?
            return;
        default:
            break;
    }

    if (!ReadTicsPacket(packet, &dd) || dd.numtics > MAX_DOOMDATA_TICS)
    {
        return;
    }
    player = dd.player & ~PL_DRONE;
    if (player >= vsettings.num_players)
    {
        return;
    }
    nodes[n].player = player;
    nodes[n].drone = (dd.player & PL_DRONE) != 0;

    if (packet_type == NCMD_RETRANSMIT)
    {
        RetransmitTics(n, dd.retransmitfrom);
    }

    starttic = NET_ExpandTicNum(sent_tics, dd.starttic);
    if (starttic > nodes[n].latest)
    {
        nodes[n].need_resend = true;
        return;
    }
    for (i = 0; i < dd.numtics; ++i)
    {
        tic = starttic + i;
        if (tic == nodes[n].latest)
        {
            nodes[n].recvwindow[tic % MAX_DOOMDATA_TICS] = dd.cmds[i];
            ++nodes[n].latest;
        }
    }
}

static boolean BuildCompleteTic(void)
{
    ticcmd_t ticcmds[NET_MAXPLAYERS];
    boolean playeringame[NET_MAXPLAYERS];
    boolean have[NET_MAXPLAYERS];
    unsigned int i, n, p;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        have[i] = false;
    }
    // We can only proceed once we have received tics from all other nodes
    // in the game. Use the node_t.player field to map nodes to players.
    for (n = 0; n < vsettings.num_nodes; ++n)
    {
        if (nodes[n].ingame && nodes[n].latest < run_tics + 1)
        {
            return false;
        }
        p = nodes[n].player;
        have[p] = true;
        if (nodes[n].drone)
        {
            // ticcmds from drones just get discarded.
        }
        else if (nodes[n].ingame)
        {
            ticcmds[p] = nodes[n].recvwindow[run_tics % MAX_DOOMDATA_TICS];
            playeringame[p] = true;
        }
        else
        {
            memset(&ticcmds[p], 0, sizeof(ticcmd_t));
            playeringame[p] = false;
        }
    }
    // Local player is hard-coded; read out of send window.
    have[vsettings.consoleplayer] = sent_tics > run_tics;
    playeringame[vsettings.consoleplayer] = true;
    ticcmds[vsettings.consoleplayer]
        = sendwindow[run_tics % MAX_DOOMDATA_TICS];

    // Check we have state for all players, and then deliver a tic.
    for (i = 0; i < vsettings.num_players; ++i)
    {
        if (!have[i])
        {
            return false;
        }
    }
    for (; i < NET_MAXPLAYERS; ++i)
    {
        playeringame[i] = false;
    }

    D_ReceiveTic(ticcmds, playeringame);
    ++run_tics;
    return true;
}

void NET_VanillaRun(void)
{
    net_packet_t *packet;
    net_addr_t *addr;

    if (vcontext == NULL)
    {
        return;
    }

    while (NET_RecvPacket(vcontext, &addr, &packet))
    {
        ProcessPacket(packet, addr);
        NET_FreePacket(packet);
        NET_ReleaseAddress(addr);
    }

    while (run_tics < sent_tics)
    {
        if (!BuildCompleteTic())
        {
            break;
        }
    }
}

