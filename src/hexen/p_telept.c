
//**************************************************************************
//**
//** p_telept.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_telept.c,v $
//** $Revision: 1.13 $
//** $Date: 95/10/08 04:23:24 $
//** $Author: paul $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// P_Teleport
//
//==========================================================================

boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, angle_t angle,
	boolean useFog)
{
	fixed_t oldx;
	fixed_t oldy;
	fixed_t oldz;
	fixed_t aboveFloor;
	fixed_t fogDelta;
	player_t *player;
	unsigned an;
	mobj_t *fog;

	oldx = thing->x;
	oldy = thing->y;
	oldz = thing->z;
	aboveFloor = thing->z-thing->floorz;
	if(!P_TeleportMove(thing, x, y))
	{
		return false;
	}
	if(thing->player)
	{
		player = thing->player;
		if(player->powers[pw_flight] && aboveFloor)
		{
			thing->z = thing->floorz+aboveFloor;
			if(thing->z+thing->height > thing->ceilingz)
			{
				thing->z = thing->ceilingz-thing->height;
			}
			player->viewz = thing->z+player->viewheight;
		}
		else
		{
			thing->z = thing->floorz;
			player->viewz = thing->z+player->viewheight;
			if(useFog)
			{
				player->lookdir = 0;
			}
		}
	}
	else if(thing->flags&MF_MISSILE)
	{
		thing->z = thing->floorz+aboveFloor;
		if(thing->z+thing->height > thing->ceilingz)
		{
			thing->z = thing->ceilingz-thing->height;
		}
	}
	else
	{
		thing->z = thing->floorz;
	}
	// Spawn teleport fog at source and destination
	if(useFog)
	{
		fogDelta = thing->flags&MF_MISSILE ? 0 : TELEFOGHEIGHT;
		fog = P_SpawnMobj(oldx, oldy, oldz+fogDelta, MT_TFOG);
		S_StartSound(fog, SFX_TELEPORT);
		an = angle>>ANGLETOFINESHIFT;
		fog = P_SpawnMobj(x+20*finecosine[an],
			y+20*finesine[an], thing->z+fogDelta, MT_TFOG);
		S_StartSound(fog, SFX_TELEPORT);
		if(thing->player && !thing->player->powers[pw_speed])
		{ // Freeze player for about .5 sec
			thing->reactiontime = 18;
		}
		thing->angle = angle;
	}
	if(thing->flags2&MF2_FLOORCLIP)
	{
		if(thing->z == thing->subsector->sector->floorheight 
			&& P_GetThingFloorType(thing) > FLOOR_SOLID)
		{
			thing->floorclip = 10*FRACUNIT;
		}
		else
		{
			thing->floorclip = 0;
		}
	}
	if(thing->flags&MF_MISSILE)
	{
		angle >>= ANGLETOFINESHIFT;
		thing->momx = FixedMul(thing->info->speed, finecosine[angle]);
		thing->momy = FixedMul(thing->info->speed, finesine[angle]);
	}
	else if(useFog) // no fog doesn't alter the player's momentums
	{
		thing->momx = thing->momy = thing->momz = 0;
	}
	return true;
}

//==========================================================================
//
// EV_Teleport
//
//==========================================================================

boolean EV_Teleport(int tid, mobj_t *thing, boolean fog)
{
	int i;
	int count;
	mobj_t *mo;
	int searcher;

	if(!thing)
	{ // Teleport function called with an invalid mobj
		return false;
	}
	if(thing->flags2&MF2_NOTELEPORT)
	{
		return false;
	}
	count = 0;
	searcher = -1;
	while(P_FindMobjFromTID(tid, &searcher) != NULL)
	{
		count++;
	}
	if(count == 0)
	{
		return false;
	}
	count = 1+(P_Random()%count);
	searcher = -1;
	for(i = 0; i < count; i++)
	{
		mo = P_FindMobjFromTID(tid, &searcher);
	}
	if (!mo) I_Error("Can't find teleport mapspot\n");
	return P_Teleport(thing, mo->x, mo->y, mo->angle, fog);
}
