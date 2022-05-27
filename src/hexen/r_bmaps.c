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
#include "h2def.h"
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

static const byte surfaces1[256] =
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
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
    0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte surfaces2[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte artifacts[256] =
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
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte flame[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte firebull[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte greenonly[256] =
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte blueonly[256] =
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
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
    {"SPAWN03", surfaces1},
    {"SPAWN09", surfaces1},
    {"SPAWN10", surfaces1},
    {"SW_1_DN", surfaces1},
    {"SW_1_MD", surfaces1},
    {"SW_2_DN", surfaces1},
    {"SW_2_MD", surfaces1},
    {"SPAWN12", surfaces2},
    {"SW51_ON", surfaces2},
    {"SW52_ON", surfaces2},
    {"X_FAC01", surfaces1},
    {"X_FAC02", surfaces1},
    {"X_FAC03", surfaces1},
    {"X_FAC04", surfaces1},
    {"X_FAC05", surfaces1},
    {"X_FAC06", surfaces1},
    {"X_FAC07", surfaces1},
    {"X_FAC08", surfaces1},
    {"X_FAC09", surfaces1},
    {"X_FAC10", surfaces1},
    {"X_FAC11", surfaces1},
    {"X_FAC12", surfaces1},

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
            // Banishment Device
            case S_ARTI_TELOTHER1:
            case S_ARTI_TELOTHER2:
            case S_ARTI_TELOTHER3:
            case S_ARTI_TELOTHER4:
            // Boots of Speed
            case S_ARTI_BOOTS1:
            case S_ARTI_BOOTS2:
            case S_ARTI_BOOTS3:
            case S_ARTI_BOOTS4:
            case S_ARTI_BOOTS5:
            case S_ARTI_BOOTS6:
            case S_ARTI_BOOTS7:
            case S_ARTI_BOOTS8:
            // Chaos Device
            case S_ARTI_ATLP1:
            case S_ARTI_ATLP2:
            case S_ARTI_ATLP3:
            case S_ARTI_ATLP4:
            // Dragonskin Bracers
            case S_ARTI_ARMOR1:
            case S_ARTI_ARMOR2:
            case S_ARTI_ARMOR3:
            case S_ARTI_ARMOR4:
            case S_ARTI_ARMOR5:
            case S_ARTI_ARMOR6:
            case S_ARTI_ARMOR7:
            case S_ARTI_ARMOR8:
            // Icon of the Defender
            case S_ARTI_INVU1:
            case S_ARTI_INVU2:
            case S_ARTI_INVU3:
            case S_ARTI_INVU4:
            // Krater of Might
            case S_ARTI_MANA:
            // Mystic Ambit Incant
            case S_ARTI_HEALRAD1:
            case S_ARTI_HEALRAD2:
            case S_ARTI_HEALRAD3:
            case S_ARTI_HEALRAD4:
            case S_ARTI_HEALRAD5:
            case S_ARTI_HEALRAD6:
            case S_ARTI_HEALRAD7:
            case S_ARTI_HEALRAD8:
            case S_ARTI_HEALRAD9:
            case S_ARTI_HEALRAD0:
            case S_ARTI_HEALRADA:
            case S_ARTI_HEALRADB:
            case S_ARTI_HEALRADC:
            case S_ARTI_HEALRADD:
            case S_ARTI_HEALRADE:
            case S_ARTI_HEALRADF:
            // Torch
            case S_ARTI_TRCH1:
            case S_ARTI_TRCH2:
            case S_ARTI_TRCH3:
            // Flame Mask
            case S_ARTIPUZZSKULL2:
            {
                return artifacts;
                break;
            }
            // Chandeiler
            case S_ZCHANDELIER1:
            case S_ZCHANDELIER2:
            case S_ZCHANDELIER3:
            {
                return flame;
                break;
            }
            // Fire Bull
            case S_ZFIREBULL1:
            case S_ZFIREBULL2:
            case S_ZFIREBULL3:
            case S_ZFIREBULL4:
            case S_ZFIREBULL5:
            case S_ZFIREBULL6:
            case S_ZFIREBULL7:
            case S_ZFIREBULL_DEATH2:
            case S_ZFIREBULL_U:
            case S_ZFIREBULL_BIRTH2:
            {
                return firebull;
                break;
            }
            // Wendigo
            case S_ICEGUY_LOOK:
            case S_ICEGUY_DORMANT:
            case S_ICEGUY_WALK1:
            case S_ICEGUY_WALK2:
            case S_ICEGUY_WALK3:
            case S_ICEGUY_WALK4:
            case S_ICEGUY_ATK1:
            case S_ICEGUY_ATK2:
            case S_ICEGUY_ATK3:
            case S_ICEGUY_ATK4:
            case S_ICEGUY_PAIN1:
            // Reiver
            case S_WRAITH_ATK1_1:
            case S_WRAITH_ATK1_2:
            case S_WRAITH_ATK1_3:
            case S_WRAITH_ATK2_1:
            case S_WRAITH_ATK2_2:
            case S_WRAITH_ATK2_3:
            // Korax
            case S_KORAX_LOOK1:
            case S_KORAX_CHASE1:
            case S_KORAX_CHASE2:
            case S_KORAX_CHASE3:
            case S_KORAX_CHASE4:
            case S_KORAX_CHASE5:
            case S_KORAX_CHASE6:
            case S_KORAX_CHASE7:
            case S_KORAX_CHASE8:
            case S_KORAX_CHASE9:
            case S_KORAX_CHASE0:
            case S_KORAX_CHASEA:
            case S_KORAX_CHASEB:
            case S_KORAX_CHASEC:
            case S_KORAX_CHASED:
            case S_KORAX_CHASEE:
            case S_KORAX_CHASEF:
            case S_KORAX_PAIN1:
            case S_KORAX_PAIN2:
            {
                return surfaces1;
                break;
            }
        }            
    }
    else
    {
        switch (state)
        {
            // Boots of Speed
            case S_ARTI_BOOTS1:
            case S_ARTI_BOOTS2:
            case S_ARTI_BOOTS3:
            case S_ARTI_BOOTS4:
            case S_ARTI_BOOTS5:
            case S_ARTI_BOOTS6:
            case S_ARTI_BOOTS7:
            case S_ARTI_BOOTS8:
            // Dragonskin Bracers
            case S_ARTI_ARMOR1:
            case S_ARTI_ARMOR2:
            case S_ARTI_ARMOR3:
            case S_ARTI_ARMOR4:
            case S_ARTI_ARMOR5:
            case S_ARTI_ARMOR6:
            case S_ARTI_ARMOR7:
            case S_ARTI_ARMOR8:
            // Krater of Might
            case S_ARTI_MANA:
            // Mystic Ambit Incant
            case S_ARTI_HEALRAD1:
            case S_ARTI_HEALRAD2:
            case S_ARTI_HEALRAD3:
            case S_ARTI_HEALRAD4:
            case S_ARTI_HEALRAD5:
            case S_ARTI_HEALRAD6:
            case S_ARTI_HEALRAD7:
            case S_ARTI_HEALRAD8:
            case S_ARTI_HEALRAD9:
            case S_ARTI_HEALRAD0:
            case S_ARTI_HEALRADA:
            case S_ARTI_HEALRADB:
            case S_ARTI_HEALRADC:
            case S_ARTI_HEALRADD:
            case S_ARTI_HEALRADE:
            case S_ARTI_HEALRADF:
            // Torch
            case S_ARTI_TRCH1:
            case S_ARTI_TRCH2:
            case S_ARTI_TRCH3:
            // Fire Bull
            case S_ZFIREBULL1:
            case S_ZFIREBULL2:
            case S_ZFIREBULL3:
            case S_ZFIREBULL4:
            case S_ZFIREBULL5:
            case S_ZFIREBULL6:
            case S_ZFIREBULL7:
            case S_ZFIREBULL_DEATH2:
            case S_ZFIREBULL_BIRTH2:
            {
                return fullbright;
                break;
            }
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
            // Timon's Axe
            case S_FAXEDOWN_G:
            case S_FAXEUP_G:
            case S_FAXEREADY_G:
            case S_FAXEREADY_G1:
            case S_FAXEREADY_G2:
            case S_FAXEREADY_G3:
            case S_FAXEREADY_G4:
            case S_FAXEREADY_G5:
            case S_FAXEATK_G1:
            case S_FAXEATK_G2:
            case S_FAXEATK_G3:
            case S_FAXEATK_G4:
            case S_FAXEATK_G5:
            case S_FAXEATK_G6:
            case S_FAXEATK_G7:
            case S_FAXEATK_G8:
            case S_FAXEATK_G9:
            case S_FAXEATK_G10:
            case S_FAXEATK_G11:
            case S_FAXEATK_G12:
            case S_FAXEATK_G13:
            // Quietus
            case S_FSWORDREADY:
            case S_FSWORDREADY1:
            case S_FSWORDREADY2:
            case S_FSWORDREADY3:
            case S_FSWORDREADY4:
            case S_FSWORDREADY5:
            case S_FSWORDREADY6:
            case S_FSWORDREADY7:
            case S_FSWORDREADY8:
            case S_FSWORDREADY9:
            case S_FSWORDREADY10:
            case S_FSWORDREADY11:
            case S_FSWORDDOWN:
            case S_FSWORDUP:
            case S_FSWORDATK_1:
            case S_FSWORDATK_2:
            case S_FSWORDATK_3:
            case S_FSWORDATK_4:
            case S_FSWORDATK_5:
            case S_FSWORDATK_6:
            case S_FSWORDATK_7:
            case S_FSWORDATK_8:
            case S_FSWORDATK_9:
            case S_FSWORDATK_10:
            case S_FSWORDATK_11:
            case S_FSWORDATK_12:
            // Firestorm
            case S_CFLAMEDOWN:
            case S_CFLAMEUP:
            case S_CFLAMEREADY1:
            case S_CFLAMEREADY2:
            case S_CFLAMEREADY3:
            case S_CFLAMEREADY4:
            case S_CFLAMEREADY5:
            case S_CFLAMEREADY6:
            case S_CFLAMEREADY7:
            case S_CFLAMEREADY8:
            case S_CFLAMEREADY9:
            case S_CFLAMEREADY10:
            case S_CFLAMEREADY11:
            case S_CFLAMEREADY12:
            case S_CFLAMEATK_1:
            case S_CFLAMEATK_2:
            case S_CFLAMEATK_3:
            case S_CFLAMEATK_7:
            case S_CFLAMEATK_8:
            // Bloodscourge
            case S_MSTAFFREADY:
            case S_MSTAFFREADY2:
            case S_MSTAFFREADY3:
            case S_MSTAFFREADY4:
            case S_MSTAFFREADY5:
            case S_MSTAFFREADY6:
            case S_MSTAFFREADY7:
            case S_MSTAFFREADY8:
            case S_MSTAFFREADY9:
            case S_MSTAFFREADY10:
            case S_MSTAFFREADY11:
            case S_MSTAFFREADY12:
            case S_MSTAFFREADY13:
            case S_MSTAFFREADY14:
            case S_MSTAFFREADY15:
            case S_MSTAFFREADY16:
            case S_MSTAFFREADY17:
            case S_MSTAFFREADY18:
            case S_MSTAFFREADY19:
            case S_MSTAFFREADY20:
            case S_MSTAFFREADY21:
            case S_MSTAFFREADY22:
            case S_MSTAFFREADY23:
            case S_MSTAFFREADY24:
            case S_MSTAFFREADY25:
            case S_MSTAFFREADY26:
            case S_MSTAFFREADY27:
            case S_MSTAFFREADY28:
            case S_MSTAFFREADY29:
            case S_MSTAFFREADY30:
            case S_MSTAFFREADY31:
            case S_MSTAFFREADY32:
            case S_MSTAFFREADY33:
            case S_MSTAFFREADY34:
            case S_MSTAFFREADY35:
            case S_MSTAFFDOWN:
            case S_MSTAFFUP:
            case S_MSTAFFATK_1:
            case S_MSTAFFATK_4:
            case S_MSTAFFATK_5:
            case S_MSTAFFATK_6:
            case S_MSTAFFATK_7:
            {
                return surfaces1;
                break;
            }
            //  Serpent Staff
            case S_CSTAFFATK_1:
            case S_CSTAFFATK_2:
            case S_CSTAFFATK_3:
            case S_CSTAFFATK_4:
            case S_CSTAFFATK2_1:
            {
                return greenonly;
                break;
            }
            // Frost Shards
            case S_CONEATK1_2: 
            case S_CONEATK1_3:
            case S_CONEATK1_4:
            case S_CONEATK1_5:
            case S_CONEATK1_6:
            case S_CONEATK1_7:
            case S_CONEATK1_8:
            // Arc of Death
            case S_MLIGHTNINGREADY:
            case S_MLIGHTNINGREADY2:
            case S_MLIGHTNINGREADY3:
            case S_MLIGHTNINGREADY4:
            case S_MLIGHTNINGREADY5:
            case S_MLIGHTNINGREADY6:
            case S_MLIGHTNINGREADY7:
            case S_MLIGHTNINGREADY8:
            case S_MLIGHTNINGREADY9:
            case S_MLIGHTNINGREADY10:
            case S_MLIGHTNINGREADY11:
            case S_MLIGHTNINGREADY12:
            case S_MLIGHTNINGREADY13:
            case S_MLIGHTNINGREADY14:
            case S_MLIGHTNINGREADY15:
            case S_MLIGHTNINGREADY16:
            case S_MLIGHTNINGREADY17:
            case S_MLIGHTNINGREADY18:
            case S_MLIGHTNINGREADY19:
            case S_MLIGHTNINGREADY20:
            case S_MLIGHTNINGREADY21:
            case S_MLIGHTNINGREADY22:
            case S_MLIGHTNINGREADY23:
            case S_MLIGHTNINGREADY24:
            case S_MLIGHTNINGDOWN:
            case S_MLIGHTNINGUP:
            case S_MLIGHTNINGATK_1:
            case S_MLIGHTNINGATK_2:
            case S_MLIGHTNINGATK_3:
            case S_MLIGHTNINGATK_4:
            case S_MLIGHTNINGATK_5:
            case S_MLIGHTNINGATK_6:
            case S_MLIGHTNINGATK_7:
            case S_MLIGHTNINGATK_8:
            case S_MLIGHTNINGATK_9:
            case S_MLIGHTNINGATK_10:
            case S_MLIGHTNINGATK_11:
            {
                return blueonly;
                break;
            }
        }
	}
    else
    {
        switch (state)
        {
            // Quentus
            case S_FSWORDREADY:
            case S_FSWORDREADY1:
            case S_FSWORDREADY2:
            case S_FSWORDREADY3:
            case S_FSWORDREADY4:
            case S_FSWORDREADY5:
            case S_FSWORDREADY6:
            case S_FSWORDREADY7:
            case S_FSWORDREADY8:
            case S_FSWORDREADY9:
            case S_FSWORDREADY10:
            case S_FSWORDREADY11:
            case S_FSWORDDOWN:
            case S_FSWORDUP:
            case S_FSWORDATK_1:
            case S_FSWORDATK_2:
            case S_FSWORDATK_3:
            case S_FSWORDATK_4:
            case S_FSWORDATK_5:
            case S_FSWORDATK_6:
            case S_FSWORDATK_7:
            case S_FSWORDATK_8:
            case S_FSWORDATK_9:
            case S_FSWORDATK_10:
            case S_FSWORDATK_11:
            case S_FSWORDATK_12:
            // Arc of Death
            case S_MLIGHTNINGREADY:
            case S_MLIGHTNINGREADY2:
            case S_MLIGHTNINGREADY3:
            case S_MLIGHTNINGREADY4:
            case S_MLIGHTNINGREADY5:
            case S_MLIGHTNINGREADY6:
            case S_MLIGHTNINGREADY7:
            case S_MLIGHTNINGREADY8:
            case S_MLIGHTNINGREADY9:
            case S_MLIGHTNINGREADY10:
            case S_MLIGHTNINGREADY11:
            case S_MLIGHTNINGREADY12:
            case S_MLIGHTNINGREADY13:
            case S_MLIGHTNINGREADY14:
            case S_MLIGHTNINGREADY15:
            case S_MLIGHTNINGREADY16:
            case S_MLIGHTNINGREADY17:
            case S_MLIGHTNINGREADY18:
            case S_MLIGHTNINGREADY19:
            case S_MLIGHTNINGREADY20:
            case S_MLIGHTNINGREADY21:
            case S_MLIGHTNINGREADY22:
            case S_MLIGHTNINGREADY23:
            case S_MLIGHTNINGREADY24:
            case S_MLIGHTNINGDOWN:
            case S_MLIGHTNINGUP:
            case S_MLIGHTNINGATK_1:
            case S_MLIGHTNINGATK_2:
            case S_MLIGHTNINGATK_3:
            case S_MLIGHTNINGATK_4:
            case S_MLIGHTNINGATK_5:
            case S_MLIGHTNINGATK_6:
            case S_MLIGHTNINGATK_7:
            case S_MLIGHTNINGATK_8:
            case S_MLIGHTNINGATK_9:
            case S_MLIGHTNINGATK_10:
            case S_MLIGHTNINGATK_11:
            {
                return fullbright;
                break;
            }
        }
    }

    return nobrightmap;
}
