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
//	Moving object handling. Spawn functions.
//

#include <stdio.h>

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "doomdef.h"
#include "p_local.h"
#include "sounds.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "s_sound.h"
#include "doomstat.h"
#include "d_main.h"     // villsa [STRIFE]

extern line_t *spechit[];  // haleyjd:
extern int     numspechit; // [STRIFE] - needed in P_XYMovement


void G_PlayerReborn (int player);
void P_SpawnMapThing (mapthing_t*	mthing);


//
// P_SetMobjState
// Returns true if the mobj is still present.
//
// [STRIFE] Verified unmodified
//
int test;

boolean
P_SetMobjState
( mobj_t*	mobj,
  statenum_t	state )
{
    state_t*	st;

    do
    {
	if (state == S_NULL)
	{
	    mobj->state = (state_t *) S_NULL;
	    P_RemoveMobj (mobj);
	    return false;
	}

	st = &states[state];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;

	// Modified handling.
	// Call action functions when the state is set
	if (st->action.acp1)		
	    st->action.acp1(mobj);	
	
	state = st->nextstate;
    } while (!mobj->tics);
				
    return true;
}


//
// P_ExplodeMissile
//
// [STRIFE] Removed randomization of deathstate tics
//
void P_ExplodeMissile (mobj_t* mo)
{
    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, mobjinfo[mo->type].deathstate);

    // villsa [STRIFE] removed tics randomization

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
        S_StartSound (mo, mo->info->deathsound);
}


//
// P_XYMovement
//
// [STRIFE] Modifications for:
// * No SKULLFLY logic (replaced by BOUNCE flag)
// * Missiles can activate G1/GR line types
// * Player walking logic
// * Air friction for players
//
#define STOPSPEED       0x1000
#define FRICTION        0xe800
#define AIRFRICTION     0xfff0  // [STRIFE]

void P_XYMovement (mobj_t* mo) 
{
    fixed_t     ptryx;
    fixed_t     ptryy;
    player_t*   player;
    fixed_t     xmove;
    fixed_t     ymove;

    // villsa [STRIFE] unused
    /*
    if (!mo->momx && !mo->momy)
    {
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;

            P_SetMobjState (mo, mo->info->spawnstate);
        }
        return;
    }
    */

    player = mo->player;

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    do
    {
        if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_TryMove (mo, ptryx, ptryy))
        {
            // blocked move
            if (mo->player)
            {   // try to slide along it
                P_SlideMove (mo);
            }
            // villsa [STRIFE] check for bouncy missiles
            else if(mo->flags & MF_BOUNCE)
            {
                mo->momx >>= 3;
                mo->momy >>= 3;

                if (P_TryMove(mo, mo->x - xmove, ymove + mo->y))
                    mo->momy = -mo->momy;
                else
                    mo->momx = -mo->momx;

                xmove = 0;
                ymove = 0;
            }
            else if (mo->flags & MF_MISSILE)
            {
                // haley 20110203: [STRIFE]
                // This modification allows missiles to activate shoot specials.
                // *** BUG: In vanilla Strife the second condition is simply
                // if(numspechit). However, numspechit can be negative, and
                // when it is, this accesses spechit[-2]. This always causes the
                // DOS exe to read from NULL, and the 'special' value there (in
                // DOS 6.22 at least) is 0x70, which does nothing.
                if(blockingline && blockingline->special)
                    P_ShootSpecialLine(mo, blockingline);
                if(numspechit > 0)
                    P_ShootSpecialLine(mo, spechit[numspechit-1]);

                // explode a missile
                if (ceilingline &&
                    ceilingline->backsector &&
                    ceilingline->backsector->ceilingpic == skyflatnum)
                {
                    // Hack to prevent missiles exploding
                    // against the sky.
                    // Does not handle sky floors.
                    P_RemoveMobj (mo);
                    return;
                }
                P_ExplodeMissile (mo);
            }
            else
                mo->momx = mo->momy = 0;
        }
    } while (xmove || ymove);
    
    // slow down
    if (player && player->cheats & CF_NOMOMENTUM)
    {
        // debug option for no sliding at all
        mo->momx = mo->momy = 0;
        return;
    }

    // villsa [STRIFE] replace skullfly flag with MF_BOUNCE
    if (mo->flags & (MF_MISSILE | MF_BOUNCE) )
        return;     // no friction for missiles ever

    // haleyjd 20110224: [STRIFE] players experience friction even in the air,
    // although less than when on the ground. With this fix, the 1.2-and-up 
    // IWAD demo is now in sync!
    if (mo->z > mo->floorz)
    {
        if(player)
        {
            mo->momx = FixedMul (mo->momx, AIRFRICTION);
            mo->momy = FixedMul (mo->momy, AIRFRICTION);
        }
        return;     // no friction when airborne
    }

    if (mo->flags & MF_CORPSE)
    {
        // do not stop sliding
        //  if halfway off a step with some momentum
        if (mo->momx > FRACUNIT/4
            || mo->momx < -FRACUNIT/4
            || mo->momy > FRACUNIT/4
            || mo->momy < -FRACUNIT/4)
        {
            if (mo->floorz != mo->subsector->sector->floorheight)
                return;
        }
    }

    if (mo->momx > -STOPSPEED
        && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED
        && mo->momy < STOPSPEED
        && (!player
            || (player->cmd.forwardmove == 0
                && player->cmd.sidemove == 0 ) ) )
    {
        // if in a walking frame, stop moving
        // villsa [STRIFE]: different player state (haleyjd - verified 20110202)
        if ( player&&(unsigned)((player->mo->state - states) - S_PLAY_01) < 4)
            P_SetMobjState (player->mo, S_PLAY_00);

        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        mo->momx = FixedMul (mo->momx, FRICTION);
        mo->momy = FixedMul (mo->momy, FRICTION);
    }
}

