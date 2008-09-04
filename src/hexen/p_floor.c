
//**************************************************************************
//**
//** p_floor.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_floor.c,v $
//** $Revision: 1.23 $
//** $Date: 95/10/06 16:53:19 $
//** $Author: paul $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

extern fixed_t FloatBobOffsets[64];

//==================================================================
//==================================================================
//
//                                                              FLOORS
//
//==================================================================
//==================================================================

//==================================================================
//
//      Move a plane (floor or ceiling) and check for crushing
//
//==================================================================
result_e	T_MovePlane(sector_t *sector,fixed_t speed,
			fixed_t dest, int crush,int floorOrCeiling,int direction)
{
	boolean flag;
	fixed_t lastpos;

	switch(floorOrCeiling)
	{
		case 0:         // FLOOR
			switch(direction)
			{
				case -1:        // DOWN
					if (sector->floorheight - speed < dest)
					{
						lastpos = sector->floorheight;
						sector->floorheight = dest;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							sector->floorheight =lastpos;
							P_ChangeSector(sector,crush);
							//return RES_CRUSHED;
						}
						return RES_PASTDEST;
					}
					else
					{
						lastpos = sector->floorheight;
						sector->floorheight -= speed;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							sector->floorheight = lastpos;
							P_ChangeSector(sector,crush);
							return RES_CRUSHED;
						}
					}
					break;

				case 1:         // UP
					if (sector->floorheight + speed > dest)
					{
						lastpos = sector->floorheight;
						sector->floorheight = dest;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							sector->floorheight = lastpos;
							P_ChangeSector(sector,crush);
							//return RES_CRUSHED;
						}
						return RES_PASTDEST;
					}
					else    // COULD GET CRUSHED
					{
						lastpos = sector->floorheight;
						sector->floorheight += speed;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							//if (crush == true)
							//{
							//	return RES_CRUSHED;
							//}
							sector->floorheight = lastpos;
							P_ChangeSector(sector,crush);
							return RES_CRUSHED;
						}
					}
					break;
			}
			break;

		case 1:         // CEILING
			switch(direction)
			{
				case -1:        // DOWN
					if (sector->ceilingheight - speed < dest)
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight = dest;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							sector->ceilingheight = lastpos;
							P_ChangeSector(sector,crush);
							//return RES_CRUSHED;
						}
						return RES_PASTDEST;
					}
					else    // COULD GET CRUSHED
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight -= speed;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							//if (crush == true)
							//{
							//	return RES_CRUSHED;
							//}
							sector->ceilingheight = lastpos;
							P_ChangeSector(sector,crush);
							return RES_CRUSHED;
						}
					}
					break;

				case 1:         // UP
					if (sector->ceilingheight + speed > dest)
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight = dest;
						flag = P_ChangeSector(sector,crush);
						if (flag == true)
						{
							sector->ceilingheight = lastpos;
							P_ChangeSector(sector,crush);
							//return RES_CRUSHED;
						}
						return RES_PASTDEST;
					}
					else
					{
						lastpos = sector->ceilingheight;
						sector->ceilingheight += speed;
						flag = P_ChangeSector(sector,crush);
						#if 0
						if (flag == true)
						{
							sector->ceilingheight = lastpos;
							P_ChangeSector(sector,crush);
							return RES_CRUSHED;
						}
						#endif
					}
					break;
			}
			break;

	}
	return RES_OK;
}

