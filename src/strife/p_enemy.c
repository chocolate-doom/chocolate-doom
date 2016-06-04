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
//	Enemy thinking, AI.
//	Action Pointer Functions
//	that are associated with states/frames. 
//

#include <stdio.h>
#include <stdlib.h>

#include "m_random.h"
#include "i_system.h"
#include "doomdef.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "g_game.h"
#include "z_zone.h"     // villsa [STRIFE]

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "sounds.h"

// [STRIFE] Dialog / Inventory
#include "p_dialog.h"
#include "deh_str.h"
#include "w_wad.h"
#include "f_finale.h"
#include "p_inter.h"

// Forward Declarations:
void A_RandomWalk(mobj_t *);
void A_ProgrammerAttack(mobj_t* actor);
void A_FireSigilEOffshoot(mobj_t *actor);
void A_SpectreCAttack(mobj_t *actor);
void A_SpectreDAttack(mobj_t *actor);
void A_SpectreEAttack(mobj_t *actor);

void P_ThrustMobj(mobj_t *actor, angle_t angle, fixed_t force);

typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
    
} dirtype_t;


//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};





void A_Fall (mobj_t *actor);


//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//
// haleyjd 09/05/10: [STRIFE] Verified unmodified
//

mobj_t*		soundtarget;

void
P_RecursiveSound
( sector_t*	sec,
  int		soundblocks )
{
    int		i;
    line_t*	check;
    sector_t*	other;
	
    // wake up all monsters in this sector
    if (sec->validcount == validcount
	&& sec->soundtraversed <= soundblocks+1)
    {
	return;		// already flooded
    }
    
    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;
	
    for (i=0 ;i<sec->linecount ; i++)
    {
	check = sec->lines[i];
	if (! (check->flags & ML_TWOSIDED) )
	    continue;
	
	P_LineOpening (check);

	if (openrange <= 0)
	    continue;	// closed door
	
	if ( sides[ check->sidenum[0] ].sector == sec)
	    other = sides[ check->sidenum[1] ] .sector;
	else
	    other = sides[ check->sidenum[0] ].sector;
	
	if (check->flags & ML_SOUNDBLOCK)
	{
	    if (!soundblocks)
		P_RecursiveSound (other, 1);
	}
	else
	    P_RecursiveSound (other, soundblocks);
    }
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
// haleyjd 09/05/10: [STRIFE] Verified unmodified
//
void
P_NoiseAlert
( mobj_t*	target,
  mobj_t*	emmiter )
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}

//
// P_WakeUpThing
//
// villsa [STRIFE] New function
// Wakes up an mobj.nearby when somebody has been punched.
//
static void P_WakeUpThing(mobj_t* puncher, mobj_t* bystander)
{
    if(!(bystander->flags & MF_NODIALOG))
    {
        bystander->target = puncher;
        if(bystander->info->seesound)
            S_StartSound(bystander, bystander->info->seesound);
        P_SetMobjState(bystander, bystander->info->seestate);
    }
}

//
// P_DoPunchAlert
//
// villsa [STRIFE] New function (by Quasar ;)
// Wake up buddies nearby when the player thinks he's gotten too clever
// with the punch dagger. Walks sector links.
//
void P_DoPunchAlert(mobj_t *puncher, mobj_t *punchee)
{   
   mobj_t *rover;
   
   // don't bother with this crap if we're already on alert
   if(punchee->subsector->sector->soundtarget)
      return;
      
   // gotta still be alive to call for help
   if(punchee->health <= 0)
      return;
      
   // has to be something you can wake up and kill too
   if(!(punchee->flags & MF_COUNTKILL) || punchee->flags & MF_NODIALOG)
      return;
   
   // make the punchee hurt - haleyjd 09/05/10: Fixed to use painstate.
   punchee->target = puncher;
   P_SetMobjState(punchee, punchee->info->painstate); 
   
   // wake up everybody nearby
   
   // scan forward on sector list
   for(rover = punchee->snext; rover; rover = rover->snext)
   {
      // we only wake up certain thing types (Acolytes and Templars?)
      if(rover->health > 0 && rover->type >= MT_GUARD1 && rover->type <= MT_PGUARD &&
         (P_CheckSight(rover, puncher) || P_CheckSight(rover, punchee)))
      {
         P_WakeUpThing(puncher, rover);
         rover->flags |= MF_NODIALOG;
      }
   }

   // scan backward on sector list
   for(rover = punchee->sprev; rover; rover = rover->sprev)
   {
      // we only wake up certain thing types (Acolytes and Templars?)
      if(rover->health > 0 && rover->type >= MT_GUARD1 && rover->type <= MT_PGUARD &&
         (P_CheckSight(rover, puncher) || P_CheckSight(rover, punchee)))
      {
         P_WakeUpThing(puncher, rover);
         rover->flags |= MF_NODIALOG;
      }
   }
}




//
// P_CheckMeleeRange
//
// [STRIFE] Minor change to meleerange.
//
boolean P_CheckMeleeRange(mobj_t* actor)
{
    mobj_t*	pl;
    fixed_t	dist;

    if(!actor->target)
        return false;

    pl = actor->target;
    if(actor->z + 3 * actor->height / 2 < pl->z) // villsa [STRIFE]
        return false;

    dist = P_AproxDistance(pl->x - actor->x, pl->y - actor->y);

    if(dist >= MELEERANGE - 20*FRACUNIT + pl->info->radius)
        return false;

    if(!P_CheckSight (actor, actor->target))
        return false;

    return true;
}

