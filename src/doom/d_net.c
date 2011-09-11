// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//	DOOM Network game communication and protocol,
//	all OS independend parts.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "doomfeatures.h"

#include "d_main.h"
#include "m_argv.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"

#include "deh_main.h"

#include "net_client.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_query.h"
#include "net_server.h"
#include "net_sdl.h"
#include "net_loop.h"

// The complete set of data for a particular tic.

typedef struct
{
    ticcmd_t cmds[MAXPLAYERS];
    boolean ingame[MAXPLAYERS];
} ticcmd_set_t;

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// recvtic is the latest tic received from the server.
//
// a gametic cannot be run until ticcmds are received for it
// from all players.
//

ticcmd_set_t ticdata[BACKUPTICS];
ticcmd_t *netcmds;

int             maketic;
int             recvtic;

// Used for original sync code.

static int      skiptics = 0;

// Reduce the bandwidth needed by sampling game input less and transmitting
// less.  If ticdup is 2, sample half normal, 3 = one third normal, etc.

int		ticdup;

// Send this many extra (backup) tics in each packet.

int             extratics;

// Amount to offset the timer for game sync.

fixed_t         offsetms;

// Use new client syncronisation code

boolean         new_sync = true;

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
    int	gameticdiv;

    // If we are running with singletics (timing a demo), this
    // is all done separately.

    if (singletics)
        return;
    
#ifdef FEATURE_MULTIPLAYER

    // Run network subsystems

    NET_CL_Run();
    NET_SV_Run();

#endif

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
    gameticdiv = gametic/ticdup;

    for (i=0 ; i<newtics ; i++)
    {
        ticcmd_t cmd;

	I_StartTic ();
	D_ProcessEvents ();

        // Always run the menu

        M_Ticker ();

        if (drone)
        {
            // In drone mode, do not generate any ticcmds.

            continue;
        }
	
        if (new_sync)
        { 
           // If playing single player, do not allow tics to buffer
           // up very far

           if ((!netgame || demoplayback) && maketic - gameticdiv > 2)
               break;

           // Never go more than ~200ms ahead

           if (maketic - gameticdiv > 8)
               break;
        }
	else
	{
           if (maketic - gameticdiv >= 5)
               break;
	}

	//printf ("mk:%i ",maketic);
	G_BuildTiccmd(&cmd);

#ifdef FEATURE_MULTIPLAYER

        if (net_client_connected)
        {
            NET_CL_SendTiccmd(&cmd, maketic);
        }

#endif
        ticdata[maketic % BACKUPTICS].cmds[consoleplayer] = cmd;
        ticdata[maketic % BACKUPTICS].ingame[consoleplayer] = true;

	++maketic;
    }
}

// Called when a player leaves the game

