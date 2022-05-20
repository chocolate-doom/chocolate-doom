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
//	Weapon sprite animation, weapon objects.
//	Action functions for weapons.
//


#include "doomdef.h"
#include "d_event.h"

#include "deh_misc.h"

#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// State.
#include "doomstat.h"

// Data.
#include "sounds.h"

#include "p_pspr.h"

#define LOWERSPEED		FRACUNIT*6
#define RAISESPEED		FRACUNIT*6

#define WEAPONBOTTOM	128*FRACUNIT
#define WEAPONTOP		32*FRACUNIT



//
// P_SetPsprite
//
// [STRIFE]
// villsa: Removed psprite sx, sy modification via misc1/2
//
void
P_SetPsprite
( player_t*     player,
  int           position,
  statenum_t    stnum )
{
    pspdef_t*   psp;
    state_t*    state;

    psp = &player->psprites[position];

    do
    {
        if (!stnum)
        {
            // object removed itself
            psp->state = NULL;
            break;
        }

        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;        // could be 0

        // villsa [STRIFE] unused
        /*if (state->misc1)
        {
            // coordinate set
            psp->sx = state->misc1 << FRACBITS;
            psp->sy = state->misc2 << FRACBITS;
        }*/

        // Call action routine.
        // Modified handling.
        if (state->action.acp2)
        {
            state->action.acp2(player, psp);
            if (!psp->state)
                break;
        }

        stnum = psp->state->nextstate;

    } while (!psp->tics);
    // an initial state of 0 could cycle through
}

// haleyjd 09/06/10: [STRIFE] Removed P_CalcSwing

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
// villsa [STRIFE] Modifications for Strife weapons
//
void P_BringUpWeapon (player_t* player)
{
    statenum_t  newstate;

    if (player->pendingweapon == wp_nochange)
        player->pendingweapon = player->readyweapon;

    if (player->pendingweapon == wp_flame)
        S_StartSound (player->mo, sfx_flidl);   // villsa [STRIFE] flame sounds

    newstate = weaponinfo[player->pendingweapon].upstate;

    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite (player, ps_weapon, newstate);

    // villsa [STRIFE] set various flash states
    if(player->pendingweapon == wp_elecbow)
        P_SetPsprite(player, ps_flash, S_XBOW_10); // 31
    else if(player->pendingweapon == wp_sigil && player->sigiltype)
        P_SetPsprite(player, ps_flash, S_SIGH_00 + player->sigiltype); // 117
    else
        P_SetPsprite(player, ps_flash, S_NULL);

    player->pendingweapon = wp_nochange;
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
// villsa [STRIFE] Changes to handle Strife weapons
//
boolean P_CheckAmmo (player_t* player)
{
    ammotype_t          ammo;
    int                 count;

    ammo = weaponinfo[player->readyweapon].ammo;

    // Minimal amount for one shot varies.
    if (player->readyweapon == wp_torpedo)
        count = 30;
    else if (player->readyweapon == wp_mauler)
        count = 20;
    else
        count = 1;      // Regular.

    // Some do not need ammunition anyway.
    // Return if current ammunition sufficient.
    if (ammo == am_noammo || player->ammo[ammo] >= count)
        return true;

    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.

    // villsa [STRIFE] new weapon preferences
    if (player->weaponowned[wp_mauler] && player->ammo[am_cell] >= 20)
        player->pendingweapon = wp_mauler;

    else if(player->weaponowned[wp_rifle] && player->ammo[am_bullets])
        player->pendingweapon = wp_rifle;

    else if (player->weaponowned[wp_elecbow] && player->ammo[am_elecbolts])
        player->pendingweapon = wp_elecbow;

    else if (player->weaponowned[wp_missile] && player->ammo[am_missiles])
        player->pendingweapon = wp_missile;

    else if (player->weaponowned[wp_flame] && player->ammo[am_cell])
        player->pendingweapon = wp_flame;

    else if (player->weaponowned[wp_hegrenade] && player->ammo[am_hegrenades])
        player->pendingweapon = wp_hegrenade;

    else if (player->weaponowned[wp_poisonbow] && player->ammo[am_poisonbolts])
        player->pendingweapon = wp_poisonbow;

    else if (player->weaponowned[wp_wpgrenade] && player->ammo[am_wpgrenades])
        player->pendingweapon = wp_wpgrenade;

    // BUG: This will *never* be selected for an automatic switch because the 
    // normal Mauler is higher priority and uses less ammo.
    else if (player->weaponowned[wp_torpedo] && player->ammo[am_cell] >= 30)
        player->pendingweapon = wp_torpedo; 

    else
        player->pendingweapon = wp_fist;


    // Now set appropriate weapon overlay.
    P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);

    return false;
}


