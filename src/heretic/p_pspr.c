//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

// P_pspr.c

#include "doomdef.h"
#include "i_system.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// Macros

#define LOWERSPEED FRACUNIT*6
#define RAISESPEED FRACUNIT*6
#define WEAPONBOTTOM 128*FRACUNIT
#define WEAPONTOP 32*FRACUNIT
#define FLAME_THROWER_TICS 10*35
#define MAGIC_JUNK 1234
#define MAX_MACE_SPOTS 8

static int MaceSpotCount;
static struct
{
    fixed_t x;
    fixed_t y;
} MaceSpots[MAX_MACE_SPOTS];

fixed_t bulletslope;

static int WeaponAmmoUsePL1[NUMWEAPONS] = {
    0,                          // staff
    USE_GWND_AMMO_1,            // gold wand
    USE_CBOW_AMMO_1,            // crossbow
    USE_BLSR_AMMO_1,            // blaster
    USE_SKRD_AMMO_1,            // skull rod
    USE_PHRD_AMMO_1,            // phoenix rod
    USE_MACE_AMMO_1,            // mace
    0,                          // gauntlets
    0                           // beak
};

static int WeaponAmmoUsePL2[NUMWEAPONS] = {
    0,                          // staff
    USE_GWND_AMMO_2,            // gold wand
    USE_CBOW_AMMO_2,            // crossbow
    USE_BLSR_AMMO_2,            // blaster
    USE_SKRD_AMMO_2,            // skull rod
    USE_PHRD_AMMO_2,            // phoenix rod
    USE_MACE_AMMO_2,            // mace
    0,                          // gauntlets
    0                           // beak
};

weaponinfo_t wpnlev1info[NUMWEAPONS] = {
    {                           // Staff
     am_noammo,                 // ammo
     S_STAFFUP,                 // upstate
     S_STAFFDOWN,               // downstate
     S_STAFFREADY,              // readystate
     S_STAFFATK1_1,             // atkstate
     S_STAFFATK1_1,             // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Gold wand
     am_goldwand,               // ammo
     S_GOLDWANDUP,              // upstate
     S_GOLDWANDDOWN,            // downstate
     S_GOLDWANDREADY,           // readystate
     S_GOLDWANDATK1_1,          // atkstate
     S_GOLDWANDATK1_1,          // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Crossbow
     am_crossbow,               // ammo
     S_CRBOWUP,                 // upstate
     S_CRBOWDOWN,               // downstate
     S_CRBOW1,                  // readystate
     S_CRBOWATK1_1,             // atkstate
     S_CRBOWATK1_1,             // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Blaster
     am_blaster,                // ammo
     S_BLASTERUP,               // upstate
     S_BLASTERDOWN,             // downstate
     S_BLASTERREADY,            // readystate
     S_BLASTERATK1_1,           // atkstate
     S_BLASTERATK1_3,           // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Skull rod
     am_skullrod,               // ammo
     S_HORNRODUP,               // upstate
     S_HORNRODDOWN,             // downstate
     S_HORNRODREADY,            // readystae
     S_HORNRODATK1_1,           // atkstate
     S_HORNRODATK1_1,           // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Phoenix rod
     am_phoenixrod,             // ammo
     S_PHOENIXUP,               // upstate
     S_PHOENIXDOWN,             // downstate
     S_PHOENIXREADY,            // readystate
     S_PHOENIXATK1_1,           // atkstate
     S_PHOENIXATK1_1,           // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Mace
     am_mace,                   // ammo
     S_MACEUP,                  // upstate
     S_MACEDOWN,                // downstate
     S_MACEREADY,               // readystate
     S_MACEATK1_1,              // atkstate
     S_MACEATK1_2,              // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Gauntlets
     am_noammo,                 // ammo
     S_GAUNTLETUP,              // upstate
     S_GAUNTLETDOWN,            // downstate
     S_GAUNTLETREADY,           // readystate
     S_GAUNTLETATK1_1,          // atkstate
     S_GAUNTLETATK1_3,          // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Beak
     am_noammo,                 // ammo
     S_BEAKUP,                  // upstate
     S_BEAKDOWN,                // downstate
     S_BEAKREADY,               // readystate
     S_BEAKATK1_1,              // atkstate
     S_BEAKATK1_1,              // holdatkstate
     S_NULL                     // flashstate
     }
};

weaponinfo_t wpnlev2info[NUMWEAPONS] = {
    {                           // Staff
     am_noammo,                 // ammo
     S_STAFFUP2,                // upstate
     S_STAFFDOWN2,              // downstate
     S_STAFFREADY2_1,           // readystate
     S_STAFFATK2_1,             // atkstate
     S_STAFFATK2_1,             // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Gold wand
     am_goldwand,               // ammo
     S_GOLDWANDUP,              // upstate
     S_GOLDWANDDOWN,            // downstate
     S_GOLDWANDREADY,           // readystate
     S_GOLDWANDATK2_1,          // atkstate
     S_GOLDWANDATK2_1,          // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Crossbow
     am_crossbow,               // ammo
     S_CRBOWUP,                 // upstate
     S_CRBOWDOWN,               // downstate
     S_CRBOW1,                  // readystate
     S_CRBOWATK2_1,             // atkstate
     S_CRBOWATK2_1,             // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Blaster
     am_blaster,                // ammo
     S_BLASTERUP,               // upstate
     S_BLASTERDOWN,             // downstate
     S_BLASTERREADY,            // readystate
     S_BLASTERATK2_1,           // atkstate
     S_BLASTERATK2_3,           // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Skull rod
     am_skullrod,               // ammo
     S_HORNRODUP,               // upstate
     S_HORNRODDOWN,             // downstate
     S_HORNRODREADY,            // readystae
     S_HORNRODATK2_1,           // atkstate
     S_HORNRODATK2_1,           // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Phoenix rod
     am_phoenixrod,             // ammo
     S_PHOENIXUP,               // upstate
     S_PHOENIXDOWN,             // downstate
     S_PHOENIXREADY,            // readystate
     S_PHOENIXATK2_1,           // atkstate
     S_PHOENIXATK2_2,           // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Mace
     am_mace,                   // ammo
     S_MACEUP,                  // upstate
     S_MACEDOWN,                // downstate
     S_MACEREADY,               // readystate
     S_MACEATK2_1,              // atkstate
     S_MACEATK2_1,              // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Gauntlets
     am_noammo,                 // ammo
     S_GAUNTLETUP2,             // upstate
     S_GAUNTLETDOWN2,           // downstate
     S_GAUNTLETREADY2_1,        // readystate
     S_GAUNTLETATK2_1,          // atkstate
     S_GAUNTLETATK2_3,          // holdatkstate
     S_NULL                     // flashstate
     },
    {                           // Beak
     am_noammo,                 // ammo
     S_BEAKUP,                  // upstate
     S_BEAKDOWN,                // downstate
     S_BEAKREADY,               // readystate
     S_BEAKATK2_1,              // atkstate
     S_BEAKATK2_1,              // holdatkstate
     S_NULL                     // flashstate
     }
};