static void D_PlayerQuitGame(player_t *player)
{
    static char exitmsg[80];
    unsigned int player_num;

    player_num = player - players;

    // Do this the same way as Vanilla Doom does, to allow dehacked
    // replacements of this message

    strncpy(exitmsg, DEH_String("Player 1 left the game"), sizeof(exitmsg));
    exitmsg[sizeof(exitmsg) - 1] = '\0';

    exitmsg[7] += player_num;

    playeringame[player_num] = false;
    players[consoleplayer].message = exitmsg;

    // TODO: check if it is sensible to do this:

    if (demorecording) 
    {
        G_CheckDemoStatus ();
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

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        if (!drone && i == consoleplayer)
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

// Load game settings from the specified structure and 
// set global variables.

static void LoadGameSettings(net_gamesettings_t *settings)
{
    unsigned int i;

    deathmatch = settings->deathmatch;
    ticdup = settings->ticdup;
    extratics = settings->extratics;
    startepisode = settings->episode;
    startmap = settings->map;
    startskill = settings->skill;
    startloadgame = settings->loadgame;
    lowres_turn = settings->lowres_turn;
    nomonsters = settings->nomonsters;
    fastparm = settings->fast_monsters;
    respawnparm = settings->respawn_monsters;
    timelimit = settings->timelimit;

    if (lowres_turn)
    {
        printf("NOTE: Turning resolution is reduced; this is probably "
               "because there is a client recording a Vanilla demo.\n");
    }

    new_sync = settings->new_sync;

    if (new_sync == false)
    {
	printf("Syncing netgames like Vanilla Doom.\n");
    }

    if (!drone)
    {
        consoleplayer = settings->consoleplayer;
    }
    else
    {
        consoleplayer = 0;
    }
    
    for (i=0; i<MAXPLAYERS; ++i) 
    {
        playeringame[i] = i < settings->num_players;
    }
}

// Save the game settings from global variables to the specified
// game settings structure.

static void SaveGameSettings(net_gamesettings_t *settings,
                             net_connect_data_t *connect_data)
{
    int i;

    // Fill in game settings structure with appropriate parameters
    // for the new game

    settings->deathmatch = deathmatch;
    settings->episode = startepisode;
    settings->map = startmap;
    settings->skill = startskill;
    settings->loadgame = startloadgame;
    settings->gameversion = gameversion;
    settings->nomonsters = nomonsters;
    settings->fast_monsters = fastparm;
    settings->respawn_monsters = respawnparm;
    settings->timelimit = timelimit;

    settings->lowres_turn = M_CheckParm("-record") > 0
                         && M_CheckParm("-longtics") == 0;

    //!
    // @category net
    //
    // Use original game sync code.
    //

    if (M_CheckParm("-oldsync") > 0)
	settings->new_sync = 0;
    else
	settings->new_sync = 1;
    
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

    //
    // Connect data
    //

    // Game type fields:

    connect_data->gamemode = gamemode;
    connect_data->gamemission = gamemission;

    // Drone mode?

    connect_data->drone = M_CheckParm("-drone") > 0;

    // Are we recording a demo? Possibly set lowres turn mode

    connect_data->lowres_turn = settings->lowres_turn;
}

void D_InitSinglePlayerGame(net_gamesettings_t *settings)
{
    // default values for single player

    settings->consoleplayer = 0;
    settings->num_players = 1;

    netgame = false;

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

boolean D_InitNetGame(net_connect_data_t *connect_data,
                      net_gamesettings_t *settings)
{
    net_addr_t *addr = NULL;
    int i;


#ifdef FEATURE_MULTIPLAYER

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
        NET_SV_AddModule(&net_sdl_module);
        NET_SV_RegisterWithMaster();

        net_loop_client_module.InitClient();
        addr = net_loop_client_module.ResolveAddress(NULL);
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
            net_sdl_module.InitClient();
            addr = net_sdl_module.ResolveAddress(myargv[i+1]);

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

        //!
        // @category net
        //
        // Run as the left screen in three screen mode.
        //

        if (M_CheckParm("-left") > 0)
        {
            viewangleoffset = ANG90;
            connect_data->drone = true;
        }

        //! 
        // @category net
        //
        // Run as the right screen in three screen mode.
        //

        if (M_CheckParm("-right") > 0)
        {
            viewangleoffset = ANG270;
            connect_data->drone = true;
        }

        if (!NET_CL_Connect(addr, connect_data))
        {
            I_Error("D_CheckNetGame: Failed to connect to %s\n", 
                    NET_AddrToString(addr));
        }

        printf("D_CheckNetGame: Connected to %s\n", NET_AddrToString(addr));

        // Wait for game start message received from server.

        NET_WaitForStart(settings);

        // Read the game settings that were received.

        NET_CL_GetSettings(settings);

        return true;
    }

#endif

    return false;
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
extern	int			viewangleoffset;

void D_CheckNetGame (void)
{
    net_connect_data_t connect_data;
    net_gamesettings_t settings;

    offsetms = 0;
    recvtic = 0;

    // Call D_QuitNetGame on exit 

    I_AtExit(D_QuitNetGame, true);

    SaveGameSettings(&settings, &connect_data);

    if (D_InitNetGame(&connect_data, &settings))
    {
        netgame = true;
        autostart = true;
    }
    else
    {
        D_InitSinglePlayerGame(&settings);
    }

    LoadGameSettings(&settings);

    DEH_printf("startskill %i  deathmatch: %i  startmap: %i  startepisode: %i\n",
               startskill, deathmatch, startmap, startepisode);
	
    DEH_printf("player %i of %i (%i nodes)\n",
               consoleplayer+1, settings.num_players, settings.num_players);

    // Show players here; the server might have specified a time limit

    if (timelimit > 0 && deathmatch)
    {
        // Gross hack to work like Vanilla:

        if (timelimit == 20 && M_CheckParm("-avg"))
        {
            DEH_printf("Austin Virtual Gaming: Levels will end "
                           "after 20 minutes\n");
        }
        else
        {
            DEH_printf("Levels will end after %d minute", timelimit);
            if (timelimit > 1)
                printf("s");
            printf(".\n");
        }
    }
}


//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame (void)
{
#ifdef FEATURE_MULTIPLAYER

    NET_SV_Shutdown();
    NET_CL_Disconnect();

#endif

}

static int GetLowTic(void)
{
    int lowtic;

    lowtic = maketic;

#ifdef FEATURE_MULTIPLAYER
    if (net_client_connected)
    {
        if (drone || recvtic < lowtic)
        {
            lowtic = recvtic;
        }
    }
#endif

    return lowtic;
}

int	frametics[4];
int	frameon;
int	frameskip[4];
int	oldnettics;

static void OldNetSync(void)
{
    unsigned int i;
    unsigned int keyplayer = -1;

    frameon++;

    // ideally maketic should be 1 - 3 tics above lowtic
    // if we are consistantly slower, speed up time

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // TODO: playeringame should not be used here.

        if (playeringame[i])
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

    if (consoleplayer == keyplayer)
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

    if (net_client_connected)
    {
        for (i = 0; i < MAXPLAYERS; ++i)
        {
            result = result || playeringame[i];
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
                    
    for (i = 0; i < MAXPLAYERS ; ++i)
    {
        cmd = &set->cmds[i];
        cmd->chatchar = 0;
        if (cmd->buttons & BT_SPECIAL)
            cmd->buttons = 0;
    }
}

static void D_RunTic(ticcmd_set_t *set)
{
    extern boolean advancedemo;
    unsigned int i;

    // Check for player quits.

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        if (playeringame[i] && !set->ingame[i])
        {
            D_PlayerQuitGame(&players[i]);
        }
    }

    netcmds = set->cmds;

    // check that there are players in the game.  if not, we cannot
    // run a tic.

    if (advancedemo)
        D_DoAdvanceDemo ();

    G_Ticker ();
}

// When running in single player mode, clear all the ingame[] array
// except the consoleplayer.

static void SinglePlayerClear(ticcmd_set_t *set)
{
    unsigned int i;

    for (i = 0; i < MAXPLAYERS; ++i)
    {
        if (i != consoleplayer)
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
    
    // get available tics
    NetUpdate ();
	
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
                    
        if (net_client_connected)
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
    
        // Don't stay in this loop forever.  The menu is still running,
        // so return to update the screen

	if (I_GetTime() / ticdup - entertic > 0)
	{
	    return;
	} 

        I_Sleep(1);
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

        if (!net_client_connected)
        {
            SinglePlayerClear(set);
        }

	for (i=0 ; i<ticdup ; i++)
	{
            if (gametic/ticdup > lowtic)
                I_Error ("gametic>lowtic");

            D_RunTic(set);
	    gametic++;
	    
	    // modify command for duplicated tics

            TicdupSquash(set);
	}

	NetUpdate ();	// check for new console commands
    }
}

