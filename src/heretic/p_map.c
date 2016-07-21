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
// P_map.c

#include <stdlib.h>

#include "doomdef.h"
#include "i_system.h"
#include "m_bbox.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

/*
===============================================================================

NOTES:


===============================================================================
*/

/*
===============================================================================

mobj_t NOTES

mobj_ts are used to tell the refresh where to draw an image, tell the world
simulation when objects are contacted, and tell the sound driver how to
position a sound.

The refresh uses the next and prev links to follow lists of things in sectors
as they are being drawn.  The sprite, frame, and angle elements determine which
patch_t is used to draw the sprite if it is visible.  The sprite and frame
values are allmost allways set from state_t structures.  The statescr.exe
utility generates the states.h and states.c files that contain the sprite/frame
numbers from the statescr.txt source file.  The xyz origin point represents a
point at the bottom middle of the sprite (between the feet of a biped).  This
is the default origin position for patch_ts grabbed with lumpy.exe.  A walking
creature will have its z equal to the floor it is standing on.
 
The sound code uses the x,y, and subsector fields to do stereo positioning of
any sound effited by the mobj_t.

The play simulation uses the blocklinks, x,y,z, radius, height to determine
when mobj_ts are touching each other, touching lines in the map, or hit by
trace lines (gunshots, lines of sight, etc). The mobj_t->flags element has
various bit flags used by the simulation.


Every mobj_t is linked into a single sector based on it's origin coordinates.
The subsector_t is found with R_PointInSubsector(x,y), and the sector_t can be
found with subsector->sector.  The sector links are only used by the rendering
code,  the play simulation does not care about them at all.

Any mobj_t that needs to be acted upon be something else in the play world
(block movement, be shot, etc) will also need to be linked into the blockmap. 
If the thing has the MF_NOBLOCK flag set, it will not use the block links. It
can still interact with other things, but only as the instigator (missiles will
run into other things, but nothing can run into a missile).   Each block in
the grid is 128*128 units, and knows about every line_t that it contains a
piece of, and every interactable mobj_t that has it's origin contained.  

A valid mobj_t is a mobj_t that has the proper subsector_t filled in for it's
xy coordinates and is linked into the subsector's sector or has the MF_NOSECTOR
flag set (the subsector_t needs to be valid even if MF_NOSECTOR is set), and is
linked into a blockmap block or has the MF_NOBLOCKMAP flag set.  Links should
only be modified by the P_[Un]SetThingPosition () functions.  Do not change
the MF_NO? flags while a thing is valid.


===============================================================================
*/

fixed_t tmbbox[4];
mobj_t *tmthing;
int tmflags;
fixed_t tmx, tmy;

boolean floatok;                // if true, move would be ok if
                                                                        // within tmfloorz - tmceilingz

fixed_t tmfloorz, tmceilingz, tmdropoffz;

// keep track of the line that lowers the ceiling, so missiles don't explode
// against sky hack walls
line_t *ceilingline;

// keep track of special lines as they are hit, but don't process them
// until the move is proven valid
#define	MAXSPECIALCROSS		8
line_t *spechit[MAXSPECIALCROSS];
int numspechit;

mobj_t *onmobj;                 //generic global onmobj...used for landing on pods/players

/*
===============================================================================

					TELEPORT MOVE
 
===============================================================================
*/

/*
==================
=
= PIT_StompThing
=
==================
*/

boolean PIT_StompThing(mobj_t * thing)
{
    fixed_t blockdist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tmthing->radius;
    if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
        return true;            // didn't hit it

    if (thing == tmthing)
        return true;            // don't clip against self

    if (!(tmthing->flags2 & MF2_TELESTOMP))
    {                           // Not allowed to stomp things
        return (false);
    }

    P_DamageMobj(thing, tmthing, tmthing, 10000);

    return true;
}


/*
===================
=
= P_TeleportMove
=
===================
*/

boolean P_TeleportMove(mobj_t * thing, fixed_t x, fixed_t y)
{
    int xl, xh, yl, yh, bx, by;
    subsector_t *newsubsec;

//
// kill anything occupying the position
//

    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector(x, y);
    ceilingline = NULL;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

//
// stomp on any things contacted
//
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_StompThing))
                return false;

