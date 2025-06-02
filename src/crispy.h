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

#define CRISPY_FPSLIMIT_MAX 500

typedef struct
{
	// [crispy] "crispness" config variables
	int automapoverlay;
	int automaprotate;
	int automapstats;
	int bobfactor;
	int brightmaps;
	int btusetimer;
	int centerweapon;
	int coloredblood;
	int coloredhud;
	int crosshair;
	int crosshairhealth;
	int crosshairtarget;
	int crosshairtype;
	int crosshaircolor;
	int defaultskill;
	int demotimer;
	int demotimerdir;
	int demobar;
	int extautomap;
	int flipcorpses;
	int fpslimit;
	int freeaim;
	int freelook;
	int freelook_hh;
	int gamma;
	int hires;
	int jump;
	int leveltime;
	int mouselook;
	int neghealth;
	int overunder;
	int pitch;
	int playercoords;
	int secretmessage;
	int smoothlight;
	int smoothmap;
	int soundfix;
	int soundfull;
	int soundmono;
	int lvlwpnsnd;
	int statsformat;
	int translucency;
#ifdef CRISPY_TRUECOLOR
	int truecolor;
#endif
	int uncapped;
	int vsync;
	int widescreen;

	// [crispy] in-game switches and variables
	int screenshotmsg;
	int snowflakes;
	int cleanscreenshot;
	int demowarp;
	int fps;

	boolean flashinghom;
	boolean fliplevels;
	boolean flipweapons;
	boolean haved1e5;
	boolean haved1e6;
	boolean havee1m10;
	boolean havemap33;
	boolean havessg;
	boolean singleplayer;
	boolean stretchsky;

	// [crispy] custom difficulty parameters
	boolean autohealth;
	boolean fast;
	boolean keysloc;
	boolean moreammo;
	boolean pistolstart;

	char *havenerve;
	char *havemaster;
	char *havesigil;
	char *havesigil2;

	const char *sdlversion;
	const char *platform;

	void (*post_rendering_hook) (void);
} crispy_t;

extern crispy_t *const crispy;
extern const crispy_t *critical;

extern void CheckCrispySingleplayer (boolean singleplayer);

enum
{
	REINIT_FRAMEBUFFERS = 1,
	REINIT_RENDERER = 2,
	REINIT_TEXTURES = 4,
	REINIT_ASPECTRATIO = 8,
};

enum
{
    BOBFACTOR_FULL,
    BOBFACTOR_75,
    BOBFACTOR_OFF,
    NUM_BOBFACTORS,
};

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
	COLOREDBLOOD_OFF,
	COLOREDBLOOD_BLOOD,
	COLOREDBLOOD_ALL,
	NUM_COLOREDBLOOD,
};

enum
{
    CENTERWEAPON_OFF,
    CENTERWEAPON_CENTER,
    CENTERWEAPON_BOB,
    NUM_CENTERWEAPON,
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
    CROSSHAIR_HE_OFF,
    CROSSHAIR_HE_OPAQUE,
    CROSSHAIR_HE_TRANSLUCENT,
    NUM_HE_CROSSHAIRS
};

enum
{
    CROSSHAIRTYPE_HE_DOT,
    CROSSHAIRTYPE_HE_CROSS1,
    CROSSHAIRTYPE_HE_CROSS2,
    NUM_HE_CROSSHAIRTYPE
};

enum
{
    CROSSHAIRCOLOR_HE_GOLD,
    CROSSHAIRCOLOR_HE_WHITE,
    CROSSHAIRCOLOR_HE_FSHUD,
    NUM_HE_CROSSHAIRCOLOR
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
    FREELOOK_HH_LOCK,
    FREELOOK_HH_SPRING,
    NUM_FREELOOKS_HH
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
    TRANSLUCENCY_OFF,
    TRANSLUCENCY_MISSILE,
    TRANSLUCENCY_ITEM,
    TRANSLUCENCY_BOTH,
    NUM_TRANSLUCENCY
};

enum
{
    SECRETMESSAGE_OFF,
    SECRETMESSAGE_ON,
    SECRETMESSAGE_COUNT,
    NUM_SECRETMESSAGE
};

enum
{
    WIDGETS_OFF,
    WIDGETS_AUTOMAP,
    WIDGETS_ALWAYS,
    WIDGETS_STBAR,
    NUM_WIDGETS
};

enum
{
    STATSFORMAT_RATIO,
    STATSFORMAT_REMAINING,
    STATSFORMAT_PERCENT,
    STATSFORMAT_BOOLEAN,
    NUM_STATSFORMATS
};

enum
{
    SKILL_ITYTD,
    SKILL_HNTR,
    SKILL_HMP,
    SKILL_UV,
    SKILL_NIGHTMARE,
    NUM_SKILLS
};

enum
{
    RATIO_ORIG,
    RATIO_MATCH_SCREEN,
    RATIO_16_10,
    RATIO_16_9,
    RATIO_21_9,
    NUM_RATIOS
};

#endif
