//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Floor animation: raising stairs.
//



#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "doomstat.h"
#include "r_state.h"
// Data.
#include "sounds.h"


//
// FLOORS
//

//
// Move a plane (floor or ceiling) and check for crushing
//
// [STRIFE] Various changes were made to remove calls to P_ChangeSector when
// P_ChangeSector returns true.
//
result_e
T_MovePlane
( sector_t*     sector,
 fixed_t        speed,
 fixed_t        dest,
 boolean        crush,
 int            floorOrCeiling,
 int            direction )
{
    boolean     flag;
    fixed_t     lastpos;

    switch(floorOrCeiling)
    {
    case 0:
        // FLOOR
        switch(direction)
        {
        case -1:
            // DOWN
            if (sector->floorheight - speed < dest)
            {
                // villsa [STRIFE] unused
                //lastpos = sector->floorheight;
                sector->floorheight = dest;
                flag = P_ChangeSector(sector,crush);

                // villsa [STRIFE] unused
                /*if (flag == true)
                {
                    sector->floorheight =lastpos;
                    P_ChangeSector(sector,crush);
                    //return crushed;
                }*/
                return pastdest;
            }
            else
            {
                // villsa [STRIFE] unused
                //lastpos = sector->floorheight;
                sector->floorheight -= speed;
                flag = P_ChangeSector(sector,crush);

                // villsa [STRIFE] unused
                /*if (flag == true)
                {
                    sector->floorheight = lastpos;
                    P_ChangeSector(sector,crush);
                    return crushed;
                }*/
                return ok;
             }
             break;

        case 1:
            // UP
            if (sector->floorheight + speed > dest)
            {
                lastpos = sector->floorheight;
                sector->floorheight = dest;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    sector->floorheight = lastpos;
                    P_ChangeSector(sector,crush);
                    //return crushed;
                }
                return pastdest;
            }
            else
            {
                // COULD GET CRUSHED
                lastpos = sector->floorheight;
                sector->floorheight += speed;
                flag = P_ChangeSector(sector,crush);
                if (flag == true)
                {
                    // haleyjd 20130210: Bug fix - Strife DOES do this.
                    if (crush == true)
                        return crushed;
                    sector->floorheight = lastpos;
                    P_ChangeSector(sector,crush);
                    return crushed;
                }
                else
                    return ok;
            }
            break;
        }
        break;

    case 1:
        // CEILING
        switch(direction)
        {
        case -1:
            // DOWN
            if (sector->ceilingheight - speed < dest)
            {
                lastpos = sector->ceilingheight;
                sector->ceilingheight = dest;
                flag = P_ChangeSector(sector,crush);

                if (flag == true)
                {
                    sector->ceilingheight = lastpos;
                    P_ChangeSector(sector,crush);
                    //return crushed;
                }
                return pastdest;
            }
            else
            {
                // COULD GET CRUSHED
                lastpos = sector->ceilingheight;
                sector->ceilingheight -= speed;
                flag = P_ChangeSector(sector,crush);

                if (flag == true)
                {
                    if (crush == true)
                        return crushed;
                    sector->ceilingheight = lastpos;
                    P_ChangeSector(sector,crush);
                    return crushed;
                }
            }
            break;

        case 1:
            // UP
            if (sector->ceilingheight + speed > dest)
            {
                // villsa [STRIFE] unused
                //lastpos = sector->ceilingheight;
                sector->ceilingheight = dest;

                // villsa [STRIFE] unused
                //flag = P_ChangeSector(sector,crush);
                /*if (flag == true)
                {
                    sector->ceilingheight = lastpos;
                    P_ChangeSector(sector,crush);
                    //return crushed;
                }*/
                return pastdest;
            }
            else
            {
                // villsa [STRIFE] unused
                //lastpos = sector->ceilingheight;
                sector->ceilingheight += speed;

                // villsa [STRIFE] unused
                //flag = P_ChangeSector(sector,crush);
                return ok;
            }
            break;
        }
        break;

    }
    return ok;
}


