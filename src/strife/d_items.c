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


// We are referring to sprite numbers.
#include "info.h"

#include "d_items.h"


//
// PSPRITE ACTIONS for waepons.
// This struct controls the weapon animations.
//
// Each entry is:
//   ammo/amunition type
//  upstate
//  downstate
// readystate
// atkstate, i.e. attack/fire/hit frame
// flashstate, muzzle flash
//

// villsa [STRIFE]
weaponinfo_t	weaponinfo[NUMWEAPONS] =
{
    {
        // fist
        am_noammo,
        S_PNCH_03,
        S_PNCH_02,
        S_PNCH_01,
        S_PNCH_04,
        S_NULL,
        1
    },	
    {
        // electric bow
        am_elecbolts,
        S_XBOW_02,
        S_XBOW_01,
        S_XBOW_00,
        S_XBOW_03,
        S_NULL,
        1
    },	
    {
        // rifle
        am_bullets,
        S_RIFG_02,
        S_RIFG_01,
        S_RIFG_00,
        S_RIFF_00,
        S_NULL,
        1
    },
    {
        // missile launcher
        am_missiles,
        S_MMIS_02,
        S_MMIS_01,
        S_MMIS_00,
        S_MMIS_03,
        S_NULL,
        0
    },
    {
        // grenade launcher
        am_hegrenades,
        S_GREN_02,
        S_GREN_01,
        S_GREN_00,
        S_GREN_03,
        S_GREF_00,
        0
    },
    {
        // flame thrower
        am_cell,
        S_FLMT_03,
        S_FLMT_02,
        S_FLMT_00,
        S_FLMF_00,
        S_NULL,
        1
    },
    {
        // mauler
        am_cell,
        S_BLST_05,
        S_BLST_04,
        S_BLST_00,
        S_BLSF_00,
        S_NULL,
        0
    },
    {
        // sigil
        am_noammo,
        S_SIGH_06,
        S_SIGH_05,
        S_SIGH_00,
        S_SIGH_07,
        S_SIGF_00,
        0
    },
    {
        // poison bow
        am_poisonbolts,
        S_XBOW_15,
        S_XBOW_14,
        S_XBOW_13,
        S_XBOW_16,
        S_NULL,
        1
    },
    {
        // wp grenade launcher
        am_wpgrenades,
        S_GREN_10,
        S_GREN_09,
        S_GREN_08,
        S_GREN_11,
        S_GREF_03,
        0
    },
    {
        // torpedo
        am_cell,
        S_BLST_18,
        S_BLST_17,
        S_BLST_13,
        S_BLST_19,
        S_NULL,
        0
    },	
};