//---------------------------------------------------------------------------
//
// PROC P_OpenWeapons
//
// Called at level load before things are loaded.
//
//---------------------------------------------------------------------------

void P_OpenWeapons(void)
{
    MaceSpotCount = 0;
}

//---------------------------------------------------------------------------
//
// PROC P_AddMaceSpot
//
//---------------------------------------------------------------------------

void P_AddMaceSpot(mapthing_t * mthing)
{
    if (MaceSpotCount == MAX_MACE_SPOTS)
    {
        I_Error("Too many mace spots.");
    }
    MaceSpots[MaceSpotCount].x = mthing->x << FRACBITS;
    MaceSpots[MaceSpotCount].y = mthing->y << FRACBITS;
    MaceSpotCount++;
}

//---------------------------------------------------------------------------
//
// PROC P_RepositionMace
//
// Chooses the next spot to place the mace.
//
//---------------------------------------------------------------------------

void P_RepositionMace(mobj_t * mo)
{
    int spot;
    subsector_t *ss;

    P_UnsetThingPosition(mo);
    spot = P_Random() % MaceSpotCount;
    mo->x = MaceSpots[spot].x;
    mo->y = MaceSpots[spot].y;
    ss = R_PointInSubsector(mo->x, mo->y);
    mo->z = mo->floorz = ss->sector->floorheight;
    mo->ceilingz = ss->sector->ceilingheight;
    P_SetThingPosition(mo);
}

//---------------------------------------------------------------------------
//
// PROC P_CloseWeapons
//
// Called at level load after things are loaded.
//
//---------------------------------------------------------------------------

void P_CloseWeapons(void)
{
    int spot;

    if (!MaceSpotCount)
    {                           // No maces placed
        return;
    }
    if (!deathmatch && P_Random() < 64)
    {                           // Sometimes doesn't show up if not in deathmatch
        return;
    }
    spot = P_Random() % MaceSpotCount;
    P_SpawnMobj(MaceSpots[spot].x, MaceSpots[spot].y, ONFLOORZ, MT_WMACE);
}

//---------------------------------------------------------------------------
//
// PROC P_SetPsprite
//
//---------------------------------------------------------------------------

void P_SetPsprite(player_t * player, int position, statenum_t stnum)
{
    pspdef_t *psp;
    state_t *state;

    psp = &player->psprites[position];
    do
    {
        if (!stnum)
        {                       // Object removed itself.
            psp->state = NULL;
            break;
        }
        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;        // could be 0
        if (state->misc1)
        {                       // Set coordinates.
            psp->sx = state->misc1 << FRACBITS;
            psp->sy = state->misc2 << FRACBITS;
        }
        if (state->action)
        {                       // Call action routine.
            state->action(player, psp);
            if (!psp->state)
            {
                break;
            }
        }
        stnum = psp->state->nextstate;
    }
    while (!psp->tics);         // An initial state of 0 could cycle through.
}

/*
=================
=
= P_CalcSwing
=
=================
*/