//
// P_CheckMissileRange
//
// [STRIFE]
// Changes to eliminate DOOM-specific code and to allow for
// varying attack ranges for Strife monsters, as well as a general tweak
// to considered distance for all monsters.
//
boolean P_CheckMissileRange(mobj_t* actor)
{
    fixed_t dist;

    if(!P_CheckSight(actor, actor->target))
        return false;

    if(actor->flags & MF_JUSTHIT)
    {
        // the target just hit the enemy,
        // so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }

    if(actor->reactiontime)
        return false;	// do not attack yet

    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance(actor->x-actor->target->x,
                           actor->y-actor->target->y) - 64*FRACUNIT;
    
    if (!actor->info->meleestate)
        dist -= 128*FRACUNIT;       // no melee attack, so fire more

    dist >>= 16;

    // villsa [STRIFE] checks for acolytes
    //  haleyjd 09/05/10: Repaired to match disassembly: Was including 
    //  SHADOWGUARD in the wrong case, was missing MT_SENTINEL entirely.
    //  Structure of ASM also indicates this was probably a switch 
    //  statement turned into a cascading if/else by the compiler.
    switch(actor->type)
    {
    case MT_GUARD1:
    case MT_GUARD2:
    case MT_GUARD3:
    case MT_GUARD4:
    case MT_GUARD5:
    case MT_GUARD6:
        // oddly, not all Acolytes are included here...
        dist >>= 4;
        break;
    case MT_SHADOWGUARD:
    case MT_CRUSADER:
    case MT_SENTINEL:
        dist >>= 1;
        break;
    default:
        break;
    }
    
    // villsa [STRIFE] changed to 150
    if (dist > 150)
        dist = 150;

    // haleyjd 20100910: Hex-Rays was leaving this out completely:
    if (actor->type == MT_CRUSADER && dist > 120)
        dist = 120;

    // haleyjd 20110224 [STRIFE]: reversed predicate
    return (dist < P_Random());
}

//
// P_CheckRobotRange
//
// villsa [STRIFE] New function
//
boolean P_CheckRobotRange(mobj_t *actor)
{
    fixed_t dist;

    if(!P_CheckSight(actor, actor->target))
        return false;

    if(actor->reactiontime)
        return false;    // do not attack yet

    dist = (P_AproxDistance(actor->x-actor->target->x,
                            actor->y-actor->target->y) - 64*FRACUNIT) >> FRACBITS;

    return (dist < 200);
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
// [STRIFE]
// villsa/haleyjd 09/05/10: Modified for terrain types and 3D object 
// clipping. Below constants are verified to be unmodified:
//
fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

#define MAXSPECIALCROSS	8

extern	line_t*	spechit[MAXSPECIALCROSS];
extern	int	numspechit;

boolean P_Move (mobj_t*	actor)
{
    fixed_t	tryx;
    fixed_t	tryy;

    line_t*	ld;

    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
    boolean	try_ok;
    boolean	good;

    if (actor->movedir == DI_NODIR)
        return false;

    if ((unsigned)actor->movedir >= 8)
        I_Error ("Weird actor->movedir!");

    tryx = actor->x + actor->info->speed*xspeed[actor->movedir];
    tryy = actor->y + actor->info->speed*yspeed[actor->movedir];

    try_ok = P_TryMove (actor, tryx, tryy);

    if (!try_ok)
    {
        // open any specials
        if (actor->flags & MF_FLOAT && floatok)
        {
            // must adjust height
            if (actor->z < tmfloorz)
                actor->z += FLOATSPEED; // [STRIFE] Note FLOATSPEED == 5*FRACUNIT
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;
        good = false;
        while (numspechit--)
        {
            ld = spechit[numspechit];
            // if the special is not a door
            // that can be opened,
            // return false
            if (P_UseSpecialLine (actor, ld,0))
                good = true;
        }
        return good;
    }
    else
    {
        actor->flags &= ~(MF_INFLOAT|MF_FEETCLIPPED);   // villsa [STRIFE]

        // villsa [STRIFE]
        if(P_GetTerrainType(actor) != FLOOR_SOLID)
            actor->flags |= MF_FEETCLIPPED;
    }


    // villsa [STRIFE] Removed pulling non-floating actors down to the ground.
    //  (haleyjd 09/05/10: Verified)
    /*if (! (actor->flags & MF_FLOAT) )	
          actor->z = actor->floorz;*/

    return true; 
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
// haleyjd 09/05/10: [STRIFE] Verified unmodified.
//
boolean P_TryWalk (mobj_t* actor)
{
    if (!P_Move (actor))
    {
        return false;
    }

    actor->movecount = P_Random()&15;
    return true;
}



//
// P_NewChaseDir
//

void P_NewChaseDir(mobj_t* actor)
{
    fixed_t	deltax;
    fixed_t	deltay;
    
    dirtype_t	d[3];
    
    int		tdir;
    dirtype_t	olddir;
    
    dirtype_t	turnaround;

    // villsa [STRIFE] don't bomb out and instead set spawnstate
    if(!actor->target)
    {
        //I_Error("P_NewChaseDir: called with no target");
        P_SetMobjState(actor, actor->info->spawnstate);
        return;
    }

    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
        d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
        d[1]= DI_WEST;
    else
        d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
        d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
        d[2]= DI_NORTH;
    else
        d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR
        && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
        if (actor->movedir != (int) turnaround && P_TryWalk(actor))
            return;
    }

    // try other directions
    if (P_Random() > 200
        ||  abs(deltay)>abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    if (d[1]==turnaround)
        d[1]=DI_NODIR;
    if (d[2]==turnaround)
        d[2]=DI_NODIR;

    if (d[1]!=DI_NODIR)
    {
        actor->movedir = d[1];
        if (P_TryWalk(actor))
        {
            // either moved forward or attacked
            return;
        }
    }

    if (d[2]!=DI_NODIR)
    {
        actor->movedir =d[2];

        if (P_TryWalk(actor))
            return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
        actor->movedir =olddir;

        if (P_TryWalk(actor))
            return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
        for ( tdir=DI_EAST;
              tdir<=DI_SOUTHEAST;
              tdir++ )
        {
            if (tdir != (int) turnaround)
            {
                actor->movedir =tdir;

                if ( P_TryWalk(actor) )
                    return;
            }
        }
    }
    else
    {
        for ( tdir=DI_SOUTHEAST;
              tdir != (DI_EAST-1);
              tdir-- )
        {
            if (tdir != (int) turnaround)
            {
                actor->movedir = tdir;

                if ( P_TryWalk(actor) )
                    return;
            }
        }
    }

    if (turnaround !=  DI_NODIR)
    {
        actor->movedir =turnaround;
        if ( P_TryWalk(actor) )
            return;
    }

    actor->movedir = DI_NODIR;	// can not move
}

//
// P_NewRandomDir
//
// villsa [STRIFE] new function
//
// haleyjd: Almost identical to the tail-end of P_NewChaseDir, this function 
// finds a purely random direction for an object to walk. Called from 
// A_RandomWalk.
//
// Shockingly similar to the RandomWalk pointer in Eternity :)
//
void P_NewRandomDir(mobj_t* actor)
{
    int dir = 0;
    int omovedir = opposite[actor->movedir]; // haleyjd 20110223: nerfed this...

    // randomly determine direction of search
    if(P_Random() & 1)
    {
        // Try all non-reversal directions forward, first
        for(dir = 0; dir < DI_NODIR; dir++)
        {
            if(dir != omovedir)
            {
                actor->movedir = dir;
                if(P_Random() & 1)
                {
                    if(P_TryWalk(actor))
                        break;
                }
            }
        }

        // haleyjd 20110223: logic missing entirely:
        // failed all non-reversal directions? try reversing
        if(dir > DI_SOUTHEAST) 
        {
            if(omovedir == DI_NODIR)
            {
                actor->movedir = DI_NODIR;
                return;
            }
            actor->movedir = omovedir;
            if(P_TryWalk(actor))
                return;
            else
            {
                actor->movedir = DI_NODIR;
                return;
            }
        }
    }
    else
    {
        // Try directions one at a time in backward order
        dir = DI_SOUTHEAST;
        while(1)
        {
            // haleyjd 09/05/10: missing random code.
            if(dir != omovedir)
            {
                actor->movedir = dir;

                // villsa 09/06/10: un-inlined code
                if(P_TryWalk(actor))
                    return;
            }

            // Ran out of non-reversal directions to try? Reverse.
            if(--dir == -1)
            {
                if(omovedir == DI_NODIR)
                {
                    actor->movedir = DI_NODIR;
                    return;
                }
                actor->movedir = omovedir;
                // villsa 09/06/10: un-inlined code
                if(P_TryWalk(actor))
                    return;
                else
                {
                    actor->movedir = DI_NODIR;
                    return;
                }
            } // end if(--dir == -1)
        } // end while(1)
    } // end else
}

// haleyjd 09/05/10: Needed below.
extern void P_BulletSlope (mobj_t *mo);

//
// P_LookForPlayers
//
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
// [STRIFE]
// haleyjd 09/05/10: Modifications to support friendly units.
//
boolean
P_LookForPlayers
( mobj_t*	actor,
  boolean	allaround )
{
    int         c;
    int         stop;
    player_t*   player;
    angle_t     an;
    fixed_t     dist;
    mobj_t  *   master = players[actor->miscdata].mo;

    // haleyjd 09/05/10: handle Allies
    if(actor->flags & MF_ALLY)
    {
        // Deathmatch: support team behavior for Rebels.
        if(netgame)
        {
            // Rebels adopt the allied player's target if it is not of the same
            // allegiance. Other allies do it unconditionally.
            if(master && master->target && 
               (master->target->type != MT_REBEL1 ||
                master->target->miscdata != actor->miscdata))
            {
                actor->target = master->target;
            }
            else
            {
                // haleyjd 09/06/10: Note that this sets actor->target in Strife!
                P_BulletSlope(actor);

                // Clear target if nothing is visible, or if the target is a
                // friendly Rebel or the allied player.
                if (linetarget == NULL
                 || (actor->target->type == MT_REBEL1
                  && actor->target->miscdata == actor->miscdata)
                 || actor->target == master)
                {
                    actor->target = NULL;
                    return false;
                }
            }
        }
        else
        {
            // Single-player: Adopt any non-allied player target.
            if(master && master->target && !(master->target->flags & MF_ALLY))
            {
                actor->target = master->target;
                return true;
            }

            // haleyjd 09/06/10: Note that this sets actor->target in Strife!
            P_BulletSlope(actor);

            // Clear target if nothing is visible, or if the target is an ally.
            if(!linetarget || actor->target->flags & MF_ALLY)
            {
                actor->target = NULL;
                return false;
            }
        }

        return true;
    }

    c = 0;

    // NOTE: This behavior has been changed from the Vanilla behavior, where
    // an infinite loop can occur if players 0-3 all quit the game. Although
    // technically this is not what Vanilla does, fixing this is highly
    // desirable, and having the game simply lock up is not acceptable.
    // stop = (actor->lastlook - 1) & 3;
    // for (;; actor->lastlook = (actor->lastlook + 1) & 3)

    stop = (actor->lastlook + MAXPLAYERS - 1) % MAXPLAYERS;

    for ( ; ; actor->lastlook = (actor->lastlook + 1) % MAXPLAYERS)
    {
        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2
            || actor->lastlook == stop)
        {
            // done looking
            return false;	
        }

        player = &players[actor->lastlook];

        if (player->health <= 0)
            continue;           // dead

        if (!P_CheckSight (actor, player->mo))
            continue;           // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2(actor->x,
                                 actor->y, 
                                 player->mo->x,
                                 player->mo->y) - actor->angle;

            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance (player->mo->x - actor->x,
                    player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > MELEERANGE)
                    continue;       // behind back
            }
        }

        actor->target = player->mo;
        return true;
    }

    return false;
}

// haleyjd 09/05/10: [STRIFE] Removed A_KeenDie

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
// [STRIFE]
// haleyjd 09/05/10: Adjusted for allies, Inquisitors, etc.
//
void A_Look (mobj_t* actor)
{
    mobj_t*         targ;

    actor->threshold = 0;       // any shot will wake up
    targ = actor->subsector->sector->soundtarget;

    if (targ
        && (targ->flags & MF_SHOOTABLE) )
    {
        // [STRIFE] Allies wander when they call this.
        if(actor->flags & MF_ALLY)
            A_RandomWalk(actor);
        else
        {
            actor->target = targ;

            if ( actor->flags & MF_AMBUSH )
            {
                if (P_CheckSight (actor, actor->target))
                    goto seeyou;
            }
            else
                goto seeyou;
        }
    }

    // haleyjd 09/05/10: This is bizarre, as Rogue keeps using the GIVEQUEST flag
    // as a parameter to control allaround look behavior. Did they just run out of
    // flags, or what? 
    // STRIFE-TODO: Needs serious verification.
    if (!P_LookForPlayers(actor, (actor->flags & MF_GIVEQUEST) != 0))
        return;

    // go into chase state
seeyou:
    if (actor->info->seesound)
    {
        int         sound   = actor->info->seesound;
        mobj_t *    emitter = actor;

        // [STRIFE] Removed DOOM random sounds.

        // [STRIFE] Only Inquisitors roar loudly here.
        if (actor->type == MT_INQUISITOR)
            emitter = NULL;

        S_StartSound (emitter, sound);
    }

    // [STRIFE] Set threshold (kinda odd as it's still set to 0 above...)
    actor->threshold = 20;

    P_SetMobjState (actor, actor->info->seestate);
}

//
// A_RandomWalk
//
// [STRIFE] New function.
// haleyjd 09/05/10: Action routine used to meander about.
//
void A_RandomWalk(mobj_t* actor)
{
    // Standing actors do not wander.
    if(actor->flags & MF_STAND)
        return;

    if(actor->reactiontime)
        actor->reactiontime--; // count down reaction time
    else
    {
        // turn to a new angle
        if(actor->movedir < DI_NODIR)
        {
            int delta;

            actor->angle &= (7 << 29);
            delta = actor->angle - (actor->movedir << 29);

            if(delta < 0)
                actor->angle += ANG90/2;
            else if(delta > 0)
                actor->angle -= ANG90/2;
        }

        // try moving
        if(--actor->movecount < 0 || !P_Move(actor))
        {
            P_NewRandomDir(actor);
            actor->movecount += 5;
        }
    }
}

//
// A_FriendLook
//
// [STRIFE] New function
// haleyjd 09/05/10: Action function used mostly by mundane characters such as
// peasants.
//
void A_FriendLook(mobj_t* actor)
{
    mobj_t *soundtarget = actor->subsector->sector->soundtarget;

    actor->threshold = 0;

    if(soundtarget && soundtarget->flags & MF_SHOOTABLE)
    {
        // Handle allies, except on maps 3 and 34 (Front Base/Movement Base)
        if((actor->flags & MF_ALLY) == (soundtarget->flags & MF_ALLY) &&
            gamemap != 3 && gamemap != 34)
        {
            // STRIFE-TODO: Needs serious verification.
            if(P_LookForPlayers(actor, (actor->flags & MF_GIVEQUEST) != 0))
            {
                P_SetMobjState(actor, actor->info->seestate);
                actor->flags |= MF_NODIALOG;
                return;
            }
        }
        else
        {
            actor->target = soundtarget;

            if(!(actor->flags & MF_AMBUSH) || P_CheckSight(actor, actor->target))
            {
                actor->threshold = 10;
                P_SetMobjState(actor, actor->info->seestate);
                return;
            }
        }
    }

    // do some idle animation
    if(P_Random() < 30)
    {
        int t = P_Random();
        P_SetMobjState(actor, (t & 1) + actor->info->spawnstate + 1);
    }

    // wander around a bit
    if(!(actor->flags & MF_STAND) && P_Random() < 40)
        P_SetMobjState(actor, actor->info->spawnstate + 3);
}