//==================================================================
//
//      MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
//==================================================================
void T_MoveFloor(floormove_t *floor)
{
	result_e        res;

	if(floor->resetDelayCount)
	{
		floor->resetDelayCount--;
		if(!floor->resetDelayCount)
		{
			floor->floordestheight = floor->resetHeight;
			floor->direction = -floor->direction;
			floor->resetDelay = 0;
			floor->delayCount = 0;
			floor->delayTotal = 0;
		}
	}					
	if(floor->delayCount)
	{
		floor->delayCount--;
		if(!floor->delayCount && floor->textureChange)
		{
			floor->sector->floorpic += floor->textureChange;
		}
		return;
	}

	res = T_MovePlane(floor->sector,floor->speed,
			floor->floordestheight,floor->crush,0,floor->direction);

	if(floor->type == FLEV_RAISEBUILDSTEP)
	{
		if((floor->direction == 1 && floor->sector->floorheight >=
			floor->stairsDelayHeight) || (floor->direction == -1 &&
			floor->sector->floorheight <= floor->stairsDelayHeight))
		{
			floor->delayCount = floor->delayTotal;
			floor->stairsDelayHeight += floor->stairsDelayHeightDelta;
		}		
	}
	if (res == RES_PASTDEST)
	{
		SN_StopSequence((mobj_t *)&floor->sector->soundorg);
		if(floor->delayTotal)
		{
			floor->delayTotal = 0;
		}
		if(floor->resetDelay)
		{
//			floor->resetDelayCount = floor->resetDelay;
//			floor->resetDelay = 0;
			return;
		}			
		floor->sector->specialdata = NULL;
		/*
		if (floor->direction == 1)
			switch(floor->type)
			{
				case donutRaise:
					floor->sector->special = floor->newspecial;
					floor->sector->floorpic = floor->texture;
				default:
					break;
			}
		else if (floor->direction == -1)
			switch(floor->type)
			{
				case lowerAndChange:
					floor->sector->special = floor->newspecial;
					floor->sector->floorpic = floor->texture;
				default:
					break;
			}
		*/
		if(floor->textureChange)
		{
			floor->sector->floorpic -= floor->textureChange;
		}
		P_TagFinished(floor->sector->tag);
		P_RemoveThinker(&floor->thinker);
	}
}

//==================================================================
//
//      HANDLE FLOOR TYPES
//
//==================================================================
int EV_DoFloor(line_t *line, byte *args, floor_e floortype)
{
	int                     secnum;
	int                     rtn;
	sector_t        *sec;
	floormove_t     *floor=NULL;

	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
	{
		sec = &sectors[secnum];

		//      ALREADY MOVING?  IF SO, KEEP GOING...
		if (sec->specialdata)
			continue;

		//
		//      new floor thinker
		//
		rtn = 1;
		floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
		memset(floor, 0, sizeof(*floor));
		P_AddThinker (&floor->thinker);
		sec->specialdata = floor;
		floor->thinker.function = T_MoveFloor;
		floor->type = floortype;
		floor->crush = 0;
		floor->speed = args[1]*(FRACUNIT/8);
		if(floortype == FLEV_LOWERTIMES8INSTANT || 
			floortype == FLEV_RAISETIMES8INSTANT)
		{
			floor->speed = 2000<<FRACBITS;
		}
		switch(floortype)
		{
			case FLEV_LOWERFLOOR:
				floor->direction = -1;
				floor->sector = sec;
				floor->floordestheight =
					P_FindHighestFloorSurrounding(sec);
				break;
			case FLEV_LOWERFLOORTOLOWEST:
				floor->direction = -1;
				floor->sector = sec;
				floor->floordestheight =
					P_FindLowestFloorSurrounding(sec);
				break;
			case FLEV_LOWERFLOORBYVALUE:
				floor->direction = -1;
				floor->sector = sec;
				floor->floordestheight = floor->sector->floorheight-
					args[2]*FRACUNIT;
				break;
			case FLEV_LOWERTIMES8INSTANT:
			case FLEV_LOWERBYVALUETIMES8:
				floor->direction = -1;
				floor->sector = sec;
				floor->floordestheight = floor->sector->floorheight-
					args[2]*FRACUNIT*8;
				break;
			case FLEV_RAISEFLOORCRUSH:
				floor->crush = args[2]; // arg[2] = crushing value
				floor->direction = 1;
				floor->sector = sec;
				floor->floordestheight = sec->ceilingheight-8*FRACUNIT;
				break;
			case FLEV_RAISEFLOOR:
				floor->direction = 1;
				floor->sector = sec;
				floor->floordestheight =
					P_FindLowestCeilingSurrounding(sec);
				if (floor->floordestheight > sec->ceilingheight)
					floor->floordestheight = sec->ceilingheight;
				break;
			case FLEV_RAISEFLOORTONEAREST:
				floor->direction = 1;
				floor->sector = sec;
				floor->floordestheight =
					P_FindNextHighestFloor(sec,sec->floorheight);
				break;
			case FLEV_RAISEFLOORBYVALUE:
				floor->direction = 1;
				floor->sector = sec;
				floor->floordestheight = floor->sector->floorheight+
					args[2]*FRACUNIT;
				break;
			case FLEV_RAISETIMES8INSTANT:
			case FLEV_RAISEBYVALUETIMES8:
				floor->direction = 1;
				floor->sector = sec;
				floor->floordestheight = floor->sector->floorheight+
					args[2]*FRACUNIT*8;
				break;
			case FLEV_MOVETOVALUETIMES8:
				floor->sector = sec;
				floor->floordestheight = args[2]*FRACUNIT*8;
				if(args[3])
				{
					floor->floordestheight = -floor->floordestheight;
				}
				if(floor->floordestheight > floor->sector->floorheight)
				{
					floor->direction = 1;
				}
				else if(floor->floordestheight < floor->sector->floorheight)
				{
					floor->direction = -1;
				}
				else
				{ // already at lowest position
					rtn = 0;
				}
				break;
			default:
				rtn = 0;
				break;
		}
	}
	if(rtn)
	{
		SN_StartSequence((mobj_t *)&floor->sector->soundorg, 
			SEQ_PLATFORM+floor->sector->seqType);
	}
	return rtn;
}

