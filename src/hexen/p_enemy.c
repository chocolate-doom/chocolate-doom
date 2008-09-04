
//**************************************************************************
//**
//** p_enemy.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_enemy.c,v $
//** $Revision: 1.170 $
//** $Date: 96/01/06 03:23:28 $
//** $Author: bgokey $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// Macros
// Types
// Private Data
// External Data
extern fixed_t FloatBobOffsets[64];


//----------------------------------------------------------------------------
//
// PROC P_RecursiveSound
//
//----------------------------------------------------------------------------

mobj_t *soundtarget;

void P_RecursiveSound(sector_t *sec, int soundblocks)
{
	int i;
	line_t *check;
	sector_t *other;

	// Wake up all monsters in this sector
	if(sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
	{ // Already flooded
		return;
	}
	sec->validcount = validcount;
	sec->soundtraversed = soundblocks+1;
	sec->soundtarget = soundtarget;
	for(i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		if(!(check->flags&ML_TWOSIDED))
		{
			continue;
		}
		P_LineOpening(check);
		if(openrange <= 0)
		{ // Closed door
			continue;
		}
		if(sides[check->sidenum[0]].sector == sec)
		{
			other = sides[check->sidenum[1]].sector;
		}
		else
		{
			other = sides[check->sidenum[0]].sector;
		}
		if(check->flags&ML_SOUNDBLOCK)
		{
			if(!soundblocks)
			{
				P_RecursiveSound(other, 1);
			}
		}
		else
		{
			P_RecursiveSound(other, soundblocks);
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC P_NoiseAlert
//
// If a monster yells at a player, it will alert other monsters to the
// player.
//
//----------------------------------------------------------------------------

void P_NoiseAlert(mobj_t *target, mobj_t *emmiter)
{
	soundtarget = target;
	validcount++;
	P_RecursiveSound(emmiter->subsector->sector, 0);
}

//----------------------------------------------------------------------------
//
// FUNC P_CheckMeleeRange
//
//----------------------------------------------------------------------------

boolean P_CheckMeleeRange(mobj_t *actor)
{
	mobj_t *mo;
	fixed_t dist;

	if(!actor->target)
	{
		return(false);
	}
	mo = actor->target;
	dist = P_AproxDistance(mo->x-actor->x, mo->y-actor->y);
	if(dist >= MELEERANGE)
	{
		return(false);
	}
	if(!P_CheckSight(actor, mo))
	{
		return(false);
	}
	if(mo->z > actor->z+actor->height)
	{ // Target is higher than the attacker
		return(false);
	}
	else if(actor->z > mo->z+mo->height)
	{ // Attacker is higher
		return(false);
	}
	return(true);
}

//----------------------------------------------------------------------------
//
// FUNC P_CheckMeleeRange2
//
//----------------------------------------------------------------------------

boolean P_CheckMeleeRange2(mobj_t *actor)
{
	mobj_t *mo;
	fixed_t dist;

	if(!actor->target)
	{
		return(false);
	}
	mo = actor->target;
	dist = P_AproxDistance(mo->x-actor->x, mo->y-actor->y);
	if(dist >= MELEERANGE*2 || dist < MELEERANGE)
	{
		return(false);
	}
	if(!P_CheckSight(actor, mo))
	{
		return(false);
	}
	if(mo->z > actor->z+actor->height)
	{ // Target is higher than the attacker
		return(false);
	}
	else if(actor->z > mo->z+mo->height)
	{ // Attacker is higher
		return(false);
	}
	return(true);
}

//----------------------------------------------------------------------------
//
// FUNC P_CheckMissileRange
//
//----------------------------------------------------------------------------

boolean P_CheckMissileRange(mobj_t *actor)
{
	fixed_t dist;

	if(!P_CheckSight(actor, actor->target))
	{
		return(false);
	}
	if(actor->flags&MF_JUSTHIT)
	{ // The target just hit the enemy, so fight back!
		actor->flags &= ~MF_JUSTHIT;
		return(true);
	}
	if(actor->reactiontime)
	{ // Don't attack yet
		return(false);
	}
	dist = (P_AproxDistance(actor->x-actor->target->x,
		actor->y-actor->target->y)>>FRACBITS)-64;
	if(!actor->info->meleestate)
	{ // No melee attack, so fire more frequently
		dist -= 128;
	}
	if(dist > 200)
	{
		dist = 200;
	}
	if(P_Random() < dist)
	{
		return(false);
	}
	return(true);
}

/*
================
=
= P_Move
=
= Move in the current direction
= returns false if the move is blocked
================
*/

fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

#define MAXSPECIALCROSS         8
extern  line_t  *spechit[MAXSPECIALCROSS];
extern  int                      numspechit;

boolean P_Move(mobj_t *actor)
{
	fixed_t tryx, tryy;
	line_t *ld;
	boolean good;

	if(actor->flags2&MF2_BLASTED) return(true);
	if(actor->movedir == DI_NODIR)
	{
		return(false);
	}
	tryx = actor->x+actor->info->speed*xspeed[actor->movedir];
	tryy = actor->y+actor->info->speed*yspeed[actor->movedir];
	if(!P_TryMove(actor, tryx, tryy))
	{ // open any specials
		if(actor->flags&MF_FLOAT && floatok)
		{ // must adjust height
			if(actor->z < tmfloorz)
			{
				actor->z += FLOATSPEED;
			}
			else
			{
				actor->z -= FLOATSPEED;
			}
			actor->flags |= MF_INFLOAT;
			return(true);
		}
		if(!numspechit)
		{
			return false;
		}
		actor->movedir = DI_NODIR;
		good = false;
		while(numspechit--)
		{
			ld = spechit[numspechit];
			// if the special isn't a door that can be opened, return false
			if(P_ActivateLine(ld, actor, 0, SPAC_USE))
			{
				good = true;
			}
/* Old version before use/cross/impact specials were combined
			if(P_UseSpecialLine(actor, ld))
			{
				good = true;
			}
*/
		}
		return(good);
	}
	else
	{
		actor->flags &= ~MF_INFLOAT;
	}
	if(!(actor->flags&MF_FLOAT))
	{
		if(actor->z > actor->floorz)
		{
			P_HitFloor(actor);
		}
		actor->z = actor->floorz;
	}
	return(true);
}

//----------------------------------------------------------------------------
//
// FUNC P_TryWalk
//
// Attempts to move actor in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor returns FALSE.
// If move is either clear of block only by a door, returns TRUE and sets.
// If a door is in the way, an OpenDoor call is made to start it opening.
//
//----------------------------------------------------------------------------

boolean P_TryWalk(mobj_t *actor)
{
	if(!P_Move(actor))
	{
		return(false);
	}
	actor->movecount = P_Random()&15;
	return(true);
}

/*
================
=
= P_NewChaseDir
=
================
*/

dirtype_t opposite[] =
{DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST, DI_EAST, DI_NORTHEAST,
DI_NORTH, DI_NORTHWEST, DI_NODIR};

dirtype_t diags[] = {DI_NORTHWEST,DI_NORTHEAST,DI_SOUTHWEST,DI_SOUTHEAST};

void P_NewChaseDir (mobj_t *actor)
{
	fixed_t         deltax,deltay;
	dirtype_t       d[3];
	dirtype_t       tdir, olddir, turnaround;

	if (!actor->target)
		I_Error ("P_NewChaseDir: called with no target");

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
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
		if (actor->movedir != turnaround && P_TryWalk(actor))
			return;
	}

// try other directions
	if (P_Random() > 200 ||  abs(deltay)>abs(deltax))
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
			return;     /*either moved forward or attacked*/
	}

	if (d[2]!=DI_NODIR)
	{
		actor->movedir =d[2];
		if (P_TryWalk(actor))
			return;
	}

/* there is no direct path to the player, so pick another direction */

	if (olddir!=DI_NODIR)
	{
		actor->movedir =olddir;
		if (P_TryWalk(actor))
			return;
	}

	if (P_Random()&1)       /*randomly determine direction of search*/
	{
		for (tdir=DI_EAST ; tdir<=DI_SOUTHEAST ; tdir++)
		{
			if (tdir!=turnaround)
			{
				actor->movedir =tdir;
				if ( P_TryWalk(actor) )
					return;
			}
		}
	}
	else
	{
		for (tdir=DI_SOUTHEAST ; tdir >= DI_EAST;tdir--)
		{
			if (tdir!=turnaround)
			{
				actor->movedir =tdir;
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

	actor->movedir = DI_NODIR;              // can't move
}

//---------------------------------------------------------------------------
//
// FUNC P_LookForMonsters
//
//---------------------------------------------------------------------------

#define MONS_LOOK_RANGE (16*64*FRACUNIT)
#define MONS_LOOK_LIMIT 64

boolean P_LookForMonsters(mobj_t *actor)
{
	int count;
	mobj_t *mo;
	thinker_t *think;

	if(!P_CheckSight(players[0].mo, actor))
	{ // Player can't see monster
		return(false);
	}
	count = 0;
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != P_MobjThinker)
		{ // Not a mobj thinker
			continue;
		}
		mo = (mobj_t *)think;
		if(!(mo->flags&MF_COUNTKILL) || (mo == actor) || (mo->health <= 0))
		{ // Not a valid monster
			continue;
		}
		if(P_AproxDistance(actor->x-mo->x, actor->y-mo->y)
			> MONS_LOOK_RANGE)
		{ // Out of range
			continue;
		}
		if(P_Random() < 16)
		{ // Skip
			continue;
		}
		if(count++ > MONS_LOOK_LIMIT)
		{ // Stop searching
			return(false);
		}
		if(!P_CheckSight(actor, mo))
		{ // Out of sight
			continue;
		}
		if (actor->type == MT_MINOTAUR)
		{
			if ((mo->type == MT_MINOTAUR) && 
				 (mo->target != ((player_t *)actor->special1)->mo))
			{
				continue;
			}
		}
		// Found a target monster
		actor->target = mo;
		return(true);
	}
	return(false);
}

/*
================
=
= P_LookForPlayers
=
= If allaround is false, only look 180 degrees in front
= returns true if a player is targeted
================
*/

boolean P_LookForPlayers(mobj_t *actor, boolean allaround)
{
	int c;
	int stop;
	player_t *player;
	sector_t *sector;
	angle_t an;
	fixed_t dist;

	if(!netgame && players[0].health <= 0)
	{ // Single player game and player is dead, look for monsters
		return(P_LookForMonsters(actor));
	}
	sector = actor->subsector->sector;
	c = 0;
	stop = (actor->lastlook-1)&3;
	for( ; ; actor->lastlook = (actor->lastlook+1)&3 )
	{
		if (!playeringame[actor->lastlook])
			continue;

		if (c++ == 2 || actor->lastlook == stop)
			return false;           // done looking

		player = &players[actor->lastlook];
		if (player->health <= 0)
			continue;               // dead
		if (!P_CheckSight (actor, player->mo))
			continue;               // out of sight

		if (!allaround)
		{
			an = R_PointToAngle2 (actor->x, actor->y,
			player->mo->x, player->mo->y) - actor->angle;
			if (an > ANG90 && an < ANG270)
			{
				dist = P_AproxDistance (player->mo->x - actor->x,
					player->mo->y - actor->y);
				// if real close, react anyway
				if (dist > MELEERANGE)
					continue;               // behind back
			}
		}
		if(player->mo->flags&MF_SHADOW)
		{ // Player is invisible
			if((P_AproxDistance(player->mo->x-actor->x,
				player->mo->y-actor->y) > 2*MELEERANGE)
				&& P_AproxDistance(player->mo->momx, player->mo->momy)
				< 5*FRACUNIT)
			{ // Player is sneaking - can't detect
				return(false);
			}
			if(P_Random() < 225)
			{ // Player isn't sneaking, but still didn't detect
				return(false);
			}
		}
		if (actor->type == MT_MINOTAUR)
		{
			if(((player_t *)(actor->special1)) == player)
			{
				continue;			// Don't target master
			}
		}

		actor->target = player->mo;
		return(true);
	}
	return(false);
}

/*
===============================================================================

						ACTION ROUTINES

===============================================================================
*/

/*
==============
=
= A_Look
=
= Stay in state until a player is sighted
=
==============
*/

void A_Look (mobj_t *actor)
{
	mobj_t          *targ;

	actor->threshold = 0;           // any shot will wake up
	targ = actor->subsector->sector->soundtarget;
	if (targ && (targ->flags & MF_SHOOTABLE) )
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


	if (!P_LookForPlayers (actor, false) )
		return;

// go into chase state
seeyou:
	if (actor->info->seesound)
	{
		int             sound;

		sound = actor->info->seesound;
		if(actor->flags2&MF2_BOSS)
		{ // Full volume
			S_StartSound(NULL, sound);
		}
		else
		{
			S_StartSound(actor, sound);
		}
	}
	P_SetMobjState(actor, actor->info->seestate);
}


/*
==============
=
= A_Chase
=
= Actor has a melee attack, so it tries to close as fast as possible
=
==============
*/

void A_Chase(mobj_t *actor)
{
	int delta;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if(gameskill == sk_nightmare)
	{ // Monsters move faster in nightmare mode
		actor->tics -= actor->tics/2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

//
// turn towards movement direction if not there yet
//
	if(actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle-(actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90/2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90/2;
		}
	}

	if(!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{ // look for a new target
		if(P_LookForPlayers(actor, true))
		{ // got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

//
// don't attack twice in a row
//
	if(actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		if (gameskill != sk_nightmare)
			P_NewChaseDir (actor);
		return;
	}

//
// check for melee attack
//
	if (actor->info->meleestate && P_CheckMeleeRange (actor))
	{
		if(actor->info->attacksound)
		{
			S_StartSound (actor, actor->info->attacksound);
		}
		P_SetMobjState (actor, actor->info->meleestate);
		return;
	}

//
// check for missile attack
//
	if (actor->info->missilestate)
	{
		if (gameskill < sk_nightmare && actor->movecount)
			goto nomissile;
		if (!P_CheckMissileRange (actor))
			goto nomissile;
		P_SetMobjState (actor, actor->info->missilestate);
		actor->flags |= MF_JUSTATTACKED;
		return;
	}
nomissile:

//
// possibly choose another target
//
	if (netgame && !actor->threshold && !P_CheckSight (actor, actor->target) )
	{
		if (P_LookForPlayers(actor,true))
			return;         // got a new target
	}

//
// chase towards player
//
	if (--actor->movecount<0 || !P_Move (actor))
	{
		P_NewChaseDir (actor);
	}

//
// make active sound
//
	if(actor->info->activesound && P_Random() < 3)
	{
		if(actor->type == MT_BISHOP && P_Random() < 128)
		{
			S_StartSound(actor, actor->info->seesound);
		}
		else if(actor->type == MT_PIG)
		{
			S_StartSound(actor, SFX_PIG_ACTIVE1+(P_Random()&1));
		}
		else if(actor->flags2&MF2_BOSS)
		{
			S_StartSound(NULL, actor->info->activesound);
		}
		else
		{
			S_StartSound(actor, actor->info->activesound);
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FaceTarget
//
//----------------------------------------------------------------------------

void A_FaceTarget(mobj_t *actor)
{
	if(!actor->target)
	{
		return;
	}
	actor->flags &= ~MF_AMBUSH;
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x,
		actor->target->y);
	if(actor->target->flags&MF_SHADOW)
	{ // Target is a ghost
		actor->angle += (P_Random()-P_Random())<<21;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_Pain
//
//----------------------------------------------------------------------------

void A_Pain(mobj_t *actor)
{
	if(actor->info->painsound)
	{
		S_StartSound(actor, actor->info->painsound);
	}
}

//============================================================================
//
// A_SetInvulnerable
//
//============================================================================

void A_SetInvulnerable(mobj_t *actor)
{
	actor->flags2 |= MF2_INVULNERABLE;
}

//============================================================================
//
// A_UnSetInvulnerable
//
//============================================================================

void A_UnSetInvulnerable(mobj_t *actor)
{
	actor->flags2 &= ~MF2_INVULNERABLE;
}

//============================================================================
//
// A_SetReflective
//
//============================================================================

void A_SetReflective(mobj_t *actor)
{
	actor->flags2 |= MF2_REFLECTIVE;

	if ((actor->type == MT_CENTAUR) ||
		(actor->type == MT_CENTAURLEADER))
	{
		A_SetInvulnerable(actor);
	}
}

//============================================================================
//
// A_UnSetReflective
//
//============================================================================

void A_UnSetReflective(mobj_t *actor)
{
	actor->flags2 &= ~MF2_REFLECTIVE;

	if ((actor->type == MT_CENTAUR) ||
		(actor->type == MT_CENTAURLEADER))
	{
		A_UnSetInvulnerable(actor);
	}
}


//----------------------------------------------------------------------------
//
// FUNC P_UpdateMorphedMonster
//
// Returns true if the pig morphs.
//
//----------------------------------------------------------------------------

boolean P_UpdateMorphedMonster(mobj_t *actor, int tics)
{
	mobj_t *fog;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	mobjtype_t moType;
	mobj_t *mo;
	mobj_t oldMonster;

	actor->special1 -= tics;
	if(actor->special1 > 0)
	{
		return(false);
	}
	moType = actor->special2;
	switch(moType)
	{
		case MT_WRAITHB:			// These must remain morphed
		case MT_SERPENT:
		case MT_SERPENTLEADER:
		case MT_MINOTAUR:
			return(false);
		default:
			break;
	}
	x = actor->x;
	y = actor->y;
	z = actor->z;
	oldMonster = *actor;			// Save pig vars

	P_RemoveMobjFromTIDList(actor);
	P_SetMobjState(actor, S_FREETARGMOBJ);
	mo = P_SpawnMobj(x, y, z, moType);

	if(P_TestMobjLocation(mo) == false)
	{ // Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, oldMonster.type);
		mo->angle = oldMonster.angle;
		mo->flags = oldMonster.flags;
		mo->health = oldMonster.health;
		mo->target = oldMonster.target;
		mo->special = oldMonster.special;
		mo->special1 = 5*35; // Next try in 5 seconds
		mo->special2 = moType;
		mo->tid = oldMonster.tid;
		memcpy(mo->args, oldMonster.args, 5);
		P_InsertMobjIntoTIDList(mo, oldMonster.tid);
		return(false);
	}
	mo->angle = oldMonster.angle;
	mo->target = oldMonster.target;
	mo->tid = oldMonster.tid;
	mo->special = oldMonster.special;
	memcpy(mo->args, oldMonster.args, 5);
	P_InsertMobjIntoTIDList(mo, oldMonster.tid);
	fog = P_SpawnMobj(x, y, z+TELEFOGHEIGHT, MT_TFOG);
	S_StartSound(fog, SFX_TELEPORT);
	return(true);
}

//----------------------------------------------------------------------------
//
// PROC A_PigLook
//
//----------------------------------------------------------------------------

void A_PigLook(mobj_t *actor)
{
	if(P_UpdateMorphedMonster(actor, 10))
	{
		return;
	}
	A_Look(actor);
}

//----------------------------------------------------------------------------
//
// PROC A_PigChase
//
//----------------------------------------------------------------------------

void A_PigChase(mobj_t *actor)
{
	if(P_UpdateMorphedMonster(actor, 3))
	{
		return;
	}
	A_Chase(actor);
}

//============================================================================
//
// A_PigAttack
//
//============================================================================

void A_PigAttack(mobj_t *actor)
{
	if(P_UpdateMorphedMonster(actor, 18))
	{
		return;
	}
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, 2+(P_Random()&1));
		S_StartSound(actor, SFX_PIG_ATTACK);
	}
}

//============================================================================
//
// A_PigPain
//
//============================================================================

void A_PigPain(mobj_t *actor)
{
	A_Pain(actor);
	if(actor->z <= actor->floorz)
	{
		actor->momz = 3.5*FRACUNIT;
	}
}



void FaceMovementDirection(mobj_t *actor)
{
	switch(actor->movedir)
	{
		case DI_EAST:
			actor->angle = 0<<24;
			break;
		case DI_NORTHEAST:
			actor->angle = 32<<24;
			break;
		case DI_NORTH:
			actor->angle = 64<<24;
			break;
		case DI_NORTHWEST:
			actor->angle = 96<<24;
			break;
		case DI_WEST:
			actor->angle = 128<<24;
			break;
		case DI_SOUTHWEST:
			actor->angle = 160<<24;
			break;
		case DI_SOUTH:
			actor->angle = 192<<24;
			break;
		case DI_SOUTHEAST:
			actor->angle = 224<<24;
			break;
	}
}


//----------------------------------------------------------------------------
//
// Minotaur variables
//
// 	special1		pointer to player that spawned it (mobj_t)
//	special2		internal to minotaur AI
//	args[0]			args[0]-args[3] together make up minotaur start time
//	args[1]			|
//	args[2]			|
//	args[3]			V
//	args[4]			charge duration countdown
//----------------------------------------------------------------------------

void A_MinotaurFade0(mobj_t *actor)
{
	actor->flags &= ~MF_ALTSHADOW;
	actor->flags |= MF_SHADOW;
}

void A_MinotaurFade1(mobj_t *actor)
{
	// Second level of transparency
	actor->flags &= ~MF_SHADOW;
	actor->flags |= MF_ALTSHADOW;
}

void A_MinotaurFade2(mobj_t *actor)
{
	// Make fully visible
	actor->flags &= ~MF_SHADOW;
	actor->flags &= ~MF_ALTSHADOW;
}


//----------------------------------------------------------------------------
//
// A_MinotaurRoam - 
//
// 
//----------------------------------------------------------------------------

void A_MinotaurLook(mobj_t *actor);

void A_MinotaurRoam(mobj_t *actor)
{
	unsigned int *starttime = (unsigned int *)actor->args;

	actor->flags &= ~MF_SHADOW;			// In case pain caused him to 
	actor->flags &= ~MF_ALTSHADOW;		// skip his fade in.

	if ((leveltime - *starttime) >= MAULATORTICS)
	{
		P_DamageMobj(actor,NULL,NULL,10000);
		return;
	}

	if (P_Random()<30)
		A_MinotaurLook(actor);		// adjust to closest target

	if (P_Random()<6)
	{
		//Choose new direction
		actor->movedir = P_Random() % 8;
		FaceMovementDirection(actor);
	}
	if (!P_Move(actor))
	{
		// Turn
		if (P_Random() & 1)
			actor->movedir = (++actor->movedir)%8;
		else
			actor->movedir = (actor->movedir+7)%8;
		FaceMovementDirection(actor);
	}
}


//----------------------------------------------------------------------------
//
//	PROC A_MinotaurLook
//
// Look for enemy of player
//----------------------------------------------------------------------------
#define MINOTAUR_LOOK_DIST		(16*54*FRACUNIT)

void A_MinotaurLook(mobj_t *actor)
{
	mobj_t *mo=NULL;
	player_t *player;
	thinker_t *think;
	fixed_t dist;
	int i;
	mobj_t *master = (mobj_t *)(actor->special1);

	actor->target = NULL;
	if (deathmatch)					// Quick search for players
	{
    	for (i=0; i<MAXPLAYERS; i++)
		{
			if (!playeringame[i]) continue;
			player = &players[i];
			mo = player->mo;
			if (mo == master) continue;
			if (mo->health <= 0) continue;
			dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
			if (dist > MINOTAUR_LOOK_DIST) continue;
			actor->target = mo;
			break;
		}
	}

	if (!actor->target)				// Near player monster search
	{
		if (master && (master->health>0) && (master->player))
			mo = P_RoughMonsterSearch(master, 20);
		else
			mo = P_RoughMonsterSearch(actor, 20);
		actor->target = mo;
	}

	if (!actor->target)				// Normal monster search
	{
		for(think = thinkercap.next; think != &thinkercap; think = think->next)
		{
			if(think->function != P_MobjThinker) continue;
			mo = (mobj_t *)think;
			if (!(mo->flags&MF_COUNTKILL)) continue;
			if (mo->health <= 0) continue;
			if (!(mo->flags&MF_SHOOTABLE)) continue;
			dist = P_AproxDistance(actor->x - mo->x, actor->y - mo->y);
			if (dist > MINOTAUR_LOOK_DIST) continue;
			if ((mo == master) || (mo == actor)) continue;
			if ((mo->type == MT_MINOTAUR) &&
				(mo->special1 == actor->special1)) continue;
			actor->target = mo;
			break;			// Found mobj to attack
		}
	}

	if (actor->target)
	{
		P_SetMobjStateNF(actor, S_MNTR_WALK1);
	}
	else
	{
		P_SetMobjStateNF(actor, S_MNTR_ROAM1);
	}
}




void A_MinotaurChase(mobj_t *actor)
{
	unsigned int *starttime = (unsigned int *)actor->args;

	actor->flags &= ~MF_SHADOW;			// In case pain caused him to 
	actor->flags &= ~MF_ALTSHADOW;		// skip his fade in.

	if ((leveltime - *starttime) >= MAULATORTICS)
	{
		P_DamageMobj(actor,NULL,NULL,10000);
		return;
	}

	if (P_Random()<30)
		A_MinotaurLook(actor);		// adjust to closest target

	if (!actor->target || (actor->target->health <= 0) ||
		!(actor->target->flags&MF_SHOOTABLE))
	{ // look for a new target
		P_SetMobjState(actor, S_MNTR_LOOK1);
		return;
	}

	FaceMovementDirection(actor);
	actor->reactiontime=0;

	// Melee attack
	if (actor->info->meleestate && P_CheckMeleeRange(actor))
	{
		if(actor->info->attacksound)
		{
			S_StartSound (actor, actor->info->attacksound);
		}
		P_SetMobjState (actor, actor->info->meleestate);
		return;
	}

	// Missile attack
	if (actor->info->missilestate && P_CheckMissileRange(actor))
	{
		P_SetMobjState (actor, actor->info->missilestate);
		return;
	}

	// chase towards target
	if (!P_Move(actor))
	{
		P_NewChaseDir(actor);
	}

	// Active sound
	if(actor->info->activesound && P_Random() < 6)
	{
		S_StartSound(actor, actor->info->activesound);
	}

}


//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk1
//
// Melee attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk1(mobj_t *actor)
{
	if (!actor->target) return;

	S_StartSound(actor, SFX_MAULATOR_HAMMER_SWING);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurDecide
//
// Choose a missile attack.
//
//----------------------------------------------------------------------------

#define MNTR_CHARGE_SPEED (23*FRACUNIT)

void A_MinotaurDecide(mobj_t *actor)
{
	angle_t angle;
	mobj_t *target = actor->target;
	int dist;

	if (!target) return;
	dist = P_AproxDistance(actor->x-target->x, actor->y-target->y);

	if(target->z+target->height > actor->z
		&& target->z+target->height < actor->z+actor->height
		&& dist < 16*64*FRACUNIT
		&& dist > 1*64*FRACUNIT
		&& P_Random() < 230)
	{ // Charge attack
		// Don't call the state function right away
		P_SetMobjStateNF(actor, S_MNTR_ATK4_1);
		actor->flags |= MF_SKULLFLY;
		A_FaceTarget(actor);
		angle = actor->angle>>ANGLETOFINESHIFT;
		actor->momx = FixedMul(MNTR_CHARGE_SPEED, finecosine[angle]);
		actor->momy = FixedMul(MNTR_CHARGE_SPEED, finesine[angle]);
		actor->args[4] = 35/2; // Charge duration
	}
	else if(target->z == target->floorz
		&& dist < 9*64*FRACUNIT
		&& P_Random() < 100)
	{ // Floor fire attack
		P_SetMobjState(actor, S_MNTR_ATK3_1);
		actor->special2 = 0;
	}
	else
	{ // Swing attack
		A_FaceTarget(actor);
		// Don't need to call P_SetMobjState because the current state
		// falls through to the swing attack
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurCharge
//
//----------------------------------------------------------------------------

void A_MinotaurCharge(mobj_t *actor)
{
	mobj_t *puff;

	if (!actor->target) return;

	if(actor->args[4] > 0)
	{
		puff = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PUNCHPUFF);
		puff->momz = 2*FRACUNIT;
		actor->args[4]--;
	}
	else
	{
		actor->flags &= ~MF_SKULLFLY;
		P_SetMobjState(actor, actor->info->seestate);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk2
//
// Swing attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk2(mobj_t *actor)
{
	mobj_t *mo;
	angle_t angle;
	fixed_t momz;

	if(!actor->target) return;

	S_StartSound(actor, SFX_MAULATOR_HAMMER_SWING);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(3));
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MT_MNTRFX1);
	if(mo)
	{
		//S_StartSound(mo, sfx_minat2);
		momz = mo->momz;
		angle = mo->angle;
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle-(ANG45/8), momz);
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle+(ANG45/8), momz);
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle-(ANG45/16), momz);
		P_SpawnMissileAngle(actor, MT_MNTRFX1, angle+(ANG45/16), momz);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk3
//
// Floor fire attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk3(mobj_t *actor)
{
	mobj_t *mo;
	player_t *player;

	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(3));
		if((player = actor->target->player) != NULL)
		{ // Squish the player
			player->deltaviewheight = -16*FRACUNIT;
		}
	}
	else
	{
		mo = P_SpawnMissile(actor, actor->target, MT_MNTRFX2);
		if(mo != NULL)
		{
			S_StartSound(mo, SFX_MAULATOR_HAMMER_HIT);
		}
	}
	if(P_Random() < 192 && actor->special2 == 0)
	{
		P_SetMobjState(actor, S_MNTR_ATK3_4);
		actor->special2 = 1;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MntrFloorFire
//
//----------------------------------------------------------------------------

void A_MntrFloorFire(mobj_t *actor)
{
	mobj_t *mo;

	actor->z = actor->floorz;
	mo = P_SpawnMobj(actor->x+((P_Random()-P_Random())<<10),
		actor->y+((P_Random()-P_Random())<<10), ONFLOORZ, MT_MNTRFX3);
	mo->target = actor->target;
	mo->momx = 1; // Force block checking
	P_CheckMissileSpawn(mo);
}


//----------------------------------------------------------------------------
//
// PROC A_Scream
//
//----------------------------------------------------------------------------

void A_Scream(mobj_t *actor)
{
	int sound;

	S_StopSound(actor);
	if(actor->player)
	{
		if(actor->player->morphTics)
		{
			S_StartSound(actor, actor->info->deathsound);
		}
		else
		{
			// Handle the different player death screams
			if(actor->momz <= -39*FRACUNIT)
			{ // Falling splat
				sound = SFX_PLAYER_FALLING_SPLAT;
			}
			else if(actor->health > -50)
			{ // Normal death sound
				switch(actor->player->class)
				{
					case PCLASS_FIGHTER:
						sound = SFX_PLAYER_FIGHTER_NORMAL_DEATH;
						break;
					case PCLASS_CLERIC:
						sound = SFX_PLAYER_CLERIC_NORMAL_DEATH;
						break;
					case PCLASS_MAGE:
						sound = SFX_PLAYER_MAGE_NORMAL_DEATH;
						break;
					default:
						sound = SFX_NONE;
						break;
				}
			}
			else if(actor->health > -100)
			{ // Crazy death sound
				switch(actor->player->class)
				{
					case PCLASS_FIGHTER:
						sound = SFX_PLAYER_FIGHTER_CRAZY_DEATH;
						break;
					case PCLASS_CLERIC:
						sound = SFX_PLAYER_CLERIC_CRAZY_DEATH;
						break;
					case PCLASS_MAGE:
						sound = SFX_PLAYER_MAGE_CRAZY_DEATH;
						break;
					default:
						sound = SFX_NONE;
						break;
				}
			}
			else
			{ // Extreme death sound
				switch(actor->player->class)
				{
					case PCLASS_FIGHTER:
						sound = SFX_PLAYER_FIGHTER_EXTREME1_DEATH;
						break;
					case PCLASS_CLERIC:
						sound = SFX_PLAYER_CLERIC_EXTREME1_DEATH;
						break;
					case PCLASS_MAGE:
						sound = SFX_PLAYER_MAGE_EXTREME1_DEATH;
						break;
					default:
						sound = SFX_NONE;
						break;
				}
				sound += P_Random()%3; // Three different extreme deaths
			}
			S_StartSound(actor, sound);
		}
	}
	else
	{
		S_StartSound(actor, actor->info->deathsound);
	}
}

//---------------------------------------------------------------------------
//
// PROC P_DropItem
//
//---------------------------------------------------------------------------

/*
void P_DropItem(mobj_t *source, mobjtype_t type, int special, int chance)
{
	mobj_t *mo;

	if(P_Random() > chance)
	{
		return;
	}
	mo = P_SpawnMobj(source->x, source->y,
		source->z+(source->height>>1), type);
	mo->momx = (P_Random()-P_Random())<<8;
	mo->momy = (P_Random()-P_Random())<<8;
	mo->momz = FRACUNIT*5+(P_Random()<<10);
	mo->flags2 |= MF2_DROPPED;
	mo->health = special;
}
*/

//----------------------------------------------------------------------------
//
// PROC A_NoBlocking
//
//----------------------------------------------------------------------------

void A_NoBlocking(mobj_t *actor)
{
	actor->flags &= ~MF_SOLID;

	// Check for monsters dropping things
/*	switch(actor->type)
	{
		// Add the monster dropped items here
		case MT_MUMMYLEADERGHOST:
			P_DropItem(actor, MT_AMGWNDWIMPY, 3, 84);
			break;
		default:
			break;
	}
*/
}

//----------------------------------------------------------------------------
//
// PROC A_Explode
//
// Handles a bunch of exploding things.
//
//----------------------------------------------------------------------------

void A_Explode(mobj_t *actor)
{
	int damage;
	int distance;
	boolean damageSelf;

	damage = 128;
	distance = 128;
	damageSelf = true;
	switch(actor->type)
	{
		case MT_FIREBOMB: // Time Bombs
			actor->z += 32*FRACUNIT;
			actor->flags &= ~MF_SHADOW;
			break;
		case MT_MNTRFX2: // Minotaur floor fire
			damage = 24;
			break;
		case MT_BISHOP: // Bishop radius death
			damage = 25+(P_Random()&15);
			break;
		case MT_HAMMER_MISSILE: // Fighter Hammer
			damage = 128;
			damageSelf = false;
			break;
		case MT_FSWORD_MISSILE: // Fighter Runesword
			damage = 64;
			damageSelf = false;
			break;
		case MT_CIRCLEFLAME: // Cleric Flame secondary flames
			damage = 20;
			damageSelf = false;
			break;
		case MT_SORCBALL1: 	// Sorcerer balls
		case MT_SORCBALL2:
		case MT_SORCBALL3:
			distance = 255;
			damage = 255;
			actor->args[0] = 1;		// don't play bounce
			break;
		case MT_SORCFX1: 	// Sorcerer spell 1
			damage = 30;
			break;
		case MT_SORCFX4: 	// Sorcerer spell 4
			damage = 20;
			break;
		case MT_TREEDESTRUCTIBLE:
			damage = 10;
			break;
		case MT_DRAGON_FX2:
			damage = 80;
			damageSelf = false;
			break;
		case MT_MSTAFF_FX:
			damage = 64;
			distance = 192;
			damageSelf = false;
			break;
		case MT_MSTAFF_FX2:
			damage = 80;
			distance = 192;
			damageSelf = false;
			break;
		case MT_POISONCLOUD:
			damage = 4;
			distance = 40;
			break;
		case MT_ZXMAS_TREE:
		case MT_ZSHRUB2:
			damage = 30;
			distance = 64;
			break;
		default:
			break;
	}
	P_RadiusAttack(actor, actor->target, damage, distance, damageSelf);
	if(actor->z <= actor->floorz+(distance<<FRACBITS)
		&& actor->type != MT_POISONCLOUD)
	{
 		P_HitFloor(actor);
	}
}

//----------------------------------------------------------------------------
//
// PROC P_Massacre
//
// Kills all monsters.
//
//----------------------------------------------------------------------------

int P_Massacre(void)
{
	int count;
	mobj_t *mo;
	thinker_t *think;

	count = 0;
	for(think = thinkercap.next; think != &thinkercap;
		think = think->next)
	{
		if(think->function != P_MobjThinker)
		{ // Not a mobj thinker
			continue;
		}
		mo = (mobj_t *)think;
		if((mo->flags&MF_COUNTKILL) && (mo->health > 0))
		{
			mo->flags2 &= ~(MF2_NONSHOOTABLE+MF2_INVULNERABLE);
			mo->flags |= MF_SHOOTABLE;
			P_DamageMobj(mo, NULL, NULL, 10000);
			count++;
		}
	}
	return count;
}



//----------------------------------------------------------------------------
//
// PROC A_SkullPop
//
//----------------------------------------------------------------------------

void A_SkullPop(mobj_t *actor)
{
	mobj_t *mo;
	player_t *player;

	if(!actor->player)
	{
		return;
	}
	actor->flags &= ~MF_SOLID;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+48*FRACUNIT,
		MT_BLOODYSKULL);
	//mo->target = actor;
	mo->momx = (P_Random()-P_Random())<<9;
	mo->momy = (P_Random()-P_Random())<<9;
	mo->momz = FRACUNIT*2+(P_Random()<<6);
	// Attach player mobj to bloody skull
	player = actor->player;
	actor->player = NULL;
	actor->special1 = player->class;
	mo->player = player;
	mo->health = actor->health;
	mo->angle = actor->angle;
	player->mo = mo;
	player->lookdir = 0;
	player->damagecount = 32;
}

//----------------------------------------------------------------------------
//
// PROC A_CheckSkullFloor
//
//----------------------------------------------------------------------------

void A_CheckSkullFloor(mobj_t *actor)
{
	if(actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, S_BLOODYSKULLX1);
		S_StartSound(actor, SFX_DRIP);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_CheckSkullDone
//
//----------------------------------------------------------------------------

void A_CheckSkullDone(mobj_t *actor)
{
	if(actor->special2 == 666)
	{
		P_SetMobjState(actor, S_BLOODYSKULLX2);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_CheckBurnGone
//
//----------------------------------------------------------------------------

void A_CheckBurnGone(mobj_t *actor)
{
	if(actor->special2 == 666)
	{
		P_SetMobjState(actor, S_PLAY_FDTH20);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FreeTargMobj
//
//----------------------------------------------------------------------------

void A_FreeTargMobj(mobj_t *mo)
{
	mo->momx = mo->momy = mo->momz = 0;
	mo->z = mo->ceilingz+4*FRACUNIT;
	mo->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY|MF_SOLID|MF_COUNTKILL);
	mo->flags |= MF_CORPSE|MF_DROPOFF|MF_NOGRAVITY;
	mo->flags2 &= ~(MF2_PASSMOBJ|MF2_LOGRAV);
	mo->flags2 |= MF2_DONTDRAW;
	mo->player = NULL;
	mo->health = -1000;		// Don't resurrect
}


//----------------------------------------------------------------------------
//
// CorpseQueue Routines
//
//----------------------------------------------------------------------------

// Corpse queue for monsters - this should be saved out
#define CORPSEQUEUESIZE	64
mobj_t *corpseQueue[CORPSEQUEUESIZE];
int corpseQueueSlot;

// throw another corpse on the queue
void A_QueueCorpse(mobj_t *actor)
{
	mobj_t *corpse;

	if(corpseQueueSlot >= CORPSEQUEUESIZE)
	{ // Too many corpses - remove an old one
		corpse = corpseQueue[corpseQueueSlot%CORPSEQUEUESIZE];
		if (corpse) P_RemoveMobj(corpse);
	}
	corpseQueue[corpseQueueSlot%CORPSEQUEUESIZE] = actor;
	corpseQueueSlot++;
}

// Remove a mobj from the queue (for resurrection)
void A_DeQueueCorpse(mobj_t *actor)
{
	int slot;

	for (slot=0; slot<CORPSEQUEUESIZE; slot++)
	{
		if (corpseQueue[slot] == actor)
		{
			corpseQueue[slot] = NULL;
			break;
		}
	}
}

void P_InitCreatureCorpseQueue(boolean corpseScan)
{
	thinker_t *think;
	mobj_t *mo;

	// Initialize queue
	corpseQueueSlot=0;
	memset(corpseQueue, 0, sizeof(mobj_t *)*CORPSEQUEUESIZE);

	if (!corpseScan) return;

	// Search mobj list for corpses and place them in this queue
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != P_MobjThinker) continue;
		mo = (mobj_t *)think;
		if (!(mo->flags&MF_CORPSE)) continue;	// Must be a corpse
		if (mo->flags&MF_ICECORPSE) continue;	// Not ice corpses
		// Only corpses that call A_QueueCorpse from death routine
		switch(mo->type)
		{
			case MT_CENTAUR:
			case MT_CENTAURLEADER:
			case MT_DEMON:
			case MT_DEMON2:
			case MT_WRAITH:
			case MT_WRAITHB:
			case MT_BISHOP:
			case MT_ETTIN:
			case MT_PIG:
			case MT_CENTAUR_SHIELD:
			case MT_CENTAUR_SWORD:
			case MT_DEMONCHUNK1:
			case MT_DEMONCHUNK2:
			case MT_DEMONCHUNK3:
			case MT_DEMONCHUNK4:
			case MT_DEMONCHUNK5:
			case MT_DEMON2CHUNK1:
			case MT_DEMON2CHUNK2:
			case MT_DEMON2CHUNK3:
			case MT_DEMON2CHUNK4:
			case MT_DEMON2CHUNK5:
			case MT_FIREDEMON_SPLOTCH1:
			case MT_FIREDEMON_SPLOTCH2:
				A_QueueCorpse(mo);		// Add corpse to queue
				break;
			default:
				break;
		}
	}
}


//----------------------------------------------------------------------------
//
// PROC A_AddPlayerCorpse
//
//----------------------------------------------------------------------------

#define BODYQUESIZE 32
mobj_t *bodyque[BODYQUESIZE];
int bodyqueslot;

void A_AddPlayerCorpse(mobj_t *actor)
{
	if(bodyqueslot >= BODYQUESIZE)
	{ // Too many player corpses - remove an old one
		P_RemoveMobj(bodyque[bodyqueslot%BODYQUESIZE]);
	}
	bodyque[bodyqueslot%BODYQUESIZE] = actor;
	bodyqueslot++;
}

//============================================================================
//
// A_SerpentUnHide
//
//============================================================================

void A_SerpentUnHide(mobj_t *actor)
{
	actor->flags2 &= ~MF2_DONTDRAW;
	actor->floorclip = 24*FRACUNIT;
}

//============================================================================
//
// A_SerpentHide
//
//============================================================================

void A_SerpentHide(mobj_t *actor)
{
	actor->flags2 |= MF2_DONTDRAW;
	actor->floorclip = 0;
}
//============================================================================
//
// A_SerpentChase
//
//============================================================================

void A_SerpentChase(mobj_t *actor)
{
	int delta;
	int oldX, oldY, oldFloor;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if(gameskill == sk_nightmare)
	{ // Monsters move faster in nightmare mode
		actor->tics -= actor->tics/2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

//
// turn towards movement direction if not there yet
//
	if(actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle-(actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90/2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90/2;
		}
	}

	if(!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{ // look for a new target
		if(P_LookForPlayers(actor, true))
		{ // got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

//
// don't attack twice in a row
//
	if(actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		if (gameskill != sk_nightmare)
			P_NewChaseDir (actor);
		return;
	}

//
// check for melee attack
//
	if (actor->info->meleestate && P_CheckMeleeRange (actor))
	{
		if(actor->info->attacksound)
		{
			S_StartSound (actor, actor->info->attacksound);
		}
		P_SetMobjState (actor, actor->info->meleestate);
		return;
	}

//
// possibly choose another target
//
	if (netgame && !actor->threshold && !P_CheckSight (actor, actor->target) )
	{
		if (P_LookForPlayers(actor,true))
			return;         // got a new target
	}

//
// chase towards player
//
	oldX = actor->x;
	oldY = actor->y;
	oldFloor = actor->subsector->sector->floorpic;
	if (--actor->movecount<0 || !P_Move (actor))
	{
		P_NewChaseDir (actor);
	}
	if(actor->subsector->sector->floorpic != oldFloor)
	{
		P_TryMove(actor, oldX, oldY);
		P_NewChaseDir (actor);
	}

//
// make active sound
//
	if(actor->info->activesound && P_Random() < 3)
	{
		S_StartSound(actor, actor->info->activesound);
	}
}

//============================================================================
//
// A_SerpentRaiseHump
// 
// Raises the hump above the surface by raising the floorclip level
//============================================================================

void A_SerpentRaiseHump(mobj_t *actor)
{
	actor->floorclip -= 4*FRACUNIT;
}

//============================================================================
//
// A_SerpentLowerHump
// 
//============================================================================

void A_SerpentLowerHump(mobj_t *actor)
{
	actor->floorclip += 4*FRACUNIT;
}

//============================================================================
//
// A_SerpentHumpDecide
//
//		Decided whether to hump up, or if the mobj is a serpent leader, 
//			to missile attack
//============================================================================

void A_SerpentHumpDecide(mobj_t *actor)
{
	if(actor->type == MT_SERPENTLEADER)
	{
		if(P_Random() > 30)
		{
			return;
		}
		else if(P_Random() < 40)
		{ // Missile attack
			P_SetMobjState(actor, S_SERPENT_SURFACE1);
			return;
		}
	}
	else if(P_Random() > 3)
	{
		return;
	}
	if(!P_CheckMeleeRange(actor))
	{ // The hump shouldn't occur when within melee range
		if(actor->type == MT_SERPENTLEADER && P_Random() < 128)
		{
			P_SetMobjState(actor, S_SERPENT_SURFACE1);
		}
		else
		{	
			P_SetMobjState(actor, S_SERPENT_HUMP1);
			S_StartSound(actor, SFX_SERPENT_ACTIVE);
		}
	}
}

//============================================================================
//
// A_SerpentBirthScream
//
//============================================================================

void A_SerpentBirthScream(mobj_t *actor)
{
	S_StartSound(actor, SFX_SERPENT_BIRTH);
}

//============================================================================
//
// A_SerpentDiveSound
//
//============================================================================

void A_SerpentDiveSound(mobj_t *actor)
{
	S_StartSound(actor, SFX_SERPENT_ACTIVE);
}

//============================================================================
//
// A_SerpentWalk
//
// Similar to A_Chase, only has a hardcoded entering of meleestate
//============================================================================

void A_SerpentWalk(mobj_t *actor)
{
	int delta;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if(gameskill == sk_nightmare)
	{ // Monsters move faster in nightmare mode
		actor->tics -= actor->tics/2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

//
// turn towards movement direction if not there yet
//
	if(actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle-(actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90/2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90/2;
		}
	}

	if(!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{ // look for a new target
		if(P_LookForPlayers(actor, true))
		{ // got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

//
// don't attack twice in a row
//
	if(actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		if (gameskill != sk_nightmare)
			P_NewChaseDir (actor);
		return;
	}

//
// check for melee attack
//
	if (actor->info->meleestate && P_CheckMeleeRange (actor))
	{
		if (actor->info->attacksound)
		{
			S_StartSound (actor, actor->info->attacksound);
		}
		P_SetMobjState(actor, S_SERPENT_ATK1);
		return;
	}
//
// possibly choose another target
//
	if (netgame && !actor->threshold && !P_CheckSight (actor, actor->target) )
	{
		if (P_LookForPlayers(actor,true))
			return;         // got a new target
	}

//
// chase towards player
//
	if (--actor->movecount<0 || !P_Move (actor))
	{
		P_NewChaseDir (actor);
	}
}

//============================================================================
//
// A_SerpentCheckForAttack
//
//============================================================================

void A_SerpentCheckForAttack(mobj_t *actor)
{
	if(!actor->target)
	{
		return;
	}
	if(actor->type == MT_SERPENTLEADER)
	{
		if(!P_CheckMeleeRange(actor))
		{
			P_SetMobjState(actor, S_SERPENT_ATK1);
			return;
		}
	}
	if(P_CheckMeleeRange2(actor))
	{
		P_SetMobjState(actor, S_SERPENT_WALK1);
	}
	else if(P_CheckMeleeRange(actor))
	{
		if(P_Random() < 32)
		{
			P_SetMobjState(actor, S_SERPENT_WALK1);
		}
		else
		{
			P_SetMobjState(actor, S_SERPENT_ATK1);
		}
	}
}

//============================================================================
//
// A_SerpentChooseAttack
//
//============================================================================

void A_SerpentChooseAttack(mobj_t *actor)
{
	if(!actor->target || P_CheckMeleeRange(actor))
	{
		return;
	}
	if(actor->type == MT_SERPENTLEADER)
	{
		P_SetMobjState(actor, S_SERPENT_MISSILE1);
	}
}
	
//============================================================================
//
// A_SerpentMeleeAttack
//
//============================================================================

void A_SerpentMeleeAttack(mobj_t *actor)
{
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(5));
		S_StartSound(actor, SFX_SERPENT_MELEEHIT);
	}
	if(P_Random() < 96)
	{
		A_SerpentCheckForAttack(actor);
	}
}

//============================================================================
//
// A_SerpentMissileAttack
//
//============================================================================
	
void A_SerpentMissileAttack(mobj_t *actor)
{
	mobj_t *mo;

	if(!actor->target)
	{
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MT_SERPENTFX);
}

//============================================================================
//
// A_SerpentHeadPop
//
//============================================================================

void A_SerpentHeadPop(mobj_t *actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, MT_SERPENT_HEAD);
}

//============================================================================
//
// A_SerpentSpawnGibs
//
//============================================================================

void A_SerpentSpawnGibs(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x+((P_Random()-128)<<12), 
		actor->y+((P_Random()-128)<<12), actor->floorz+FRACUNIT,
		MT_SERPENT_GIB1);	
	if(mo)
	{
		mo->momx = (P_Random()-128)<<6;
		mo->momy = (P_Random()-128)<<6;
		mo->floorclip = 6*FRACUNIT;
	}
	mo = P_SpawnMobj(actor->x+((P_Random()-128)<<12), 
		actor->y+((P_Random()-128)<<12), actor->floorz+FRACUNIT,
		MT_SERPENT_GIB2);	
	if(mo)
	{
		mo->momx = (P_Random()-128)<<6;
		mo->momy = (P_Random()-128)<<6;
		mo->floorclip = 6*FRACUNIT;
	}
	mo = P_SpawnMobj(actor->x+((P_Random()-128)<<12), 
		actor->y+((P_Random()-128)<<12), actor->floorz+FRACUNIT,
		MT_SERPENT_GIB3);	
	if(mo)
	{
		mo->momx = (P_Random()-128)<<6;
		mo->momy = (P_Random()-128)<<6;
		mo->floorclip = 6*FRACUNIT;
	}
}

//============================================================================
//
// A_FloatGib
//
//============================================================================

void A_FloatGib(mobj_t *actor)
{
	actor->floorclip -= FRACUNIT;
}

//============================================================================
//
// A_SinkGib
//
//============================================================================

void A_SinkGib(mobj_t *actor)
{
	actor->floorclip += FRACUNIT;
}

//============================================================================
//
// A_DelayGib
//
//============================================================================

void A_DelayGib(mobj_t *actor)
{
	actor->tics -= P_Random()>>2;
}

//============================================================================
//
// A_SerpentHeadCheck
//
//============================================================================

void A_SerpentHeadCheck(mobj_t *actor)
{
	if(actor->z <= actor->floorz)
	{
		if(P_GetThingFloorType(actor) >= FLOOR_LIQUID)
		{
			P_HitFloor(actor);
			P_SetMobjState(actor, S_NULL);
		}
		else
		{
			P_SetMobjState(actor, S_SERPENT_HEAD_X1);
		}
	}
}

//============================================================================
//
// A_CentaurAttack
//
//============================================================================

void A_CentaurAttack(mobj_t *actor)
{
	if(!actor->target)
	{
		return;
	}
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, P_Random()%7+3);
	}
}

//============================================================================
//
// A_CentaurAttack2
//
//============================================================================

void A_CentaurAttack2(mobj_t *actor)
{
	if(!actor->target)
	{
		return;
	}
	P_SpawnMissile(actor, actor->target, MT_CENTAUR_FX);
	S_StartSound(actor, SFX_CENTAURLEADER_ATTACK);
}

//============================================================================
//
// A_CentaurDropStuff
//
// 	Spawn shield/sword sprites when the centaur pulps //============================================================================

void A_CentaurDropStuff(mobj_t *actor)
{
	mobj_t *mo;
	angle_t angle;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_CENTAUR_SHIELD);
	if(mo)
	{
		angle = actor->angle+ANG90;
		mo->momz = FRACUNIT*8+(P_Random()<<10);
		mo->momx = FixedMul(((P_Random()-128)<<11)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul(((P_Random()-128)<<11)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_CENTAUR_SWORD);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = FRACUNIT*8+(P_Random()<<10);
		mo->momx = FixedMul(((P_Random()-128)<<11)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul(((P_Random()-128)<<11)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
}

//============================================================================
//
// A_CentaurDefend
//
//============================================================================

void A_CentaurDefend(mobj_t *actor)
{
	A_FaceTarget(actor);
	if(P_CheckMeleeRange(actor) && P_Random() < 32)
	{
		A_UnSetInvulnerable(actor);
		P_SetMobjState(actor, actor->info->meleestate);
	}
}

//============================================================================
//
// A_BishopAttack
//
//============================================================================

void A_BishopAttack(mobj_t *actor)
{
	if(!actor->target)
	{
		return;
	}
	S_StartSound(actor, actor->info->attacksound);
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(4));
		return;
	}
	actor->special1 = (P_Random()&3)+5;
}

//============================================================================
//
// A_BishopAttack2
//
//		Spawns one of a string of bishop missiles
//============================================================================

void A_BishopAttack2(mobj_t *actor)
{
	mobj_t *mo;

	if(!actor->target || !actor->special1)
	{
		actor->special1 = 0;
		P_SetMobjState(actor, S_BISHOP_WALK1);
		return;
	}
	mo = P_SpawnMissile(actor, actor->target, MT_BISH_FX);
	if(mo)
	{
		mo->special1 = (int)actor->target;
		mo->special2 = 16; // High word == x/y, Low word == z
	}
	actor->special1--;
}

//============================================================================
//
// A_BishopMissileWeave
//
//============================================================================

void A_BishopMissileWeave(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2>>16;
	weaveZ = actor->special2&0xFFFF;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<1);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]<<1);
	weaveXY = (weaveXY+2)&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<1);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]<<1);
	P_TryMove(actor, newX, newY);
	actor->z -= FloatBobOffsets[weaveZ];
	weaveZ = (weaveZ+2)&63;
	actor->z += FloatBobOffsets[weaveZ];	
	actor->special2 = weaveZ+(weaveXY<<16);
}

//============================================================================
//
// A_BishopMissileSeek
//
//============================================================================

void A_BishopMissileSeek(mobj_t *actor)
{
	P_SeekerMissile(actor, ANGLE_1*2, ANGLE_1*3);
}

//============================================================================
//
// A_BishopDecide
//
//============================================================================

void A_BishopDecide(mobj_t *actor)
{
	if(P_Random() < 220)
	{
		return;
	}
	else
	{
		P_SetMobjState(actor, S_BISHOP_BLUR1);
	}		
}

//============================================================================
//
// A_BishopDoBlur
//
//============================================================================

void A_BishopDoBlur(mobj_t *actor)
{
	actor->special1 = (P_Random()&3)+3; // Random number of blurs
	if(P_Random() < 120)
	{
		P_ThrustMobj(actor, actor->angle+ANG90, 11*FRACUNIT);
	}
	else if(P_Random() > 125)
	{
		P_ThrustMobj(actor, actor->angle-ANG90, 11*FRACUNIT);
	}
	else
	{ // Thrust forward
		P_ThrustMobj(actor, actor->angle, 11*FRACUNIT);
	}
	S_StartSound(actor, SFX_BISHOP_BLUR);
}

//============================================================================
//
// A_BishopSpawnBlur
//
//============================================================================

void A_BishopSpawnBlur(mobj_t *actor)
{
	mobj_t *mo;

	if(!--actor->special1)
	{
		actor->momx = 0;
		actor->momy = 0;
		if(P_Random() > 96)
		{
			P_SetMobjState(actor, S_BISHOP_WALK1);
		}
		else
		{
			P_SetMobjState(actor, S_BISHOP_ATK1);
		}
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_BISHOPBLUR);
	if(mo)
	{
		mo->angle = actor->angle;
	}
}

//============================================================================
//
// A_BishopChase
//
//============================================================================

void A_BishopChase(mobj_t *actor)
{
	actor->z -= FloatBobOffsets[actor->special2]>>1;
	actor->special2 = (actor->special2+4)&63;
	actor->z += FloatBobOffsets[actor->special2]>>1;
}

//============================================================================
//
// A_BishopPuff
//
//============================================================================

void A_BishopPuff(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z+40*FRACUNIT, 	
		MT_BISHOP_PUFF);
	if(mo)
	{
		mo->momz = FRACUNIT/2;
	}
}

//============================================================================
//
// A_BishopPainBlur
//
//============================================================================

void A_BishopPainBlur(mobj_t *actor)
{
	mobj_t *mo;

	if(P_Random() < 64)
	{
		P_SetMobjState(actor, S_BISHOP_BLUR1);
		return;
	}
	mo = P_SpawnMobj(actor->x+((P_Random()-P_Random())<<12), actor->y
		+((P_Random()-P_Random())<<12), actor->z+((P_Random()-P_Random())<<11),
		MT_BISHOPPAINBLUR);
	if(mo)
	{
		mo->angle = actor->angle;
	}
}

//============================================================================
//
// DragonSeek
//
//============================================================================

static void DragonSeek(mobj_t *actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t *target;
	int search;
	int i;
	int bestArg;
	angle_t bestAngle;
	angle_t angleToSpot, angleToTarget;
	mobj_t *mo;

	target = (mobj_t *)actor->special1;
	if(target == NULL)
	{
		return;
	}
	dir = P_FaceMobj(actor, target, &delta);
	if(delta > thresh)
	{
		delta >>= 1;
		if(delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if(dir)
	{ // Turn clockwise
		actor->angle += delta;
	}
	else
	{ // Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);
	if(actor->z+actor->height < target->z 
		|| target->z+target->height < actor->z)
	{
		dist = P_AproxDistance(target->x-actor->x, target->y-actor->y);
		dist = dist/actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		actor->momz = (target->z-actor->z)/dist;
	}
	else
	{
		dist = P_AproxDistance(target->x-actor->x, target->y-actor->y);
		dist = dist/actor->info->speed;
	}
	if(target->flags&MF_SHOOTABLE && P_Random() < 64)
	{ // attack the destination mobj if it's attackable
		mobj_t *oldTarget;
	
		if(abs(actor->angle-R_PointToAngle2(actor->x, actor->y, 
			target->x, target->y)) < ANGLE_45/2)
		{
			oldTarget = actor->target;
			actor->target = target;
			if(P_CheckMeleeRange(actor))
			{
				P_DamageMobj(actor->target, actor, actor, HITDICE(10));
				S_StartSound(actor, SFX_DRAGON_ATTACK);
			}
			else if(P_Random() < 128 && P_CheckMissileRange(actor))
			{
				P_SpawnMissile(actor, target, MT_DRAGON_FX);						
				S_StartSound(actor, SFX_DRAGON_ATTACK);
			}
			actor->target = oldTarget;
		}
	}
	if(dist < 4)
	{ // Hit the target thing
		if(actor->target && P_Random() < 200)
		{
			bestArg = -1;
			bestAngle = ANGLE_MAX;
			angleToTarget = R_PointToAngle2(actor->x, actor->y,
				actor->target->x, actor->target->y);
			for(i = 0; i < 5; i++)
			{
				if(!target->args[i])
				{
					continue;
				}
				search = -1;
				mo = P_FindMobjFromTID(target->args[i], &search);
				angleToSpot = R_PointToAngle2(actor->x, actor->y, 
					mo->x, mo->y);
				if(abs(angleToSpot-angleToTarget) < bestAngle)
				{
					bestAngle = abs(angleToSpot-angleToTarget);
					bestArg = i;
				}
			}
			if(bestArg != -1)
			{
				search = -1;
				actor->special1 = (int)P_FindMobjFromTID(target->args[bestArg], 
					&search);
			}
		}
		else
		{
			do
			{
				i = (P_Random()>>2)%5;
			} while(!target->args[i]);
			search = -1;
			actor->special1 = (int)P_FindMobjFromTID(target->args[i], &search);
		}
	}
}

//============================================================================
//
// A_DragonInitFlight
//
//============================================================================

void A_DragonInitFlight(mobj_t *actor)
{
	int search;

	search = -1;
	do
	{ // find the first tid identical to the dragon's tid
		actor->special1 = (int)P_FindMobjFromTID(actor->tid, &search);
		if(search == -1)
		{
			P_SetMobjState(actor, actor->info->spawnstate);
			return;
		}
	} while(actor->special1 == (int)actor);
	P_RemoveMobjFromTIDList(actor);
}

//============================================================================
//
// A_DragonFlight
//
//============================================================================

void A_DragonFlight(mobj_t *actor)
{
	angle_t angle;

	DragonSeek(actor, 4*ANGLE_1, 8*ANGLE_1);
	if(actor->target)
	{
		if(!(actor->target->flags&MF_SHOOTABLE))
		{ // target died
			actor->target = NULL;
			return;
		}
		angle = R_PointToAngle2(actor->x, actor->y, actor->target->x,
			actor->target->y);
		if(abs(actor->angle-angle) < ANGLE_45/2 && P_CheckMeleeRange(actor))
		{
			P_DamageMobj(actor->target, actor, actor, HITDICE(8));
			S_StartSound(actor, SFX_DRAGON_ATTACK);
		}
		else if(abs(actor->angle-angle) <= ANGLE_1*20)
		{
			P_SetMobjState(actor, actor->info->missilestate);
			S_StartSound(actor, SFX_DRAGON_ATTACK);
		}
	}
	else
	{
		P_LookForPlayers(actor, true);
	}
}

//============================================================================
//
// A_DragonFlap
//
//============================================================================

void A_DragonFlap(mobj_t *actor)
{
	A_DragonFlight(actor);
	if(P_Random() < 240)
	{
		S_StartSound(actor, SFX_DRAGON_WINGFLAP);
	}
	else
	{
		S_StartSound(actor, actor->info->activesound);
	}
}

//============================================================================
//
// A_DragonAttack
//
//============================================================================

void A_DragonAttack(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMissile(actor, actor->target, MT_DRAGON_FX);						
}

//============================================================================
//
// A_DragonFX2
//
//============================================================================

void A_DragonFX2(mobj_t *actor)
{
	mobj_t *mo;
	int i;
	int delay;

	delay = 16+(P_Random()>>3);
	for(i = 1+(P_Random()&3); i; i--)
	{
		mo = P_SpawnMobj(actor->x+((P_Random()-128)<<14), 
			actor->y+((P_Random()-128)<<14), actor->z+((P_Random()-128)<<12),
			MT_DRAGON_FX2);
		if(mo)
		{
			mo->tics = delay+(P_Random()&3)*i*2;
			mo->target = actor->target;
		}
	} 
}

//============================================================================
//
// A_DragonPain
//
//============================================================================

void A_DragonPain(mobj_t *actor)
{
	A_Pain(actor);
	if(!actor->special1)
	{ // no destination spot yet
		P_SetMobjState(actor, S_DRAGON_INIT);
	}
}

//============================================================================
//
// A_DragonCheckCrash
//
//============================================================================

void A_DragonCheckCrash(mobj_t *actor)
{
	if(actor->z <= actor->floorz)
	{
		P_SetMobjState(actor, S_DRAGON_CRASH1);
	}
}

//============================================================================
// Demon AI
//============================================================================

//
// A_DemonAttack1 (melee)
//
void A_DemonAttack1(mobj_t *actor)
{
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
	}
}


//
// A_DemonAttack2 (missile)
//
void A_DemonAttack2(mobj_t *actor)
{
	mobj_t *mo;
	int fireBall;

	if(actor->type == MT_DEMON)
	{
		fireBall = MT_DEMONFX1;
	}
	else
	{
		fireBall = MT_DEMON2FX1;
	}
	mo = P_SpawnMissile(actor, actor->target, fireBall);
	if (mo)
	{
		mo->z += 30*FRACUNIT;
		S_StartSound(actor, SFX_DEMON_MISSILE_FIRE);
	}
}

//
// A_DemonDeath
//

void A_DemonDeath(mobj_t *actor)
{
	mobj_t *mo;
	angle_t angle;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMONCHUNK1);
	if(mo)
	{
		angle = actor->angle+ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMONCHUNK2);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMONCHUNK3);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMONCHUNK4);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMONCHUNK5);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
}

//===========================================================================
//
// A_Demon2Death
//
//===========================================================================

void A_Demon2Death(mobj_t *actor)
{
	mobj_t *mo;
	angle_t angle;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMON2CHUNK1);
	if(mo)
	{
		angle = actor->angle+ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMON2CHUNK2);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMON2CHUNK3);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMON2CHUNK4);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+45*FRACUNIT, 
		MT_DEMON2CHUNK5);
	if(mo)
	{
		angle = actor->angle-ANG90;
		mo->momz = 8*FRACUNIT;
		mo->momx = FixedMul((P_Random()<<10)+FRACUNIT,
			finecosine[angle>>ANGLETOFINESHIFT]);
		mo->momy = FixedMul((P_Random()<<10)+FRACUNIT, 
			finesine[angle>>ANGLETOFINESHIFT]);
		mo->target = actor;
	}
}



//
// A_SinkMobj
// Sink a mobj incrementally into the floor
//

boolean A_SinkMobj(mobj_t *actor)
{
	if (actor->floorclip <  actor->info->height)
	{
		switch(actor->type)
		{
			case MT_THRUSTFLOOR_DOWN:
			case MT_THRUSTFLOOR_UP:
				actor->floorclip += 6*FRACUNIT;
				break;
			default:
				actor->floorclip += FRACUNIT;
				break;
		}
		return false;
	}
	return true;
}

//
// A_RaiseMobj
// Raise a mobj incrementally from the floor to 
// 

boolean A_RaiseMobj(mobj_t *actor)
{
	int done = true;

	// Raise a mobj from the ground
	if (actor->floorclip > 0)
	{
		switch(actor->type)
		{
			case MT_WRAITHB:
				actor->floorclip -= 2*FRACUNIT;
				break;
			case MT_THRUSTFLOOR_DOWN:
			case MT_THRUSTFLOOR_UP:
				actor->floorclip -= actor->special2*FRACUNIT;
				break;
			default:
				actor->floorclip -= 2*FRACUNIT;
				break;
		}
		if (actor->floorclip <= 0)
		{
			actor->floorclip = 0;
			done=true;
		}
		else
		{
			done = false;
		}
	}
	return done;		// Reached target height
}


//============================================================================
// Wraith Variables
//
//	special1				Internal index into floatbob
//	special2
//============================================================================

//
// A_WraithInit
//

void A_WraithInit(mobj_t *actor)
{
	actor->z += 48<<FRACBITS;
	actor->special1 = 0;			// index into floatbob
}

void A_WraithRaiseInit(mobj_t *actor)
{
	actor->flags2 &= ~MF2_DONTDRAW;
	actor->flags2 &= ~MF2_NONSHOOTABLE;
	actor->flags |= MF_SHOOTABLE|MF_SOLID;
	actor->floorclip = actor->info->height;
}

void A_WraithRaise(mobj_t *actor)
{
	if (A_RaiseMobj(actor))
	{
		// Reached it's target height
		P_SetMobjState(actor,S_WRAITH_CHASE1);
	}

	P_SpawnDirt(actor, actor->radius);
}


void A_WraithMelee(mobj_t *actor)
{
	int amount;

	// Steal health from target and give to player
	if(P_CheckMeleeRange(actor) && (P_Random()<220))
	{
		amount = HITDICE(2);
		P_DamageMobj(actor->target, actor, actor, amount);
		actor->health += amount;
	}
}

void A_WraithMissile(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMissile(actor, actor->target, MT_WRAITHFX1);
	if (mo)
	{
		S_StartSound(actor, SFX_WRAITH_MISSILE_FIRE);
	}
}


//
// A_WraithFX2 - spawns sparkle tail of missile
//

void A_WraithFX2(mobj_t *actor)
{
	mobj_t *mo;
	angle_t angle;
	int i;

	for (i=0; i<2; i++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_WRAITHFX2);
		if(mo)
		{
			if (P_Random()<128)
			{
				 angle = actor->angle+(P_Random()<<22);
			}
			else
			{
				 angle = actor->angle-(P_Random()<<22);
			}
			mo->momz = 0;
			mo->momx = FixedMul((P_Random()<<7)+FRACUNIT,
				 finecosine[angle>>ANGLETOFINESHIFT]);
			mo->momy = FixedMul((P_Random()<<7)+FRACUNIT, 
				 finesine[angle>>ANGLETOFINESHIFT]);
			mo->target = actor;
			mo->floorclip = 10*FRACUNIT;
		}
	}
}


