//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//     Main loop code.
//

#include <stdlib.h>
#include <string.h>

#include "safe.h"

#include "d_event.h"
#include "d_loop.h"
#include "d_mode.h"
#include "d_ticcmd.h"

#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"

#include "m_argv.h"
#include "m_fixed.h"
#include "m_misc.h"

#include "net_client.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_query.h"
#include "net_server.h"
#include "net_sdl.h"
#include "net_loop.h"
#include "net_vanilla.h"

// The complete set of data for a particular tic.

typedef struct
{
    ticcmd_t cmds[NET_MAXPLAYERS];
    boolean ingame[NET_MAXPLAYERS];
} ticcmd_set_t;

// Maximum time that we wait in TryRunTics() for netgame data to be
// received before we bail out and render a frame anyway.
// Vanilla Doom used 20 for this value, but we use a smaller value
// instead for better responsiveness of the menu when we're stuck.
#define MAX_NETGAME_STALL_TICS  5

//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// recvtic is the latest tic received from the server.
//
// a gametic cannot be run until ticcmds are received for it
// from all players.
//

static ticcmd_set_t ticdata[BACKUPTICS];

// The index of the next tic to be made (with a call to BuildTiccmd).

static int maketic;

// The number of complete tics received from the server so far.

static int recvtic;

// The number of tics that have been run (using RunTic) so far.

int gametic;

// When set to true, a single tic is run each time TryRunTics() is called.
// This is used for -timedemo mode.

boolean singletics = false;

// Index of the local player.

static int localplayer;

// Used for original sync code.

static int      skiptics = 0;

// Reduce the bandwidth needed by sampling game input less and transmitting
// less.  If ticdup is 2, sample half normal, 3 = one third normal, etc.

int		ticdup;

// Amount to offset the timer for game sync.

fixed_t         offsetms;

// Use new client syncronisation code

static boolean  new_sync = true;

// Callback functions for loop code.

static loop_interface_t *loop_interface = NULL;

// Current players in the multiplayer game.
// This is distinct from playeringame[] used by the game code, which may
// modify playeringame[] when playing back multiplayer demos.

static boolean local_playeringame[NET_MAXPLAYERS];

// Requested player class "sent" to the server on connect.
// If we are only doing a single player game then this needs to be remembered
// and saved in the game settings.

static int player_class;

// If true, we're playing a vanilla-compatible peer-to-peer game.
boolean net_vanilla_game;

// 35 fps clock adjusted by offsetms milliseconds

static int GetAdjustedTime(void)
{
    int time_ms;

    time_ms = I_GetTimeMS();

    if (new_sync)
    {
	// Use the adjustments from net_client.c only if we are
	// using the new sync mode.

        time_ms += (offsetms / FRACUNIT);
    }

    return (time_ms * TICRATE) / 1000;
}

static boolean BuildNewTic(void)
{
    int	gameticdiv;
    ticcmd_t cmd;

    gameticdiv = gametic/ticdup;

    I_StartTic ();
    loop_interface->ProcessEvents();

    // Always run the menu

    loop_interface->RunMenu();

    if (drone)
    {
        // In drone mode, do not generate any ticcmds.

        return false;
    }

    if (new_sync)
    {
       // If playing single player, do not allow tics to buffer
       // up very far

       if (!net_client_connected && !net_vanilla_game
        && maketic - gameticdiv > 2)
           return false;

       // Never go more than ~200ms ahead

       if (maketic - gameticdiv > 8)
           return false;
    }
    else
    {
       if (maketic - gameticdiv >= 5)
           return false;
    }

    //printf ("mk:%i ",maketic);
    memset(&cmd, 0, sizeof(ticcmd_t));
    loop_interface->BuildTiccmd(&cmd, maketic);

    if (net_client_connected)
    {
        NET_CL_SendTiccmd(&cmd, maketic);
    }
    if (net_vanilla_game)
    {
        NET_VanillaSendTiccmd(&cmd, maketic);
    }

    ticdata[maketic % BACKUPTICS].cmds[localplayer] = cmd;
    ticdata[maketic % BACKUPTICS].ingame[localplayer] = true;

    ++maketic;

    return true;
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
int      lasttime;

void NetUpdate (void)
{
    int nowtime;
    int newtics;
    int	i;

    // If we are running with singletics (timing a demo), this
    // is all done separately.

    if (singletics)
        return;

    // Run network subsystems
    NET_CL_Run();
    NET_SV_Run();
    NET_VanillaRun();

    // check time
    nowtime = GetAdjustedTime() / ticdup;
    newtics = nowtime - lasttime;

    lasttime = nowtime;

    if (skiptics <= newtics)
    {
        newtics -= skiptics;
        skiptics = 0;
    }
    else
    {
        skiptics -= newtics;
        newtics = 0;
    }

    // build new ticcmds for console player

    for (i=0 ; i<newtics ; i++)
    {
        if (!BuildNewTic())
        {
            break;
        }
    }
}

static void D_Disconnected(void)
{
    // In drone mode, the game cannot continue once disconnected.

    if (drone)
    {
        I_Error("Disconnected from server in drone mode.");
    }

    // disconnected from server

    printf("Disconnected from server.\n");
}

//
// Invoked by the network engine when a complete set of ticcmds is
// available.
//

void D_ReceiveTic(ticcmd_t *ticcmds, boolean *players_mask)
{
    int i;

    // Disconnected from server?

    if (ticcmds == NULL && players_mask == NULL)
    {
        D_Disconnected();
        return;
    }

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (!drone && i == localplayer)
        {
            // This is us.  Don't overwrite it.
        }
        else
        {
            ticdata[recvtic % BACKUPTICS].cmds[i] = ticcmds[i];
            ticdata[recvtic % BACKUPTICS].ingame[i] = players_mask[i];
        }
    }

    ++recvtic;
}