//
// the move is ok, so link the thing into its new position
//      
    P_UnsetThingPosition(thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    return true;
}

/*
===============================================================================

					MOVEMENT ITERATOR FUNCTIONS
 
===============================================================================
*/

/*
==================
=
= PIT_CheckLine
=
= Adjusts tmfloorz and tmceilingz as lines are contacted
==================
*/

boolean PIT_CheckLine(line_t * ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    {
        return (true);
    }
    if (P_BoxOnLineSide(tmbbox, ld) != -1)
    {
        return (true);
    }

// a line has been hit
/*
=
= The moving thing's destination position will cross the given line.
= If this should not be allowed, return false.
= If the line is special, keep track of it to process later if the move
= 	is proven ok.  NOTE: specials are NOT sorted by order, so two special lines
= 	that are only 8 pixels apart could be crossed in either order.
*/

    if (!ld->backsector)
    {                           // One sided line
        if (tmthing->flags & MF_MISSILE)
        {                       // Missiles can trigger impact specials
            if (ld->special)
            {
                spechit[numspechit] = ld;
                numspechit++;
            }
        }
        return false;
    }
    if (!(tmthing->flags & MF_MISSILE))
    {
        if (ld->flags & ML_BLOCKING)
        {                       // Explicitly blocking everything
            return (false);
        }
        if (!tmthing->player && ld->flags & ML_BLOCKMONSTERS
            && tmthing->type != MT_POD)
        {                       // Block monsters only
            return (false);
        }
    }
    P_LineOpening(ld);          // set openrange, opentop, openbottom
    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmceilingz = opentop;
        ceilingline = ld;
    }
    if (openbottom > tmfloorz)
    {
        tmfloorz = openbottom;
    }
    if (lowfloor < tmdropoffz)
    {
        tmdropoffz = lowfloor;
    }
    if (ld->special)
    {                           // Contacted a special line, add it to the list
        spechit[numspechit] = ld;
        numspechit++;
    }
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC PIT_CheckThing
//
//---------------------------------------------------------------------------

boolean PIT_CheckThing(mobj_t * thing)
{
    fixed_t blockdist;
    boolean solid;
    int damage;

    if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
    {                           // Can't hit thing
        return (true);
    }
    blockdist = thing->radius + tmthing->radius;
    if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
    {                           // Didn't hit thing
        return (true);
    }
    if (thing == tmthing)
    {                           // Don't clip against self
        return (true);
    }
    if (tmthing->flags2 & MF2_PASSMOBJ)
    {                           // check if a mobj passed over/under another object
        if ((tmthing->type == MT_IMP || tmthing->type == MT_WIZARD)
            && (thing->type == MT_IMP || thing->type == MT_WIZARD))
        {                       // don't let imps/wizards fly over other imps/wizards
            return false;
        }
        if (tmthing->z > thing->z + thing->height
            && !(thing->flags & MF_SPECIAL))
        {
            return (true);
        }
        else if (tmthing->z + tmthing->height < thing->z
                 && !(thing->flags & MF_SPECIAL))
        {                       // under thing
            return (true);
        }
    }
    // Check for skulls slamming into things
    if (tmthing->flags & MF_SKULLFLY)
    {
        damage = ((P_Random() % 8) + 1) * tmthing->damage;
        P_DamageMobj(thing, tmthing, tmthing, damage);
        tmthing->flags &= ~MF_SKULLFLY;
        tmthing->momx = tmthing->momy = tmthing->momz = 0;
        P_SetMobjState(tmthing, tmthing->info->seestate);
        return (false);
    }
    // Check for missile
    if (tmthing->flags & MF_MISSILE)
    {
        // Check for passing through a ghost
        if ((thing->flags & MF_SHADOW) && (tmthing->flags2 & MF2_THRUGHOST))
        {
            return (true);
        }
        // Check if it went over / under
        if (tmthing->z > thing->z + thing->height)
        {                       // Over thing
            return (true);
        }
        if (tmthing->z + tmthing->height < thing->z)
        {                       // Under thing
            return (true);
        }
        if (tmthing->target && tmthing->target->type == thing->type)
        {                       // Don't hit same species as originator
            if (thing == tmthing->target)
            {                   // Don't missile self
                return (true);
            }
            if (thing->type != MT_PLAYER)
            {                   // Hit same species as originator, explode, no damage
                return (false);
            }
        }
        if (!(thing->flags & MF_SHOOTABLE))
        {                       // Didn't do any damage
            return !(thing->flags & MF_SOLID);
        }
        if (tmthing->flags2 & MF2_RIP)
        {
            if (!(thing->flags & MF_NOBLOOD))
            {                   // Ok to spawn some blood
                P_RipperBlood(tmthing);
            }
            S_StartSound(tmthing, sfx_ripslop);
            damage = ((P_Random() & 3) + 2) * tmthing->damage;
            P_DamageMobj(thing, tmthing, tmthing->target, damage);
            if (thing->flags2 & MF2_PUSHABLE
                && !(tmthing->flags2 & MF2_CANNOTPUSH))
            {                   // Push thing
                thing->momx += tmthing->momx >> 2;
                thing->momy += tmthing->momy >> 2;
            }
            numspechit = 0;
            return (true);
        }
        // Do damage
        damage = ((P_Random() % 8) + 1) * tmthing->damage;
        if (damage)
        {
            if (!(thing->flags & MF_NOBLOOD) && P_Random() < 192)
            {
                P_BloodSplatter(tmthing->x, tmthing->y, tmthing->z, thing);
            }
            P_DamageMobj(thing, tmthing, tmthing->target, damage);
        }
        return (false);
    }
    if (thing->flags2 & MF2_PUSHABLE && !(tmthing->flags2 & MF2_CANNOTPUSH))
    {                           // Push thing
        thing->momx += tmthing->momx >> 2;
        thing->momy += tmthing->momy >> 2;
    }
    // Check for special thing
    if (thing->flags & MF_SPECIAL)
    {
        solid = (thing->flags & MF_SOLID) != 0;
        if (tmflags & MF_PICKUP)
        {                       // Can be picked up by tmthing
            P_TouchSpecialThing(thing, tmthing);        // Can remove thing
        }
        return (!solid);
    }
    return (!(thing->flags & MF_SOLID));
}