// Spawn an FX3 around the actor during attacks
void A_WraithFX3(mobj_t *actor)
{
	mobj_t *mo;
	int numdropped=P_Random()%15;
	int i;

	for (i=0; i<numdropped; i++)
	{
		  mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_WRAITHFX3);
		  if(mo)
		  {
				mo->x += (P_Random()-128)<<11;
				mo->y += (P_Random()-128)<<11;
				mo->z += (P_Random()<<10);
				mo->target = actor;
		  }
	}
}

// Spawn an FX4 during movement
void A_WraithFX4(mobj_t *actor)
{
	mobj_t *mo;
	int chance = P_Random();
	int spawn4,spawn5;

	if (chance < 10)
	{
		spawn4 = true;
		spawn5 = false;
	}
	else if (chance < 20)
	{
		spawn4 = false;
		spawn5 = true;
	}
	else if (chance < 25)
	{
		spawn4 = true;
		spawn5 = true;
	}
	else
	{
		spawn4 = false;
		spawn5 = false;
	}

	if (spawn4)
	{
		  mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_WRAITHFX4);
		  if(mo)
		  {
				mo->x += (P_Random()-128)<<12;
				mo->y += (P_Random()-128)<<12;
				mo->z += (P_Random()<<10);
				mo->target = actor;
		  }
	}
	if (spawn5)
	{
		  mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_WRAITHFX5);
		  if(mo)
		  {
				mo->x += (P_Random()-128)<<11;
				mo->y += (P_Random()-128)<<11;
				mo->z += (P_Random()<<10);
				mo->target = actor;
		  }
	}
}


