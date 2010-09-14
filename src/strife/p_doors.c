// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION: Door animation code (opening/closing)
//
//-----------------------------------------------------------------------------



#include "z_zone.h"
#include "doomdef.h"
#include "deh_main.h"
#include "p_local.h"

#include "s_sound.h"


// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "dstrings.h"
#include "sounds.h"


//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
void T_VerticalDoor (vldoor_t* door)
{
    result_e	res;
	
    switch(door->direction)
    {
      case 0:
	// WAITING
	if (!--door->topcountdown)
	{
	    switch(door->type)
	    {
	      case blazeRaise:
		door->direction = -1; // time to go back down
		S_StartSound(&door->sector->soundorg, sfx_bdcls);
		break;
		
	      case normal:
		door->direction = -1; // time to go back down
		S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
		break;
		
	      case close30ThenOpen:
		door->direction = 1;
		S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
		break;
		
	      default:
		break;
	    }
	}
	break;
	
      case 2:
	//  INITIAL WAIT
	if (!--door->topcountdown)
	{
	    switch(door->type)
	    {
	      case raiseIn5Mins:
		door->direction = 1;
		door->type = normal;
		S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
		break;
		
	      default:
		break;
	    }
	}
	break;
	
      case -1:
	// DOWN
	res = T_MovePlane(door->sector,
			  door->speed,
			  door->sector->floorheight,
			  false,1,door->direction);
	if (res == pastdest)
	{
	    switch(door->type)
	    {
	      case blazeRaise:
	      case blazeClose:
		door->sector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);  // unlink and free
		S_StartSound(&door->sector->soundorg, sfx_bdcls);
		break;
		
	      case normal:
	      case close:
		door->sector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);  // unlink and free
		break;
		
	      case close30ThenOpen:
		door->direction = 0;
		door->topcountdown = TICRATE*30;
		break;
		
	      default:
		break;
	    }
	}
	else if (res == crushed)
	{
	    switch(door->type)
	    {
	      case blazeClose:
	      case close:		// DO NOT GO BACK UP!
		break;
		
	      default:
		door->direction = 1;
		S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
		break;
	    }
	}
	break;
	
      case 1:
	// UP
	res = T_MovePlane(door->sector,
			  door->speed,
			  door->topheight,
			  false,1,door->direction);
	
	if (res == pastdest)
	{
	    switch(door->type)
	    {
	      case blazeRaise:
	      case normal:
		door->direction = 0; // wait at top
		door->topcountdown = door->topwait;
		break;
		
	      case close30ThenOpen:
	      case blazeOpen:
	      case open:
		door->sector->specialdata = NULL;
		P_RemoveThinker (&door->thinker);  // unlink and free
		break;
		
	      default:
		break;
	    }
	}
	break;
    }
}


//
// EV_DoLockedDoor
// Move a locked door up/down
//

int
EV_DoLockedDoor
( line_t*	line,
  vldoor_e	type,
  mobj_t*	thing )
{
    player_t*	p;
	
    p = thing->player;
	
    if (!p)
	return 0;
		
/*    switch(line->special)
    {
      case 99:	// Blue Lock
      case 133:
	if ( !p )
	    return 0;
	if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
	{
	    p->message = DEH_String(PD_BLUEO);
	    S_StartSound(NULL,sfx_oof);
	    return 0;
	}
	break;
	
      case 134: // Red Lock
      case 135:
	if ( !p )
	    return 0;
	if (!p->cards[it_redcard] && !p->cards[it_redskull])
	{
	    p->message = DEH_String(PD_REDO);
	    S_StartSound(NULL,sfx_oof);
	    return 0;
	}
	break;
	
      case 136:	// Yellow Lock
      case 137:
	if ( !p )
	    return 0;
	if (!p->cards[it_yellowcard] &&
	    !p->cards[it_yellowskull])
	{
	    p->message = DEH_String(PD_YELLOWO);
	    S_StartSound(NULL,sfx_oof);
	    return 0;
	}
	break;	
    }*/

    return EV_DoDoor(line,type);
}