//
// P_ZMovement
//
// [STRIFE] Modifications for:
// * 3D Object Clipping
// * Different momz handling
// * No SKULLFLY logic (replaced with BOUNCE)
// * Missiles don't hit sky flats
//
void P_ZMovement (mobj_t* mo)
{
    fixed_t	dist;
    fixed_t	delta;

    // check for smooth step up
    if (mo->player && mo->z < mo->floorz)
    {
        mo->player->viewheight -= mo->floorz-mo->z;

        mo->player->deltaviewheight
            = (VIEWHEIGHT - mo->player->viewheight)>>3;
    }
    
    // adjust height
    // villsa [STRIFE] check for things standing on top of other things
    if(!P_CheckPositionZ(mo, mo->z + mo->momz))
    {
        if(mo->momz >= 0)
            mo->ceilingz = mo->height + mo->z;
        else
            mo->floorz = mo->z;
    }

    //mo->z += mo->momz; // villsa [STRIFE] unused

    if ( mo->flags & MF_FLOAT
        && mo->target)
    {
        // float down towards target if too close
        if ( /*!(mo->flags & MF_SKULLFLY)   // villsa [STRIFE] unused
             &&*/ !(mo->flags & MF_INFLOAT) )
        {
            dist = P_AproxDistance (mo->x - mo->target->x,
                                    mo->y - mo->target->y);

            delta =(mo->target->z + (mo->height>>1)) - mo->z;

            if (delta<0 && dist < -(delta*3) )
                mo->z -= FLOATSPEED;
            else if (delta>0 && dist < (delta*3) )
                mo->z += FLOATSPEED;
        }
    }
    
    // clip movement
    if (mo->z <= mo->floorz)
    {
        // hit the floor

        if (mo->flags & MF_BOUNCE)
        {
            // the skull slammed into something
            // villsa [STRIFE] affect reactiontime
            // momz is also shifted by 1
            mo->momz = -mo->momz >> 1;
            mo->reactiontime >>= 1;

            // villsa [STRIFE] get terrain type
            if(P_GetTerrainType(mo) != FLOOR_SOLID)
                mo->flags &= ~MF_BOUNCE;
        }

        if (mo->momz < 0)
        {
            if (mo->player
                && mo->momz < -GRAVITY*8)
            {
                // Squat down.
                // Decrease viewheight for a moment
                // after hitting the ground (hard),
                // and utter appropriate sound.
                mo->player->deltaviewheight = mo->momz>>3;

                // villsa [STRIFE] fall damage
                // haleyjd 09/18/10: Repaired calculation
                if(mo->momz < -20*FRACUNIT)
                    P_DamageMobj(mo, NULL, mo, mo->momz / -25000);

                // haleyjd 20110224: *Any* fall centers your view, not just
                // damaging falls (moved outside the above if).
                mo->player->centerview = 1;
                S_StartSound (mo, sfx_oof);
            }
            mo->momz = 0;
        }
        mo->z = mo->floorz;


        // cph 2001/05/26 -
        // See lost soul bouncing comment above. We need this here for bug
        // compatibility with original Doom2 v1.9 - if a soul is charging and
        // hit by a raising floor this incorrectly reverses its Y momentum.
        //

        // villsa [STRIFE] unused
        /*
        if (!correct_lost_soul_bounce && mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;
        */

        // villsa [STRIFE] also check for MF_BOUNCE
        if ( (mo->flags & MF_MISSILE)
            && !(mo->flags & (MF_NOCLIP|MF_BOUNCE)) )
        {
            P_ExplodeMissile (mo);
        }
    }
    else // haleyjd 20110224: else here, not else if - Strife change or what?
    {
        if (! (mo->flags & MF_NOGRAVITY) )
        {
            if (mo->momz == 0)
                mo->momz = -GRAVITY*2;
            else
                mo->momz -= GRAVITY;
        }

        if (mo->z + mo->height > mo->ceilingz)
        {
            // villsa [STRIFE] replace skullfly flag with MF_BOUNCE
            if (mo->flags & MF_BOUNCE)
            {
                // villsa [STRIFE] affect reactiontime
                // momz is also shifted by 1
                mo->momz = -mo->momz >> 1;
                mo->reactiontime >>= 1;
            }

            // hit the ceiling
            if (mo->momz > 0)
                mo->momz = 0;

            mo->z = mo->ceilingz - mo->height;

            // villsa [STRIFE] also check for MF_BOUNCE
            if ( (mo->flags & MF_MISSILE)
                && !(mo->flags & (MF_NOCLIP|MF_BOUNCE)) )
            {
                // villsa [STRIFE] check against skies
                if(mo->subsector->sector->ceilingpic == skyflatnum)
                    P_RemoveMobj(mo);
                else
                    P_ExplodeMissile (mo);
            }
        }
    }
} 