void A_WraithLook(mobj_t *actor)
{
//	A_WraithFX4(actor);		// too expensive
	A_Look(actor);
}


void A_WraithChase(mobj_t *actor)
{
	int weaveindex = actor->special1;
	actor->z += FloatBobOffsets[weaveindex];
	actor->special1 = (weaveindex+2)&63;
//	if (actor->floorclip > 0)
//	{
//		P_SetMobjState(actor, S_WRAITH_RAISE2);
//		return;
//	}
	A_Chase(actor);
	A_WraithFX4(actor);
}



//============================================================================
// Ettin AI
//============================================================================

void A_EttinAttack(mobj_t *actor)
{
	if(P_CheckMeleeRange(actor))
	{
		P_DamageMobj(actor->target, actor, actor, HITDICE(2));
	}
}


void A_DropMace(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y,
		actor->z+(actor->height>>1), MT_ETTIN_MACE);
	if (mo)
	{
		mo->momx = (P_Random()-128)<<11;
		mo->momy = (P_Random()-128)<<11;
		mo->momz = FRACUNIT*10+(P_Random()<<10);
		mo->target = actor;
	}
}


//============================================================================
// Fire Demon AI
//
// special1			index into floatbob
// special2			whether strafing or not
//============================================================================

void A_FiredSpawnRock(mobj_t *actor)
{
	mobj_t *mo;
	int x,y,z;
	int rtype;

	switch(P_Random()%5)
	{
		case 0:
			rtype = MT_FIREDEMON_FX1;
			break;
		case 1:
			rtype = MT_FIREDEMON_FX2;
			break;
		case 2:
			rtype = MT_FIREDEMON_FX3;
			break;
		case 3:
			rtype = MT_FIREDEMON_FX4;
			break;
		case 4:
			rtype = MT_FIREDEMON_FX5;
			break;
	}

	x = actor->x + ((P_Random()-128) << 12);
	y = actor->y + ((P_Random()-128) << 12);
	z = actor->z + ((P_Random()) << 11);
	mo = P_SpawnMobj(x,y,z,rtype);
	if (mo)
	{
		mo->target = actor;
		mo->momx = (P_Random()-128)<<10;
		mo->momy = (P_Random()-128)<<10;
		mo->momz = (P_Random()<<10);
		mo->special1 = 2;		// Number bounces
	}

	// Initialize fire demon
	actor->special2 = 0;
	actor->flags &= ~MF_JUSTATTACKED;
}

