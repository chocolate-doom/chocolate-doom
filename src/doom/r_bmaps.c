//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2017 Brad Harding
// Copyright(C) 2017 Fabian Greffrath
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Brightmaps for wall textures
//	Adapted from doomretro/src/r_data.c:97-209
//

#include "doomtype.h"
#include "doomstat.h"
#include "r_data.h"
#include "w_wad.h"

// [crispy] brightmap data

static byte nobrightmap[256] = {0};

static byte notgray[256] =
{
	0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static byte notgrayorbrown[256] =
{
	0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static byte redonly[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte greenonly1[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte greenonly2[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte greenonly3[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte yellowonly[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
};

static byte redandgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

byte *dc_brightmap = nobrightmap;

// [crispy] brightmaps for textures

enum
{
	DOOM1AND2,
	DOOM1ONLY,
	DOOM2ONLY,
};

typedef struct
{
	const char *const texture;
	const int game;
	byte *colormask;
} fullbright_t;

static const fullbright_t fullbright_doom[] = {
	// [crispy] common textures
	{"COMP2",    DOOM1AND2, notgrayorbrown},
	{"COMPSTA1", DOOM1AND2, notgray},
	{"COMPSTA2", DOOM1AND2, notgray},
	{"COMPUTE1", DOOM1AND2, notgrayorbrown},
	{"COMPUTE2", DOOM1AND2, notgrayorbrown},
	{"COMPUTE3", DOOM1AND2, notgrayorbrown},
	{"EXITSIGN", DOOM1AND2, notgray},
	{"EXITSTON", DOOM1AND2, redonly},
	{"PLANET1",  DOOM1AND2, notgray},
	{"SILVER2",  DOOM1AND2, notgray},
	{"SILVER3",  DOOM1AND2, notgrayorbrown},
	{"SLADSKUL", DOOM1AND2, redonly},
	{"SW1BRCOM", DOOM1AND2, redonly},
	{"SW1BRIK",  DOOM1AND2, redonly},
	{"SW1BRN1",  DOOM2ONLY, redonly},
	{"SW1COMM",  DOOM1AND2, redonly},
	{"SW1DIRT",  DOOM1AND2, redonly},
	{"SW1MET2",  DOOM1AND2, redonly},
	{"SW1STARG", DOOM2ONLY, redonly},
	{"SW1STON1", DOOM1AND2, redonly},
	{"SW1STON2", DOOM2ONLY, redonly},
	{"SW1STONE", DOOM1AND2, redonly},
	{"SW1STRTN", DOOM1AND2, redonly},
	{"SW2BLUE",  DOOM1AND2, redonly},
	{"SW2BRCOM", DOOM1AND2, greenonly2},
	{"SW2BRIK",  DOOM1AND2, greenonly1},
	{"SW2BRN1",  DOOM1AND2, greenonly2},
	{"SW2BRN2",  DOOM1AND2, greenonly1},
	{"SW2BRNGN", DOOM1AND2, greenonly3},
	{"SW2COMM",  DOOM1AND2, greenonly1},
	{"SW2COMP",  DOOM1AND2, redonly},
	{"SW2DIRT",  DOOM1AND2, greenonly2},
	{"SW2EXIT",  DOOM1AND2, notgray},
	{"SW2GRAY",  DOOM1AND2, notgray},
	{"SW2GRAY1", DOOM1AND2, notgray},
	{"SW2GSTON", DOOM1AND2, redonly},
	{"SW2MARB",  DOOM2ONLY, redonly},
	{"SW2MET2",  DOOM1AND2, greenonly1},
	{"SW2METAL", DOOM1AND2, greenonly3},
	{"SW2MOD1",  DOOM1AND2, greenonly1},
	{"SW2PANEL", DOOM1AND2, redonly},
	{"SW2ROCK",  DOOM1AND2, redonly},
	{"SW2SLAD",  DOOM1AND2, redonly},
	{"SW2STARG", DOOM2ONLY, greenonly2},
	{"SW2STON1", DOOM1AND2, greenonly3},
	// [crispy] beware!
	{"SW2STON2", DOOM1ONLY, redonly},
	{"SW2STON2", DOOM2ONLY, greenonly2},
	{"SW2STON6", DOOM1AND2, redonly},
	{"SW2STONE", DOOM1AND2, greenonly2},
	{"SW2STRTN", DOOM1AND2, greenonly1},
	{"SW2TEK",   DOOM1AND2, greenonly1},
	{"SW2VINE",  DOOM1AND2, greenonly1},
	{"SW2WOOD",  DOOM1AND2, redonly},
	{"SW2ZIM",   DOOM1AND2, redonly},
	{"WOOD4",    DOOM1AND2, redonly},
	{"WOODGARG", DOOM1AND2, redonly},
	{"WOODSKUL", DOOM1AND2, redonly},
//	{"ZELDOOR",  DOOM1AND2, redonly},
	{"LITEBLU1", DOOM1AND2, notgray},
	{"LITEBLU2", DOOM1AND2, notgray},
	{"SPCDOOR3", DOOM2ONLY, greenonly1},
	{"PIPEWAL1", DOOM2ONLY, greenonly1},
	{"TEKLITE2", DOOM2ONLY, greenonly1},
	{"TEKBRON2", DOOM2ONLY, yellowonly},
//	{"SW2SKULL", DOOM2ONLY, greenonly2},
	// [crispy] Final Doom textures
	// TNT - Evilution exclusive
	{"PNK4EXIT", DOOM2ONLY, redonly},
	{"SLAD2",    DOOM2ONLY, notgrayorbrown},
	{"SLAD3",    DOOM2ONLY, notgrayorbrown},
	{"SLAD4",    DOOM2ONLY, notgrayorbrown},
	{"SLAD5",    DOOM2ONLY, notgrayorbrown},
	{"SLAD6",    DOOM2ONLY, notgrayorbrown},
	{"SLAD7",    DOOM2ONLY, notgrayorbrown},
	{"SLAD8",    DOOM2ONLY, notgrayorbrown},
	{"SLAD9",    DOOM2ONLY, notgrayorbrown},
	{"SLAD10",   DOOM2ONLY, notgrayorbrown},
	{"SLAD11",   DOOM2ONLY, notgrayorbrown},
	{"SLADRIP1", DOOM2ONLY, notgrayorbrown},
	{"SLADRIP3", DOOM2ONLY, notgrayorbrown},
	{"M_TEC",    DOOM2ONLY, greenonly2},
	{"LITERED2", DOOM2ONLY, redonly},
	{"BTNTMETL", DOOM2ONLY, notgrayorbrown},
	{"BTNTSLVR", DOOM2ONLY, notgrayorbrown},
	{"LITEYEL2", DOOM2ONLY, yellowonly},
	{"LITEYEL3", DOOM2ONLY, yellowonly},
	{"YELMETAL", DOOM2ONLY, yellowonly},
	// Plutonia exclusive
//	{"SW2SKULL", DOOM2ONLY, redonly},
};

static const fullbright_t fullbright_chex[] = {
	{"BIGDOOR1", DOOM1AND2, greenonly1},
	{"BIGDOOR4", DOOM1AND2, greenonly1},
	{"BRNBIGL",  DOOM1AND2, greenonly1},
	{"BRNBIGR",  DOOM1AND2, greenonly1},
	{"BRNSMAL2", DOOM1AND2, greenonly1},
	{"COMP2",    DOOM1AND2, notgray},
//	{"COMPTALL", DOOM1ONLY, notgray}, // hmm... probably not safe
	{"COMPTALL", DOOM2ONLY, greenonly1},
	{"COMPUTE2", DOOM1AND2, notgray},
	{"LITE5",    DOOM1ONLY, greenonly1},
	{"STARTAN3", DOOM1AND2, greenonly1},
	{"SW1BRCOM", DOOM1AND2, redonly},
	{"SW1BRN1",  DOOM1AND2, greenonly1},
	{"SW1BRN2",  DOOM1AND2, redonly},
	{"SW1BRNGN", DOOM1AND2, redonly},
	{"SW1BROWN", DOOM1ONLY, redonly},
	{"SW1COMM",  DOOM1AND2, redonly},
	{"SW1COMP",  DOOM1ONLY, redonly},
	{"SW1DIRT",  DOOM1AND2, greenonly1},
	{"SW1METAL", DOOM1AND2, redonly},
	{"SW1PIPE",  DOOM1AND2, greenonly1},
	{"SW1STARG", DOOM1AND2, redonly},
	{"SW1STON1", DOOM1AND2, redonly},
	{"SW1STRTN", DOOM1AND2, redonly},
	{"SW2BRCOM", DOOM1AND2, greenonly1},
	{"SW2BRN1",  DOOM1AND2, redonly},
	{"SW2BRN2",  DOOM1AND2, greenonly1},
	{"SW2BRNGN", DOOM1AND2, greenonly1},
	{"SW2BROWN", DOOM1AND2, greenonly1},// beware C2!
	{"SW2COMM",  DOOM1AND2, greenonly1},
	{"SW2COMP", DOOM1AND2, greenonly1},
	{"SW2DIRT",  DOOM1AND2, redonly},
	{"SW2METAL", DOOM1AND2, greenonly1},
	{"SW2PIPE",  DOOM1AND2, redonly},
	{"SW2STARG", DOOM1AND2, greenonly1},
	{"SW2STON1", DOOM1AND2, greenonly1},
	{"SW2STONE", DOOM1AND2, greenonly1},
	{"SW2STRTN", DOOM1AND2, greenonly1},
	{"BIGDOOR5", DOOM1AND2, greenonly1},
	{"BIGDOOR6", DOOM1AND2, greenonly1},
	{"CEMENT3",  DOOM1AND2, greenonly1},
	{"SKINFACE", DOOM1AND2, greenonly1},
	{"SKINTEK1", DOOM1ONLY, greenonly1},
	{"SKSPINE2", DOOM1AND2, greenonly1},
	{"SW1BLUE",  DOOM1AND2, greenonly1},
	{"SW1HOT",   DOOM1AND2, greenonly1},
	{"SW1SKIN",  DOOM1AND2, greenonly1},
//	{"SW1VINE",  DOOM1ONLY, greenonly1}, // not safe for green vines!
	{"SW1WOOD",  DOOM1AND2, greenonly1},
	{"SW2BLUE",  DOOM1AND2, redonly},
	{"SW2CMT",   DOOM1AND2, greenonly1},
	{"SW2GSTON", DOOM1AND2, redonly},
	{"SW2HOT",   DOOM1AND2, redonly},
	{"SW2SKIN",  DOOM1AND2, redonly},
	{"SW2VINE",  DOOM1ONLY, redonly},
	{"SW2WOOD",  DOOM1AND2, redonly},
	{"WOOD4",    DOOM1AND2, notgray},
	{"WOODGARG", DOOM1AND2, redonly},
	{"WOODSKUL", DOOM1AND2, greenonly1},
};

static byte *R_BrightmapForTexName_Doom (const char *texname)
{
	int i;

	for (i = 0; i < arrlen(fullbright_doom); i++)
	{
		const fullbright_t *fullbright = &fullbright_doom[i];

		if ((gamemission == doom && fullbright->game == DOOM2ONLY) ||
		    (gamemission != doom && fullbright->game == DOOM1ONLY))
		{
			continue;
		}

		if (!strncasecmp(fullbright->texture, texname, 8))
		{
			return fullbright->colormask;
		}
	}

	return nobrightmap;
}

static boolean chex2 = false;

static byte *R_BrightmapForTexName_Chex (const char *texname)
{
	int i;

	for (i = 0; i < arrlen(fullbright_chex); i++)
	{
		const fullbright_t *fullbright = &fullbright_chex[i];

		if ((chex2 && fullbright->game == DOOM1ONLY) ||
		    (!chex2 && fullbright->game == DOOM2ONLY))
		{
			continue;
		}

		if (!strncasecmp(fullbright->texture, texname, 8))
		{
			return fullbright->colormask;
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForTexName_None (const char *texname)
{
	return nobrightmap;
}

// [crispy] brightmaps for sprites

// [crispy] adapted from russian-doom/src/doom/r_things.c:617-639
static byte *R_BrightmapForSprite_Doom (const int type)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (type)
		{
			// Armor Bonus
			case SPR_BON2:
			// Cell Charge
			case SPR_CELL:
			{
				return greenonly1;
				break;
			}
			// Cell Charge Pack
			case SPR_CELP:
			{
				return yellowonly;
				break;
			}
			// BFG9000
			case SPR_BFUG:
			// Plasmagun
			case SPR_PLAS:
			{
				return redonly;
				break;
			}
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForSprite_Chex (const int type)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (type)
		{
			// Chainsaw
			case SPR_CSAW:
			// Shotgun
			case SPR_SHOT:
			// Chaingun
			case SPR_MGUN:
			// Rocket launcher
			case SPR_LAUN:
			// Plasmagun
			case SPR_PLAS:
			// BFG9000
			case SPR_BFUG:
			{
				return redandgreen;
				break;
			}
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForSprite_None (const int type)
{
	return nobrightmap;
}

// [crispy] brightmaps for flats

static int bmapflatnum1, bmapflatnum2, bmapflatnum3;

static byte *R_BrightmapForFlatNum_Doom (const int num)
{
	if (crispy->brightmaps & BRIGHTMAPS_TEXTURES)
	{
		if (num == bmapflatnum1 ||
		    num == bmapflatnum2 ||
		    num == bmapflatnum3)
		{
			return notgrayorbrown;
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForFlatNum_None (const int num)
{
	return nobrightmap;
}

// [crispy] initialize brightmaps

byte *(*R_BrightmapForTexName) (const char *texname);
byte *(*R_BrightmapForSprite) (const int type);
byte *(*R_BrightmapForFlatNum) (const int type);

void R_InitBrightmaps (void)
{
	if (gameversion == exe_hacx)
	{
		R_BrightmapForTexName = R_BrightmapForTexName_None;
		R_BrightmapForSprite = R_BrightmapForSprite_None;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_None;
	}
	else
	if (gameversion == exe_chex)
	{
		int lump;

		// [crispy] detect Chex Quest 2
		lump = W_CheckNumForName("INTERPIC");
		if (!strcasecmp(lumpinfo[lump]->wad_file->basename, "chex2.wad"))
		{
			chex2 = true;
		}

		R_BrightmapForTexName = R_BrightmapForTexName_Chex;
		R_BrightmapForSprite = R_BrightmapForSprite_Chex;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_None;
	}
	else
	{
		// [crispy] only three select brightmapped flats
		bmapflatnum1 = R_FlatNumForName("CONS1_1");
		bmapflatnum2 = R_FlatNumForName("CONS1_5");
		bmapflatnum3 = R_FlatNumForName("CONS1_7");

		R_BrightmapForTexName = R_BrightmapForTexName_Doom;
		R_BrightmapForSprite = R_BrightmapForSprite_Doom;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_Doom;
	}
}
