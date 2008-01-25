// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2006 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//      Pixel-doubling scale up functions.
//
//-----------------------------------------------------------------------------


#ifndef __I_SCALE__
#define __I_SCALE__

#include "doomtype.h"

void I_InitScale(byte *_src_buffer, byte *_dest_buffer, int _dest_pitch);

// Scaled modes (direct multiples of 320x200)

extern screen_mode_t mode_scale_1x;
extern screen_mode_t mode_scale_2x;
extern screen_mode_t mode_scale_3x;
extern screen_mode_t mode_scale_4x;
extern screen_mode_t mode_scale_5x;

// Vertically stretched modes (320x200 -> multiples of 320x240)

extern screen_mode_t mode_stretch_1x;
extern screen_mode_t mode_stretch_2x;
extern screen_mode_t mode_stretch_3x;
extern screen_mode_t mode_stretch_4x;
extern screen_mode_t mode_stretch_5x;

// Horizontally squashed modes (320x200 -> multiples of 256x200)

extern screen_mode_t mode_squash_1x;
extern screen_mode_t mode_squash_2x;
extern screen_mode_t mode_squash_3x;
extern screen_mode_t mode_squash_4x;
extern screen_mode_t mode_squash_5x;

#endif /* #ifndef __I_SCALE__ */

