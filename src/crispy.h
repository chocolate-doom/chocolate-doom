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

#define singleplayer (!crispy_democritical && !netgame)
#define uncrispy (demorecording || netgame)

extern boolean crispy_cleanscreenshot;
extern uint8_t crispy_coloredblood;
extern boolean crispy_democritical;
extern boolean crispy_flashinghom;
extern boolean crispy_fliplevels;
extern boolean crispy_havemap33;
extern boolean crispy_havessg;
extern boolean crispy_nwtmerge;
extern uint8_t crispy_pretrans;

extern int crispy_automapstats;
extern int crispy_coloredhud;
extern int crispy_crosshair;
extern int crispy_freeaim;
extern int crispy_freelook;
extern int crispy_jump;
extern int crispy_mouselook;
extern int crispy_overunder;
extern int crispy_recoil;
extern int crispy_secretmessage;
extern int crispy_translucency;

#endif

