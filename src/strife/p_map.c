//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard, Andrey Budko
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
//	Movement, collision handling.
//	Shooting and aiming.
//

#include <stdio.h>
#include <stdlib.h>

#include "deh_misc.h"

#include "m_bbox.h"
#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "m_argv.h"
#include "m_misc.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"
// Data.
#include "sounds.h"

// Spechit overrun magic value.
//
// This is the value used by PrBoom-plus.  I think the value below is 
// actually better and works with more demos.  However, I think
// it's better for the spechits emulation to be compatible with
// PrBoom-plus, at least so that the big spechits emulation list
// on Doomworld can also be used with Chocolate Doom.

#define DEFAULT_SPECHIT_MAGIC 0x01C09C98

// This is from a post by myk on the Doomworld forums, 
// outputted from entryway's spechit_magic generator for
// s205n546.lmp.  The _exact_ value of this isn't too
// important; as long as it is in the right general
// range, it will usually work.  Otherwise, we can use
// the generator (hacked doom2.exe) and provide it 
// with -spechit.

//#define DEFAULT_SPECHIT_MAGIC 0x84f968e8


fixed_t     tmbbox[4];
mobj_t*     tmthing;
int         tmflags;
fixed_t     tmx;
fixed_t     tmy;


// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean     floatok;

fixed_t     tmfloorz;
fixed_t     tmceilingz;
fixed_t     tmdropoffz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t*     ceilingline;

// haleyjd 20110203: [STRIFE] New global
// "blockingline" tracks the linedef responsible for blocking motion of an mobj
// for purposes of doing impact special activation by missiles. Suspiciously 
// similar to the solution used by Raven in Heretic and Hexen.
line_t     *blockingline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// fraggle: I have increased the size of this buffer.  In the original Doom,
// overrunning past this limit caused other bits of memory to be overwritten,
// affecting demo playback.  However, in doing so, the limit was still 
// exceeded.  So we have to support more than 8 specials.
//
// We keep the original limit, to detect what variables in memory were 
// overwritten (see SpechitOverrun())

#define MAXSPECIALCROSS 		20
#define MAXSPECIALCROSS_ORIGINAL	8

line_t*		spechit[MAXSPECIALCROSS];
int		numspechit;



//
// TELEPORT MOVE
// 

//
// PIT_StompThing
//
// [STRIFE] haleyjd 09/15/10: Modified so monsters can telestomp.
//
boolean PIT_StompThing (mobj_t* thing)
{
    fixed_t blockdist;

    if (!(thing->flags & MF_SHOOTABLE) )
        return true;

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist
        || abs(thing->y - tmy) >= blockdist )
    {
        // didn't hit it
        return true;
    }

    // don't clip against self
    if (thing == tmthing)
        return true;

    // [STRIFE] monsters DO stomp things, on all levels
    // Basically, one thing involved must be a player.
    // Monsters can telefrag players, and players can telefrag monsters, but
    // monsters cannot telefrag other monsters.
    if (!(tmthing->player || thing->player))
        return false;

    P_DamageMobj (thing, tmthing, tmthing, 10000);

    return true;
}


//
// P_TeleportMove
//
// [STRIFE]
// haleyjd 09/15/10: Modified to set thing z position.
//
boolean P_TeleportMove(mobj_t*  thing, fixed_t  x, fixed_t  y)
{
    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;
    int                 bx;
    int                 by;
    
    subsector_t*        newsubsec;
    
    // kill anything occupying the position
    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = NULL;
    
    // The base floor/ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;
    
    // stomp on any things contacted
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
                return false;
    
    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;
    thing->z = tmfloorz; // haleyjd 09/15/10: [STRIFE] Rogue added a z-set here

    P_SetThingPosition (thing);

    return true;
}


//
// MOVEMENT ITERATOR FUNCTIONS
//