//---------------------------------------------------------------------------
//
// PIT_CheckOnmobjZ
//
//---------------------------------------------------------------------------

boolean PIT_CheckOnmobjZ(mobj_t * thing)
{
    fixed_t blockdist;

    if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
    {                           // Can't hit thing
        return (true);
    }
    blockdist = thing->radius + tmthing->radius;
    if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
    {                           // Didn't hit thing
        return (true);
    }
    if (thing == tmthing)
    {                           // Don't clip against self
        return (true);
    }
    if (tmthing->z > thing->z + thing->height)
    {
        return (true);
    }
    else if (tmthing->z + tmthing->height < thing->z)
    {                           // under thing
        return (true);
    }
    if (thing->flags & MF_SOLID)
    {
        onmobj = thing;
    }
    return (!(thing->flags & MF_SOLID));
}

/*
===============================================================================

 						MOVEMENT CLIPPING
 
===============================================================================
*/

//----------------------------------------------------------------------------
//
// FUNC P_TestMobjLocation
//
// Returns true if the mobj is not blocked by anything at its current
// location, otherwise returns false.
//
//----------------------------------------------------------------------------

boolean P_TestMobjLocation(mobj_t * mobj)
{
    int flags;

    flags = mobj->flags;
    mobj->flags &= ~MF_PICKUP;
    if (P_CheckPosition(mobj, mobj->x, mobj->y))
    {                           // XY is ok, now check Z
        mobj->flags = flags;
        if ((mobj->z < mobj->floorz)
            || (mobj->z + mobj->height > mobj->ceilingz))
        {                       // Bad Z
            return (false);
        }
        return (true);
    }
    mobj->flags = flags;
    return (false);
}

/*
==================
=
= P_CheckPosition
=
= This is purely informative, nothing is modified (except things picked up)

in:
a mobj_t (can be valid or invalid)
a position to be checked (doesn't need to be related to the mobj_t->x,y)

during:
special things are touched if MF_PICKUP
early out on solid lines?

out:
newsubsec
floorz
ceilingz
tmdropoffz		the lowest point contacted (monsters won't move to a dropoff)
speciallines[]
numspeciallines

==================
*/

