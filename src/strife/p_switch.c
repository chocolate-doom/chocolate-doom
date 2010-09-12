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
//
// DESCRIPTION:
//	Switches, buttons. Two-state animation. Exits.
//
//-----------------------------------------------------------------------------

#include <stdio.h>

#include "i_system.h"
#include "deh_main.h"
#include "doomdef.h"
#include "p_local.h"

#include "g_game.h"
#include "d_main.h" // villsa [STRIFE]
#include "s_sound.h"

// Data.
#include "sounds.h"

// State.
#include "doomstat.h"
#include "r_state.h"
#include "m_bbox.h"     // villsa [STRIFE]


//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
// villsa [STRIFE] new switch list
switchlist_t alphSwitchList[] =
{
    { "GLASS01",    "GLASS02",      1,  sfx_bglass  },
    { "GLASS03",    "GLASS04",      1,  sfx_bglass  },
    { "GLASS05",    "GLASS06",      1,  sfx_bglass  },
    { "GLASS07",    "GLASS08",      1,  sfx_bglass  },
    { "GLASS17",    "GLASS18",      1,  sfx_bglass  },
    { "GLASS19",    "GLASS20",      1,  sfx_bglass  },
    { "SWKNOB01",   "SWKNOB02",     1,  sfx_swknob  },
    { "SWLITE01",   "SWLITE02",     1,  sfx_None    },
    { "SWCHN01",    "SWCHN02",      1,  sfx_pulchn  },
    { "COMP01",     "COMP04B",      1,  sfx_bglass  },
    { "COMP05",     "COMP12B",      1,  sfx_bglass  },
    { "COMP09",     "COMP12B",      1,  sfx_bglass  },
    { "COMP12",     "COMP04B",      1,  sfx_bglass  },
    { "COMP13",     "COMP12B",      1,  sfx_bglass  },
    { "COMP17",     "COMP20B",      1,  sfx_bglass  },
    { "COMP21",     "COMP28B",      1,  sfx_bglass  },
    { "WALTEK09",   "WALTEKB1",     1,  sfx_None    },
    { "WALTEK10",   "WALTEKB1",     1,  sfx_None    },
    { "WALTEK15",   "WALTEKB1",     1,  sfx_None    },
    { "SWFORC01",   "SWFORC02",     1,  sfx_None    },
    { "SWEXIT01",   "SWEXIT02",     1,  sfx_None    },
    { "DORSBK01",   "DORSBK02",     1,  sfx_swston  },
    { "SWSLD01",    "SWSLD02",      1,  sfx_None    },
    { "DORWS04",    "DORWS05",      1,  sfx_swbolt  },
    { "SWIRON01",   "SWIRON02",     1,  sfx_None    },
    { "GLASS09",    "GLASS10",      2,  sfx_bglass  },
    { "GLASS11",    "GLASS12",      2,  sfx_bglass  },
    { "GLASS13",    "GLASS14",      2,  sfx_bglass  },
    { "GLASS15",    "GLASS16",      2,  sfx_bglass  },
    { "SWFORC03",   "SWFORC04",     2,  sfx_None    },
    { "SWCIT01",    "SWCIT02",      2,  sfx_None    },
    { "SWTRMG01",   "SWTRMG04",     2,  sfx_None    },
    { "SWMETL01",   "SWMETL02",     2,  sfx_None    },
    { "SWWOOD01",   "SWWOOD02",     2,  sfx_None    },
    { "SWTKBL01",   "SWTKBL02",     2,  sfx_None    },
    { "AZWAL21",    "AZWAL22",      2,  sfx_None    },
    { "SWINDT01",   "SWINDT02",     2,  sfx_None    },
    { "SWRUST01",   "SWRUST02",     2,  sfx_None    },
    { "SWCHAP01",   "SWCHAP02",     2,  sfx_None    },
    { "SWALIN01",   "SWALIN02",     2,  sfx_None    },
    { "SWWALG01",   "SWWALG02",     2,  sfx_None    },
    { "SWWALG03",   "SWWALG04",     2,  sfx_None    },
    { "SWTRAM01",   "SWTRAM02",     2,  sfx_None    },
    { "SWTRAM03",   "SWTRAM04",     2,  sfx_None    },
    { "SWORC01",    "SWORC02",      2,  sfx_None    },
    { "SWBRIK01",   "SWBRIK02",     2,  sfx_None    },
    { "SWIRON03",   "SWIRON04",     2,  sfx_None    },
    { "SWIRON05",   "SWIRON06",     2,  sfx_None    },
    { "SWIRON07",   "SWIRON08",     2,  sfx_None    },
    { "SWCARD01",   "SWCARD02",     2,  sfx_keycrd  },
    { "SWSIGN01",   "SWSIGN02",     2,  sfx_None    },
    { "SWLEV01",    "SWLEV02",      2,  sfx_None    },
    { "SWLEV03",    "SWLEV04",      2,  sfx_None    },
    { "SWLEV05",    "SWLEV06",      2,  sfx_None    },
    { "SWBRN01",    "SWBRN02",      2,  sfx_keycrd  },
    { "SWPIP01",    "SWPIP02",      2,  sfx_valve   },
    { "SWPALM01",   "SWPALM02",     2,  sfx_swscan  },
    { "SWKNOB03",   "SWKNOB04",     2,  sfx_swknob  },
    { "ALTSW01",    "ALTSW02",      2,  sfx_None    },
    { "COMP25",     "COMP28B",      2,  sfx_bglass  },
    { "COMP29",     "COMP20B",      2,  sfx_bglass  },
    { "COMP33",     "COMP50",       2,  sfx_bglass  },
    { "COMP42",     "COMP51",       2,  sfx_bglass  },
    { "GODSCRN1",   "GODSCRN2",     2,  sfx_difool  },
    { "ALIEN04",    "ALIEN05",      2,  sfx_None    },
    { "CITADL04",   "CITADL05",     2,  sfx_None    },
    { "SWITE03",    "SWITE04",      2,  sfx_None    },
    { "SWTELP01",   "SWTELP02",     2,  sfx_None    },
    { "BRNSCN01",   "BRNSCN05",     2,  sfx_firxpl  },
    { "\0",         "\0",           0,  sfx_None    }
};