//
// P_NightmareRespawn
//
// [STRIFE] Modifications for:
// * Destination fog z coordinate
// * Restoration of all Strife mapthing flags
//
void
P_NightmareRespawn (mobj_t* mobj)
{
    fixed_t      x;
    fixed_t      y;
    fixed_t      z; 
    mobj_t*      mo;
    mapthing_t*  mthing;

    x = mobj->spawnpoint.x << FRACBITS; 
    y = mobj->spawnpoint.y << FRACBITS; 

    // somthing is occupying it's position?
    if (!P_CheckPosition (mobj, x, y) ) 
        return;	// no respwan

    // spawn a teleport fog at old spot
    // because of removal of the body?
    mo = P_SpawnMobj (mobj->x,
                      mobj->y,
                      mobj->subsector->sector->floorheight , MT_TFOG); 
    // initiate teleport sound
    S_StartSound (mo, sfx_telept);

    // spawn a teleport fog at the new spot
    //ss = R_PointInSubsector (x,y); 

    // haleyjd [STRIFE]: Uses ONFLOORZ instead of ss->sector->floorheight
    mo = P_SpawnMobj (x, y, ONFLOORZ , MT_TFOG); 

    S_StartSound (mo, sfx_telept);

    // spawn the new monster
    mthing = &mobj->spawnpoint;

    // spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    // inherit attributes from deceased one
    mo = P_SpawnMobj (x,y,z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle/45);

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;
    if (mthing->options & MTF_STAND)       // [STRIFE] Standing mode, for NPCs
        mobj->flags |= MF_STAND;
    if (mthing->options & MTF_FRIEND)      // [STRIFE] Allies
        mobj->flags |= MF_ALLY;
    if (mthing->options & MTF_TRANSLUCENT) // [STRIFE] Translucent object
        mobj->flags |= MF_SHADOW;
    if (mthing->options & MTF_MVIS)        // [STRIFE] Alt. Translucency
        mobj->flags |= MF_MVIS;

    mo->reactiontime = 18;

    // remove the old monster,
    P_RemoveMobj (mobj);
}