//
// P_FireWeapon.
//
// villsa [STRIFE] Changes for player state and weapons
//
void P_FireWeapon (player_t* player)
{
    statenum_t  newstate;

    if (!P_CheckAmmo (player))
        return;

    P_SetMobjState (player->mo, S_PLAY_05); // 292
    newstate = weaponinfo[player->readyweapon].atkstate;
    P_SetPsprite (player, ps_weapon, newstate);
    
    // villsa [STRIFE] exclude these weapons from causing noise
    if(player->readyweapon > wp_elecbow && player->readyweapon != wp_poisonbow)
        P_NoiseAlert (player->mo, player->mo);
}



//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon (player_t* player)
{
    P_SetPsprite (player,
                  ps_weapon,
                  weaponinfo[player->readyweapon].downstate);
}



//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady( player_t* player, pspdef_t* psp)
{
    statenum_t  newstate;
    int         angle;
    
    // get out of attack state
    if (player->mo->state == &states[S_PLAY_05] || // 292
        player->mo->state == &states[S_PLAY_06])   // 293
    {
        P_SetMobjState (player->mo, S_PLAY_00); // 287
    }
    
    // villsa [STRIFE] check for wp_flame instead of chainsaw
    // haleyjd 09/06/10: fixed state (00 rather than 01)
    if (player->readyweapon == wp_flame
        && psp->state == &states[S_FLMT_00]) // 62
    {
        S_StartSound (player->mo, sfx_flidl);
    }
    
    // check for change
    //  if player is dead, put the weapon away
    if (player->pendingweapon != wp_nochange || !player->health)
    {
        // change weapon
        //  (pending weapon should allready be validated)
        newstate = weaponinfo[player->readyweapon].downstate;
        P_SetPsprite (player, ps_weapon, newstate);
        return;
    }
    
    // check for fire
    //  the missile launcher and torpedo do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
        if ( !player->attackdown
            || (player->readyweapon != wp_missile
            && player->readyweapon != wp_torpedo)) // villsa [STRIFE] replace bfg with torpedo
        {
            player->attackdown = true;
            P_FireWeapon (player);
            return;
        }
    }
    else
        player->attackdown = false;
    
    // bob the weapon based on movement speed
    angle = (128*leveltime)&FINEMASK;
    psp->sx = FRACUNIT + FixedMul (player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul (player->bob, finesine[angle]);
}



//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire
( player_t*	player,
  pspdef_t*	psp )
{
    
    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if ( (player->cmd.buttons & BT_ATTACK) 
        && player->pendingweapon == wp_nochange
        && player->health)
    {
        player->refire++;
        P_FireWeapon (player);
    }
    else
    {
        player->refire = 0;
        P_CheckAmmo (player);
    }
}

//
// A_CheckReload
//
void A_CheckReload(player_t* player, pspdef_t* psp)
{
    P_CheckAmmo(player);

    // villsa [STRIFE] set animating sprite for crossbow
    if(player->readyweapon == wp_elecbow)
        P_SetPsprite(player, player->readyweapon, S_XBOW_10);
}



