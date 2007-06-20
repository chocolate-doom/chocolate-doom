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

void I_InitStretchTables(byte *palette);
void I_InitScale(byte *_src_buffer, byte *_dest_buffer, int _dest_pitch);

// Normal pixel-perfect doubling functions.

void I_Scale1x(int x1, int y1, int x2, int y2);
void I_Scale2x(int x1, int y1, int x2, int y2);
void I_Scale3x(int x1, int y1, int x2, int y2);
void I_Scale4x(int x1, int y1, int x2, int y2);
void I_Scale5x(int x1, int y1, int x2, int y2);

// Aspect ratio correcting scale up functions

void I_Stretch1x(int x1, int y1, int x2, int y2);
void I_Stretch2x(int x1, int y1, int x2, int y2);
void I_Stretch3x(int x1, int y1, int x2, int y2);
void I_Stretch4x(int x1, int y1, int x2, int y2);
void I_Stretch5x(int x1, int y1, int x2, int y2);

#endif /* #ifndef __I_SCALE__ */