int		switchlist[MAXSWITCHES * 2];
int		numswitches;
button_t        buttonlist[MAXBUTTONS];

//
// P_InitSwitchList
// Only called at game initialization.
//
void P_InitSwitchList(void)
{
    int		i;
    int		index;
    int		episode;
	
    episode = 1;

    if(isregistered)
        episode = 2;
    // villsa [STRIFE] unused
    /*else
	if ( gamemode == commercial )
	    episode = 3;*/
		
    for(index = 0, i = 0; i < MAXSWITCHES; i++)
    {
	if(!alphSwitchList[i].episode)
	{
	    numswitches = index/2;
	    switchlist[index] = -1;
	    break;
	}
		
	if (alphSwitchList[i].episode <= episode)
	{
	    switchlist[index++] = R_TextureNumForName(DEH_String(alphSwitchList[i].name1));
	    switchlist[index++] = R_TextureNumForName(DEH_String(alphSwitchList[i].name2));
	}
    }
}


//
// P_StartButton
// Start a button counting down till it turns off.
//
void P_StartButton(line_t* line, bwhere_e w, int texture, int time)
{
    int		i;
    
    // See if button is already pressed
    for(i = 0; i < MAXBUTTONS; i++)
    {
	if(buttonlist[i].btimer && buttonlist[i].line == line)
            return;
    }
    

    
    for(i = 0; i < MAXBUTTONS; i++)
    {
	if(!buttonlist[i].btimer)
	{
	    buttonlist[i].line = line;
	    buttonlist[i].where = w;
	    buttonlist[i].btexture = texture;
	    buttonlist[i].btimer = time;
	    buttonlist[i].soundorg = &line->frontsector->soundorg;
	    return;
	}
    }
    
    I_Error("P_StartButton: no button slots left!");
}


//
// P_SpawnBrokenGlass
// villsa [STRIFE] new function
//
static void P_SpawnBrokenGlass(line_t* line)
{
    fixed_t x1;
    fixed_t x2;
    fixed_t y1;
    fixed_t y2;
    int i;
    mobj_t* glass;
    angle_t an;

    x1 = (line->v2->x + line->v1->x) / 2;
    y1 = (line->v2->y + line->v1->y) / 2;
    x2 = ((line->frontsector->soundorg.x - x1) / 5) + x1;
    y2 = ((line->frontsector->soundorg.y - y1) / 5) + y1;

    for(i = 0; i < 7; i++)
    {
        glass = P_SpawnMobj(x2, y2, ONFLOORZ, MT_JUNK);
        glass->z += (24*FRACUNIT);
        glass->flags |= (MF_SHADOW|MF_MVIS);

        P_SetMobjState(glass, P_Random() % 3 + S_SHRD_03); // 284

        an = ((P_Random() << 13) / 255);

        glass->angle = (an << ANGLETOFINESHIFT);
        glass->momx = FixedMul(finecosine[an], (P_Random() & 3) << FRACBITS);
        glass->momy = FixedMul(finesine[an], (P_Random() & 3) << FRACBITS);
        glass->momz = (P_Random() & 7) << FRACBITS;
        glass->tics += (P_Random() + 7) & 7;
    }
}