//
// Start game loop
//
// Called after the screen is set but before the game starts running.
//

void D_StartGameLoop(void)
{
    lasttime = GetAdjustedTime() / ticdup;
}

void D_StartNetGame(net_gamesettings_t *settings,
                    net_startup_callback_t callback)
{
    int i;

    offsetms = 0;
    recvtic = 0;

    settings->consoleplayer = 0;
    settings->num_players = 1;
    settings->player_classes[0] = player_class;

    //!
    // @category net
    //
    // Use original network client sync code rather than the improved
    // sync code.
    //
    settings->new_sync = !M_ParmExists("-oldsync");

    //!
    // @category net
    // @arg <n>
    //
    // Send n extra tics in every packet as insurance against dropped
    // packets.
    //

    i = M_CheckParmWithArgs("-extratics", 1);

    if (i > 0)
        settings->extratics = atoi(myargv[i+1]);
    else
        settings->extratics = 1;

    //!
    // @category net
    // @arg <n>
    //
    // Reduce the resolution of the game by a factor of n, reducing
    // the amount of network bandwidth needed.
    //

    i = M_CheckParmWithArgs("-dup", 1);

    if (i > 0)
        settings->ticdup = atoi(myargv[i+1]);
    else
        settings->ticdup = 1;

    if (net_client_connected)
    {
        // Send our game settings and block until game start is received
        // from the server.

        NET_CL_StartGame(settings);
        if (!NET_CL_WaitForStart(callback))
        {
            I_Error("Game start aborted.");
        }

        // Read the game settings that were received.
        NET_CL_GetSettings(settings);
    }
    if (net_vanilla_game && !NET_VanillaSyncSettings(settings, callback))
    {
        I_Error("Game start aborted.");
    }

    if (drone)
    {
        settings->consoleplayer = 0;
    }

    // Set the local player and playeringame[] values.

    localplayer = settings->consoleplayer;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        local_playeringame[i] = i < settings->num_players;
    }

    // Copy settings to global variables.

    ticdup = settings->ticdup;
    new_sync = settings->new_sync;

    if (ticdup < 1)
    {
        I_Error("D_StartNetGame: invalid ticdup value (%d)", ticdup);
    }

    // TODO: Message disabled until we fix new_sync.
    //if (!new_sync)
    //{
    //    printf("Syncing netgames like Vanilla Doom.\n");
    //}
}

static net_vanilla_protocol_t VanillaProtocol(int mission)
{
    switch (mission)
    {
        default:
            I_Error("No vanilla protocol for game mission '%s'.",
                    D_GameMissionString(mission));
        case doom:
        case doom2:
        case pack_tnt:
        case pack_plut:
        case pack_chex:
        case pack_hacx:
            return NET_VANILLA_PROTO_DOOM;
        case heretic:
            return NET_VANILLA_PROTO_HERETIC;
        case hexen:
            return NET_VANILLA_PROTO_HEXEN;
    }
}

