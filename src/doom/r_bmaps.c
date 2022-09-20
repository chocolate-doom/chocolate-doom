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

static const byte nobrightmap[256] = {0};

static const byte notgray[256] =
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

static const byte notgrayorbrown[256] =
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

static const byte notgrayorbrown2[256] =
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
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const byte bluegreenbrownred[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte bluegreenbrown[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte blueandorange[256] =
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
	0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte redonly[256] =
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

static const byte redonly2[256] =
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte greenonly1[256] =
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

static const byte greenonly2[256] =
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

static const byte greenonly3[256] =
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

static const byte yellowonly[256] =
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

static const byte redandgreen[256] =
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

static const byte blueandgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte brighttan[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0,
	1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
	0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// [crispy] Chex Quest's "locked" door switches

static const byte chexred[256] =
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// [crispy] Chex Quest's "open" door switches

static const byte chexgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// [crispy] Chex Quest's "lock"/"open" knobs

static const byte chexredgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte hacxlightning[256] =
{
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const byte *dc_brightmap = nobrightmap;

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
	const byte *colormask;
} fullbright_t;

static const fullbright_t fullbright_doom[] = {
	// [crispy] common textures
	{"COMP2",    DOOM1AND2, blueandgreen},
	{"COMPSTA1", DOOM1AND2, notgray},
	{"COMPSTA2", DOOM1AND2, notgray},
	{"COMPUTE1", DOOM1AND2, bluegreenbrownred},
	{"COMPUTE2", DOOM1AND2, bluegreenbrown},
	{"COMPUTE3", DOOM1AND2, blueandorange},
	{"EXITSIGN", DOOM1AND2, notgray},
	{"EXITSTON", DOOM1AND2, redonly},
	{"PLANET1",  DOOM1AND2, notgray},
	{"SILVER2",  DOOM1AND2, notgray},
	{"SILVER3",  DOOM1AND2, notgrayorbrown2},
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
	// [crispy] Special case: fewer colors lit.
	{"SW2HOT",   DOOM1AND2, redonly2},
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
	{"SW2SATYR", DOOM1AND2, brighttan},
	{"SW2LION",  DOOM1AND2, brighttan},
	{"SW2GARG",  DOOM1AND2, brighttan},
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
	{"BIGDOOR1", DOOM1AND2, greenonly3},
//	{"BIGDOOR4", DOOM1AND2, greenonly3}, // C1: some stray green pixels, C2: many stray green pixels
//	{"BRNBIGL",  DOOM1AND2, greenonly3},
//	{"BRNBIGR",  DOOM1AND2, greenonly3}, // C1, C2: one stray green pixel
//	{"BRNSMAL2", DOOM1AND2, greenonly3}, // C1, C2: many stray green pixels
	{"COMP2",    DOOM1AND2, notgray},
//	{"COMPTALL", DOOM1ONLY, notgray},
//	{"COMPTALL", DOOM2ONLY, greenonly3}, // C2: many stray green pixels
	{"COMPUTE2", DOOM1AND2, notgray},
	{"LITE5",    DOOM1ONLY, greenonly2},
	{"STARTAN3", DOOM1AND2, greenonly2},
	{"SW1BRCOM", DOOM1AND2, chexred},
	{"SW1BRN1",  DOOM1AND2, chexgreen},
	{"SW1BRN2",  DOOM1AND2, chexred},
	{"SW1BRNGN", DOOM1AND2, chexred},
	{"SW1BROWN", DOOM1AND2, chexred},
	{"SW1COMM",  DOOM1AND2, chexred},
	{"SW1COMP",  DOOM1AND2, chexred},
	{"SW1DIRT",  DOOM1AND2, chexgreen},
	{"SW1METAL", DOOM1AND2, chexredgreen},
	{"SW1PIPE",  DOOM1AND2, chexgreen},
	{"SW1STARG", DOOM1AND2, chexred},
	{"SW1STON1", DOOM1AND2, chexred},
	{"SW1STRTN", DOOM1AND2, chexred},
	{"SW2BRCOM", DOOM1AND2, chexgreen},
	{"SW2BRN1",  DOOM1AND2, chexred},
	{"SW2BRN2",  DOOM1AND2, chexgreen},
	{"SW2BRNGN", DOOM1AND2, chexgreen},
	{"SW2BROWN", DOOM1AND2, chexgreen},
	{"SW2COMM",  DOOM1AND2, chexgreen},
	{"SW2COMP",  DOOM1AND2, chexgreen},
	{"SW2DIRT",  DOOM1AND2, chexred},
	{"SW2METAL", DOOM1AND2, chexredgreen},
	{"SW2PIPE",  DOOM1AND2, chexred},
	{"SW2STARG", DOOM1AND2, chexgreen},
	{"SW2STON1", DOOM1AND2, chexgreen},
	{"SW2STONE", DOOM1AND2, chexgreen},
	{"SW2STRTN", DOOM1AND2, chexgreen},
//	{"BIGDOOR5", DOOM1AND2, greenonly1}, // C1, C2: some stray green pixels
//	{"BIGDOOR6", DOOM1AND2, greenonly1}, // C1, C2: some stray green pixels
	{"CEMENT3",  DOOM1AND2, greenonly3},
	{"SKINFACE", DOOM1AND2, greenonly1},
	{"SKINTEK1", DOOM1ONLY, greenonly1},
	{"SKSPINE2", DOOM1AND2, greenonly3},
	{"SW1BLUE",  DOOM1AND2, chexgreen},
	{"SW1HOT",   DOOM1AND2, chexgreen},
	{"SW1SKIN",  DOOM1AND2, chexgreen},
	{"SW1VINE",  DOOM1ONLY, chexgreen}, // C1: some stray green pixels in the vines
	{"SW1WOOD",  DOOM1AND2, chexgreen},
	{"SW2BLUE",  DOOM1AND2, chexred},
	{"SW2CMT",   DOOM1AND2, chexgreen},
	{"SW2GSTON", DOOM1AND2, chexred},
	{"SW2HOT",   DOOM1AND2, chexred},
	{"SW2SKIN",  DOOM1AND2, chexred},
	{"SW2VINE",  DOOM1ONLY, chexred},
	{"SW2WOOD",  DOOM1AND2, chexred},
	{"WOOD4",    DOOM1AND2, chexredgreen},
	{"WOODGARG", DOOM1AND2, chexred},
	{"WOODSKUL", DOOM1AND2, chexredgreen},
};

static const fullbright_t fullbright_hacx[] = {
//	{"BFALL1",   DOOM2ONLY, redandgreen},
//	{"BFALL2",   DOOM2ONLY, redandgreen},
//	{"BFALL3",   DOOM2ONLY, redandgreen},
//	{"BFALL4",   DOOM2ONLY, redandgreen},
	{"BRNSMALR", DOOM2ONLY, greenonly1},
	{"DOORRED",  DOOM2ONLY, redandgreen},
	{"SLADWALL", DOOM2ONLY, chexred},
//	{"SW1BRCOM", DOOM2ONLY, redonly},
//	{"SW1BRN1",  DOOM2ONLY, redandgreen},
	{"SW1BRN2",  DOOM2ONLY, notgrayorbrown},
	{"SW1BRNGN", DOOM2ONLY, notgrayorbrown},
//	{"SW1BROWN", DOOM2ONLY, notgrayorbrown},
//	{"SW2BRCOM", DOOM2ONLY, greenonly1},
//	{"SW2BRN1",  DOOM2ONLY, redandgreen},
	{"SW2BRN2",  DOOM2ONLY, notgrayorbrown},
//	{"SW2BROWN", DOOM2ONLY, notgrayorbrown},
	{"COMPSPAN", DOOM2ONLY, greenonly1},
	{"COMPSTA1", DOOM2ONLY, notgrayorbrown},
//	{"COMPSTA2", DOOM2ONLY, notgrayorbrown},
	{"HD5",      DOOM2ONLY, redandgreen},
//	{"HD8",      DOOM2ONLY, redandgreen},
//	{"HD9",      DOOM2ONLY, redandgreen},
	{"BLAKWAL2", DOOM2ONLY, redandgreen},
	{"CEMENT7",  DOOM2ONLY, greenonly1},
	{"ROCK4",    DOOM2ONLY, redonly},
//	{"SLOPPY1",  DOOM2ONLY, notgrayorbrown},
//	{"SPCDOOR4", DOOM2ONLY, notgrayorbrown},
	{"ZZZFACE1", DOOM2ONLY, greenonly1},
	{"ZZZFACE2", DOOM2ONLY, redandgreen},
	{"HW166",    DOOM2ONLY, redandgreen},
	{"HW510",    DOOM2ONLY, notgrayorbrown},
	{"HW511",    DOOM2ONLY, notgrayorbrown},
	{"HW512",    DOOM2ONLY, notgrayorbrown},
};

static const byte *R_BrightmapForTexName_Doom (const char *texname)
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

static const byte *R_BrightmapForTexName_Chex (const char *texname)
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

static const byte *R_BrightmapForTexName_Hacx (const char *texname)
{
	int i;

	for (i = 0; i < arrlen(fullbright_hacx); i++)
	{
		const fullbright_t *fullbright = &fullbright_hacx[i];

		if (!strncasecmp(fullbright->texture, texname, 8))
		{
			return fullbright->colormask;
		}
	}

	return nobrightmap;
}

// [crispy] brightmaps for sprites

// [crispy] adapted from russian-doom/src/doom/r_things.c:617-639
static const byte *R_BrightmapForSprite_Doom (const int type)
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
				return greenonly2;
				break;
			}
			// Barrel
			case SPR_BAR1:
			{
				return greenonly3;
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

static const byte *R_BrightmapForSprite_Chex (const int type)
{
	// [crispy] TODO
	/*
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
	*/
	return nobrightmap;
}

static const byte *R_BrightmapForSprite_Hacx (const int type)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (type)
		{
			// Chainsaw
			case SPR_CSAW:
			// Plasmagun
			case SPR_PLAS:
			// Cell Charge
			case SPR_CELL:
			// Cell Charge Pack
			case SPR_CELP:
			{
				return redonly;
				break;
			}
			// Rocket launcher
			case SPR_LAUN:
			// Medikit
			case SPR_MEDI:
			{
				return redandgreen;
				break;
			}
			// Rocket
			case SPR_ROCK:
			// Box of rockets
			case SPR_BROK:
			{
				return greenonly1;
				break;
			}
			// Health Bonus
			case SPR_BON1:
			// Stimpack
			case SPR_STIM:
			{
				return notgrayorbrown;
				break;
			}
		}
	}

	return nobrightmap;
}

// [crispy] brightmaps for flats

static int bmapflatnum[12];

static const byte *R_BrightmapForFlatNum_Doom (const int num)
{
	if (crispy->brightmaps & BRIGHTMAPS_TEXTURES)
	{
		if (num == bmapflatnum[0] ||
		    num == bmapflatnum[1] ||
		    num == bmapflatnum[2])
		{
			return notgrayorbrown;
		}
	}

	return nobrightmap;
}

static const byte *R_BrightmapForFlatNum_Hacx (const int num)
{
	if (crispy->brightmaps & BRIGHTMAPS_TEXTURES)
	{
		if (num == bmapflatnum[0] ||
		    num == bmapflatnum[1] ||
		    num == bmapflatnum[2] ||
		    num == bmapflatnum[3] ||
		    num == bmapflatnum[4] ||
		    num == bmapflatnum[5] ||
		    num == bmapflatnum[9] ||
		    num == bmapflatnum[10] ||
		    num == bmapflatnum[11])
		{
			return notgrayorbrown;
		}

		if (num == bmapflatnum[6] ||
		    num == bmapflatnum[7] ||
		    num == bmapflatnum[8])
		{
			return greenonly1;
		}
	}

	return nobrightmap;
}

static const byte *R_BrightmapForFlatNum_None (const int num)
{
	return nobrightmap;
}

// [crispy] brightmaps for states

static const byte *R_BrightmapForState_Doom (const int state)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (state)
		{
			case S_BFG1:
			case S_BFG2:
			case S_BFG3:
			case S_BFG4:
			{
				return redonly;
				break;
			}
		}
	}

	return nobrightmap;
}

