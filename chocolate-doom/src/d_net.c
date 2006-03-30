// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
// $Log$
// Revision 1.22  2006/02/24 19:14:22  fraggle
// Remove redundant stuff relating to the old network code
//
// Revision 1.21  2006/02/23 23:42:00  fraggle
// Replace -client with -connect which takes a hostname/ip to connect to.
//
// Revision 1.20  2006/02/23 20:22:57  fraggle
// Do not allow tics to buffer up in single player (stops the gun instantly
// appearing on level start)
//
// Revision 1.19  2006/02/23 19:12:43  fraggle
// Set maketic-gametic lag back to 1 second.
//
// Revision 1.18  2006/02/23 19:12:01  fraggle
// Add lowres_turn to indicate whether we generate angleturns which are
// 8-bit as opposed to 16-bit.  This is used when recording demos without
// -longtics enabled.  Sync this option between clients in a netgame, so
// that if one player is recording a Vanilla demo, all clients record
// in lowres.
//
// Revision 1.17  2006/02/19 13:42:27  fraggle
// Move tic number expansion code to common code.  Parse game data packets
// received from the server.
// Strip down d_net.[ch] to work through the new networking code.  Remove
// game sync code.
// Remove i_net.[ch] as it is no longer needed.
// Working networking!
//
// Revision 1.16  2006/01/13 23:56:00  fraggle
// Add text-mode I/O functions.
// Use text-mode screen for the waiting screen.
//
// Revision 1.15  2006/01/02 21:04:10  fraggle
// Create NET_SV_Shutdown function to shut down the server.  Call it
// when quitting the game.  Print the IP of the server correctly when
// connecting.
//
// Revision 1.14  2006/01/02 20:14:29  fraggle
// Add a "-client" option to test connecting to a local server.
//
// Revision 1.13  2006/01/02 00:17:42  fraggle
// Encapsulate the event queue code properly.  Add a D_PopEvent function
// to read a new event from the event queue.
//
// Revision 1.12  2006/01/02 00:00:08  fraggle
// Neater prefixes: NET_Client -> NET_CL_.  NET_Server -> NET_SV_.
//
// Revision 1.11  2006/01/01 23:54:31  fraggle
// Client disconnect code
//
// Revision 1.10  2006/01/01 23:53:15  fraggle
// Remove GS_WAITINGSTART gamestate.  This will be independent of the main
// loop to avoid interfering with the main game code too much.
//
// Revision 1.9  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//
// Revision 1.8  2005/09/22 13:13:47  fraggle
// Remove external statistics driver support (-statcopy):
// nonfunctional on modern systems and never used.
// Fix for systems where sizeof(int) != sizeof(void *)
//
// Revision 1.7  2005/09/08 22:10:40  fraggle
// Delay calls so we don't use the entire CPU
//
// Revision 1.6  2005/09/04 18:44:22  fraggle
// shut up compiler warnings
//
// Revision 1.5  2005/08/31 21:24:24  fraggle
// Remove the last traces of NORMALUNIX
//
// Revision 1.4  2005/08/04 22:55:07  fraggle
// Use DOOM_VERSION to define the Doom version (don't conflict with
// automake's config.h).  Display GPL message instead of anti-piracy
// messages.
//
// Revision 1.3  2005/07/23 19:17:11  fraggle
// Use ANSI-standard limit constants.  Remove LINUX define.
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:50  fraggle
// Initial import
//
//
// DESCRIPTION:
//	DOOM Network game communication and protocol,
//	all OS independend parts.
//
//-----------------------------------------------------------------------------


static const char rcsid[] = "$Id$";

#include "doomfeatures.h"

#include "d_main.h"
#include "m_argv.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_video.h"
#include "g_game.h"
#include "doomdef.h"
#include "doomstat.h"

#include "net_client.h"
#include "net_gui.h"
#include "net_io.h"
#include "net_server.h"
#include "net_sdl.h"
#include "net_loop.h"


#define FPS 35

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