//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//
void
A_Lower
( player_t*	player,
  pspdef_t*	psp )
{	
    psp->sy += LOWERSPEED;

    // Is already down.
    if (psp->sy < WEAPONBOTTOM )
        return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
        psp->sy = WEAPONBOTTOM;

        // don't bring weapon back up
        return;
    }
    
    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->health)
    {
        // Player is dead, so keep the weapon off screen.
        P_SetPsprite (player,  ps_weapon, S_NULL);
        return;
    }

    player->readyweapon = player->pendingweapon; 

    P_BringUpWeapon (player);
}


//
// A_Raise
//
void
A_Raise
( player_t*	player,
  pspdef_t*	psp )
{
    statenum_t  newstate;

    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP )
        return;

    psp->sy = WEAPONTOP;

    // The weapon has been raised all the way,
    //  so change to the ready state.
    newstate = weaponinfo[player->readyweapon].readystate;

    P_SetPsprite (player, ps_weapon, newstate);
}



//
// A_GunFlash
//
void
A_GunFlash
( player_t*	player,
  pspdef_t*	psp ) 
{
    P_SetMobjState (player->mo, S_PLAY_06);
    P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);
}



//
// WEAPON ATTACKS
//


//
// A_Punch
//

void A_Punch(player_t* player, pspdef_t* psp) 
{
    angle_t     angle;
    int         damage;
    int         slope;
    int         sound;
    int         stamina;
    int         t;

    // villsa [STRIFE] new damage formula
    // haleyjd 09/19/10: seriously corrected...
    stamina = player->stamina;
    damage = (P_Random() & ((stamina/10) + 7)) * ((stamina/10) + 2);

    if(player->powers[pw_strength])
        damage *= 10;

    angle = player->mo->angle;
    t = P_Random();
    angle += (t - P_Random()) << 18;
    slope = P_AimLineAttack (player->mo, angle, PLAYERMELEERANGE);
    P_LineAttack (player->mo, angle, PLAYERMELEERANGE, slope, damage);

    // turn to face target
    if(linetarget)
    {
        // villsa [STRIFE] check for non-flesh types
        if(linetarget->flags & MF_NOBLOOD)
            sound = sfx_mtalht;
        else
            sound = sfx_meatht;

        S_StartSound (player->mo, sound);
        player->mo->angle = R_PointToAngle2 (player->mo->x,
                                             player->mo->y,
                                             linetarget->x,
                                             linetarget->y);

        // villsa [STRIFE] apply flag
        player->mo->flags |= MF_JUSTATTACKED;

        // villsa [STRIFE] do punch alert routine
        P_DoPunchAlert(player->mo, linetarget);
    }
    else
        S_StartSound (player->mo, sfx_swish);
}


//
// A_FireFlameThrower
//
// villsa [STRIFE] new codepointer
//
void A_FireFlameThrower(player_t* player, pspdef_t* psp) 
{
    mobj_t* mo;
    int t;

    P_SetMobjState(player->mo, S_PLAY_06);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    t = P_Random();
    player->mo->angle += (t - P_Random()) << 18;

    mo = P_SpawnPlayerMissile(player->mo, MT_SFIREBALL);
    mo->momz += (5*FRACUNIT);
}

//
// A_FireMissile
//
// villsa [STRIFE] completly new compared to the original
//
void A_FireMissile(player_t* player, pspdef_t* psp) 
{
    angle_t an;
    int t;

    // haleyjd 09/19/10: I previously missed an add op that meant it should be
    // accuracy * 5, not 4. Checks out with other sources.
    an = player->mo->angle;
    t = P_Random();
    player->mo->angle += (t - P_Random()) << (19 - (player->accuracy * 5 / 100));
    P_SetMobjState(player->mo, S_PLAY_06);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_MINIMISSLE);
    player->mo->angle = an;
}

//
// A_FireMauler2
//
// villsa [STRIFE] - new codepointer
//
void A_FireMauler2(player_t* player, pspdef_t* pspr)
{
    P_SetMobjState(player->mo, S_PLAY_06);
    P_DamageMobj(player->mo, player->mo, NULL, 20);
    player->ammo[weaponinfo[player->readyweapon].ammo] -= 30;
    P_SpawnPlayerMissile(player->mo, MT_TORPEDO);
    P_Thrust(player, player->mo->angle + ANG180, 512000);
}