// Initialize a peer-to-peer game with the -net command line argument.
static void ParseVanillaNet(int mission, int p)
{
    net_context_t *vcontext;
    net_vanilla_settings_t vsettings;
    int i;

    vcontext = NET_NewContext();
    net_udp_module.InitServer();
    NET_AddModule(vcontext, &net_udp_module);

    vsettings.protocol = VanillaProtocol(mission);
    vsettings.consoleplayer = atoi(myargv[p]);
    vsettings.player_class = player_class;
    vsettings.num_nodes = 0;
    vsettings.num_players = 1;

    ++p;
    for (i = 0; i < NET_MAXPLAYERS && p < myargc; ++i)
    {
        if (M_StringStartsWith(myargv[p], "-"))
        {
            break;
        }
        vsettings.addrs[i] = NET_ResolveAddress(vcontext, myargv[p]);
        if (vsettings.addrs[i] == NULL)
        {
            I_Error("Unable to parse address '%s'", myargv[p]);
        }
        ++p;
        ++vsettings.num_nodes;
        ++vsettings.num_players;
    }

    NET_VanillaInit(vcontext, &vsettings);
    net_vanilla_game = true;
}

// Initialize an IPX game by connecting to a DOSbox IPX server.
static void IPXConnect(int mission, char *address)
{
    net_context_t *context;
    net_vanilla_settings_t settings;
    int want_nodes = 2;
    int p;

    //!
    // @category vnet
    // @arg <n>
    //
    // Number of players when starting a game with the -dbconnect command
    // line argument. If this is not provided then a two-player game is
    // started by default.
    //
    p = M_CheckParmWithArgs("-nodes", 1);
    if (p > 0)
    {
        want_nodes = atoi(myargv[p + 1]);
    }

    context = NET_DBIPX_Connect(address);
    settings.player_class = player_class;
    settings.protocol = VanillaProtocol(mission);
    NET_DBIPX_ArbitrateGame(&settings, want_nodes);
    NET_VanillaInit(context, &settings);
    net_vanilla_game = true;
}

// Initialize a serial game connecting to a DOSBox modem server.
static void SerialDial(int mission, char *address)
{
    net_context_t *context;
    net_vanilla_settings_t settings;

    context = NET_Serial_Connect(address);
    settings.player_class = player_class;
    settings.protocol = VanillaProtocol(mission);
    NET_Serial_ArbitrateGame(context, &settings);
    NET_VanillaInit(context, &settings);
    net_vanilla_game = true;
}

// Initialize a serial game listening for a connection from a DOSBox
// emulated "modem".
static void SerialAnswer(int mission)
{
    net_context_t *context;
    net_vanilla_settings_t settings;

    context = NET_Serial_Answer();
    settings.protocol = VanillaProtocol(mission);
    settings.player_class = player_class;
    NET_Serial_ArbitrateGame(context, &settings);
    NET_VanillaInit(context, &settings);
    net_vanilla_game = true;
}

boolean D_InitNetGame(net_connect_data_t *connect_data)
{
    boolean result = false;
    net_addr_t *addr = NULL;
    int i;

    // Call D_QuitNetGame on exit:
    I_AtExit(D_QuitNetGame, true);

    player_class = connect_data->player_class;

    i = M_CheckParmWithArgs("-net", 2);
    if (i > 0)
    {
        ParseVanillaNet(connect_data->gamemission, i + 1);
        return true;
    }

    //!
    // @category vnet
    // @arg <address:port>
    //
    // Connect to a DOSBox IPX server, to play a multiplayer game against
    // vanilla Doom with ipxsetup.exe.
    //

    i = M_CheckParmWithArgs("-dbconnect", 1);
    if (i > 0)
    {
        IPXConnect(connect_data->gamemission, myargv[i + 1]);
        return true;
    }

    //!
    // @category vnet
    //
    // Listen for a TCP connection ("call") from a DOSBox emulated modem,
    // to play a multiplayer game against vanilla Doom with sersetup.exe.

    if (M_ParmExists("-dbanswer"))
    {
        SerialAnswer(connect_data->gamemission);
        return true;
    }

    //!
    // @category vnet
    // @arg <address:port>
    //
    // Connect to a DOSBox virtual modem, to play a multiplayer game against
    // vanilla Doom with sersetup.exe.

    i = M_CheckParmWithArgs("-dbdial", 1);
    if (i > 0)
    {
        SerialDial(connect_data->gamemission, myargv[i + 1]);
        return true;
    }

    //!
    // @category net
    //
    // Start a multiplayer server, listening for connections.
    //

    if (M_CheckParm("-server") > 0
     || M_CheckParm("-privateserver") > 0)
    {
        NET_SV_Init();
        NET_SV_AddModule(&net_loop_server_module);
        NET_SV_AddModule(&net_udp_module);
        NET_SV_RegisterWithMaster();

        net_loop_client_module.InitClient();
        addr = net_loop_client_module.ResolveAddress(NULL);
        NET_ReferenceAddress(addr);
    }
    else
    {
        //!
        // @category net
        //
        // Automatically search the local LAN for a multiplayer
        // server and join it.
        //

        i = M_CheckParm("-autojoin");

        if (i > 0)
        {
            addr = NET_FindLANServer();

            if (addr == NULL)
            {
                I_Error("No server found on local LAN");
            }
        }

        //!
        // @arg <address>
        // @category net
        //
        // Connect to a multiplayer server running on the given
        // address.
        //

        i = M_CheckParmWithArgs("-connect", 1);

        if (i > 0)
        {
            net_udp_module.InitClient();
            addr = net_udp_module.ResolveAddress(myargv[i+1]);
            NET_ReferenceAddress(addr);

            if (addr == NULL)
            {
                I_Error("Unable to resolve '%s'\n", myargv[i+1]);
            }
        }
    }

    if (addr != NULL)
    {
        if (M_CheckParm("-drone") > 0)
        {
            connect_data->drone = true;
        }

        if (!NET_CL_Connect(addr, connect_data))
        {
            I_Error("D_InitNetGame: Failed to connect to %s:\n%s\n",
                    NET_AddrToString(addr), net_client_reject_reason);
        }

        printf("D_InitNetGame: Connected to %s\n", NET_AddrToString(addr));
        NET_ReleaseAddress(addr);

        // Wait for launch message received from server.

        NET_WaitForLaunch();

        result = true;
    }

    return result;
}