//
// A_Listen
//
// [STRIFE] New function
// haleyjd 09/05/10: Action routine used to strictly listen for a target.
//
void A_Listen(mobj_t* actor)
{
    mobj_t *soundtarget;

    actor->threshold = 0;

    soundtarget = actor->subsector->sector->soundtarget;

    if(soundtarget && (soundtarget->flags & MF_SHOOTABLE))
    {
        if((actor->flags & MF_ALLY) != (soundtarget->flags & MF_ALLY))
        {
            actor->target = soundtarget;

            if(!(actor->flags & MF_AMBUSH) || P_CheckSight(actor, actor->target))
            {
                if(actor->info->seesound)
                    S_StartSound(actor, actor->info->seesound);

                actor->threshold = 10;

                P_SetMobjState(actor, actor->info->seestate);
            }
        }
    }
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
// haleyjd 09/05/10: [STRIFE] Various minor changes
//
void A_Chase (mobj_t*	actor)
{
    int         delta;

    if (actor->reactiontime)
        actor->reactiontime--;

    // modify target threshold
    if  (actor->threshold)
    {
        // haleyjd 20110204 [STRIFE]: No health <= 0 check here!
        if (actor->target)
            actor->threshold--;
        else
            actor->threshold = 0;
    }
    
    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

    if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor, true))
            return; 	// got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }
    
    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        // [STRIFE] Checks only against fastparm, not gameskill == 5
        if (!fastparm)
            P_NewChaseDir (actor);
        return;
    }
    
    // check for melee attack
    if (actor->info->meleestate
        && P_CheckMeleeRange (actor))
    {
        if (actor->info->attacksound)
            S_StartSound (actor, actor->info->attacksound);

        P_SetMobjState (actor, actor->info->meleestate);
        return;
    }
    
    // check for missile attack
    if (actor->info->missilestate)
    {
        // [STRIFE] Checks only fastparm.
        if (!fastparm && actor->movecount)
        {
            goto nomissile;
        }

        if (!P_CheckMissileRange (actor))
            goto nomissile;

        P_SetMobjState (actor, actor->info->missilestate);

        // [STRIFE] Add NODIALOG flag to disable dialog
        actor->flags |= (MF_NODIALOG|MF_JUSTATTACKED);
        return;
    }

    // ?
nomissile:
    // possibly choose another target
    if (netgame
        && !actor->threshold
        && !P_CheckSight (actor, actor->target) )
    {
        if (P_LookForPlayers(actor, true))
            return; // got a new target
    }
    
    // chase towards player
    if (--actor->movecount<0
        || !P_Move (actor))
    {
        P_NewChaseDir (actor);
    }

    // [STRIFE] Changes to active sound behavior:
    // * Significantly more frequent
    // * Acolytes have randomized wandering sounds

    // make active sound
    if (actor->info->activesound && P_Random () < 38)
    {
        if(actor->info->activesound >= sfx_agrac1 &&
           actor->info->activesound <= sfx_agrac4)
        {
            S_StartSound(actor, sfx_agrac1 + P_Random() % 4);
        }
        else
            S_StartSound(actor, actor->info->activesound);
    }
}


//
// A_FaceTarget
//
// [STRIFE]
// haleyjd 09/05/10: Handling for visibility-modifying flags.
//
void A_FaceTarget (mobj_t* actor)
{
    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;

    actor->angle = R_PointToAngle2 (actor->x,
                                    actor->y,
                                    actor->target->x,
                                    actor->target->y);

    if(actor->target->flags & MF_SHADOW)
    {
        // [STRIFE] increased SHADOW inaccuracy by a power of 2
        int t = P_Random();
        actor->angle += (t - P_Random()) << 22;
    }
    else if(actor->target->flags & MF_MVIS)
    {
        // [STRIFE] MVIS gives even worse aiming!
        int t = P_Random();
        actor->angle += (t - P_Random()) << 23;
    }
}

//
// A_PeasantPunch
//
// [STRIFE] New function
// haleyjd 09/05/10: Attack used by Peasants as a one-time retaliation
// when the player or a monster injures them. Weak doesn't begin to
// describe it :P
//
void A_PeasantPunch(mobj_t* actor)
{
    if(!actor->target)
        return;

    A_FaceTarget(actor);
    if(P_CheckMeleeRange(actor))
        P_DamageMobj(actor->target, actor, actor, 2 * (P_Random() % 5) + 2);
}

//
// A_ReaverAttack
//
// [STRIFE] New function
// haleyjd 09/06/10: Action routine used by Reavers to fire bullets.
// Also apparently used by Inquistors, though they don't seem to use
// it too often, as they're content to blow your face off with their
// HE grenades instead.
//
void A_ReaverAttack(mobj_t* actor)
{
    int i = 0;
    fixed_t slope;

    if(!actor->target)
        return;

    S_StartSound(actor, sfx_reavat);
    A_FaceTarget(actor);

    slope = P_AimLineAttack(actor, actor->angle, 2048*FRACUNIT);

    do
    {
        int     t          = P_Random();
        angle_t shootangle = actor->angle + ((t - P_Random()) << 20);
        int     damage     = 3*((P_Random() & 7) + 1);

        P_LineAttack(actor, shootangle, 2048*FRACUNIT, slope, damage);
        ++i;
    }
    while(i < 3);
}

//
// A_BulletAttack
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for generic bullet attacks. Used by
// a lot of different characters, including Acolytes, Rebels, and Macil.
//
void A_BulletAttack(mobj_t* actor)
{
    int t, damage;
    fixed_t slope;
    angle_t shootangle;

    if(!actor->target)
        return;

    S_StartSound(actor, sfx_rifle);
    A_FaceTarget(actor);
    
    slope = P_AimLineAttack(actor, actor->angle, 2048*FRACUNIT);
    t = P_Random();
    shootangle = ((t - P_Random()) << 19) + actor->angle;
    damage = 3 * (P_Random() % 5 + 1);

    P_LineAttack(actor, shootangle, 2048*FRACUNIT, slope, damage);
}

//
// A_CheckTargetVisible
//
// [STRIFE] New function
// haleyjd 09/06/10: Action routine which sets a thing back to its
// seestate at random, or if it cannot see its target, or its target
// is dead. Used by diverse actors.
//
void A_CheckTargetVisible(mobj_t* actor)
{
    A_FaceTarget(actor);

    if(P_Random() >= 30)
    {
        mobj_t *target = actor->target;
        
        if(!target || target->health <= 0 || !P_CheckSight(actor, target) ||
            P_Random() < 40)
        {
            P_SetMobjState(actor, actor->info->seestate);
        }
    }
}

//
// A_SentinelAttack
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function implementing the Sentinel's laser attack
// villsa 09/06/10 implemented
//
void A_SentinelAttack(mobj_t* actor)
{
    mobj_t* mo;
    mobj_t* mo2;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t an;
    int i;

    mo = P_SpawnFacingMissile(actor, actor->target, MT_L_LASER);
    an = actor->angle >> ANGLETOFINESHIFT;

    if(mo->momy | mo->momx) // villsa - fixed typo (yes, they actually used '|' instead of'||')
    {
        for(i = 8; i > 1; i--)
        {
            x = mo->x + FixedMul(mobjinfo[MT_L_LASER].radius * i, finecosine[an]);
            y = mo->y + FixedMul(mobjinfo[MT_L_LASER].radius * i, finesine[an]);
            z = mo->z + i * (mo->momz >> 2);
            mo2 = P_SpawnMobj(x, y, z, MT_R_LASER);
            mo2->target = actor;
            mo2->momx = mo->momx;
            mo2->momy = mo->momy;
            mo2->momz = mo->momz;
            P_CheckMissileSpawn(mo2);
        }
    }

    mo->z += mo->momz >> 2;
}

//
// A_StalkerThink
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function to drive Stalker logic.
//
void A_StalkerThink(mobj_t* actor)
{
    statenum_t statenum;

    if(actor->flags & MF_NOGRAVITY)
    {
        if(actor->ceilingz - actor->info->height <= actor->z)
            return;
        statenum = S_SPID_11; // 1020
    }
    else
        statenum = S_SPID_18; // 1027

    P_SetMobjState(actor, statenum);
}

//
// A_StalkerSetLook
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function to marshall transitions to the
// Stalker's spawnstate.
//
void A_StalkerSetLook(mobj_t* actor)
{
    statenum_t statenum;

    if(!actor) // weird; totally unnecessary.
        return;

    if(actor->flags & MF_NOGRAVITY)
    {
        if(actor->state->nextstate == S_SPID_01) // 1010
            return;
        statenum = S_SPID_01; // 1010
    }
    else
    {
        if(actor->state->nextstate == S_SPID_02) // 1011
            return;
        statenum = S_SPID_02; // 1011
    }

    P_SetMobjState(actor, statenum);
}

//
// A_StalkerDrop
//
// [STRIFE] New function
// haleyjd 09/06/10: Dead simple: removes NOGRAVITY status.
//
void A_StalkerDrop(mobj_t* actor)
{
    actor->flags &= ~MF_NOGRAVITY;
}

//
// A_StalkerScratch
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for Stalker's attack.
//
void A_StalkerScratch(mobj_t* actor)
{
    if(actor->flags & MF_NOGRAVITY)
    {
        // Drop him down before he can attack
        P_SetMobjState(actor, S_SPID_11); // 1020
        return;
    }

    if(!actor->target)
        return;

    A_FaceTarget(actor);
    if(P_CheckMeleeRange(actor))
        P_DamageMobj(actor->target, actor, actor, 2 * (P_Random() % 8) + 2);
}