//
// P_MobjThinker
//
// [STRIFE] Modified for:
// * Terrain effects
// * Stonecold cheat
// * Altered skill 5 respawn behavior
//
void P_MobjThinker (mobj_t* mobj)
{
    // momentum movement
    if (mobj->momx
        || mobj->momy
        /*|| (mobj->flags&MF_SKULLFLY)*/ )  // villsa [STRIFE] unused
    {
        P_XYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;     // mobj was removed

        // villsa [STRIFE] terrain clipping
        if(P_GetTerrainType(mobj) == FLOOR_SOLID)
            mobj->flags &= ~MF_FEETCLIPPED;
        else
            mobj->flags |= MF_FEETCLIPPED;

    }
    if ( (mobj->z != mobj->floorz && !(mobj->flags & MF_NOGRAVITY)) // villsa [STRIFE]
        || mobj->momz )
    {
        P_ZMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;     // mobj was removed

        // villsa [STRIFE] terrain clipping and sounds
        if(P_GetTerrainType(mobj) == FLOOR_SOLID)
            mobj->flags &= ~MF_FEETCLIPPED;
        else
        {
            S_StartSound(mobj, sfx_wsplsh);
            mobj->flags |= MF_FEETCLIPPED;
        }

    }

    
    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // villsa [STRIFE] stonecold cheat
        if(stonecold)
        {
            if(mobj->flags & MF_COUNTKILL)
                P_DamageMobj(mobj, mobj, mobj, 10);
        }

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;     // freed itself
    }
    else
    {
        // check for nightmare respawn
        if (! (mobj->flags & MF_COUNTKILL) )
            return;

        if (!respawnmonsters)
            return;

        mobj->movecount++;

        // haleyjd [STRIFE]: respawn time increased from 12 to 16
        if (mobj->movecount < 16*TICRATE)
            return;

        if ( leveltime&31 )
            return;

        if (P_Random () > 4)
            return;

        // haleyjd [STRIFE]: NOTDMATCH things don't respawn
        if(mobj->flags & MF_NOTDMATCH)
            return;

        P_NightmareRespawn (mobj);
    }
}


//
// P_SpawnMobj
//
// [STRIFE] Modifications to reactiontime and for terrain types.
//
mobj_t*
P_SpawnMobj
( fixed_t	x,
  fixed_t	y,
  fixed_t	z,
  mobjtype_t	type )
{
    mobj_t*	mobj;
    state_t*	st;
    mobjinfo_t*	info;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->health = info->spawnhealth;

    // haleyjd 09/25/10: [STRIFE] Doesn't do this; messes up flamethrower
    // and a lot of other stuff using reactiontime as a counter.
    //if (gameskill != sk_nightmare)
    mobj->reactiontime = info->reactiontime;
    
    mobj->lastlook = P_Random () % MAXPLAYERS;
    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // set subsector and/or block links
    P_SetThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if (z == ONFLOORZ)
    {
        mobj->z = mobj->floorz;

        // villsa [STRIFE]
        if(P_GetTerrainType(mobj) != FLOOR_SOLID)
            mobj->flags |= MF_FEETCLIPPED;

    }
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->info->height;
    else 
        mobj->z = z;

    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;

    P_AddThinker (&mobj->thinker);

    return mobj;
}