//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame (void)
{
    NET_SV_Shutdown();
    NET_CL_Disconnect();
    NET_VanillaQuit();
}

static int GetLowTic(void)
{
    int lowtic;

    lowtic = maketic;

    if (net_client_connected || net_vanilla_game)
    {
        if (drone || recvtic < lowtic)
        {
            lowtic = recvtic;
        }
    }

    return lowtic;
}

static int frameon;
static int frameskip[4];
static int oldnettics;

static void OldNetSync(void)
{
    unsigned int i;
    int keyplayer = -1;

    frameon++;

    // ideally maketic should be 1 - 3 tics above lowtic
    // if we are consistantly slower, speed up time

    for (i=0 ; i<NET_MAXPLAYERS ; i++)
    {
        if (local_playeringame[i])
        {
            keyplayer = i;
            break;
        }
    }

    if (keyplayer < 0)
    {
        // If there are no players, we can never advance anyway

        return;
    }

    if (localplayer == keyplayer)
    {
        // the key player does not adapt
    }
    else
    {
        if (maketic <= recvtic)
        {
            lasttime--;
            // printf ("-");
        }

        frameskip[frameon & 3] = oldnettics > recvtic;
        oldnettics = maketic;

        if (frameskip[0] && frameskip[1] && frameskip[2] && frameskip[3])
        {
            skiptics = 1;
            // printf ("+");
        }
    }
}

// Returns true if there are players in the game:

static boolean PlayersInGame(void)
{
    boolean result = false;
    unsigned int i;

    // If we are connected to a server, check if there are any players
    // in the game.

    if (net_client_connected || net_vanilla_game)
    {
        for (i = 0; i < NET_MAXPLAYERS; ++i)
        {
            result = result || local_playeringame[i];
        }
    }

    // Whether single or multi-player, unless we are running as a drone,
    // we are in the game.

    if (!drone)
    {
        result = true;
    }

    return result;
}

// When using ticdup, certain values must be cleared out when running
// the duplicate ticcmds.

static void TicdupSquash(ticcmd_set_t *set)
{
    ticcmd_t *cmd;
    unsigned int i;

    for (i = 0; i < NET_MAXPLAYERS ; ++i)
    {
        cmd = &set->cmds[i];
        cmd->chatchar = 0;
        if (cmd->buttons & BT_SPECIAL)
            cmd->buttons = 0;
    }
}

// When running in single player mode, clear all the ingame[] array
// except the local player.

static void SinglePlayerClear(ticcmd_set_t *set)
{
    unsigned int i;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (i != localplayer)
        {
            set->ingame[i] = false;
        }
    }
}

//
// TryRunTics
//