//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
void P_ChangeSwitchTexture(line_t* line, int useAgain)
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;
    int     sound;
    boolean breakglass; // villsa [STRIFE]
    switchlist_t* sl;   // villsa [STRIFE]

    breakglass = false; // villsa [STRIFE]

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;
	
    sound = sfx_swtchn;

    // EXIT SWITCH?
    // villsa [STRIFE] check for linetype 182 (break glass)
    if(line->special == 182)
    {
        line->flags &= ~ML_BLOCKMONSTERS;
        breakglass = true;

        if(useAgain)
        {
            texMid = 0;
            texTop = 0;
        }

        if(texBot)
            useAgain = 0;

        sound = sfx_bglass;
    }

    if(!useAgain)
	line->special = 0;
	
    for(i = 0; i < numswitches*2; i++)
    {
        sl = &alphSwitchList[i / 2]; // villsa [STRIFE]

	if(switchlist[i] == texTop)
	{
            // villsa [STRIFE] set sound
            if(sl->sound)
                sound = sl->sound;

	    S_StartSound(buttonlist->soundorg,sound);
	    sides[line->sidenum[0]].toptexture = switchlist[i^1];

	    if(useAgain)
		P_StartButton(line,top,switchlist[i],BUTTONTIME);

            if(breakglass)
                P_SpawnBrokenGlass(line);

	    return;
	}
	else
	{
	    if(switchlist[i] == texMid)
	    {
                // villsa [STRIFE] set sound
                if(sl->sound)
                    sound = sl->sound;

		S_StartSound(buttonlist->soundorg,sound);
		sides[line->sidenum[0]].midtexture = switchlist[i^1];

                // villsa [STRIFE] affect second side of line
                if(line->flags & ML_TWOSIDED)
                    sides[line->sidenum[1]].midtexture = switchlist[i^1];

		if(useAgain)
		    P_StartButton(line, middle,switchlist[i],BUTTONTIME);

                // villsa [STRIFE]
                if(sound == sfx_firxpl)
                {
                    breakglass = true;

                    // give quest token #28 to player
                    players[0].questflags |= (1 << ((MT_TOKEN_QUEST28 - MT_TOKEN_QUEST1) + 1));

                    // give stamina/accuracy items
                    if(!netgame)
                    {
                        P_GiveItemToPlayer(players, SPR_TOKN, MT_TOKEN_STAMINA);
                        P_GiveItemToPlayer(players, SPR_TOKN, MT_TOKEN_NEW_ACCURACY);
                    }

                }

                // villsa [STRIFE]
                if(breakglass || sound == sfx_bglass)
                    P_SpawnBrokenGlass(line);

		return;
	    }
	    else
	    {
		if(switchlist[i] == texBot)
		{
                    // villsa [STRIFE] set sound
                    if(sl->sound)
                        sound = sl->sound;

		    S_StartSound(buttonlist->soundorg,sound);
		    sides[line->sidenum[0]].bottomtexture = switchlist[i^1];

		    if(useAgain)
			P_StartButton(line, bottom,switchlist[i],BUTTONTIME);

                    if(breakglass)
                        P_SpawnBrokenGlass(line);

		    return;
		}
	    }
	}
    }
}

//
// P_MoveWall
//
// villsa [STRIFE] New function.
// Dynamically move a solid line. Unused in Strife
//
static void P_MoveWall(line_t *line, mobj_t *thing)
{
    vertex_t *v2;
    vertex_t *v1;
    fixed_t x;
    fixed_t y;

    v1 = line->v1;
    v2 = line->v2;
    S_StartSound(thing, sfx_stnmov);

    if (line->dx)
    {
        if (thing->x >= v1->x)
        {
            v1->y -= (8 * FRACUNIT);
            v2->y -= (8 * FRACUNIT);
        }
        else
        {
            v1->y += (8 * FRACUNIT);
            v2->y += (8 * FRACUNIT);
        }
    }
    else
    {
        if (thing->y >= v1->y)
        {
            v1->x -= (8 * FRACUNIT);
            v2->x -= (8 * FRACUNIT);
        }
        else
        {
            v1->x += (8 * FRACUNIT);
            v2->x += (8 * FRACUNIT);
        }
    }

    if (v1->x >= v2->x)
    {
        line->bbox[BOXLEFT] = v2->x;
        x = v1->x;
    }
    else
    {
        line->bbox[BOXLEFT] = v1->x;
        x = v2->x;
    }

    line->bbox[BOXRIGHT] = x;

    if (v1->y >= v2->y)
    {
        line->bbox[BOXBOTTOM] = v2->y;
        y = v1->y;
    }
    else
    {
        line->bbox[BOXBOTTOM] = v1->y;
        y = v2->y;
    }

    line->bbox[BOXTOP] = y;
}




