
//**************************************************************************
//**
//** p_plats.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_plats.c,v $
//** $Revision: 1.11 $
//** $Date: 95/09/11 22:06:30 $
//** $Author: cjr $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

plat_t  *activeplats[MAXPLATS];

//==================================================================
//
//      Move a plat up and down
//
//==================================================================
void T_PlatRaise(plat_t *plat)
{
	result_e res;

	switch(plat->status)
	{
		case PLAT_UP:
			res = T_MovePlane(plat->sector, plat->speed,
					plat->high, plat->crush, 0, 1);
			if (res == RES_CRUSHED && (!plat->crush))
			{
				plat->count = plat->wait;
				plat->status = PLAT_DOWN;
				SN_StartSequence((mobj_t *)&plat->sector->soundorg, 
					SEQ_PLATFORM+plat->sector->seqType);
			}
			else
			if (res == RES_PASTDEST)
			{
				plat->count = plat->wait;
				plat->status = PLAT_WAITING;
				SN_StopSequence((mobj_t *)&plat->sector->soundorg);
				switch(plat->type)
				{
					case PLAT_DOWNWAITUPSTAY:
					case PLAT_DOWNBYVALUEWAITUPSTAY:
						P_RemoveActivePlat(plat);
						break;
					default:
						break;
				}
			}
			break;
		case PLAT_DOWN:
			res = T_MovePlane(plat->sector,plat->speed,plat->low,false,0,-1);
			if (res == RES_PASTDEST)
			{
				plat->count = plat->wait;
				plat->status = PLAT_WAITING;
				switch(plat->type)
				{
					case PLAT_UPWAITDOWNSTAY:
					case PLAT_UPBYVALUEWAITDOWNSTAY:
						P_RemoveActivePlat(plat);
						break;
					default:
						break;
				}
				SN_StopSequence((mobj_t *)&plat->sector->soundorg);
			}
			break;
		case PLAT_WAITING:
			if (!--plat->count)
			{
				if (plat->sector->floorheight == plat->low)
					plat->status = PLAT_UP;
				else
					plat->status = PLAT_DOWN;
				SN_StartSequence((mobj_t *)&plat->sector->soundorg, 
					SEQ_PLATFORM+plat->sector->seqType);
			}
//		case PLAT_IN_STASIS:
//			break;
	}
}

//==================================================================
//
//      Do Platforms
//      "amount" is only used for SOME platforms.
//
//==================================================================
int EV_DoPlat(line_t *line, byte *args, plattype_e type, int amount)
{
	plat_t          *plat;
	int                     secnum;
	int                     rtn;
	sector_t        *sec;

	secnum = -1;
	rtn = 0;

/*
	//
	//      Activate all <type> plats that are in_stasis
	//
	switch(type)
	{
		case PLAT_PERPETUALRAISE:
			P_ActivateInStasis(args[0]);
			break;
		default:
			break;
	}
*/

	while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		//
		// Find lowest & highest floors around sector
		//
		rtn = 1;
		plat = Z_Malloc( sizeof(*plat), PU_LEVSPEC, 0);
		P_AddThinker(&plat->thinker);

		plat->type = type;
		plat->sector = sec;
		plat->sector->specialdata = plat;
		plat->thinker.function = T_PlatRaise;
		plat->crush = false;
		plat->tag = args[0];
		plat->speed = args[1]*(FRACUNIT/8);
		switch(type)
		{
			case PLAT_DOWNWAITUPSTAY:
				plat->low = P_FindLowestFloorSurrounding(sec)+8*FRACUNIT;
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = sec->floorheight;
				plat->wait = args[2];
				plat->status = PLAT_DOWN;
				break;
			case PLAT_DOWNBYVALUEWAITUPSTAY:
				plat->low = sec->floorheight-args[3]*8*FRACUNIT;
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = sec->floorheight;
				plat->wait = args[2];
				plat->status = PLAT_DOWN;
				break;
			case PLAT_UPWAITDOWNSTAY:
				plat->high = P_FindHighestFloorSurrounding(sec);
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->low = sec->floorheight;
				plat->wait = args[2];
				plat->status = PLAT_UP;
				break;
			case PLAT_UPBYVALUEWAITDOWNSTAY:
				plat->high = sec->floorheight+args[3]*8*FRACUNIT;
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->low = sec->floorheight;
				plat->wait = args[2];
				plat->status = PLAT_UP;
				break;
			case PLAT_PERPETUALRAISE:
				plat->low = P_FindLowestFloorSurrounding(sec)+8*FRACUNIT;
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->wait = args[2];
				plat->status = P_Random()&1;
				break;
		}
		P_AddActivePlat(plat);
		SN_StartSequence((mobj_t *)&sec->soundorg, SEQ_PLATFORM+sec->seqType);
	}
	return rtn;
}

#if 0
void P_ActivateInStasis(int tag)
{
	int             i;

	for (i = 0;i < MAXPLATS;i++)
		if (activeplats[i] &&
			(activeplats[i])->tag == tag &&
			(activeplats[i])->status == PLAT_IN_STASIS)
		{
			(activeplats[i])->status = (activeplats[i])->oldstatus;
			(activeplats[i])->thinker.function = T_PlatRaise;
		}
}
#endif

void EV_StopPlat(line_t *line, byte *args)
{
	int i;

	for(i = 0; i < MAXPLATS; i++)
	{
		if((activeplats[i])->tag = args[0])
		{
			(activeplats[i])->sector->specialdata = NULL;
			P_TagFinished((activeplats[i])->sector->tag);
			P_RemoveThinker(&(activeplats[i])->thinker);
			activeplats[i] = NULL;

			return;
		}
	}

/*
	int             j;

	for (j = 0;j < MAXPLATS;j++)
	{
		if (activeplats[j] && ((activeplats[j])->status != PLAT_IN_STASIS) &&
			((activeplats[j])->tag == args[0]))
		{
			(activeplats[j])->oldstatus = (activeplats[j])->status;
			(activeplats[j])->status = PLAT_IN_STASIS;
			(activeplats[j])->thinker.function = NULL;
			SN_StopSequence((mobj_t *)&(activeplats[j])->sector->soundorg);
		}
	}
*/
}

void P_AddActivePlat(plat_t *plat)
{
	int             i;
	for (i = 0;i < MAXPLATS;i++)
		if (activeplats[i] == NULL)
		{
			activeplats[i] = plat;
			return;
		}
	I_Error ("P_AddActivePlat: no more plats!");
}

void P_RemoveActivePlat(plat_t *plat)
{
	int             i;
	for (i = 0;i < MAXPLATS;i++)
		if (plat == activeplats[i])
		{
			(activeplats[i])->sector->specialdata = NULL;
			P_TagFinished(plat->sector->tag);
			P_RemoveThinker(&(activeplats[i])->thinker);
			activeplats[i] = NULL;
			return;
		}
	I_Error ("P_RemoveActivePlat: can't find plat!");
}
