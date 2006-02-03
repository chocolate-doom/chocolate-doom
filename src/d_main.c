// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_main.c 361 2006-02-03 18:40:00Z fraggle $
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
// Revision 1.39.2.4  2006/02/03 18:39:59  fraggle
// Support NWT-style WAD merging (-af and -as command line parameters).
// Restructure WAD loading so that merged WADs are always loaded before
// normal PWADs.  Remove W_InitMultipleFiles().
//
// Revision 1.39.2.3  2006/01/23 00:47:22  fraggle
// Rearrange the order of startup code to allow replacing the IWAD filename via dehacked
//
// Revision 1.39.2.2  2006/01/22 21:19:00  fraggle
// Dehacked string replacements for startup messages, IWAD names, demo names and backgrounds
//
// Revision 1.39.2.1  2006/01/20 00:58:17  fraggle
// Remove new networking code from stable version
//
// Revision 1.39  2006/01/14 02:06:48  fraggle
// Include the game version in the settings structure.
//
// Revision 1.38  2006/01/13 23:56:00  fraggle
// Add text-mode I/O functions.
// Use text-mode screen for the waiting screen.
//
// Revision 1.37  2006/01/10 22:14:13  fraggle
// Shut up compiler warnings
//
// Revision 1.36  2006/01/09 01:50:51  fraggle
// Deduce a sane player name by examining environment variables.  Add
// a "player_name" setting to chocolate-doom.cfg.  Transmit the name
// to the server and use the names players send in the waiting data list.
//
// Revision 1.35  2006/01/02 21:52:06  fraggle
// Move I_InitGraphics call to be invoked earlier in D_DoomMain.  Call the
// NET_WaitForStart function to wait for a start signal in network games.
//
// Revision 1.34  2006/01/02 00:17:42  fraggle
// Encapsulate the event queue code properly.  Add a D_PopEvent function
// to read a new event from the event queue.
//
// Revision 1.33  2006/01/01 23:53:14  fraggle
// Remove GS_WAITINGSTART gamestate.  This will be independent of the main
// loop to avoid interfering with the main game code too much.
//
// Revision 1.32  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//
// Revision 1.31  2005/10/24 18:50:39  fraggle
// Allow the game version to emulate to be specified from the command line
// and set compatibility options accordingly.
//
// Revision 1.30  2005/10/17 23:48:05  fraggle
// DEH_CheckCommandLine -> DEH_Init, for consistency with other Init
// functions
//
// Revision 1.29  2005/10/16 20:55:50  fraggle
// Fix the '-cdrom' command-line option.
//
// Revision 1.28  2005/10/16 01:18:10  fraggle
// Global "configdir" variable with directory to store config files in.
// Create a function to find the filename for a savegame slot.  Store
// savegames in the config dir.
//
// Revision 1.27  2005/10/15 17:57:47  fraggle
// Add warning message for WADs with FF_START or SS_START in, suggesting
// the -merge option.
//
// Revision 1.26  2005/10/15 17:38:49  fraggle
// Print startup banners which have been modified by dehacked.
//
// Revision 1.25  2005/10/12 21:52:01  fraggle
// doomfeatures.h to allow certain features to be disabled in the build
//
// Revision 1.24  2005/10/09 14:34:19  fraggle
// Fix banner string for ultimate doom
//
// Revision 1.23  2005/10/09 00:20:24  fraggle
// Detect registered DOOM banner in dehacked patches
//
// Revision 1.22  2005/10/08 21:01:55  fraggle
// Change dehacked startup message
//
// Revision 1.21  2005/10/08 20:10:51  fraggle
// Shut up compiler warning
//
// Revision 1.20  2005/10/08 19:33:48  fraggle
// Allow dehacked patches to override the name of the game via the
// startup banner.
//
// Revision 1.19  2005/10/08 18:23:18  fraggle
// WAD merging code
//
// Revision 1.18  2005/10/04 00:41:49  fraggle
// Move call to dehacked entrypoint to stop crashes
//
// Revision 1.17  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
// Revision 1.16  2005/10/02 04:16:47  fraggle
// Fixes for Final Doom
//
// Revision 1.15  2005/09/22 13:13:47  fraggle
// Remove external statistics driver support (-statcopy):
// nonfunctional on modern systems and never used.
// Fix for systems where sizeof(int) != sizeof(void *)
//
// Revision 1.14  2005/09/11 20:25:56  fraggle
// Second configuration file to allow chocolate doom-specific settings.
// Adjust some existing command line logic (for graphics settings and
// novert) to adjust for this.
//
// Revision 1.13  2005/09/08 22:05:17  fraggle
// Allow alt-tab away while running fullscreen
//
// Revision 1.12  2005/09/04 18:44:22  fraggle
// shut up compiler warnings
//
// Revision 1.11  2005/09/04 15:59:45  fraggle
// 'novert' command line option to disable vertical mouse movement
//
// Revision 1.10  2005/08/31 21:50:57  fraggle
// Nicer banner showing the game type (once we know).  Remove dead code.
// Find the config file properly.
//
// Revision 1.9  2005/08/31 21:35:42  fraggle
// Display the game name in the title bar.  Move game start code to later
// in initialisation because of the IWAD detection changes.
//
// Revision 1.8  2005/08/31 21:24:24  fraggle
// Remove the last traces of NORMALUNIX
//
// Revision 1.7  2005/08/31 21:21:18  fraggle
// Better IWAD detection and identification. Support '-iwad' to specify
// the IWAD to use.
//
// Revision 1.6  2005/08/30 22:11:10  fraggle
// Windows fixes
//
// Revision 1.5  2005/08/04 22:55:07  fraggle
// Use DOOM_VERSION to define the Doom version (don't conflict with
// automake's config.h).  Display GPL message instead of anti-piracy
// messages.
//
// Revision 1.4  2005/08/04 21:48:32  fraggle
// Turn on compiler optimisation and warning options
// Add SDL_mixer sound code
//
// Revision 1.3  2005/08/04 18:42:15  fraggle
// Silence compiler warnings
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:34  fraggle
// Initial import
//
//
// DESCRIPTION:
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//
//-----------------------------------------------------------------------------