int
EV_DoDoor
( line_t*	line,
  vldoor_e	type )
{
    int		secnum,rtn;
    sector_t*	sec;
    vldoor_t*	door;
	
    secnum = -1;
    rtn = 0;
    
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
	sec = &sectors[secnum];
	if (sec->specialdata)
	    continue;
		
	
	// new door thinker
	rtn = 1;
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;

	door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
	door->sector = sec;
	door->type = type;
	door->topwait = VDOORWAIT;
	door->speed = VDOORSPEED;
		
	switch(type)
	{
	  case blazeClose:
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    door->direction = -1;
	    door->speed = VDOORSPEED * 4;
	    S_StartSound(&door->sector->soundorg, sfx_bdcls);
	    break;
	    
	  case close:
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    door->direction = -1;
	    S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
	    break;
	    
	  case close30ThenOpen:
	    door->topheight = sec->ceilingheight;
	    door->direction = -1;
	    S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
	    break;
	    
	  case blazeRaise:
	  case blazeOpen:
	    door->direction = 1;
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    door->speed = VDOORSPEED * 4;
	    if (door->topheight != sec->ceilingheight)
		S_StartSound(&door->sector->soundorg, sfx_bdopn);
	    break;
	    
	  case normal:
	  case open:
	    door->direction = 1;
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4*FRACUNIT;
	    if (door->topheight != sec->ceilingheight)
		S_StartSound(&door->sector->soundorg, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
	    break;
	    
	  default:
	    break;
	}
		
    }
    return rtn;
}


//
// EV_VerticalDoor : open a door manually, no tag value
//
void
EV_VerticalDoor
( line_t*	line,
  mobj_t*	thing )
{
    player_t*	player;
    int		secnum;
    sector_t*	sec;
    vldoor_t*	door;
    int		side;
	
    side = 0;	// only front sides can be used

    //	Check for locks
    player = thing->player;
		
/*    switch(line->special)
    {
      case 26: // Blue Lock
      case 32:
	if ( !player )
	    return;
	
	if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
	{
	    player->message = DEH_String(PD_BLUEK);
	    S_StartSound(NULL,sfx_oof);
	    return;
	}
	break;
	
      case 27: // Yellow Lock
      case 34:
	if ( !player )
	    return;
	
	if (!player->cards[it_yellowcard] &&
	    !player->cards[it_yellowskull])
	{
	    player->message = DEH_String(PD_YELLOWK);
	    S_StartSound(NULL,sfx_oof);
	    return;
	}
	break;
	
      case 28: // Red Lock
      case 33:
	if ( !player )
	    return;
	
	if (!player->cards[it_redcard] && !player->cards[it_redskull])
	{
	    player->message = DEH_String(PD_REDK);
	    S_StartSound(NULL,sfx_oof);
	    return;
	}
	break;
    }*/
	
    // if the sector has an active thinker, use it
    sec = sides[ line->sidenum[side^1]] .sector;
    secnum = sec-sectors;

    if (sec->specialdata)
    {
	door = sec->specialdata;
	switch(line->special)
	{
	  case	1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
	  case	26:
	  case	27:
	  case	28:
	  case	117:
	    if (door->direction == -1)
		door->direction = 1;	// go back up
	    else
	    {
		if (!thing->player)
		    return;		// JDC: bad guys never close doors

                // When is a door not a door?
                // In Vanilla, door->direction is set, even though
                // "specialdata" might not actually point at a door.

                if (door->thinker.function.acp1 == (actionf_p1) T_VerticalDoor)
                {
                    door->direction = -1;	// start going down immediately
                }
                else if (door->thinker.function.acp1 == (actionf_p1) T_PlatRaise)
                {
                    // Erm, this is a plat, not a door.
                    // This notably causes a problem in ep1-0500.lmp where
                    // a plat and a door are cross-referenced; the door
                    // doesn't open on 64-bit.
                    // The direction field in vldoor_t corresponds to the wait
                    // field in plat_t.  Let's set that to -1 instead.

                    plat_t *plat;

                    plat = (plat_t *) door;
                    plat->wait = -1;
                }
                else
                {
                    // This isn't a door OR a plat.  Now we're in trouble.

                    fprintf(stderr, "EV_VerticalDoor: Tried to close "
                                    "something that wasn't a door.\n");

                    // Try closing it anyway. At least it will work on 32-bit
                    // machines.

                    door->direction = -1;
                }
	    }
	    return;
	}
    }
	
    // for proper sound
    switch(line->special)
    {
      case 117:	// BLAZING DOOR RAISE
      case 118:	// BLAZING DOOR OPEN
	S_StartSound(&sec->soundorg,sfx_bdopn);
	break;
	
      case 1:	// NORMAL DOOR SOUND
      case 31:
	S_StartSound(&sec->soundorg,sfx_swish); // villsa [STRIFE] TODO - fix sounds
	break;
	
      default:	// LOCKED DOOR SOUND
	S_StartSound(&sec->soundorg,sfx_swish); // villsa [STRIFE] TODO - fix sounds
	break;
    }
	
    
    // new door thinker
    door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
    P_AddThinker (&door->thinker);
    sec->specialdata = door;
    door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;

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
	
      case 117:	// blazing door raise
	door->type = blazeRaise;
	door->speed = VDOORSPEED*4;
	break;
      case 118:	// blazing door open
	door->type = blazeOpen;
	line->special = 0;
	door->speed = VDOORSPEED*4;
	break;
    }
    
    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
}