boolean P_CheckPosition(mobj_t * thing, fixed_t x, fixed_t y)
{
    int xl, xh, yl, yh, bx, by;
    subsector_t *newsubsec;

    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector(x, y);
    ceilingline = NULL;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    if (tmflags & MF_NOCLIP)
        return true;

//
// check things first, possibly picking things up
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_CheckThing))
                return false;
//
// check lines
//
    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, PIT_CheckLine))
                return false;

    return true;
}

//=============================================================================
//
// P_CheckOnmobj(mobj_t *thing)
//
//              Checks if the new Z position is legal
//=============================================================================

mobj_t *P_CheckOnmobj(mobj_t * thing)
{
    int xl, xh, yl, yh, bx, by;
    subsector_t *newsubsec;
    fixed_t x;
    fixed_t y;
    mobj_t oldmo;

    x = thing->x;
    y = thing->y;
    tmthing = thing;
    tmflags = thing->flags;
    oldmo = *thing;             // save the old mobj before the fake zmovement
    P_FakeZMovement(tmthing);

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector(x, y);
    ceilingline = NULL;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    if (tmflags & MF_NOCLIP)
        return NULL;

//
// check things first, possibly picking things up
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_CheckOnmobjZ))
            {
                *tmthing = oldmo;
                return onmobj;
            }
    *tmthing = oldmo;
    return NULL;
}

//=============================================================================
//
// P_FakeZMovement
//
//              Fake the zmovement so that we can check if a move is legal
//=============================================================================

void P_FakeZMovement(mobj_t * mo)
{
    int dist;
    int delta;
//
// adjust height
//
    mo->z += mo->momz;
    if (mo->flags & MF_FLOAT && mo->target)
    {                           // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            dist =
                P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y);
            delta = (mo->target->z + (mo->height >> 1)) - mo->z;
            if (delta < 0 && dist < -(delta * 3))
                mo->z -= FLOATSPEED;
            else if (delta > 0 && dist < (delta * 3))
                mo->z += FLOATSPEED;
        }
    }
    if (mo->player && mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
        && leveltime & 2)
    {
        mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
    }

//
// clip movement
//
    if (mo->z <= mo->floorz)
    {                           // Hit the floor
        mo->z = mo->floorz;
        if (mo->momz < 0)
        {
            mo->momz = 0;
        }
        if (mo->flags & MF_SKULLFLY)
        {                       // The skull slammed into something
            mo->momz = -mo->momz;
        }
        if (mo->info->crashstate && (mo->flags & MF_CORPSE))
        {
            return;
        }
    }
    else if (mo->flags2 & MF2_LOGRAV)
    {
        if (mo->momz == 0)
            mo->momz = -(GRAVITY >> 3) * 2;
        else
            mo->momz -= GRAVITY >> 3;
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (mo->momz == 0)
            mo->momz = -GRAVITY * 2;
        else
            mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {                           // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
        if (mo->flags & MF_SKULLFLY)
        {                       // the skull slammed into something
            mo->momz = -mo->momz;
        }
    }
}

//==========================================================================
//
// CheckMissileImpact
//
//==========================================================================

void CheckMissileImpact(mobj_t * mobj)
{
    int i;

    if (!numspechit || !(mobj->flags & MF_MISSILE) || !mobj->target)
    {
        return;
    }
    if (!mobj->target->player)
    {
        return;
    }
    for (i = numspechit - 1; i >= 0; i--)
    {
        P_ShootSpecialLine(mobj->target, spechit[i]);
    }
}

/*
===================
=
= P_TryMove
=
= Attempt to move to a new position, crossing special lines unless MF_TELEPORT
= is set
=
===================
*/