//
// A_FloatWeave
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function which is responsible for floating
// actors' constant upward and downward movement. Probably a really bad
// idea in retrospect given how dodgy the 3D clipping implementation is.
//
void A_FloatWeave(mobj_t* actor)
{
    fixed_t height;
    fixed_t z;

    if(actor->threshold)
        return;

    if(actor->flags & MF_INFLOAT)
        return;

    height = actor->info->height;         // v2
    z      = actor->floorz + 96*FRACUNIT; // v1

    if ( z > actor->ceilingz - height - 16*FRACUNIT )
        z = actor->ceilingz - height - 16*FRACUNIT;

    if ( z >= actor->z )
        actor->momz += FRACUNIT;
    else
        actor->momz -= FRACUNIT;

    if ( z == actor->z )
        actor->threshold = 4;
    else
        actor->threshold = 8;
}

//
// A_RobotMelee
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for Reaver and Templar melee attacks.
//
void A_RobotMelee(mobj_t* actor)
{
    if(!actor->target)
        return;

    A_FaceTarget(actor);
    if(P_CheckMeleeRange(actor))
    {
        S_StartSound(actor, sfx_revbld);
        P_DamageMobj(actor->target, actor, actor, 3 * (P_Random() % 8 + 1));
    }
}

//
// A_TemplarMauler
//
// [STRIFE] New function
// haleyjd 09/06/10: Exactly what it sounds like. Kicks your ass.
//
void A_TemplarMauler(mobj_t* actor)
{
    int i, t;
    int angle;
    int bangle;
    int damage;
    int slope;

    if(!actor->target)
        return;

    S_StartSound(actor, sfx_pgrdat);
    A_FaceTarget(actor);
    bangle = actor->angle;
    slope = P_AimLineAttack(actor, bangle, 2048*FRACUNIT);

    for(i = 0; i < 10; i++)
    {
        // haleyjd 09/06/10: Very carefully preserved order of P_Random calls
        damage = (P_Random() & 4) * 2;
        t = P_Random();
        angle = bangle + ((t - P_Random()) << 19);
        t = P_Random();
        slope = ((t - P_Random()) << 5) + slope;
        P_LineAttack(actor, angle, 2112*FRACUNIT, slope, damage);
    }
}

//
// A_CrusaderAttack
//
// villsa [STRIFE] new codepointer
// 09/06/10: Action function for the Crusader's Flamethrower.
// Very similar to the player's flamethrower, seeing how it was ripped
// off a Crusader by the Rat People ;)
//
void A_CrusaderAttack(mobj_t* actor)
{
    if(!actor->target)
        return;

    actor->z += (8*FRACUNIT);

    if(P_CheckRobotRange(actor))
    {
        A_FaceTarget(actor);
        actor->angle -= (ANG90 / 8);
        P_SpawnFacingMissile(actor, actor->target, MT_C_FLAME);
    }
    else if(P_CheckMissileRange(actor))
    {
        A_FaceTarget(actor);
        actor->z += (16*FRACUNIT);
        P_SpawnFacingMissile(actor, actor->target, MT_C_MISSILE);

        actor->angle -= (ANG45 / 32);
        actor->z -= (16*FRACUNIT);
        P_SpawnFacingMissile(actor, actor->target, MT_C_MISSILE);

        actor->angle += (ANG45 / 16);
        P_SpawnFacingMissile(actor, actor->target, MT_C_MISSILE);

        P_SetMobjState(actor, actor->info->seestate);
        actor->reactiontime += 15;
    }
    else
        P_SetMobjState(actor, actor->info->seestate);
    
    actor->z -= (8*FRACUNIT);
}

//
// A_CrusaderLeft
//
// villsa [STRIFE] new codepointer
//
void A_CrusaderLeft(mobj_t* actor)
{
    mobj_t* mo;

    actor->angle += (ANG90 / 16);
    mo = P_SpawnFacingMissile(actor, actor->target, MT_C_FLAME);
    mo->momz = FRACUNIT;
    mo->z += (16*FRACUNIT);

}

//
// A_CrusaderRight
//
// villsa [STRIFE] new codepointer
//
void A_CrusaderRight(mobj_t* actor)
{
    mobj_t* mo;

    actor->angle -= (ANG90 / 16);
    mo = P_SpawnFacingMissile(actor, actor->target, MT_C_FLAME);
    mo->momz = FRACUNIT;
    mo->z += (16*FRACUNIT);
}

//
// A_CheckTargetVisible2
//
// [STRIFE] New function
// haleyjd 09/06/10: Mostly the same as CheckTargetVisible, except without
// the randomness.
//
void A_CheckTargetVisible2(mobj_t* actor)
{
    if(!actor->target || actor->target->health <= 0 || 
        !P_CheckSight(actor, actor->target))
    {
        P_SetMobjState(actor, actor->info->seestate);
    }
}

//
// A_InqFlyCheck
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function to check if an Inquisitor wishes
// to take to flight.
//
void A_InqFlyCheck(mobj_t* actor)
{
    if(!actor->target)
        return;

    A_FaceTarget(actor);

    // if not in "robot" range, shoot grenades.
    if(!P_CheckRobotRange(actor))
        P_SetMobjState(actor, S_ROB3_14); // 1061

    if(actor->z != actor->target->z)
    {
        // Take off all zig!
        if(actor->z + actor->height + 54*FRACUNIT < actor->ceilingz)
            P_SetMobjState(actor, S_ROB3_17); // 1064
    }
}

//
// A_InqGrenade
//
// villsa [STRIFE] new codepointer
// 09/06/10: Inquisitor grenade attack action routine.
//
void A_InqGrenade(mobj_t* actor)
{
    mobj_t* mo;

    if(!actor->target)
        return;

    A_FaceTarget(actor);

    actor->z += MAXRADIUS;

    // grenade 1
    actor->angle -= (ANG45 / 32);
    mo = P_SpawnFacingMissile(actor, actor->target, MT_INQGRENADE);
    mo->momz += (9*FRACUNIT);

    // grenade 2
    actor->angle += (ANG45 / 16);
    mo = P_SpawnFacingMissile(actor, actor->target, MT_INQGRENADE);
    mo->momz += (16*FRACUNIT);

    actor->z -= MAXRADIUS;
}

//
// A_InqTakeOff
//
// [STRIFE] New function
// haleyjd 09/06/10: Makes an Inquisitor start flying.
//
void A_InqTakeOff(mobj_t* actor)
{
    angle_t an;
    fixed_t speed = actor->info->speed * (2 * FRACUNIT / 3);
    fixed_t dist;

    if(!actor->target)
        return;

    S_StartSound(actor, sfx_inqjmp);

    actor->z += 64 * FRACUNIT;

    A_FaceTarget(actor);

    an = actor->angle >> ANGLETOFINESHIFT;

    actor->momx = FixedMul(finecosine[an], speed);
    actor->momy = FixedMul(finesine[an],   speed);

    dist = P_AproxDistance(actor->target->x - actor->x,
                           actor->target->y - actor->y);

    dist /= speed;
    if(dist < 1)
        dist = 1;

    actor->momz = (actor->target->z - actor->z) / dist;
    actor->reactiontime = 60;
    actor->flags |= MF_NOGRAVITY;
}

//
// A_InqFly
//
// [STRIFE] New function
// haleyjd 09/06/10: Handles an Inquisitor in flight.
//
void A_InqFly(mobj_t* actor)
{
    if(!(leveltime & 7))
        S_StartSound(actor, sfx_inqjmp);

    if(--actor->reactiontime < 0 || !actor->momx || !actor->momy ||
        actor->z <= actor->floorz)
    {
        // Come in for a landing.
        P_SetMobjState(actor, actor->info->seestate);
        actor->reactiontime = 0;
        actor->flags &= ~MF_NOGRAVITY;
    }
}

//
// A_FireSigilWeapon
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for the Entity's attack.
//
void A_FireSigilWeapon(mobj_t* actor)
{
    int choice = P_Random() % 5;

    // STRIFE-TODO: Needs verification. This switch is just weird.
    switch(choice)
    {
    case 0:
        A_ProgrammerAttack(actor);
        break;
    // ain't not seen no case 1, bub...
    case 2:
        A_FireSigilEOffshoot(actor);
        break;
    case 3:
        A_SpectreCAttack(actor);
        break;
    case 4:
        A_SpectreDAttack(actor);
        break;
    case 5: // BUG: never used? wtf were they thinking?
        A_SpectreEAttack(actor);
        break;
    default:
        break;
    }
}

//
// A_ProgrammerAttack
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for the Programmer's main
// attack; equivalent to the player's first Sigil.
//
void A_ProgrammerAttack(mobj_t* actor)
{
    mobj_t *mo;

    if(!actor->target)
        return;

    mo = P_SpawnMobj(actor->target->x, actor->target->y, ONFLOORZ, 
                     MT_SIGIL_A_GROUND);
    mo->threshold = 25;
    mo->target = actor;
    mo->health = -2;
    mo->tracer = actor->target;
}

//
// A_Sigil_A_Action
//
// [STRIFE] New function
// haleyjd 09/06/10: Called by MT_SIGIL_A_GROUND to zot anyone nearby with
// corny looking lightning bolts.
//
void A_Sigil_A_Action(mobj_t* actor)
{
    int t, x, y, type;
    mobj_t *mo;

    if(actor->threshold)
        actor->threshold--;

    t = P_Random();
    actor->momx += ((t & 3) - (P_Random() & 3)) << FRACBITS;
    t = P_Random();
    actor->momy += ((t & 3) - (P_Random() & 3)) << FRACBITS;

    t = P_Random();
    x = 50*FRACUNIT * ((t & 3) - (P_Random() & 3)) + actor->x;
    t = P_Random();
    y = 50*FRACUNIT * ((t & 3) - (P_Random() & 3)) + actor->y;

    if(actor->threshold <= 25)
        type = MT_SIGIL_A_ZAP_LEFT;
    else
        type = MT_SIGIL_A_ZAP_RIGHT;

    mo = P_SpawnMobj(x, y, ONCEILINGZ, type);
    mo->momz = -18 * FRACUNIT;
    mo->target = actor->target;
    mo->health = actor->health;

    mo = P_SpawnMobj(actor->x, actor->y,  ONCEILINGZ, MT_SIGIL_A_ZAP_RIGHT);
    mo->momz = -18 * FRACUNIT;
    mo->target = actor->target;
    mo->health = actor->health;
}