//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30 (sector_t* sec)
{
    vldoor_t*	door;
	
    door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);

    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * TICRATE;
}

//
// Spawn a door that opens after 5 minutes
//
void
P_SpawnDoorRaiseIn5Mins
( sector_t*	sec,
  int		secnum )
{
    vldoor_t*	door;
	
    door = Z_Malloc ( sizeof(*door), PU_LEVSPEC, 0);
    
    P_AddThinker (&door->thinker);

    sec->specialdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * TICRATE;
}


// villsa [STRIFE] resurrected sliding doors
//

//
// villsa [STRIFE]
//
// Sliding door name information
//
static slidename_t slideFrameNames[MAXSLIDEDOORS] =
{
    // SIGLDR
    {
        { "SIGLDR01" },  // frame1
        { "SIGLDR02" },  // frame2
        { "SIGLDR03" },  // frame3
        { "SIGLDR04" },  // frame4
        { "SIGLDR05" },  // frame5
        { "SIGLDR06" },  // frame6
        { "SIGLDR07" },  // frame7
        { "SIGLDR08" }   // frame8
    },
    // DORSTN
    {
        { "DORSTN01" },  // frame1
        { "DORSTN02" },  // frame2
        { "DORSTN03" },  // frame3
        { "DORSTN04" },  // frame4
        { "DORSTN05" },  // frame5
        { "DORSTN06" },  // frame6
        { "DORSTN07" },  // frame7
        { "DORSTN08" }   // frame8
    },

    // DORQTR
    {
        { "DORQTR01" },  // frame1
        { "DORQTR02" },  // frame2
        { "DORQTR03" },  // frame3
        { "DORQTR04" },  // frame4
        { "DORQTR05" },  // frame5
        { "DORQTR06" },  // frame6
        { "DORQTR07" },  // frame7
        { "DORQTR08" }   // frame8
    },

    // DORCRG
    {
        { "DORCRG01" },  // frame1
        { "DORCRG02" },  // frame2
        { "DORCRG03" },  // frame3
        { "DORCRG04" },  // frame4
        { "DORCRG05" },  // frame5
        { "DORCRG06" },  // frame6
        { "DORCRG07" },  // frame7
        { "DORCRG08" }   // frame8
    },

    // DORCHN
    {
        { "DORCHN01" },  // frame1
        { "DORCHN02" },  // frame2
        { "DORCHN03" },  // frame3
        { "DORCHN04" },  // frame4
        { "DORCHN05" },  // frame5
        { "DORCHN06" },  // frame6
        { "DORCHN07" },  // frame7
        { "DORCHN08" }   // frame8
    },

    // DORIRS
    {
        { "DORIRS01" },  // frame1
        { "DORIRS02" },  // frame2
        { "DORIRS03" },  // frame3
        { "DORIRS04" },  // frame4
        { "DORIRS05" },  // frame5
        { "DORIRS06" },  // frame6
        { "DORIRS07" },  // frame7
        { "DORIRS08" }   // frame8
    },

    // DORALN
    {
        { "DORALN01" },  // frame1
        { "DORALN02" },  // frame2
        { "DORALN03" },  // frame3
        { "DORALN04" },  // frame4
        { "DORALN05" },  // frame5
        { "DORALN06" },  // frame6
        { "DORALN07" },  // frame7
        { "DORALN08" }   // frame8
    },

    {"\0","\0","\0","\0","\0","\0","\0","\0"}
};