boolean P_TryMove(mobj_t * thing, fixed_t x, fixed_t y)
{
    fixed_t oldx, oldy;
    int side, oldside;
    line_t *ld;

    floatok = false;
    if (!P_CheckPosition(thing, x, y))
    {                           // Solid wall or thing
        CheckMissileImpact(thing);
        return false;
    }
    if (!(thing->flags & MF_NOCLIP))
    {
        if (tmceilingz - tmfloorz < thing->height)
        {                       // Doesn't fit
            CheckMissileImpact(thing);
            return false;
        }
        floatok = true;
        if (!(thing->flags & MF_TELEPORT)
            && tmceilingz - thing->z < thing->height
            && !(thing->flags2 & MF2_FLY))
        {                       // mobj must lower itself to fit
            CheckMissileImpact(thing);
            return false;
        }
        if (thing->flags2 & MF2_FLY)
        {
            if (thing->z + thing->height > tmceilingz)
            {
                thing->momz = -8 * FRACUNIT;
                return false;
            }
            else if (thing->z < tmfloorz
                     && tmfloorz - tmdropoffz > 24 * FRACUNIT)
            {
                thing->momz = 8 * FRACUNIT;
                return false;
            }
        }
        if (!(thing->flags & MF_TELEPORT)
            // The Minotaur floor fire (MT_MNTRFX2) can step up any amount
            && thing->type != MT_MNTRFX2
            && tmfloorz - thing->z > 24 * FRACUNIT)
        {                       // Too big a step up
            CheckMissileImpact(thing);
            return false;
        }
        if ((thing->flags & MF_MISSILE) && tmfloorz > thing->z)
        {
            CheckMissileImpact(thing);
        }
        if (!(thing->flags & (MF_DROPOFF | MF_FLOAT))
            && tmfloorz - tmdropoffz > 24 * FRACUNIT)
        {                       // Can't move over a dropoff
            return false;
        }
    }

//
// the move is ok, so link the thing into its new position
//
    P_UnsetThingPosition(thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    if (thing->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(thing) != FLOOR_SOLID)
    {
        thing->flags2 |= MF2_FEETARECLIPPED;
    }
    else if (thing->flags2 & MF2_FEETARECLIPPED)
    {
        thing->flags2 &= ~MF2_FEETARECLIPPED;
    }

//
// if any special lines were hit, do the effect
//
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
        while (numspechit--)
        {
            // see if the line was crossed
            ld = spechit[numspechit];
            side = P_PointOnLineSide(thing->x, thing->y, ld);
            oldside = P_PointOnLineSide(oldx, oldy, ld);
            if (side != oldside)
            {
                if (ld->special)
                    P_CrossSpecialLine(ld - lines, oldside, thing);
            }
        }

    return true;
}

/*
==================
=
= P_ThingHeightClip
=
= Takes a valid thing and adjusts the thing->floorz, thing->ceilingz,
= anf possibly thing->z
=
= This is called for all nearby monsters whenever a sector changes height
=
= If the thing doesn't fit, the z will be set to the lowest value and
= false will be returned
==================
*/

boolean P_ThingHeightClip(mobj_t * thing)
{
    boolean onfloor;

    onfloor = (thing->z == thing->floorz);

    P_CheckPosition(thing, thing->x, thing->y);
    // what about stranding a monster partially off an edge?

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;

    if (onfloor)
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    else
    {                           // don't adjust a floating monster unless forced to
        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }

    if (thing->ceilingz - thing->floorz < thing->height)
        return false;

    return true;
}


/*
==============================================================================

							SLIDE MOVE

Allows the player to slide along any angled walls

==============================================================================
*/

fixed_t bestslidefrac, secondslidefrac;
line_t *bestslideline, *secondslideline;
mobj_t *slidemo;

fixed_t tmxmove, tmymove;

/*
==================
=
= P_HitSlideLine
=
= Adjusts the xmove / ymove so that the next move will slide along the wall
==================
*/

void P_HitSlideLine(line_t * ld)
{
    int side;
    angle_t lineangle, moveangle, deltaangle;
    fixed_t movelen, newlen;


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

    side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);
    if (side == 1)
        lineangle += ANG180;
    moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
    deltaangle = moveangle - lineangle;
    if (deltaangle > ANG180)
        deltaangle += ANG180;
//              I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance(tmxmove, tmymove);
    newlen = FixedMul(movelen, finecosine[deltaangle]);
    tmxmove = FixedMul(newlen, finecosine[lineangle]);
    tmymove = FixedMul(newlen, finesine[lineangle]);
}

/*
==============
=
= PTR_SlideTraverse
=
==============
*/