void A_FiredRocks(mobj_t *actor)
{
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
	A_FiredSpawnRock(actor);
}

void A_FiredAttack(mobj_t *actor)
{
	mobj_t *mo;
	mo = P_SpawnMissile(actor, actor->target, MT_FIREDEMON_FX6);
	if (mo) S_StartSound(actor, SFX_FIRED_ATTACK);
}

void A_SmBounce(mobj_t *actor)
{
	// give some more momentum (x,y,&z)
	actor->z = actor->floorz + FRACUNIT;
	actor->momz = (2*FRACUNIT) + (P_Random()<<10);
	actor->momx = P_Random()%3<<FRACBITS;
	actor->momy = P_Random()%3<<FRACBITS;
}


#define FIREDEMON_ATTACK_RANGE	64*8*FRACUNIT

void A_FiredChase(mobj_t *actor)
{
	int weaveindex = actor->special1;
	mobj_t *target = actor->target;
	angle_t ang;
	fixed_t dist;

	if(actor->reactiontime) actor->reactiontime--;
	if(actor->threshold) actor->threshold--;

	// Float up and down
	actor->z += FloatBobOffsets[weaveindex];
	actor->special1 = (weaveindex+2)&63;

	// Insure it stays above certain height
	if (actor->z < actor->floorz + (64*FRACUNIT))
	{
		actor->z += 2*FRACUNIT;
	}

	if(!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{	// Invalid target
		P_LookForPlayers(actor,true);
		return;
	}

	// Strafe
	if (actor->special2 > 0)
	{
		actor->special2--;
	}
	else
	{
		actor->special2 = 0;
		actor->momx = actor->momy = 0;
		dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
		if (dist < FIREDEMON_ATTACK_RANGE)
		{
			if (P_Random()<30)
			{
				ang = R_PointToAngle2(actor->x, actor->y, target->x, target->y);
				if (P_Random()<128)
					ang += ANGLE_90;
				else
					ang -= ANGLE_90;
				ang>>=ANGLETOFINESHIFT;
				actor->momx = FixedMul(8*FRACUNIT, finecosine[ang]);
				actor->momy = FixedMul(8*FRACUNIT, finesine[ang]);
				actor->special2 = 3;		// strafe time
			}
		}
	}

	FaceMovementDirection(actor);

	// Normal movement
	if (!actor->special2)
	{
		if (--actor->movecount<0 || !P_Move (actor))
		{
			P_NewChaseDir (actor);
		}
	}

	// Do missile attack
	if (!(actor->flags&MF_JUSTATTACKED))
	{
		if (P_CheckMissileRange(actor) && (P_Random()<20))
		{
			P_SetMobjState (actor, actor->info->missilestate);
			actor->flags |= MF_JUSTATTACKED;
			return;
		}
	}
	else
	{
		actor->flags &= ~MF_JUSTATTACKED;
	}

	// make active sound
	if(actor->info->activesound && P_Random() < 3)
	{
		S_StartSound(actor, actor->info->activesound);
	}
}

void A_FiredSplotch(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x,actor->y,actor->z, MT_FIREDEMON_SPLOTCH1);
	if (mo)
	{
		mo->momx = (P_Random()-128)<<11;
		mo->momy = (P_Random()-128)<<11;
		mo->momz = FRACUNIT*3 + (P_Random()<<10);
	}
	mo = P_SpawnMobj(actor->x,actor->y,actor->z, MT_FIREDEMON_SPLOTCH2);
	if (mo)
	{
		mo->momx = (P_Random()-128)<<11;
		mo->momy = (P_Random()-128)<<11;
		mo->momz = FRACUNIT*3 + (P_Random()<<10);
	}
}


