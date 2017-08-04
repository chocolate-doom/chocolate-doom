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

#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "doomdef.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_checksum.h"

#include "deh_main.h"

#include "d_loop.h"

ticcmd_t *netcmds;

extern void D_DoAdvanceDemo(void);
extern void D_ProcessEvents(void);
extern void G_BuildTiccmd(ticcmd_t *cmd, int maketic);
extern boolean G_CheckDemoStatus(void);

// Called when a player leaves the game

static void PlayerQuitGame(player_t *player)
{
    static char exitmsg[80];
    unsigned int player_num;

    player_num = player - players;

    // Note:
    // The Heretic source code does this, which doesn't actually work.
    // As a result, the exit message is never seen.

    M_StringCopy(exitmsg, "PLAYER 1 LEFT THE GAME", sizeof(exitmsg));
    exitmsg[7] += player_num;
    players[consoleplayer].message = exitmsg;

    playeringame[player_num] = false;
    players[consoleplayer].message = exitmsg;

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

    for (i = 0; i < MAXPLAYERS; ++i)
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
        D_DoAdvanceDemo ();

    G_Ticker ();
}

static loop_interface_t doom_loop_interface = {
    D_ProcessEvents,
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

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        playeringame[i] = i < settings->num_players;
    }
}

// Save the game settings from global variables to the specified
// game settings structure.

static void SaveGameSettings(net_gamesettings_t *settings)
{
    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings->deathmatch = deathmatch;
    settings->episode = startepisode;
    settings->map = startmap;
    settings->skill = startskill;
    // TODO settings->loadgame = startloadgame;
    settings->gameversion = exe_heretic_1_3;
    settings->nomonsters = nomonsters;
    settings->respawn_monsters = respawnparm;
    settings->timelimit = 0;

    settings->lowres_turn = M_ParmExists("-record")
                         && !M_ParmExists("-longtics");
}

static void InitConnectData(net_connect_data_t *connect_data)
{
    connect_data->drone = false;
    connect_data->max_players = MAXPLAYERS;

    //
    // Connect data
    //

    // Game type fields:

    connect_data->gamemode = gamemode;
    connect_data->gamemission = heretic;

    // Are we recording a demo? Possibly set lowres turn mode

    connect_data->lowres_turn = M_ParmExists("-record")
                             && !M_ParmExists("-longtics");

    // Read checksums of our WAD directory and dehacked information

    W_Checksum(connect_data->wad_sha1sum);
    DEH_Checksum(connect_data->deh_sha1sum);

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

//
// D_CheckNetGame
// Works out player numbers among the net participants
//

void D_CheckNetGame (void)
{
    net_gamesettings_t settings;

    D_RegisterLoopCallbacks(&doom_loop_interface);

    if (netgame)
    {
        autostart = true;
    }

    SaveGameSettings(&settings);
    D_StartNetGame(&settings, NULL);
    LoadGameSettings(&settings);
}