/*
fixed_t	swingx, swingy;
void P_CalcSwing (player_t *player)
{
	fixed_t	swing;
	int		angle;

// OPTIMIZE: tablify this

	swing = player->bob;

	angle = (FINEANGLES/70*leveltime)&FINEMASK;
	swingx = FixedMul ( swing, finesine[angle]);

	angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
	swingy = -FixedMul ( swingx, finesine[angle]);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_ActivateBeak
//
//---------------------------------------------------------------------------

void P_ActivateBeak(player_t * player)
{
    player->pendingweapon = wp_nochange;
    player->readyweapon = wp_beak;
    player->psprites[ps_weapon].sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon, S_BEAKREADY);
}

//---------------------------------------------------------------------------
//
// PROC P_PostChickenWeapon
//
//---------------------------------------------------------------------------

void P_PostChickenWeapon(player_t * player, weapontype_t weapon)
{
    if (weapon == wp_beak)
    {                           // Should never happen
        weapon = wp_staff;
    }
    player->pendingweapon = wp_nochange;
    player->readyweapon = weapon;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon, wpnlev1info[weapon].upstate);
}

//---------------------------------------------------------------------------
//
// PROC P_BringUpWeapon
//
// Starts bringing the pending weapon up from the bottom of the screen.
//
//---------------------------------------------------------------------------

void P_BringUpWeapon(player_t * player)
{
    statenum_t new;

    if (player->pendingweapon == wp_nochange)
    {
        player->pendingweapon = player->readyweapon;
    }
    if (player->pendingweapon == wp_gauntlets)
    {
        S_StartSound(player->mo, sfx_gntact);
    }
    if (player->powers[pw_weaponlevel2])
    {
        new = wpnlev2info[player->pendingweapon].upstate;
    }
    else
    {
        new = wpnlev1info[player->pendingweapon].upstate;
    }
    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon, new);
}

//---------------------------------------------------------------------------
//
// FUNC P_CheckAmmo
//
// Returns true if there is enough ammo to shoot.  If not, selects the
// next weapon to use.
//
//---------------------------------------------------------------------------

boolean P_CheckAmmo(player_t * player)
{
    ammotype_t ammo;
    int *ammoUse;
    int count;

    ammo = wpnlev1info[player->readyweapon].ammo;
    if (player->powers[pw_weaponlevel2] && !deathmatch)
    {
        ammoUse = WeaponAmmoUsePL2;
    }
    else
    {
        ammoUse = WeaponAmmoUsePL1;
    }
    count = ammoUse[player->readyweapon];
    if (ammo == am_noammo || player->ammo[ammo] >= count)
    {
        return (true);
    }
    // out of ammo, pick a weapon to change to
    do
    {
        if (player->weaponowned[wp_skullrod]
            && player->ammo[am_skullrod] > ammoUse[wp_skullrod])
        {
            player->pendingweapon = wp_skullrod;
        }
        else if (player->weaponowned[wp_blaster]
                 && player->ammo[am_blaster] > ammoUse[wp_blaster])
        {
            player->pendingweapon = wp_blaster;
        }
        else if (player->weaponowned[wp_crossbow]
                 && player->ammo[am_crossbow] > ammoUse[wp_crossbow])
        {
            player->pendingweapon = wp_crossbow;
        }
        else if (player->weaponowned[wp_mace]
                 && player->ammo[am_mace] > ammoUse[wp_mace])
        {
            player->pendingweapon = wp_mace;
        }
        else if (player->ammo[am_goldwand] > ammoUse[wp_goldwand])
        {
            player->pendingweapon = wp_goldwand;
        }
        else if (player->weaponowned[wp_gauntlets])
        {
            player->pendingweapon = wp_gauntlets;
        }
        else if (player->weaponowned[wp_phoenixrod]
                 && player->ammo[am_phoenixrod] > ammoUse[wp_phoenixrod])
        {
            player->pendingweapon = wp_phoenixrod;
        }
        else
        {
            player->pendingweapon = wp_staff;
        }
    }
    while (player->pendingweapon == wp_nochange);
    if (player->powers[pw_weaponlevel2])
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev2info[player->readyweapon].downstate);
    }
    else
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev1info[player->readyweapon].downstate);
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC P_FireWeapon
//
//---------------------------------------------------------------------------

void P_FireWeapon(player_t * player)
{
    weaponinfo_t *wpinfo;
    statenum_t attackState;

    if (!P_CheckAmmo(player))
    {
        return;
    }
    P_SetMobjState(player->mo, S_PLAY_ATK2);
    wpinfo = player->powers[pw_weaponlevel2] ? &wpnlev2info[0]
        : &wpnlev1info[0];
    attackState = player->refire ? wpinfo[player->readyweapon].holdatkstate
        : wpinfo[player->readyweapon].atkstate;
    P_SetPsprite(player, ps_weapon, attackState);
    P_NoiseAlert(player->mo, player->mo);
    if (player->readyweapon == wp_gauntlets && !player->refire)
    {                           // Play the sound for the initial gauntlet attack
        S_StartSound(player->mo, sfx_gntuse);
    }
}

//---------------------------------------------------------------------------
//
// PROC P_DropWeapon
//
// The player died, so put the weapon away.
//
//---------------------------------------------------------------------------

void P_DropWeapon(player_t * player)
{
    if (player->powers[pw_weaponlevel2])
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev2info[player->readyweapon].downstate);
    }
    else
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev1info[player->readyweapon].downstate);
    }
}

//---------------------------------------------------------------------------
//
// PROC A_WeaponReady
//
// The player can fire the weapon or change to another weapon at this time.
//
//---------------------------------------------------------------------------

void A_WeaponReady(player_t * player, pspdef_t * psp)
{
    int angle;

    if (player->chickenTics)
    {                           // Change to the chicken beak
        P_ActivateBeak(player);
        return;
    }
    // Change player from attack state
    if (player->mo->state == &states[S_PLAY_ATK1]
        || player->mo->state == &states[S_PLAY_ATK2])
    {
        P_SetMobjState(player->mo, S_PLAY);
    }
    // Check for staff PL2 active sound
    if ((player->readyweapon == wp_staff)
        && (psp->state == &states[S_STAFFREADY2_1]) && P_Random() < 128)
    {
        S_StartSound(player->mo, sfx_stfcrk);
    }
    // Put the weapon away if the player has a pending weapon or has
    // died.
    if (player->pendingweapon != wp_nochange || !player->health)
    {
        if (player->powers[pw_weaponlevel2])
        {
            P_SetPsprite(player, ps_weapon,
                         wpnlev2info[player->readyweapon].downstate);
        }
        else
        {
            P_SetPsprite(player, ps_weapon,
                         wpnlev1info[player->readyweapon].downstate);
        }
        return;
    }

    // Check for fire.  The phoenix rod does not auto fire.
    if (player->cmd.buttons & BT_ATTACK)
    {
        if (!player->attackdown || (player->readyweapon != wp_phoenixrod))
        {
            player->attackdown = true;
            P_FireWeapon(player);
            return;
        }
    }
    else
    {
        player->attackdown = false;
    }

    // Bob the weapon based on movement speed.
    angle = (128 * leveltime) & FINEMASK;
    psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
    angle &= FINEANGLES / 2 - 1;
    psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);
}

//---------------------------------------------------------------------------
//
// PROC P_UpdateBeak
//
//---------------------------------------------------------------------------

void P_UpdateBeak(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP + (player->chickenPeck << (FRACBITS - 1));
}

//---------------------------------------------------------------------------
//
// PROC A_BeakReady
//
//---------------------------------------------------------------------------

void A_BeakReady(player_t * player, pspdef_t * psp)
{
    if (player->cmd.buttons & BT_ATTACK)
    {                           // Chicken beak attack
        player->attackdown = true;
        P_SetMobjState(player->mo, S_CHICPLAY_ATK1);
        if (player->powers[pw_weaponlevel2])
        {
            P_SetPsprite(player, ps_weapon, S_BEAKATK2_1);
        }
        else
        {
            P_SetPsprite(player, ps_weapon, S_BEAKATK1_1);
        }
        P_NoiseAlert(player->mo, player->mo);
    }
    else
    {
        if (player->mo->state == &states[S_CHICPLAY_ATK1])
        {                       // Take out of attack state
            P_SetMobjState(player->mo, S_CHICPLAY);
        }
        player->attackdown = false;
    }
}

//---------------------------------------------------------------------------
//
// PROC A_ReFire
//
// The player can re fire the weapon without lowering it entirely.
//
//---------------------------------------------------------------------------

void A_ReFire(player_t * player, pspdef_t * psp)
{
    if ((player->cmd.buttons & BT_ATTACK)
        && player->pendingweapon == wp_nochange && player->health)
    {
        player->refire++;
        P_FireWeapon(player);
    }
    else
    {
        player->refire = 0;
        P_CheckAmmo(player);
    }
}

//---------------------------------------------------------------------------
//
// PROC A_Lower
//
//---------------------------------------------------------------------------

void A_Lower(player_t * player, pspdef_t * psp)
{
    if (player->chickenTics)
    {
        psp->sy = WEAPONBOTTOM;
    }
    else
    {
        psp->sy += LOWERSPEED;
    }
    if (psp->sy < WEAPONBOTTOM)
    {                           // Not lowered all the way yet
        return;
    }
    if (player->playerstate == PST_DEAD)
    {                           // Player is dead, so don't bring up a pending weapon
        psp->sy = WEAPONBOTTOM;
        return;
    }
    if (!player->health)
    {                           // Player is dead, so keep the weapon off screen
        P_SetPsprite(player, ps_weapon, S_NULL);
        return;
    }
    player->readyweapon = player->pendingweapon;
    P_BringUpWeapon(player);
}

//---------------------------------------------------------------------------
//
// PROC A_BeakRaise
//
//---------------------------------------------------------------------------

void A_BeakRaise(player_t * player, pspdef_t * psp)
{
    psp->sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon,
                 wpnlev1info[player->readyweapon].readystate);
}

//---------------------------------------------------------------------------
//
// PROC A_Raise
//
//---------------------------------------------------------------------------

void A_Raise(player_t * player, pspdef_t * psp)
{
    psp->sy -= RAISESPEED;
    if (psp->sy > WEAPONTOP)
    {                           // Not raised all the way yet
        return;
    }
    psp->sy = WEAPONTOP;
    if (player->powers[pw_weaponlevel2])
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev2info[player->readyweapon].readystate);
    }
    else
    {
        P_SetPsprite(player, ps_weapon,
                     wpnlev1info[player->readyweapon].readystate);
    }
}

/*
===============
=
= P_BulletSlope
=
= Sets a slope so a near miss is at aproximately the height of the
= intended target
=
===============
*/