//============================================================================
//
// A_IceGuyLook
//
//============================================================================

void A_IceGuyLook(mobj_t *actor)
{
	fixed_t dist;
	fixed_t an;

	A_Look(actor);
	if(P_Random() < 64)
	{
		dist = ((P_Random()-128)*actor->radius)>>7;
		an = (actor->angle+ANG90)>>ANGLETOFINESHIFT;

		P_SpawnMobj(actor->x+FixedMul(dist, finecosine[an]),
			actor->y+FixedMul(dist, finesine[an]), actor->z+60*FRACUNIT,
			MT_ICEGUY_WISP1+(P_Random()&1));
	}
}

//============================================================================
//
// A_IceGuyChase
//
//============================================================================

void A_IceGuyChase(mobj_t *actor)
{
	fixed_t dist;
	fixed_t an;
	mobj_t *mo;

	A_Chase(actor);
	if(P_Random() < 128)
	{
		dist = ((P_Random()-128)*actor->radius)>>7;
		an = (actor->angle+ANG90)>>ANGLETOFINESHIFT;

		mo = P_SpawnMobj(actor->x+FixedMul(dist, finecosine[an]),
			actor->y+FixedMul(dist, finesine[an]), actor->z+60*FRACUNIT,
			MT_ICEGUY_WISP1+(P_Random()&1));
		if(mo)
		{
			mo->momx = actor->momx;
			mo->momy = actor->momy;
			mo->momz = actor->momz;
			mo->target = actor;
		}
	}
}