//
// P_RemoveMobj
//
// [STRIFE] Modifications for item respawn timing
//
mapthing_t  itemrespawnque[ITEMQUESIZE];
int         itemrespawntime[ITEMQUESIZE];
int         iquehead;
int         iquetail;

void P_RemoveMobj (mobj_t* mobj)
{
    // villsa [STRIFE] removed invuln/invis. sphere exceptions
    if ((mobj->flags & MF_SPECIAL)
        && !(mobj->flags & MF_DROPPED))
    {
        itemrespawnque[iquehead] = mobj->spawnpoint;
        itemrespawntime[iquehead] = leveltime + 30*TICRATE; // [STRIFE]

        // [STRIFE] haleyjd 20130915
        // -random parameter affects the behavior of respawning items here.
        if(randomparm && iquehead != iquetail)
        {
            short type    = itemrespawnque[iquehead].type;
            short options = itemrespawnque[iquehead].options;

            // swap the type and options of iquehead and iquetail
            itemrespawnque[iquehead].type    = itemrespawnque[iquetail].type;
            itemrespawnque[iquehead].options = itemrespawnque[iquetail].options;
            itemrespawnque[iquetail].type    = type;
            itemrespawnque[iquetail].options = options;
        }

        iquehead = (iquehead+1)&(ITEMQUESIZE-1);

        // lose one off the end?
        if (iquehead == iquetail)
            iquetail = (iquetail+1)&(ITEMQUESIZE-1);
    }

    // unlink from sector and block lists
    P_UnsetThingPosition (mobj);
    
    // stop any playing sound
    S_StopSound (mobj);
    
    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}




//
// P_RespawnSpecials
//
// [STRIFE] modification to item respawn time handling
//
void P_RespawnSpecials (void)
{
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    
    subsector_t*    ss; 
    mobj_t*         mo;
    mapthing_t*     mthing;
    
    int         i;

    // only respawn items in deathmatch
    if (deathmatch != 2)
        return;

    // nothing left to respawn?
    if (iquehead == iquetail)
        return;

    // haleyjd [STRIFE]: 30 second wait is not accounted for here, see above.
    if (leveltime < itemrespawntime[iquetail])
        return;

    mthing = &itemrespawnque[iquetail];

    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y); 
    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG); 
    S_StartSound (mo, sfx_itmbk);

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
    {
        if (mthing->type == mobjinfo[i].doomednum)
            break;
    }

    if (i >= NUMMOBJTYPES)
    {
        I_Error("P_RespawnSpecials: Failed to find mobj type with doomednum "
                "%d when respawning thing. This would cause a buffer overrun "
                "in vanilla Strife.", mthing->type);
    }

    // spawn it
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    mo = P_SpawnMobj (x,y,z, i);
    mo->spawnpoint = *mthing;
    mo->angle = ANG45 * (mthing->angle/45);

    // pull it from the que
    iquetail = (iquetail+1)&(ITEMQUESIZE-1);
}