boolean PTR_SlideTraverse(intercept_t * in)
{
    line_t *li;

    if (!in->isaline)
        I_Error("PTR_SlideTraverse: not a line?");

    li = in->d.line;
    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
            return true;        // don't hit the back side
        goto isblocking;
    }

    P_LineOpening(li);          // set openrange, opentop, openbottom
    if (openrange < slidemo->height)
        goto isblocking;        // doesn't fit

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;        // mobj is too high

    if (openbottom - slidemo->z > 24 * FRACUNIT)
        goto isblocking;        // too big a step up

    return true;                // this line doesn't block movement

// the line does block movement, see if it is closer than best so far
  isblocking:
    if (in->frac < bestslidefrac)
    {
        secondslidefrac = bestslidefrac;
        secondslideline = bestslideline;
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;               // stop
}


/*
==================
=
= P_SlideMove
=
= The momx / momy move is bad, so try to slide along a wall
=
= Find the first line hit, move flush to it, and slide along it
=
= This is a kludgy mess.
==================
*/

void P_SlideMove(mobj_t * mo)
{
    fixed_t leadx, leady;
    fixed_t trailx, traily;
    fixed_t newx, newy;
    int hitcount;

    slidemo = mo;
    hitcount = 0;
  retry:
    if (++hitcount == 3)
        goto stairstep;         // don't loop forever

//
// trace along the three leading corners
//
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

    bestslidefrac = FRACUNIT + 1;

    P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);

//
// move up to the wall
//
    if (bestslidefrac == FRACUNIT + 1)
    {                           // the move most have hit the middle, so stairstep
      stairstep:
        if (!P_TryMove(mo, mo->x, mo->y + mo->momy))
            P_TryMove(mo, mo->x + mo->momx, mo->y);
        return;
    }

    bestslidefrac -= 0x800;     // fudge a bit to make sure it doesn't hit
    if (bestslidefrac > 0)
    {
        newx = FixedMul(mo->momx, bestslidefrac);
        newy = FixedMul(mo->momy, bestslidefrac);
        if (!P_TryMove(mo, mo->x + newx, mo->y + newy))
            goto stairstep;
    }

//
// now continue along the wall
//
    bestslidefrac = FRACUNIT - (bestslidefrac + 0x800); // remainder
    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;
    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul(mo->momx, bestslidefrac);
    tmymove = FixedMul(mo->momy, bestslidefrac);

    P_HitSlideLine(bestslideline);      // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove))
    {
        goto retry;
    }
}



/*
==============================================================================

							P_LineAttack

==============================================================================
*/


mobj_t *linetarget;             // who got hit (or NULL)
mobj_t *shootthing;
fixed_t shootz;                 // height if not aiming up or down
                                                                        // ???: use slope for monsters?
int la_damage;
fixed_t attackrange;

fixed_t aimslope;

extern fixed_t topslope, bottomslope;   // slopes to top and bottom of target

/*
===============================================================================
=
= PTR_AimTraverse
=
= Sets linetaget and aimslope when a target is aimed at
===============================================================================
*/

boolean PTR_AimTraverse(intercept_t * in)
{
    line_t *li;
    mobj_t *th;
    fixed_t slope, thingtopslope, thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {
        li = in->d.line;
        if (!(li->flags & ML_TWOSIDED))
            return false;       // stop
//
// crosses a two sided line
// a two sided line will restrict the possible target ranges
        P_LineOpening(li);

        if (openbottom >= opentop)
            return false;       // stop

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(openbottom - shootz, dist);
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(opentop - shootz, dist);
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;       // stop

        return true;            // shot continues
    }

//
// shoot a thing
//
    th = in->d.thing;
    if (th == shootthing)
        return true;            // can't shoot self
    if (!(th->flags & MF_SHOOTABLE))
        return true;            // corpse or something
    if (th->type == MT_POD)
    {                           // Can't auto-aim at pods
        return (true);
    }

// check angles to see if the thing can be aimed at

    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);
    if (thingtopslope < bottomslope)
        return true;            // shot over the thing
    thingbottomslope = FixedDiv(th->z - shootz, dist);
    if (thingbottomslope > topslope)
        return true;            // shot under the thing

//
// this thing can be hit!
//
    if (thingtopslope > topslope)
        thingtopslope = topslope;
    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope + thingbottomslope) / 2;
    linetarget = th;

    return false;               // don't go any farther
}