//============================================================================
//
// A_IceGuyAttack
//
//============================================================================

void A_IceGuyAttack(mobj_t *actor)
{
	fixed_t an;

	if(!actor->target) 
	{
		return;
	}
	an = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	P_SpawnMissileXYZ(actor->x+FixedMul(actor->radius>>1,
		finecosine[an]), actor->y+FixedMul(actor->radius>>1,
		finesine[an]), actor->z+40*FRACUNIT, actor, actor->target,
		MT_ICEGUY_FX);
	an = (actor->angle-ANG90)>>ANGLETOFINESHIFT;
	P_SpawnMissileXYZ(actor->x+FixedMul(actor->radius>>1,
		finecosine[an]), actor->y+FixedMul(actor->radius>>1,
		finesine[an]), actor->z+40*FRACUNIT, actor, actor->target,
		MT_ICEGUY_FX);
	S_StartSound(actor, actor->info->attacksound);
}

//============================================================================
//
// A_IceGuyMissilePuff
//
//============================================================================

void A_IceGuyMissilePuff(mobj_t *actor)
{
	mobj_t *mo;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z+2*FRACUNIT, MT_ICEFX_PUFF);
}

//============================================================================
//
// A_IceGuyDie
//
//============================================================================

void A_IceGuyDie(mobj_t *actor)
{
	void A_FreezeDeathChunks(mobj_t *actor);

	actor->momx = 0;
	actor->momy = 0;
	actor->momz = 0;
	actor->height <<= 2;
	A_FreezeDeathChunks(actor);
}

//============================================================================
//
// A_IceGuyMissileExplode
//
//============================================================================

void A_IceGuyMissileExplode(mobj_t *actor)
{
	mobj_t *mo;
	int i;

	for(i = 0; i < 8; i++)
	{
		mo = P_SpawnMissileAngle(actor, MT_ICEGUY_FX2, i*ANG45,	-0.3*FRACUNIT);
		if(mo)
		{
			mo->target = actor->target;
		}
	}
}









//============================================================================
//
//	Sorcerer stuff
//
// Sorcerer Variables
//		special1		Angle of ball 1 (all others relative to that)
//		special2		which ball to stop at in stop mode (MT_???)
//		args[0]			Denfense time
//		args[1]			Number of full rotations since stopping mode
//		args[2]			Target orbit speed for acceleration/deceleration
//		args[3]			Movement mode (see SORC_ macros)
//		args[4]			Current ball orbit speed
//	Sorcerer Ball Variables
//		special1		Previous angle of ball (for woosh)
//		special2		Countdown of rapid fire (FX4)
//		args[0]			If set, don't play the bounce sound when bouncing
//============================================================================

#define SORCBALL_INITIAL_SPEED 		7
#define SORCBALL_TERMINAL_SPEED		25
#define SORCBALL_SPEED_ROTATIONS 	5
#define SORC_DEFENSE_TIME			255
#define SORC_DEFENSE_HEIGHT			45
#define BOUNCE_TIME_UNIT			(35/2)
#define SORCFX4_RAPIDFIRE_TIME		(6*3)		// 3 seconds
#define SORCFX4_SPREAD_ANGLE		20

#define SORC_DECELERATE		0
#define SORC_ACCELERATE 	1
#define SORC_STOPPING		2
#define SORC_FIRESPELL		3
#define SORC_STOPPED		4
#define SORC_NORMAL			5
#define SORC_FIRING_SPELL	6

#define BALL1_ANGLEOFFSET	0
#define BALL2_ANGLEOFFSET	(ANGLE_MAX/3)
#define BALL3_ANGLEOFFSET	((ANGLE_MAX/3)*2)

void A_SorcBallOrbit(mobj_t *actor);
void A_SorcSpinBalls(mobj_t *actor);
void A_SpeedBalls(mobj_t *actor);
void A_SlowBalls(mobj_t *actor);
void A_StopBalls(mobj_t *actor);
void A_AccelBalls(mobj_t *actor);
void A_DecelBalls(mobj_t *actor);
void A_SorcBossAttack(mobj_t *actor);
void A_SpawnFizzle(mobj_t *actor);
void A_CastSorcererSpell(mobj_t *actor);
void A_SorcUpdateBallAngle(mobj_t *actor);
void A_BounceCheck(mobj_t *actor);
void A_SorcFX1Seek(mobj_t *actor);
void A_SorcOffense1(mobj_t *actor);
void A_SorcOffense2(mobj_t *actor);


// Spawn spinning balls above head - actor is sorcerer
void A_SorcSpinBalls(mobj_t *actor)
{
	mobj_t *mo;
	fixed_t z;

	A_SlowBalls(actor);
	actor->args[0] = 0;									// Currently no defense
	actor->args[3] = SORC_NORMAL;
	actor->args[4] = SORCBALL_INITIAL_SPEED;		// Initial orbit speed
	actor->special1 = ANGLE_1;
	z = actor->z - actor->floorclip + actor->info->height;
	
	mo = P_SpawnMobj(actor->x, actor->y, z, MT_SORCBALL1);
	if (mo)
	{
		mo->target = actor;
		mo->special2 = SORCFX4_RAPIDFIRE_TIME;
	}
	mo = P_SpawnMobj(actor->x, actor->y, z, MT_SORCBALL2);
	if (mo) mo->target = actor;
	mo = P_SpawnMobj(actor->x, actor->y, z, MT_SORCBALL3);
	if (mo) mo->target = actor;
}


//
// A_SorcBallOrbit() ==========================================
//

void A_SorcBallOrbit(mobj_t *actor)
{
	int x,y;
	angle_t angle, baseangle;
	int mode = actor->target->args[3];
	mobj_t *parent = (mobj_t *)actor->target;
	int dist = parent->radius - (actor->radius<<1);
	angle_t prevangle = actor->special1;
	
	if (actor->target->health <= 0)
		P_SetMobjState(actor, actor->info->painstate);

	baseangle = (angle_t)parent->special1;
	switch(actor->type)
	{
		case MT_SORCBALL1:
			angle = baseangle + BALL1_ANGLEOFFSET;
			break;
		case MT_SORCBALL2:
			angle = baseangle + BALL2_ANGLEOFFSET;
			break;
		case MT_SORCBALL3:
			angle = baseangle + BALL3_ANGLEOFFSET;
			break;
		default:
			I_Error("corrupted sorcerer");
			break;
	}
	actor->angle = angle;
	angle >>= ANGLETOFINESHIFT;

	switch(mode)
	{
		case SORC_NORMAL:				// Balls rotating normally
			A_SorcUpdateBallAngle(actor);
			break;
		case SORC_DECELERATE:		// Balls decelerating
			A_DecelBalls(actor);
			A_SorcUpdateBallAngle(actor);
			break;
		case SORC_ACCELERATE:		// Balls accelerating
			A_AccelBalls(actor);
			A_SorcUpdateBallAngle(actor);
			break;
		case SORC_STOPPING:			// Balls stopping
			if ((parent->special2 == actor->type) &&
				 (parent->args[1] > SORCBALL_SPEED_ROTATIONS) &&
				 (abs(angle - (parent->angle>>ANGLETOFINESHIFT)) < (30<<5)))
			{
				// Can stop now
				actor->target->args[3] = SORC_FIRESPELL;
				actor->target->args[4] = 0;
				// Set angle so ball angle == sorcerer angle
				switch(actor->type)
				{
					case MT_SORCBALL1:
						parent->special1 = (int)(parent->angle -
														BALL1_ANGLEOFFSET);
						break;
					case MT_SORCBALL2:
						parent->special1 = (int)(parent->angle -
														BALL2_ANGLEOFFSET);
						break;
					case MT_SORCBALL3:
						parent->special1 = (int)(parent->angle -
														BALL3_ANGLEOFFSET);
						break;
					default:
						break;
				}
			}
			else
			{
				A_SorcUpdateBallAngle(actor);
			}
			break;
		case SORC_FIRESPELL:			// Casting spell
			if (parent->special2 == actor->type)
			{
				// Put sorcerer into special throw spell anim
				if (parent->health > 0)
					P_SetMobjStateNF(parent, S_SORC_ATTACK1);

				if (actor->type==MT_SORCBALL1 && P_Random()<200)
				{
					S_StartSound(NULL, SFX_SORCERER_SPELLCAST);
					actor->special2 = SORCFX4_RAPIDFIRE_TIME;
					actor->args[4] = 128;
					parent->args[3] = SORC_FIRING_SPELL;
				}
				else
				{
					A_CastSorcererSpell(actor);
					parent->args[3] = SORC_STOPPED;
				}
			}
			break;
		case SORC_FIRING_SPELL:
			if (parent->special2 == actor->type)
			{
				if (actor->special2-- <= 0)
				{
					// Done rapid firing 
					parent->args[3] = SORC_STOPPED;
					// Back to orbit balls
					if (parent->health > 0)
						P_SetMobjStateNF(parent, S_SORC_ATTACK4);	
				}
				else
				{
					// Do rapid fire spell
					A_SorcOffense2(actor);
				}
			}
			break;
		case SORC_STOPPED:			// Balls stopped
		default:
			break;
	}

	if ((angle < prevangle) && (parent->args[4]==SORCBALL_TERMINAL_SPEED))
	{
		parent->args[1]++;			// Bump rotation counter
		// Completed full rotation - make woosh sound
		S_StartSound(actor, SFX_SORCERER_BALLWOOSH);
	}
	actor->special1 = angle;		// Set previous angle
	x = parent->x + FixedMul(dist, finecosine[angle]);
	y = parent->y + FixedMul(dist, finesine[angle]);
	actor->x = x;
	actor->y = y;
	actor->z = parent->z - parent->floorclip + parent->info->height;
}


//
// Set balls to speed mode - actor is sorcerer
//
void A_SpeedBalls(mobj_t *actor)
{
	actor->args[3] = SORC_ACCELERATE;				// speed mode
	actor->args[2] = SORCBALL_TERMINAL_SPEED;		// target speed
}


//
// Set balls to slow mode - actor is sorcerer
//
void A_SlowBalls(mobj_t *actor)
{
	actor->args[3] = SORC_DECELERATE;				// slow mode
	actor->args[2] = SORCBALL_INITIAL_SPEED;		// target speed
}


//
// Instant stop when rotation gets to ball in special2
//		actor is sorcerer
//
void A_StopBalls(mobj_t *actor)
{
	int chance = P_Random();
	actor->args[3] = SORC_STOPPING;				// stopping mode
	actor->args[1] = 0;							// Reset rotation counter

	if ((actor->args[0] <= 0) && (chance < 200))
	{
		actor->special2 = MT_SORCBALL2;			// Blue
	}
	else if((actor->health < (actor->info->spawnhealth >> 1)) &&
			(chance < 200))
	{
		actor->special2 = MT_SORCBALL3;			// Green
	}
	else
	{
		actor->special2 = MT_SORCBALL1;			// Yellow
	}


}


//
// Increase ball orbit speed - actor is ball
//
void A_AccelBalls(mobj_t *actor)
{
	mobj_t *sorc = actor->target;

	if (sorc->args[4] < sorc->args[2])
	{
		sorc->args[4]++;
	}
	else
	{
		sorc->args[3] = SORC_NORMAL;
		if (sorc->args[4] >= SORCBALL_TERMINAL_SPEED)
		{
			// Reached terminal velocity - stop balls
			A_StopBalls(sorc);
		}
	}
}


// Decrease ball orbit speed - actor is ball
void A_DecelBalls(mobj_t *actor)
{
	mobj_t *sorc = actor->target;

	if (sorc->args[4] > sorc->args[2])
	{
		sorc->args[4]--;
	}
	else
	{
		sorc->args[3] = SORC_NORMAL;
	}
}


// Update angle if first ball - actor is ball
void A_SorcUpdateBallAngle(mobj_t *actor)
{
	if (actor->type == MT_SORCBALL1)
	{
		actor->target->special1 += ANGLE_1*actor->target->args[4];
	}
}


// actor is ball
void A_CastSorcererSpell(mobj_t *actor)
{
	mobj_t *mo;
	int spell = actor->type;
	angle_t ang1,ang2;
	fixed_t z;
	mobj_t *parent = actor->target;

	S_StartSound(NULL, SFX_SORCERER_SPELLCAST);

	// Put sorcerer into throw spell animation
	if (parent->health > 0) P_SetMobjStateNF(parent, S_SORC_ATTACK4);

	switch(spell)
	{
		case MT_SORCBALL1:				// Offensive
			A_SorcOffense1(actor);
			break;
		case MT_SORCBALL2:				// Defensive
			z = parent->z - parent->floorclip + 
				SORC_DEFENSE_HEIGHT*FRACUNIT;
			mo = P_SpawnMobj(actor->x, actor->y, z, MT_SORCFX2);
			parent->flags2 |= MF2_REFLECTIVE|MF2_INVULNERABLE;
			parent->args[0] = SORC_DEFENSE_TIME;
			if (mo) mo->target = parent;
			break;
		case MT_SORCBALL3:				// Reinforcements
			ang1 = actor->angle - ANGLE_45;
			ang2 = actor->angle + ANGLE_45;
			if(actor->health < (actor->info->spawnhealth/3))
			{	// Spawn 2 at a time
				mo = P_SpawnMissileAngle(parent, MT_SORCFX3, ang1, 4*FRACUNIT);
				if (mo) mo->target = parent;
				mo = P_SpawnMissileAngle(parent, MT_SORCFX3, ang2, 4*FRACUNIT);
				if (mo) mo->target = parent;
			}			
			else
			{
				if (P_Random() < 128)
					ang1 = ang2;
				mo = P_SpawnMissileAngle(parent, MT_SORCFX3, ang1, 4*FRACUNIT);
				if (mo) mo->target = parent;
			}
			break;
		default:
			break;
	}
}

/*
void A_SpawnReinforcements(mobj_t *actor)
{
	mobj_t *parent = actor->target;
	mobj_t *mo;
	angle_t ang;

	ang = ANGLE_1 * P_Random();
	mo = P_SpawnMissileAngle(actor, MT_SORCFX3, ang, 5*FRACUNIT);
	if (mo) mo->target = parent;
}
*/

