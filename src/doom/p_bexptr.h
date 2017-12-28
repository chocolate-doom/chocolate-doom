//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] additional BOOM and MBF code pointers
//

#ifndef __P_BEXPTR__
#define __P_BEXPTR__

#include "doomtype.h"
#include "d_player.h"
#include "p_mobj.h"

extern void A_Explode();
extern void A_FaceTarget();

extern boolean P_CheckMeleeRange (mobj_t *actor);
extern void P_Thrust (player_t* player, angle_t angle, fixed_t move);

#endif
