//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//
//


#ifndef __P_INTER__
#define __P_INTER__


boolean	P_GivePower(player_t*, int);

// [marshmallow]
typedef struct {
	int weapons[NUMWEAPONS];
	int ammo[NUMAMMO];

	boolean backpack;

} backpack_s;

backpack_s backpacks[4];

boolean DropInventoryInBackpack(mobj_t* target, int p);
void RecoverInventoryFromBackpack(player_t* player, int p);


#endif