//============================================================================
//
// EV_DoFloorAndCeiling
//
//============================================================================

int EV_DoFloorAndCeiling(line_t *line, byte *args, boolean raise)
{
	boolean floor, ceiling;
	int                     secnum;
	sector_t        *sec;

	if(raise)
	{
		floor = EV_DoFloor(line, args, FLEV_RAISEFLOORBYVALUE);
		secnum = -1;
		while((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
		{
			sec = &sectors[secnum];
			sec->specialdata = NULL;
		}
		ceiling = EV_DoCeiling(line, args, CLEV_RAISEBYVALUE);
	}		
	else 
	{
		floor = EV_DoFloor(line, args, FLEV_LOWERFLOORBYVALUE);
		secnum = -1;
		while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
		{
			sec = &sectors[secnum];
			sec->specialdata = NULL;
		}
		ceiling = EV_DoCeiling(line, args, CLEV_LOWERBYVALUE);
	}
	return (floor|ceiling);
}
			
// ===== Build Stairs Private Data =====

#define STAIR_SECTOR_TYPE       26
#define STAIR_QUEUE_SIZE        32

struct
{
	sector_t *sector;
	int type;
	int height;
} StairQueue[STAIR_QUEUE_SIZE];

static int QueueHead;
static int QueueTail;

static int StepDelta;
static int Direction;
static int Speed;
static int Texture;
static int StartDelay;
static int StartDelayDelta;
static int TextureChange;
static int StartHeight;

//==========================================================================
//
// QueueStairSector
//
//==========================================================================

static void QueueStairSector(sector_t *sec, int type, int height)
{
	if((QueueTail+1)%STAIR_QUEUE_SIZE == QueueHead)
	{
		I_Error("BuildStairs:  Too many branches located.\n");
	}
	StairQueue[QueueTail].sector = sec;
	StairQueue[QueueTail].type = type;
	StairQueue[QueueTail].height = height;

	QueueTail = (QueueTail+1)%STAIR_QUEUE_SIZE;
}

//==========================================================================
//
// DequeueStairSector
//
//==========================================================================

static sector_t *DequeueStairSector(int *type, int *height)
{
	sector_t *sec;

	if(QueueHead == QueueTail)
	{ // queue is empty
		return NULL;
	}
	*type = StairQueue[QueueHead].type;
	*height = StairQueue[QueueHead].height;
	sec = StairQueue[QueueHead].sector;
	QueueHead = (QueueHead+1)%STAIR_QUEUE_SIZE;

	return sec;
}

//==========================================================================
//
// ProcessStairSector
//
//==========================================================================

static void ProcessStairSector(sector_t *sec, int type, int height, 
	stairs_e stairsType, int delay, int resetDelay)
{
	int             i;
	sector_t        *tsec;
	floormove_t     *floor;

	//
	// new floor thinker
	//
	height += StepDelta;
	floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
	memset(floor, 0, sizeof(*floor));
	P_AddThinker(&floor->thinker);
	sec->specialdata = floor;
	floor->thinker.function = T_MoveFloor;
	floor->type = FLEV_RAISEBUILDSTEP;
	floor->direction = Direction;
	floor->sector = sec;
	floor->floordestheight = height;
	switch(stairsType)
	{
		case STAIRS_NORMAL:
			floor->speed = Speed;
			if(delay)
			{
				floor->delayTotal = delay;
				floor->stairsDelayHeight = sec->floorheight+StepDelta;
				floor->stairsDelayHeightDelta = StepDelta;
			}
			floor->resetDelay = resetDelay;
			floor->resetDelayCount = resetDelay;
			floor->resetHeight = sec->floorheight;
			break;
		case STAIRS_SYNC:
			floor->speed = FixedMul(Speed, FixedDiv(height-StartHeight,
				StepDelta));
			floor->resetDelay = delay; //arg4
			floor->resetDelayCount = delay;
			floor->resetHeight = sec->floorheight;
			break;
/*
		case STAIRS_PHASED:
			floor->floordestheight = sec->floorheight+StepDelta;
			floor->speed = Speed;
			floor->delayCount = StartDelay;
			StartDelay += StartDelayDelta;
			floor->textureChange = TextureChange;
			floor->resetDelayCount = StartDelay;
			break;
*/
		default:
			break;
	}
	SN_StartSequence((mobj_t *)&sec->soundorg, SEQ_PLATFORM+sec->seqType);
	//
	// Find next sector to raise
	// Find nearby sector with sector special equal to type
	//
	for (i = 0; i < sec->linecount; i++)
	{
		if(!((sec->lines[i])->flags&ML_TWOSIDED))
		{
			continue;
		}
		tsec = (sec->lines[i])->frontsector;
		if(tsec->special == type+STAIR_SECTOR_TYPE && !tsec->specialdata
			&& tsec->floorpic == Texture && tsec->validcount != validcount)
		{
			QueueStairSector(tsec, type^1, height);
			tsec->validcount = validcount;
			//tsec->special = 0;
		}
		tsec = (sec->lines[i])->backsector;
		if(tsec->special == type+STAIR_SECTOR_TYPE && !tsec->specialdata
			&& tsec->floorpic == Texture && tsec->validcount != validcount)
		{
			QueueStairSector(tsec, type^1, height);
			tsec->validcount = validcount;
			//tsec->special = 0;
		}
	}
}

//==================================================================
//
//      BUILD A STAIRCASE!
//
// Direction is either positive or negative, denoting build stairs
//      up or down.
//==================================================================

int EV_BuildStairs(line_t *line, byte *args, int direction, 
	stairs_e stairsType)
{
	int secnum;
	int height;
	int	delay;
	int resetDelay;
	sector_t        *sec;
	sector_t *qSec;
	int type;

	// Set global stairs variables
	TextureChange = 0;
	Direction = direction;
	StepDelta = Direction*(args[2]*FRACUNIT);
	Speed = args[1]*(FRACUNIT/8);
	resetDelay = args[4];
	delay = args[3];
	if(stairsType == STAIRS_PHASED)
	{
		StartDelayDelta = args[3];
		StartDelay = StartDelayDelta;
		resetDelay = StartDelayDelta;
		delay = 0;
		TextureChange = args[4];
	}

	secnum = -1;

	validcount++; 
	while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
	{
		sec = &sectors[secnum];

		Texture = sec->floorpic;
		StartHeight = sec->floorheight;

		// ALREADY MOVING?  IF SO, KEEP GOING...
		if (sec->specialdata)
			continue;

		QueueStairSector(sec, 0, sec->floorheight);
		sec->special = 0;
	}
	while((qSec = DequeueStairSector(&type, &height)) != NULL)
	{
		ProcessStairSector(qSec, type, height, stairsType, delay, resetDelay);
	}
	return(1);
}

//=========================================================================
//
// T_BuildPillar
//
//=========================================================================

void T_BuildPillar(pillar_t *pillar)
{
	result_e        res1;
	result_e res2;

	// First, raise the floor
	res1 = T_MovePlane(pillar->sector, pillar->floorSpeed, pillar->floordest,
		pillar->crush, 0, pillar->direction); // floorOrCeiling, direction
	// Then, lower the ceiling
	res2 = T_MovePlane(pillar->sector, pillar->ceilingSpeed,
		 pillar->ceilingdest, pillar->crush, 1, -pillar->direction);
	if (res1 == RES_PASTDEST && res2 == RES_PASTDEST)
	{
		pillar->sector->specialdata = NULL;
		SN_StopSequence((mobj_t *)&pillar->sector->soundorg);
		P_TagFinished(pillar->sector->tag);
		P_RemoveThinker(&pillar->thinker);
	}
}

//=========================================================================
//
// EV_BuildPillar
//
//=========================================================================

int EV_BuildPillar(line_t *line, byte *args, boolean crush)
{
	int secnum;
	sector_t *sec;
	pillar_t *pillar;
	int newHeight;
	int rtn;

	rtn = 0;
	secnum = -1;
	while((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if(sec->specialdata)
			continue; // already moving
		if(sec->floorheight == sec->ceilingheight)
		{ // pillar is already closed
			continue;
		}
		rtn = 1;
		if(!args[2])
		{
			newHeight = sec->floorheight+
				((sec->ceilingheight-sec->floorheight)/2);
		}
		else
		{
			newHeight = sec->floorheight+(args[2]<<FRACBITS);
		}

		pillar = Z_Malloc(sizeof(*pillar), PU_LEVSPEC, 0);
		sec->specialdata = pillar;
		P_AddThinker(&pillar->thinker);
		pillar->thinker.function = T_BuildPillar;
		pillar->sector = sec;
		if(!args[2])
		{
			pillar->ceilingSpeed = pillar->floorSpeed = args[1]*(FRACUNIT/8);
		}
		else if(newHeight-sec->floorheight > sec->ceilingheight-newHeight)
		{
			pillar->floorSpeed = args[1]*(FRACUNIT/8);
			pillar->ceilingSpeed = FixedMul(sec->ceilingheight-newHeight,
				FixedDiv(pillar->floorSpeed, newHeight-sec->floorheight));
		}
		else
		{
			pillar->ceilingSpeed = args[1]*(FRACUNIT/8);
			pillar->floorSpeed = FixedMul(newHeight-sec->floorheight,
				FixedDiv(pillar->ceilingSpeed, sec->ceilingheight-newHeight));
		}
		pillar->floordest = newHeight;
		pillar->ceilingdest = newHeight;
		pillar->direction = 1;
		pillar->crush = crush*args[3];
		SN_StartSequence((mobj_t *)&pillar->sector->soundorg, 
			SEQ_PLATFORM+pillar->sector->seqType);
	}
	return rtn;
}

//=========================================================================
//
// EV_OpenPillar
//
//=========================================================================

int EV_OpenPillar(line_t *line, byte *args)
{
	int secnum;
	sector_t *sec;
	pillar_t *pillar;
	int rtn;

	rtn = 0;
	secnum = -1;
	while((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if(sec->specialdata)
			continue; // already moving
		if(sec->floorheight != sec->ceilingheight)
		{ // pillar isn't closed
			continue;
		}
		rtn = 1;
		pillar = Z_Malloc(sizeof(*pillar), PU_LEVSPEC, 0);
		sec->specialdata = pillar;
		P_AddThinker(&pillar->thinker);
		pillar->thinker.function = T_BuildPillar;
		pillar->sector = sec;
		if(!args[2])
		{
			pillar->floordest = P_FindLowestFloorSurrounding(sec);
		}
		else
		{
			pillar->floordest = sec->floorheight-(args[2]<<FRACBITS);
		}
		if(!args[3])
		{
			pillar->ceilingdest = P_FindHighestCeilingSurrounding(sec);
		}
		else
		{
			pillar->ceilingdest = sec->ceilingheight+(args[3]<<FRACBITS);
		}
		if(sec->floorheight-pillar->floordest >= pillar->ceilingdest-
			sec->ceilingheight)
		{
			pillar->floorSpeed = args[1]*(FRACUNIT/8);
			pillar->ceilingSpeed = FixedMul(sec->ceilingheight-
				pillar->ceilingdest, FixedDiv(pillar->floorSpeed,
				pillar->floordest-sec->floorheight));
		}
		else
		{
			pillar->ceilingSpeed = args[1]*(FRACUNIT/8);
			pillar->floorSpeed = FixedMul(pillar->floordest-sec->floorheight,
				FixedDiv(pillar->ceilingSpeed, sec->ceilingheight-
				pillar->ceilingdest));
		}
		pillar->direction = -1; // open the pillar
		SN_StartSequence((mobj_t *)&pillar->sector->soundorg, 
			SEQ_PLATFORM+pillar->sector->seqType);
	}
	return rtn;
}

//=========================================================================
//
// EV_FloorCrushStop
//
//=========================================================================

int EV_FloorCrushStop(line_t *line, byte *args)
{
	thinker_t *think;
	floormove_t *floor;
	boolean rtn;

	rtn = 0;
	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != T_MoveFloor)
		{
			continue;
		}
		floor = (floormove_t *)think;
		if(floor->type != FLEV_RAISEFLOORCRUSH)
		{
			continue;
		}
		// Completely remove the crushing floor
		SN_StopSequence((mobj_t *)&floor->sector->soundorg);
		floor->sector->specialdata = NULL;
		P_TagFinished(floor->sector->tag);
		P_RemoveThinker(&floor->thinker);
		rtn = 1;
	}	
	return rtn;
}

//==========================================================================
//
// T_FloorWaggle
//
//==========================================================================

#define WGLSTATE_EXPAND 1
#define WGLSTATE_STABLE 2
#define WGLSTATE_REDUCE 3

void T_FloorWaggle(floorWaggle_t *waggle)
{
	switch(waggle->state)
	{
		case WGLSTATE_EXPAND:
			if((waggle->scale += waggle->scaleDelta)
				>= waggle->targetScale)
			{
				waggle->scale = waggle->targetScale;
				waggle->state = WGLSTATE_STABLE;
			}
			break;
		case WGLSTATE_REDUCE:
			if((waggle->scale -= waggle->scaleDelta) <= 0)
			{ // Remove
				waggle->sector->floorheight = waggle->originalHeight;
				P_ChangeSector(waggle->sector, true);
				waggle->sector->specialdata = NULL;
				P_TagFinished(waggle->sector->tag);
				P_RemoveThinker(&waggle->thinker);
				return;
			}
			break;
		case WGLSTATE_STABLE:
			if(waggle->ticker != -1)
			{
				if(!--waggle->ticker)
				{
					waggle->state = WGLSTATE_REDUCE;
				}
			}
			break;
	}
	waggle->accumulator += waggle->accDelta;
	waggle->sector->floorheight = waggle->originalHeight
		+FixedMul(FloatBobOffsets[(waggle->accumulator>>FRACBITS)&63],
		waggle->scale);
	P_ChangeSector(waggle->sector, true);
}

//==========================================================================
//
// EV_StartFloorWaggle
//
//==========================================================================

boolean EV_StartFloorWaggle(int tag, int height, int speed, int offset,
	int timer)
{
	int sectorIndex;
	sector_t *sector;
	floorWaggle_t *waggle;
	boolean retCode;

	retCode = false;
	sectorIndex = -1;
	while((sectorIndex = P_FindSectorFromTag(tag, sectorIndex)) >= 0)
	{
		sector = &sectors[sectorIndex];
		if(sector->specialdata)
		{ // Already busy with another thinker
			continue;
		}
		retCode = true;
		waggle = Z_Malloc(sizeof(*waggle), PU_LEVSPEC, 0);
		sector->specialdata = waggle;
		waggle->thinker.function = T_FloorWaggle;
		waggle->sector = sector;
		waggle->originalHeight = sector->floorheight;
		waggle->accumulator = offset*FRACUNIT;
		waggle->accDelta = speed<<10;
		waggle->scale = 0;
		waggle->targetScale = height<<10;
		waggle->scaleDelta = waggle->targetScale
			/(35+((3*35)*height)/255);
		waggle->ticker = timer ? timer*35 : -1;
		waggle->state = WGLSTATE_EXPAND;
		P_AddThinker(&waggle->thinker);
	}
	return retCode;
}
