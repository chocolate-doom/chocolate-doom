//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2024 Fabian Greffrath
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
//	[crispy] Truecolor rendering
//

#include "config.h"

#ifdef CRISPY_TRUECOLOR

#include "crispy.h"
#include "i_truecolor.h"

const uint32_t (*blendfunc) (const uint32_t fg, const uint32_t bg) = I_BlendOverTranmap;

const uint32_t (*I_BlendOverTinttab) (const uint32_t fg, const uint32_t bg); // [crispy] points to function for heretic/hexen normal blending
const uint32_t (*I_BlendOverAltTinttab) (const uint32_t fg, const uint32_t bg); // [crispy] points to function for heretic/hexen alternative blending

static const uint32_t I_BlendWeakOverTinttab (const uint32_t bg, const uint32_t fg);
static const uint32_t I_BlendStrongOverTinttab (const uint32_t bg, const uint32_t fg);

typedef union
{
    uint32_t i;
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
} tcpixel_t;

void I_InitTinttab (GameMission_t mission)
{
    if (mission == heretic)
    {
        I_BlendOverTinttab = I_BlendStrongOverTinttab;
        I_BlendOverAltTinttab = I_BlendWeakOverTinttab;
    }
    else
    {
        I_BlendOverTinttab = I_BlendWeakOverTinttab;
        I_BlendOverAltTinttab = I_BlendStrongOverTinttab;
    }
}

const uint32_t I_BlendAdd (const uint32_t bg_i, const uint32_t fg_i)
{
    tcpixel_t bg, fg, ret;

    bg.i = bg_i;
    fg.i = fg_i;

    ret.a = 0xFFU;
    ret.r = MIN(bg.r + fg.r, 0xFFU);
    ret.g = MIN(bg.g + fg.g, 0xFFU);
    ret.b = MIN(bg.b + fg.b, 0xFFU);

    return ret.i;
}

const uint32_t I_BlendDark (const uint32_t bg_i, const int d)
{
    tcpixel_t bg, ret;

    bg.i = bg_i;

    ret.a = 0xFFU;
    ret.r = (bg.r * d) >> 8;
    ret.g = (bg.g * d) >> 8;
    ret.b = (bg.b * d) >> 8;

    return ret.i;
}

const uint32_t I_BlendOver (const uint32_t bg_i, const uint32_t fg_i, const int amount)
{
    tcpixel_t bg, fg, ret;

    bg.i = bg_i;
    fg.i = fg_i;

    ret.a = 0xFFU;
    ret.r = (amount * fg.r + (0XFFU - amount) * bg.r) >> 8;
    ret.g = (amount * fg.g + (0XFFU - amount) * bg.g) >> 8;
    ret.b = (amount * fg.b + (0XFFU - amount) * bg.b) >> 8;

    return ret.i;
}

// [crispy] TRANMAP blending emulation, used for Doom
const uint32_t I_BlendOverTranmap (const uint32_t bg, const uint32_t fg)
{
    return I_BlendOver(bg, fg, 0xA8); // 168 (66% opacity)
}

// [crispy] TINTTAB blending emulation, Heretic AltTinttab - Hexen Tinttab
static const uint32_t I_BlendWeakOverTinttab (const uint32_t bg, const uint32_t fg)
{
    return I_BlendOver(bg, fg, 0x60); // 96 (38% opacity)
}

// [crispy] TINTTAB blending emulation, Heretic Tinttab - Hexen AltTinttab
static const uint32_t I_BlendStrongOverTinttab (const uint32_t bg, const uint32_t fg)
{
    return I_BlendOver(bg, fg, 0x8E); // 142 (56% opacity)
}

// [crispy] More opaque XLATAB blending emulation, used for Strife
const uint32_t I_BlendOverXlatab (const uint32_t bg, const uint32_t fg)
{
    return I_BlendOver(bg, fg, 0xC0); // 192 (75% opacity)
}

// [crispy] Less opaque ("Alt") XLATAB blending emulation, used for Strife
const uint32_t I_BlendOverAltXlatab (const uint32_t bg, const uint32_t fg)
{
    return I_BlendOver(bg, fg, 0x40); // 64 (25% opacity)
}

#endif