//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
boolean
P_UseSpecialLine
( mobj_t*	thing,
  line_t*	line,
  int		side )
{               

    // Err...
    // Use the back sides of VERY SPECIAL lines...
    if (side)
    {
	switch(line->special)
	{
	  case 124:
	    // Sliding door open&close
	    // UNUSED?
	    break;

	  default:
	    return false;
	    break;
	}
    }

    
    // Switches that other things can activate.
    if (!thing->player)
    {
	// never open secret doors
	if (line->flags & ML_SECRET)
	    return false;
	
	switch(line->special)
	{
	  case 1: 	// MANUAL DOOR RAISE
	  case 32:	// MANUAL BLUE
	  case 33:	// MANUAL RED
	  case 34:	// MANUAL YELLOW
	    break;
	    
	  default:
	    return false;
	    break;
	}
    }

    
    // do something  
    switch (line->special)
    {
	// MANUALS
      case 1:		// Vertical Door
      case 26:		// Blue Door/Locked
      case 27:		// Yellow Door /Locked
      case 28:		// Red Door /Locked

      case 31:		// Manual door open
      case 32:		// Blue locked door open
      case 33:		// Red locked door open
      case 34:		// Yellow locked door open

      case 117:		// Blazing door raise
      case 118:		// Blazing door open
	EV_VerticalDoor (line, thing);
	break;
	
	//UNUSED - Door Slide Open&Close
	// case 124:
	// EV_SlidingDoor (line, thing);
	// break;

	// SWITCHES
      case 7:
	// Build Stairs
	if (EV_BuildStairs(line,build8))
	    P_ChangeSwitchTexture(line,0);
	break;

      case 9:
	// Change Donut
	if (EV_DoDonut(line))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 11:
	// Exit level
	P_ChangeSwitchTexture(line,0);
	G_ExitLevel (0);
	break;
	
      case 14:
	// Raise Floor 32 and change texture
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 15:
	// Raise Floor 24 and change texture
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 18:
	// Raise Floor to next highest floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 20:
	// Raise Plat next highest floor and change texture
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 21:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line,downWaitUpStay,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 23:
	// Lower Floor to Lowest
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 29:
	// Raise Door
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 41:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 71:
	// Turbo Lower Floor
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 49:
	// Ceiling Crush And Raise
	if (EV_DoCeiling(line,crushAndRaise))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 50:
	// Close Door
	if (EV_DoDoor(line,close))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 51:
	// Secret EXIT
	P_ChangeSwitchTexture(line,0);
	//G_SecretExitLevel ();
	break;
	
      case 55:
	// Raise Floor Crush
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 101:
	// Raise Floor
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 102:
	// Lower Floor to Surrounding floor height
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 103:
	// Open Door
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 111:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 112:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 113:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 122:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 127:
	// Build Stairs Turbo 16
	if (EV_BuildStairs(line,turbo16))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 131:
	// Raise Floor Turbo
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 133:
	// BlzOpenDoor BLUE
      case 135:
	// BlzOpenDoor RED
      case 137:
	// BlzOpenDoor YELLOW
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 140:
	// Raise Floor 512
	if (EV_DoFloor(line,raiseFloor512))
	    P_ChangeSwitchTexture(line,0);
	break;
	
	// BUTTONS
      case 42:
	// Close Door
	if (EV_DoDoor(line,close))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 43:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 45:
	// Lower Floor to Surrounding floor height
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 60:
	// Lower Floor to Lowest
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 61:
	// Open Door
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 62:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line,downWaitUpStay,1))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 63:
	// Raise Door
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 64:
	// Raise Floor to ceiling
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 66:
	// Raise Floor 24 and change texture
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 67:
	// Raise Floor 32 and change texture
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 65:
	// Raise Floor Crush
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 68:
	// Raise Plat to next highest floor and change texture
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 69:
	// Raise Floor to next highest floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 70:
	// Turbo Lower Floor
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 114:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 115:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 116:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 123:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 132:
	// Raise Floor Turbo
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 99:
	// BlzOpenDoor BLUE
      case 134:
	// BlzOpenDoor RED
      case 136:
	// BlzOpenDoor YELLOW
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 138:
	// Light Turn On
	EV_LightTurnOn(line,255);
	P_ChangeSwitchTexture(line,1);
	break;
	
      case 139:
	// Light Turn Off
	EV_LightTurnOn(line,35);
	P_ChangeSwitchTexture(line,1);
	break;

      case 666:     // villsa [STRIFE]
          // Move wall
          P_MoveWall(line, thing);
          break;
			
    }
	
    return true;
}

