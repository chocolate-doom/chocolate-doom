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

#include "p_local.h"
#include "m_random.h"
#include "s_sound.h"

extern void A_Explode(mobj_t* thingy);
extern void A_FaceTarget(mobj_t* actor);

extern boolean P_CheckMeleeRange (mobj_t *actor);
extern void P_Thrust (player_t* player, angle_t angle, fixed_t move);

// killough 11/98: kill an object
void A_Die(mobj_t *actor)
{
  P_DamageMobj(actor, NULL, NULL, actor->health);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//

void A_Detonate(mobj_t *mo)
{
  P_RadiusAttack(mo, mo->target, mo->info->damage);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
// Original idea: Linguica
//

void A_Mushroom(mobj_t *actor)
{
  int i, j, n = actor->info->damage;

  // Mushroom parameters are part of code pointer's state
  fixed_t misc1 = actor->state->misc1 ? actor->state->misc1 : FRACUNIT*4;
  fixed_t misc2 = actor->state->misc2 ? actor->state->misc2 : FRACUNIT/2;

  A_Explode(actor);               // make normal explosion

  for (i = -n; i <= n; i += 8)    // launch mushroom cloud
    for (j = -n; j <= n; j += 8)
      {
	mobj_t target = *actor, *mo;
	target.x += i << FRACBITS;    // Aim in many directions from source
	target.y += j << FRACBITS;
	target.z += P_AproxDistance(i,j) * misc1;           // Aim fairly high
	mo = P_SpawnMissile(actor, &target, MT_FATSHOT);    // Launch fireball
	mo->momx = FixedMul(mo->momx, misc2);
	mo->momy = FixedMul(mo->momy, misc2);               // Slow down a bit
	mo->momz = FixedMul(mo->momz, misc2);
	mo->flags &= ~MF_NOGRAVITY;   // Make debris fall under gravity
      }
}

//
// A_BetaSkullAttack()
// killough 10/98: this emulates the beta version's lost soul attacks
//

void A_BetaSkullAttack(mobj_t *actor)
{
  int damage;
  if (!actor->target || actor->target->type == MT_SKULL)
    return;
  S_StartSound(actor, actor->info->attacksound);
  A_FaceTarget(actor);
  damage = (P_Random(/* pr_skullfly */)%8+1)*actor->info->damage;
  P_DamageMobj(actor->target, actor, actor, damage);
}

void A_Stop(mobj_t *actor)
{
  actor->momx = actor->momy = actor->momz = 0;
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
//
// A small set of highly-sought-after code pointers
//

void A_Spawn(mobj_t *mo)
{
  if (mo->state->misc1)
    {
/*    mobj_t *newmobj = */ P_SpawnMobj(mo->x, mo->y,
				    (mo->state->misc2 << FRACBITS) + mo->z,
				    mo->state->misc1 - 1);
//    newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

    }
}

void A_Turn(mobj_t *mo)
{
  mo->angle += (angle_t)(((uint64_t) mo->state->misc1 << 32) / 360);
}

void A_Face(mobj_t *mo)
{
  mo->angle = (angle_t)(((uint64_t) mo->state->misc1 << 32) / 360);
}

void A_Scratch(mobj_t *mo)
{
  mo->target && (A_FaceTarget(mo), P_CheckMeleeRange(mo)) ?
    mo->state->misc2 ? S_StartSound(mo, mo->state->misc2) : (void) 0,
    P_DamageMobj(mo->target, mo, mo, mo->state->misc1) : (void) 0;
}

void A_PlaySound(mobj_t *mo)
{
  S_StartSound(mo->state->misc2 ? NULL : mo, mo->state->misc1);
}

// [crispy] this is pretty much the only action pointer that makes sense for both mobj and pspr states
void A_RandomJump(mobj_t *mo, player_t *player, pspdef_t *psp)
{
	// [crispy] first, try to apply to pspr states
	if (player && psp)
	{
		if (Crispy_Random() < psp->state->misc2)
		{
			extern void P_SetPsprite (player_t *player, int position, statenum_t stnum);

			P_SetPsprite(player, psp - &player->psprites[0], psp->state->misc1);
		}
	}
	else
	// [crispy] second, apply to mobj states
	if (mo)
	{
		if (Crispy_Random() < mo->state->misc2)
		{
			P_SetMobjState(mo, mo->state->misc1);
		}
	}
}

//
// This allows linedef effects to be activated inside deh frames.
//

void A_LineEffect(mobj_t *mo)
{
//if (!(mo->intflags & MIF_LINEDONE))                // Unless already used up
    {
      line_t junk = *lines;                          // Fake linedef set to 1st
      if ((junk.special = (short)mo->state->misc1))  // Linedef type
	{
	  static player_t player;                    // [crispy] made static
	  player_t *oldplayer = mo->player;          // Remember player status
	  mo->player = &player;                      // Fake player
	  player.health = 100;                       // Alive player
	  junk.tag = (short)mo->state->misc2;        // Sector tag for linedef
	  if (!P_UseSpecialLine(mo, &junk, 0))       // Try using it
	    P_CrossSpecialLinePtr(&junk, 0, mo);     // Try crossing it
//	  if (!junk.special)                         // If type cleared,
//	    mo->intflags |= MIF_LINEDONE;            // no more for this thing
	  mo->player = oldplayer;                    // Restore player status
	}
    }
}

//
// A_FireOldBFG
//
// This function emulates Doom's Pre-Beta BFG
// By Lee Killough 6/6/98, 7/11/98, 7/19/98, 8/20/98
//
// This code may not be used in other mods without appropriate credit given.
// Code leeches will be telefragged.

void A_FireOldBFG(mobj_t *mobj, player_t *player, pspdef_t *psp)
{
  int type = MT_PLASMA1;
  extern void P_CheckMissileSpawn (mobj_t* th);

  if (!player) return; // [crispy] let pspr action pointers get called from mobj states

  player->ammo[weaponinfo[player->readyweapon].ammo]--;

  player->extralight = 2;

  do
    {
      mobj_t *th, *mo = player->mo;
      angle_t an = mo->angle;
      angle_t an1 = ((P_Random(/* pr_bfg */)&127) - 64) * (ANG90/768) + an;
      angle_t an2 = ((P_Random(/* pr_bfg */)&127) - 64) * (ANG90/640) + ANG90;
//    extern int autoaim;

//    if (autoaim || !beta_emulation)
	{
	  // killough 8/2/98: make autoaiming prefer enemies
	  int mask = 0;//MF_FRIEND;
	  fixed_t slope;
	  if (critical->freeaim == FREEAIM_DIRECT)
	    slope = PLAYER_SLOPE(player);
	  else
	  do
	    {
	      slope = P_AimLineAttack(mo, an, 16*64*FRACUNIT);//, mask);
	      if (!linetarget)
		slope = P_AimLineAttack(mo, an += 1<<26, 16*64*FRACUNIT);//, mask);
	      if (!linetarget)
		slope = P_AimLineAttack(mo, an -= 2<<26, 16*64*FRACUNIT);//, mask);
	      if (!linetarget)
		slope = (critical->freeaim == FREEAIM_BOTH) ? PLAYER_SLOPE(player) : 0, an = mo->angle;
	    }
	  while (mask && (mask=0, !linetarget));     // killough 8/2/98
	  an1 += an - mo->angle;
	  // [crispy] consider negative slope
	  if (slope < 0)
	    an2 -= tantoangle[-slope >> DBITS];
	  else
	  an2 += tantoangle[slope >> DBITS];
	}

      th = P_SpawnMobj(mo->x, mo->y,
		       mo->z + 62*FRACUNIT - player->psprites[ps_weapon].sy,
		       type);
      // [NS] Play projectile sound.
      if (th->info->seesound)
      {
	S_StartSound (th, th->info->seesound);
      }
      th->target = mo; // P_SetTarget(&th->target, mo);
      th->angle = an1;
      // [NS] Use speed from thing info.
      th->momx = FixedMul(th->info->speed, finecosine[an1>>ANGLETOFINESHIFT]);
      th->momy = FixedMul(th->info->speed, finesine[an1>>ANGLETOFINESHIFT]);
      th->momz = FixedMul(th->info->speed, finetangent[an2>>ANGLETOFINESHIFT]);
      // [crispy] suppress interpolation of player missiles for the first tic
      th->interp = -1;
      P_CheckMissileSpawn(th);
    }
  while ((type != MT_PLASMA2) && (type = MT_PLASMA2)); //killough: obfuscated!
}