//
// A_SpectreEAttack
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for the Loremaster's Spectre.
// Equivalent to the player's final Sigil attack.
//
void A_SpectreEAttack(mobj_t* actor)
{
    mobj_t *mo;

    if(!actor->target)
        return;
    
    mo = P_SpawnMissile(actor, actor->target, MT_SIGIL_SE_SHOT);
    mo->health = -2;
}

//
// A_SpectreCAttack
//
// villsa [STRIFE] new codepointer
// 09/06/10: Action routine for the Oracle's Spectre. Equivalent to the player's
// third Sigil attack.
//
void A_SpectreCAttack(mobj_t* actor)
{
    mobj_t* mo;
    int i;

    if(!actor->target)
        return;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + (32*FRACUNIT), MT_SIGIL_A_ZAP_RIGHT);
    mo->momz = -(18*FRACUNIT);
    mo->target = actor;
    mo->health = -2;
    mo->tracer = actor->target;
    
    actor->angle -= ANG90;
    for(i = 0; i < 20; i++)
    {
        actor->angle += (ANG90 / 10);
        mo = P_SpawnMortar(actor, MT_SIGIL_C_SHOT);
        mo->health = -2;
        mo->z = actor->z + (32*FRACUNIT);
    }
    actor->angle -= ANG90;
}

//
// A_AlertSpectreC
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function called by the Oracle when it is
// killed. Finds an MT_SPECTRE_C anywhere on the map and awakens it.
//
void A_AlertSpectreC(mobj_t* actor)
{
    thinker_t *th;

    for(th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if(th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;

            if(mo->type == MT_SPECTRE_C)
            {
                P_SetMobjState(mo, mo->info->seestate);
                mo->target = actor->target;
                return;
            }
        }
    }
}

//
// A_Sigil_E_Action
//
// villsa [STRIFE] new codepointer
// 09/06/10: Action routine for Sigil "E" shots. Spawns the room-filling
// lightning bolts that seem to often do almost nothing.
//
void A_Sigil_E_Action(mobj_t* actor)
{
    actor->angle += ANG90;
    P_SpawnMortar(actor, MT_SIGIL_E_OFFSHOOT);

    actor->angle -= ANG180;
    P_SpawnMortar(actor, MT_SIGIL_E_OFFSHOOT);

    actor->angle += ANG90;
    P_SpawnMortar(actor, MT_SIGIL_E_OFFSHOOT);

}

//
// A_SigilTrail
//
// villsa [STRIFE] new codepointer
//
void A_SigilTrail(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x - actor->momx,
                     actor->y - actor->momy,
                     actor->z, MT_SIGIL_TRAIL);

    mo->angle = actor->angle;
    mo->health = actor->health;

}

//
// A_SpectreDAttack
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function for Macil's Spectre.
// Equivalent of the player's fourth Sigil attack.
//
void A_SpectreDAttack(mobj_t* actor)
{
    mobj_t *mo;

    if(!actor->target)
        return;

    mo = P_SpawnMissile(actor, actor->target, MT_SIGIL_SD_SHOT);
    mo->health = -2;
    mo->tracer = actor->target;
}

//
// A_FireSigilEOffshoot
//
// [STRIFE] New function
// haleyjd 09/06/10: Action function to fire part of a Sigil E
// attack. Used at least by the Entity.
//
void A_FireSigilEOffshoot(mobj_t* actor)
{
    mobj_t *mo;

    if(!actor->target)
        return;

    mo = P_SpawnMissile(actor, actor->target, MT_SIGIL_E_OFFSHOOT);
    mo->health = -2;
}

//
// A_ShadowOff
//
// villsa [STRIFE] new codepointer
// 09/06/10: Disables SHADOW and MVIS flags.
//
void A_ShadowOff(mobj_t* actor)
{
    actor->flags &= ~(MF_SHADOW|MF_MVIS);
}

//
// A_ModifyVisibility
//
// villsa [STRIFE] new codepointer
// 09/06/10: Turns on SHADOW, and turns off MVIS.
//
void A_ModifyVisibility(mobj_t* actor)
{
    actor->flags |= MF_SHADOW;
    actor->flags &= ~MF_MVIS;
}

//
// A_ShadowOn
//
// villsa [STRIFE] new codepointer
// 09/06/10: Turns on SHADOW and MVIS.
//
void A_ShadowOn(mobj_t* actor)
{
    actor->flags |= (MF_SHADOW|MF_MVIS);
}

//
// A_SetTLOptions
//
// villsa [STRIFE] new codepointer
// 09/06/10: Sets SHADOW and/or MVIS based on the thing's spawnpoint options.
//
void A_SetTLOptions(mobj_t* actor)
{
    if(actor->spawnpoint.options & MTF_TRANSLUCENT)
        actor->flags |= MF_SHADOW;
    if(actor->spawnpoint.options & MTF_MVIS)
        actor->flags |= MF_MVIS;
}

//
// A_BossMeleeAtk
//
// villsa [STRIFE] new codepointer
// 09/06/10: Gratuitous melee attack used by multiple boss characters,
// just for the sake of having one. It's not like anybody in their right
// mind would get close to any of the maniacs that use this ;)
//
void A_BossMeleeAtk(mobj_t* actor)
{
    if(!actor->target)
        return;

    P_DamageMobj(actor->target, actor, actor, 10 * (P_Random() & 9));
}

//
// A_BishopAttack
//
// villsa [STRIFE] new codepointer
// 09/06/10: Bishop's homing missile attack.
//
void A_BishopAttack(mobj_t* actor)
{
    mobj_t* mo;

    if(!actor->target)
        return;

    actor->z += MAXRADIUS;

    mo = P_SpawnMissile(actor, actor->target, MT_SEEKMISSILE);
    mo->tracer = actor->target;

    actor->z -= MAXRADIUS;
}

//
// A_FireHookShot
//
// villsa [STRIFE] new codepointer
// 09/06/10: Action function for the Loremaster's hookshot attack.
//
void A_FireHookShot(mobj_t* actor)
{
    if(!actor->target)
        return;

    P_SpawnMissile(actor, actor->target, MT_HOOKSHOT);
}

//
// A_FireChainShot
//
// villsa [STRIFE] new codepointer
// 09/06/10: Action function for the hookshot projectile. Spawns echoes
// to create a chain-like appearance.
//
void A_FireChainShot(mobj_t* actor)
{
    S_StartSound(actor, sfx_tend);

    P_SpawnMobj(actor->x, actor->y, actor->z, MT_CHAINSHOT); // haleyjd: fixed type

    P_SpawnMobj(actor->x - (actor->momx >> 1),
                actor->y - (actor->momy >> 1),
                actor->z, MT_CHAINSHOT);

    P_SpawnMobj(actor->x - actor->momx,
                actor->y - actor->momy,
                actor->z, MT_CHAINSHOT);
}

//
// A_MissileSmoke
//
// villsa [STRIFE] new codepointer
//
void A_MissileSmoke(mobj_t* actor)
{
    mobj_t* mo;

    S_StartSound(actor, sfx_rflite);
    P_SpawnPuff(actor->x, actor->y, actor->z);
    mo = P_SpawnMobj(actor->x - actor->momx,
                     actor->y - actor->momy,
                     actor->z, MT_MISSILESMOKE);

    mo->momz = FRACUNIT;
}

//
// A_SpawnSparkPuff
//
// villsa [STRIFE] new codepointer
//
void A_SpawnSparkPuff(mobj_t* actor)
{
    int r;
    mobj_t* mo;
    fixed_t x;
    fixed_t y;

    r = P_Random();
    x = (10*FRACUNIT) * ((r & 3) - (P_Random() & 3)) + actor->x;
    r = P_Random();
    y = (10*FRACUNIT) * ((r & 3) - (P_Random() & 3)) + actor->y;

    mo = P_SpawnMobj(x, y, actor->z, MT_SPARKPUFF);
    P_SetMobjState(mo, S_BNG4_01); // 199
    mo->momz = FRACUNIT;
}


// haleyjd 09/05/10: [STRIFE] Removed:
// A_PosAttack, A_SPosAttack, A_CPosAttack, A_CPosRefire, A_SpidRefire, 
// A_BspiAttack, A_TroopAttack, A_SargAttack, A_HeadAttack, A_CyberAttack,
// A_BruisAttack, A_SkelMissile


int TRACEANGLE = 0xE000000; // villsa [STRIFE] changed from 0xC000000 to 0xE000000

//
// A_Tracer
//
void A_Tracer (mobj_t* actor)
{
    angle_t exact;
    fixed_t dist;
    fixed_t slope;
    mobj_t* dest;
    //mobj_t* th;

    // villsa [STRIFE] removed all randomization and puff code

    // adjust direction
    dest = actor->tracer;

    if(!dest || dest->health <= 0)
        return;

    // change angle	
    exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

    if(exact != actor->angle)
    {
        // villsa [STRIFE] slightly different algorithm
        if(exact - actor->angle <= 0x80000000)
        {
            actor->angle += TRACEANGLE;
            if(exact - actor->angle > 0x80000000)
                actor->angle = exact;
        }
        else
        {
            actor->angle -= TRACEANGLE;
            if (exact - actor->angle < 0x80000000)
                actor->angle = exact;
        }
    }

    exact = actor->angle>>ANGLETOFINESHIFT;
    actor->momx = FixedMul (actor->info->speed, finecosine[exact]);
    actor->momy = FixedMul (actor->info->speed, finesine[exact]);

    // change slope
    dist = P_AproxDistance (dest->x - actor->x,
        dest->y - actor->y);

    dist = dist / actor->info->speed;

    if (dist < 1)
        dist = 1;
    slope = (dest->z+40*FRACUNIT - actor->z) / dist;

    if (slope < actor->momz)
        actor->momz -= FRACUNIT/8;
    else
        actor->momz += FRACUNIT/8;
}

//
// A_ProgrammerMelee
//
// villsa [STRIFE] new codepointer
// 09/08/10: Melee attack for the Programmer.
// haleyjd - fixed damage formula
//
void A_ProgrammerMelee(mobj_t* actor)
{
    if(!actor->target)
        return;

    A_FaceTarget(actor);
    if(P_CheckMeleeRange(actor))
    {
        int damage = 6 * (P_Random() % 10 + 1);
        
        S_StartSound(actor, sfx_mtalht);
        P_DamageMobj(actor->target, actor, actor, damage);
    }

}