static void SpechitOverrun(line_t *ld);

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
boolean PIT_CheckLine (line_t* ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
        return true;

    if (P_BoxOnLineSide (tmbbox, ld) != -1)
        return true;

    // A line has been hit
    
    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.
    
    if (!ld->backsector)
        return false;           // one sided line

    if (!(tmthing->flags & MF_MISSILE) )
    {
        // villsa [STRIFE] include jumpover flag
        if ( ld->flags & ML_BLOCKING &&
            (!(ld->flags & ML_JUMPOVER) || tmfloorz + (32*FRACUNIT) > tmthing->z) )
            return false;   // explicitly blocking everything

        // villsa [STRIFE] exclude floaters from blockmonster lines
        if ( !tmthing->player && (ld->flags & ML_BLOCKMONSTERS) &&
            !(tmthing->flags & MF_FLOAT))
            return false;   // block monsters only

        // villsa [STRIFE]
        if ( ld->flags & ML_BLOCKFLOATERS && tmthing->flags & MF_FLOAT )
            return false;   // block floaters only
    }

    // set openrange, opentop, openbottom
    P_LineOpening (ld);	

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmceilingz = opentop;
        ceilingline = ld;
    }

    if (openbottom > tmfloorz)
        tmfloorz = openbottom;	

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    // if contacted a special line, add it to the list
    if (ld->special)
    {
        spechit[numspechit] = ld;
        numspechit++;

        // fraggle: spechits overrun emulation code from prboom-plus
        if (numspechit > MAXSPECIALCROSS_ORIGINAL)
        {
            SpechitOverrun(ld);
        }
    }

    return true;
}

//
// PIT_CheckThing
//
boolean PIT_CheckThing (mobj_t* thing)
{
    fixed_t             blockdist;
    boolean             solid;
    int                 damage;

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
        return true;

    // don't clip against self
    if (thing == tmthing)
        return true;

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist
        || abs(thing->y - tmy) >= blockdist )
    {
        // didn't hit it
        return true;	
    }

    // villsa [STRIFE] see if it went over / under
    if(thing->height + thing->z < tmthing->z)
        return true;    // overhead

    // villsa [STRIFE] see if it went over / under
    if (tmthing->z + tmthing->height < thing->z)
        return true;    // underneath

    // villsa [STRIFE] unused
    // check for skulls slamming into things (removed)

    // missiles can hit other things
    if (tmthing->flags & MF_MISSILE)
    {
        // villsa [STRIFE] code to check over/under clipping moved to the beginning of the 
        // function, so that it applies to everything.

        // villsa [STRIFE] updated to strife version
        if (tmthing->target && (tmthing->target->type == thing->type))
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
                return true;

            // sdh: Add deh_species_infighting here.  We can override the
            // "monsters of the same species cant hurt each other" behavior
            // through dehacked patches

            if (thing->type != MT_PLAYER && !deh_species_infighting)
            {
                // Explode, but do no damage.
                // Let players missile other players.
                return false;
            }
        }

        if (!(thing->flags & MF_SHOOTABLE))
        {
            // didn't do any damage
            return !(thing->flags & MF_SOLID);
        }

        // haleyjd 09/01/10: [STRIFE] Spectral check:
        // Missiles cannot hit SPECTRAL entities unless the missiles are also
        // flagged as SPECTRAL.
        if (thing->flags & MF_SPECTRAL && !(tmthing->flags & MF_SPECTRAL))
            return true; // keep going

        // damage / explode
        // haleyjd 09/01/10: [STRIFE] Modified missile damage formula
        damage = ((P_Random()&3)+1)*tmthing->info->damage;
        P_DamageMobj (thing, tmthing, tmthing->target, damage);

        // don't traverse any more
        return false;
    }
    
    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        solid = (thing->flags & MF_SOLID) != 0;
        if (tmthing->player) // villsa [STRIFE] no longer checks MF_PICKUP flag
        {
            // can remove thing
            P_TouchSpecialThing (thing, tmthing);
        }
        return !solid;
    }

    return !(thing->flags & MF_SOLID);
}


//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
// 
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//
// haleyjd 20110203:
// [STRIFE] Modified to clear blockingline in advance of P_BlockLinesIterator
//
boolean
P_CheckPosition
( mobj_t*   thing,
  fixed_t   x,
  fixed_t   y )
{
    int             xl;
    int             xh;
    int             yl;
    int             yh;
    int             bx;
    int             by;
    subsector_t*    newsubsec;

    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    
    // [STRIFE] clear blockingline (see P_XYMovement, P_BlockLinesIterator)
    blockingline = NULL;
    ceilingline  = NULL;
    
    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    if ( tmflags & MF_NOCLIP )
        return true;
    
    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
                return false;
    
    // check lines
    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
                return false;

    return true;
}