//
// A_FireGrenade
//
// villsa [STRIFE] - new codepointer
//
void A_FireGrenade(player_t* player, pspdef_t* pspr)
{
    mobjtype_t type;
    mobj_t* mo;
    state_t* st1;
    state_t* st2;
    angle_t an;
    fixed_t radius;

    // decide on what type of grenade to spawn
    if(player->readyweapon == wp_hegrenade)
    {
        type = MT_HEGRENADE;
    }
    else if(player->readyweapon == wp_wpgrenade)
    {
        type = MT_PGRENADE;
    }
    else
    {
        type = MT_HEGRENADE;
        fprintf(stderr, "Warning: A_FireGrenade used on wrong weapon!\n");
    }

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    // set flash frame
    st1 = &states[(pspr->state - states) + weaponinfo[player->readyweapon].flashstate];
    st2 = &states[weaponinfo[player->readyweapon].atkstate];
    P_SetPsprite(player, ps_flash, st1 - st2);

    player->mo->z += 32*FRACUNIT; // ugh
    mo = P_SpawnMortar(player->mo, type);
    player->mo->z -= 32*FRACUNIT; // ugh

    // change momz based on player's pitch
    mo->momz = FixedMul((player->pitch<<FRACBITS) / 160, mo->info->speed) + (8*FRACUNIT);
    S_StartSound(mo, mo->info->seesound);

    radius = mobjinfo[type].radius + player->mo->info->radius;
    an = (player->mo->angle >> ANGLETOFINESHIFT);

    mo->x += FixedMul(finecosine[an], radius + (4*FRACUNIT));
    mo->y += FixedMul(finesine[an], radius + (4*FRACUNIT));

    // shoot grenade from left or right side?
    if(&states[weaponinfo[player->readyweapon].atkstate] == pspr->state)
        an = (player->mo->angle - ANG90) >> ANGLETOFINESHIFT;
    else
        an = (player->mo->angle + ANG90) >> ANGLETOFINESHIFT;

    mo->x += FixedMul((15*FRACUNIT), finecosine[an]);
    mo->y += FixedMul((15*FRACUNIT), finesine[an]);

    // set bounce flag
    mo->flags |= MF_BOUNCE;
}

//
// A_FireElectricBolt
// villsa [STRIFE] - new codepointer
//

void A_FireElectricBolt(player_t* player, pspdef_t* pspr)
{
    angle_t an = player->mo->angle;
    int t;

    // haleyjd 09/19/10: Use 5 mul on accuracy here as well
    t = P_Random();
    player->mo->angle += (t - P_Random()) << (18 - (player->accuracy * 5 / 100));
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_ELECARROW);
    player->mo->angle = an;
    S_StartSound(player->mo, sfx_xbow);
}

//
// A_FirePoisonBolt
// villsa [STRIFE] - new codepointer
//

void A_FirePoisonBolt(player_t* player, pspdef_t* pspr)
{
    angle_t an = player->mo->angle;
    int t;

    // haleyjd 09/19/10: Use 5 mul on accuracy here as well
    t = P_Random();
    player->mo->angle += (t - P_Random()) << (18 - (player->accuracy * 5 / 100));
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_POISARROW);
    player->mo->angle = an;
    S_StartSound(player->mo, sfx_xbow);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
// haleyjd 09/06/10 [STRIFE] Modified with a little target hack...
//
fixed_t         bulletslope;


void P_BulletSlope (mobj_t *mo)
{
    angle_t	an;
    
    // see which target is to be aimed at
    an = mo->angle;
    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
        an += 1<<26;
        bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
        if (!linetarget)
        {
            an -= 2<<26;
            bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
        }
    }

    // haleyjd 09/06/10: [STRIFE] Somebody added this here, and without it, you
    // will get spurious crashing in routines such as P_LookForPlayers!
    if(linetarget)
        mo->target = linetarget;
}


