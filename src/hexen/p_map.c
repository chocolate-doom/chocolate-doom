
//**************************************************************************
//**
//** p_map.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_map.c,v $
//** $Revision: 1.107 $
//** $Date: 95/10/13 04:26:47 $
//** $Author: paul $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

static void CheckForPushSpecial(line_t *line, int side, mobj_t *mobj);

/*
===============================================================================

NOTES:


===============================================================================
*/

/*
===============================================================================

mobj_t NOTES

mobj_ts are used to tell the refresh where to draw an image, tell the world simulation when objects are contacted, and tell the sound driver how to position a sound.

The refresh uses the next and prev links to follow lists of things in sectors as they are being drawn.  The sprite, frame, and angle elements determine which patch_t is used to draw the sprite if it is visible.  The sprite and frame values are allmost allways set from state_t structures.  The statescr.exe utility generates the states.h and states.c files that contain the sprite/frame numbers from the statescr.txt source file.  The xyz origin point represents a point at the bottom middle of the sprite (between the feet of a biped).  This is the default origin position for patch_ts grabbed with lumpy.exe.  A walking creature will have its z equal to the floor it is standing on.

The sound code uses the x,y, and subsector fields to do stereo positioning of any sound effited by the mobj_t.

The play simulation uses the blocklinks, x,y,z, radius, height to determine when mobj_ts are touching each other, touching lines in the map, or hit by trace lines (gunshots, lines of sight, etc). The mobj_t->flags element has various bit flags used by the simulation.


Every mobj_t is linked into a single sector based on it's origin coordinates.
The subsector_t is found with R_PointInSubsector(x,y), and the sector_t can be found with subsector->sector.  The sector links are only used by the rendering code,  the play simulation does not care about them at all.

Any mobj_t that needs to be acted upon be something else in the play world (block movement, be shot, etc) will also need to be linked into the blockmap.  If the thing has the MF_NOBLOCK flag set, it will not use the block links. It can still interact with other things, but only as the instigator (missiles will run into other things, but nothing can run into a missile).   Each block in the grid is 128*128 units, and knows about every line_t that it contains a piece of, and every interactable mobj_t that has it's origin contained.

A valid mobj_t is a mobj_t that has the proper subsector_t filled in for it's xy coordinates and is linked into the subsector's sector or has the MF_NOSECTOR flag set (the subsector_t needs to be valid even if MF_NOSECTOR is set), and is linked into a blockmap block or has the MF_NOBLOCKMAP flag set.  Links should only be modified by the P_[Un]SetThingPosition () functions.  Do not change the MF_NO? flags while a thing is valid.


===============================================================================
*/

fixed_t         tmbbox[4];
mobj_t          *tmthing;
mobj_t			 *tsthing;
int                     tmflags;
fixed_t         tmx, tmy;

boolean         floatok;	// if true, move would be ok if
							// within tmfloorz - tmceilingz

fixed_t tmfloorz, tmceilingz, tmdropoffz;
int tmfloorpic;

// keep track of the line that lowers the ceiling, so missiles don't explode
// against sky hack walls
line_t *ceilingline;

// keep track of special lines as they are hit, but don't process them
// until the move is proven valid
#define MAXSPECIALCROSS 8
line_t *spechit[MAXSPECIALCROSS];
int numspechit;

mobj_t *onmobj; // generic global onmobj...used for landing on pods/players
mobj_t *BlockingMobj;

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

boolean PIT_StompThing (mobj_t *thing)
{
	fixed_t         blockdist;

	if (!(thing->flags & MF_SHOOTABLE) )
		return true;

	blockdist = thing->radius + tmthing->radius;
	if ( abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist )
		return true;            // didn't hit it

	if (thing == tmthing)
		return true;            // don't clip against self

	if(!(tmthing->flags2&MF2_TELESTOMP))
	{ // Not allowed to stomp things
		return(false);
	}

	P_DamageMobj (thing, tmthing, tmthing, 10000);

	return true;
}


/*
===================
=
= P_TeleportMove
=
===================
*/

boolean P_TeleportMove (mobj_t *thing, fixed_t x, fixed_t y)
{
	int                     xl,xh,yl,yh,bx,by;
	subsector_t             *newsubsec;

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

	newsubsec = R_PointInSubsector (x,y);
	ceilingline = NULL;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;
	tmfloorpic = newsubsec->sector->floorpic;

	validcount++;
	numspechit = 0;

//
// stomp on any things contacted
//
	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
				return false;

//
// the move is ok, so link the thing into its new position
//
	P_UnsetThingPosition (thing);

	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->x = x;
	thing->y = y;

	P_SetThingPosition (thing);

	return true;
}


boolean PIT_ThrustStompThing (mobj_t *thing)
{
	fixed_t         blockdist;

	if (!(thing->flags & MF_SHOOTABLE) )
		return true;

	blockdist = thing->radius + tsthing->radius;
	if ( abs(thing->x - tsthing->x) >= blockdist || 
		  abs(thing->y - tsthing->y) >= blockdist ||
			(thing->z > tsthing->z+tsthing->height) )
		return true;            // didn't hit it

	if (thing == tsthing)
		return true;            // don't clip against self

	P_DamageMobj (thing, tsthing, tsthing, 10001);
	tsthing->args[1] = 1;	// Mark thrust thing as bloody

	return true;
}