/*
==============================================================================
=
= PTR_ShootTraverse
=
==============================================================================
*/

boolean PTR_ShootTraverse(intercept_t * in)
{
    fixed_t x, y, z;
    fixed_t frac;
    line_t *li;
    mobj_t *th;
    fixed_t slope;
    fixed_t dist;
    fixed_t thingtopslope, thingbottomslope;
    mobj_t *mo;

    if (in->isaline)
    {
        li = in->d.line;
        if (li->special)
            P_ShootSpecialLine(shootthing, li);
        if (!(li->flags & ML_TWOSIDED))
            goto hitline;

//
// crosses a two sided line
//
        P_LineOpening(li);

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(openbottom - shootz, dist);
            if (slope > aimslope)
                goto hitline;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(opentop - shootz, dist);
            if (slope < aimslope)
                goto hitline;
        }

        return true;            // shot continues
//
// hit line
//
      hitline:
        // position a bit closer
        frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
        x = trace.x + FixedMul(trace.dx, frac);
        y = trace.y + FixedMul(trace.dy, frac);
        z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            if (z > li->frontsector->ceilingheight)
                return false;   // don't shoot the sky!
            if (li->backsector && li->backsector->ceilingpic == skyflatnum)
                return false;   // it's a sky hack wall
        }

        P_SpawnPuff(x, y, z);
        return false;           // don't go any farther
    }

//
// shoot a thing
//
    th = in->d.thing;
    if (th == shootthing)
        return true;            // can't shoot self
    if (!(th->flags & MF_SHOOTABLE))
        return true;            // corpse or something

//
// check for physical attacks on a ghost
//
    if (th->flags & MF_SHADOW && shootthing->player->readyweapon == wp_staff)
    {
        return (true);
    }

// check angles to see if the thing can be aimed at
    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);
    if (thingtopslope < aimslope)
        return true;            // shot over the thing
    thingbottomslope = FixedDiv(th->z - shootz, dist);
    if (thingbottomslope > aimslope)
        return true;            // shot under the thing

//
// hit thing
//
    // position a bit closer
    frac = in->frac - FixedDiv(10 * FRACUNIT, attackrange);
    x = trace.x + FixedMul(trace.dx, frac);
    y = trace.y + FixedMul(trace.dy, frac);
    z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));
    if (PuffType == MT_BLASTERPUFF1)
    {                           // Make blaster big puff
        mo = P_SpawnMobj(x, y, z, MT_BLASTERPUFF2);
        S_StartSound(mo, sfx_blshit);
    }
    else
    {
        P_SpawnPuff(x, y, z);
    }
    if (la_damage)
    {
        if (!(in->d.thing->flags & MF_NOBLOOD) && P_Random() < 192)
        {
            P_BloodSplatter(x, y, z, in->d.thing);
        }
        P_DamageMobj(th, shootthing, shootthing, la_damage);
    }
    return (false);             // don't go any farther
}

/*
=================
=
= P_AimLineAttack
=
=================
*/

fixed_t P_AimLineAttack(mobj_t * t1, angle_t angle, fixed_t distance)
{
    fixed_t x2, y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    topslope = 100 * FRACUNIT / 160;    // can't shoot outside view angles
    bottomslope = -100 * FRACUNIT / 160;
    attackrange = distance;
    linetarget = NULL;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS,
                   PTR_AimTraverse);

    if (linetarget)
        return aimslope;
    return 0;
}



/*
=================
=
= P_LineAttack
=
= if damage == 0, it is just a test trace that will leave linetarget set
=
=================
*/

void P_LineAttack(mobj_t * t1, angle_t angle, fixed_t distance, fixed_t slope,
                  int damage)
{
    fixed_t x2, y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    if (t1->flags2 & MF2_FEETARECLIPPED)
    {
        shootz -= FOOTCLIPSIZE;
    }
    attackrange = distance;
    aimslope = slope;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS,
                   PTR_ShootTraverse);
}



/*
==============================================================================

							USE LINES

==============================================================================
*/

mobj_t *usething;

