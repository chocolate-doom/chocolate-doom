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

// haleyjd [STRIFE]: Multiple externals added
boolean P_GiveCard(player_t* player, card_t card);
boolean P_GiveBody(player_t* player, int num);
boolean P_GiveArmor(player_t* player, int armortype);
boolean P_GivePower(player_t* player, powertype_t power);
boolean P_GiveAmmo(player_t* player, ammotype_t ammo, int num);
boolean P_GiveWeapon(player_t* player, weapontype_t weapon, boolean dropped);
void    P_KillMobj(mobj_t* source, mobj_t* target);

#endif
