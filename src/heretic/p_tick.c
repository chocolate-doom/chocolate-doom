
// P_tick.c

#include "DoomDef.h"
#include "P_local.h"

int leveltime;
int TimerGame;

/*
====================
=
= P_ArchivePlayers
=
====================
*/

void P_ArchivePlayers(void)
{
	int i;
	int j;
	player_t dest;

	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(!playeringame[i])
		{
			continue;
		}
		memcpy(&dest, &players[i], sizeof(player_t));
		for(j = 0; j < NUMPSPRITES; j++)
		{
			if(dest.psprites[j].state)
			{
				dest.psprites[j].state =
					(state_t *)(dest.psprites[j].state-states);
			}
		}
		SV_Write(&dest, sizeof(player_t));
	}
}

/*
====================
=
= P_UnArchivePlayers
=
====================
*/

void P_UnArchivePlayers (void)
{
	int		i,j;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;
		memcpy (&players[i],save_p, sizeof(player_t));
		save_p += sizeof(player_t);
		players[i].mo = NULL;		// will be set when unarc thinker
		players[i].message = NULL;
		players[i].attacker = NULL;
		for (j=0 ; j<NUMPSPRITES ; j++)
			if (players[i]. psprites[j].state)
				players[i]. psprites[j].state 
				= &states[ (int)players[i].psprites[j].state ];
	}
}

//=============================================================================


/*
====================
=
= P_ArchiveWorld
=
====================
*/

void P_ArchiveWorld(void)
{
	int i, j;
	sector_t *sec;
	line_t *li;
	side_t *si;

	// Sectors
	for(i = 0, sec = sectors; i < numsectors; i++, sec++)
	{
		SV_WriteWord(sec->floorheight>>FRACBITS);
		SV_WriteWord(sec->ceilingheight>>FRACBITS);
		SV_WriteWord(sec->floorpic);
		SV_WriteWord(sec->ceilingpic);
		SV_WriteWord(sec->lightlevel);
		SV_WriteWord(sec->special); // needed?
		SV_WriteWord(sec->tag); // needed?
	}

	// Lines
	for(i = 0, li = lines; i < numlines; i++, li++)
	{
		SV_WriteWord(li->flags);
		SV_WriteWord(li->special);
		SV_WriteWord(li->tag);
		for(j = 0; j < 2; j++)
		{
			if(li->sidenum[j] == -1)
			{
				continue;
			}
			si = &sides[li->sidenum[j]];
			SV_WriteWord(si->textureoffset>>FRACBITS);
			SV_WriteWord(si->rowoffset>>FRACBITS);
			SV_WriteWord(si->toptexture);
			SV_WriteWord(si->bottomtexture);
			SV_WriteWord(si->midtexture);
		}
	}
}

/*
====================
=
= P_UnArchiveWorld
=
====================
*/

void P_UnArchiveWorld (void)
{
	int			i,j;
	sector_t	*sec;
	line_t		*li;
	side_t		*si;
	short		*get;
	
	get = (short *)save_p;
		
//
// do sectors
//
	for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
	{
		sec->floorheight = *get++ << FRACBITS;
		sec->ceilingheight = *get++ << FRACBITS;
		sec->floorpic = *get++;
		sec->ceilingpic = *get++;
		sec->lightlevel = *get++;
		sec->special = *get++;	// needed?
		sec->tag = *get++;		// needed?
		sec->specialdata = 0;
		sec->soundtarget = 0;
	}	

//
// do lines
//
	for (i=0, li = lines ; i<numlines ; i++,li++)
	{
		li->flags = *get++;
		li->special = *get++;
		li->tag = *get++;
		for (j=0 ; j<2 ; j++)
		{
			if (li->sidenum[j] == -1)
				continue;
			si = &sides[li->sidenum[j]];
			si->textureoffset = *get++ << FRACBITS;
			si->rowoffset = *get++ << FRACBITS;
			si->toptexture = *get++;
			si->bottomtexture = *get++;
			si->midtexture = *get++;
		}
	}
		
	save_p = (byte *)get;	
}

//=============================================================================

typedef enum
{
	tc_end,
	tc_mobj
} thinkerclass_t;

/*
====================
=
= P_ArchiveThinkers
=
====================
*/

void P_ArchiveThinkers(void)
{
	thinker_t *th;
	mobj_t mobj;

	for(th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if(th->function == P_MobjThinker)
		{
			SV_WriteByte(tc_mobj);
			memcpy(&mobj, th, sizeof(mobj_t));
			mobj.state = (state_t *)(mobj.state-states);
			if(mobj.player)
			{
				mobj.player = (player_t *)((mobj.player-players)+1);
			}
			SV_Write(&mobj, sizeof(mobj_t));
			continue;
		}
		//I_Error("P_ArchiveThinkers: Unknown thinker function");
	}

	// Add a terminating marker
	SV_WriteByte(tc_end);
}