boolean PTR_UseTraverse(intercept_t * in)
{
    if (!in->d.line->special)
    {
        P_LineOpening(in->d.line);
        if (openrange <= 0)
        {
            //S_StartSound (usething, sfx_noway);
            return false;       // can't use through a wall
        }
        return true;            // not a special line, but keep checking
    }

    if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
        return false;           // don't use back sides

    P_UseSpecialLine(usething, in->d.line);

    return false;               // can't use for than one special line in a row
}


/*
================
=
= P_UseLines
=
= Looks for special lines in front of the player to activate
================ 
*/

void P_UseLines(player_t * player)
{
    int angle;
    fixed_t x1, y1, x2, y2;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;
    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse);
}



/*
==============================================================================

							RADIUS ATTACK

==============================================================================
*/

mobj_t *bombsource;
mobj_t *bombspot;
int bombdamage;

/*
=================
=
= PIT_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

boolean PIT_RadiusAttack(mobj_t * thing)
{
    fixed_t dx, dy, dist;

    if (!(thing->flags & MF_SHOOTABLE))
    {
        return true;
    }
    if (thing->type == MT_MINOTAUR || thing->type == MT_SORCERER1
        || thing->type == MT_SORCERER2)
    {                           // Episode 2 and 3 bosses take no damage from PIT_RadiusAttack
        return (true);
    }
    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);
    dist = dx > dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;
    if (dist < 0)
    {
        dist = 0;
    }
    if (dist >= bombdamage)
    {                           // Out of range
        return true;
    }
    if (P_CheckSight(thing, bombspot))
    {                           // OK to damage, target is in direct path
        P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);
    }
    return (true);
}

/*
=================
=
= P_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

void P_RadiusAttack(mobj_t * spot, mobj_t * source, int damage)
{
    int x, y, xl, xh, yl, yh;
    fixed_t dist;

    dist = (damage + MAXRADIUS) << FRACBITS;
    yh = (spot->y + dist - bmaporgy) >> MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy) >> MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx) >> MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx) >> MAPBLOCKSHIFT;
    bombspot = spot;
    if (spot->type == MT_POD && spot->target)
    {
        bombsource = spot->target;
    }
    else
    {
        bombsource = source;
    }
    bombdamage = damage;
    for (y = yl; y <= yh; y++)
        for (x = xl; x <= xh; x++)
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);
}


/*
==============================================================================

						SECTOR HEIGHT CHANGING

= After modifying a sectors floor or ceiling height, call this
= routine to adjust the positions of all things that touch the
= sector.
=
= If anything doesn't fit anymore, true will be returned.
= If crunch is true, they will take damage as they are being crushed
= If Crunch is false, you should set the sector height back the way it
= was and call P_ChangeSector again to undo the changes
==============================================================================
*/

boolean crushchange;
boolean nofit;

/*
===============
=
= PIT_ChangeSector
=
===============
*/

boolean PIT_ChangeSector(mobj_t * thing)
{
    mobj_t *mo;

    if (P_ThingHeightClip(thing))
        return true;            // keep checking

    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        //P_SetMobjState (thing, S_GIBS);
        thing->height = 0;
        thing->radius = 0;
        return true;            // keep checking
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj(thing);
        return true;            // keep checking
    }

    if (!(thing->flags & MF_SHOOTABLE))
        return true;            // assume it is bloody gibs or something

    nofit = true;
    if (crushchange && !(leveltime & 3))
    {
        P_DamageMobj(thing, NULL, NULL, 10);
        // spray blood in a random direction
        mo = P_SpawnMobj(thing->x, thing->y, thing->z + thing->height / 2,
                         MT_BLOOD);
        mo->momx = P_SubRandom() << 12;
        mo->momy = P_SubRandom() << 12;
    }

    return true;                // keep checking (crush other things)   
}

/*
===============
=
= P_ChangeSector
=
===============
*/

boolean P_ChangeSector(sector_t * sector, boolean crunch)
{
    int x, y;

    nofit = false;
    crushchange = crunch;

// recheck heights for all things near the moving sector

    for (x = sector->blockbox[BOXLEFT]; x <= sector->blockbox[BOXRIGHT]; x++)
        for (y = sector->blockbox[BOXBOTTOM]; y <= sector->blockbox[BOXTOP];
             y++)
            P_BlockThingsIterator(x, y, PIT_ChangeSector);


    return nofit;
}