void P_BulletSlope(mobj_t * mo)
{
    angle_t an;

//
// see which target is to be aimed at
//
    an = mo->angle;
    bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
    if (!linetarget)
    {
        an += 1 << 26;
        bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
        if (!linetarget)
        {
            an -= 2 << 26;
            bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
        }
        if (!linetarget)
        {
            an += 2 << 26;
            bulletslope = (mo->player->lookdir << FRACBITS) / 173;
        }
    }
}

//****************************************************************************
//
// WEAPON ATTACKS
//
//****************************************************************************

//----------------------------------------------------------------------------
//
// PROC A_BeakAttackPL1
//
//----------------------------------------------------------------------------

void A_BeakAttackPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 1 + (P_Random() & 3);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
    PuffType = MT_BEAKPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
    S_StartSound(player->mo, sfx_chicpk1 + (P_Random() % 3));
    player->chickenPeck = 12;
    psp->tics -= P_Random() & 7;
}

//----------------------------------------------------------------------------
//
// PROC A_BeakAttackPL2
//
//----------------------------------------------------------------------------

void A_BeakAttackPL2(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = HITDICE(4);
    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
    PuffType = MT_BEAKPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
    S_StartSound(player->mo, sfx_chicpk1 + (P_Random() % 3));
    player->chickenPeck = 12;
    psp->tics -= P_Random() & 3;
}

//----------------------------------------------------------------------------
//
// PROC A_StaffAttackPL1
//
//----------------------------------------------------------------------------

void A_StaffAttackPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    damage = 5 + (P_Random() & 15);
    angle = player->mo->angle;
    angle += P_SubRandom() << 18;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
    PuffType = MT_STAFFPUFF;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        //S_StartSound(player->mo, sfx_stfhit);
        // turn to face target
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_StaffAttackPL2
//
//----------------------------------------------------------------------------

void A_StaffAttackPL2(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;

    // P_inter.c:P_DamageMobj() handles target momentums
    damage = 18 + (P_Random() & 63);
    angle = player->mo->angle;
    angle += P_SubRandom() << 18;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
    PuffType = MT_STAFFPUFF2;
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
    if (linetarget)
    {
        //S_StartSound(player->mo, sfx_stfpow);
        // turn to face target
        player->mo->angle = R_PointToAngle2(player->mo->x,
                                            player->mo->y, linetarget->x,
                                            linetarget->y);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_FireBlasterPL1
//
//----------------------------------------------------------------------------

void A_FireBlasterPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    angle_t angle;
    int damage;

    mo = player->mo;
    S_StartSound(mo, sfx_gldhit);
    player->ammo[am_blaster] -= USE_BLSR_AMMO_1;
    P_BulletSlope(mo);
    damage = HITDICE(4);
    angle = mo->angle;
    if (player->refire)
    {
        angle += P_SubRandom() << 18;
    }
    PuffType = MT_BLASTERPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartSound(player->mo, sfx_blssht);
}

//----------------------------------------------------------------------------
//
// PROC A_FireBlasterPL2
//
//----------------------------------------------------------------------------

void A_FireBlasterPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    player->ammo[am_blaster] -=
        deathmatch ? USE_BLSR_AMMO_1 : USE_BLSR_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, MT_BLASTERFX1);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
    }
    S_StartSound(player->mo, sfx_blssht);
}

//----------------------------------------------------------------------------
//
// PROC A_FireGoldWandPL1
//
//----------------------------------------------------------------------------