// haleyjd 09/05/10: [STRIFE] Removed:
// A_SkelWhoosh, A_SkelFist, PIT_VileCheck, A_VileChase, A_VileStart, 
// A_StartFire, A_FireCrackle, A_Fire, A_VileTarget, A_VileAttack
// A_FatRaise, A_FatAttack1, A_FatAttack2, A_FatAttack3, A_SkullAttack, 
// A_PainShootSkull, A_PainAttack, A_PainDie

//
// A_Scream
//
// villsa [STRIFE] 
// * Has no random death sounds, so play deathsound directly
// * Full-volume roars for the Entity and Inquisitor.
//
void A_Scream(mobj_t* actor)
{
    if(!actor->info->deathsound)
        return;

    // Check for bosses.
    if(actor->type == MT_ENTITY || actor->type == MT_INQUISITOR)
        S_StartSound(NULL, actor->info->deathsound);   // full volume
    else
        S_StartSound(actor, actor->info->deathsound);
}

//
// A_XScream
//
// villsa [STRIFE]
// * Robots will play deathsound while non-robots play the slop sfx
//
void A_XScream(mobj_t* actor)
{
    int sound;

    if(actor->flags & MF_NOBLOOD && actor->info->deathsound)
        sound = actor->info->deathsound;
    else
        sound = sfx_slop;

    S_StartSound(actor, sound);
}

//
// A_Pain
//
// villsa [STRIFE] 
// * Play random peasant sounds; otherwise play painsound directly
//
void A_Pain(mobj_t* actor)
{
    int sound = actor->info->painsound;

    if(sound)
    {
        if(sound >= sfx_pespna && sound <= sfx_pespnd)
            sound = sfx_pespna + (P_Random() % 4);
        
        S_StartSound(actor, sound);
    }
}

//
// A_PeasantCrash
//
// villsa [STRIFE] new codepointer
// 09/08/10: Called from Peasant's "crash" state (not to be confused with
// Heretic crash states), which is invoked when the Peasant has taken
// critical but sub-fatal damage. It will "bleed out" the rest of its
// health by calling this function repeatedly.
//
void A_PeasantCrash(mobj_t* actor)
{
    // Set NODIALOG, because you probably wouldn't feel like talking either
    // if somebody just stabbed you in the gut with a punch dagger...
    actor->flags |= MF_NODIALOG;

    if(!(P_Random() % 5))
    {
        A_Pain(actor);  // inlined in asm
        actor->health--;
    }

    if(actor->health <= 0)
        P_KillMobj(actor->target, actor);
}

//
// A_Fall
//
// [STRIFE]
// * Set NODIALOG, and clear NOGRAVITY and SHADOW
//
void A_Fall (mobj_t *actor)
{
    // villsa [STRIFE] set NODIALOG flag to stop dialog
    actor->flags |= MF_NODIALOG;

    // actor is on ground, it can be walked over
    // villsa [STRIFE] remove nogravity/shadow flags as well
    actor->flags &= ~(MF_SOLID|MF_NOGRAVITY|MF_SHADOW);
}

//
// A_HideZombie
//
// villsa [STRIFE] new codepointer
// Used by the "Becoming" Acolytes on the Loremaster's level.
//
void A_HideZombie(mobj_t* actor)
{
    line_t junk;

    junk.tag = 999;
    EV_DoDoor(&junk, vld_blazeClose);

    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

//
// A_MerchantPain
//
// villsa [STRIFE] new codepointer
// 09/08/10: Pain pointer for merchant characters. They close up shop for
// a while and set off the alarm.
//
void A_MerchantPain(mobj_t* actor)
{
    line_t junk;

    junk.tag = 999;
    EV_DoDoor(&junk, vld_shopClose);

    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

// haleyjd 09/05/10: Removed unused CheckBossEnd Choco routine.

// haleyjd 09/05/10: [STRIFE] Removed:
// A_Hoof, A_Metal, A_BabyMetal, A_OpenShotgun2, A_LoadShotgun2, 
// A_CloseShotgun2, A_BrainAwake, A_BrainPain, A_BrainScream, A_BrainExplode,
// A_BrainDie, A_BrainSpit, A_SpawnSound, A_SpawnFly

//
// A_ProgrammerDie
//
// villsa [STRIFE] new codepointer
// 09/08/10: Action routine for the Programmer's grisly death. Spawns the 
// separate mechanical base object and sends it flying off in some random 
// direction.
//
void A_ProgrammerDie(mobj_t* actor)
{
    int r;
    angle_t an;
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 24*FRACUNIT, MT_PROGRAMMERBASE);

    // haleyjd 20110223: fix add w/ANG180
    r = P_Random();
    an = ((r - P_Random()) << 22) + actor->angle + ANG180;
    mo->angle = an;

    P_ThrustMobj(mo, an, mo->info->speed);  // inlined in asm

    mo->momz = P_Random() << 9;
}

//
// A_InqTossArm
//
// villsa [STRIFE] new codepointer
// 09/08/10: Inquisitor death action. Spawns an arm and tosses it.
//
void A_InqTossArm(mobj_t* actor)
{
    int r;
    angle_t an;
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + (24*FRACUNIT), MT_INQARM);

    r = P_Random();
    an = ((r - P_Random()) << 22) + actor->angle - ANG90;
    mo->angle = an;

    P_ThrustMobj(mo, an, mo->info->speed);  // inlined in asm

    mo->momz = P_Random() << 10;
}

//
// A_SpawnSpectreA
//
// villsa [STRIFE] new codepointer (unused)
// 09/08/10: Spawns Spectre A. Or would, if anything actually used this.
// This is evidence that the Programmer's spectre, which appears in the 
// Catacombs in the final version, was originally meant to be spawned
// after his death.
//
void A_SpawnSpectreA(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPECTRE_A);
    mo->momz = P_Random() << 9;
}

//
// A_SpawnSpectreB
//
// villsa [STRIFE] new codepointer
// 09/08/10: Action function to spawn the Bishop's spectre.
//
void A_SpawnSpectreB(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPECTRE_B);
    mo->momz = P_Random() << 9;
}

//
// A_SpawnSpectreC
//
// villsa [STRIFE] new codepointer (unused)
// 09/08/10: Action function to spawn the Oracle's spectre. Also
// unused, because the Oracle's spectre is already present on the
// map and is awakened on his death. Also left over from the 
// unreleased beta (and demo) versions.
//
void A_SpawnSpectreC(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPECTRE_C);
    mo->momz = P_Random() << 9;
}

//
// A_SpawnSpectreD
//
// villsa [STRIFE] new codepointer
// 09/08/10: Action function to spawn Macil's Spectre.
//
void A_SpawnSpectreD(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPECTRE_D);
    mo->momz = P_Random() << 9;
}

//
// A_SpawnSpectreE
//
// villsa [STRIFE] new codepointer
// 09/08/10: Action function to spawn the Loremaster's Spectre.
//
void A_SpawnSpectreE(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPECTRE_E);
    mo->momz = P_Random() << 9;
}

// [STRIFE] New statics - Remember the Entity's spawning position.
static fixed_t entity_pos_x = 0;
static fixed_t entity_pos_y = 0;
static fixed_t entity_pos_z = 0;

//
// A_SpawnEntity
//
// villsa [STRIFE] new codepointer
// 09/08/10: You will fall on your knees before the True God, the One Light.
//
void A_SpawnEntity(mobj_t* actor)
{
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 70*FRACUNIT, MT_ENTITY);
    mo->momz = 5*FRACUNIT;

    entity_pos_x = mo->x;
    entity_pos_y = mo->y;
    entity_pos_z = mo->z;
}

//
// P_ThrustMobj
//
// villsa [STRIFE] new function
// Thrusts an thing in a specified force/direction
// Beware! This is inlined everywhere in the asm
//
void P_ThrustMobj(mobj_t *actor, angle_t angle, fixed_t force)
{
    angle_t an = angle >> ANGLETOFINESHIFT;
    actor->momx += FixedMul(finecosine[an], force);
    actor->momy += FixedMul(finesine[an],   force);
}

//
// A_EntityDeath
//
// [STRIFE]
// haleyjd 09/08/10: The death of the Entity's spectre brings forth
// three subentities, which are significantly less dangerous on their
// own but threatening together.
//
void A_EntityDeath(mobj_t* actor)
{
    mobj_t *subentity;
    angle_t an;
    fixed_t dist;

    dist = 2 * mobjinfo[MT_SUBENTITY].radius;

    // Subentity One
    an = actor->angle >> ANGLETOFINESHIFT;
    subentity = P_SpawnMobj(FixedMul(finecosine[an], dist) + entity_pos_x,
                            FixedMul(finesine[an],   dist) + entity_pos_y,
                            entity_pos_z, MT_SUBENTITY);
    subentity->target = actor->target;
    A_FaceTarget(subentity);
    P_ThrustMobj(subentity, subentity->angle, 625 << 13);

    // Subentity Two
    an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    subentity = P_SpawnMobj(FixedMul(finecosine[an], dist) + entity_pos_x, 
                            FixedMul(finesine[an],   dist) + entity_pos_y,
                            entity_pos_z, MT_SUBENTITY);
    subentity->target = actor->target;
    P_ThrustMobj(subentity, actor->angle + ANG90, 4);
    A_FaceTarget(subentity);

    // Subentity Three
    an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
    subentity = P_SpawnMobj(FixedMul(finecosine[an], dist) + entity_pos_x, 
                            FixedMul(finesine[an],   dist) + entity_pos_y,
                            entity_pos_z, MT_SUBENTITY);
    subentity->target = actor->target;
    P_ThrustMobj(subentity, actor->angle - ANG90, 4);
    A_FaceTarget(subentity);
}

//
// A_SpawnZombie
//
// villsa [STRIFE] new codepointer
//
void A_SpawnZombie(mobj_t* actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z, MT_ZOMBIE);
}

//
// A_ZombieInSpecialSector
//
// villsa [STRIFE] new codepointer
//
void A_ZombieInSpecialSector(mobj_t* actor)
{
    sector_t* sector;
    fixed_t force;
    angle_t angle;
    int tagval;

    sector = actor->subsector->sector;
    if(actor->z != sector->floorheight)
        return;

    if(sector->special <= 15)
        P_DamageMobj(actor, NULL, NULL, 999);
    else if(sector->special == 18)
    {
        tagval = sector->tag - 100;
        force = (tagval % 10) << 12;
        angle = (tagval / 10) << 29;
        P_ThrustMobj(actor, angle, force);  // inlined in asm
    }
}

