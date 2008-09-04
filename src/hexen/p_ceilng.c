
//**************************************************************************
//**
//** p_ceilng.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_ceilng.c,v $
//** $Revision: 1.17 $
//** $Date: 95/09/11 22:06:25 $
//** $Author: cjr $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

//==================================================================
//==================================================================
//
//                                                      CEILINGS
//
//==================================================================
//==================================================================

ceiling_t       *activeceilings[MAXCEILINGS];

//==================================================================
//
//      T_MoveCeiling
//
//==================================================================
void T_MoveCeiling (ceiling_t *ceiling)
{
	result_e        res;

	switch(ceiling->direction)
	{
//		case 0:         // IN STASIS
//			break;
		case 1:         // UP
			res = T_MovePlane(ceiling->sector,ceiling->speed,
					ceiling->topheight, false, 1, ceiling->direction);
			if (res == RES_PASTDEST)
			{
				SN_StopSequence((mobj_t *)&ceiling->sector->soundorg);
				switch(ceiling->type)
				{
					case CLEV_CRUSHANDRAISE:
						ceiling->direction = -1;
						ceiling->speed = ceiling->speed*2;
						break;
					default:
						P_RemoveActiveCeiling(ceiling);
						break;
				}
			}
			break;
		case -1:        // DOWN
			res = T_MovePlane(ceiling->sector,ceiling->speed,
				ceiling->bottomheight, ceiling->crush, 1, ceiling->direction);
			if(res == RES_PASTDEST)
			{
				SN_StopSequence((mobj_t *)&ceiling->sector->soundorg);
				switch(ceiling->type)
				{
					case CLEV_CRUSHANDRAISE:
					case CLEV_CRUSHRAISEANDSTAY:
						ceiling->direction = 1;
						ceiling->speed = ceiling->speed/2;
						break;
					default:
						P_RemoveActiveCeiling(ceiling);
						break;
				}
			} 
			else if(res == RES_CRUSHED)
			{
				switch(ceiling->type)
				{
					case CLEV_CRUSHANDRAISE:
					case CLEV_LOWERANDCRUSH:
					case CLEV_CRUSHRAISEANDSTAY:
						//ceiling->speed = ceiling->speed/4;
						break;
					default:
						break;
				}
			}
			break;
	}
}

