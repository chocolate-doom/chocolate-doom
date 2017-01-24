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
//	DOOM Network game communication and protocol,
//	all OS independend parts.
//

#include <stdlib.h>

#include "m_argv.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_videohr.h"
#include "h2def.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "w_checksum.h"

#include "deh_main.h"

#include "d_loop.h"

ticcmd_t *netcmds;

extern void H2_DoAdvanceDemo(void);
extern void H2_ProcessEvents(void);
extern void G_BuildTiccmd(ticcmd_t *cmd, int maketic);
extern boolean G_CheckDemoStatus(void);

extern boolean demorecording;

// Called when a player leaves the game

static void PlayerQuitGame(player_t *player)
{
    static char exitmsg[80];
    unsigned int player_num;

    player_num = player - players;

    M_StringCopy(exitmsg, "PLAYER 1 LEFT THE GAME", sizeof(exitmsg));
    exitmsg[7] += player_num;
    P_SetMessage(&players[consoleplayer], exitmsg, true);
    S_StartSound(NULL, SFX_CHAT);

    playeringame[player_num] = false;

    // TODO: check if it is sensible to do this:

    if (demorecording) 
    {
        G_CheckDemoStatus ();
    }
}

static void RunTic(ticcmd_t *cmds, boolean *ingame)
{
    extern boolean advancedemo;
    unsigned int i;

    // Check for player quits.

    for (i = 0; i < maxplayers; ++i)
    {
        if (!demoplayback && playeringame[i] && !ingame[i])
        {
            PlayerQuitGame(&players[i]);
        }
    }

    netcmds = cmds;

    // check that there are players in the game.  if not, we cannot
    // run a tic.

    if (advancedemo)
        H2_DoAdvanceDemo ();

    G_Ticker ();
}

static loop_interface_t hexen_loop_interface = {
    H2_ProcessEvents,
    G_BuildTiccmd,
    RunTic,
    MN_Ticker
};


// Load game settings from the specified structure and 
// set global variables.

static void LoadGameSettings(net_gamesettings_t *settings)
{
    unsigned int i;

    deathmatch = settings->deathmatch;
    ticdup = settings->ticdup;
    startepisode = settings->episode;
    startmap = settings->map;
    startskill = settings->skill;
    // TODO startloadgame = settings->loadgame;
    lowres_turn = settings->lowres_turn;
    nomonsters = settings->nomonsters;
    respawnparm = settings->respawn_monsters;
    consoleplayer = settings->consoleplayer;

    if (lowres_turn)
    {
        printf("NOTE: Turning resolution is reduced; this is probably "
               "because there is a client recording a Vanilla demo.\n");
    }

    for (i=0; i<maxplayers; ++i)
    {
        playeringame[i] = i < settings->num_players;
        PlayerClass[i] = settings->player_classes[i];

        if (PlayerClass[i] >= NUMCLASSES)
        {
            PlayerClass[i] = PCLASS_FIGHTER;
        }
    }
}

// Save the game settings from global variables to the specified
// game settings structure.

static void SaveGameSettings(net_gamesettings_t *settings)
{
    // jhaley 20120715: Some parts of the structure are being left
    // uninitialized. If -class is not used on the command line, this
    // can lead to a crash in SB_Init due to player class == 0xCCCCCCCC.
    memset(settings, 0, sizeof(*settings));

    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings->deathmatch = deathmatch;
    settings->episode = startepisode;
    settings->map = startmap;
    settings->skill = startskill;
    // TODO settings->loadgame = startloadgame;
    settings->gameversion = exe_hexen_1_1;
    settings->nomonsters = nomonsters;
    settings->respawn_monsters = respawnparm;
    settings->timelimit = 0;

    settings->lowres_turn = M_ParmExists("-record")
                         && !M_ParmExists("-longtics");
}

static void InitConnectData(net_connect_data_t *connect_data)
{
    int i;

    //
    // Connect data
    //

    // Game type fields:

    connect_data->gamemode = gamemode;
    connect_data->gamemission = hexen;

    // Are we recording a demo? Possibly set lowres turn mode

    connect_data->lowres_turn = M_ParmExists("-record")
                             && !M_ParmExists("-longtics");

    connect_data->drone = false;
    connect_data->max_players = maxplayers;

    //!
    // @category net
    // @arg <n>
    //
    // Specify player class: 0=fighter, 1=cleric, 2=mage, 3=pig.
    //

    i = M_CheckParmWithArgs("-class", 1);

    if (i > 0)
    {
        connect_data->player_class = atoi(myargv[i + 1]);
    }
    else
    {
        connect_data->player_class = PCLASS_FIGHTER;
    }

    // Read checksums of our WAD directory and dehacked information

    W_Checksum(connect_data->wad_sha1sum);
    memset(connect_data->deh_sha1sum, 0, sizeof(sha1_digest_t));

    connect_data->is_freedoom = 0;
}

void D_ConnectNetGame(void)
{
    net_connect_data_t connect_data;

    InitConnectData(&connect_data);
    netgame = D_InitNetGame(&connect_data);

    //!
    // @category net
    //
    // Start the game playing as though in a netgame with a single
    // player.  This can also be used to play back single player netgame
    // demos.
    //

    if (M_CheckParm("-solo-net") > 0)
    {
        netgame = true;
    }
}

static boolean StartupProgress(int now_ready, int total)
{
    static int ready = 0;

    while (ready < now_ready)
    {
        ST_NetProgress();
        ++ready;
    }

    ready = now_ready;

    // Allow the user to hit escape during netgame startup to abort.
    return !I_CheckAbortHR();
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//

void D_CheckNetGame(void)
{
    net_gamesettings_t settings;

    D_RegisterLoopCallbacks(&hexen_loop_interface);

    if (netgame)
    {
        autostart = true;
    }

    SaveGameSettings(&settings);
    D_StartNetGame(&settings, StartupProgress);
    LoadGameSettings(&settings);

    // Finish netgame progress on startup screen.

    if (netgame)
    {
        StartupProgress(settings.num_players, settings.num_players);
        ST_NetDone();
    }
}

//==========================================================================
//
// NET_SendFrags
//
//==========================================================================

void NET_SendFrags(player_t * player)
{
    // Not sure what this is intended for. Unused?
}