static const char rcsid[] = "$Id: d_main.c 361 2006-02-03 18:40:00Z fraggle $";

#define	BGCOLOR		7
#define	FGCOLOR		8


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif


#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "doomfeatures.h"
#include "sounds.h"


#include "z_zone.h"
#include "w_wad.h"
#include "w_merge.h"
#include "s_sound.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"
#include "p_saveg.h"

#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"

#include "g_game.h"

#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

#include "p_setup.h"
#include "r_local.h"


#include "d_main.h"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);

// Location where all configuration data is stored - 
// default.cfg, savegames, etc.

char *          configdir;

// location of IWAD and WAD files

char *          iwadfile;


boolean		devparm;	// started game with -devparm
boolean         nomonsters;	// checkparm of -nomonsters
boolean         respawnparm;	// checkparm of -respawn
boolean         fastparm;	// checkparm of -fast

boolean         drone;

boolean		singletics = false; // debug flag to cancel adaptiveness



//extern int soundVolume;
//extern  int	sfxVolume;
//extern  int	musicVolume;

extern  boolean	inhelpscreens;

skill_t		startskill;
int             startepisode;
int		startmap;
boolean		autostart;

FILE*		debugfile;

boolean		advancedemo;




char		wadfile[1024];		// primary wad file
char		mapdir[1024];           // directory of development maps


void D_CheckNetGame (void);
void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t* cmd);
void D_DoAdvanceDemo (void);


//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
//

#define MAXEVENTS		64

static event_t         events[MAXEVENTS];
static int             eventhead;
static int 		eventtail;


//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent (event_t* ev)
{
    events[eventhead] = *ev;
    eventhead = (eventhead + 1) % MAXEVENTS;
}

// Read an event from the queue.

event_t *D_PopEvent(void)
{
    event_t *result;

    // No more events waiting.

    if (eventtail == eventhead)
    {
        return NULL;
    }
    
    result = &events[eventtail];

    // Advance to the next event in the queue.

    eventtail = (eventtail + 1) % MAXEVENTS;

    return result;
}


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*	ev;
	
    // IF STORE DEMO, DO NOT ACCEPT INPUT
    if ( ( gamemode == commercial )
	 && (W_CheckNumForName("map01")<0) )
      return;
	
    while ((ev = D_PopEvent()) != NULL)
    {
	if (M_Responder (ev))
	    continue;               // menu ate the event
	G_Responder (ev);
    }
}




//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t     wipegamestate = GS_DEMOSCREEN;
extern  boolean setsizeneeded;
extern  int             showMessages;
void R_ExecuteSetViewSize (void);

