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
//   All the global variables that store the internal state.
//   Theoretically speaking, the internal state of the engine
//    should be found by looking at the variables collected
//    here, and every relevant module will have to include
//    this header file.
//   In practice, things are a bit messy.
//


#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"
#include "d_loop.h"

// We need the playr data structure as well.
#include "d_player.h"

// Game mode/mission
#include "d_mode.h"

#include "net_defs.h"



// ------------------------
// Command line parameters.
//
extern  boolean	nomonsters;	// checkparm of -nomonsters
extern  boolean	respawnparm;	// checkparm of -respawn
extern  boolean	fastparm;	// checkparm of -fast

extern  boolean	devparm;	// DEBUG: launched with -devparm


// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//
extern GameMode_t	gamemode;
extern GameMission_t	gamemission;
extern GameVersion_t    gameversion;
extern char            *gamedescription;

// If true, we're using one of the mangled BFG edition IWADs.
extern boolean bfgedition;

// Convenience macro.
// 'gamemission' can be equal to pack_chex or pack_hacx, but these are
// just modified versions of doom and doom2, and should be interpreted
// as the same most of the time.

#define logical_gamemission                             \
    (gamemission == pack_chex ? doom :                  \
     gamemission == pack_hacx ? doom2 : gamemission)

// Set if homebrew PWAD stuff has been added.
extern  boolean	modifiedgame;


// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern  skill_t		startskill;
extern  int             startepisode;
extern	int		startmap;

// Savegame slot to load on startup.  This is the value provided to
// the -loadgame option.  If this has not been provided, this is -1.

extern  int             startloadgame;

extern  boolean		autostart;

// Selected by user. 
extern  skill_t         gameskill;
extern  int		gameepisode;
extern  int		gamemap;

// If non-zero, exit the level after this number of minutes
extern  int             timelimit;

// Nightmare mode flag, single player.
extern  boolean         respawnmonsters;

// Netgame? Only true if >1 player.
extern  boolean	netgame;

// 0=Cooperative; 1=Deathmatch; 2=Altdeath
extern int deathmatch;

// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int sfxVolume;
extern int musicVolume;

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
// Ideally, this would use indices found
//  in: /usr/include/linux/soundcard.h
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;


// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern  boolean statusbaractive;

extern  boolean automapactive;	// In AutoMap mode?
extern  boolean	menuactive;	// Menu overlayed?
extern  boolean	paused;		// Game Pause?


extern  boolean		viewactive;

extern  boolean		nodrawers;


extern  boolean         testcontrols;
extern  int             testcontrols_mousespeed;




// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  int	viewangleoffset;

// Player taking events, and displaying.
extern  int	consoleplayer;	
extern  int	displayplayer;


// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern  int	totalkills;
extern	int	totalitems;
extern	int	totalsecret;

// Timer, for scores.
extern  int	levelstarttic;	// gametic at level start
extern  int	leveltime;	// tics in game play for par



// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern  boolean	usergame;

//?
extern  boolean	demoplayback;
extern  boolean	demorecording;

// Round angleturn in ticcmds to the nearest 256.  This is used when
// recording Vanilla demos in netgames.

extern boolean lowres_turn;

// Quit after playing a demo from cmdline.
extern  boolean		singledemo;	




//?
extern  gamestate_t     gamestate;






//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.



// Bookkeeping on players - state.
extern	player_t	players[MAXPLAYERS];

// Alive? Disconnected?
extern  boolean		playeringame[MAXPLAYERS];


// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   10
extern  mapthing_t      deathmatchstarts[MAX_DM_STARTS];
extern  mapthing_t*	deathmatch_p;

// Player spawn spots.
extern  mapthing_t      playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern  wbstartstruct_t		wminfo;	







//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern  char *          savegamedir;
extern	char		basedefault[1024];

// if true, load all graphics at level load
extern  boolean         precache;


// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern  gamestate_t     wipegamestate;

extern  int             mouseSensitivity;

extern  int             bodyqueslot;



// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int		skyflatnum;



// Netgame stuff (buffers and pointers, i.e. indices).


extern	int		rndindex;

extern  ticcmd_t       *netcmds;


#endif