//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// [STRIFE] Modifications for:
// * stonecold cheat, -workparm
// * default inventory/questflags
//
void P_SpawnPlayer(mapthing_t* mthing)
{
    player_t*   p;
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    mobj_t*     mobj;

    if(mthing->type == 0)
        return;

    // not playing?
    if(!playeringame[mthing->type-1])
        return;

    p = &players[mthing->type-1];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn (mthing->type-1);

    x       = mthing->x << FRACBITS;
    y       = mthing->y << FRACBITS;
    z       = ONFLOORZ;
    mobj    = P_SpawnMobj (x,y,z, MT_PLAYER);

    // set color translations for player sprites
    if(mthing->type > 1)
        mobj->flags |= (mthing->type-1)<<MF_TRANSSHIFT;

    mobj->angle  = ANG45 * (mthing->angle/45);
    mobj->player = p;
    mobj->health = p->health;

    p->mo               = mobj;
    p->playerstate      = PST_LIVE;	
    p->refire           = 0;
    p->message          = NULL;
    p->damagecount      = 0;
    p->bonuscount       = 0;
    p->extralight       = 0;
    p->fixedcolormap    = 0;
    p->viewheight       = VIEWHEIGHT;

    // setup gun psprite
    P_SetupPsprites(p);

    // villsa [STRIFE]
    stonecold = false;

    // villsa [STRIFE] what a nasty hack...
    if(gamemap == 10)
        p->weaponowned[wp_sigil] = true;

    // villsa [STRIFE] instead of just giving cards in deathmatch mode, also
    // set accuracy to 50 and give all quest flags
    if(deathmatch)
    {
        int i;

        p->accuracy = 50;
        p->questflags = QF_ALLQUESTS; // 0x7fffffff

        for(i = 0; i < NUMCARDS; i++)
            p->cards[i] = true;
    }

    // villsa [STRIFE] set godmode?
    if(workparm)
        p->cheats |= CF_GODMODE;

    if(mthing->type - 1 == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();
    }
}


//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//
// [STRIFE] Modifications for:
// * No Lost Souls, item count
// * New mapthing_t flag bits
// * 8-player support
//
void P_SpawnMapThing (mapthing_t* mthing)
{
    int         i;
    int         bit;
    mobj_t*     mobj;
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;

    // count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[10])
        {
            memcpy (deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p++;
        }
        return;
    }

    if (mthing->type <= 0)
    {
        // Thing type 0 is actually "player -1 start".  
        // For some reason, Vanilla Doom accepts/ignores this.

        return;
    }

    // check for players specially
    // haleyjd 20120209: [STRIFE] 8 player starts
    if (mthing->type <= 8)
    {
        // save spots for respawning in network games
        playerstarts[mthing->type-1] = *mthing;
        if (!deathmatch)
            P_SpawnPlayer (mthing);

        return;
    }

    // check for apropriate skill level
    if (!netgame && (mthing->options & 16) )
        return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1<<(gameskill-1);

    if (!(mthing->options & bit) )
        return;

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i==NUMMOBJTYPES)
        I_Error ("P_SpawnMapThing: Unknown type %i at (%i, %i)",
                 mthing->type,
                 mthing->x, mthing->y);

    // don't spawn keycards and players in deathmatch
    if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
        return;

    // don't spawn any monsters if -nomonsters
    // villsa [STRIFE] Removed MT_SKULL
    if (nomonsters && (mobjinfo[i].flags & MF_COUNTKILL))
        return;
    
    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;
    
    mobj = P_SpawnMobj (x,y,z, i);
    mobj->spawnpoint = *mthing;

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random () % mobj->tics);
    if (mobj->flags & MF_COUNTKILL)
        totalkills++;

    // villsa [STRIFE] unused
    /*
    if (mobj->flags & MF_COUNTITEM)
        totalitems++;
    */

    mobj->angle = ANG45 * (mthing->angle/45);
    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;
    if (mthing->options & MTF_STAND)       // [STRIFE] Standing mode, for NPCs
        mobj->flags |= MF_STAND;
    if (mthing->options & MTF_FRIEND)      // [STRIFE] Allies
        mobj->flags |= MF_ALLY;
    if (mthing->options & MTF_TRANSLUCENT) // [STRIFE] Translucent object
        mobj->flags |= MF_SHADOW;
    if (mthing->options & MTF_MVIS)        // [STRIFE] Alt. Translucency
        mobj->flags |= MF_MVIS;
}



