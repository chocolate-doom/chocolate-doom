//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2017 Brad Harding
// Copyright(C) 2017-2022 Julia Nechaevskaya
// Copyright(C) 2017-2022 Fabian Greffrath
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
//	Brightmaps for walls, floors, sprites and weapon states
//	Adapted from doomretro/src/r_data.c:97-209
//

#include "crispy.h"
#include "doomdef.h"
#include "r_bmaps.h"

// [crispy] brightmap data

static const byte nobrightmap[256] = {0};

static const byte fullbright[256] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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

static const byte surfaces[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
};


static const byte consumables[256] =
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
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte hellstaff_world[256] =
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
    0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte hellstaff_attack[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

static const byte flame[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte ethereal[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte energy[256] =
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte iron_lich_1[256] =
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
    0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte iron_lich_2[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
};

const byte *dc_brightmap = nobrightmap;

// [crispy] brightmaps for textures

typedef struct
{
    const char *const texture;
    const byte *colormask;
} fullbright_t;

static const fullbright_t fullbright_walls[] = {
    // [crispy] common textures
    {"DOOREXIT", surfaces},
    {"GRSKULL3", surfaces},
    {"SW1ON",    surfaces},
    {"SW1OFF",   surfaces},
    {"SW2ON",    surfaces},
    {"SW2OFF",   surfaces},
};

const byte *R_BrightmapForTexName (const char *texname)
{
    int i;

    for (i = 0; i < arrlen(fullbright_walls); i++)
    {
        const fullbright_t *fullbright = &fullbright_walls[i];

        if (!strncasecmp(fullbright->texture, texname, 8))
        {
            return fullbright->colormask;
        }
    }

    return nobrightmap;
}

// [crispy] brightmaps for sprites

const byte *R_BrightmapForSprite (const int state)
{
    if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
    {
        switch (state)
        {
            // Enchanted Shield
            case S_ITEM_SHD2_1:
            // Morph Ovum
            case S_ARTI_EGGC1:
            case S_ARTI_EGGC2:
            case S_ARTI_EGGC3:
            case S_ARTI_EGGC4:
            // Ring of Invincibility
            case S_ARTI_INVU1:
            case S_ARTI_INVU2:
            case S_ARTI_INVU3:
            case S_ARTI_INVU4:
            // Chaos Device
            case S_ARTI_ATLP1:
            case S_ARTI_ATLP2:
            case S_ARTI_ATLP3:
            case S_ARTI_ATLP4:
            // Ethereal Crossbow
            case S_WBOW:
            // Phoenix Rod
            case S_WPHX:
            {
                return consumables;
                break;
            }
            // Hell Staff (world)
            case S_WSKL:
            {
                return hellstaff_world;
                break;
            }
            // Flame Orb
            case S_AMP1_1:
            case S_AMP1_2:
            case S_AMP1_3:
            // Infernal Orb
            case S_AMP2_1:
            case S_AMP2_2:
            case S_AMP2_3:
            // Chandeiler
            case S_CHANDELIER1:
            case S_CHANDELIER2:
            case S_CHANDELIER3:
            // Fire Brazier
            case S_FIREBRAZIER1:
            case S_FIREBRAZIER2:
            case S_FIREBRAZIER3:
            case S_FIREBRAZIER4:
            case S_FIREBRAZIER5:
            case S_FIREBRAZIER6:
            case S_FIREBRAZIER7:
            case S_FIREBRAZIER8:
            // Serpent Torch
            case S_SERPTORCH1:
            case S_SERPTORCH2:
            case S_SERPTORCH3:
            // Torch (artifact)
            case S_ARTI_TRCH1:
            case S_ARTI_TRCH2:
            case S_ARTI_TRCH3:
            // Volcano
            case S_VOLCANO1:
            case S_VOLCANO2:
            case S_VOLCANO3:
            case S_VOLCANO4:
            case S_VOLCANO5:
            case S_VOLCANO6:
            case S_VOLCANO7:
            case S_VOLCANO8:
            case S_VOLCANO9:
            // Sabreclaw (death states)
            case S_CLINK_DIE1:
            case S_CLINK_DIE2:
            case S_CLINK_DIE3:
            case S_CLINK_DIE4:
            case S_CLINK_DIE5:
            case S_CLINK_DIE6:
            {
                return flame;
                break;
            }
            // Iron Lich (idle and attack states)
            case S_HEAD_LOOK:
            case S_HEAD_FLOAT:
            case S_HEAD_ATK1:
            case S_HEAD_ATK2:
            {
                return iron_lich_1;
                break;
            }
            // Iron Lich (death states)
            case S_HEAD_DIE1:
            case S_HEAD_DIE2:
            case S_HEAD_DIE3:
            case S_HEAD_DIE4:
            case S_HEAD_DIE5:
            case S_HEAD_DIE6:
            {
                return iron_lich_2;
                break;
            }
            // Disciple of D'Sparil (attack, pain and death states)
            case S_WIZARD_ATK1:
            case S_WIZARD_ATK2:
            case S_WIZARD_ATK3:
            case S_WIZARD_ATK4:
            case S_WIZARD_ATK5:
            case S_WIZARD_ATK6:
            case S_WIZARD_ATK7:
            case S_WIZARD_ATK8:
            case S_WIZARD_ATK9:
            case S_WIZARD_PAIN1:
            case S_WIZARD_PAIN2:
            case S_WIZARD_DIE1:
            case S_WIZARD_DIE2:
            case S_WIZARD_DIE3:
            case S_WIZARD_DIE4:
            case S_WIZARD_DIE5:
            case S_WIZARD_DIE6:
            case S_WIZARD_DIE7:
            // D'Sparil on Serpent (death states)
            case S_SRCR1_DIE9:
            case S_SRCR1_DIE10:
            case S_SRCR1_DIE11:
            case S_SRCR1_DIE12:
            case S_SRCR1_DIE13:
            case S_SRCR1_DIE14:
            case S_SRCR1_DIE15:
            // Walking D'Sparil (attack states)
            case S_SOR2_ATK1:
            case S_SOR2_ATK2:
            case S_SOR2_ATK3:
            // Walking D'Sparil (death states)
            case S_SOR2_DIE1:
            case S_SOR2_DIE2:
            case S_SOR2_DIE3:
            case S_SOR2_DIE4:
            case S_SOR2_DIE5:
            case S_SOR2_DIE6:
            case S_SOR2_DIE7:
            case S_SOR2_DIE8:
            case S_SOR2_DIE9:
            {
                return energy;
                break;
            }

            // Initially unlit objects:
            // Weredragon fireball
            case S_BEASTBALL1:
            case S_BEASTBALL2:
            case S_BEASTBALL3:
            case S_BEASTBALL4:
            case S_BEASTBALL5:
            case S_BEASTBALL6:
            // Iron Lich flame wall
            case S_HEADFX3_1:
            case S_HEADFX3_2:
            case S_HEADFX3_3:
            case S_HEADFX3_4:
            case S_HEADFX3_5:
            case S_HEADFX3_6:
            case S_HEADFXI3_1:
            case S_HEADFXI3_2:
            case S_HEADFXI3_3:
            case S_HEADFXI3_4:
            // Volcano fireball
            case S_VOLCANOBALL1:
            case S_VOLCANOBALL2:
            case S_VOLCANOBALLX1:
            case S_VOLCANOBALLX2:
            case S_VOLCANOBALLX3:
            case S_VOLCANOBALLX4:
            case S_VOLCANOBALLX5:
            case S_VOLCANOBALLX6:
            case S_VOLCANOTBALL1:
            case S_VOLCANOTBALL2:
            case S_VOLCANOTBALLX1:
            case S_VOLCANOTBALLX2:
            case S_VOLCANOTBALLX3:
            case S_VOLCANOTBALLX4:
            case S_VOLCANOTBALLX5:
            case S_VOLCANOTBALLX6:
            case S_VOLCANOTBALLX7:
            {
                return fullbright;
                break;
            }
        }
    }
    else
    {
        switch (state)
        {
            // Fire Brazier
            case S_FIREBRAZIER1:
            case S_FIREBRAZIER2:
            case S_FIREBRAZIER3:
            case S_FIREBRAZIER4:
            case S_FIREBRAZIER5:
            case S_FIREBRAZIER6:
            case S_FIREBRAZIER7:
            case S_FIREBRAZIER8:
            // Torch (artifact)
            case S_ARTI_TRCH1:
            case S_ARTI_TRCH2:
            case S_ARTI_TRCH3:
            {
                return fullbright;
                break;
            }
        }
    }

    return nobrightmap;
}

// [crispy] brightmaps for flats

static int bmapflatnum[12];

const byte *R_BrightmapForFlatNum (const int num)
{
    if (crispy->brightmaps & BRIGHTMAPS_TEXTURES)
    {
        if (num == bmapflatnum[0]
        ||  num == bmapflatnum[1]
        ||  num == bmapflatnum[2]
        ||  num == bmapflatnum[3]
        ||  num == bmapflatnum[4])
        {
            return surfaces;
        }
    }

    return nobrightmap;
}

// [crispy] brightmaps for states

const byte *R_BrightmapForState (const int state)
{

    if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
    {
        switch (state)
        {
            // Gauntlets of the Necromancer
            case S_GAUNTLETATK1_3:
            case S_GAUNTLETATK1_4:
            case S_GAUNTLETATK1_5:
            case S_GAUNTLETATK1_6:
            case S_GAUNTLETATK1_7:
            {
                return consumables;
                break;
            }
            // Gauntlets of the Necromancer (powered)
            case S_GAUNTLETREADY2_1:
            case S_GAUNTLETREADY2_2:
            case S_GAUNTLETREADY2_3:
            case S_GAUNTLETATK2_1:
            case S_GAUNTLETATK2_2:
            case S_GAUNTLETATK2_3:
            case S_GAUNTLETATK2_4:
            case S_GAUNTLETATK2_5:
            case S_GAUNTLETATK2_6:
            case S_GAUNTLETATK2_7:
            {
                return flame;
                break;
            }
            // Elven Wand
            case S_GOLDWANDATK1_1:
            case S_GOLDWANDATK1_2:
            case S_GOLDWANDATK1_3:
            case S_GOLDWANDATK1_4:
            case S_GOLDWANDATK2_1:
            case S_GOLDWANDATK2_2:
            case S_GOLDWANDATK2_3:
            case S_GOLDWANDATK2_4:
            {
                return flame;
                break;
            }
            // Ethereal Crossbow
            case S_CRBOW1:
            case S_CRBOW2:
            case S_CRBOW3:
            case S_CRBOW4:
            case S_CRBOW5:
            case S_CRBOW6:
            case S_CRBOW7:
            case S_CRBOW8:
            case S_CRBOW9:
            case S_CRBOW10:
            case S_CRBOW11:
            case S_CRBOW12:
            case S_CRBOW13:
            case S_CRBOW14:
            case S_CRBOW15:
            case S_CRBOW16:
            case S_CRBOW17:
            case S_CRBOW18:
            case S_CRBOWDOWN:
            case S_CRBOWUP:
            case S_CRBOWATK1_1:
            case S_CRBOWATK1_2:
            case S_CRBOWATK1_3:
            case S_CRBOWATK1_4:
            case S_CRBOWATK1_5:
            case S_CRBOWATK1_6:
            case S_CRBOWATK1_7:
            case S_CRBOWATK1_8:
            case S_CRBOWATK2_1:
            case S_CRBOWATK2_2:
            case S_CRBOWATK2_3:
            case S_CRBOWATK2_4:
            case S_CRBOWATK2_5:
            case S_CRBOWATK2_6:
            case S_CRBOWATK2_7:
            case S_CRBOWATK2_8:
            {
                return ethereal;
                break;
            }
            // Dragon Claw
            case S_BLASTERATK1_1:
            case S_BLASTERATK1_2:
            case S_BLASTERATK1_3:
            case S_BLASTERATK1_4:
            case S_BLASTERATK1_5:
            case S_BLASTERATK1_6:
            case S_BLASTERATK2_1:
            case S_BLASTERATK2_2:
            case S_BLASTERATK2_3:
            case S_BLASTERATK2_4:
            case S_BLASTERATK2_5:
            case S_BLASTERATK2_6:
            {
                return energy;
                break;
            }
            // Hell Staff:
            case S_HORNRODATK1_1:
            case S_HORNRODATK1_2:
            case S_HORNRODATK1_3:
            case S_HORNRODATK2_1:
            case S_HORNRODATK2_2:
            case S_HORNRODATK2_3:
            case S_HORNRODATK2_4:
            case S_HORNRODATK2_5:
            case S_HORNRODATK2_6:
            case S_HORNRODATK2_7:
            case S_HORNRODATK2_8:
            case S_HORNRODATK2_9:
            {
                return hellstaff_attack;
                break;
            }
            // Phoenix Rod (idle)
            case S_PHOENIXREADY:
            {
                return consumables;
                break;
            }
            // Phoenix Rod (attack)
            case S_PHOENIXATK1_1:
            case S_PHOENIXATK1_2:
            case S_PHOENIXATK1_3:
            case S_PHOENIXATK1_4:
            case S_PHOENIXATK1_5:
            case S_PHOENIXATK2_1:
            case S_PHOENIXATK2_2:
            case S_PHOENIXATK2_3:
            case S_PHOENIXATK2_4:
            {
                return fullbright;
                break;
            }
        }
	}

    return nobrightmap;
}

// [crispy] initialize brightmaps

void R_InitBrightmaps ()
{
    // [crispy] only five select brightmapped flats
    bmapflatnum[0] = R_FlatNumForName("FLOOR21");
    bmapflatnum[1] = R_FlatNumForName("FLOOR22");
    bmapflatnum[2] = R_FlatNumForName("FLOOR23");
    bmapflatnum[3] = R_FlatNumForName("FLOOR24");
    bmapflatnum[4] = R_FlatNumForName("FLOOR26");
}
