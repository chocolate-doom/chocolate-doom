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
//   Duh.
// 


#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"
#include "tables.h"

//
// GAME
//
void G_DeathMatchSpawnPlayer (int playernum);

// [STRIFE] Removed episode parameter
void G_InitNew (skill_t skill, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
// [STRIFE] Removed episode parameter
void G_DeferedInitNew (skill_t skill, int map);

void G_DeferedPlayDemo (char* demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame (char* name);

void G_DoLoadGame (boolean userload);

// Called by M_Responder.
void G_SaveGame (int slot, char* description);

// Only called by startup code.
void G_RecordDemo (char* name);

void G_BeginRecording (void);

void G_PlayDemo (char* name);
void G_TimeDemo (char* name);
boolean G_CheckDemoStatus (void);

void G_RiftExitLevel(int map, int spot, angle_t angle); // [STRIFE]
void G_ExitLevel (int dest);
//void G_SecretExitLevel (void);

void G_StartFinale(void); // [STRIFE]

//void G_WorldDone (void);

boolean G_RiftCheat(int riftSpotNum); // [STRIFE]

// Read current data from inputs and build a player movement command.

void G_BuildTiccmd (ticcmd_t *cmd, int maketic);

void G_Ticker (void);
boolean G_Responder (event_t*	ev);

void G_ScreenShot (void);

void G_DrawMouseSpeedBox(void);

// [STRIFE]
boolean G_WriteSaveName(int slot, const char *charname);
void    G_ReadCurrent(const char *path);

extern int vanilla_savegame_limit;
extern int vanilla_demo_limit;
#endif