//
// P_GunShot
//
// [STRIFE] Modifications to support accuracy.
//
void
P_GunShot
( mobj_t*	mo,
  boolean	accurate )
{
    angle_t     angle;
    int         damage;

    angle = mo->angle;

    // villsa [STRIFE] apply player accuracy
    // haleyjd 09/18/10: made some corrections: use 5x accuracy;
    // eliminated order-of-evaluation dependency
    if (!accurate)
    {
        int t = P_Random();
        angle += (t - P_Random()) << (20 - ((mo->player->accuracy * 5) / 100));
    }

    // haleyjd 09/18/10 [STRIFE] corrected damage formula and moved down to
    // preserve proper P_Random call order.
    damage = 4 * (P_Random() % 3 + 1);

    P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FireRifle
//
// villsa [STRIFE] - new codepointer
//
void A_FireRifle(player_t* player, pspdef_t* pspr)
{
    S_StartSound(player->mo, sfx_rifle);

    if(player->ammo[weaponinfo[player->readyweapon].ammo])
    {
        P_SetMobjState(player->mo, S_PLAY_06); // 293
        player->ammo[weaponinfo[player->readyweapon].ammo]--;
        P_BulletSlope(player->mo);
        P_GunShot(player->mo, !player->refire);
    }
}

//
// A_FireMauler1
//
// villsa [STRIFE] - new codepointer
//
void A_FireMauler1(player_t* player, pspdef_t* pspr)
{
    int i;
    angle_t angle;
    int damage;

    // haleyjd 09/18/10: Corrected ammo check to use >=
    if(player->ammo[weaponinfo[player->readyweapon].ammo] >= 20)
    {
        player->ammo[weaponinfo[player->readyweapon].ammo] -= 20;
        P_BulletSlope(player->mo);
        S_StartSound(player->mo, sfx_pgrdat);

        for(i = 0; i < 20; i++)
        {
            int t;
            damage = 5*(P_Random ()%3+1);
            angle = player->mo->angle;
            t = P_Random();
            angle += (t - P_Random()) << 19;
            t = P_Random();
            P_LineAttack(player->mo, angle, 2112*FRACUNIT,
                         bulletslope + ((t - P_Random())<<5), damage);
        }
    }
}

//
// A_SigilSound
//
// villsa [STRIFE] - new codepointer
//
void A_SigilSound(player_t* player, pspdef_t* pspr)
{
    S_StartSound(player->mo, sfx_siglup);
    player->extralight = 2;

}

//
// A_FireSigil
//
// villsa [STRIFE] - new codepointer
//
void A_FireSigil(player_t* player, pspdef_t* pspr)
{
    mobj_t* mo;
    angle_t an;
    int i;

    // keep info on armor because sigil does piercing damage
    i = player->armortype;
    player->armortype = 0;

    // BUG: setting inflictor causes firing the Sigil to always push the player
    // toward the east, no matter what direction he is facing.
    P_DamageMobj(player->mo, player->mo, NULL, 4 * (player->sigiltype + 1));

    // restore armor
    player->armortype = i;

    S_StartSound(player->mo, sfx_siglup);

    switch(player->sigiltype)
    {
        // falling lightning bolts from the sky
    case 0:
        P_BulletSlope(player->mo);
        if(linetarget)
        {
            // haleyjd 09/18/10: corrected z coordinate
            mo = P_SpawnMobj(linetarget->x, linetarget->y, ONFLOORZ, 
                             MT_SIGIL_A_GROUND);
            mo->tracer = linetarget;
        }
        else
        {
            an = player->mo->angle>>ANGLETOFINESHIFT;
            mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, 
                             MT_SIGIL_A_GROUND);
            mo->momx += FixedMul((28*FRACUNIT), finecosine[an]);
            mo->momy += FixedMul((28*FRACUNIT), finesine[an]);
        }
        mo->health = -1;
        mo->target = player->mo;
        break;

        // simple projectile
    case 1:
        P_SpawnPlayerMissile(player->mo, MT_SIGIL_B_SHOT)->health = -1;
        break;

        // spread shot
    case 2:
        player->mo->angle -= ANG90; // starting at 270...
        for(i = 0; i < 20; i++)     // increment by 1/10 of 90, 20 times.
        {
            player->mo->angle += (ANG90 / 10);
            mo = P_SpawnMortar(player->mo, MT_SIGIL_C_SHOT);
            mo->health = -1;
            mo->z = player->mo->z + (32*FRACUNIT);
        }
        player->mo->angle -= ANG90; // subtract off the extra 90
        break;

        // tracer attack
    case 3:
        P_BulletSlope(player->mo);
        if(linetarget)
        {
            mo = P_SpawnPlayerMissile(player->mo, MT_SIGIL_D_SHOT);
            mo->tracer = linetarget;
        }
        else
        {
            an = player->mo->angle >> ANGLETOFINESHIFT;
            mo = P_SpawnPlayerMissile(player->mo, MT_SIGIL_D_SHOT);
            mo->momx += FixedMul(mo->info->speed, finecosine[an]);
            mo->momy += FixedMul(mo->info->speed, finesine[an]);
        }
        mo->health = -1;
        break;

        // mega blast
    case 4:
        mo = P_SpawnPlayerMissile(player->mo, MT_SIGIL_E_SHOT);
        mo->health = -1;
        if(!linetarget)
        {
            an = (unsigned int)player->pitch >> ANGLETOFINESHIFT;
            mo->momz += FixedMul(finesine[an], mo->info->speed); 
        }
        break;

    default:
        break;
    }
}