void PIT_ThrustSpike(mobj_t *actor)
{
	int xl,xh,yl,yh,bx,by;
	int x0,x2,y0,y2;

	tsthing = actor;

	x0 = actor->x - actor->info->radius;
	x2 = actor->x + actor->info->radius;
	y0 = actor->y - actor->info->radius;
	y2 = actor->y + actor->info->radius;

	xl = (x0 - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (x2 - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (y0 - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (y2 - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

	// stomp on any things contacted
	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			P_BlockThingsIterator(bx,by,PIT_ThrustStompThing);
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

boolean PIT_CheckLine(line_t *ld)
{
	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
		||      tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
		||      tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
		||      tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
	{
		return(true);
	}
	if(P_BoxOnLineSide(tmbbox, ld) != -1)
	{
		return(true);
	}

// a line has been hit
/*
=
= The moving thing's destination position will cross the given line.
= If this should not be allowed, return false.
= If the line is special, keep track of it to process later if the move
=       is proven ok.  NOTE: specials are NOT sorted by order, so two special lines
=       that are only 8 pixels apart could be crossed in either order.
*/

	if(!ld->backsector)
	{ // One sided line
		if (tmthing->flags2&MF2_BLASTED)
		{
			P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass>>5);
		}
		CheckForPushSpecial(ld, 0, tmthing);
		return(false);
	}
	if(!(tmthing->flags&MF_MISSILE))
	{
		if(ld->flags&ML_BLOCKING)
		{ // Explicitly blocking everything
			if (tmthing->flags2&MF2_BLASTED)
			{
				P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass>>5);
			}
			CheckForPushSpecial(ld, 0, tmthing);
			return(false);
		}
		if(!tmthing->player && ld->flags&ML_BLOCKMONSTERS)
		{ // Block monsters only
			if (tmthing->flags2&MF2_BLASTED)
			{
				P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass>>5);
			}
			return(false);
		}
	}
	P_LineOpening(ld);              // set openrange, opentop, openbottom
	// adjust floor / ceiling heights
	if(opentop < tmceilingz)
	{
		tmceilingz = opentop;
		ceilingline = ld;
	}
	if(openbottom > tmfloorz)
	{
		tmfloorz = openbottom;
	}
	if(lowfloor < tmdropoffz)
	{
		tmdropoffz = lowfloor;
	}
	if(ld->special)
	{ // Contacted a special line, add it to the list
		spechit[numspechit] = ld;
		numspechit++;
	}
	return(true);
}

//---------------------------------------------------------------------------
//
// FUNC PIT_CheckThing
//
//---------------------------------------------------------------------------

boolean PIT_CheckThing(mobj_t *thing)
{
	fixed_t blockdist;
	boolean solid;
	int damage;

	if(!(thing->flags&(MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
	{ // Can't hit thing
		return(true);
	}
	blockdist = thing->radius+tmthing->radius;
	if(abs(thing->x-tmx) >= blockdist || abs(thing->y-tmy) >= blockdist)
	{ // Didn't hit thing
		return(true);
	}
	if(thing == tmthing)
	{ // Don't clip against self
		return(true);
	}
	BlockingMobj = thing;
 	if(tmthing->flags2&MF2_PASSMOBJ)
	{ // check if a mobj passed over/under another object
		if(tmthing->type == MT_BISHOP && thing->type == MT_BISHOP)
		{ // don't let bishops fly over other bishops
			return false;
		}
		if(tmthing->z >= thing->z+thing->height
			&& !(thing->flags&MF_SPECIAL))
		{
			return(true);
		}
		else if(tmthing->z+tmthing->height < thing->z
			&& !(thing->flags&MF_SPECIAL))
		{ // under thing
			return(true);
		}
	}
	// Check for skulls slamming into things
	if(tmthing->flags&MF_SKULLFLY)
	{
		if(tmthing->type == MT_MINOTAUR)
		{
			// Slamming minotaurs shouldn't move non-creatures
			if (!(thing->flags&MF_COUNTKILL))
			{
				return(false);
			}
		}
		else if(tmthing->type == MT_HOLY_FX)
		{
			if(thing->flags&MF_SHOOTABLE && thing != tmthing->target)
			{
				if(netgame && !deathmatch && thing->player)
				{ // don't attack other co-op players
					return true;
				}
				if(thing->flags2&MF2_REFLECTIVE
					&& (thing->player || thing->flags2&MF2_BOSS))
				{
					tmthing->special1 = (int)tmthing->target;
					tmthing->target = thing;
					return true;
				}
				if(thing->flags&MF_COUNTKILL || thing->player)
				{
					tmthing->special1 = (int)thing;
				}
				if(P_Random() < 96)
				{
					damage = 12;
					if(thing->player || thing->flags2&MF2_BOSS)
					{
						damage = 3;
						// ghost burns out faster when attacking players/bosses
						tmthing->health -= 6;
					}
					P_DamageMobj(thing, tmthing, tmthing->target, damage);
					if(P_Random() < 128)
					{
						P_SpawnMobj(tmthing->x, tmthing->y, tmthing->z,
							MT_HOLY_PUFF);
						S_StartSound(tmthing, SFX_SPIRIT_ATTACK);
						if(thing->flags&MF_COUNTKILL && P_Random() < 128
						&& !S_GetSoundPlayingInfo(thing, SFX_PUPPYBEAT))
						{
							if ((thing->type == MT_CENTAUR) ||
								(thing->type == MT_CENTAURLEADER) ||
								(thing->type == MT_ETTIN))
							{
								S_StartSound(thing, SFX_PUPPYBEAT);
							}
						}
					}
				}
				if(thing->health <= 0)
				{
					tmthing->special1 = 0;
				}
			}
			return true;
		}
		damage = ((P_Random()%8)+1)*tmthing->damage;
		P_DamageMobj(thing, tmthing, tmthing, damage);
		tmthing->flags &= ~MF_SKULLFLY;
		tmthing->momx = tmthing->momy = tmthing->momz = 0;
		P_SetMobjState(tmthing, tmthing->info->seestate);
		return(false);
	}
	// Check for blasted thing running into another
	if(tmthing->flags2&MF2_BLASTED && thing->flags&MF_SHOOTABLE)
	{
		if (!(thing->flags2&MF2_BOSS) &&
			(thing->flags&MF_COUNTKILL))
		{
			thing->momx += tmthing->momx;
			thing->momy += tmthing->momy;
			if ((thing->momx + thing->momy) > 3*FRACUNIT)
			{
				damage = (tmthing->info->mass/100)+1;
				P_DamageMobj(thing, tmthing, tmthing, damage);
				damage = (thing->info->mass/100)+1;
				P_DamageMobj(tmthing, thing, thing, damage>>2);
			}
			return(false);
		}
	}
	// Check for missile
	if(tmthing->flags&MF_MISSILE)
	{
		// Check for a non-shootable mobj
		if(thing->flags2&MF2_NONSHOOTABLE)
		{
			return true;
		}
		// Check if it went over / under
		if(tmthing->z > thing->z+thing->height)
		{ // Over thing
			return(true);
		}
		if(tmthing->z+tmthing->height < thing->z)
		{ // Under thing
			return(true);
		}
		if(tmthing->flags2&MF2_FLOORBOUNCE)
		{
			if(tmthing->target == thing || !(thing->flags&MF_SOLID))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		if(tmthing->type == MT_LIGHTNING_FLOOR
			|| tmthing->type == MT_LIGHTNING_CEILING)
		{
			if(thing->flags&MF_SHOOTABLE && thing != tmthing->target)
			{
				if(thing->info->mass != MAXINT)
				{
					thing->momx += tmthing->momx>>4;
					thing->momy += tmthing->momy>>4;
				}
				if((!thing->player && !(thing->flags2&MF2_BOSS))
					|| !(leveltime&1))
				{
					if(thing->type == MT_CENTAUR 
					|| thing->type == MT_CENTAURLEADER)
					{ // Lightning does more damage to centaurs
						P_DamageMobj(thing, tmthing, tmthing->target, 9);
					}
					else
					{
						P_DamageMobj(thing, tmthing, tmthing->target, 3);
					}
					if(!(S_GetSoundPlayingInfo(tmthing, 
						SFX_MAGE_LIGHTNING_ZAP)))
					{
						S_StartSound(tmthing, SFX_MAGE_LIGHTNING_ZAP);
					}
					if(thing->flags&MF_COUNTKILL && P_Random() < 64 
					&& !S_GetSoundPlayingInfo(thing, SFX_PUPPYBEAT))
					{
						if ((thing->type == MT_CENTAUR) ||
							(thing->type == MT_CENTAURLEADER) ||
							(thing->type == MT_ETTIN))
						{
							S_StartSound(thing, SFX_PUPPYBEAT);
						}
					}
				}
				tmthing->health--;
				if(tmthing->health <= 0 || thing->health <= 0)
				{
					return false;
				}
				if(tmthing->type == MT_LIGHTNING_FLOOR)
				{
					if(tmthing->special2 
						&& !((mobj_t *)tmthing->special2)->special1)
					{
						((mobj_t *)tmthing->special2)->special1 = 
							(int)thing;
					}
				}
				else if(!tmthing->special1)
				{
					tmthing->special1 = (int)thing;
				}
			}
			return true; // lightning zaps through all sprites
		}
		else if(tmthing->type == MT_LIGHTNING_ZAP)
		{
			mobj_t *lmo;

			if(thing->flags&MF_SHOOTABLE && thing != tmthing->target)
			{			
				lmo = (mobj_t *)tmthing->special2;
				if(lmo)
				{
					if(lmo->type == MT_LIGHTNING_FLOOR)
					{
						if(lmo->special2 
							&& !((mobj_t *)lmo->special2)->special1)
						{
							((mobj_t *)lmo->special2)->special1 = (int)thing;
						}
					}
					else if(!lmo->special1)
					{
						lmo->special1 = (int)thing;
					}
					if(!(leveltime&3))
					{
						lmo->health--;
					}
				}
			}
		}
		else if(tmthing->type == MT_MSTAFF_FX2 && thing != tmthing->target)
		{
			if(!thing->player && !(thing->flags2&MF2_BOSS))
			{
				switch(thing->type)
				{
					case MT_FIGHTER_BOSS:	// these not flagged boss
					case MT_CLERIC_BOSS:	// so they can be blasted
					case MT_MAGE_BOSS:
						break;
					default:
						P_DamageMobj(thing, tmthing, tmthing->target, 10);
						return true;
						break;
				}
			}
		}
		if(tmthing->target && tmthing->target->type == thing->type)
		{ // Don't hit same species as originator
			if(thing == tmthing->target)
			{ // Don't missile self
				return(true);
			}
			if(!thing->player)
			{ // Hit same species as originator, explode, no damage
				return(false);
			}
		}
		if(!(thing->flags&MF_SHOOTABLE))
		{ // Didn't do any damage
			return!(thing->flags&MF_SOLID);
		}
		if(tmthing->flags2&MF2_RIP)
		{
			if (!(thing->flags&MF_NOBLOOD) &&
				!(thing->flags2&MF2_REFLECTIVE) &&
				!(thing->flags2&MF2_INVULNERABLE))
			{ // Ok to spawn some blood
				P_RipperBlood(tmthing);
			}
			//S_StartSound(tmthing, sfx_ripslop);
			damage = ((P_Random()&3)+2)*tmthing->damage;
			P_DamageMobj(thing, tmthing, tmthing->target, damage);
			if(thing->flags2&MF2_PUSHABLE
				&& !(tmthing->flags2&MF2_CANNOTPUSH))
			{ // Push thing
				thing->momx += tmthing->momx>>2;
				thing->momy += tmthing->momy>>2;
			}
			numspechit = 0;
			return(true);
		}
		// Do damage
		damage = ((P_Random()%8)+1)*tmthing->damage;
		if(damage)
		{
			if (!(thing->flags&MF_NOBLOOD) && 
				!(thing->flags2&MF2_REFLECTIVE) &&
				!(thing->flags2&MF2_INVULNERABLE) &&
				!(tmthing->type == MT_TELOTHER_FX1) &&
				!(tmthing->type == MT_TELOTHER_FX2) &&
				!(tmthing->type == MT_TELOTHER_FX3) &&
				!(tmthing->type == MT_TELOTHER_FX4) &&
				!(tmthing->type == MT_TELOTHER_FX5) &&
				(P_Random() < 192))
			{
				P_BloodSplatter(tmthing->x, tmthing->y, tmthing->z, thing);
			}
			P_DamageMobj(thing, tmthing, tmthing->target, damage);
		}
		return(false);
	}
	if(thing->flags2&MF2_PUSHABLE && !(tmthing->flags2&MF2_CANNOTPUSH))
	{ // Push thing
		thing->momx += tmthing->momx>>2;
		thing->momy += tmthing->momy>>2;
	}
	// Check for special thing
	if(thing->flags&MF_SPECIAL)
	{
		solid = thing->flags&MF_SOLID;
		if(tmflags&MF_PICKUP)
		{ // Can be picked up by tmthing
			P_TouchSpecialThing(thing, tmthing); // Can remove thing
		}
		return(!solid);
	}
	return(!(thing->flags&MF_SOLID));
}

//---------------------------------------------------------------------------
//
// PIT_CheckOnmobjZ
//
//---------------------------------------------------------------------------

boolean PIT_CheckOnmobjZ(mobj_t *thing)
{
	fixed_t blockdist;

	if(!(thing->flags&(MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
	{ // Can't hit thing
		return(true);
	}
	blockdist = thing->radius+tmthing->radius;
	if(abs(thing->x-tmx) >= blockdist || abs(thing->y-tmy) >= blockdist)
	{ // Didn't hit thing
		return(true);
	}
	if(thing == tmthing)
	{ // Don't clip against self
		return(true);
	}
	if(tmthing->z > thing->z+thing->height)
	{
		return(true);
	}
	else if(tmthing->z+tmthing->height < thing->z)
	{ // under thing
		return(true);
	}
	if(thing->flags&MF_SOLID)
	{
		onmobj = thing;
	}
	return(!(thing->flags&MF_SOLID));
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

boolean P_TestMobjLocation(mobj_t *mobj)
{
	int flags;

	flags = mobj->flags;
	mobj->flags &= ~MF_PICKUP;
	if(P_CheckPosition(mobj, mobj->x, mobj->y))
	{ // XY is ok, now check Z
		mobj->flags = flags;
		if((mobj->z < mobj->floorz)
			|| (mobj->z+mobj->height > mobj->ceilingz))
		{ // Bad Z
			return(false);
		}
		return(true);
	}
	mobj->flags = flags;
	return(false);
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
tmdropoffz = the lowest point contacted (monsters won't move to a dropoff)
speciallines[]
numspeciallines
mobj_t *BlockingMobj = pointer to thing that blocked position (NULL if not
blocked, or blocked by a line).

==================
*/

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y)
{
	int                     xl,xh,yl,yh,bx,by;
	subsector_t             *newsubsec;

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

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;
	tmfloorpic = newsubsec->sector->floorpic;

	validcount++;
	numspechit = 0;

	if(tmflags&MF_NOCLIP && !(tmflags&MF_SKULLFLY))
	{
		return true;
	}

//
// check things first, possibly picking things up
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

	BlockingMobj = NULL;
	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
				return false;
//
// check lines
//
	if(tmflags&MF_NOCLIP)
	{
		return true;
	}

	BlockingMobj = NULL;
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

//=============================================================================
//
// P_CheckOnmobj(mobj_t *thing)
//
//              Checks if the new Z position is legal
//=============================================================================

mobj_t *P_CheckOnmobj(mobj_t *thing)
{
	int                     xl,xh,yl,yh,bx,by;
	subsector_t             *newsubsec;
	fixed_t x;
	fixed_t y;
	mobj_t oldmo;

	x = thing->x;
	y = thing->y;
	tmthing = thing;
	tmflags = thing->flags;
	oldmo = *thing; // save the old mobj before the fake zmovement
	P_FakeZMovement(tmthing);

	tmx = x;
	tmy = y;

	tmbbox[BOXTOP] = y + tmthing->radius;
	tmbbox[BOXBOTTOM] = y - tmthing->radius;
	tmbbox[BOXRIGHT] = x + tmthing->radius;
	tmbbox[BOXLEFT] = x - tmthing->radius;

	newsubsec = R_PointInSubsector (x,y);
	ceilingline = NULL;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;
	tmfloorpic = newsubsec->sector->floorpic;

	validcount++;
	numspechit = 0;

	if ( tmflags & MF_NOCLIP )
		return NULL;

//
// check things first, possibly picking things up
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
	xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			if (!P_BlockThingsIterator(bx,by,PIT_CheckOnmobjZ))
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

void P_FakeZMovement(mobj_t *mo)
{
	int dist;
	int delta;
//
// adjust height
//
	mo->z += mo->momz;
	if(mo->flags&MF_FLOAT && mo->target)
	{       // float down towards target if too close
		if(!(mo->flags&MF_SKULLFLY) && !(mo->flags&MF_INFLOAT))
		{
			dist = P_AproxDistance(mo->x-mo->target->x, mo->y-mo->target->y);
			delta =( mo->target->z+(mo->height>>1))-mo->z;
			if (delta < 0 && dist < -(delta*3))
				mo->z -= FLOATSPEED;
			else if (delta > 0 && dist < (delta*3))
				mo->z += FLOATSPEED;
		}
	}
	if(mo->player && mo->flags2&MF2_FLY && !(mo->z <= mo->floorz)
		&& leveltime&2)
	{
		mo->z += finesine[(FINEANGLES/20*leveltime>>2)&FINEMASK];
	}

//
// clip movement
//
	if(mo->z <= mo->floorz)
	{ // Hit the floor
		mo->z = mo->floorz;
		if(mo->momz < 0)
		{
			mo->momz = 0;
		}
		if(mo->flags&MF_SKULLFLY)
		{ // The skull slammed into something
			mo->momz = -mo->momz;
		}
		if(mo->info->crashstate && (mo->flags&MF_CORPSE))
		{
			return;
		}
	}
	else if(mo->flags2&MF2_LOGRAV)
	{
		if(mo->momz == 0)
			mo->momz = -(GRAVITY>>3)*2;
		else
			mo->momz -= GRAVITY>>3;
	}
	else if (! (mo->flags & MF_NOGRAVITY) )
	{
		if (mo->momz == 0)
			mo->momz = -GRAVITY*2;
		else
			mo->momz -= GRAVITY;
	}

	if (mo->z + mo->height > mo->ceilingz)
	{       // hit the ceiling
		if (mo->momz > 0)
			mo->momz = 0;
		mo->z = mo->ceilingz - mo->height;
		if (mo->flags & MF_SKULLFLY)
		{       // the skull slammed into something
			mo->momz = -mo->momz;
		}
	}
}

//===========================================================================
//
// CheckForPushSpecial
//
//===========================================================================

static void CheckForPushSpecial(line_t *line, int side, mobj_t *mobj)
{
	if (line->special)
	{
		if(mobj->flags2&MF2_PUSHWALL)
		{
			P_ActivateLine(line, mobj, side, SPAC_PUSH);
		}
		else if(mobj->flags2&MF2_IMPACT)
		{
			P_ActivateLine(line, mobj, side, SPAC_IMPACT);
		}	
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

boolean P_TryMove (mobj_t *thing, fixed_t x, fixed_t y)
{
	fixed_t         oldx, oldy;
	int                     side, oldside;
	line_t          *ld;

	floatok = false;
	if(!P_CheckPosition(thing, x, y))
	{ // Solid wall or thing
		if(!BlockingMobj || BlockingMobj->player 
			|| !thing->player)
		{ 
			goto pushline;
		}
		else if (BlockingMobj->z+BlockingMobj->height-thing->z 
			> 24*FRACUNIT 
			|| (BlockingMobj->subsector->sector->ceilingheight
			-(BlockingMobj->z+BlockingMobj->height) < thing->height)
			|| (tmceilingz-(BlockingMobj->z+BlockingMobj->height) 
			< thing->height))
		{
			goto pushline;
		}
	}
	if(!(thing->flags&MF_NOCLIP))
	{
		if(tmceilingz-tmfloorz < thing->height)
		{ // Doesn't fit
			goto pushline;
		}
		floatok = true;
		if(!(thing->flags&MF_TELEPORT)
			&& tmceilingz-thing->z < thing->height
			&& thing->type != MT_LIGHTNING_CEILING
			&& !(thing->flags2&MF2_FLY))
		{ // mobj must lower itself to fit
			goto pushline;
		}
		if(thing->flags2&MF2_FLY)
		{
			if(thing->z+thing->height > tmceilingz)
			{
				thing->momz = -8*FRACUNIT;
				goto pushline;
			}
			else if(thing->z < tmfloorz && tmfloorz-tmdropoffz > 24*FRACUNIT)
			{
				thing->momz = 8*FRACUNIT;
				goto pushline;
			}
		}
		if(!(thing->flags&MF_TELEPORT)
			// The Minotaur floor fire (MT_MNTRFX2) can step up any amount
			&& thing->type != MT_MNTRFX2 && thing->type != MT_LIGHTNING_FLOOR
			&& tmfloorz-thing->z > 24*FRACUNIT)
		{
			goto pushline;
		}
		if (!(thing->flags&(MF_DROPOFF|MF_FLOAT)) && 
			(tmfloorz-tmdropoffz > 24*FRACUNIT) &&
			!(thing->flags2&MF2_BLASTED))
		{ // Can't move over a dropoff unless it's been blasted
				return(false);
		}
		if(thing->flags2&MF2_CANTLEAVEFLOORPIC 
			&& (tmfloorpic != thing->subsector->sector->floorpic
				|| tmfloorz-thing->z != 0))
		{ // must stay within a sector of a certain floor type
			return false;
		}
	}

//
// the move is ok, so link the thing into its new position
//
	P_UnsetThingPosition (thing);

	oldx = thing->x;
	oldy = thing->y;
	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->floorpic = tmfloorpic;
	thing->x = x;
	thing->y = y;

	P_SetThingPosition (thing);

	if(thing->flags2&MF2_FLOORCLIP)
	{
		if(thing->z == thing->subsector->sector->floorheight
			&& P_GetThingFloorType(thing) >= FLOOR_LIQUID)
		{
			thing->floorclip = 10*FRACUNIT;
		}
		else 
		{
			thing->floorclip = 0;
		}
	}

//
// if any special lines were hit, do the effect
//
	if (! (thing->flags&(MF_TELEPORT|MF_NOCLIP)) )
	{
		while (numspechit > 0)
		{
			numspechit--;
			// see if the line was crossed
			ld = spechit[numspechit];
			side = P_PointOnLineSide (thing->x, thing->y, ld);
			oldside = P_PointOnLineSide (oldx, oldy, ld);
			if (side != oldside)
			{
				if (ld->special)
				{
					if(thing->player)
					{
						P_ActivateLine(ld, thing, oldside, SPAC_CROSS);
					}
					else if(thing->flags2&MF2_MCROSS)
					{
						P_ActivateLine(ld, thing, oldside, SPAC_MCROSS);
					}
					else if(thing->flags2&MF2_PCROSS)
					{
						P_ActivateLine(ld, thing, oldside, SPAC_PCROSS);
					}
				}
			}
		}
	}
	return true;

pushline:
	if(!(thing->flags&(MF_TELEPORT|MF_NOCLIP)))
	{
		int numSpecHitTemp;

		if (tmthing->flags2&MF2_BLASTED)
		{
			P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass>>5);
		}
		numSpecHitTemp = numspechit;
		while (numSpecHitTemp > 0)
		{
			numSpecHitTemp--;
			// see if the line was crossed
			ld = spechit[numSpecHitTemp];
			side = P_PointOnLineSide (thing->x, thing->y, ld);
			CheckForPushSpecial(ld, side, thing);
		}
	}
	return false;
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

boolean P_ThingHeightClip (mobj_t *thing)
{
	boolean         onfloor;

	onfloor = (thing->z == thing->floorz);

	P_CheckPosition (thing, thing->x, thing->y);
	// what about stranding a monster partially off an edge?

	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;
	thing->floorpic = tmfloorpic;

	if (onfloor)
	{ // walking monsters rise and fall with the floor
		if((thing->z-thing->floorz < 9*FRACUNIT) 
			|| (thing->flags&MF_NOGRAVITY))
		{ 
			thing->z = thing->floorz;
		}
	}
	else
	{       // don't adjust a floating monster unless forced to
		if (thing->z+thing->height > thing->ceilingz)
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

fixed_t         bestslidefrac, secondslidefrac;
line_t          *bestslideline, *secondslideline;
mobj_t          *slidemo;

fixed_t         tmxmove, tmymove;

/*
==================
=
= P_HitSlideLine
=
= Adjusts the xmove / ymove so that the next move will slide along the wall
==================
*/

void P_HitSlideLine (line_t *ld)
{
	int                     side;
	angle_t         lineangle, moveangle, deltaangle;
	fixed_t         movelen, newlen;


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
//              I_Error ("SlideLine: ang>ANG180");

	lineangle >>= ANGLETOFINESHIFT;
	deltaangle >>= ANGLETOFINESHIFT;

	movelen = P_AproxDistance (tmxmove, tmymove);
	newlen = FixedMul (movelen, finecosine[deltaangle]);
	tmxmove = FixedMul (newlen, finecosine[lineangle]);
	tmymove = FixedMul (newlen, finesine[lineangle]);
}

/*
==============
=
= PTR_SlideTraverse
=
==============
*/

boolean         PTR_SlideTraverse (intercept_t *in)
{
	line_t  *li;

	if (!in->isaline)
		I_Error ("PTR_SlideTraverse: not a line?");

	li = in->d.line;
	if ( ! (li->flags & ML_TWOSIDED) )
	{
		if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
			return true;            // don't hit the back side
		goto isblocking;
	}

	P_LineOpening (li);                  // set openrange, opentop, openbottom
	if (openrange < slidemo->height)
		goto isblocking;                // doesn't fit

	if (opentop - slidemo->z < slidemo->height)
		goto isblocking;                // mobj is too high

	if (openbottom - slidemo->z > 24*FRACUNIT )
		goto isblocking;                // too big a step up

	return true;            // this line doesn't block movement

// the line does block movement, see if it is closer than best so far
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

void P_SlideMove (mobj_t *mo)
{
	fixed_t         leadx, leady;
	fixed_t         trailx, traily;
	fixed_t         newx, newy;
	int                     hitcount;

	slidemo = mo;
	hitcount = 0;
retry:
	if (++hitcount == 3)
		goto stairstep;                 // don't loop forever

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

	bestslidefrac = FRACUNIT+1;

	P_PathTraverse ( leadx, leady, leadx+mo->momx, leady+mo->momy,
	 PT_ADDLINES, PTR_SlideTraverse );
	P_PathTraverse ( trailx, leady, trailx+mo->momx, leady+mo->momy,
	 PT_ADDLINES, PTR_SlideTraverse );
	P_PathTraverse ( leadx, traily, leadx+mo->momx, traily+mo->momy,
	 PT_ADDLINES, PTR_SlideTraverse );

//
// move up to the wall
//
	if(bestslidefrac == FRACUNIT+1)
	{ // the move must have hit the middle, so stairstep
stairstep:
		if(!P_TryMove(mo, mo->x, mo->y+mo->momy))
		{
			P_TryMove(mo, mo->x+mo->momx, mo->y);
		}
		return;
	}

	bestslidefrac -= 0x800; // fudge a bit to make sure it doesn't hit
	if (bestslidefrac > 0)
	{
		newx = FixedMul (mo->momx, bestslidefrac);
		newy = FixedMul (mo->momy, bestslidefrac);
		if (!P_TryMove (mo, mo->x+newx, mo->y+newy))
			goto stairstep;
	}

//
// now continue along the wall
//
	bestslidefrac = FRACUNIT-(bestslidefrac+0x800); // remainder
	if (bestslidefrac > FRACUNIT)
		bestslidefrac = FRACUNIT;
	if (bestslidefrac <= 0)
		return;

	tmxmove = FixedMul (mo->momx, bestslidefrac);
	tmymove = FixedMul (mo->momy, bestslidefrac);

	P_HitSlideLine (bestslideline);                         // clip the moves

	mo->momx = tmxmove;
	mo->momy = tmymove;

	if (!P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove))
	{
		goto retry;
	}
}

//============================================================================
//
// PTR_BounceTraverse
//
//============================================================================

boolean PTR_BounceTraverse(intercept_t *in)
{
	line_t  *li;

	if (!in->isaline)
		I_Error ("PTR_BounceTraverse: not a line?");

	li = in->d.line;
	if (!(li->flags&ML_TWOSIDED))
	{
		if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
			return true;            // don't hit the back side
		goto bounceblocking;
	}

	P_LineOpening (li);                  // set openrange, opentop, openbottom
	if (openrange < slidemo->height)
		goto bounceblocking;                // doesn't fit

	if (opentop - slidemo->z < slidemo->height)
		goto bounceblocking;                // mobj is too high
	return true;            // this line doesn't block movement

// the line does block movement, see if it is closer than best so far
bounceblocking:
	if (in->frac < bestslidefrac)
	{
		secondslidefrac = bestslidefrac;
		secondslideline = bestslideline;
		bestslidefrac = in->frac;
		bestslideline = li;
	}
	return false;   // stop
}

//============================================================================
//
// P_BounceWall
//
//============================================================================

void P_BounceWall(mobj_t *mo)
{
	fixed_t         leadx, leady;
	int             side;
	angle_t         lineangle, moveangle, deltaangle;
	fixed_t         movelen;

	slidemo = mo;

//
// trace along the three leading corners
//
	if(mo->momx > 0)
	{
		leadx = mo->x+mo->radius;
	}
	else
	{
		leadx = mo->x-mo->radius;
	}
	if(mo->momy > 0)
	{
		leady = mo->y+mo->radius;
	}
	else
	{
		leady = mo->y-mo->radius;
	}
	bestslidefrac = FRACUNIT+1;
	P_PathTraverse(leadx, leady, leadx+mo->momx, leady+mo->momy,
		PT_ADDLINES, PTR_BounceTraverse);

	side = P_PointOnLineSide(mo->x, mo->y, bestslideline);
	lineangle = R_PointToAngle2(0, 0, bestslideline->dx,
		bestslideline->dy);
	if(side == 1)
		lineangle += ANG180;
	moveangle = R_PointToAngle2(0, 0, mo->momx, mo->momy);
	deltaangle = (2*lineangle)-moveangle;
//	if (deltaangle > ANG180)
//		deltaangle += ANG180;
//              I_Error ("SlideLine: ang>ANG180");

	lineangle >>= ANGLETOFINESHIFT;
	deltaangle >>= ANGLETOFINESHIFT;

	movelen = P_AproxDistance(mo->momx, mo->momy);
	movelen = FixedMul(movelen, 0.75*FRACUNIT); // friction
	if (movelen < FRACUNIT) movelen = 2*FRACUNIT;
	mo->momx = FixedMul(movelen, finecosine[deltaangle]);
	mo->momy = FixedMul(movelen, finesine[deltaangle]);
}


/*
==============================================================================

							P_LineAttack

==============================================================================
*/


mobj_t *PuffSpawned;
mobj_t          *linetarget;                    // who got hit (or NULL)
mobj_t          *shootthing;
fixed_t         shootz;                                 // height if not aiming up or down
									// ???: use slope for monsters?
int                     la_damage;
fixed_t         attackrange;

fixed_t         aimslope;

extern  fixed_t         topslope, bottomslope;  // slopes to top and bottom of target

/*
===============================================================================
=
= PTR_AimTraverse
=
= Sets linetaget and aimslope when a target is aimed at
===============================================================================
*/

boolean         PTR_AimTraverse (intercept_t *in)
{
	line_t          *li;
	mobj_t          *th;
	fixed_t         slope, thingtopslope, thingbottomslope;
	fixed_t         dist;

	if (in->isaline)
	{
		li = in->d.line;
		if ( !(li->flags & ML_TWOSIDED) )
			return false;           // stop
//
// crosses a two sided line
// a two sided line will restrict the possible target ranges
		P_LineOpening (li);

		if (openbottom >= opentop)
			return false;           // stop

		dist = FixedMul (attackrange, in->frac);

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
			return false;           // stop

		return true;            // shot continues
	}

//
// shoot a thing
//
	th = in->d.thing;
	if (th == shootthing)
		return true;            // can't shoot self
	if(!(th->flags&MF_SHOOTABLE))
	{ // corpse or something
		return true;
	}
	if(th->player && netgame && !deathmatch)
	{ // don't aim at fellow co-op players
		return true;
	}

// check angles to see if the thing can be aimed at

	dist = FixedMul (attackrange, in->frac);
	thingtopslope = FixedDiv (th->z+th->height - shootz , dist);
	if (thingtopslope < bottomslope)
		return true;            // shot over the thing
	thingbottomslope = FixedDiv (th->z - shootz, dist);
	if (thingbottomslope > topslope)
		return true;            // shot under the thing

//
// this thing can be hit!
//
	if (thingtopslope > topslope)
		thingtopslope = topslope;
	if (thingbottomslope < bottomslope)
		thingbottomslope = bottomslope;

	aimslope = (thingtopslope+thingbottomslope)/2;
	linetarget = th;

	return false;                   // don't go any farther
}


/*
==============================================================================
=
= PTR_ShootTraverse
=
==============================================================================
*/

boolean         PTR_ShootTraverse (intercept_t *in)
{
	fixed_t         x,y,z;
	fixed_t         frac;
	line_t          *li;
	mobj_t          *th;
	fixed_t         slope;
	fixed_t         dist;
	fixed_t         thingtopslope, thingbottomslope;

	extern mobj_t LavaInflictor;

	if (in->isaline)
	{
		li = in->d.line;
		if (li->special)
		{
			P_ActivateLine(li, shootthing, 0, SPAC_IMPACT);
//			P_ShootSpecialLine (shootthing, li);
		}
		if ( !(li->flags & ML_TWOSIDED) )
			goto hitline;

//
// crosses a two sided line
//
		P_LineOpening (li);

		dist = FixedMul (attackrange, in->frac);

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

		return true;            // shot continues
//
// hit line
//
hitline:
		// position a bit closer
		frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
		x = trace.x + FixedMul (trace.dx, frac);
		y = trace.y + FixedMul (trace.dy, frac);
		z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

		if (li->frontsector->ceilingpic == skyflatnum)
		{
			if (z > li->frontsector->ceilingheight)
				return false;           // don't shoot the sky!
			if (li->backsector && li->backsector->ceilingpic == skyflatnum)
				return false;           // it's a sky hack wall
		}

		P_SpawnPuff (x,y,z);
		return false;                   // don't go any farther
	}

//
// shoot a thing
//
	th = in->d.thing;
	if (th == shootthing)
		return true;            // can't shoot self
	if (!(th->flags&MF_SHOOTABLE))
		return true;            // corpse or something

//
// check for physical attacks on a ghost
//
/*  FIX:  Impliment Heretic 2 weapons here
	if(th->flags&MF_SHADOW && shootthing->player->readyweapon == wp_staff)
	{
		return(true);
	}
*/

// check angles to see if the thing can be aimed at
	dist = FixedMul (attackrange, in->frac);
	thingtopslope = FixedDiv (th->z+th->height - shootz , dist);
	if (thingtopslope < aimslope)
		return true;            // shot over the thing
	thingbottomslope = FixedDiv (th->z - shootz, dist);
	if (thingbottomslope > aimslope)
		return true;            // shot under the thing

//
// hit thing
//
	// position a bit closer
	frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);
	x = trace.x + FixedMul(trace.dx, frac);
	y = trace.y + FixedMul(trace.dy, frac);
	z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));
	P_SpawnPuff(x, y, z);
	if(la_damage)
	{
		if (!(in->d.thing->flags&MF_NOBLOOD) &&
			!(in->d.thing->flags2&MF2_INVULNERABLE))
		{
			if(PuffType == MT_AXEPUFF || PuffType == MT_AXEPUFF_GLOW)
			{
				P_BloodSplatter2(x, y, z, in->d.thing);
			}
			if(P_Random() < 192)
			{
				P_BloodSplatter(x, y, z, in->d.thing);
			}
		}
		if(PuffType == MT_FLAMEPUFF2)
		{ // Cleric FlameStrike does fire damage
			P_DamageMobj(th, &LavaInflictor, shootthing, la_damage);
		}
		else
		{ 
			P_DamageMobj(th, shootthing, shootthing, la_damage);
		}
	}
	return(false); // don't go any farther
}

/*
=================
=
= P_AimLineAttack
=
=================
*/

fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, fixed_t distance)
{
	fixed_t         x2, y2;

	angle >>= ANGLETOFINESHIFT;
	shootthing = t1;
	x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
	y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
	shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
	topslope = 100*FRACUNIT/160;    // can't shoot outside view angles
	bottomslope = -100*FRACUNIT/160;
	attackrange = distance;
	linetarget = NULL;

	P_PathTraverse ( t1->x, t1->y, x2, y2
		, PT_ADDLINES|PT_ADDTHINGS, PTR_AimTraverse );

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

void P_LineAttack (mobj_t *t1, angle_t angle, fixed_t distance, fixed_t slope, int damage)
{
	fixed_t         x2, y2;

	angle >>= ANGLETOFINESHIFT;
	shootthing = t1;
	la_damage = damage;
	x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
	y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
	shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
	shootz -= t1->floorclip;
	attackrange = distance;
	aimslope = slope;

	if(P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS,
		PTR_ShootTraverse))
	{
		switch(PuffType)
		{
			case MT_PUNCHPUFF:
				S_StartSound(t1, SFX_FIGHTER_PUNCH_MISS);
				break;
			case MT_HAMMERPUFF:
			case MT_AXEPUFF:
			case MT_AXEPUFF_GLOW:
				S_StartSound(t1, SFX_FIGHTER_HAMMER_MISS);
				break;
			case MT_FLAMEPUFF:
				P_SpawnPuff(x2, y2, shootz+FixedMul(slope, distance));
				break;
			default:
				break;
		}
	}
}

/*
==============================================================================

							USE LINES

==============================================================================
*/

mobj_t          *usething;

boolean         PTR_UseTraverse (intercept_t *in)
{
	int sound;
	fixed_t pheight;

	if (!in->d.line->special)
	{
		P_LineOpening (in->d.line);
		if (openrange <= 0)
		{
			if(usething->player)
			{
				switch(usething->player->class)
				{
					case PCLASS_FIGHTER:
						sound = SFX_PLAYER_FIGHTER_FAILED_USE;
						break;
					case PCLASS_CLERIC:
						sound = SFX_PLAYER_CLERIC_FAILED_USE;
						break;
					case PCLASS_MAGE:
						sound = SFX_PLAYER_MAGE_FAILED_USE;
						break;
					case PCLASS_PIG:
						sound = SFX_PIG_ACTIVE1;
						break;
					default:
						sound = SFX_NONE;
						break;
				}
				S_StartSound(usething, sound);
			}
			return false;   // can't use through a wall
		}
		if(usething->player)
		{
			pheight = usething->z + (usething->height/2);
			if ((opentop < pheight) || (openbottom > pheight))
			{
				switch(usething->player->class)
				{
					case PCLASS_FIGHTER:
						sound = SFX_PLAYER_FIGHTER_FAILED_USE;
						break;
					case PCLASS_CLERIC:
						sound = SFX_PLAYER_CLERIC_FAILED_USE;
						break;
					case PCLASS_MAGE:
						sound = SFX_PLAYER_MAGE_FAILED_USE;
						break;
					case PCLASS_PIG:
						sound = SFX_PIG_ACTIVE1;
						break;
					default:
						sound = SFX_NONE;
						break;
				}
				S_StartSound(usething, sound);
			}
		}
		return true ;           // not a special line, but keep checking
	}

	if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
		return false;           // don't use back sides

//	P_UseSpecialLine (usething, in->d.line);
	P_ActivateLine(in->d.line, usething, 0, SPAC_USE);

	return false;                   // can't use for than one special line in a row
}


/*
================
=
= P_UseLines
=
= Looks for special lines in front of the player to activate
================
*/

void P_UseLines (player_t *player)
{
	int                     angle;
	fixed_t         x1, y1, x2, y2;

	usething = player->mo;

	angle = player->mo->angle >> ANGLETOFINESHIFT;
	x1 = player->mo->x;
	y1 = player->mo->y;
	x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
	y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

	P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
}

//==========================================================================
//
// PTR_PuzzleItemTraverse
//
//==========================================================================

#define USE_PUZZLE_ITEM_SPECIAL 129

static mobj_t *PuzzleItemUser;
static int PuzzleItemType;
static boolean PuzzleActivated;

boolean PTR_PuzzleItemTraverse(intercept_t *in)
{
	mobj_t *mobj;
	int sound;

	if(in->isaline)
	{ // Check line
		if(in->d.line->special != USE_PUZZLE_ITEM_SPECIAL)
		{
			P_LineOpening(in->d.line);
			if(openrange <= 0)
			{
				sound = SFX_NONE;
				if(PuzzleItemUser->player)
				{
					switch(PuzzleItemUser->player->class)
					{
						case PCLASS_FIGHTER:
							sound = SFX_PUZZLE_FAIL_FIGHTER;
							break;
						case PCLASS_CLERIC:
							sound = SFX_PUZZLE_FAIL_CLERIC;
							break;
						case PCLASS_MAGE:
							sound = SFX_PUZZLE_FAIL_MAGE;
							break;
						default:
							sound = SFX_NONE;
							break;
					}
				}
				S_StartSound(PuzzleItemUser, sound);
				return false; // can't use through a wall
			}
			return true; // Continue searching
		}
		if(P_PointOnLineSide(PuzzleItemUser->x, PuzzleItemUser->y,
			in->d.line) == 1)
		{ // Don't use back sides
			return false;
		}
		if(PuzzleItemType != in->d.line->arg1)
		{ // Item type doesn't match
			return false;
		}
		P_StartACS(in->d.line->arg2, 0, &in->d.line->arg3,
			PuzzleItemUser, in->d.line, 0);
		in->d.line->special = 0;
		PuzzleActivated = true;
		return false; // Stop searching
	}
	// Check thing
	mobj = in->d.thing;
	if(mobj->special != USE_PUZZLE_ITEM_SPECIAL)
	{ // Wrong special
		return true;
	}
	if(PuzzleItemType != mobj->args[0])
	{ // Item type doesn't match
		return true;
	}
	P_StartACS(mobj->args[1], 0, &mobj->args[2], PuzzleItemUser, NULL, 0);
	mobj->special = 0;
	PuzzleActivated = true;
	return false; // Stop searching
}

//==========================================================================
//
// P_UsePuzzleItem
//
// Returns true if the puzzle item was used on a line or a thing.
//
//==========================================================================

boolean P_UsePuzzleItem(player_t *player, int itemType)
{
	int angle;
	fixed_t x1, y1, x2, y2;

	PuzzleItemType = itemType;
	PuzzleItemUser = player->mo;
	PuzzleActivated = false;
	angle = player->mo->angle>>ANGLETOFINESHIFT;
	x1 = player->mo->x;
	y1 = player->mo->y;
	x2 = x1+(USERANGE>>FRACBITS)*finecosine[angle];
	y2 = y1+(USERANGE>>FRACBITS)*finesine[angle];
	P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES|PT_ADDTHINGS,
		PTR_PuzzleItemTraverse);
	return PuzzleActivated;
}

/*
==============================================================================

							RADIUS ATTACK

==============================================================================
*/

mobj_t *bombsource;
mobj_t *bombspot;
int bombdamage;
int bombdistance;
boolean DamageSource;

/*
=================
=
= PIT_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

boolean PIT_RadiusAttack (mobj_t *thing)
{
	fixed_t dx, dy, dist;
	int damage;

	if(!(thing->flags&MF_SHOOTABLE))
	{
		return true;
	}
//	if(thing->flags2&MF2_BOSS)
//	{	// Bosses take no damage from PIT_RadiusAttack
//		return(true);
//	}
	if(!DamageSource && thing == bombsource)
	{ // don't damage the source of the explosion
		return true;
	}
	if(abs((thing->z-bombspot->z)>>FRACBITS) > 2*bombdistance)
	{ // too high/low
		return true;
	}
	dx = abs(thing->x-bombspot->x);
	dy = abs(thing->y-bombspot->y);
	dist = dx > dy ? dx : dy;
	dist = (dist-thing->radius)>>FRACBITS;
	if(dist < 0)
	{
		dist = 0;
	}
	if(dist >= bombdistance)
	{ // Out of range
		return true;
	}
	if(P_CheckSight(thing, bombspot))
	{ // OK to damage, target is in direct path
		damage = (bombdamage*(bombdistance-dist)/bombdistance)+1;
		if(thing->player)
		{
			damage >>= 2;
		}
		P_DamageMobj(thing, bombspot, bombsource, damage);
	}
	return(true);
}

/*
=================
=
= P_RadiusAttack
=
= Source is the creature that caused the explosion at spot
=================
*/

void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage, int distance,
	boolean damageSource)
{
	int                     x,y, xl, xh, yl, yh;
	fixed_t         dist;

	dist = (distance+MAXRADIUS)<<FRACBITS;
	yh = (spot->y+dist-bmaporgy)>>MAPBLOCKSHIFT;
	yl = (spot->y-dist-bmaporgy)>>MAPBLOCKSHIFT;
	xh = (spot->x+dist-bmaporgx)>>MAPBLOCKSHIFT;
	xl = (spot->x-dist-bmaporgx)>>MAPBLOCKSHIFT;
	bombspot = spot;
	bombsource = source;
	bombdamage = damage;
	bombdistance = distance;
	DamageSource = damageSource;
	for (y = yl; y <= yh; y++)
	{
		for (x = xl; x <= xh; x++)
		{
			P_BlockThingsIterator(x, y, PIT_RadiusAttack);
		}
	}
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

int         crushchange;
boolean         nofit;

/*
===============
=
= PIT_ChangeSector
=
===============
*/

boolean PIT_ChangeSector (mobj_t *thing)
{
	mobj_t          *mo;

	if (P_ThingHeightClip (thing))
		return true;            // keep checking

	// crunch bodies to giblets
	if ((thing->flags&MF_CORPSE) && (thing->health <= 0))
	{
		if (thing->flags&MF_NOBLOOD)
		{
			P_RemoveMobj (thing);
		}
		else
		{
			if (thing->state != &states[S_GIBS1])
			{
				P_SetMobjState (thing, S_GIBS1);
				thing->height = 0;
				thing->radius = 0;
				S_StartSound(thing, SFX_PLAYER_FALLING_SPLAT);
			}
		}
		return true;            // keep checking
	}

	// crunch dropped items
	if (thing->flags2&MF2_DROPPED)
	{
		P_RemoveMobj (thing);
		return true;            // keep checking
	}

	if (! (thing->flags & MF_SHOOTABLE) )
		return true;                            // assume it is bloody gibs or something

	nofit = true;
	if (crushchange && !(leveltime&3))
	{
		P_DamageMobj(thing, NULL, NULL, crushchange);
		// spray blood in a random direction
		if ((!(thing->flags&MF_NOBLOOD)) &&
			(!(thing->flags2&MF2_INVULNERABLE)))
		{
			mo = P_SpawnMobj (thing->x, thing->y, thing->z + thing->height/2, 
				MT_BLOOD);
			mo->momx = (P_Random() - P_Random ())<<12;
			mo->momy = (P_Random() - P_Random ())<<12;
		}
	}

	return true;            // keep checking (crush other things)
}

/*
===============
=
= P_ChangeSector
=
===============
*/

boolean P_ChangeSector (sector_t *sector, int crunch)
{
	int                     x,y;

	nofit = false;
	crushchange = crunch;

// recheck heights for all things near the moving sector

	for (x=sector->blockbox[BOXLEFT] ; x<= sector->blockbox[BOXRIGHT] ; x++)
		for (y=sector->blockbox[BOXBOTTOM];y<= sector->blockbox[BOXTOP] ; y++)
			P_BlockThingsIterator (x, y, PIT_ChangeSector);


	return nofit;
}