// actor is ball
void A_SorcOffense1(mobj_t *actor)
{
	mobj_t *mo;
	angle_t ang1,ang2;
	mobj_t *parent=(mobj_t *)actor->target;

	ang1 = actor->angle + ANGLE_1*70;
	ang2 = actor->angle - ANGLE_1*70;
	mo = P_SpawnMissileAngle(parent, MT_SORCFX1, ang1, 0);
	if (mo)
	{
		mo->target = parent;
		mo->special1 = (int)parent->target;
		mo->args[4] = BOUNCE_TIME_UNIT;
		mo->args[3] = 15;				// Bounce time in seconds
	}
	mo = P_SpawnMissileAngle(parent, MT_SORCFX1, ang2, 0);
	if (mo)
	{
		mo->target = parent;
		mo->special1 = (int)parent->target;
		mo->args[4] = BOUNCE_TIME_UNIT;
		mo->args[3] = 15;				// Bounce time in seconds
	}
}


// Actor is ball
void A_SorcOffense2(mobj_t *actor)
{
	angle_t ang1;
	mobj_t *mo;
	int delta, index;
	mobj_t *parent = actor->target;
	mobj_t *dest = parent->target;
	int dist;

	index = actor->args[4] << 5;
	actor->args[4] += 15;
	delta = (finesine[index])*SORCFX4_SPREAD_ANGLE;
	delta = (delta>>FRACBITS)*ANGLE_1;
	ang1 = actor->angle + delta;
	mo = P_SpawnMissileAngle(parent, MT_SORCFX4, ang1, 0);
	if (mo)
	{
		mo->special2 = 35*5/2;		// 5 seconds
		dist = P_AproxDistance(dest->x - mo->x, dest->y - mo->y);
		dist = dist/mo->info->speed;
		if(dist < 1) dist = 1;
		mo->momz = (dest->z-mo->z)/dist;
	}
}


// Resume ball spinning
void A_SorcBossAttack(mobj_t *actor)
{
	actor->args[3] = SORC_ACCELERATE;
	actor->args[2] = SORCBALL_INITIAL_SPEED;
}


// spell cast magic fizzle
void A_SpawnFizzle(mobj_t *actor)
{
	fixed_t x,y,z;
	fixed_t dist = 5*FRACUNIT;
	angle_t angle = actor->angle >> ANGLETOFINESHIFT;
	fixed_t speed = actor->info->speed;
	angle_t rangle;
	mobj_t *mo;
	int ix;

	x = actor->x + FixedMul(dist,finecosine[angle]);
	y = actor->y + FixedMul(dist,finesine[angle]);
	z = actor->z - actor->floorclip + (actor->height>>1);
	for (ix=0; ix<5; ix++)
	{
		mo = P_SpawnMobj(x,y,z,MT_SORCSPARK1);
		if (mo)
		{
			rangle = angle + ((P_Random()%5) << 1);
			mo->momx = FixedMul(P_Random()%speed,finecosine[rangle]);
			mo->momy = FixedMul(P_Random()%speed,finesine[rangle]);
			mo->momz = FRACUNIT*2;
		}
	}
}


//============================================================================
// Yellow spell - offense
//============================================================================

void A_SorcFX1Seek(mobj_t *actor)
{
	A_BounceCheck(actor);
	P_SeekerMissile(actor,ANGLE_1*2,ANGLE_1*6);
}


//============================================================================
// Blue spell - defense
//============================================================================
//
// FX2 Variables
//		special1		current angle
//		special2
//		args[0]		0 = CW,  1 = CCW
//		args[1]		
//============================================================================

// Split ball in two
void A_SorcFX2Split(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SORCFX2);
	if (mo)
	{
		mo->target = actor->target;
		mo->args[0] = 0;									// CW
		mo->special1 = actor->angle;					// Set angle
		P_SetMobjStateNF(mo, S_SORCFX2_ORBIT1);
	}
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SORCFX2);
	if (mo)
	{
		mo->target = actor->target;
		mo->args[0] = 1;									// CCW
		mo->special1 = actor->angle;					// Set angle
		P_SetMobjStateNF(mo, S_SORCFX2_ORBIT1);
	}
	P_SetMobjStateNF(actor, S_NULL);
}


// Orbit FX2 about sorcerer
void A_SorcFX2Orbit(mobj_t *actor)
{
	angle_t angle;
	fixed_t x,y,z;
	mobj_t *parent = actor->target;
	fixed_t dist = parent->info->radius;

	if ((parent->health <= 0) ||		// Sorcerer is dead
		(!parent->args[0]))				// Time expired
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
		parent->args[0] = 0;
		parent->flags2 &= ~MF2_REFLECTIVE;
		parent->flags2 &= ~MF2_INVULNERABLE;
	}

	if (actor->args[0] && (parent->args[0]-- <= 0))		// Time expired
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
		parent->args[0] = 0;
		parent->flags2 &= ~MF2_REFLECTIVE;
	}

	// Move to new position based on angle
	if (actor->args[0])		// Counter clock-wise
	{
		actor->special1 += ANGLE_1*10;
		angle = ((angle_t)actor->special1) >> ANGLETOFINESHIFT;
		x = parent->x + FixedMul(dist, finecosine[angle]);
		y = parent->y + FixedMul(dist, finesine[angle]);
		z = parent->z - parent->floorclip + SORC_DEFENSE_HEIGHT*FRACUNIT;
		z += FixedMul(15*FRACUNIT,finecosine[angle]);
		// Spawn trailer
		P_SpawnMobj(x,y,z, MT_SORCFX2_T1);
	}
	else							// Clock wise
	{
		actor->special1 -= ANGLE_1*10;
		angle = ((angle_t)actor->special1) >> ANGLETOFINESHIFT;
		x = parent->x + FixedMul(dist, finecosine[angle]);
		y = parent->y + FixedMul(dist, finesine[angle]);
		z = parent->z - parent->floorclip + SORC_DEFENSE_HEIGHT*FRACUNIT;
		z += FixedMul(20*FRACUNIT,finesine[angle]);
		// Spawn trailer
		P_SpawnMobj(x,y,z, MT_SORCFX2_T1);
	}

	actor->x = x;
	actor->y = y;
	actor->z = z;
}



//============================================================================
// Green spell - spawn bishops
//============================================================================

void A_SpawnBishop(mobj_t *actor)
{
	mobj_t *mo;
	mo=P_SpawnMobj(actor->x, actor->y, actor->z, MT_BISHOP);
	if(mo)
	{
		if(!P_TestMobjLocation(mo))
		{
			P_SetMobjState(mo, S_NULL);
		}
	}
	P_SetMobjState(actor, S_NULL);
}

/*
void A_SmokePuffEntry(mobj_t *actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MT_MNTRSMOKE);
}
*/

void A_SmokePuffExit(mobj_t *actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MT_MNTRSMOKEEXIT);
}

void A_SorcererBishopEntry(mobj_t *actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MT_SORCFX3_EXPLOSION);
	S_StartSound(actor, actor->info->seesound);
}


//============================================================================
// FX4 - rapid fire balls
//============================================================================

void A_SorcFX4Check(mobj_t *actor)
{
	if (actor->special2-- <= 0)
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
	}
}

//============================================================================
// Ball death - spawn stuff
//============================================================================

void A_SorcBallPop(mobj_t *actor)
{
	S_StartSound(NULL, SFX_SORCERER_BALLPOP);
	actor->flags &= ~MF_NOGRAVITY;
	actor->flags2 |= MF2_LOGRAV;
	actor->momx = ((P_Random()%10)-5) << FRACBITS;
	actor->momy = ((P_Random()%10)-5) << FRACBITS;
	actor->momz = (2+(P_Random()%3)) << FRACBITS;
	actor->special2 = 4*FRACUNIT;		// Initial bounce factor
	actor->args[4] = BOUNCE_TIME_UNIT;	// Bounce time unit
	actor->args[3] = 5;					// Bounce time in seconds
}



void A_BounceCheck(mobj_t *actor)
{
	if (actor->args[4]-- <= 0)
	{
		if (actor->args[3]-- <= 0)
		{
			P_SetMobjState(actor, actor->info->deathstate);
			switch(actor->type)
			{
				case MT_SORCBALL1:
				case MT_SORCBALL2:
				case MT_SORCBALL3:
					S_StartSound(NULL, SFX_SORCERER_BIGBALLEXPLODE);
					break;
				case MT_SORCFX1:
					S_StartSound(NULL, SFX_SORCERER_HEADSCREAM);
					break;
				default:
					break;
			}
		}
		else
		{
			actor->args[4] = BOUNCE_TIME_UNIT;
		}
	}
}




//============================================================================
// Class Bosses
//============================================================================
#define CLASS_BOSS_STRAFE_RANGE	64*10*FRACUNIT

