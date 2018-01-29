//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 Fabian Greffrath
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

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef BETWEEN
#define BETWEEN(l,u,x) (((l)>(x))?(l):((x)>(u))?(u):(x))
#endif

typedef struct
{
	// [crispy] "crispness" config variables
	int automapstats;
	int brightmaps;
	int centerweapon;
	int coloredblood;
	int coloredhud;
	int crosshair;
	int crosshairtype;
	int demotimer;
	int demotimerdir;
	int demobar;
	int extautomap;
	int extsaveg;
	int flipcorpses;
	int freeaim;
	int freelook;
	int jump;
	int mouselook;
	int neghealth;
	int overunder;
	int pitch;
	int recoil;
	int secretmessage;
	int smoothscaling;
	int soundfix;
	int soundfull;
	int translucency;
	int uncapped;
	int weaponsquat;

	// [crispy] in-game switches and variables
	int screenshotmsg;
	int cleanscreenshot;
	int demowarp;
	int fps;

	boolean automapoverlay;
	boolean flashinghom;
	boolean fliplevels;
	boolean havee1m10;
	boolean havemap33;
	boolean havessg;
	boolean singleplayer;
	boolean stretchsky;

	const char *sdlversion;
	const char *platform;
} crispy_t;

extern crispy_t *const crispy;
extern const crispy_t *critical;

extern void CheckCrispySingleplayer (boolean singleplayer);

enum
{
    BRIGHTMAPS_OFF,
    BRIGHTMAPS_TEXTURES,
    BRIGHTMAPS_SPRITES,
    BRIGHTMAPS_BOTH,
    NUM_BRIGHTMAPS,
};

enum
{
    CENTERWEAPON_OFF,
    CENTERWEAPON_HOR,
    CENTERWEAPON_HORVER,
    CENTERWEAPON_BOB,
    CENTERWEAPON_BOB2,
    NUM_CENTERWEAPON,
};

enum
{
    COLOREDBLOOD_OFF,
    COLOREDBLOOD_BLOOD,
    COLOREDBLOOD_CORPSE,
    COLOREDBLOOD_BOTH,
    NUM_COLOREDBLOOD,
    COLOREDBLOOD_FIX = 0x10
};

enum
{
    COLOREDHUD_OFF,
    COLOREDHUD_BAR,
    COLOREDHUD_TEXT,
    COLOREDHUD_BOTH,
    NUM_COLOREDHUD
};

enum
{
    CROSSHAIR_OFF,
    CROSSHAIR_STATIC,
    CROSSHAIR_PROJECTED,
    NUM_CROSSHAIRS,
    CROSSHAIR_INTERCEPT = 0x10
};

enum
{
    DEMOTIMER_OFF,
    DEMOTIMER_RECORD,
    DEMOTIMER_PLAYBACK,
    DEMOTIMER_BOTH,
    NUM_DEMOTIMERS
};

enum
{
    FREEAIM_AUTO,
    FREEAIM_DIRECT,
    FREEAIM_BOTH,
    NUM_FREEAIMS
};

enum
{
    FREELOOK_OFF,
    FREELOOK_SPRING,
    FREELOOK_LOCK,
    NUM_FREELOOKS
};

enum
{
    JUMP_OFF,
    JUMP_LOW,
    JUMP_HIGH,
    NUM_JUMPS
};

enum
{
    NEGHEALTH_OFF,
    NEGHEALTH_DM,
    NEGHEALTH_ON,
    NUM_NEGHEALTHS
};

enum
{
    TRANSLUCENCY_OFF,
    TRANSLUCENCY_MISSILE,
    TRANSLUCENCY_ITEM,
    TRANSLUCENCY_BOTH,
    NUM_TRANSLUCENCY
};

enum
{
    UNCAPPED_OFF,
    UNCAPPED_ON,
    UNCAPPED_VSYNC,
    NUM_UNCAPPED
};

#endif