void A_FireGoldWandPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    angle_t angle;
    int damage;

    mo = player->mo;
    player->ammo[am_goldwand] -= USE_GWND_AMMO_1;
    P_BulletSlope(mo);
    damage = 7 + (P_Random() & 7);
    angle = mo->angle;
    if (player->refire)
    {
        angle += P_SubRandom() << 18;
    }
    PuffType = MT_GOLDWANDPUFF1;
    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
    S_StartSound(player->mo, sfx_gldhit);
}

//----------------------------------------------------------------------------
//
// PROC A_FireGoldWandPL2
//
//----------------------------------------------------------------------------

void A_FireGoldWandPL2(player_t * player, pspdef_t * psp)
{
    int i;
    mobj_t *mo;
    angle_t angle;
    int damage;
    fixed_t momz;

    mo = player->mo;
    player->ammo[am_goldwand] -=
        deathmatch ? USE_GWND_AMMO_1 : USE_GWND_AMMO_2;
    PuffType = MT_GOLDWANDPUFF2;
    P_BulletSlope(mo);
    momz = FixedMul(mobjinfo[MT_GOLDWANDFX2].speed, bulletslope);
    P_SpawnMissileAngle(mo, MT_GOLDWANDFX2, mo->angle - (ANG45 / 8), momz);
    P_SpawnMissileAngle(mo, MT_GOLDWANDFX2, mo->angle + (ANG45 / 8), momz);
    angle = mo->angle - (ANG45 / 8);
    for (i = 0; i < 5; i++)
    {
        damage = 1 + (P_Random() & 7);
        P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
        angle += ((ANG45 / 8) * 2) / 4;
    }
    S_StartSound(player->mo, sfx_gldhit);
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL1B
//
//----------------------------------------------------------------------------

void A_FireMacePL1B(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;
    mobj_t *ball;
    angle_t angle;

    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
    {
        return;
    }
    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    pmo = player->mo;

    // Vanilla bug here:
    // Original code here looks like:
    //   (pmo->flags2 & MF2_FEETARECLIPPED != 0)
    // C's operator precedence interprets this as:
    //   (pmo->flags2 & (MF2_FEETARECLIPPED != 0))
    // Which simplifies to:
    //   (pmo->flags2 & 1)
    ball = P_SpawnMobj(pmo->x, pmo->y, pmo->z + 28 * FRACUNIT
                       - FOOTCLIPSIZE * (pmo->flags2 & 1), MT_MACEFX2);

    ball->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
    angle = pmo->angle;
    ball->target = pmo;
    ball->angle = angle;
    ball->z += (player->lookdir) << (FRACBITS - 4);
    angle >>= ANGLETOFINESHIFT;
    ball->momx = (pmo->momx >> 1)
        + FixedMul(ball->info->speed, finecosine[angle]);
    ball->momy = (pmo->momy >> 1)
        + FixedMul(ball->info->speed, finesine[angle]);
    S_StartSound(ball, sfx_lobsht);
    P_CheckMissileSpawn(ball);
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL1
//
//----------------------------------------------------------------------------

void A_FireMacePL1(player_t * player, pspdef_t * psp)
{
    mobj_t *ball;

    if (P_Random() < 28)
    {
        A_FireMacePL1B(player, psp);
        return;
    }
    if (player->ammo[am_mace] < USE_MACE_AMMO_1)
    {
        return;
    }
    player->ammo[am_mace] -= USE_MACE_AMMO_1;
    psp->sx = ((P_Random() & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (P_Random() & 3) * FRACUNIT;
    ball = P_SPMAngle(player->mo, MT_MACEFX1, player->mo->angle
                      + (((P_Random() & 7) - 4) << 24));
    if (ball)
    {
        ball->special1.i = 16;    // tics till dropoff
    }
}

//----------------------------------------------------------------------------
//
// PROC A_MacePL1Check
//
//----------------------------------------------------------------------------

void A_MacePL1Check(mobj_t * ball)
{
    angle_t angle;

    if (ball->special1.i == 0)
    {
        return;
    }
    ball->special1.i -= 4;
    if (ball->special1.i > 0)
    {
        return;
    }
    ball->special1.i = 0;
    ball->flags2 |= MF2_LOGRAV;
    angle = ball->angle >> ANGLETOFINESHIFT;
    ball->momx = FixedMul(7 * FRACUNIT, finecosine[angle]);
    ball->momy = FixedMul(7 * FRACUNIT, finesine[angle]);
    ball->momz -= ball->momz >> 1;
}

//----------------------------------------------------------------------------
//
// PROC A_MaceBallImpact
//
//----------------------------------------------------------------------------

void A_MaceBallImpact(mobj_t * ball)
{
    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->health != MAGIC_JUNK) && (ball->z <= ball->floorz)
        && ball->momz)
    {                           // Bounce
        ball->health = MAGIC_JUNK;
        ball->momz = (ball->momz * 192) >> 8;
        ball->flags2 &= ~MF2_FLOORBOUNCE;
        P_SetMobjState(ball, ball->info->spawnstate);
        S_StartSound(ball, sfx_bounce);
    }
    else
    {                           // Explode
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        S_StartSound(ball, sfx_lobhit);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_MaceBallImpact2
//
//----------------------------------------------------------------------------

void A_MaceBallImpact2(mobj_t * ball)
{
    mobj_t *tiny;
    angle_t angle;

    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->z != ball->floorz) || (ball->momz < 2 * FRACUNIT))
    {                           // Explode
        ball->momx = ball->momy = ball->momz = 0;
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~(MF2_LOGRAV | MF2_FLOORBOUNCE);
    }
    else
    {                           // Bounce
        ball->momz = (ball->momz * 192) >> 8;
        P_SetMobjState(ball, ball->info->spawnstate);

        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, MT_MACEFX3);
        angle = ball->angle + ANG90;
        tiny->target = ball->target;
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (ball->momx >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finecosine[angle]);
        tiny->momy = (ball->momy >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finesine[angle]);
        tiny->momz = ball->momz;
        P_CheckMissileSpawn(tiny);

        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, MT_MACEFX3);
        angle = ball->angle - ANG90;
        tiny->target = ball->target;
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = (ball->momx >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finecosine[angle]);
        tiny->momy = (ball->momy >> 1) + FixedMul(ball->momz - FRACUNIT,
                                                  finesine[angle]);
        tiny->momz = ball->momz;
        P_CheckMissileSpawn(tiny);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL2
//
//----------------------------------------------------------------------------

void A_FireMacePL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    player->ammo[am_mace] -= deathmatch ? USE_MACE_AMMO_1 : USE_MACE_AMMO_2;
    mo = P_SpawnPlayerMissile(player->mo, MT_MACEFX4);
    if (mo)
    {
        mo->momx += player->mo->momx;
        mo->momy += player->mo->momy;
        mo->momz = 2 * FRACUNIT + ((player->lookdir) << (FRACBITS - 5));
        if (linetarget)
        {
            mo->special1.m = linetarget;
        }
    }
    S_StartSound(player->mo, sfx_lobsht);
}

//----------------------------------------------------------------------------
//
// PROC A_DeathBallImpact
//
//----------------------------------------------------------------------------

void A_DeathBallImpact(mobj_t * ball)
{
    int i;
    mobj_t *target;
    angle_t angle;
    boolean newAngle;

    if ((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
    {                           // Landed in some sort of liquid
        P_RemoveMobj(ball);
        return;
    }
    if ((ball->z <= ball->floorz) && ball->momz)
    {                           // Bounce
        newAngle = false;
        target = (mobj_t *) ball->special1.m;
        if (target)
        {
            if (!(target->flags & MF_SHOOTABLE))
            {                   // Target died
                ball->special1.m = NULL;
            }
            else
            {                   // Seek
                angle = R_PointToAngle2(ball->x, ball->y,
                                        target->x, target->y);
                newAngle = true;
            }
        }
        else
        {                       // Find new target
            angle = 0;
            for (i = 0; i < 16; i++)
            {
                P_AimLineAttack(ball, angle, 10 * 64 * FRACUNIT);
                if (linetarget && ball->target != linetarget)
                {
                    ball->special1.m = linetarget;
                    angle = R_PointToAngle2(ball->x, ball->y,
                                            linetarget->x, linetarget->y);
                    newAngle = true;
                    break;
                }
                angle += ANG45 / 2;
            }
        }
        if (newAngle)
        {
            ball->angle = angle;
            angle >>= ANGLETOFINESHIFT;
            ball->momx = FixedMul(ball->info->speed, finecosine[angle]);
            ball->momy = FixedMul(ball->info->speed, finesine[angle]);
        }
        P_SetMobjState(ball, ball->info->spawnstate);
        S_StartSound(ball, sfx_pstop);
    }
    else
    {                           // Explode
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        S_StartSound(ball, sfx_phohit);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_SpawnRippers
//
//----------------------------------------------------------------------------

void A_SpawnRippers(mobj_t * actor)
{
    unsigned int i;
    angle_t angle;
    mobj_t *ripper;

    for (i = 0; i < 8; i++)
    {
        ripper = P_SpawnMobj(actor->x, actor->y, actor->z, MT_RIPPER);
        angle = i * ANG45;
        ripper->target = actor->target;
        ripper->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        ripper->momx = FixedMul(ripper->info->speed, finecosine[angle]);
        ripper->momy = FixedMul(ripper->info->speed, finesine[angle]);
        P_CheckMissileSpawn(ripper);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_FireCrossbowPL1
//
//----------------------------------------------------------------------------

void A_FireCrossbowPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    pmo = player->mo;
    player->ammo[am_crossbow] -= USE_CBOW_AMMO_1;
    P_SpawnPlayerMissile(pmo, MT_CRBOWFX1);
    P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle - (ANG45 / 10));
    P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle + (ANG45 / 10));
}

//----------------------------------------------------------------------------
//
// PROC A_FireCrossbowPL2
//
//----------------------------------------------------------------------------

void A_FireCrossbowPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    pmo = player->mo;
    player->ammo[am_crossbow] -=
        deathmatch ? USE_CBOW_AMMO_1 : USE_CBOW_AMMO_2;
    P_SpawnPlayerMissile(pmo, MT_CRBOWFX2);
    P_SPMAngle(pmo, MT_CRBOWFX2, pmo->angle - (ANG45 / 10));
    P_SPMAngle(pmo, MT_CRBOWFX2, pmo->angle + (ANG45 / 10));
    P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle - (ANG45 / 5));
    P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle + (ANG45 / 5));
}

//----------------------------------------------------------------------------
//
// PROC A_BoltSpark
//
//----------------------------------------------------------------------------

void A_BoltSpark(mobj_t * bolt)
{
    mobj_t *spark;

    if (P_Random() > 50)
    {
        spark = P_SpawnMobj(bolt->x, bolt->y, bolt->z, MT_CRBOWFX4);
        spark->x += P_SubRandom() << 10;
        spark->y += P_SubRandom() << 10;
    }
}

//----------------------------------------------------------------------------
//
// PROC A_FireSkullRodPL1
//
//----------------------------------------------------------------------------

void A_FireSkullRodPL1(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    if (player->ammo[am_skullrod] < USE_SKRD_AMMO_1)
    {
        return;
    }
    player->ammo[am_skullrod] -= USE_SKRD_AMMO_1;
    mo = P_SpawnPlayerMissile(player->mo, MT_HORNRODFX1);
    // Randomize the first frame
    if (mo && P_Random() > 128)
    {
        P_SetMobjState(mo, S_HRODFX1_2);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_FireSkullRodPL2
//
// The special2 field holds the player number that shot the rain missile.
// The special1 field is used for the seeking routines, then as a counter
// for the sound looping.
//
//----------------------------------------------------------------------------

void A_FireSkullRodPL2(player_t * player, pspdef_t * psp)
{
    player->ammo[am_skullrod] -=
        deathmatch ? USE_SKRD_AMMO_1 : USE_SKRD_AMMO_2;
    P_SpawnPlayerMissile(player->mo, MT_HORNRODFX2);
    // Use MissileMobj instead of the return value from
    // P_SpawnPlayerMissile because we need to give info to the mobj
    // even if it exploded immediately.
    if (netgame)
    {                           // Multi-player game
        MissileMobj->special2.i = P_GetPlayerNum(player);
    }
    else
    {                           // Always use red missiles in single player games
        MissileMobj->special2.i = 2;
    }
    if (linetarget)
    {
        MissileMobj->special1.m = linetarget;
    }
    S_StartSound(MissileMobj, sfx_hrnpow);
}

//----------------------------------------------------------------------------
//
// PROC A_SkullRodPL2Seek
//
//----------------------------------------------------------------------------

void A_SkullRodPL2Seek(mobj_t * actor)
{
    P_SeekerMissile(actor, ANG1_X * 10, ANG1_X * 30);
}

//----------------------------------------------------------------------------
//
// PROC A_AddPlayerRain
//
//----------------------------------------------------------------------------

void A_AddPlayerRain(mobj_t * actor)
{
    int playerNum;
    player_t *player;

    playerNum = netgame ? actor->special2.i : 0;
    if (!playeringame[playerNum])
    {                           // Player left the game
        return;
    }
    player = &players[playerNum];
    if (player->health <= 0)
    {                           // Player is dead
        return;
    }
    if (player->rain1 && player->rain2)
    {                           // Terminate an active rain
        if (player->rain1->health < player->rain2->health)
        {
            if (player->rain1->health > 16)
            {
                player->rain1->health = 16;
            }
            player->rain1 = NULL;
        }
        else
        {
            if (player->rain2->health > 16)
            {
                player->rain2->health = 16;
            }
            player->rain2 = NULL;
        }
    }
    // Add rain mobj to list
    if (player->rain1)
    {
        player->rain2 = actor;
    }
    else
    {
        player->rain1 = actor;
    }
}

//----------------------------------------------------------------------------
//
// PROC A_SkullRodStorm
//
//----------------------------------------------------------------------------

void A_SkullRodStorm(mobj_t * actor)
{
    fixed_t x;
    fixed_t y;
    mobj_t *mo;
    int playerNum;
    player_t *player;

    if (actor->health-- == 0)
    {
        P_SetMobjState(actor, S_NULL);
        playerNum = netgame ? actor->special2.i : 0;
        if (!playeringame[playerNum])
        {                       // Player left the game
            return;
        }
        player = &players[playerNum];
        if (player->health <= 0)
        {                       // Player is dead
            return;
        }
        if (player->rain1 == actor)
        {
            player->rain1 = NULL;
        }
        else if (player->rain2 == actor)
        {
            player->rain2 = NULL;
        }
        return;
    }
    if (P_Random() < 25)
    {                           // Fudge rain frequency
        return;
    }
    x = actor->x + ((P_Random() & 127) - 64) * FRACUNIT;
    y = actor->y + ((P_Random() & 127) - 64) * FRACUNIT;
    mo = P_SpawnMobj(x, y, ONCEILINGZ, MT_RAINPLR1 + actor->special2.i);
    mo->target = actor->target;
    mo->momx = 1;               // Force collision detection
    mo->momz = -mo->info->speed;
    mo->special2.i = actor->special2.i;     // Transfer player number
    P_CheckMissileSpawn(mo);
    if (!(actor->special1.i & 31))
    {
        S_StartSound(actor, sfx_ramrain);
    }
    actor->special1.i++;
}

//----------------------------------------------------------------------------
//
// PROC A_RainImpact
//
//----------------------------------------------------------------------------

void A_RainImpact(mobj_t * actor)
{
    if (actor->z > actor->floorz)
    {
        P_SetMobjState(actor, S_RAINAIRXPLR1_1 + actor->special2.i);
    }
    else if (P_Random() < 40)
    {
        P_HitFloor(actor);
    }
}

//----------------------------------------------------------------------------
//
// PROC A_HideInCeiling
//
//----------------------------------------------------------------------------

void A_HideInCeiling(mobj_t * actor)
{
    actor->z = actor->ceilingz + 4 * FRACUNIT;
}

//----------------------------------------------------------------------------
//
// PROC A_FirePhoenixPL1
//
//----------------------------------------------------------------------------

void A_FirePhoenixPL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;

    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_1;
    P_SpawnPlayerMissile(player->mo, MT_PHOENIXFX1);
    //P_SpawnPlayerMissile(player->mo, MT_MNTRFX2);
    angle = player->mo->angle + ANG180;
    angle >>= ANGLETOFINESHIFT;
    player->mo->momx += FixedMul(4 * FRACUNIT, finecosine[angle]);
    player->mo->momy += FixedMul(4 * FRACUNIT, finesine[angle]);
}

//----------------------------------------------------------------------------
//
// PROC A_PhoenixPuff
//
//----------------------------------------------------------------------------

void A_PhoenixPuff(mobj_t * actor)
{
    mobj_t *puff;
    angle_t angle;

    P_SeekerMissile(actor, ANG1_X * 5, ANG1_X * 10);
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PHOENIXPUFF);
    angle = actor->angle + ANG90;
    angle >>= ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
    puff = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PHOENIXPUFF);
    angle = actor->angle - ANG90;
    angle >>= ANGLETOFINESHIFT;
    puff->momx = FixedMul((fixed_t)(FRACUNIT * 1.3), finecosine[angle]);
    puff->momy = FixedMul((fixed_t)(FRACUNIT * 1.3), finesine[angle]);
    puff->momz = 0;
}