//
// GAME SPAWN FUNCTIONS
//


//
// P_SpawnPuff
//
// [STRIFE] Modifications for:
// * No spawn tics randomization
// * Player melee behavior
//
extern fixed_t attackrange;

void
P_SpawnPuff
( fixed_t	x,
  fixed_t	y,
  fixed_t	z )
{
    mobj_t*	th;
    int t;
    
    t = P_Random();
    z += ((t - P_Random()) << 10);

    // [STRIFE] removed momz and tics randomization

    th = P_SpawnMobj(x, y, z, MT_STRIFEPUFF); // [STRIFE]: new type

    // don't make punches spark on the wall
    // [STRIFE] Use a separate melee attack range for the player
    if(attackrange == PLAYERMELEERANGE)
        P_SetMobjState(th, S_POW2_00); // 141

    // villsa [STRIFE] unused
    /*
    if (th->tics < 1)
        th->tics = 1;
    */
}

//
// P_SpawnSparkPuff
//
// villsa [STRIFE] new function
//
mobj_t* P_SpawnSparkPuff(fixed_t x, fixed_t y, fixed_t z)
{
    int t = P_Random();
    return P_SpawnMobj(x, y, ((t - P_Random()) << 10) + z, MT_SPARKPUFF);
}

//
// P_SpawnBlood
// 
// [STRIFE] Modifications for:
// * No spawn tics randomization
// * Different damage ranges for state setting
//
void
P_SpawnBlood
( fixed_t       x,
  fixed_t       y,
  fixed_t       z,
  int           damage )
{
    mobj_t*     th;
    int temp;
    
    temp = P_Random();
    z += (temp - P_Random()) << 10;
    th = P_SpawnMobj(x, y, z, MT_BLOOD_DEATH);
    th->momz = FRACUNIT*2;
    
    // villsa [STRIFE]: removed tics randomization

    // villsa [STRIFE] different checks for damage range
    if(damage >= 10 && damage <= 13)
        P_SetMobjState(th, S_BLOD_00);
    else if(damage >= 7 && damage < 10)
        P_SetMobjState(th, S_BLOD_01);
    else if(damage < 7)
        P_SetMobjState(th, S_BLOD_02);
}



//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
// [STRIFE] Modifications for:
// * No spawn tics randomization
//
void P_CheckMissileSpawn (mobj_t* th)
{
    // villsa [STRIFE] removed tics randomization
    
    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y))
        P_ExplodeMissile (th);
}

// Certain functions assume that a mobj_t pointer is non-NULL,
// causing a crash in some situations where it is NULL.  Vanilla
// Doom did not crash because of the lack of proper memory 
// protection. This function substitutes NULL pointers for
// pointers to a dummy mobj, to avoid a crash.

mobj_t *P_SubstNullMobj(mobj_t *mobj)
{
    if (mobj == NULL)
    {
        static mobj_t dummy_mobj;

        dummy_mobj.x = 0;
        dummy_mobj.y = 0;
        dummy_mobj.z = 0;
        dummy_mobj.flags = 0;

        mobj = &dummy_mobj;
    }

    return mobj;
}