//
// villsa [STRIFE]
//
// Sliding door open sounds
//
static sfxenum_t slideOpenSounds[MAXSLIDEDOORS] =
{
    sfx_drlmto, sfx_drston, sfx_airlck, sfx_drsmto,
    sfx_drchno, sfx_airlck, sfx_airlck, sfx_None
};

//
// villsa [STRIFE]
//
// Sliding door close sounds
//
static sfxenum_t slideCloseSounds[MAXSLIDEDOORS] =
{
    sfx_drlmtc, sfx_drston, sfx_airlck, sfx_drsmtc,
    sfx_drchnc, sfx_airlck, sfx_airlck, sfx_None
};

slideframe_t slideFrames[MAXSLIDEDOORS];

//
// P_InitSlidingDoorFrames
//
// villsa [STRIFE] resurrected
//
void P_InitSlidingDoorFrames(void)
{
    int i;
    int f1;
    int f2;
    int f3;
    int f4;

    memset(slideFrames, 0, sizeof(slideframe_t) * MAXSLIDEDOORS);
	
    for(i = 0; i < MAXSLIDEDOORS; i++)
    {
	if(!slideFrameNames[i].frame1[0])
	    break;
			
	f1 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame1));
	f2 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame2));
	f3 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame3));
	f4 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame4));

	slideFrames[i].frames[0] = f1;
	slideFrames[i].frames[1] = f2;
	slideFrames[i].frames[2] = f3;
	slideFrames[i].frames[3] = f4;
		
	f1 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame5));
	f2 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame6));
	f3 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame7));
	f4 = R_TextureNumForName(DEH_String(slideFrameNames[i].frame8));

	slideFrames[i].frames[4] = f1;
	slideFrames[i].frames[5] = f2;
	slideFrames[i].frames[6] = f3;
	slideFrames[i].frames[7] = f4;
    }
}


//
// P_FindSlidingDoorType
//
// Return index into "slideFrames" array
// for which door type to use
//
// villsa [STRIFE] resurrected
//
int P_FindSlidingDoorType(line_t* line)
{
    int i;
    int val;
	
    for(i = 0; i < MAXSLIDEDOORS; i++)
    {
        val = sides[line->sidenum[0]].toptexture;
	if(val == slideFrames[i].frames[0])
	    return i;
    }
	
    return -1;
}

//
// T_SlidingDoor
//
// villsa [STRIFE] resurrected
//
void T_SlidingDoor(slidedoor_t* door)
{
    sector_t* sec;

    sec = door->frontsector;

    switch(door->status)
    {
    case sd_opening:
        if(!door->timer--)
        {
            if(++door->frame == SNUMFRAMES)
            {
                // IF DOOR IS DONE OPENING...
                door->line1->flags &= ~ML_BLOCKING;
                door->line2->flags &= ~ML_BLOCKING;

                if(door->type == sdt_openOnly)
                {
                    door->frontsector->specialdata = NULL;
                    P_RemoveThinker (&door->thinker);
                    return;
                }

                door->timer = SDOORWAIT;
                door->status = sd_waiting;
            }
            else
            {
                // IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
                door->timer = SWAITTICS;

                sides[door->line2->sidenum[0]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];

                sides[door->line2->sidenum[1]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];

                sides[door->line1->sidenum[0]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];

                sides[door->line1->sidenum[1]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];
            }
        }

        return;

    case sd_waiting:
        // IF DOOR IS DONE WAITING...
        if(!door->timer--)
        {
            fixed_t speed;
            boolean thinglist;

            sec = door->frontsector;
            thinglist = (sec->thinglist != NULL);

            // CAN DOOR CLOSE?
            if(thinglist)
            {
                door->timer = SDOORWAIT;
                return;
            }
            else
            {

                speed = sec->ceilingheight -
                    sec->floorheight - (10*FRACUNIT);

                // something blocking it?
                // [STRIFE] TODO this function here is causing some abnormalites when closing
                if(T_MovePlane(sec, speed, sec->floorheight, thinglist, 1, -1) == 1)
                {
                    door->timer = SDOORWAIT;
                    return;
                }
                else
                {
                    // Instantly move plane
                    T_MovePlane(sec, (128*FRACUNIT), sec->ceilingheight, 0, 1, 1);

                    // turn line blocking back on
                    door->line1->flags |= ML_BLOCKING;
                    door->line2->flags |= ML_BLOCKING;

                    // play close sound
                    S_StartSound(&sec->soundorg, slideCloseSounds[door->whichDoorIndex]);

                    door->status = sd_closing;
                    door->timer = SWAITTICS;
                }
            }
        }

        return;

    case sd_closing:
        if (!door->timer--)
        {
            if(--door->frame < 0)
            {
                // IF DOOR IS DONE CLOSING...
                T_MovePlane(sec, (128*FRACUNIT), sec->floorheight, 0, 1, -1);
                door->frontsector->specialdata = NULL;
                P_RemoveThinker (&door->thinker);
                return;
            }
            else
            {
                // IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
                door->timer = SWAITTICS;

                sides[door->line2->sidenum[0]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];

                sides[door->line2->sidenum[1]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];

                sides[door->line1->sidenum[0]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];

                sides[door->line1->sidenum[1]].midtexture =
                    slideFrames[door->whichDoorIndex].frames[door->frame];
            }
        }

        return;
    }
}