/*
====================
=
= P_UnArchiveThinkers
=
====================
*/

void P_UnArchiveThinkers (void)
{
	byte		tclass;
	thinker_t	*currentthinker, *next;
	mobj_t		*mobj;
	
//
// remove all the current thinkers
//
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		next = currentthinker->next;
		if (currentthinker->function == P_MobjThinker)
			P_RemoveMobj ((mobj_t *)currentthinker);
		else
			Z_Free (currentthinker);
		currentthinker = next;
	}
	P_InitThinkers ();
	
// read in saved thinkers
	while (1)
	{
		tclass = *save_p++;
		switch (tclass)
		{
		case tc_end:
			return;			// end of list
			
		case tc_mobj:
			mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
			memcpy (mobj, save_p, sizeof(*mobj));
			save_p += sizeof(*mobj);
			mobj->state = &states[(int)mobj->state];
			mobj->target = NULL;
			if (mobj->player)
			{
				mobj->player = &players[(int)mobj->player-1];
				mobj->player->mo = mobj;
			}
			P_SetThingPosition (mobj);
			mobj->info = &mobjinfo[mobj->type];
			mobj->floorz = mobj->subsector->sector->floorheight;
			mobj->ceilingz = mobj->subsector->sector->ceilingheight;
			mobj->thinker.function = P_MobjThinker;
			P_AddThinker (&mobj->thinker);
			break;
			
		default:
			I_Error ("Unknown tclass %i in savegame",tclass);
		}
	
	}

}

//=============================================================================


/*
====================
=
= P_ArchiveSpecials
=
====================
*/
enum
{
	tc_ceiling,
	tc_door,
	tc_floor,
	tc_plat,
	tc_flash,
	tc_strobe,
	tc_glow,
	tc_endspecials
} specials_e;	

void P_ArchiveSpecials(void)
{
/*
T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
T_VerticalDoor, (vldoor_t: sector_t * swizzle),
T_MoveFloor, (floormove_t: sector_t * swizzle),
T_LightFlash, (lightflash_t: sector_t * swizzle),
T_StrobeFlash, (strobe_t: sector_t *),
T_Glow, (glow_t: sector_t *),
T_PlatRaise, (plat_t: sector_t *), - active list
*/

	thinker_t *th;
	ceiling_t ceiling;
	vldoor_t door;
	floormove_t floor;
	plat_t plat;
	lightflash_t flash;
	strobe_t strobe;
	glow_t glow;

	for(th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if(th->function == T_MoveCeiling)
		{
			SV_WriteByte(tc_ceiling);
			memcpy(&ceiling, th, sizeof(ceiling_t));
			ceiling.sector = (sector_t *)(ceiling.sector-sectors);
			SV_Write(&ceiling, sizeof(ceiling_t));
			continue;
		}
		if(th->function == T_VerticalDoor)
		{
			SV_WriteByte(tc_door);
			memcpy(&door, th, sizeof(vldoor_t));
			door.sector = (sector_t *)(door.sector-sectors);
			SV_Write(&door, sizeof(vldoor_t));
			continue;
		}
		if(th->function == T_MoveFloor)
		{
			SV_WriteByte(tc_floor);
			memcpy(&floor, th, sizeof(floormove_t));
			floor.sector = (sector_t *)(floor.sector-sectors);
			SV_Write(&floor, sizeof(floormove_t));
			continue;
		}
		if(th->function == T_PlatRaise)
		{
			SV_WriteByte(tc_plat);
			memcpy(&plat, th, sizeof(plat_t));
			plat.sector = (sector_t *)(plat.sector-sectors);
			SV_Write(&plat, sizeof(plat_t));
			continue;
		}
		if(th->function == T_LightFlash)
		{
			SV_WriteByte(tc_flash);
			memcpy(&flash, th, sizeof(lightflash_t));
			flash.sector = (sector_t *)(flash.sector-sectors);
			SV_Write(&flash, sizeof(lightflash_t));
			continue;
		}
		if(th->function == T_StrobeFlash)
		{
			SV_WriteByte(tc_strobe);
			memcpy(&strobe, th, sizeof(strobe_t));
			strobe.sector = (sector_t *)(strobe.sector-sectors);
			SV_Write(&strobe, sizeof(strobe_t));
			continue;
		}
		if(th->function == T_Glow)
		{
			SV_WriteByte(tc_glow);
			memcpy(&glow, th, sizeof(glow_t));
			glow.sector = (sector_t *)(glow.sector-sectors);
			SV_Write(&glow, sizeof(glow_t));
			continue;
		}
	}
	// Add a terminating marker
	SV_WriteByte(tc_endspecials);
}

/*
====================
=
= P_UnArchiveSpecials
=
====================
*/