//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void T_MoveFloor(floormove_t* floor)
{
    result_e	res;
	
    res = T_MovePlane(floor->sector,
		      floor->speed,
		      floor->floordestheight,
		      floor->crush,0,floor->direction);
    
    if (!(leveltime&7))
	S_StartSound(&floor->sector->soundorg, sfx_stnmov);
    
    if (res == pastdest)
    {
	floor->sector->specialdata = NULL;

	if (floor->direction == 1)
	{
	    switch(floor->type)
	    {
	      case donutRaise:
		floor->sector->special = floor->newspecial;
		floor->sector->floorpic = floor->texture;
	      default:
		break;
	    }
	}
	else if (floor->direction == -1)
	{
	    switch(floor->type)
	    {
	      case lowerAndChange:
		floor->sector->special = floor->newspecial;
		floor->sector->floorpic = floor->texture;
	      default:
		break;
	    }
	}
	P_RemoveThinker(&floor->thinker);

	S_StartSound(&floor->sector->soundorg, sfx_pstop);
    }

}

//
// HANDLE FLOOR TYPES
//
// haleyjd 09/16/2010: [STRIFE] Modifications to floortypes:
// * raiseFloor24 was changed into raiseFloor64
// * turboLower does not appear to adjust the floor height (STRIFE-TODO: verify)
// * raiseFloor512AndChange type was added.
//
int
EV_DoFloor
( line_t*       line,
  floor_e       floortype )
{
    int                 secnum;
    int                 rtn;
    int                 i;
    sector_t*           sec;
    floormove_t*        floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];

        // ALREADY MOVING?  IF SO, KEEP GOING...
        if (sec->specialdata)
            continue;

        // new floor thinker
        rtn = 1;
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);
        sec->specialdata = floor;
        floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        floor->type = floortype;
        floor->crush = false;

        switch(floortype)
        {
        case lowerFloor: // [STRIFE] verified unmodified
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
                P_FindHighestFloorSurrounding(sec);
            break;

        case lowerFloorToLowest: // [STRIFE] verified unmodified
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
                P_FindLowestFloorSurrounding(sec);
            break;

        case turboLower: // [STRIFE] Modified: does not += 8
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED * 4;
            floor->floordestheight = 
                P_FindHighestFloorSurrounding(sec);
            //if (floor->floordestheight != sec->floorheight)
            //    floor->floordestheight += 8*FRACUNIT;
            break;

        case raiseFloorCrush: // [STRIFE] verified unmodified
            floor->crush = true;
        case raiseFloor:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
                P_FindLowestCeilingSurrounding(sec);
            if (floor->floordestheight > sec->ceilingheight)
                floor->floordestheight = sec->ceilingheight;
            floor->floordestheight -= (8*FRACUNIT)*
                (floortype == raiseFloorCrush);
            break;

        case raiseFloorTurbo: // [STRIFE] verified unmodified
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED*4;
            floor->floordestheight = 
                P_FindNextHighestFloor(sec,sec->floorheight);
            break;

        case raiseFloorToNearest: // [STRIFE] verified unmodified
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
                P_FindNextHighestFloor(sec,sec->floorheight);
            break;

        case raiseFloor64: // [STRIFE] modified from raiseFloor24!
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 
                64 * FRACUNIT; // [STRIFE]
            break;

        case raiseFloor512: // [STRIFE] verified unmodified
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight +
                512 * FRACUNIT;
            break;

        case raiseFloor24AndChange: // [STRIFE] verified unmodified
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight +
                24 * FRACUNIT;
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            break;

        case raiseFloor512AndChange: // [STRIFE] New floor type
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight +
                512 * FRACUNIT;
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            break;

        case raiseToTexture: // [STRIFE] verified unmodified
            {
                int	minsize = INT_MAX;
                side_t*	side;

                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                for (i = 0; i < sec->linecount; i++)
                {
                    if (twoSided (secnum, i) )
                    {
                        side = getSide(secnum,i,0);
                        if (side->bottomtexture >= 0)
                            if (textureheight[side->bottomtexture] < 
                                minsize)
                                minsize = 
                                textureheight[side->bottomtexture];
                        side = getSide(secnum,i,1);
                        if (side->bottomtexture >= 0)
                            if (textureheight[side->bottomtexture] < 
                                minsize)
                                minsize = 
                                textureheight[side->bottomtexture];
                    }
                }
                floor->floordestheight =
                    floor->sector->floorheight + minsize;
            }
            break;

        case lowerAndChange: // [STRIFE] verified unmodified
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = 
                P_FindLowestFloorSurrounding(sec);
            floor->texture = sec->floorpic;

            for (i = 0; i < sec->linecount; i++)
            {
                if ( twoSided(secnum, i) )
                {
                    if (getSide(secnum,i,0)->sector-sectors == secnum)
                    {
                        sec = getSector(secnum,i,1);

                        if (sec->floorheight == floor->floordestheight)
                        {
                            floor->texture = sec->floorpic;
                            floor->newspecial = sec->special;
                            break;
                        }
                    }
                    else
                    {
                        sec = getSector(secnum,i,0);

                        if (sec->floorheight == floor->floordestheight)
                        {
                            floor->texture = sec->floorpic;
                            floor->newspecial = sec->special;
                            break;
                        }
                    }
                }
            }
        default:
            break;
        }
    }
    return rtn;
}