//
// P_SpawnMissile
//
// [STRIFE] Added MVIS inaccuracy
//
mobj_t*
P_SpawnMissile
( mobj_t*       source,
  mobj_t*       dest,
  mobjtype_t    type )
{
    mobj_t*     th;
    angle_t     an;
    int         dist;

    th = P_SpawnMobj (source->x,
                      source->y,
                      source->z + 4*8*FRACUNIT, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;	// where it came from
    an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

    // fuzzy player
    if (dest->flags & MF_SHADOW)
    {
        int t = P_Random(); // haleyjd 20110223: remove order-of-evaluation dependencies
        an += (t - P_Random()) << 21;
    }
    // villsa [STRIFE] check for heavily transparent things
    else if(dest->flags & MF_MVIS)
    {
        int t = P_Random();
        an += (t - P_Random()) << 22;
    }

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;

    if (dist < 1)
        dist = 1;

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn (th);

    return th;
}

//
// P_SpawnFacingMissile
//
// villsa [STRIFE] new function
// Spawn a missile based on source's angle
//
mobj_t* P_SpawnFacingMissile(mobj_t* source, mobj_t* target, mobjtype_t type)
{
    mobj_t* th;
    angle_t an;
    fixed_t dist;

    th = P_SpawnMobj(source->x, source->y, source->z + (32*FRACUNIT), type);

    if(th->info->seesound)
        S_StartSound(th, th->info->seesound);

    th->target = source;    // where it came from
    th->angle = source->angle; // haleyjd 09/06/10: fix0red
    an = th->angle;

    // fuzzy player
    if (target->flags & MF_SHADOW)
    {
        int t = P_Random();
        an += (t - P_Random()) << 21;
    }
    // villsa [STRIFE] check for heavily transparent things
    else if(target->flags & MF_MVIS)
    {
        int t = P_Random();
        an += (t - P_Random()) << 22;
    }

    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (target->x - source->x, target->y - source->y);
    dist = dist / th->info->speed;

    if(dist < 1)
        dist = 1;

    th->momz = (target->z - source->z) / dist;
    P_CheckMissileSpawn (th);

    return th;
}

//
// P_SpawnPlayerMissile
//
// Tries to aim at a nearby monster
// villsa [STRIFE] now returns a mobj
// * Also modified to allow up/down look, and to account for foot-clipping
//   by liquid terrain.
//
mobj_t* P_SpawnPlayerMissile(mobj_t* source, mobjtype_t type)
{
    mobj_t*	th;
    angle_t	an;
    
    fixed_t	x;
    fixed_t	y;
    fixed_t	z;
    fixed_t	slope;
    
    // see which target is to be aimed at
    an = source->angle;
    slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
    
    if (!linetarget)
    {
        an += 1<<26;
        slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

        if (!linetarget)
        {
            an -= 2<<26;
            slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
        }

        if (!linetarget)
        {
            an = source->angle;

            // haleyjd 09/21/10: [STRIFE] Removed, for look up/down support.
            //slope = 0; 
        }
    }

    // villsa [STRIFE]
    if(linetarget)
        source->target = linetarget;

    x = source->x;
    y = source->y;
    
    // villsa [STRIFE]
    if(!(source->flags & MF_FEETCLIPPED))
        z = source->z + 32*FRACUNIT;
    else
        z = source->z + 22*FRACUNIT;

    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;
    th->angle = an;
    th->momx = FixedMul( th->info->speed,
                         finecosine[an>>ANGLETOFINESHIFT]);
    th->momy = FixedMul( th->info->speed,
                         finesine[an>>ANGLETOFINESHIFT]);
    th->momz = FixedMul( th->info->speed, slope);

    P_CheckMissileSpawn (th);

    return th;
}

//
// P_SpawnMortar
//
// villsa [STRIFE] new function
// Spawn a high-arcing ballistic projectile
//
mobj_t* P_SpawnMortar(mobj_t *source, mobjtype_t type)
{
    mobj_t* th;
    angle_t an;
    fixed_t slope;

    an = source->angle;

    th = P_SpawnMobj(source->x, source->y, source->z, type);
    th->target = source;
    th->angle = an;
    an >>= ANGLETOFINESHIFT;

    // haleyjd 20110203: corrected order of function calls
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);

    P_CheckMissileSpawn(th);
    
    slope = P_AimLineAttack(source, source->angle, 1024*FRACUNIT);
    th->momz = FixedMul(th->info->speed, slope);

    return th;
}