//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
boolean
P_TryMove
( mobj_t*	thing,
  fixed_t	x,
  fixed_t	y )
{
    fixed_t oldx;
    fixed_t oldy;
    int     side;
    int     oldside;
    line_t* ld;

    floatok = false;
    if (!P_CheckPosition (thing, x, y))
        return false;       // solid wall or thing

    if ( !(thing->flags & MF_NOCLIP) )
    {
        if (tmceilingz - tmfloorz < thing->height)
            return false;   // doesn't fit

        floatok = true;

        // villsa [STRIFE] Removed MF_TELEPORT
        if (tmceilingz - thing->z < thing->height)
            return false;       // mobj must lower itself to fit

        // villsa [STRIFE] non-robots are limited to 16 unit step height
        if ((thing->flags & MF_NOBLOOD) == 0 && tmfloorz - thing->z > (16*FRACUNIT))
            return false;
        if (tmfloorz - thing->z > 24*FRACUNIT)
            return false;       // too big a step up

        // villsa [STRIFE] special case for missiles
        if((thing->flags & MF_MISSILE) && tmfloorz - thing->z > 4*FRACUNIT)
            return false;

        // haleyjd 20110204 [STRIFE]: dropoff height changed 24 -> 32
        if ( !(thing->flags&(MF_DROPOFF|MF_FLOAT))
            && tmfloorz - tmdropoffz > 32*FRACUNIT ) 
            return false;       // don't stand over a dropoff
    }
    
    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);
    
    // if any special lines were hit, do the effect
    if (! (thing->flags&MF_NOCLIP) ) // villsa [STRIFE] MF_TELEPORT not used
    {
        while (numspechit--)
        {
            // see if the line was crossed
            ld = spechit[numspechit];
            side = P_PointOnLineSide (thing->x, thing->y, ld);
            oldside = P_PointOnLineSide (oldx, oldy, ld);
            if (side != oldside)
            {
                if (ld->special)
                    P_CrossSpecialLine (ld-lines, oldside, thing);
            }
        }
    }

    return true;
}

//
// P_CheckPositionZ
//
// villsa [STRIFE] new function
// Check colliding things on top of one another; ie., 3D Object Clipping
//
boolean P_CheckPositionZ(mobj_t* thing, fixed_t height)
{
    fixed_t         x;
    fixed_t         y;
    fixed_t         z;
    int             xl;
    int             xh;
    int             yl;
    int             yh;
    int             bx;
    int             by;
    subsector_t*    newsubsec;

    x = thing->x;
    y = thing->y;
    z = thing->z;

    thing->z = height;

    tmthing = thing;
    tmflags = thing->flags;
    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    ceilingline = 0;
    newsubsec = thing->subsector;

    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.

    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    if(tmflags & MF_NOCLIP)
        return true;

    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.

    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    for(bx = xl; bx <= xh; bx++)
    {
        for(by = yl; by <= yh; by++)
        {
            if(!P_BlockThingsIterator(bx, by, PIT_CheckThing))
            {
                tmthing->z = z;
                return false;
            }
        }
    }

    return true;
}


//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
// [STRIFE] Verified unmodified
//
boolean P_ThingHeightClip (mobj_t* thing)
{
    boolean     onfloor;

    onfloor = (thing->z == thing->floorz);

    P_CheckPosition (thing, thing->x, thing->y);
    // what about stranding a monster partially off an edge?

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;

    if (onfloor)
    {
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    }
    else
    {
        // don't adjust a floating monster unless forced to
        if (thing->z+thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }

    if (thing->ceilingz - thing->floorz < thing->height)
        return false;

    return true;
}



//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t		bestslidefrac;
fixed_t		secondslidefrac;

line_t*		bestslideline;
line_t*		secondslideline;

mobj_t*		slidemo;

fixed_t		tmxmove;
fixed_t		tmymove;



//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
// [STRIFE] Verified unmodified
//
void P_HitSlideLine (line_t* ld)
{
    int         side;

    angle_t     lineangle;
    angle_t     moveangle;
    angle_t     deltaangle;

    fixed_t     movelen;
    fixed_t     newlen;


    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);

    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);
    deltaangle = moveangle-lineangle;

    if (deltaangle > ANG180)
        deltaangle += ANG180;
    //  I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance (tmxmove, tmymove);
    newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (newlen, finecosine[lineangle]);	
    tmymove = FixedMul (newlen, finesine[lineangle]);	
}


