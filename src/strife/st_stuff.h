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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "m_cheat.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_HEIGHT	32
#define ST_WIDTH	SCREENWIDTH
#define ST_Y		(SCREENHEIGHT - ST_HEIGHT)


//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);

// Called by main loop.
void ST_Drawer (boolean fullscreen, boolean refresh);

// haleyjd 09/01/10: [STRIFE] New function.
// Called by main loop to draw external status bar bits.
// Returns true if a popup is drawing.
boolean ST_DrawExternal(void);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called by startup code.
void ST_Init (void);



// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState
    
} st_stateenum_t;


// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState
    
} st_chatstateenum_t;



extern byte *st_backing_screen;

extern cheatseq_t cheat_mus;     // [STRIFE]: idmus -> spin
extern cheatseq_t cheat_god;     // [STRIFE]: iddqd -> omnipotent
extern cheatseq_t cheat_ammo;    // [STRIFE]: idfa -> boomstix
extern cheatseq_t cheat_noclip;  // [STRIFE]: idclip -> elvis
extern cheatseq_t cheat_clev;    // [STRIFE]: idclev -> rift
extern cheatseq_t cheat_mypos;   // [STRIFE]: idmypos -> gps
extern cheatseq_t cheat_scoot;   // [STRIFE]: new cheat scoot
extern cheatseq_t cheat_nuke;    // [STRIFE]: new cheat stonecold
extern cheatseq_t cheat_keys;    // [STRIFE]: new cheat jimmy (all keys)
extern cheatseq_t cheat_stealth; // [STRIFE]: new cheat gripper
extern cheatseq_t cheat_midas;   // [STRIFE]: new cheat
extern cheatseq_t cheat_lego;    // [STRIFE]: new cheat
extern cheatseq_t cheat_dev;     // [STRIFE]: new cheat

extern cheatseq_t cheat_powerup[];


#endif