//
// A_CrystalExplode
//
// villsa [STRIFE] new codepointer
// Throws out debris from the Power Crystal and sets its sector floorheight
// to the lowest surrounding floor (this is maybe the only time a direct
// level-changing action is done by an object in this fashion in any of
// the DOOM engine games... they usually call a line special instead)
//
void A_CrystalExplode(mobj_t* actor)
{
    sector_t* sector;
    mobj_t* rubble;
    int i;
    int r;

    sector = actor->subsector->sector;
    sector->lightlevel = 0;
    sector->floorheight = P_FindLowestFloorSurrounding(sector);

    // spawn rubble
    for(i = 0; i < 8; i++)
    {
        rubble = P_SpawnMobj(actor->x, actor->y, actor->z, MT_RUBBLE1 + i);
        r = P_Random();
        rubble->momx = ((r & 0x0f) - (P_Random() & 7)) << FRACBITS;
        r = P_Random();
        rubble->momy = ((r & 7) - (P_Random() & 7)) << FRACBITS;
        rubble->momz = ((P_Random() & 3) << FRACBITS) + (7*FRACUNIT);
    }
}

// [STRIFE] New static global - buffer used for various player messages.
static char pmsgbuffer[80];

// 
// P_FreePrisoners
//
// haleyjd 09/08/10: [STRIFE] New function
// * Called when the prisoners get freed, obviously. Gives a
//   message and awards quest token 13.
//
void P_FreePrisoners(void)
{
    int i;

    DEH_snprintf(pmsgbuffer, sizeof(pmsgbuffer), "You've freed the prisoners!");

    for(i = 0; i < MAXPLAYERS; i++)
    {
        P_GiveItemToPlayer(&players[i], SPR_TOKN, MT_TOKEN_QUEST13);
        players[i].message = pmsgbuffer;
    }
}

//
// P_DestroyConverter
//
// haleyjd 09/08/10: [STRIFE] New function
// * Called when the converter is shut down in the factory.
//   Gives several items and a message.
//
void P_DestroyConverter(void)
{
    int i;

    DEH_snprintf(pmsgbuffer, sizeof(pmsgbuffer), "You've destroyed the Converter!");

    for(i = 0; i < MAXPLAYERS; i++)
    {
        P_GiveItemToPlayer(&players[i], SPR_TOKN, MT_TOKEN_QUEST25);
        P_GiveItemToPlayer(&players[i], SPR_TOKN, MT_TOKEN_STAMINA);
        P_GiveItemToPlayer(&players[i], SPR_TOKN, MT_TOKEN_NEW_ACCURACY);
        players[i].message = pmsgbuffer;
    }
}

//
// A_QuestMsg
//
// villsa [STRIFE] new codepointer
// Displays text based on quest item's name
// Quest item is based on actor's speed
//
void A_QuestMsg(mobj_t* actor)
{
    char* name;
    int quest;
    int i;

    // get name
    name = DEH_String(mobjinfo[(MT_TOKEN_QUEST1 - 1) + actor->info->speed].name);
    M_StringCopy(pmsgbuffer, name, sizeof(pmsgbuffer));   // inlined in asm

    // give quest and display message to players
    for(i = 0; i < MAXPLAYERS; i++)
    {
        quest = 1 << (actor->info->speed - 1);
        players[i].message = pmsgbuffer;
        players[i].questflags |= quest;
    }
}

//
// A_ExtraLightOff
//
// villsa [STRIFE] new codepointer
// 09/08/10: Called by the Power Crystal to turn off the extended
// flash of light caused by its explosion.
//
void A_ExtraLightOff(mobj_t* actor)
{
    if(!actor->target)
        return;

    if(!actor->target->player)
        return;

    actor->target->player->extralight = 0;
}

//
// A_CrystalRadiusAtk
//
// villsa [STRIFE] new codepointer
// 09/08/10: Called by the power crystal when it dies.
//
void A_CrystalRadiusAtk(mobj_t* actor)
{
    P_RadiusAttack(actor, actor->target, 512);

    if(!(actor->target && actor->target->player))
        return;

    // set extralight to 5 for near full-bright
    actor->target->player->extralight = 5;
}

//
// A_DeathExplode5
//
// villsa [STRIFE] new codepointer
//
void A_DeathExplode5(mobj_t* actor)
{
    P_RadiusAttack(actor, actor->target, 192);
    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

//
// A_DeathExplode1
//
// villsa [STRIFE] new codepointer
//
void A_DeathExplode1(mobj_t* actor)
{
    P_RadiusAttack(actor, actor->target, 128);
    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

//
// A_DeathExplode2
//
// villsa [STRIFE] new codepointer
//
void A_DeathExplode2(mobj_t* actor)
{
    P_RadiusAttack(actor, actor->target, 64);
    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

//
// A_DeathExplode3
//
// villsa [STRIFE] new codepointer
//
void A_DeathExplode3(mobj_t* actor)
{
    P_RadiusAttack(actor, actor->target, 32);
    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

//
// A_RaiseAlarm
//
// villsa [STRIFE] new codepointer
// 09/08/10: Set off the infamous alarm. This is just a noise alert.
//
void A_RaiseAlarm(mobj_t* actor)
{
    if(actor->target && actor->target->player)
        P_NoiseAlert(actor->target, actor); // inlined in asm
}

//
// A_MissileTick
// villsa [STRIFE] - new codepointer
//
void A_MissileTick(mobj_t* actor)
{
    if(--actor->reactiontime <= 0)
    {
        P_ExplodeMissile(actor);
        actor->flags &= ~MF_MISSILE;
    }
}

//
// A_SpawnGrenadeFire
// villsa [STRIFE] - new codepointer
//
void A_SpawnGrenadeFire(mobj_t* actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z, MT_PFLAME);
}

//
// A_NodeChunk
//
// villsa [STRIFE] - new codepointer
// Throw out "nodes" from a spectral entity
//
void A_NodeChunk(mobj_t* actor)
{
    int r;
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 10*FRACUNIT, MT_NODE);
    r = P_Random();
    mo->momx = ((r & 0x0f) - (P_Random() & 7)) << FRACBITS;
    r = P_Random();
    mo->momy = ((r & 7) - (P_Random() & 0x0f)) << FRACBITS;
    mo->momz = (P_Random() & 0x0f) << FRACBITS;
}

//
// A_HeadChunk
//
// villsa [STRIFE] - new codepointer
// Throw out the little "eye"-like object from a spectral entity when it dies.
//
void A_HeadChunk(mobj_t* actor)
{
    int r;
    mobj_t* mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + 10*FRACUNIT, MT_SPECTREHEAD);
    r = P_Random();
    mo->momx = ((r & 7) - (P_Random() & 0x0f)) << FRACBITS;
    r = P_Random();
    mo->momy = ((r & 0x0f) - (P_Random() & 7)) << FRACBITS;
    mo->momz = (P_Random() & 7) << FRACBITS;
}

//
// A_BurnSpread
// villsa [STRIFE] - new codepointer
//
void A_BurnSpread(mobj_t* actor)
{
    int t;
    mobj_t* mo;
    fixed_t x;
    fixed_t y;

    actor->momz -= (8*FRACUNIT);

    t = P_Random();
    actor->momx += ((t & 3) - (P_Random() & 3)) << FRACBITS;
    t = P_Random();
    actor->momy += ((t & 3) - (P_Random() & 3)) << FRACBITS;

    S_StartSound(actor, sfx_lgfire);

    if(actor->flags & MF_DROPPED)
        return; // not the parent

    // haleyjd 20110223: match order of calls in binary
    y = actor->y + (((P_Random() + 12) & 31) << FRACBITS);
    x = actor->x + (((P_Random() + 12) & 31) << FRACBITS);

    // spawn child
    mo = P_SpawnMobj(x, y, actor->z + (4*FRACUNIT), MT_PFLAME);

    t = P_Random();
    mo->momx += ((t & 7) - (P_Random() & 7)) << FRACBITS;
    t = P_Random();
    mo->momy += ((t & 7) - (P_Random() & 7)) << FRACBITS;
    mo->momz -= FRACUNIT;
    mo->flags |= MF_DROPPED;
    mo->reactiontime = (P_Random() & 3) + 2;
}

//
// A_BossDeath
//
// Possibly trigger special effects
// if on first boss level
//
// haleyjd 09/17/10: [STRIFE]
// * Modified to handle all Strife bosses.
//
void A_BossDeath (mobj_t* actor)
{
    int i;
    thinker_t *th;
    line_t junk;

    // only the following types can be a boss:
    switch(actor->type)
    {
    case MT_CRUSADER:
    case MT_SPECTRE_A:
    case MT_SPECTRE_B:
    case MT_SPECTRE_C:
    case MT_SPECTRE_D:
    case MT_SPECTRE_E:
    case MT_SUBENTITY:
    case MT_PROGRAMMER:
        break;
    default:
        return;
    }

    // check for a living player
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i] && players[i].health > 0)
            break;
    }
    if(i == MAXPLAYERS)
        return; // everybody's dead.

    // check for a still living boss
    for(th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if(th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;

            if(mo != actor && mo->type == actor->type && mo->health > 0)
                return; // one is still alive.
        }
    }

    // Victory!
    switch(actor->type)
    {
    case MT_CRUSADER:
        junk.tag = 667;
        EV_DoFloor(&junk, lowerFloorToLowest);
        break;

    case MT_SPECTRE_A:
        GiveVoiceObjective("VOC95", "LOG95", 0);
        junk.tag = 999;
        EV_DoFloor(&junk, lowerFloorToLowest);
        break;

    case MT_SPECTRE_B:
        P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_BISHOP);
        GiveVoiceObjective("VOC74", "LOG74", 0);
        break;

    case MT_SPECTRE_C:
        // Look for an MT_ORACLE - this is for in case the player awakened the 
        // Oracle's spectre without killing the Oracle, which is possible by 
        // looking up to max and firing the Sigil at it. If this were not done,
        // a serious sequence break possibility would arise where one could 
        // kill both the Oracle AND Macil, possibly throwing the game out of
        // sorts entirely. Too bad they thought of it ;)  However this also
        // causes a bug sometimes! The Oracle, in its death state, sets the
        // Spectre C back to its seestate. If the Spectre C is already dead,
        // it becomes an undead ghost monster. Then it's a REAL spectre ;)
        for(th = thinkercap.next; th != &thinkercap; th = th->next)
        {
            if(th->function.acp1 == (actionf_p1) P_MobjThinker)
            {
                mobj_t *mo = (mobj_t *)th;

                // KILL ALL ORACLES! RAWWR!
                if(mo != actor && mo->type == MT_ORACLE && mo->health > 0)
                    P_KillMobj(actor, mo);
            }
        }
        P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_ORACLE);
        
        // Bishop is dead? - verify.
        if(players[0].questflags & QF_QUEST21) 
            P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_QUEST22);

        // Macil is dead?
        if(players[0].questflags & QF_QUEST24)
        {
            // Loremaster is dead?
            if(players[0].questflags & QF_QUEST26)
            {
                // We wield the complete sigil, blahblah
                GiveVoiceObjective("VOC85", "LOG85", 0);
            }
        }
        else
        {
            // So much for prognostication!
            GiveVoiceObjective("VOC87", "LOG87", 0);
        }
        junk.tag = 222;         // Open the exit door again;
        EV_DoDoor(&junk, vld_open); // Note this is NOT the Loremaster door...
        break;

    case MT_SPECTRE_D:
        P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_MACIL);
        if(players[0].questflags & QF_QUEST25) // Destroyed converter?
            GiveVoiceObjective("VOC106", "LOG106", 0);
        else
            GiveVoiceObjective("VOC79", "LOG79", 0);
        break;

    case MT_SPECTRE_E:
        P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_LOREMASTER);
        if(!netgame)
        {
            P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_STAMINA);
            P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_NEW_ACCURACY);
        }
        if(players[0].sigiltype == 4)
            GiveVoiceObjective("VOC85", "LOG85", 0);
        else
            GiveVoiceObjective("VOC83", "LOG83", 0);
        junk.tag = 666;
        EV_DoFloor(&junk, lowerFloorToLowest);
        break;

    case MT_SUBENTITY:
        F_StartFinale();
        break;

    case MT_PROGRAMMER:
        F_StartFinale();
        G_ExitLevel(0);
        break;

    default:
        // Real classy, Rogue.
        if(actor->type)
            I_Error("Error: Unconnected BossDeath id %d", actor->type);
        break;
    }
}