//
// EV_RemoteSlidingDoor
//
// villsa [STRIFE] TODO
//
void EV_RemoteSlidingDoor(line_t* line, mobj_t* thing)
{
}


//
// EV_SlidingDoor
//
// villsa [STRIFE]
//
void EV_SlidingDoor(line_t* line, mobj_t* thing)
{
    sector_t*       sec;
    slidedoor_t*    door;
    int             i;
    line_t*         secline;

    // Make sure door isn't already being animated
    sec = sides[line->sidenum[1]].sector;
    door = NULL;
    if(sec->specialdata)
    {
        if (!thing->player)
            return;

        door = sec->specialdata;
        if(door->type == sdt_openAndClose)
        {
            if(door->status == sd_waiting)
            {
                // [STRIFE] TODO lines are still passable when closed manually by player
                door->status = sd_closing;
                door->timer = SWAITTICS;    // villsa [STRIFE]
            }
        }
        else
            return;
    }

    // Init sliding door vars
    if(!door)
    {
        door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
        P_AddThinker (&door->thinker);

        sec->specialdata = door;

        door->type = sdt_openAndClose;
        door->status = sd_opening;
        door->whichDoorIndex = P_FindSlidingDoorType(line);

        // villsa [STRIFE] different error message
        if(door->whichDoorIndex < 0)
            I_Error(DEH_String("EV_SlidingDoor: Textures are not defined for sliding door!"));

        sides[line->sidenum[0]].midtexture = sides[line->sidenum[0]].toptexture;

        // villsa [STRIFE]
        door->line1 = line;
        door->line2 = line;

        // villsa [STRIFE] this loop assumes that the sliding door is made up
        // of only four linedefs!
        for(i = 0; i < 4; i++)
        {
            secline = sec->lines[i];
            if(secline != line)
            {
                side_t* side1;
                side_t* side2;

                side1 = &sides[secline->sidenum[0]];
                side2 = &sides[line->sidenum[0]];

                if(side1->toptexture == side2->toptexture)
                    door->line2 = secline;
            }
        }

        door->thinker.function.acp1 = (actionf_p1)T_SlidingDoor;
        door->timer = SWAITTICS;
        door->frontsector = sec;
        door->frame = 0;

        // villsa [STRIFE] preset flags
        door->line1->flags |= ML_BLOCKING;
        door->line2->flags |= ML_BLOCKING;

        // villsa [STRIFE] set the closing sector
        T_MovePlane(
            door->frontsector,
            (128*FRACUNIT),
            P_FindLowestCeilingSurrounding(door->frontsector),
            0,
            1,
            1);

        // villsa [STRIFE] play open sound
        S_StartSound(&door->frontsector->soundorg, slideOpenSounds[door->whichDoorIndex]);
    }
}