//
// PTR_SlideTraverse
//
// [STRIFE] Modified for smaller step-up height
//
boolean PTR_SlideTraverse (intercept_t* in)
{
    line_t* li;

    if (!in->isaline)
        I_Error ("PTR_SlideTraverse: not a line?");

    li = in->d.line;

    if ( ! (li->flags & ML_TWOSIDED) )
    {
        if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
        {
            // don't hit the back side
            return true;
        }
        goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_LineOpening (li);

    if (openrange < slidemo->height)
        goto isblocking;        // doesn't fit

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;        // mobj is too high

    // villsa [STRIFE] change from 24 to 16
    if (openbottom - slidemo->z > 16*FRACUNIT )
        goto isblocking;        // too big a step up

    // this line doesn't block movement
    return true;

    // the line does block movement,
    // see if it is closer than best so far
isblocking:
    if (in->frac < bestslidefrac)
    {
        secondslidefrac = bestslidefrac;
        secondslideline = bestslideline;
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;   // stop
}



//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
// [STRIFE] Verified unmodified
//
void P_SlideMove (mobj_t* mo)
{
    fixed_t     leadx;
    fixed_t     leady;
    fixed_t     trailx;
    fixed_t     traily;
    fixed_t     newx;
    fixed_t     newy;
    int         hitcount;

    slidemo = mo;
    hitcount = 0;
    
retry:
    if (++hitcount == 3)
        goto stairstep;     // don't loop forever

    // trace along the three leading corners
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT+1;

    P_PathTraverse ( leadx, leady, leadx+mo->momx, leady+mo->momy,
                     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( trailx, leady, trailx+mo->momx, leady+mo->momy,
                     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( leadx, traily, leadx+mo->momx, traily+mo->momy,
                     PT_ADDLINES, PTR_SlideTraverse );
    
    // move up to the wall
    if (bestslidefrac == FRACUNIT+1)
    {
        // the move most have hit the middle, so stairstep
stairstep:
        if (!P_TryMove (mo, mo->x, mo->y + mo->momy))
            P_TryMove (mo, mo->x + mo->momx, mo->y);
        return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if (bestslidefrac > 0)
    {
        newx = FixedMul (mo->momx, bestslidefrac);
        newy = FixedMul (mo->momy, bestslidefrac);

        if (!P_TryMove (mo, mo->x+newx, mo->y+newy))
            goto stairstep;
    }
    
    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT-(bestslidefrac+0x800);

    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;

    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul (mo->momx, bestslidefrac);
    tmymove = FixedMul (mo->momy, bestslidefrac);

    P_HitSlideLine (bestslideline); // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove))
    {
        goto retry;
    }
}


//
// P_LineAttack
//
mobj_t*		linetarget;	// who got hit (or NULL)
mobj_t*		shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t		shootz;	

int		la_damage;
fixed_t		attackrange;

fixed_t		aimslope;

// slopes to top and bottom of target
extern fixed_t	topslope;
extern fixed_t	bottomslope;	


//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
// [STRIFE] Verified unmodified
//
boolean
PTR_AimTraverse (intercept_t* in)
{
    line_t*     li;
    mobj_t*     th;
    fixed_t     slope;
    fixed_t     thingtopslope;
    fixed_t     thingbottomslope;
    fixed_t     dist;

    if (in->isaline)
    {
        li = in->d.line;

        if ( !(li->flags & ML_TWOSIDED) )
            return false;       // stop

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.
        P_LineOpening (li);

        if (openbottom >= opentop)
            return false;       // stop

        dist = FixedMul (attackrange, in->frac);

        // Return false if there is no back sector.  This should never
        // be the case if the line is two-sided; however, some WADs
        // (eg. ottawau.wad) use this as an "impassible glass" trick
        // and rely on Vanilla Doom's (unintentional) support for this.

        if (li->backsector == NULL)
        {
            return false;
        }

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv (openbottom - shootz , dist);
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv (opentop - shootz , dist);
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;       // stop

        return true;            // shot continues
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true;            // can't shoot self

    if (!(th->flags&MF_SHOOTABLE))
        return true;            // corpse or something

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < bottomslope)
        return true;            // shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true;            // shot under the thing

    // this thing can be hit!
    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope+thingbottomslope)/2;
    linetarget = th;

    return false;               // don't go any farther
}


//
// PTR_ShootTraverse
//
// [STRIFE] Changes for Spectres and Mauler puff/damage inflictor
//
boolean PTR_ShootTraverse (intercept_t* in)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    fixed_t             frac;

    line_t*             li;

    mobj_t*             th;
    mobj_t*             th2;    // villsa [STRIFE]

    fixed_t             slope;
    fixed_t             dist;
    fixed_t             thingtopslope;
    fixed_t             thingbottomslope;

    if (in->isaline)
    {
        li = in->d.line;

        if (li->special)
            P_ShootSpecialLine (shootthing, li);

        if ( !(li->flags & ML_TWOSIDED) )
            goto hitline;

        // crosses a two sided line
        P_LineOpening (li);

        dist = FixedMul (attackrange, in->frac);

        // Check if backsector is NULL.  See comment in PTR_AimTraverse.

        if (li->backsector == NULL)
        {
            goto hitline;
        }

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv (openbottom - shootz , dist);
            if (slope > aimslope)
                goto hitline;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv (opentop - shootz , dist);
            if (slope < aimslope)
                goto hitline;
        }

        // shot continues
        return true;


        // hit line
hitline:
        // position a bit closer
        frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
        x = trace.x + FixedMul (trace.dx, frac);
        y = trace.y + FixedMul (trace.dy, frac);
        z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            // don't shoot the sky!
            if (z > li->frontsector->ceilingheight)
                return false;

            // it's a sky hack wall
            if	(li->backsector && li->backsector->ceilingpic == skyflatnum)
                return false;
        }

        // villsa [STRIFE]
        if(la_damage > 0)
        {
            // villsa [STRIFE] Test against Mauler attack range
            if(attackrange != 2112*FRACUNIT)
                P_SpawnPuff(x, y, z); // Spawn bullet puffs.
            else
                P_SpawnMobj(x, y, z, MT_STRIFEPUFF3);
        }
        
        // don't go any farther
        return false;	
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true;        // can't shoot self

    if (!(th->flags&MF_SHOOTABLE))
        return true;        // corpse or something

    // haleyjd 09/18/10: [STRIFE] Corrected - not MVIS, but SPECTRAL.
    if(th->flags & MF_SPECTRAL)
        return true;        // is a spectral entity

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < aimslope)
        return true;        // shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > aimslope)
        return true;        // shot under the thing

    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);

    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);
    z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

    // villsa [STRIFE] Check for Mauler attack range
    if(attackrange == 2112*FRACUNIT)
    {
        th2 = P_SpawnMobj(x, y, z, MT_STRIFEPUFF3);
        th2->momz = -FRACUNIT;
        P_DamageMobj(th, th2, shootthing, la_damage);
        return false;
    }

    // villsa [STRIFE] disabled check for damage
    //if (la_damage)
    P_DamageMobj (th, shootthing, shootthing, la_damage);

    // Spawn bullet puffs or blod spots,
    // depending on target type.
    if (in->d.thing->flags & MF_NOBLOOD)
        P_SpawnSparkPuff(x, y, z);  // villsa [STRIFE] call spark puff function instead
    else
        P_SpawnBlood (x,y,z, la_damage);

    // don't go any farther
    return false;

}