void TryRunTics (void)
{
    int	i;
    int	lowtic;
    int	entertic;
    static int oldentertics;
    int realtics;
    int	availabletics;
    int	counts;

    // get real tics
    entertic = I_GetTime() / ticdup;
    realtics = entertic - oldentertics;
    oldentertics = entertic;

    // in singletics mode, run a single tic every time this function
    // is called.

    if (singletics)
    {
        BuildNewTic();
    }
    else
    {
        NetUpdate ();
    }

    lowtic = GetLowTic();

    availabletics = lowtic - gametic/ticdup;

    // decide how many tics to run

    if (new_sync)
    {
	counts = availabletics;
    }
    else
    {
        // decide how many tics to run
        if (realtics < availabletics-1)
            counts = realtics+1;
        else if (realtics < availabletics)
            counts = realtics;
        else
            counts = availabletics;

        if (counts < 1)
            counts = 1;

        if (net_client_connected || net_vanilla_game)
        {
            OldNetSync();
        }
    }

    if (counts < 1)
	counts = 1;

    // wait for new tics if needed
    while (!PlayersInGame() || lowtic < gametic/ticdup + counts)
    {
	NetUpdate ();

        lowtic = GetLowTic();

	if (lowtic < gametic/ticdup)
	    I_Error ("TryRunTics: lowtic < gametic");

        // Still no tics to run? Sleep until some are available.
        if (lowtic < gametic/ticdup + counts)
        {
            // If we're in a netgame, we might spin forever waiting for
            // new network data to be received. So don't stay in here
            // forever - give the menu a chance to work.
            if (I_GetTime() / ticdup - entertic >= MAX_NETGAME_STALL_TICS)
            {
                return;
            }

            I_Sleep(1);
        }
    }

    // run the count * ticdup dics
    while (counts--)
    {
        ticcmd_set_t *set;

        if (!PlayersInGame())
        {
            return;
        }

        set = &ticdata[(gametic / ticdup) % BACKUPTICS];

        if (!net_client_connected && !net_vanilla_game)
        {
            SinglePlayerClear(set);
        }

	for (i=0 ; i<ticdup ; i++)
	{
            if (gametic/ticdup > lowtic)
                I_Error ("gametic>lowtic");

            memcpy(local_playeringame, set->ingame, sizeof(local_playeringame));

            loop_interface->RunTic(set->cmds, set->ingame);
	    gametic++;

	    // modify command for duplicated tics

            TicdupSquash(set);
	}

	NetUpdate ();	// check for new console commands
    }
}

void D_RegisterLoopCallbacks(loop_interface_t *i)
{
    loop_interface = i;
}

// TODO: Move nonvanilla demo functions into a dedicated file.
#include "m_misc.h"
#include "w_wad.h"

static boolean StrictDemos(void)
{
    //!
    // @category demo
    //
    // When recording or playing back demos, disable any extensions
    // of the vanilla demo format - record demos as vanilla would do,
    // and play back demos as vanilla would do.
    //
    return M_ParmExists("-strictdemos");
}

// If the provided conditional value is true, we're trying to record
// a demo file that will include a non-vanilla extension. The function
// will return true if the conditional is true and it's allowed to use
// this extension (no extensions are allowed if -strictdemos is given
// on the command line). A warning is shown on the console using the
// provided string describing the non-vanilla expansion.
boolean D_NonVanillaRecord(boolean conditional, const char *feature)
{
    if (!conditional || StrictDemos())
    {
        return false;
    }

    printf("Warning: Recording a demo file with a non-vanilla extension "
           "(%s). Use -strictdemos to disable this extension.\n",
           feature);

    return true;
}

// Returns true if the given lump number corresponds to data from a .lmp
// file, as opposed to a WAD.
static boolean IsDemoFile(int lumpnum)
{
    char *lower;
    boolean result;

    lower = X_StringDuplicate(lumpinfo[lumpnum]->wad_file->path);
    M_ForceLowercase(lower);
    result = M_StringEndsWith(lower, ".lmp");
    free(lower);

    return result;
}

// If the provided conditional value is true, we're trying to play back
// a demo that includes a non-vanilla extension. We return true if the
// conditional is true and it's allowed to use this extension, checking
// that:
//  - The -strictdemos command line argument is not provided.
//  - The given lumpnum identifying the demo to play back identifies a
//    demo that comes from a .lmp file, not a .wad file.
//  - Before proceeding, a warning is shown to the user on the console.
boolean D_NonVanillaPlayback(boolean conditional, int lumpnum,
                             const char *feature)
{
    if (!conditional || StrictDemos())
    {
        return false;
    }

    if (!IsDemoFile(lumpnum))
    {
        printf("Warning: WAD contains demo with a non-vanilla extension "
               "(%s)\n", feature);
        return false;
    }

    printf("Warning: Playing back a demo file with a non-vanilla extension "
           "(%s). Use -strictdemos to disable this extension.\n",
           feature);

    return true;
}