void A_FastChase(mobj_t *actor)
{
	int delta;
	fixed_t dist;
	angle_t ang;
	mobj_t *target;

	if(actor->reactiontime)
	{
		actor->reactiontime--;
	}

	// Modify target threshold
	if(actor->threshold)
	{
		actor->threshold--;
	}

	if(gameskill == sk_nightmare)
	{ // Monsters move faster in nightmare mode
		actor->tics -= actor->tics/2;
		if(actor->tics < 3)
		{
			actor->tics = 3;
		}
	}

//
// turn towards movement direction if not there yet
//
	if(actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle-(actor->movedir << 29);
		if(delta > 0)
		{
			actor->angle -= ANG90/2;
		}
		else if(delta < 0)
		{
			actor->angle += ANG90/2;
		}
	}

	if(!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{ // look for a new target
		if(P_LookForPlayers(actor, true))
		{ // got a new target
			return;
		}
		P_SetMobjState(actor, actor->info->spawnstate);
		return;
	}

//
// don't attack twice in a row
//
	if(actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		if (gameskill != sk_nightmare)
			P_NewChaseDir (actor);
		return;
	}

	// Strafe
	if (actor->special2 > 0)
	{
		actor->special2--;
	}
	else
	{
		target = actor->target;
		actor->special2 = 0;
		actor->momx = actor->momy = 0;
		dist=P_AproxDistance(actor->x - target->x,
								actor->y - target->y);
		if (dist < CLASS_BOSS_STRAFE_RANGE)
		{
			if (P_Random()<100)
			{
				ang = R_PointToAngle2(actor->x, actor->y,
									target->x, target->y);
				if (P_Random()<128)
					ang += ANGLE_90;
				else
					ang -= ANGLE_90;
				ang>>=ANGLETOFINESHIFT;
				actor->momx = FixedMul(13*FRACUNIT, finecosine[ang]);
				actor->momy = FixedMul(13*FRACUNIT, finesine[ang]);
				actor->special2 = 3;		// strafe time
			}
		}
	}

//
// check for missile attack
//
	if (actor->info->missilestate)
	{
		if (gameskill < sk_nightmare && actor->movecount)
			goto nomissile;
		if (!P_CheckMissileRange (actor))
			goto nomissile;
		P_SetMobjState (actor, actor->info->missilestate);
		actor->flags |= MF_JUSTATTACKED;
		return;
	}
nomissile:

//
// possibly choose another target
//
	if (netgame && !actor->threshold && !P_CheckSight (actor, actor->target) )
	{
		if (P_LookForPlayers(actor,true))
			return;         // got a new target
	}

//
// chase towards player
//
	if (!actor->special2)
	{
		if (--actor->movecount<0 || !P_Move (actor))
		{
			P_NewChaseDir (actor);
		}
	}
}


void A_FighterAttack(mobj_t *actor)
{
	extern void A_FSwordAttack2(mobj_t *actor);

	if(!actor->target) return;
	A_FSwordAttack2(actor);
}


void A_ClericAttack(mobj_t *actor)
{
	extern void A_CHolyAttack3(mobj_t *actor);

	if(!actor->target) return;
	A_CHolyAttack3(actor);
}



void A_MageAttack(mobj_t *actor)
{
	extern void A_MStaffAttack2(mobj_t *actor);

	if(!actor->target) return;
	A_MStaffAttack2(actor);
}

void A_ClassBossHealth(mobj_t *actor)
{
	if (netgame && !deathmatch)		// co-op only
	{
		if (!actor->special1)
		{
			actor->health *= 5;
			actor->special1 = true;   // has been initialized
		}
	}
}


//===========================================================================
//
// A_CheckFloor - Checks if an object hit the floor
//
//===========================================================================

void A_CheckFloor(mobj_t *actor)
{
	if(actor->z <= actor->floorz)
	{
		actor->z = actor->floorz;
		actor->flags2 &= ~MF2_LOGRAV;
		P_SetMobjState(actor, actor->info->deathstate);
	}
}

//============================================================================
//
// A_FreezeDeath
//
//============================================================================

void A_FreezeDeath(mobj_t *actor)
{
	actor->tics = 75+P_Random()+P_Random();
	actor->flags |= MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
	actor->flags2 |= MF2_PUSHABLE|MF2_TELESTOMP|MF2_PASSMOBJ|MF2_SLIDE;
	actor->height <<= 2;
	S_StartSound(actor, SFX_FREEZE_DEATH);

	if(actor->player)
	{
		actor->player->damagecount = 0;
		actor->player->poisoncount = 0;
		actor->player->bonuscount = 0;
		if(actor->player == &players[consoleplayer])
		{
			SB_PaletteFlash(false);
		}
	}
	else if(actor->flags&MF_COUNTKILL && actor->special)
	{ // Initiate monster death actions
		P_ExecuteLineSpecial(actor->special, actor->args, NULL, 0, actor);
	}
}

//============================================================================
//
// A_IceSetTics
//
//============================================================================

void A_IceSetTics(mobj_t *actor)
{
	int floor;

	actor->tics = 70+(P_Random()&63);
	floor = P_GetThingFloorType(actor);
	if(floor == FLOOR_LAVA)
	{
		actor->tics >>= 2;
	}
	else if(floor == FLOOR_ICE)
	{
		actor->tics <<= 1;
	}
}

//============================================================================
//
// A_IceCheckHeadDone
//
//============================================================================

void A_IceCheckHeadDone(mobj_t *actor)
{
	if(actor->special2 == 666)
	{
		P_SetMobjState(actor, S_ICECHUNK_HEAD2);
	}
}

//============================================================================
//
// A_FreezeDeathChunks
//
//============================================================================

void A_FreezeDeathChunks(mobj_t *actor)
{
	int i;
	mobj_t *mo;
	
	if(actor->momx || actor->momy || actor->momz)
	{
		actor->tics = 105;
		return;
	}
	S_StartSound(actor, SFX_FREEZE_SHATTER);

	for(i = 12+(P_Random()&15); i >= 0; i--)
	{
		mo = P_SpawnMobj(actor->x+(((P_Random()-128)*actor->radius)>>7), 
			actor->y+(((P_Random()-128)*actor->radius)>>7), 
			actor->z+(P_Random()*actor->height/255), MT_ICECHUNK);
		P_SetMobjState(mo, mo->info->spawnstate+(P_Random()%3));
		if(mo)
		{
			mo->momz = FixedDiv(mo->z-actor->z, actor->height)<<2;
			mo->momx = (P_Random()-P_Random())<<(FRACBITS-7);
			mo->momy = (P_Random()-P_Random())<<(FRACBITS-7);
			A_IceSetTics(mo); // set a random tic wait
		}
	}
	for(i = 12+(P_Random()&15); i >= 0; i--)
	{
		mo = P_SpawnMobj(actor->x+(((P_Random()-128)*actor->radius)>>7), 
			actor->y+(((P_Random()-128)*actor->radius)>>7), 
			actor->z+(P_Random()*actor->height/255), MT_ICECHUNK);
		P_SetMobjState(mo, mo->info->spawnstate+(P_Random()%3));
		if(mo)
		{
			mo->momz = FixedDiv(mo->z-actor->z, actor->height)<<2;
			mo->momx = (P_Random()-P_Random())<<(FRACBITS-7);
			mo->momy = (P_Random()-P_Random())<<(FRACBITS-7);
			A_IceSetTics(mo); // set a random tic wait
		}
	}
	if(actor->player)
	{ // attach the player's view to a chunk of ice
		mo = P_SpawnMobj(actor->x, actor->y, actor->z+VIEWHEIGHT, MT_ICECHUNK);
		P_SetMobjState(mo, S_ICECHUNK_HEAD);
		mo->momz = FixedDiv(mo->z-actor->z, actor->height)<<2;
		mo->momx = (P_Random()-P_Random())<<(FRACBITS-7);
		mo->momy = (P_Random()-P_Random())<<(FRACBITS-7);
		mo->flags2 |= MF2_ICEDAMAGE; // used to force blue palette
		mo->flags2 &= ~MF2_FLOORCLIP;
		mo->player = actor->player;
		actor->player = NULL;
		mo->health = actor->health;
		mo->angle = actor->angle;
		mo->player->mo = mo;
		mo->player->lookdir = 0;
	}
	P_RemoveMobjFromTIDList(actor);
	P_SetMobjState(actor, S_FREETARGMOBJ);
	actor->flags2 |= MF2_DONTDRAW;
}

//===========================================================================
// Korax Variables
//	special1	last teleport destination
//	special2	set if "below half" script not yet run
//
// Korax Scripts (reserved)
//	249		Tell scripts that we are below half health
//	250-254	Control scripts
//	255		Death script
//
// Korax TIDs (reserved)
//	245		Reserved for Korax himself
//  248		Initial teleport destination
//	249		Teleport destination
//	250-254	For use in respective control scripts
//	255		For use in death script (spawn spots)
//===========================================================================
#define KORAX_SPIRIT_LIFETIME	(5*(35/5))	// 5 seconds
#define KORAX_COMMAND_HEIGHT	(120*FRACUNIT)
#define KORAX_COMMAND_OFFSET	(27*FRACUNIT)

void KoraxFire1(mobj_t *actor, int type);
void KoraxFire2(mobj_t *actor, int type);
void KoraxFire3(mobj_t *actor, int type);
void KoraxFire4(mobj_t *actor, int type);
void KoraxFire5(mobj_t *actor, int type);
void KoraxFire6(mobj_t *actor, int type);
void KSpiritInit(mobj_t *spirit, mobj_t *korax);

#define KORAX_TID					(245)
#define KORAX_FIRST_TELEPORT_TID	(248)
#define KORAX_TELEPORT_TID			(249)

void A_KoraxChase(mobj_t *actor)
{
	mobj_t *spot;
	int lastfound;
	byte args[3]={0,0,0};

	if ((!actor->special2) &&
		(actor->health <= (actor->info->spawnhealth/2)))
	{
		lastfound = 0;
		spot = P_FindMobjFromTID(KORAX_FIRST_TELEPORT_TID, &lastfound);
		if (spot)
		{
			P_Teleport(actor, spot->x, spot->y, spot->angle, true);
		}

		P_StartACS(249, 0, args, actor, NULL, 0);
		actor->special2 = 1;	// Don't run again

		return;
	}

	if (!actor->target) return;
	if (P_Random()<30)
	{
		P_SetMobjState(actor, actor->info->missilestate);
	}
	else if (P_Random()<30)
	{
		S_StartSound(NULL, SFX_KORAX_ACTIVE);
	}

	// Teleport away
	if (actor->health < (actor->info->spawnhealth>>1))
	{
		if (P_Random()<10)
		{
			lastfound = actor->special1;
			spot = P_FindMobjFromTID(KORAX_TELEPORT_TID, &lastfound);
			actor->special1 = lastfound;
			if (spot)
			{
				P_Teleport(actor, spot->x, spot->y, spot->angle, true);
			}
		}
	}
}

void A_KoraxStep(mobj_t *actor)
{
	A_Chase(actor);
}

void A_KoraxStep2(mobj_t *actor)
{
	S_StartSound(NULL, SFX_KORAX_STEP);
	A_Chase(actor);
}

void A_KoraxBonePop(mobj_t *actor)
{
	fixed_t x,y,z;
	mobj_t *mo;
	byte args[5];

	args[0]=args[1]=args[2]=args[3]=args[4]=0;
	x=actor->x, y=actor->y, z=actor->z;

	// Spawn 6 spirits equalangularly
	mo = P_SpawnMissileAngle(actor,MT_KORAX_SPIRIT1, ANGLE_60*0, 5*FRACUNIT);
	if (mo) KSpiritInit(mo,actor);
	mo = P_SpawnMissileAngle(actor,MT_KORAX_SPIRIT2, ANGLE_60*1, 5*FRACUNIT);
	if (mo) KSpiritInit(mo,actor);
	mo = P_SpawnMissileAngle(actor,MT_KORAX_SPIRIT3, ANGLE_60*2, 5*FRACUNIT);
	if (mo) KSpiritInit(mo,actor);
	mo = P_SpawnMissileAngle(actor,MT_KORAX_SPIRIT4, ANGLE_60*3, 5*FRACUNIT);
	if (mo) KSpiritInit(mo,actor);
	mo = P_SpawnMissileAngle(actor,MT_KORAX_SPIRIT5, ANGLE_60*4, 5*FRACUNIT);
	if (mo) KSpiritInit(mo,actor);
	mo = P_SpawnMissileAngle(actor,MT_KORAX_SPIRIT6, ANGLE_60*5, 5*FRACUNIT);
	if (mo) KSpiritInit(mo,actor);

	P_StartACS(255, 0, args, actor, NULL, 0);		// Death script
}

void KSpiritInit(mobj_t *spirit, mobj_t *korax)
{
	int i;
	mobj_t *tail, *next;

	spirit->health = KORAX_SPIRIT_LIFETIME;

	spirit->special1 = (int)korax;				// Swarm around korax
	spirit->special2 = 32+(P_Random()&7);		// Float bob index
	spirit->args[0] = 10; 						// initial turn value
	spirit->args[1] = 0; 						// initial look angle

	// Spawn a tail for spirit
	tail = P_SpawnMobj(spirit->x, spirit->y, spirit->z, MT_HOLY_TAIL);
	tail->special2 = (int)spirit; // parent
	for(i = 1; i < 3; i++)
	{
		next = P_SpawnMobj(spirit->x, spirit->y, spirit->z, MT_HOLY_TAIL);
		P_SetMobjState(next, next->info->spawnstate+1);
		tail->special1 = (int)next;
		tail = next;
	}
	tail->special1 = 0; // last tail bit
}

void A_KoraxDecide(mobj_t *actor)
{
	if (P_Random()<220)
	{
		P_SetMobjState(actor, S_KORAX_MISSILE1);
	}
	else
	{
		P_SetMobjState(actor, S_KORAX_COMMAND1);
	}
}

void A_KoraxMissile(mobj_t *actor)
{
	int type = P_Random()%6;
	int sound;

	S_StartSound(actor, SFX_KORAX_ATTACK);

	switch(type)
	{
		case 0:
			type = MT_WRAITHFX1;
			sound = SFX_WRAITH_MISSILE_FIRE;
			break;
		case 1:
			type = MT_DEMONFX1;
			sound = SFX_DEMON_MISSILE_FIRE;
			break;
		case 2:
			type = MT_DEMON2FX1;
			sound = SFX_DEMON_MISSILE_FIRE;
			break;
		case 3:
			type = MT_FIREDEMON_FX6;
			sound = SFX_FIRED_ATTACK;
			break;
		case 4:
			type = MT_CENTAUR_FX;
			sound = SFX_CENTAURLEADER_ATTACK;
			break;
		case 5:
			type = MT_SERPENTFX;
			sound = SFX_CENTAURLEADER_ATTACK;
			break;
	}

	// Fire all 6 missiles at once
	S_StartSound(NULL, sound);
	KoraxFire1(actor, type);
	KoraxFire2(actor, type);
	KoraxFire3(actor, type);
	KoraxFire4(actor, type);
	KoraxFire5(actor, type);
	KoraxFire6(actor, type);
}


// Call action code scripts (250-254)
void A_KoraxCommand(mobj_t *actor)
{
	byte args[5];
	fixed_t x,y,z;
	angle_t ang;
	int numcommands;

	S_StartSound(actor, SFX_KORAX_COMMAND);

	// Shoot stream of lightning to ceiling
	ang = (actor->angle - ANGLE_90) >> ANGLETOFINESHIFT;
	x=actor->x + FixedMul(KORAX_COMMAND_OFFSET,finecosine[ang]);
	y=actor->y + FixedMul(KORAX_COMMAND_OFFSET,finesine[ang]);
	z=actor->z + KORAX_COMMAND_HEIGHT;
	P_SpawnMobj(x,y,z, MT_KORAX_BOLT);

	args[0]=args[1]=args[2]=args[3]=args[4]=0;

	if (actor->health <= (actor->info->spawnhealth >> 1))
	{
		numcommands = 5;
	}
	else
	{
		numcommands = 4;
	}

	switch(P_Random() % numcommands)
	{
		case 0:
			P_StartACS(250, 0, args, actor, NULL, 0);
			break;
		case 1:
			P_StartACS(251, 0, args, actor, NULL, 0);
			break;
		case 2:
			P_StartACS(252, 0, args, actor, NULL, 0);
			break;
		case 3:
			P_StartACS(253, 0, args, actor, NULL, 0);
			break;
		case 4:
			P_StartACS(254, 0, args, actor, NULL, 0);
			break;
	}
}


#define KORAX_DELTAANGLE			(85*ANGLE_1)
#define KORAX_ARM_EXTENSION_SHORT	(40*FRACUNIT)
#define KORAX_ARM_EXTENSION_LONG	(55*FRACUNIT)

#define KORAX_ARM1_HEIGHT			(108*FRACUNIT)
#define KORAX_ARM2_HEIGHT			(82*FRACUNIT)
#define KORAX_ARM3_HEIGHT			(54*FRACUNIT)
#define KORAX_ARM4_HEIGHT			(104*FRACUNIT)
#define KORAX_ARM5_HEIGHT			(86*FRACUNIT)
#define KORAX_ARM6_HEIGHT			(53*FRACUNIT)


// Arm projectiles
//		arm positions numbered:
//			1	top left
//			2	middle left
//			3	lower left
//			4	top right
//			5	middle right
//			6	lower right


// Arm 1 projectile
void KoraxFire1(mobj_t *actor, int type)
{
	mobj_t *mo;
	angle_t ang;
	fixed_t x,y,z;

	ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_SHORT, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_SHORT, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM1_HEIGHT;
	mo = P_SpawnKoraxMissile(x,y,z,actor, actor->target, type);
}


// Arm 2 projectile
void KoraxFire2(mobj_t *actor, int type)
{
	mobj_t *mo;
	angle_t ang;
	fixed_t x,y,z;

	ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM2_HEIGHT;
	mo = P_SpawnKoraxMissile(x,y,z,actor, actor->target, type);
}

// Arm 3 projectile
void KoraxFire3(mobj_t *actor, int type)
{
	mobj_t *mo;
	angle_t ang;
	fixed_t x,y,z;

	ang = (actor->angle - KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM3_HEIGHT;
	mo = P_SpawnKoraxMissile(x,y,z,actor, actor->target, type);
}

// Arm 4 projectile
void KoraxFire4(mobj_t *actor, int type)
{
	mobj_t *mo;
	angle_t ang;
	fixed_t x,y,z;

	ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_SHORT, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_SHORT, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM4_HEIGHT;
	mo = P_SpawnKoraxMissile(x,y,z,actor, actor->target, type);
}

// Arm 5 projectile
void KoraxFire5(mobj_t *actor, int type)
{
	mobj_t *mo;
	angle_t ang;
	fixed_t x,y,z;

	ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM5_HEIGHT;
	mo = P_SpawnKoraxMissile(x,y,z,actor, actor->target, type);
}

// Arm 6 projectile
void KoraxFire6(mobj_t *actor, int type)
{
	mobj_t *mo;
	angle_t ang;
	fixed_t x,y,z;

	ang = (actor->angle + KORAX_DELTAANGLE) >> ANGLETOFINESHIFT;
	x = actor->x + FixedMul(KORAX_ARM_EXTENSION_LONG, finecosine[ang]);
	y = actor->y + FixedMul(KORAX_ARM_EXTENSION_LONG, finesine[ang]);
	z = actor->z - actor->floorclip + KORAX_ARM6_HEIGHT;
	mo = P_SpawnKoraxMissile(x,y,z,actor, actor->target, type);
}


void A_KSpiritWeave(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2>>16;
	weaveZ = actor->special2&0xFFFF;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]<<2);
	weaveXY = (weaveXY+(P_Random()%5))&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	P_TryMove(actor, newX, newY);
	actor->z -= FloatBobOffsets[weaveZ]<<1;
	weaveZ = (weaveZ+(P_Random()%5))&63;
	actor->z += FloatBobOffsets[weaveZ]<<1;	
	actor->special2 = weaveZ+(weaveXY<<16);
}

void A_KSpiritSeeker(mobj_t *actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t *target;
	fixed_t newZ;
	fixed_t deltaZ;

	target = (mobj_t *)actor->special1;
	if(target == NULL)
	{
		return;
	}
	dir = P_FaceMobj(actor, target, &delta);
	if(delta > thresh)
	{
		delta >>= 1;
		if(delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if(dir)
	{ // Turn clockwise
		actor->angle += delta;
	}
	else
	{ // Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);

	if(!(leveltime&15) 
		|| actor->z > target->z+(target->info->height)
		|| actor->z+actor->height < target->z)
	{
		newZ = target->z+((P_Random()*target->info->height)>>8);
		deltaZ = newZ-actor->z;
		if(abs(deltaZ) > 15*FRACUNIT)
		{
			if(deltaZ > 0)
			{
				deltaZ = 15*FRACUNIT;
			}
			else
			{
				deltaZ = -15*FRACUNIT;
			}
		}
		dist = P_AproxDistance(target->x-actor->x, target->y-actor->y);
		dist = dist/actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		actor->momz = deltaZ/dist;
	}
	return;
}


void A_KSpiritRoam(mobj_t *actor)
{
	if (actor->health-- <= 0)
	{
		S_StartSound(actor, SFX_SPIRIT_DIE);
		P_SetMobjState(actor, S_KSPIRIT_DEATH1);
	}
	else
	{
		if (actor->special1)
		{
			A_KSpiritSeeker(actor, actor->args[0]*ANGLE_1,
							actor->args[0]*ANGLE_1*2);
		}
		A_KSpiritWeave(actor);
		if (P_Random()<50)
		{
			S_StartSound(NULL, SFX_SPIRIT_ACTIVE);
		}
	}
}

void A_KBolt(mobj_t *actor)
{
	// Countdown lifetime
	if (actor->special1-- <= 0)
	{
		P_SetMobjState(actor, S_NULL);
	}
}


#define KORAX_BOLT_HEIGHT		48*FRACUNIT
#define KORAX_BOLT_LIFETIME		3

void A_KBoltRaise(mobj_t *actor)
{
	mobj_t *mo;
	fixed_t z;

	// Spawn a child upward
	z = actor->z + KORAX_BOLT_HEIGHT;

	if ((z + KORAX_BOLT_HEIGHT) < actor->ceilingz)
	{
		mo = P_SpawnMobj(actor->x, actor->y, z, MT_KORAX_BOLT);
		if (mo)
		{
			mo->special1 = KORAX_BOLT_LIFETIME;
		}
	}
	else
	{
		// Maybe cap it off here
	}
}