void P_UnArchiveSpecials (void)
{
	byte		tclass;
	ceiling_t	*ceiling;
	vldoor_t	*door;
	floormove_t	*floor;
	plat_t		*plat;
	lightflash_t *flash;
	strobe_t	*strobe;
	glow_t		*glow;
	
	
// read in saved thinkers
	while (1)
	{
		tclass = *save_p++;
		switch (tclass)
		{
			case tc_endspecials:
				return;			// end of list
			
			case tc_ceiling:
				ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
				memcpy (ceiling, save_p, sizeof(*ceiling));
				save_p += sizeof(*ceiling);
				ceiling->sector = &sectors[(int)ceiling->sector];
				ceiling->sector->specialdata = T_MoveCeiling;
				if (ceiling->thinker.function)
					ceiling->thinker.function = T_MoveCeiling;
				P_AddThinker (&ceiling->thinker);
				P_AddActiveCeiling(ceiling);
				break;

			case tc_door:
				door = Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
				memcpy (door, save_p, sizeof(*door));
				save_p += sizeof(*door);
				door->sector = &sectors[(int)door->sector];
				door->sector->specialdata = door;
				door->thinker.function = T_VerticalDoor;
				P_AddThinker (&door->thinker);
				break;

			case tc_floor:
				floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);
				memcpy (floor, save_p, sizeof(*floor));
				save_p += sizeof(*floor);
				floor->sector = &sectors[(int)floor->sector];
				floor->sector->specialdata = T_MoveFloor;
				floor->thinker.function = T_MoveFloor;
				P_AddThinker (&floor->thinker);
				break;
				
			case tc_plat:
				plat = Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
				memcpy (plat, save_p, sizeof(*plat));
				save_p += sizeof(*plat);
				plat->sector = &sectors[(int)plat->sector];
				plat->sector->specialdata = T_PlatRaise;
				if (plat->thinker.function)
					plat->thinker.function = T_PlatRaise;
				P_AddThinker (&plat->thinker);
				P_AddActivePlat(plat);
				break;
				
			case tc_flash:
				flash = Z_Malloc (sizeof(*flash), PU_LEVEL, NULL);
				memcpy (flash, save_p, sizeof(*flash));
				save_p += sizeof(*flash);
				flash->sector = &sectors[(int)flash->sector];
				flash->thinker.function = T_LightFlash;
				P_AddThinker (&flash->thinker);
				break;
				
			case tc_strobe:
				strobe = Z_Malloc (sizeof(*strobe), PU_LEVEL, NULL);
				memcpy (strobe, save_p, sizeof(*strobe));
				save_p += sizeof(*strobe);
				strobe->sector = &sectors[(int)strobe->sector];
				strobe->thinker.function = T_StrobeFlash;
				P_AddThinker (&strobe->thinker);
				break;
				
			case tc_glow:
				glow = Z_Malloc (sizeof(*glow), PU_LEVEL, NULL);
				memcpy (glow, save_p, sizeof(*glow));
				save_p += sizeof(*glow);
				glow->sector = &sectors[(int)glow->sector];
				glow->thinker.function = T_Glow;
				P_AddThinker (&glow->thinker);
				break;
				
			default:
				I_Error ("P_UnarchiveSpecials:Unknown tclass %i "
							"in savegame",tclass);
		}
	
	}

}



/*
===============================================================================

								THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.  The actual
structures will vary in size, but the first element must be thinker_t.

===============================================================================
*/

thinker_t	thinkercap;	// both the head and tail of the thinker list

/*
===============
=
= P_InitThinkers
=
===============
*/

void P_InitThinkers (void)
{
	thinkercap.prev = thinkercap.next  = &thinkercap;
}


/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker (thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

/*
===============
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
===============
*/

void P_RemoveThinker (thinker_t *thinker)
{
	thinker->function = (think_t)-1;
}

/*
===============
=
= P_AllocateThinker
=
= Allocates memory and adds a new thinker at the end of the list
=
===============
*/

void P_AllocateThinker (thinker_t *thinker)
{
}


/*
===============
=
= P_RunThinkers
=
===============
*/

void P_RunThinkers (void)
{
	thinker_t	*currentthinker;

	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		if (currentthinker->function == (think_t)-1)
		{	// time to remove it
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			Z_Free (currentthinker);
		}
		else
		{
			if (currentthinker->function)
				currentthinker->function (currentthinker);
		}
		currentthinker = currentthinker->next;
	}
}

//----------------------------------------------------------------------------
//
// PROC P_Ticker
//
//----------------------------------------------------------------------------

void P_Ticker(void)
{
	int i;

	if(paused)
	{
		return;
	}
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(playeringame[i])
		{
			P_PlayerThink(&players[i]);
		}
	}
	if(TimerGame)
	{
		if(!--TimerGame)
		{
			G_ExitLevel();
		}
	}
	P_RunThinkers();
	P_UpdateSpecials();
	P_AmbientSound();
	leveltime++;
}