//
// P_AimLineAttack
//
// [STRIFE] Modified to support player->pitch
//
fixed_t
P_AimLineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance )
{
    fixed_t     x2;
    fixed_t     y2;

    t1 = P_SubstNullMobj(t1);

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    
    x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
    y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

    // can't shoot outside view angles
    topslope = 100*FRACUNIT/160;	
    bottomslope = -100*FRACUNIT/160;
    
    attackrange = distance;
    linetarget = NULL;

    P_PathTraverse ( t1->x, t1->y,
                     x2, y2,
                     PT_ADDLINES|PT_ADDTHINGS,
                     PTR_AimTraverse );

    if (linetarget)
        return aimslope;
    else    // villsa [STRIFE] checks for player pitch
    {
        if(t1->player)
            return (t1->player->pitch << FRACBITS) / 160;
    }

    return 0;
}
 

//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//
// [STRIFE] Modified to check lines only if damage <= 0 (see P_RadiusAttack)
//
void
P_LineAttack
( mobj_t*       t1,
  angle_t       angle,
  fixed_t       distance,
  fixed_t       slope,
  int	        damage )
{
    fixed_t     x2;
    fixed_t     y2;
    int         traverseflags;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
    y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
    attackrange = distance;
    aimslope = slope;

    // villsa [STRIFE] test lines only if damage is <= 0
    if(damage >= 1)
        traverseflags = (PT_ADDLINES|PT_ADDTHINGS);
    else
        traverseflags = PT_ADDLINES;

    P_PathTraverse(t1->x, t1->y,
                   x2, y2,
                   traverseflags,
                   PTR_ShootTraverse);
}
 


