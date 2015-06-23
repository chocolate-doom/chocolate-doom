//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath
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
//	Crispy Doom specific variables.
//


#ifndef __CRISPY_H__
#define __CRISPY_H__

#include "doomtype.h"

#define CRISPY_HUD 12
#define CRISPY_CROSSHAIR "STCFN043"

#define singleplayer (!demorecording && !demoplayback && !netgame)

extern boolean crispy_automapoverlay;
extern boolean crispy_cleanscreenshot;
extern boolean crispy_flashinghom;
extern boolean crispy_fliplevels;
extern boolean crispy_havee1m10;
extern boolean crispy_havemap33;
extern boolean crispy_havessg;
extern boolean crispy_nwtmerge;
extern boolean crispy_showfps;

#define crispy_stretchsky (crispy_freelook || crispy_mouselook || crispy_pitch)

extern int crispy_automapstats;
extern int crispy_centerweapon;
extern int crispy_coloredblood;
extern int crispy_coloredblood2;
extern int crispy_coloredhud;
extern int crispy_crosshair;
extern int crispy_flipcorpses;
extern int crispy_freeaim;
extern int crispy_freelook;
extern int crispy_jump;
extern int crispy_mouselook;
extern int crispy_overunder;
extern int crispy_pitch;
extern int crispy_recoil;
extern int crispy_secretmessage;
extern int crispy_translucency;
extern int crispy_uncapped;

extern int crispy_demowarp;

enum
{
    JUMP_OFF,
    JUMP_LOW,
    JUMP_HIGH,
    NUM_JUMPS
};

enum
{
    CROSSHAIR_OFF,
    CROSSHAIR_STATIC,
    CROSSHAIR_PROJECTED,
    NUM_CROSSHAIRS,
    CROSSHAIR_INTERCEPT = 0x10
};

#endif