//==================================================================
//
//              EV_DoCeiling
//              Move a ceiling up/down and all around!
//
//==================================================================
int EV_DoCeiling (line_t *line, byte *arg, ceiling_e type)
{
	int                     secnum,rtn;
	sector_t                *sec;
	ceiling_t               *ceiling;

	secnum = -1;
	rtn = 0;

/* Old Ceiling stasis code
	//
	//      Reactivate in-stasis ceilings...for certain types.
	//
	switch(type)
	{
		case CLEV_CRUSHANDRAISE:
			P_ActivateInStasisCeiling(line);
		default:
			break;
	}
*/
	while ((secnum = P_FindSectorFromTag(arg[0], secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		//
		// new door thinker
		//
		rtn = 1;
		ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVSPEC, 0);
		P_AddThinker (&ceiling->thinker);
		sec->specialdata = ceiling;
		ceiling->thinker.function = T_MoveCeiling;
		ceiling->sector = sec;
		ceiling->crush = 0;
		ceiling->speed = arg[1]*(FRACUNIT/8);
		switch(type)
		{
			case CLEV_CRUSHRAISEANDSTAY:
				ceiling->crush = arg[2]; // arg[2] = crushing value
				ceiling->topheight = sec->ceilingheight;
				ceiling->bottomheight = sec->floorheight + (8*FRACUNIT);
				ceiling->direction = -1;
				break;
			case CLEV_CRUSHANDRAISE:
				ceiling->topheight = sec->ceilingheight;
			case CLEV_LOWERANDCRUSH:
				ceiling->crush = arg[2]; // arg[2] = crushing value
			case CLEV_LOWERTOFLOOR:
				ceiling->bottomheight = sec->floorheight;
				if(type != CLEV_LOWERTOFLOOR)
				{
					ceiling->bottomheight += 8*FRACUNIT;
				}
				ceiling->direction = -1;
				break;
			case CLEV_RAISETOHIGHEST:
				ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
				ceiling->direction = 1;
				break;
			case CLEV_LOWERBYVALUE:
				ceiling->bottomheight = sec->ceilingheight-arg[2]*FRACUNIT;
				ceiling->direction = -1;
				break;
			case CLEV_RAISEBYVALUE:
				ceiling->topheight = sec->ceilingheight+arg[2]*FRACUNIT;
				ceiling->direction = 1;
				break;
			case CLEV_MOVETOVALUETIMES8:
			{
				int destHeight = arg[2]*FRACUNIT*8;

				if(arg[3])
				{
					destHeight = -destHeight;
				}
				if(sec->ceilingheight <= destHeight)
				{
					ceiling->direction = 1;
					ceiling->topheight = destHeight;
					if(sec->ceilingheight == destHeight)
					{
						rtn = 0;
					}
				}
				else if(sec->ceilingheight > destHeight)
				{
					ceiling->direction = -1;
					ceiling->bottomheight = destHeight;
				}
				break;
			}
			default:
				rtn = 0;
				break;
		}
		ceiling->tag = sec->tag;
		ceiling->type = type;
		P_AddActiveCeiling(ceiling);
		if(rtn)
		{
			SN_StartSequence((mobj_t *)&ceiling->sector->soundorg, 
				SEQ_PLATFORM+ceiling->sector->seqType);
		}
	}
	return rtn;
}

//==================================================================
//
//              Add an active ceiling
//
//==================================================================
void P_AddActiveCeiling(ceiling_t *c)
{
	int             i;
	for (i = 0; i < MAXCEILINGS;i++)
		if (activeceilings[i] == NULL)
		{
			activeceilings[i] = c;
			return;
		}
}

//==================================================================
//
//              Remove a ceiling's thinker
//
//==================================================================
void P_RemoveActiveCeiling(ceiling_t *c)
{
	int             i;

	for (i = 0;i < MAXCEILINGS;i++)
		if (activeceilings[i] == c)
		{
			activeceilings[i]->sector->specialdata = NULL;
			P_RemoveThinker (&activeceilings[i]->thinker);
			P_TagFinished(activeceilings[i]->sector->tag);
			activeceilings[i] = NULL;
			break;
		}
}

#if 0
//==================================================================
//
//              Restart a ceiling that's in-stasis
//
//==================================================================
void P_ActivateInStasisCeiling(line_t *line)
{
	int     i;

	for (i = 0;i < MAXCEILINGS;i++)
		if (activeceilings[i] && (activeceilings[i]->tag == line->arg1) &&
			(activeceilings[i]->direction == 0))
		{
			activeceilings[i]->direction = activeceilings[i]->olddirection;
			activeceilings[i]->thinker.function = T_MoveCeiling;
			SN_StartSequence((mobj_t *)&activeceilings[i]->sector->soundorg,
				SEQ_PLATFORM+activeceilings[i]->sector->seqType);
		}
}
#endif

//==================================================================
//
//              EV_CeilingCrushStop
//              Stop a ceiling from crushing!
//
//==================================================================

int EV_CeilingCrushStop(line_t *line, byte *args)
{
	int             i;
	int             rtn;

	rtn = 0;
	for (i = 0;i < MAXCEILINGS;i++)
	{
		if(activeceilings[i] && activeceilings[i]->tag == args[0])		
		{
			rtn = 1;
			SN_StopSequence((mobj_t*)&activeceilings[i]->sector->soundorg);
			activeceilings[i]->sector->specialdata = NULL;
			P_RemoveThinker (&activeceilings[i]->thinker);
			P_TagFinished(activeceilings[i]->sector->tag);
			activeceilings[i] = NULL;
			break;
		}
	}
	return rtn;
}
