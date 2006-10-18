// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
//	Refresh (R_*) module, global header.
//	All the rendering/drawing stuff is here.
//
//-----------------------------------------------------------------------------

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Binary Angles, sine/cosine/atan lookups.
#include "tables.h"

// Screen size related parameters.
#include "doomdef.h"

// Include the refresh/render data structs.
#include "r_data.h"



//
// Separate header file for each module.
//
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_data.h"
#include "r_things.h"
#include "r_draw.h"

#endif		// __R_LOCAL__
