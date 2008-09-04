
// P_doors.c

#include "DoomDef.h"
#include "P_local.h"
#include "soundst.h"

//==================================================================
//==================================================================
//
//							VERTICAL DOORS
//
//==================================================================
//==================================================================

//==================================================================
//
//	T_VerticalDoor
//
//==================================================================
void T_VerticalDoor(vldoor_t *door)
{
	result_e res;

	switch(door->direction)
	{
		case 0: // WAITING
			if(!--door->topcountdown)
				switch(door->type)
				{
					case normal:
						door->direction = -1; // time to go back down
						S_StartSound((mobj_t *)
							&door->sector->soundorg, sfx_doropn);
						break;
					case close30ThenOpen:
						door->direction = 1;
						S_StartSound((mobj_t *)
							&door->sector->soundorg, sfx_doropn);
						break;
					default:
						break;
				}
			break;
		case 2: // INITIAL WAIT
			if(!--door->topcountdown)
			{
				switch(door->type)
				{
					case raiseIn5Mins:
						door->direction = 1;
						door->type = normal;
						S_StartSound((mobj_t *)
							&door->sector->soundorg, sfx_doropn);
						break;
					default:
						break;
				}
			}
			break;
		case -1: // DOWN
			res = T_MovePlane(door->sector, door->speed,
				door->sector->floorheight, false, 1, door->direction);
			if(res == pastdest)
			{
				switch(door->type)
				{
					case normal:
					case close:
						door->sector->specialdata = NULL;
						P_RemoveThinker(&door->thinker);  // unlink and free
						S_StartSound((mobj_t *)
							&door->sector->soundorg, sfx_dorcls);
						break;
					case close30ThenOpen:
						door->direction = 0;
						door->topcountdown = 35*30;
						break;
					default:
						break;
				}
			}
			else if(res == crushed)
			{
				switch(door->type)
				{
					case close: // DON'T GO BACK UP!
						break;
					default:
						door->direction = 1;
						S_StartSound((mobj_t *)
							&door->sector->soundorg,sfx_doropn);
						break;
				}
			}
			break;
		case 1: // UP
			res = T_MovePlane(door->sector, door->speed,
				door->topheight, false, 1, door->direction);
			if(res == pastdest)
			{
				switch(door->type)
				{
					case normal:
						door->direction = 0; // wait at top
						door->topcountdown = door->topwait;
						break;
					case close30ThenOpen:
					case open:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker); // unlink and free
						S_StopSound((mobj_t *)&door->sector->soundorg);
						break;
					default:
						break;
				}
			}
			break;
	}
}

//----------------------------------------------------------------------------
//
// EV_DoDoor
//
// Move a door up/down
//
//----------------------------------------------------------------------------

int EV_DoDoor(line_t *line, vldoor_e type, fixed_t speed)
{
	int secnum;
	int retcode;
	sector_t *sec;
	vldoor_t *door;

	secnum = -1;
	retcode = 0;
	while((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if(sec->specialdata)
		{
			continue;
		}
		// Add new door thinker
		retcode = 1;
		door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker(&door->thinker);
		sec->specialdata = door;
		door->thinker.function = T_VerticalDoor;
		door->sector = sec;
		switch(type)
		{
			case close:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
				break;
			case close30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);
				break;
			case normal:
			case open:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				if(door->topheight != sec->ceilingheight)
				{
					S_StartSound((mobj_t *)&door->sector->soundorg,
						sfx_doropn);
				}
				break;
			default:
				break;
		}
		door->type = type;
		door->speed = speed;
		door->topwait = VDOORWAIT;
	}
	return(retcode);
}

//==================================================================
//
//	EV_VerticalDoor : open a door manually, no tag value
//
//==================================================================
void EV_VerticalDoor(line_t *line, mobj_t *thing)
{
	player_t		*player;
	int				secnum;
	sector_t		*sec;
	vldoor_t		*door;
	int				side;
	
	side = 0; // only front sides can be used
//
//	Check for locks
//
	player = thing->player;
	switch(line->special)
	{
		case 26: // Blue Lock
		case 32:
			if(!player)
			{
				return;
			}
			if(!player->keys[key_blue])
			{
				P_SetMessage(player, TXT_NEEDBLUEKEY, false);
				S_StartSound(NULL, sfx_plroof);
				return;
			}
			break;
		case 27: // Yellow Lock
		case 34:
			if(!player)
			{
				return;
			}
			if(!player->keys[key_yellow])
			{
				P_SetMessage(player, TXT_NEEDYELLOWKEY, false);
				S_StartSound(NULL, sfx_plroof);
				return;
			}
			break;
		case 28: // Green Lock
		case 33:
			if(!player)
			{
				return;
			}
			if(!player->keys[key_green])
			{
				P_SetMessage(player, TXT_NEEDGREENKEY, false);
				S_StartSound(NULL, sfx_plroof);
				return;
			}
			break;
	}

	// if the sector has an active thinker, use it
	sec = sides[line->sidenum[side^1]].sector;
	secnum = sec-sectors;
	if(sec->specialdata)
	{
		door = sec->specialdata;
		switch(line->special)
		{
			case 1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
			case 26:
			case 27:
			case 28:
				if(door->direction == -1)
				{
					door->direction = 1; // go back up
				}
				else
				{
					if(!thing->player)
					{ // Monsters don't close doors
						return;
					}
					door->direction = -1; // start going down immediately
				}
				return;
		}
	}

	// for proper sound
	switch(line->special)
	{
		case 1: // NORMAL DOOR SOUND
		case 31:
			S_StartSound((mobj_t *)&sec->soundorg, sfx_doropn);
			//S_StartSound((mobj_t *)&sec->soundorg, sfx_dormov);
			break;
		default: // LOCKED DOOR SOUND
			S_StartSound((mobj_t *)&sec->soundorg, sfx_doropn);
			//S_StartSound((mobj_t *)&sec->soundorg, sfx_dormov);
			break;
	}

	//
	// new door thinker
	//
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker(&door->thinker);
	sec->specialdata = door;
	door->thinker.function = T_VerticalDoor;
	door->sector = sec;
	door->direction = 1;
	switch(line->special)
	{
		case 1:
		case 26:
		case 27:
		case 28:
			door->type = normal;
			break;
		case 31:
		case 32:
		case 33:
		case 34:
			door->type = open;
			line->special = 0;
			break;
	}
	door->speed = VDOORSPEED;
	door->topwait = VDOORWAIT;
	
	//
	// find the top and bottom of the movement range
	//
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
}

//==================================================================
//
//	Spawn a door that closes after 30 seconds
//
//==================================================================
void P_SpawnDoorCloseIn30(sector_t *sec)
{
	vldoor_t *door;

	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker(&door->thinker);
	sec->specialdata = door;
	sec->special = 0;
	door->thinker.function = T_VerticalDoor;
	door->sector = sec;
	door->direction = 0;
	door->type = normal;
	door->speed = VDOORSPEED;
	door->topcountdown = 30*35;
}

//==================================================================
//
//	Spawn a door that opens after 5 minutes
//
//==================================================================
void P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum)
{
	vldoor_t *door;

	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker(&door->thinker);
	sec->specialdata = door;
	sec->special = 0;
	door->thinker.function = T_VerticalDoor;
	door->sector = sec;
	door->direction = 2;
	door->type = raiseIn5Mins;
	door->speed = VDOORSPEED;
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
	door->topwait = VDOORWAIT;
	door->topcountdown = 5*60*35;
}