static const byte *R_BrightmapForState_Hacx (const int state)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (state)
		{
			case S_SAW2:
			case S_SAW3:
			{
				return hacxlightning;
				break;
			}
			case S_MISSILE:
			{
				return redandgreen;
				break;
			}
			case S_SAW:
			case S_SAWB:
			case S_PLASMA:
			case S_PLASMA2:
			{
				return redonly;
				break;
			}
		}
	}

	return nobrightmap;
}

static const byte *R_BrightmapForState_None (const int state)
{
	return nobrightmap;
}

// [crispy] initialize brightmaps

const byte *(*R_BrightmapForTexName) (const char *texname);
const byte *(*R_BrightmapForSprite) (const int type);
const byte *(*R_BrightmapForFlatNum) (const int num);
const byte *(*R_BrightmapForState) (const int state);

void R_InitBrightmaps ()
{
	if (gameversion == exe_hacx)
	{
		bmapflatnum[0] = R_FlatNumForName("FLOOR1_1");
		bmapflatnum[1] = R_FlatNumForName("FLOOR1_7");
		bmapflatnum[2] = R_FlatNumForName("FLOOR3_3");
		bmapflatnum[3] = R_FlatNumForName("NUKAGE1");
		bmapflatnum[4] = R_FlatNumForName("NUKAGE2");
		bmapflatnum[5] = R_FlatNumForName("NUKAGE3");
		bmapflatnum[6] = R_FlatNumForName("BLOOD1");
		bmapflatnum[7] = R_FlatNumForName("BLOOD2");
		bmapflatnum[8] = R_FlatNumForName("BLOOD3");
		bmapflatnum[9] = R_FlatNumForName("SLIME13");
		bmapflatnum[10] = R_FlatNumForName("SLIME14");
		bmapflatnum[11] = R_FlatNumForName("SLIME15");

		R_BrightmapForTexName = R_BrightmapForTexName_Hacx;
		R_BrightmapForSprite = R_BrightmapForSprite_Hacx;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_Hacx;
		R_BrightmapForState = R_BrightmapForState_Hacx;
	}
	else
	if (gameversion == exe_chex)
	{
		int lump;

		// [crispy] detect Chex Quest 2
		lump = W_CheckNumForName("INTERPIC");
		if (!strcasecmp(W_WadNameForLump(lumpinfo[lump]), "chex2.wad"))
		{
			chex2 = true;
		}

		R_BrightmapForTexName = R_BrightmapForTexName_Chex;
		R_BrightmapForSprite = R_BrightmapForSprite_Chex;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_None;
		R_BrightmapForState = R_BrightmapForState_None;
	}
	else
	{
		// [crispy] only three select brightmapped flats
		bmapflatnum[0] = R_FlatNumForName("CONS1_1");
		bmapflatnum[1] = R_FlatNumForName("CONS1_5");
		bmapflatnum[2] = R_FlatNumForName("CONS1_7");

		R_BrightmapForTexName = R_BrightmapForTexName_Doom;
		R_BrightmapForSprite = R_BrightmapForSprite_Doom;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_Doom;
		R_BrightmapForState = R_BrightmapForState_Doom;
	}
}