//
// A_GunFlashThinker
//
// villsa [STRIFE] - new codepointer
//
void A_GunFlashThinker(player_t* player, pspdef_t* pspr)
{
    if(player->readyweapon == wp_sigil && player->sigiltype)
        P_SetPsprite(player, ps_flash, S_SIGH_00 + player->sigiltype);
    else
        P_SetPsprite(player, ps_flash, S_NULL);

}


//
// ?
//
void A_Light0 (player_t *player, pspdef_t *psp)
{
    player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
    player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
    player->extralight = 2;
}

//
// A_SigilShock
//
// villsa [STRIFE] - new codepointer
//
void A_SigilShock (player_t *player, pspdef_t *psp)
{
    player->extralight = -3;
}

//
// A_TorpedoExplode
//
// villsa [STRIFE] - new codepointer
//
void A_TorpedoExplode(mobj_t* actor)
{
    int i;

    actor->angle -= ANG180;

    for(i = 0; i < 80; i++)
    {
        actor->angle += (ANG90 / 20);
        P_SpawnMortar(actor, MT_TORPEDOSPREAD)->target = actor->target;
    }
}

//
// A_MaulerSound
//
// villsa [STRIFE] - new codepointer
//
void A_MaulerSound(player_t *player, pspdef_t *psp)
{
    int t;
    S_StartSound(player->mo, sfx_proton);
    t = P_Random();
    psp->sx += (t - P_Random()) << 10;
    t = P_Random();
    psp->sy += (t - P_Random()) << 10;

}


//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites(player_t* player) 
{
    int	i;

    // remove all psprites
    for(i = 0; i < NUMPSPRITES; i++)
        player->psprites[i].state = NULL;

    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon(player);
}




//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites (player_t* player) 
{
    int		i;
    pspdef_t*	psp;
    state_t*	state;

    psp = &player->psprites[0];
    for(i = 0; i < NUMPSPRITES; i++, psp++)
    {
        // a null state means not active
        if((state = psp->state))	
        {
            // drop tic count and possibly change state

            // a -1 tic count never changes
            if(psp->tics != -1)	
            {
                psp->tics--;
                if(!psp->tics)
                    P_SetPsprite (player, i, psp->state->nextstate);
            }
        }
    }
    
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;

    // villsa [STRIFE] extra stuff for targeter
    player->psprites[ps_targleft].sx =
        (160*FRACUNIT) - ((100 - player->accuracy) << FRACBITS);

    player->psprites[ps_targright].sx =
        ((100 - player->accuracy) << FRACBITS) + (160*FRACUNIT);
}