//
// This function was present in the Heretic 1.0 executable for the
// removed "secondary phoenix flash" object (MT_PHOENIXFX_REMOVED).
// The purpose of this object is unknown, as is this function.
//

void A_RemovedPhoenixFunc(mobj_t *actor)
{
    I_Error("Action function invoked for removed Phoenix action!");
}

//----------------------------------------------------------------------------
//
// PROC A_InitPhoenixPL2
//
//----------------------------------------------------------------------------

void A_InitPhoenixPL2(player_t * player, pspdef_t * psp)
{
    player->flamecount = FLAME_THROWER_TICS;
}

//----------------------------------------------------------------------------
//
// PROC A_FirePhoenixPL2
//
// Flame thrower effect.
//
//----------------------------------------------------------------------------

void A_FirePhoenixPL2(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    mobj_t *pmo;
    angle_t angle;
    fixed_t x, y, z;
    fixed_t slope;

    if (--player->flamecount == 0)
    {                           // Out of flame
        P_SetPsprite(player, ps_weapon, S_PHOENIXATK2_4);
        player->refire = 0;
        return;
    }
    pmo = player->mo;
    angle = pmo->angle;
    x = pmo->x + (P_SubRandom() << 9);
    y = pmo->y + (P_SubRandom() << 9);
    z = pmo->z + 26 * FRACUNIT + ((player->lookdir) << FRACBITS) / 173;
    if (pmo->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    slope = ((player->lookdir) << FRACBITS) / 173 + (FRACUNIT / 10);
    mo = P_SpawnMobj(x, y, z, MT_PHOENIXFX2);
    mo->target = pmo;
    mo->angle = angle;
    mo->momx = pmo->momx + FixedMul(mo->info->speed,
                                    finecosine[angle >> ANGLETOFINESHIFT]);
    mo->momy = pmo->momy + FixedMul(mo->info->speed,
                                    finesine[angle >> ANGLETOFINESHIFT]);
    mo->momz = FixedMul(mo->info->speed, slope);
    if (!player->refire || !(leveltime % 38))
    {
        S_StartSound(player->mo, sfx_phopow);
    }
    P_CheckMissileSpawn(mo);
}

//----------------------------------------------------------------------------
//
// PROC A_ShutdownPhoenixPL2
//
//----------------------------------------------------------------------------

void A_ShutdownPhoenixPL2(player_t * player, pspdef_t * psp)
{
    player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
}

//----------------------------------------------------------------------------
//
// PROC A_FlameEnd
//
//----------------------------------------------------------------------------

void A_FlameEnd(mobj_t * actor)
{
    actor->momz += (fixed_t)(1.5 * FRACUNIT);
}

//----------------------------------------------------------------------------
//
// PROC A_FloatPuff
//
//----------------------------------------------------------------------------

void A_FloatPuff(mobj_t * puff)
{
    puff->momz += (fixed_t)(1.8 * FRACUNIT);
}

//---------------------------------------------------------------------------
//
// PROC A_GauntletAttack
//
//---------------------------------------------------------------------------

void A_GauntletAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    int randVal;
    fixed_t dist;

    psp->sx = ((P_Random() & 3) - 2) * FRACUNIT;
    psp->sy = WEAPONTOP + (P_Random() & 3) * FRACUNIT;
    angle = player->mo->angle;
    if (player->powers[pw_weaponlevel2])
    {
        damage = HITDICE(2);
        dist = 4 * MELEERANGE;
        angle += P_SubRandom() << 17;
        PuffType = MT_GAUNTLETPUFF2;
    }
    else
    {
        damage = HITDICE(2);
        dist = MELEERANGE + 1;
        angle += P_SubRandom() << 18;
        PuffType = MT_GAUNTLETPUFF1;
    }
    slope = P_AimLineAttack(player->mo, angle, dist);
    P_LineAttack(player->mo, angle, dist, slope, damage);
    if (!linetarget)
    {
        if (P_Random() > 64)
        {
            player->extralight = !player->extralight;
        }
        S_StartSound(player->mo, sfx_gntful);
        return;
    }
    randVal = P_Random();
    if (randVal < 64)
    {
        player->extralight = 0;
    }
    else if (randVal < 160)
    {
        player->extralight = 1;
    }
    else
    {
        player->extralight = 2;
    }
    if (player->powers[pw_weaponlevel2])
    {
        P_GiveBody(player, damage >> 1);
        S_StartSound(player->mo, sfx_gntpow);
    }
    else
    {
        S_StartSound(player->mo, sfx_gnthit);
    }
    // turn to face target
    angle = R_PointToAngle2(player->mo->x, player->mo->y,
                            linetarget->x, linetarget->y);
    if (angle - player->mo->angle > ANG180)
    {
        if (angle - player->mo->angle < -ANG90 / 20)
            player->mo->angle = angle + ANG90 / 21;
        else
            player->mo->angle -= ANG90 / 20;
    }
    else
    {
        if (angle - player->mo->angle > ANG90 / 20)
            player->mo->angle = angle - ANG90 / 21;
        else
            player->mo->angle += ANG90 / 20;
    }
    player->mo->flags |= MF_JUSTATTACKED;
}