//
// USE LINES
//
// [STRIFE] Verified unmodified
//
mobj_t*     usething;

boolean PTR_UseTraverse (intercept_t* in)
{
    int     side;

    if (!in->d.line->special)
    {
        P_LineOpening (in->d.line);
        if (openrange <= 0)
        {
            S_StartSound (usething, sfx_noway);

            // can't use through a wall
            return false;
        }
        // not a special line, but keep checking
        return true ;
    }

    side = 0;
    if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
        side = 1;

    //  return false;       // don't use back side

    P_UseSpecialLine (usething, in->d.line, side);

    // can't use for than one special line in a row
    return false;
}


//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
// [STRIFE] Verified unmodified
//
void P_UseLines (player_t*	player)
{
    int     angle;
    fixed_t x1;
    fixed_t y1;
    fixed_t x2;
    fixed_t y2;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
    y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

    P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
}


//
// RADIUS ATTACK
//
mobj_t*     bombsource;
mobj_t*     bombspot;
int         bombdamage;


//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
// [STRIFE] Modified for Spectral and Inquisitor exclusions
//
boolean PIT_RadiusAttack (mobj_t* thing)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t dist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    // haleyjd 10/04/10: Spectrals are not damaged by blast radii
    if(thing->flags & MF_SPECTRAL)
        return true;

    // Boss spider and cyborg
    // take no damage from concussion.
    // villsa [STRIFE] unused
    // - haleyjd: INQUISITOR

    if(thing->type == MT_INQUISITOR)
        return true;

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);

    dist = dx>dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist >= bombdamage)
        return true;        // out of range

    if ( P_CheckSight (thing, bombspot) )
    {
        // must be in direct path
        P_DamageMobj (thing, bombspot, bombsource, bombdamage - dist);
    }

    return true;
}


