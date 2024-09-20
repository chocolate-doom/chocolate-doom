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

#ifndef __I_TRUECOLOR__
#define __I_TRUECOLOR__

#include "config.h"

#ifdef CRISPY_TRUECOLOR

#include <stdint.h>

extern const uint32_t (*blendfunc) (const uint32_t fg, const uint32_t bg);

const uint32_t I_BlendAdd (const uint32_t bg_i, const uint32_t fg_i);
const uint32_t I_BlendDark (const uint32_t bg_i, const int d);
const uint32_t I_BlendOver (const uint32_t bg_i, const uint32_t fg_i, const int amount);

const uint32_t I_BlendOverTranmap (const uint32_t bg, const uint32_t fg);
const uint32_t I_BlendOverTinttab (const uint32_t bg, const uint32_t fg);
const uint32_t I_BlendOverAltTinttab (const uint32_t bg, const uint32_t fg);
const uint32_t I_BlendOverXlatab (const uint32_t bg, const uint32_t fg);
const uint32_t I_BlendOverAltXlatab (const uint32_t bg, const uint32_t fg);

#endif

#endif
