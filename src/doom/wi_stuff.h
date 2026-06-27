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
//  Intermission.
//

#ifndef __WI_STUFF__
#define __WI_STUFF__

//#include "v_video.h"

#include "doomdef.h"

// States for the intermission

typedef enum
{
    NoState = -1,
    StatCount,
    ShowNextLoc,
} stateenum_t;

typedef enum
{
    sysop_intermission_none,
    sysop_intermission_stats,
    sysop_intermission_next,
} sysop_intermission_mode_t;

typedef struct
{
    int active;
    sysop_intermission_mode_t mode;
    int deathmatch;
    int netgame;
    int show_frags;
    int show_par;
    int episode;
    int last;
    int next;
    int me;
    int player_in_game[MAXPLAYERS];
    int kills[MAXPLAYERS];
    int items[MAXPLAYERS];
    int secrets[MAXPLAYERS];
    int frags[MAXPLAYERS];
    int dm_frags[MAXPLAYERS][MAXPLAYERS];
    int dm_totals[MAXPLAYERS];
    int time;
    int par;
} sysop_intermission_snapshot_t;

// Called by main loop, animate the intermission.
void WI_Ticker (void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer (void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t*	 wbstartstruct);

// Shut down the intermission screen
void WI_End(void);

void WI_GetSysopIntermissionSnapshot(sysop_intermission_snapshot_t *snapshot);

#endif