//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
// [STRIFE] Modified to emit "test" tracers which can shatter glass screens
//          and windows.
//
void
P_RadiusAttack
( mobj_t*       spot,
  mobj_t*       source,
  int           damage )
{
    int         x;
    int         y;
    
    int         xl;
    int         xh;
    int         yl;
    int         yh;
    
    fixed_t     dist;

    dist = (damage+MAXRADIUS)<<FRACBITS;
    yh = (spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = spot;
    bombsource = source;
    bombdamage = damage;

    for (y=yl ; y<=yh ; y++)
        for (x=xl ; x<=xh ; x++)
            P_BlockThingsIterator (x, y, PIT_RadiusAttack );

    // villsa [STRIFE] Send out 0 damage tracers to shatter nearby glass.
    spot->z += 32*FRACUNIT;
    P_LineAttack(spot, 0,      dist, 1, 0);
    P_LineAttack(spot, ANG90,  dist, 1, 0);
    P_LineAttack(spot, ANG180, dist, 1, 0);
    P_LineAttack(spot, ANG270, dist, 1, 0);
    spot->z -= 32*FRACUNIT;
}



//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//
boolean		crushchange;
boolean		nofit;


//
// PIT_ChangeSector
//
// [STRIFE] Changes to crushing behavior
//
boolean PIT_ChangeSector (mobj_t*   thing)
{
    mobj_t*	mo;

    if (P_ThingHeightClip (thing))
    {
        // keep checking
        return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        // villsa [STRIFE] do something with the player
        if(thing->player && thing->subsector->sector->specialdata)
        {
            nofit = true;
            return false;
        }
        //P_SetMobjState (thing, S_GIBS);   // villsa [STRIFE] unused

        A_BodyParts(thing); // villsa [STRIFE] spit out meat/junk stuff
        thing->flags &= ~MF_SOLID;
        thing->height = 0;
        thing->radius = 0;

        // keep checking
        return true;
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj (thing);

        // keep checking
        return true;
    }

    if (! (thing->flags & MF_SHOOTABLE) )
    {
        // assume it is bloody gibs or something
        return true;
    }
    
    nofit = true;

    if (crushchange && !(leveltime&3) )
    {
        int t;
        S_StartSound(thing, sfx_pcrush);   // villsa [STRIFE]
        P_DamageMobj(thing,NULL,NULL,10);

        // spray blood in a random direction
        mo = P_SpawnMobj (thing->x,
                          thing->y,
                          thing->z + thing->height/2, MT_BLOOD_DEATH);

        t = P_Random();
        mo->momx = (t - P_Random ()) << 12;
        t = P_Random();
        mo->momy = (t - P_Random ()) << 12;
    }

    // keep checking (crush other things)
    return true;
}



//
// P_ChangeSector
//
// [STRIFE] Verified unmodified
//
boolean
P_ChangeSector
( sector_t* sector,
  boolean   crunch )
{
    int     x;
    int     y;

    nofit = false;
    crushchange = crunch;

    // re-check heights for all things near the moving sector
    for (x=sector->blockbox[BOXLEFT] ; x<= sector->blockbox[BOXRIGHT] ; x++)
        for (y=sector->blockbox[BOXBOTTOM];y<= sector->blockbox[BOXTOP] ; y++)
            P_BlockThingsIterator (x, y, PIT_ChangeSector);

    return nofit;
}

// Code to emulate the behavior of Vanilla Doom when encountering an overrun
// of the spechit array.  This is by Andrey Budko (e6y) and comes from his
// PrBoom plus port.  A big thanks to Andrey for this.

static void SpechitOverrun(line_t *ld)
{
    static unsigned int baseaddr = 0;
    unsigned int addr;
   
    if (baseaddr == 0)
    {
        int p;

        // This is the first time we have had an overrun.  Work out
        // what base address we are going to use.
        // Allow a spechit value to be specified on the command line.

        //!
        // @category compat
        // @arg <n>
        //
        // Use the specified magic value when emulating spechit overruns.
        //

        p = M_CheckParmWithArgs("-spechit", 1);
        
        if (p > 0)
        {
            M_StrToInt(myargv[p+1], (int *) &baseaddr);
        }
        else
        {
            baseaddr = DEFAULT_SPECHIT_MAGIC;
        }
    }
    
    // Calculate address used in doom2.exe

    addr = baseaddr + (ld - lines) * 0x3E;

    switch(numspechit)
    {
        case 9: 
        case 10:
        case 11:
        case 12:
            tmbbox[numspechit-9] = addr;
            break;
        case 13: 
            nofit = addr; // haleyjd 20110204: nofit/crushchange are in opposite 
            break;        // order in the Strife binary.
        case 14: 
            crushchange = addr; 
            break;
        default:
            fprintf(stderr, "SpechitOverrun: Warning: unable to emulate"
                            "an overrun where numspechit=%i\n",
                            numspechit);
            break;
    }
}