void D_Display (void)
{
    static  boolean		viewactivestate = false;
    static  boolean		menuactivestate = false;
    static  boolean		inhelpscreensstate = false;
    static  boolean		fullscreen = false;
    static  gamestate_t		oldgamestate = -1;
    static  int			borderdrawcount;
    int				nowtime;
    int				tics;
    int				wipestart;
    int				y;
    boolean			done;
    boolean			wipe;
    boolean			redrawsbar;

    if (nodrawers)
	return;                    // for comparative timing / profiling
		
    redrawsbar = false;
    
    // change the view size if needed
    if (setsizeneeded)
    {
	R_ExecuteSetViewSize ();
	oldgamestate = -1;                      // force background redraw
	borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
	wipe = true;
	wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
	wipe = false;

    if (gamestate == GS_LEVEL && gametic)
	HU_Erase();
    
    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
	if (!gametic)
	    break;
	if (automapactive)
	    AM_Drawer ();
	if (wipe || (viewheight != 200 && fullscreen) )
	    redrawsbar = true;
	if (inhelpscreensstate && !inhelpscreens)
	    redrawsbar = true;              // just put away the help screen
	ST_Drawer (viewheight == 200, redrawsbar );
	fullscreen = viewheight == 200;
	break;

      case GS_INTERMISSION:
	WI_Drawer ();
	break;

      case GS_FINALE:
	F_Drawer ();
	break;

      case GS_DEMOSCREEN:
	D_PageDrawer ();
	break;
    }
    
    // draw buffered stuff to screen
    I_UpdateNoBlit ();
    
    // draw the view directly
    if (gamestate == GS_LEVEL && !automapactive && gametic)
	R_RenderPlayerView (&players[displayplayer]);

    if (gamestate == GS_LEVEL && gametic)
	HU_Drawer ();
    
    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
	I_SetPalette (W_CacheLumpName ("PLAYPAL",PU_CACHE));

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
	viewactivestate = false;        // view was not active
	R_FillBackScreen ();    // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != 320)
    {
	if (menuactive || menuactivestate || !viewactivestate)
	    borderdrawcount = 3;
	if (borderdrawcount)
	{
	    R_DrawViewBorder ();    // erase old menu stuff
	    borderdrawcount--;
	}

    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;
    
    // draw pause pic
    if (paused)
    {
	if (automapactive)
	    y = 4;
	else
	    y = viewwindowy+4;
	V_DrawPatchDirect(viewwindowx+(scaledviewwidth-68)/2,
			  y,0,W_CacheLumpName ("M_PAUSE", PU_CACHE));
    }


    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation


    // normal update
    if (!wipe)
    {
	I_FinishUpdate ();              // page flip or blit buffer
	return;
    }
    
    // wipe update
    wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    wipestart = I_GetTime () - 1;

    do
    {
	do
	{
	    nowtime = I_GetTime ();
	    tics = nowtime - wipestart;
	} while (!tics);
	wipestart = nowtime;
	done = wipe_ScreenWipe(wipe_Melt
			       , 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
	I_UpdateNoBlit ();
	M_Drawer ();                            // menu is drawn even on top of wipes
	I_FinishUpdate ();                      // page flip or blit buffer
    } while (!done);
}



//
//  D_DoomLoop
//
extern  boolean         demorecording;

void D_DoomLoop (void)
{
    if (demorecording)
	G_BeginRecording ();
		
    if (M_CheckParm ("-debugfile"))
    {
	char    filename[20];
	sprintf (filename,"debug%i.txt",consoleplayer);
	printf ("debug output to: %s\n",filename);
	debugfile = fopen (filename,"w");
    }

    I_InitGraphics ();

    while (1)
    {
	// frame syncronous IO operations
	I_StartFrame ();                
	
	// process one or more tics
	if (singletics)
	{
	    I_StartTic ();
	    D_ProcessEvents ();
	    G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
	    if (advancedemo)
		D_DoAdvanceDemo ();
	    M_Ticker ();
	    G_Ticker ();
	    gametic++;
	    maketic++;
	}
	else
	{
	    TryRunTics (); // will run at least one tic
	}
		
	S_UpdateSounds (players[consoleplayer].mo);// move positional sounds

	// Update display, next frame, with current state.
        if (screenvisible)
            D_Display ();
    }
}



//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char                    *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
	D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
    V_DrawPatch (0,0, 0, W_CacheLumpName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;

    if ( gamemode == retail )
      demosequence = (demosequence+1)%7;
    else
      demosequence = (demosequence+1)%6;
    
    switch (demosequence)
    {
      case 0:
	if ( gamemode == commercial )
	    pagetic = 35 * 11;
	else
	    pagetic = 170;
	gamestate = GS_DEMOSCREEN;
	pagename = DEH_String("TITLEPIC");
	if ( gamemode == commercial )
	  S_StartMusic(mus_dm2ttl);
	else
	  S_StartMusic (mus_intro);
	break;
      case 1:
	G_DeferedPlayDemo(DEH_String("demo1"));
	break;
      case 2:
	pagetic = 200;
	gamestate = GS_DEMOSCREEN;
	pagename = DEH_String("CREDIT");
	break;
      case 3:
	G_DeferedPlayDemo(DEH_String("demo2"));
	break;
      case 4:
	gamestate = GS_DEMOSCREEN;
	if ( gamemode == commercial)
	{
	    pagetic = 35 * 11;
	    pagename = DEH_String("TITLEPIC");
	    S_StartMusic(mus_dm2ttl);
	}
	else
	{
	    pagetic = 200;

	    if ( gamemode == retail )
	      pagename = DEH_String("CREDIT");
	    else
	      pagename = DEH_String("HELP2");
	}
	break;
      case 5:
	G_DeferedPlayDemo(DEH_String("demo3"));
	break;
        // THE DEFINITIVE DOOM Special Edition demo
      case 6:
	G_DeferedPlayDemo(DEH_String("demo4"));
	break;
    }
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo ();
}




//      print title for every printed line
char            title[128];


static void D_AddFile(char *filename)
{
    printf(" adding %s\n", filename);
    W_AddFile(filename);
}


// Check if a file exists

static int FileExists(char *filename)
{
    FILE *fstream;

    fstream = fopen(filename, "r");

    if (fstream != NULL)
    {
        fclose(fstream);
        return 1;
    }
    else
    {
        return 0;
    }
}

struct 
{
    char *name;
    GameMission_t mission;
} iwads[] = {
    {"doom.wad",     doom},
    {"doom2.wad",    doom2},
    {"tnt.wad",      pack_tnt},
    {"plutonia.wad", pack_plut},
    {"doom1.wad",    doom},
};

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise NULL.

static char *SearchDirectoryForIWAD(char *dir)
{
    int i;

    for (i=0; i<sizeof(iwads) / sizeof(*iwads); ++i) 
    {
        char *filename; 
	char *iwadname;

	iwadname = DEH_String(iwads[i].name);
	
	filename = malloc(strlen(dir) + strlen(iwadname) + 3);

        sprintf(filename, "%s/%s", dir, iwadname);

        if (FileExists(filename))
        {
            iwadfile = filename;
            gamemission = iwads[i].mission;
            return filename;
        }

        free(filename);
    }

    return NULL;
}

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.

static void IdentifyIWADByName(char *name)
{
    int i;

    gamemission = none;
    
    for (i=0; i<sizeof(iwads) / sizeof(*iwads); ++i)
    {
	char *iwadname;

	iwadname = DEH_String(iwads[i].name);

        if (strlen(name) < strlen(iwadname))
            continue;

        // Check if it ends in this IWAD name.

        if (!strcasecmp(name + strlen(name) - strlen(iwadname), 
                        iwadname))
        {
            gamemission = iwads[i].mission;
            break;
        }
    }
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//

static void FindIWAD (void)
{
    char *doomwaddir;
    char *result;
    int iwadparm;

    result = 0;
    doomwaddir = getenv("DOOMWADDIR");
    iwadparm = M_CheckParm("-iwad");

    if (iwadparm)
    {
        iwadfile = myargv[iwadparm + 1];
        result = iwadfile;
        IdentifyIWADByName(iwadfile);
    }
    else if (doomwaddir != NULL)
    {
        result = SearchDirectoryForIWAD(doomwaddir);
    }

    if (result == NULL)
        result = SearchDirectoryForIWAD(".");
    if (result == NULL)
        result = SearchDirectoryForIWAD("/usr/share/games/doom");
    if (result == NULL)
        result = SearchDirectoryForIWAD("/usr/local/share/games/doom");

    if (result == NULL)
    {
        I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
                "specifying one with the '-iwad' command line parameter.\n");
    }
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static char *banners[] = 
{
    // doom1.wad
    "                            "
    "DOOM Shareware Startup v%i.%i"
    "                           ",
    // doom.wad
    "                            "
    "DOOM Registered Startup v%i.%i"
    "                           ",
    // Registered DOOM uses this
    "                          "
    "DOOM System Startup v%i.%i"
    "                          ",
    // doom.wad (Ultimate DOOM)
    "                         "
    "The Ultimate DOOM Startup v%i.%i"
    "                        ",
    // doom2.wad
    "                         "
    "DOOM 2: Hell on Earth v%i.%i"
    "                           ",
    // tnt.wad
    "                     "
    "DOOM 2: TNT - Evilution v%i.%i"
    "                           ",
    // plutonia.wad
    "                   "
    "DOOM 2: Plutonia Experiment v%i.%i"
    "                           ",
};

// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are 
// replaced by dehacked.

static char *copyright_banners[] =
{
    "===========================================================================\n"
    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
    "        You will not receive technical support for modified games.\n"
    "                      press enter to continue\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                 Commercial product - do not distribute!\n"
    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                                Shareware!\n"
    "===========================================================================\n"
};

//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// 

static char *GetGameName(char *gamename)
{
    int i;
    char *deh_sub;
    
    for (i=0; i<sizeof(banners) / sizeof(*banners); ++i)
    {
        // Has the banner been replaced?

        deh_sub = DEH_String(banners[i]);
        
        if (deh_sub != banners[i])
        {
            // Has been replaced
            // We need to expand via printf to include the Doom version 
            // number
            // We also need to cut off spaces to get the basic name

            gamename = Z_Malloc(strlen(deh_sub) + 10, PU_STATIC, 0);
            sprintf(gamename, deh_sub, DOOM_VERSION / 100, DOOM_VERSION % 100);

            while (gamename[0] != '\0' && isspace(gamename[0]))
                strcpy(gamename, gamename+1);

            while (gamename[0] != '\0' && isspace(gamename[strlen(gamename)-1]))
                gamename[strlen(gamename) - 1] = '\0';
            
            return gamename;
        }
    }

    return gamename;
}


//
// Find out what version of Doom is playing.
//

static void IdentifyVersion(void)
{
    // gamemission is set up by the FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.

    if (gamemission == none)
    {
        int i;

        for (i=0; i<numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i].name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            } 
            else if (!strncasecmp(lumpinfo[i].name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
        {
            // Still no idea.  I don't think this is going to work.

            I_Error("Unknown or invalid IWAD file.");
        }
    }

    // Make sure gamemode is set up correctly

    if (gamemission == doom)
    {
        // Doom 1.  But which version?

        if (W_CheckNumForName("E4M1") > 0)
        {
            // Ultimate Doom

            gamemode = retail;
        } 
        else if (W_CheckNumForName("E3M1") > 0)
        {
            gamemode = registered;
        }
        else
        {
            gamemode = shareware;
        }
    }
    else
    {
        // Doom 2 of some kind.

        gamemode = commercial;
    }
}

// Set the gamedescription string

static void SetGameDescription(void)
{
    gamedescription = "Unknown";

    if (gamemission == doom)
    {
        // Doom 1.  But which version?

        if (gamemode == retail)
        {
            // Ultimate Doom

            gamedescription = GetGameName("The Ultimate DOOM");
        } 
        else if (gamemode == registered)
        {
            gamedescription = GetGameName("DOOM Registered");
        }
        else if (gamemode == shareware)
        {
            gamedescription = GetGameName("DOOM Shareware");
        }
    }
    else
    {
        // Doom 2 of some kind.  But which mission?

        if (gamemission == doom2)
            gamedescription = GetGameName("DOOM 2: Hell on Earth");
        else if (gamemission == pack_plut)
            gamedescription = GetGameName("DOOM 2: Plutonia Experiment"); 
        else if (gamemission == pack_tnt)
            gamedescription = GetGameName("DOOM 2: TNT - Evilution");
    }
}

//
// Find a Response File
//
void FindResponseFile (void)
{
    int             i;
#define MAXARGVS        100
	
    for (i = 1;i < myargc;i++)
	if (myargv[i][0] == '@')
	{
	    FILE *          handle;
	    int             size;
	    int             k;
	    int             index;
	    int             indexinfile;
	    char    *infile;
	    char    *file;
	    char    *moreargs[20];
	    char    *firstargv;
			
	    // READ THE RESPONSE FILE INTO MEMORY
	    handle = fopen (&myargv[i][1],"rb");
	    if (!handle)
	    {
		printf ("\nNo such response file!");
		exit(1);
	    }
	    printf("Found response file %s!\n",&myargv[i][1]);
	    fseek (handle,0,SEEK_END);
	    size = ftell(handle);
	    fseek (handle,0,SEEK_SET);
	    file = malloc (size);
	    fread (file,size,1,handle);
	    fclose (handle);
			
	    // KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
	    for (index = 0,k = i+1; k < myargc; k++)
		moreargs[index++] = myargv[k];
			
	    firstargv = myargv[0];
	    myargv = malloc(sizeof(char *)*MAXARGVS);
	    memset(myargv,0,sizeof(char *)*MAXARGVS);
	    myargv[0] = firstargv;
			
	    infile = file;
	    indexinfile = k = 0;
	    indexinfile++;  // SKIP PAST ARGV[0] (KEEP IT)
	    do
	    {
		myargv[indexinfile++] = infile+k;
		while(k < size &&
		      ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
		    k++;
		*(infile+k) = 0;
		while(k < size &&
		      ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
		    k++;
	    } while(k < size);
			
	    for (k = 0;k < index;k++)
		myargv[indexinfile++] = moreargs[k];
	    myargc = indexinfile;
	
	    // DISPLAY ARGS
	    printf("%d command-line args:\n",myargc);
	    for (k=1;k<myargc;k++)
		printf("%s\n",myargv[k]);

	    break;
	}
}

// Startup banner

void PrintBanner(char *msg)
{
    int i;
    int spaces = 35 - (strlen(msg) / 2);

    for (i=0; i<spaces; ++i)
        putchar(' ');

    puts(msg);
}

// Prints a message only if it has been modified by dehacked.

void PrintDehackedBanners(void)
{
    int i;

    for (i=0; i<sizeof(copyright_banners) / sizeof(char *); ++i)
    {
        char *deh_s;

        deh_s = DEH_String(copyright_banners[i]);

        if (deh_s != copyright_banners[i])
        {
            printf("%s", deh_s);
        }
    }
}

// 
// SetConfigDir:
//
// Sets the location of the configuration directory, where configuration
// files are stored - default.cfg, chocolate-doom.cfg, savegames, etc.
//

static void SetConfigDir(void)
{
    char *homedir;

    homedir = getenv("HOME");

    if (homedir != NULL)
    {
        // put all configuration in a config directory off the
        // homedir

        configdir = malloc(strlen(homedir) + strlen(PACKAGE_TARNAME) + 5);

        sprintf(configdir, "%s/.%s/", homedir, PACKAGE_TARNAME);

        // make the directory if it doesnt already exist
#ifdef _WIN32
        mkdir(configdir);
#else
        mkdir(configdir, 0755);
#endif
    }
    else
    {
#ifdef _WIN32
        // when given the -cdrom option, save config+savegames in 
        // c:\doomdata.  This only applies under Windows.

        if (M_CheckParm("-cdrom") > 0)
        {
            printf(D_CDROM);
            configdir = strdup("c:\\doomdata\\");
        }
        else
#endif
        {
            configdir = strdup("");
        }
    }
}

static struct 
{
    char *description;
    char *cmdline;
    GameVersion_t version;
} gameversions[] = {
    {"Doom 1.9",             "1.9",        exe_doom_1_9},
    {"Ultimate Doom",        "ultimate",   exe_ultimate},
    {"Final Doom",           "final",      exe_final},
    {NULL},
};

// Initialise the game version

static void InitGameVersion(void)
{
    int p;
    int i;

    p = M_CheckParm("-gameversion");

    if (p > 0)
    {
        for (i=0; gameversions[i].description != NULL; ++i)
        {
            if (!strcmp(myargv[p+1], gameversions[i].cmdline))
            {
                gameversion = gameversions[i].version;
                break;
            }
        }
        
        if (gameversions[i].description == NULL) 
        {
            printf("Supported game versions:\n");

            for (i=0; gameversions[i].description != NULL; ++i)
            {
                printf("\t%s (%s)\n", gameversions[i].cmdline,
                        gameversions[i].description);
            }
            
            I_Error("Unknown game version '%s'", myargv[p+1]);
        }
    }
    else
    {
        // Determine automatically

        if (gamemode == shareware || gamemode == registered)
        {
            // original

            gameversion = exe_doom_1_9;
        }
        else if (gamemode == retail)
        {
            gameversion = exe_ultimate;
        }
        else if (gamemode == commercial)
        {
            if (gamemission == doom2)
            {
                gameversion = exe_doom_1_9;
            }
            else
            {
                // Final Doom: tnt or plutonia

                gameversion = exe_final;
            }
        }
    }
    
    // The original exe does not support retail - 4th episode not supported

    if (gameversion < exe_ultimate && gamemode == retail)
    {
        gamemode = registered;
    }

    // EXEs prior to the Final Doom exes do not support Final Doom.

    if (gameversion < exe_final && gamemode == commercial)
    {
        gamemission = doom2;
    }
}

void PrintGameVersion(void)
{
    int i;

    for (i=0; gameversions[i].description != NULL; ++i)
    {
        if (gameversions[i].version == gameversion)
        {
            printf("Emulating the behaviour of the "
                   "'%s' executable.\n", gameversions[i].description);
            break;
        }
    }
}

//
// D_DoomMain
//
void D_DoomMain (void)
{
    int             p;
    char                    file[256];

    FindResponseFile ();
	
    // print banner

    PrintBanner(PACKAGE_STRING);

    printf (DEH_String("Z_Init: Init zone memory allocation daemon. \n"));
    Z_Init ();

#ifdef FEATURE_DEHACKED
    printf("DEH_Init: Init Dehacked support.\n");
    DEH_Init();
#endif

    FindIWAD ();
	
    setbuf (stdout, NULL);
    modifiedgame = false;
	
    nomonsters = M_CheckParm ("-nomonsters");
    respawnparm = M_CheckParm ("-respawn");
    fastparm = M_CheckParm ("-fast");
    devparm = M_CheckParm ("-devparm");
    if (M_CheckParm ("-altdeath"))
	deathmatch = 2;
    else if (M_CheckParm ("-deathmatch"))
	deathmatch = 1;

    if (devparm)
	printf(DEH_String(D_DEVSTR));
    
    // find which dir to use for config files

    SetConfigDir();
    
    // turbo option
    if ( (p=M_CheckParm ("-turbo")) )
    {
	int     scale = 200;
	extern int forwardmove[2];
	extern int sidemove[2];
	
	if (p<myargc-1)
	    scale = atoi (myargv[p+1]);
	if (scale < 10)
	    scale = 10;
	if (scale > 400)
	    scale = 400;
	printf (DEH_String("turbo scale: %i%%\n"),scale);
	forwardmove[0] = forwardmove[0]*scale/100;
	forwardmove[1] = forwardmove[1]*scale/100;
	sidemove[0] = sidemove[0]*scale/100;
	sidemove[1] = sidemove[1]*scale/100;
    }
    
    // init subsystems
    printf (DEH_String("V_Init: allocate screens.\n"));
    V_Init ();

    printf (DEH_String("M_LoadDefaults: Load system defaults.\n"));
    M_LoadDefaults ();              // load before initing other systems

    printf (DEH_String("W_Init: Init WADfiles.\n"));
    D_AddFile(iwadfile);

#ifdef FEATURE_WAD_MERGE
    // Merged PWADs are loaded first, because they are supposed to be 
    // modified IWADs.

    p = M_CheckParm("-merge");

    if (p > 0)
    {
        for (p = p + 1; p<myargc && myargv[p][0] != '-'; ++p)
        {
            printf(" merging %s\n", myargv[p]);
            W_MergeFile(myargv[p]);
        }
    }

    // NWT-style merging:

    // Add flats

    p = M_CheckParm("-af");

    if (p > 0)
    {
        for (p = p + 1; p<myargc && myargv[p][0] != '-'; ++p)
        {
            printf(" merging flats from %s\n", myargv[p]);
            W_NWTMergeFile(myargv[p], W_NWT_MERGE_FLATS);
        }
    }

    // Add sprites

    p = M_CheckParm("-as");

    if (p > 0)
    {
        for (p = p + 1; p<myargc && myargv[p][0] != '-'; ++p)
        {
            printf(" merging sprites from %s\n", myargv[p]);
            W_NWTMergeFile(myargv[p], W_NWT_MERGE_SPRITES);
        }
    }

    // Add sprites AND flats

    p = M_CheckParm("-aa");

    if (p > 0)
    {
        for (p = p + 1; p<myargc && myargv[p][0] != '-'; ++p)
        {
            printf(" merging sprites and flats from %s\n", myargv[p]);
            W_NWTMergeFile(myargv[p], W_NWT_MERGE_SPRITES | W_NWT_MERGE_FLATS);
        }
    }

#endif

    // Load normal PWADs

    p = M_CheckParm ("-file");
    if (p)
    {
	// the parms after p are wadfile/lump names,
	// until end of parms or another - preceded parm
	modifiedgame = true;            // homebrew levels
	while (++p != myargc && myargv[p][0] != '-')
	    D_AddFile (myargv[p]);
    }

    // add any files specified on the command line with -file wadfile
    // to the wad list
    //
    // convenience hack to allow -wart e m to add a wad file
    // prepend a tilde to the filename so wadfile will be reloadable
    p = M_CheckParm ("-wart");
    if (p)
    {
	myargv[p][4] = 'p';     // big hack, change to -warp

	// Map name handling.
	switch (gamemode )
	{
	  case shareware:
	  case retail:
	  case registered:
	    sprintf (file,"~"DEVMAPS"E%cM%c.wad",
		     myargv[p+1][0], myargv[p+2][0]);
	    printf("Warping to Episode %s, Map %s.\n",
		   myargv[p+1],myargv[p+2]);
	    break;
	    
	  case commercial:
	  default:
	    p = atoi (myargv[p+1]);
	    if (p<10)
	      sprintf (file,"~"DEVMAPS"cdata/map0%i.wad", p);
	    else
	      sprintf (file,"~"DEVMAPS"cdata/map%i.wad", p);
	    break;
	}
	D_AddFile (file);
    }
	
    p = M_CheckParm ("-playdemo");

    if (!p)
	p = M_CheckParm ("-timedemo");

    if (p && p < myargc-1)
    {
	sprintf (file,"%s.lmp", myargv[p+1]);
	D_AddFile (file);
	printf(DEH_String("Playing demo %s.lmp.\n"),myargv[p+1]);
    }
    
    IdentifyVersion();
    InitGameVersion();
    SetGameDescription();

    // Check for -file in shareware
    if (modifiedgame)
    {
	// These are the lumps that will be checked in IWAD,
	// if any one is not present, execution will be aborted.
	char name[23][8]=
	{
	    "e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
	    "e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
	    "dphoof","bfgga0","heada1","cybra1","spida1d1"
	};
	int i;
	
	if ( gamemode == shareware)
	    I_Error(DEH_String("\nYou cannot -file with the shareware "
			       "version. Register!"));

	// Check for fake IWAD with right name,
	// but w/o all the lumps of the registered version. 
	if (gamemode == registered)
	    for (i = 0;i < 23; i++)
		if (W_CheckNumForName(name[i])<0)
		    I_Error(DEH_String("\nThis is not the registered version."));
    }
    
    // get skill / episode / map from parms
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

		
    p = M_CheckParm ("-skill");
    if (p && p < myargc-1)
    {
	startskill = myargv[p+1][0]-'1';
	autostart = true;
    }

    p = M_CheckParm ("-episode");
    if (p && p < myargc-1)
    {
	startepisode = myargv[p+1][0]-'0';
	startmap = 1;
	autostart = true;
    }
	
    p = M_CheckParm ("-timer");
    if (p && p < myargc-1 && deathmatch)
    {
	int     time;
	time = atoi(myargv[p+1]);
	printf(DEH_String("Levels will end after %d minute"),time);
	if (time>1)
	    printf("s");
	printf(".\n");
    }

    p = M_CheckParm ("-avg");
    if (p && p < myargc-1 && deathmatch)
	printf(DEH_String("Austin Virtual Gaming: Levels will end "
			  "after 20 minutes\n"));

    p = M_CheckParm ("-warp");
    if (p && p < myargc-1)
    {
	if (gamemode == commercial)
	    startmap = atoi (myargv[p+1]);
	else
	{
	    startepisode = myargv[p+1][0]-'0';
	    startmap = myargv[p+2][0]-'0';
	}
	autostart = true;
    }

    if (M_CheckParm("-novert"))
        novert = true;
    else if (M_CheckParm("-nonovert"))
        novert = false;

    if (W_CheckNumForName("SS_START") >= 0
     || W_CheckNumForName("FF_START") >= 0)
    {
        printf ("===========================================================================\n");
        printf(" WARNING: The loaded WAD file contains modified sprites or\n"
               " floor textures.  You may want to use the '-merge' command\n"
               " line option instead of '-file'.\n");
    }
    
    printf ("===========================================================================\n");

    PrintBanner(gamedescription);

    
    printf (
	    "===========================================================================\n"
	    " " PACKAGE_NAME " is free software, covered by the GNU General Public\n"
            " License.  There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
            " FOR A PARTICULAR PURPOSE. You are welcome to change and distribute\n"
            " copies under certain conditions. See the source for more information.\n"

	    "===========================================================================\n"
	);

    PrintDehackedBanners();

    printf (DEH_String("M_Init: Init miscellaneous info.\n"));
    M_Init ();

    printf (DEH_String("R_Init: Init DOOM refresh daemon - "));
    R_Init ();

    printf (DEH_String("\nP_Init: Init Playloop state.\n"));
    P_Init ();

    printf (DEH_String("I_Init: Setting up machine state.\n"));
    I_Init ();

    printf (DEH_String("D_CheckNetGame: Checking network game status.\n"));
    D_CheckNetGame ();

    PrintGameVersion();

    printf (DEH_String("S_Init: Setting up sound.\n"));
    S_Init (snd_SfxVolume /* *8 */, snd_MusicVolume /* *8*/ );

    printf (DEH_String("HU_Init: Setting up heads up display.\n"));
    HU_Init ();

    printf (DEH_String("ST_Init: Init status bar.\n"));
    ST_Init ();

    // start the apropriate game based on parms
    p = M_CheckParm ("-record");

    if (p && p < myargc-1)
    {
	G_RecordDemo (myargv[p+1]);
	autostart = true;
    }
	
    p = M_CheckParm ("-playdemo");
    if (p && p < myargc-1)
    {
	singledemo = true;              // quit after one demo
	G_DeferedPlayDemo (myargv[p+1]);
	D_DoomLoop ();  // never returns
    }
	
    p = M_CheckParm ("-timedemo");
    if (p && p < myargc-1)
    {
	G_TimeDemo (myargv[p+1]);
	D_DoomLoop ();  // never returns
    }
	
    p = M_CheckParm ("-loadgame");
    if (p && p < myargc-1)
    {
        strcpy(file, P_SaveGameFile(atoi(myargv[p+1])));
	G_LoadGame (file);
    }
	
    if (gameaction != ga_loadgame )
    {
	if (autostart || netgame)
	    G_InitNew (startskill, startepisode, startmap);
	else
	    D_StartTitle ();                // start up intro loop

    }
    
    D_DoomLoop ();  // never returns
}