//
// BUILD A STAIRCASE!
//
int
EV_BuildStairs
( line_t*	line,
  stair_e	type )
{
    int                 secnum;
    int                 height;
    int                 i;
    int                 newsecnum;
    int                 texture;
    int                 ok;
    int                 rtn;

    sector_t*           sec;
    sector_t*           tsec;

    floormove_t*        floor;

    // Either Watcom or Rogue moved the switch out of the loop below, probably
    // because it was a loop invariant, and put the default values in the 
    // initializers here. I cannot be bothered to figure it out without doing
    // this myself :P
    fixed_t             stairsize = 8*FRACUNIT;
    fixed_t             speed     = FLOORSPEED;
    int                 direction = 1;

    switch(type)
    {
    case build8: // [STRIFE] Verified unmodified.
        speed = FLOORSPEED/4;
        break;
    case turbo16: // [STRIFE] Verified unmodified.
        speed = FLOORSPEED*4;
        stairsize = 16*FRACUNIT;
        break;
    case buildDown16: // [STRIFE] New stair type
        stairsize = -16*FRACUNIT;
        direction = -1;
        break;
    }

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];

        // ALREADY MOVING?  IF SO, KEEP GOING...
        if (sec->specialdata)
            continue;

        // new floor thinker
        rtn = 1;
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);
        sec->tag = 0; // haleyjd 20140919: [STRIFE] clears tag of first stair sector
        sec->specialdata = floor;
        floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        floor->direction = direction; // haleyjd 20140919: bug fix: direction, not "1"
        floor->sector = sec;
        floor->speed = speed;
        height = sec->floorheight + stairsize;
        floor->floordestheight = height;

        texture = sec->floorpic;

        // Find next sector to raise
        // 1.	Find 2-sided line with same sector side[0]
        // 2.	Other side is the next sector to raise
        do
        {
            ok = 0;
            for (i = 0;i < sec->linecount;i++)
            {
                if ( !((sec->lines[i])->flags & ML_TWOSIDED) )
                    continue;

                tsec = (sec->lines[i])->frontsector;
                newsecnum = tsec-sectors;

                if (secnum != newsecnum)
                    continue;

                tsec = (sec->lines[i])->backsector;
                newsecnum = tsec - sectors;

                if (tsec->floorpic != texture)
                    continue;

                height += stairsize;

                if (tsec->specialdata)
                    continue;

                sec = tsec;
                secnum = newsecnum;
                floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);

                P_AddThinker (&floor->thinker);

                sec->specialdata = floor;
                floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
                floor->direction = direction; // [STRIFE]: for buildDown16
                floor->sector = sec;
                floor->speed = speed;
                floor->floordestheight = height;
                ok = 1;
                break;
            }
        } while(ok);
    }
    return rtn;
}