int		lastnettic;
int		ticdup;		
int             extratics;
fixed_t         offsetms;


void D_ProcessEvents (void); 
void G_BuildTiccmd (ticcmd_t *cmd); 
void D_DoAdvanceDemo (void);
 

// 35 fps clock adjusted by offsetms milliseconds

static int GetAdjustedTime(void)
{
    int time_ms;

    time_ms = I_GetTimeMS() + (offsetms / FRACUNIT);

    return (time_ms * FPS) / 1000;
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
int      lasttime;

void NetUpdate (void)
{
    int             nowtime;
    int             newtics;
    int				i;
    int				gameticdiv;

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
    nowtime = GetAdjustedTime()/ticdup;
    newtics = nowtime - lasttime;
    lasttime = nowtime;

    if (newtics <= 0) 	// nothing new to update
        return;

    // build new ticcmds for console player
    gameticdiv = gametic/ticdup;

    for (i=0 ; i<newtics ; i++)
    {
        ticcmd_t cmd;

	I_StartTic ();
	D_ProcessEvents ();

        // Always run the menu

        M_Ticker ();
	
        // If playing single player, do not allow tics to buffer
        // up very far

        if ((!netgame || demoplayback) && maketic - gameticdiv > 2)
            break;

        // Never go more than ~200ms ahead

        if (maketic - gameticdiv > 8)
            break;

	//printf ("mk:%i ",maketic);
	G_BuildTiccmd(&cmd);

#ifdef FEATURE_MULTIPLAYER
        
        if (netgame && !demoplayback)
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

#ifdef FEATURE_MULTIPLAYER

    {
        net_module_t *connect_module = NULL;
        char *connect_addr;

        if (M_CheckParm("-server") > 0)
        {
            NET_SV_Init();
            NET_SV_AddModule(&net_loop_server_module);
            NET_SV_AddModule(&net_sdl_module);

            connect_module = &net_loop_client_module;
            connect_addr = "";
        }
        else
        {
            i = M_CheckParm("-connect");

            if (i > 0)
            {
                connect_module = &net_sdl_module;
                connect_addr = myargv[i+1];
            }
        }

        if (connect_module != NULL)
        {
            net_addr_t *addr;

            connect_module->InitClient();

            addr = connect_module->ResolveAddress(connect_addr);

            if (addr == NULL)
            {
                I_Error("Unable to resolve \"%s\"", connect_addr);
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

    printf ("startskill %i  deathmatch: %i  startmap: %i  startepisode: %i\n",
	    startskill, deathmatch, startmap, startepisode);
	
    printf ("player %i of %i (%i nodes)\n",
	    consoleplayer+1, num_players, num_players);

}


//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame (void)
{
    if (debugfile)
	fclose (debugfile);

#ifdef FEATURE_MULTIPLAYER

    NET_SV_Shutdown();
    NET_CL_Disconnect();

#endif

}

static int GetLowTic(void)
{
    int i;
    int lowtic;

    if (demoplayback)
    {
        lowtic = maketic;
    }
    else
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

    return lowtic;
}

//
// TryRunTics
//
int	oldnettics;

extern	boolean	advancedemo;

void TryRunTics (void)
{
    int		i;
    int		lowtic;
    int		entertic;
    int		availabletics;
    int		counts;
    
    // get real tics		
    entertic = I_GetTime ()/ticdup;
    
    // get available tics
    NetUpdate ();
	
    lowtic = GetLowTic();

    availabletics = lowtic - gametic/ticdup;
    
    // decide how many tics to run
    
    counts = availabletics;

    if (counts < 1)
	counts = 1;
		
    // wait for new tics if needed
    while (lowtic < gametic/ticdup + counts)	
    {
	NetUpdate ();   

        lowtic = GetLowTic();
	
	if (lowtic < gametic/ticdup)
	    I_Error ("TryRunTics: lowtic < gametic");
    
        // Don't stay in this loop forever.  The menu is still running,
        // so return to update the screen

	if (I_GetTime ()/ticdup - entertic > 0)
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