//
// A_AcolyteSpecial
//
// villsa [STRIFE] - new codepointer
// Awards quest #7 when all the Blue Acolytes are killed in Tarnhill
//
void A_AcolyteSpecial(mobj_t* actor)
{
    int i;
    thinker_t* th;

    if(actor->type != MT_GUARD8)
        return; // must be MT_GUARD8

    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i] && players[i].health > 0)
            break;
    }

    if(i == 8)
        return;

    for(th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if(th->function.acp1 == (actionf_p1) P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;

            // Found a living MT_GUARD8?
            if(mo != actor && mo->type == actor->type && mo->health > 0)
                return;
        }
    }

    // All MT_GUARD8 are dead, give quest token #7 to all players
    for(i = 0; i < MAXPLAYERS; i++)
        P_GiveItemToPlayer(&players[i], SPR_TOKN, MT_TOKEN_QUEST7);

    // play voice, give objective
    GiveVoiceObjective("VOC14", "LOG14", 0);
}

//
// A_InqChase
// villsa [STRIFE] - new codepointer
//
void A_InqChase(mobj_t* actor)
{
    S_StartSound(actor, sfx_inqact);
    A_Chase(actor);
}

//
// A_StalkerChase
// villsa [STRIFE] - new codepointer
//
void A_StalkerChase(mobj_t* actor)
{
    S_StartSound(actor, sfx_spdwlk);
    A_Chase(actor);
}

//
// A_PlayerScream
//
// [STRIFE]
// * Modified to eliminate gamemode check and to use Strife sound.
//
void A_PlayerScream (mobj_t* mo)
{
    // Default death sound.
    int sound = sfx_pldeth;

    // villsa [STRIFE] don't check for gamemode
    if(mo->health < -50)
    {
        // IF THE PLAYER DIES
        // LESS THAN -50% WITHOUT GIBBING
        sound = sfx_plxdth;  // villsa [STRIFE] different sound
    }

    S_StartSound (mo, sound);
}

//
// A_TeleportBeacon
//
// villsa [STRIFE] - new codepointer
//
void A_TeleportBeacon(mobj_t* actor)
{
    mobj_t* mobj;
    mobj_t* fog;
    fixed_t fog_x;
    fixed_t fog_y;

    if(actor->target != players[actor->miscdata].mo)
        actor->target = players[actor->miscdata].mo;

    mobj = P_SpawnMobj(actor->x, actor->y, ONFLOORZ, MT_REBEL1);

    // haleyjd 20141024: missing code from disassembly; transfer allegiance
    // originally from master player to the rebel.
    mobj->miscdata = actor->miscdata;

    if(!P_TryMove(mobj, mobj->x, mobj->y))
    {
        // Rebel is probably stuck in something.. too bad
        P_RemoveMobj(mobj);
        return;
    }

    // beacon no longer special
    actor->flags &= ~MF_SPECIAL;

    // 20160306: set rebel threshold
    mobj->threshold = 100;

    // set rebel color and flags
    mobj->flags |= ((actor->miscdata << MF_TRANSSHIFT) | MF_NODIALOG);
    mobj->target = NULL;

    // double Rebel's health in deathmatch mode
    if(deathmatch)
        mobj->health <<= 1;

    if(actor->target)
    {
        mobj_t* targ = actor->target->target;

        if(targ)
        {
            if(targ->type != MT_REBEL1 || targ->miscdata != mobj->miscdata)
                mobj->target = targ;
        }
    }

    P_SetMobjState(mobj, mobj->info->seestate);
    mobj->angle = actor->angle;

    fog_x = mobj->x + FixedMul(20*FRACUNIT, finecosine[actor->angle>>ANGLETOFINESHIFT]);
    fog_y = mobj->y + FixedMul(20*FRACUNIT, finesine[actor->angle>>ANGLETOFINESHIFT]);

    fog = P_SpawnMobj(fog_x, fog_y, mobj->z, MT_TFOG);
    S_StartSound(fog, sfx_telept);

    if(--actor->health < 0)
        P_RemoveMobj(actor);
}

//
// A_BodyParts
//
// villsa [STRIFE] new codepointer
// 09/06/10: Spawns gibs when organic actors get splattered, or junk
// when robots explode.
//
void A_BodyParts(mobj_t* actor)
{
    mobjtype_t type;
    mobj_t* mo;
    angle_t an;

    if(actor->flags & MF_NOBLOOD) // Robots are flagged NOBLOOD
        type = MT_JUNK;
    else
        type = MT_MEAT;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + (24*FRACUNIT), type);
    P_SetMobjState(mo, mo->info->spawnstate + (P_Random() % 19));

    an = (P_Random() << 13) / 255;
    mo->angle = an << ANGLETOFINESHIFT;

    mo->momx = FixedMul(finecosine[an], (P_Random() & 0x0f) << FRACBITS);
    mo->momy = FixedMul(finesine[an], (P_Random() & 0x0f) << FRACBITS);
    mo->momz = (P_Random() & 0x0f) << FRACBITS;
}

//
// A_ClaxonBlare
//
// [STRIFE] New function
// haleyjd 09/08/10: The ever-dreadful Strife alarm!
//
void A_ClaxonBlare(mobj_t* actor)
{
    // Timer ran down?
    if(--actor->reactiontime < 0)
    {
        // reset to initial state
        actor->target = NULL;
        actor->reactiontime = actor->info->reactiontime;

        // listen for more noise
        A_Listen(actor);
        
        // If we heard something, stay on for a while,
        // otherwise return to spawnstate.
        if(actor->target)
            actor->reactiontime = 50;
        else
            P_SetMobjState(actor, actor->info->spawnstate);
    }

    // When almost ran down, clear the soundtarget so it doesn't
    // retrigger the alarm.
    // Also, play the harsh, grating claxon.
    if(actor->reactiontime == 2)
        actor->subsector->sector->soundtarget = NULL;
    else if(actor->reactiontime > 50)
        S_StartSound(actor, sfx_alarm);
}

//
// A_ActiveSound
//
// villsa [STRIFE] new codepointer
// 09/06/10: Plays an object's active sound periodically.
//
void A_ActiveSound(mobj_t* actor)
{
    if(actor->info->activesound)
    {
        if(!(leveltime & 7)) // haleyjd: added parens
            S_StartSound(actor, actor->info->activesound);
    }
}

//
// A_ClearSoundTarget
//
// villsa [STRIFE] new codepointer
// 09/06/10: Clears the actor's sector soundtarget, so that the actor
// will not be continually alerted/awakened ad infinitum. Used by
// shopkeepers.
//
void A_ClearSoundTarget(mobj_t* actor)
{
    actor->subsector->sector->soundtarget = NULL;
}

//
// A_DropBurnFlesh
//
// villsa [STRIFE] new codepointer
//
void A_DropBurnFlesh(mobj_t* actor)
{
    mobj_t* mo;
    mobjtype_t type;

    type = actor->type;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z + (24*FRACUNIT), MT_BURNDROP);
    mo->momz = -FRACUNIT;

    actor->type = MT_SFIREBALL;
    P_RadiusAttack(actor, actor, 64);
    actor->type = type;
}

//
// A_FlameDeath
//
// villsa [STRIFE] new codepointer
// 09/06/10: Death animation for flamethrower fireballs.
//
void A_FlameDeath(mobj_t* actor)
{
    actor->flags |= MF_NOGRAVITY;
    actor->momz = (P_Random() & 3) << FRACBITS;
}

//
// A_ClearForceField
//
// villsa [STRIFE] new codepointer
// check for all matching lines in the sector
// and disable blocking/midtextures
//
void A_ClearForceField(mobj_t* actor)
{
    int i;
    sector_t *sec;
    line_t *secline;

    actor->flags &= ~(MF_SOLID|MF_SPECIAL);
    sec = actor->subsector->sector;

    if(!sec->linecount)
        return;

    for(i = 0; i < sec->linecount; i++)
    {
        secline = sec->lines[i];
        // BUG: will crash if 1S line has TWOSIDED flag!
        if(!(secline->flags & ML_TWOSIDED))
            continue;
        if(secline->special != 148)
            continue;

        secline->flags &= ~ML_BLOCKING;
        secline->special = 0;
        sides[secline->sidenum[0]].midtexture = 0;
        sides[secline->sidenum[1]].midtexture = 0;
    }
}

