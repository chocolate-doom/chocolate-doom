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


//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tick that hasn't had control made for it yet
// nettics[] has the maketics for all players 
//
// a gametic cannot be run until nettics[] > gametic for all players
//

ticcmd_t        netcmds[MAXPLAYERS][BACKUPTICS];
int         	nettics[MAXPLAYERS];

int             maketic;

// Used for original sync code.

int		lastnettic;
int             skiptics = 0;

// Reduce the bandwidth needed by sampling game input less and transmitting
// less.  If ticdup is 2, sample half normal, 3 = one third normal, etc.

int		ticdup;

// Send this many extra (backup) tics in each packet.

int             extratics;

// Amount to offset the timer for game sync.

fixed_t         offsetms;

// Use new client syncronisation code

boolean         net_cl_new_sync = true;

// Connected but not participating in the game (observer)

boolean drone = false;

// 35 fps clock adjusted by offsetms milliseconds

static int GetAdjustedTime(void)
{
    int time_ms;

    time_ms = I_GetTimeMS();

    if (net_cl_new_sync)
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
	
        if (net_cl_new_sync)
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
        netcmds[consoleplayer][maketic % BACKUPTICS] = cmd;

	++maketic;
        nettics[consoleplayer] = maketic;
    }
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


//
// D_CheckNetGame
// Works out player numbers among the net participants
//
extern	int			viewangleoffset;

void D_CheckNetGame (void)
{
    int i;
    int num_players;

    // default values for single player

    consoleplayer = 0;
    netgame = false;
    ticdup = 1;
    extratics = 1;
    lowres_turn = false;
    offsetms = 0;
    
    for (i=0; i<MAXPLAYERS; i++)
    {
        playeringame[i] = false;
       	nettics[i] = 0;
    }

    playeringame[0] = true;

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

#ifdef FEATURE_MULTIPLAYER

    {
        net_addr_t *addr = NULL;

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
                drone = true;
            }

            //!
            // @category net
            //
            // Run as the left screen in three screen mode.
            //

            if (M_CheckParm("-left") > 0)
            {
                viewangleoffset = ANG90;
                drone = true;
            }

            //! 
            // @category net
            //
            // Run as the right screen in three screen mode.
            //

            if (M_CheckParm("-right") > 0)
            {
                viewangleoffset = ANG270;
                drone = true;
            }

            if (!NET_CL_Connect(addr))
            {
                I_Error("D_CheckNetGame: Failed to connect to %s\n", 
                        NET_AddrToString(addr));
            }

            printf("D_CheckNetGame: Connected to %s\n", NET_AddrToString(addr));

            NET_WaitForStart();
        }
    }

#endif

    num_players = 0;

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (playeringame[i])
            ++num_players;
    }

    DEH_printf("startskill %i  deathmatch: %i  startmap: %i  startepisode: %i\n",
               startskill, deathmatch, startmap, startepisode);
	
    DEH_printf("player %i of %i (%i nodes)\n",
               consoleplayer+1, num_players, num_players);

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

// Returns true if there are currently any players in the game.

static boolean PlayersInGame(void)
{
    int i;

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (playeringame[i])
        {
            return true;
        }
    }

    return false;
}

static int GetLowTic(void)
{
    int i;
    int lowtic;

#ifdef FEATURE_MULTIPLAYER
    if (net_client_connected)
    {
        lowtic = INT_MAX;
    
        for (i=0; i<MAXPLAYERS; ++i)
        {
            if (playeringame[i])
            {
                if (nettics[i] < lowtic)
                    lowtic = nettics[i];
            }
        }
    }
    else
#endif
    {
        lowtic = maketic;
    }

    return lowtic;
}

//
// TryRunTics
//
int	oldnettics;
int	frametics[4];
int	frameon;
int	frameskip[4];
int	oldnettics;

extern	boolean	advancedemo;

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
    
    if (net_cl_new_sync)
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
                    
        frameon++;

        if (!demoplayback)
        {
	    int keyplayer = -1;

            // ideally maketic should be 1 - 3 tics above lowtic
            // if we are consistantly slower, speed up time

            for (i=0 ; i<MAXPLAYERS ; i++)
	    {
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
                if (maketic <= nettics[keyplayer])
                {
                    lasttime--;
                    // printf ("-");
                }

                frameskip[frameon & 3] = (oldnettics > nettics[keyplayer]);
                oldnettics = maketic;

                if (frameskip[0] && frameskip[1] && frameskip[2] && frameskip[3])
                {
                    skiptics = 1;
                    // printf ("+");
                }
            }
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
	for (i=0 ; i<ticdup ; i++)
	{
            // check that there are players in the game.  if not, we cannot
            // run a tic.
        
            if (!PlayersInGame())
            {
                return;
            }
    
	    if (gametic/ticdup > lowtic)
		I_Error ("gametic>lowtic");
	    if (advancedemo)
		D_DoAdvanceDemo ();

	    G_Ticker ();
	    gametic++;
	    
	    // modify command for duplicated tics
	    if (i != ticdup-1)
	    {
		ticcmd_t	*cmd;
		int			buf;
		int			j;
				
		buf = (gametic/ticdup)%BACKUPTICS; 
		for (j=0 ; j<MAXPLAYERS ; j++)
		{
		    cmd = &netcmds[j][buf];
		    cmd->chatchar = 0;
		    if (cmd->buttons & BT_SPECIAL)
			cmd->buttons = 0;
		}
	    }
	}
	NetUpdate ();	// check for new console commands
    }
}