void A_Light0(player_t * player, pspdef_t * psp)
{
    player->extralight = 0;
}

void A_Light1(player_t * player, pspdef_t * psp)
{
    player->extralight = 1;
}

void A_Light2(player_t * player, pspdef_t * psp)
{
    player->extralight = 2;
}

//------------------------------------------------------------------------
//
// PROC P_SetupPsprites
//
// Called at start of level for each player
//
//------------------------------------------------------------------------

void P_SetupPsprites(player_t * player)
{
    int i;

    // Remove all psprites
    for (i = 0; i < NUMPSPRITES; i++)
    {
        player->psprites[i].state = NULL;
    }
    // Spawn the ready weapon
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon(player);
}

//------------------------------------------------------------------------
//
// PROC P_MovePsprites
//
// Called every tic by player thinking routine
//
//------------------------------------------------------------------------

void P_MovePsprites(player_t * player)
{
    int i;
    pspdef_t *psp;
    state_t *state;

    psp = &player->psprites[0];
    for (i = 0; i < NUMPSPRITES; i++, psp++)
    {
        if ((state = psp->state) != 0)  // a null state means not active
        {
            // drop tic count and possibly change state
            if (psp->tics != -1)        // a -1 tic count never changes
            {
                psp->tics--;
                if (!psp->tics)
                {
                    P_SetPsprite(player, i, psp->state->nextstate);
                }
            }
        }
    }
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}
