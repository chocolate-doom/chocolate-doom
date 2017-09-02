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
//
// DESCRIPTION:
//	Switches, buttons. Two-state animation. Exits.
//

#include <stdio.h>

#include "i_system.h"
#include "deh_main.h"
#include "doomdef.h"
#include "p_local.h"

#include "g_game.h"
#include "d_main.h" // villsa [STRIFE]
#include "z_zone.h" // villsa [STRIFE]
#include "w_wad.h"  // villsa [STRIFE]
#include "s_sound.h"
#include "m_random.h" // haleyjd [STRIFE]
#include "p_dialog.h"
#include "p_local.h"  // haleyjd [STRIFE]
#include "m_bbox.h"   // villsa [STRIFE]
#include "m_misc.h"

// Data.
#include "sounds.h"

// State.
#include "doomstat.h"
#include "r_state.h"

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
    int i, slindex, episode;

    // Note that this is called "episode" here but it's actually something
    // quite different. As we progress from Shareware->Registered->Doom II
    // we support more switch textures.
    if (isregistered)
    {
        episode = 2;
    }
    else
    {
        episode = 1;
    }

    slindex = 0;

    for (i = 0; i < arrlen(alphSwitchList); i++)
    {
	if (alphSwitchList[i].episode <= episode)
	{
	    switchlist[slindex++] =
                R_TextureNumForName(DEH_String(alphSwitchList[i].name1));
	    switchlist[slindex++] =
                R_TextureNumForName(DEH_String(alphSwitchList[i].name2));
	}
    }

    numswitches = slindex / 2;
    switchlist[slindex] = -1;
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
        glass->momy = FixedMul(finesine[an],   (P_Random() & 3) << FRACBITS);
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

    // villsa [STRIFE] check for linetype 182 (break glass)
    if(line->special == 182)
    {
        line->flags &= ~ML_BLOCKING;
        breakglass = true;

        if(useAgain)
        {
            // haleyjd 09/21/10: Corrected (>> 16 == next field)
            texTop = 0;
            texBot = 0;
        }

        if(texMid) // haleyjd 09/21/10: Corrected (>> 16 == next field)
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

            // haleyjd 20141026: [STRIFE]: Rogue fixed wrong sound origin
            S_StartSound(&line->frontsector->soundorg, sound);
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

                // haleyjd 20141026: [STRIFE]: Rogue fixed wrong sound origin
                S_StartSound(&line->frontsector->soundorg, sound);
                sides[line->sidenum[0]].midtexture = switchlist[i^1];

                // villsa [STRIFE] affect second side of line
                // BUG: will crash if 1S line is marked with TWOSIDED flag!
                if(line->flags & ML_TWOSIDED)
                    sides[line->sidenum[1]].midtexture = switchlist[i^1];

                if(useAgain)
                    P_StartButton(line, middle,switchlist[i],BUTTONTIME);

                // villsa [STRIFE]: Mines Transmitter hack
                if(sound == sfx_firxpl)
                {
                    breakglass = true;

                    // give quest flag 29 to player
                    players[0].questflags |= QF_QUEST29;

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

                    // haleyjd 20141026: [STRIFE]: Rogue fixed wrong sound origin
                    S_StartSound(&line->frontsector->soundorg, sound);
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

// villsa [STRIFE]
static char usemessage[92];

//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
boolean P_UseSpecialLine(mobj_t* thing, line_t* line, int side)
{
    // Err...
    // Use the back sides of VERY SPECIAL lines...
    if (side)
    {
        switch(line->special)
        {
        case 148: // haleyjd [STRIFE]
            break;

        default:
            return false;
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
        case 1:         // MANUAL DOOR RAISE
        case 31:        // haleyjd [STRIFE]
        case 144:       // haleyjd [STRIFE] Manual sliding door
            break;

        default:
            return false;
            break;
        }
    }
   
    // do something  
    switch(line->special)
    {
        // MANUALS
    case 1:             // Vertical Door
    case 26:            // DR ID Card
    case 27:            // DR Pass Card
    case 28:            // DR ID Badge
    case 31:            // Manual door open
    case 32:            // D1 ID Card
    case 33:            // D1 ID Badge
    case 34:            // D1 Pass Card
    case 117:           // Blazing door raise
    case 118:           // Blazing door open
    case 156:           // haleyjd [STRIFE] D1 Brass Key
    case 157:           // haleyjd [STRIFE] D1 Silver Key
    case 158:           // haleyjd [STRIFE] D1 Gold Key
    case 159:           // haleyjd [STRIFE] DR Gold Key
    case 160:           // haleyjd [STRIFE] DR Silver Key
    case 161:           // haleyjd [STRIFE] DR Brass Key
    case 165:           // villsa  [STRIFE] That doesn't seem to work
    case 166:           // haleyjd [STRIFE] DR Hand Print
    case 169:           // haleyjd [STRIFE] DR Base Key
    case 170:           // haleyjd [STRIFE] DR Gov's Key
    case 190:           // haleyjd [STRIFE] DR Order Key
    case 205:           // villsa  [STRIFE] Available in retail only
    case 213:           // haleyjd [STRIFE] DR Chalice
    case 217:           // haleyjd [STRIFE] DR Core Key
    case 221:           // haleyjd [STRIFE] DR Mauler Key
    case 224:           // haleyjd [STRIFE] DR Chapel Key
    case 225:           // haleyjd [STRIFE] DR Catacomb Key
    case 232:           // villsa  [STRIFE] DR Oracle Pass
        EV_VerticalDoor (line, thing);
        break;

    // haleyjd: For the sake of our sanity, I have reordered all the line
    // specials from this point down so that they are strictly in numeric
    // order, and not divided up in a semi-arbitrary fashion.

    case 7:
        // Build Stairs - [STRIFE] Verified unmodified
        if (EV_BuildStairs(line,build8))
            P_ChangeSwitchTexture(line,0);
        break;

    case 9:
        // Change Donut - [STRIFE] Verified unmodified
        if (EV_DoDonut(line))
            P_ChangeSwitchTexture(line,0);
        break;

    case 11:
        // Exit level - [STRIFE] Modified to take tag, etc.
        P_ChangeSwitchTexture(line, 1);
        if(levelTimer && levelTimeCount)
            break;
        G_ExitLevel(line->tag);
        break;

    case 14:
        // Raise Floor 32 and change texture - [STRIFE] Verified unmodified
        if (EV_DoPlat(line, raiseAndChange,32))
            P_ChangeSwitchTexture(line,0);
        break;

    case 15:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line, raiseAndChange,24))
            P_ChangeSwitchTexture(line,0);
        break;

    case 18:
        // Raise Floor to next highest floor - [STRIFE] Verified unmodified
        if (EV_DoFloor(line, raiseFloorToNearest))
            P_ChangeSwitchTexture(line,0);
        break;

    case 20:
        // Raise Plat next highest floor and change texture - [STRIFE] Verified unmodified
        if(EV_DoPlat(line, raiseToNearestAndChange, 0))
            P_ChangeSwitchTexture(line,0);
        break;

    case 21:
        // PlatDownWaitUpStay - [STRIFE] Verified unmodified
        if (EV_DoPlat(line, downWaitUpStay,0))
            P_ChangeSwitchTexture(line,0);
        break;

    case 23:
        // Lower Floor to Lowest - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,lowerFloorToLowest))
            P_ChangeSwitchTexture(line,0);
        break;

    case 29:
        // Raise Door - [STRIFE] Verified unmodified
        if (EV_DoDoor(line,vld_normal))
            P_ChangeSwitchTexture(line,0);
        break;

    case 40:
        // villsa [STRIFE] Split Open Door
        if(EV_DoDoor(line, vld_splitOpen))
            P_ChangeSwitchTexture(line, 0);
        break; // haleyjd

    case 41:
        // Lower Ceiling to Floor - [STRIFE] Verified unmodified
        if (EV_DoCeiling(line,lowerToFloor))
            P_ChangeSwitchTexture(line,0);
        break;

    case 42:
        // Close Door - [STRIFE] Verified unmodified
        if (EV_DoDoor(line,vld_close))
            P_ChangeSwitchTexture(line,1);
        break;

    case 43:
        // Lower Ceiling to Floor - [STRIFE] Verified unmodified
        if (EV_DoCeiling(line,lowerToFloor))
            P_ChangeSwitchTexture(line,1);
        break;

    case 45:
        // Lower Floor to Surrounding floor height - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,lowerFloor))
            P_ChangeSwitchTexture(line,1);
        break;

    case 49:
        // Ceiling Crush And Raise - [STRIFE] Verified unmodified
        if (EV_DoCeiling(line,crushAndRaise))
            P_ChangeSwitchTexture(line,0);
        break;

    case 50:
        // Close Door - [STRIFE] Verified unmodified
        if (EV_DoDoor(line,vld_close))
            P_ChangeSwitchTexture(line,0);
        break;

    case 51:
        // [STRIFE] Modifed into S1 Start Finale (was Secret Exit)
        P_ChangeSwitchTexture(line,0);
        G_StartFinale();
        break;

    case 55:
        // Raise Floor Crush - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloorCrush))
            P_ChangeSwitchTexture(line,0);
        break;

    case 60:
        // Lower Floor to Lowest - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,lowerFloorToLowest))
            P_ChangeSwitchTexture(line,1);
        break;

    case 61:
        // Open Door - [STRIFE] Verified unmodified
        if (EV_DoDoor(line,vld_open))
            P_ChangeSwitchTexture(line,1);
        break;

    case 62:
        // PlatDownWaitUpStay - [STRIFE] Verified unmodified
        if (EV_DoPlat(line, downWaitUpStay,1))
            P_ChangeSwitchTexture(line,1);
        break;

    case 63:
        // Raise Door - [STRIFE] Verified unmodified
        if (EV_DoDoor(line,vld_normal))
            P_ChangeSwitchTexture(line,1);
        break;

    case 64:
        // Raise Floor to ceiling - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloor))
            P_ChangeSwitchTexture(line,1);
        break;

    case 65:
        // Raise Floor Crush - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloorCrush))
            P_ChangeSwitchTexture(line,1);
        break;

    case 66:
        // Raise Floor 24 and change texture - [STRIFE] Verified unmodified
        if (EV_DoPlat(line, raiseAndChange, 24))
            P_ChangeSwitchTexture(line,1);
        break;

    case 67:
        // Raise Floor 32 and change texture - [STRIFE] Verified unmodified
        if (EV_DoPlat(line, raiseAndChange, 32))
            P_ChangeSwitchTexture(line,1);
        break;
   
    case 68:
        // Raise Plat to next highest floor and change texture - [STRIFE] Verified unmodified
        if (EV_DoPlat(line, raiseToNearestAndChange, 0))
            P_ChangeSwitchTexture(line,1);
        break;

    case 69:
        // Raise Floor to next highest floor - [STRIFE] Verified unmodified
        if (EV_DoFloor(line, raiseFloorToNearest))
            P_ChangeSwitchTexture(line,1);
        break;

    case 70:
        // Turbo Lower Floor - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,turboLower))
            P_ChangeSwitchTexture(line,1);
        break;

    case 71:
        // Turbo Lower Floor - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,turboLower))
            P_ChangeSwitchTexture(line,0);
        break;

    case 101:
        // Raise Floor - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloor))
            P_ChangeSwitchTexture(line,0);
        break;

    case 102:
        // Lower Floor to Surrounding floor height - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,lowerFloor))
            P_ChangeSwitchTexture(line,0);
        break;

    case 103:
        // Open Door - [STRIFE] Verified unmodified
        if (EV_DoDoor(line,vld_open))
            P_ChangeSwitchTexture(line,0);
        break;

    case 111:
        // Blazing Door Raise (faster than TURBO!) - [STRIFE] Verified unmodified
        if (EV_DoDoor (line,vld_blazeRaise))
            P_ChangeSwitchTexture(line,0);
        break;

    case 112:
        // Blazing Door Open (faster than TURBO!) - [STRIFE] Verified unmodified
        if (EV_DoDoor (line,vld_blazeOpen))
            P_ChangeSwitchTexture(line,0);
        break;

    case 113:
        // Blazing Door Close (faster than TURBO!) - [STRIFE] Verified unmodified
        if (EV_DoDoor (line,vld_blazeClose))
            P_ChangeSwitchTexture(line,0);
        break;
    
    case 114:
        // Blazing Door Raise (faster than TURBO!) - [STRIFE] Verified unmodified
        if (EV_DoDoor (line,vld_blazeRaise))
            P_ChangeSwitchTexture(line,1);
        break;

    case 115:
        // Blazing Door Open (faster than TURBO!) - [STRIFE] Verified unmodified
        if (EV_DoDoor (line,vld_blazeOpen))
            P_ChangeSwitchTexture(line,1);
        break;

    case 116:
        // Blazing Door Close (faster than TURBO!) - [STRIFE] Verified unmodified
        if (EV_DoDoor (line,vld_blazeClose))
            P_ChangeSwitchTexture(line,1);
        break;

    case 122:
        // Blazing PlatDownWaitUpStay - [STRIFE] Verified unmodified
        if(EV_DoPlat(line, blazeDWUS, 0))
            P_ChangeSwitchTexture(line,0);
        break;

    case 123:
        // Blazing PlatDownWaitUpStay - [STRIFE] Verified unmodified
        if(EV_DoPlat(line, blazeDWUS, 0))
            P_ChangeSwitchTexture(line,1);
        break;

    case 127:
        // Build Stairs Turbo 16 - [STRIFE] Verified unmodified
        if (EV_BuildStairs(line,turbo16))
            P_ChangeSwitchTexture(line,0);
        break;

    case 131:
        // Raise Floor Turbo - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloorTurbo))
            P_ChangeSwitchTexture(line,0);
        break;
    
    case 132:
        // Raise Floor Turbo - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloorTurbo))
            P_ChangeSwitchTexture(line,1);
        break;

    case 133: // [STRIFE] TODO - which key is it?
    case 135: // [STRIFE] TODO - which key is it?
    case 137: // [STRIFE] TODO - which key is it?
        if (EV_DoLockedDoor (line,vld_blazeOpen,thing))
            P_ChangeSwitchTexture(line,0);
        break;

    case 99:  // [STRIFE] TODO: which key is it?
    case 134: // [STRIFE] TODO: which key is it?
    case 136: // [STRIFE] TODO: which key is it?
        if (EV_DoLockedDoor (line,vld_blazeOpen,thing))
            P_ChangeSwitchTexture(line,1);
        break;

    case 138:
        // Light Turn On - [STRIFE] Verified unmodified
        EV_LightTurnOn(line,255);
        P_ChangeSwitchTexture(line,1);
        break;

    case 139:
        // Light Turn Off - [STRIFE] Verified unmodified
        EV_LightTurnOn(line,35);
        P_ChangeSwitchTexture(line,1);
        break;

    case 140:
        // Raise Floor 512 - [STRIFE] Verified unmodified
        if (EV_DoFloor(line,raiseFloor512))
            P_ChangeSwitchTexture(line,0);
        break;

    case 144:
        // villsa [STRIFE] manual sliding door
        EV_SlidingDoor(line, thing);
        break;

    case 146:
        // haleyjd 09/24/10: [STRIFE] S1 Build Stairs Down 16 (new type)
        if(EV_BuildStairs(line, buildDown16))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 147:
        // haleyjd 09/24/10: [STRIFE] S1 Clear Force Fields
        if(EV_ClearForceFields(line))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 148:
        // haleyjd 09/16/10: [STRIFE] using forcefields hurts
        P_DamageMobj(thing, NULL, NULL, 16);
        P_Thrust(thing->player, thing->angle + ANG180, 125*FRACUNIT/16);
        break;

    case 151: // villsa [STRIFE] BlzOpenDoor Gold key
    case 152: // [STRIFE] TODO: which key is it?
    case 153: // [STRIFE] TODO: which key is it?
        if(EV_DoLockedDoor(line, vld_blazeOpen, thing))
            P_ChangeSwitchTexture(line, 1);
        break;

    case 154:
        // villsa [STRIFE] plat lower wait rise if have gold key
        if(thing->player->cards[key_GoldKey])
        {
            if(EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, 1);
        }
        else
        {
            thing->player->message = DEH_String("You need a gold key");
            S_StartSound(thing, sfx_oof);
        }
        break;

    case 155:
        // villsa [STRIFE] raise plat wait lower
        if(EV_DoPlat(line, upWaitDownStay, 0))
            P_ChangeSwitchTexture(line, 1);
        break;

    case 162: // [STRIFE] TODO: which key is it?
    case 163: // [STRIFE] TODO: which key is it?
    case 164: // villsa [STRIFE] BlzOpenDoor Gold key
    case 167: // [STRIFE] TODO: which key is it?
        if(EV_DoLockedDoor(line, vld_blazeOpen, thing))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 168: // [STRIFE] TODO: which key is it?
        // haleyjd 09/25/10: [STRIFE] SR Blaze Open Door ???? Key
        if(EV_DoLockedDoor(line, vld_blazeOpen, thing))
            P_ChangeSwitchTexture(line, 1);
        break;

    case 171: // [STRIFE] TODO: which key is it?
        // haleyjd 09/25/10: [STRIFE] S1 Open Door ???? Key
        if(EV_DoLockedDoor(line, vld_open, thing))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 172: // [STRIFE] TODO: which key is it?
    case 173: // [STRIFE] TODO: which key is it?
    case 176: // [STRIFE] TODO: which key is it?
    case 191: // [STRIFE] TODO: which key is it?
    case 192: // [STRIFE] TODO: which key is it?
    case 223: // [STRIFE] TODO: which key is it?
        if(EV_DoLockedDoor(line, vld_normal, thing))
            P_ChangeSwitchTexture(line, 1);
        break;

    case 177:
        // villsa [STRIFE] plat lower wait rise if have power3 key
        if(thing->player->cards[key_Power3Key])
        {
            if(EV_DoPlat(line, downWaitUpStay, 0))
                P_ChangeSwitchTexture(line, 1);
        }
        else
        {
            thing->player->message = DEH_String("You don't have the key");
            S_StartSound(thing, sfx_oof);
        }
        break;

    case 181:
        // haleyjd 09/25/10: [STRIFE] S1 Floor Raise 512 & Change
        if(EV_DoFloor(line, raiseFloor512AndChange))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 189: // [STRIFE] TODO: which key is it???
        // haleyjd 09/25/10: [STRIFE] S1 Split Open Door ???? Key
        if(EV_DoLockedDoor(line, vld_splitOpen, thing))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 194:
        // villsa [STRIFE] S1 Free Prisoners
        if(EV_DoDoor(line, vld_open))
        {
            P_ChangeSwitchTexture(line, 0);
            P_FreePrisoners();
        }
        break;

    case 199:
        // haleyjd 09/25/10: [STRIFE] S1 Destroy Converter
        if(EV_DoCeiling(line, lowerAndCrush))
        {
            P_ChangeSwitchTexture(line, 0);
            P_DestroyConverter();
        }
        break;

    case 207:
        // villsa [STRIFE] SR Remote Sliding Door
        if(EV_RemoteSlidingDoor(line, thing))
            P_ChangeSwitchTexture(line, 1);
        break; // haleyjd

    case 209:
        // haleyjd 09/24/10: [STRIFE] S1 Build Stairs Down 16 if Have Chalice
        if(!P_PlayerHasItem(thing->player, MT_INV_CHALICE))
        {
            DEH_snprintf(usemessage, sizeof(usemessage), "You need the chalice!");
            thing->player->message = usemessage;
            S_StartSound(thing, sfx_oof);
            break;
        }
        else if(EV_BuildStairs(line, buildDown16))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 211:
        // villsa [STRIFE] S1 Play VOC## sound
        if(&players[consoleplayer] == thing->player &&
            thing->player->powers[pw_communicator])
        {
            DEH_snprintf(usemessage, sizeof(usemessage), "voc%i", line->tag);
            I_StartVoice(usemessage);
            line->special = 0;
        }
        break;

    case 214:
        // villsa [STRIFE] S1 slow lift lower wait up stay
        if(EV_DoPlat(line, slowDWUS, 1))
            P_ChangeSwitchTexture(line, 1);
        break;

    case 219:
        // haleyjd 09/25/10: S1 Lower Floor Blue Crystal
        if(!thing->player->cards[key_BlueCrystalKey])
        {
            thing->player->message = DEH_String("You need the Blue Crystal");
            S_StartSound(thing, sfx_oof);
        }
        else if(EV_DoFloor(line, lowerFloor))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 220:
        // haleyjd 09/25/10: S1 Lower Floor Red Crystal
        if(!thing->player->cards[key_RedCrystalKey])
        {
            thing->player->message = DEH_String("You need the Red Crystal");
            S_StartSound(thing, sfx_oof);
        }
        else if(EV_DoFloor(line, lowerFloor))
            P_ChangeSwitchTexture(line, 0);
        break;

    case 226:
        // villsa [STRIFE] S1 Complete Training Area
        if(EV_DoFloor(line, lowerFloor))
        {
            P_GiveItemToPlayer(thing->player, SPR_TOKN, MT_TOKEN_STAMINA);
            P_GiveItemToPlayer(thing->player, SPR_TOKN, MT_TOKEN_NEW_ACCURACY);
            P_ChangeSwitchTexture(line, 0);
            DEH_snprintf(usemessage, sizeof(usemessage),
                DEH_String("Congratulations! You have completed the training area."));
            thing->player->message = usemessage;
        }
        break;

    case 229:
        // villsa [STRIFE] SR Sigil Sliding Door
        if(thing->player->sigiltype == 4)
        {
            if(EV_RemoteSlidingDoor(line, thing))
                P_ChangeSwitchTexture(line, 1);
        }
        break; // haleyjd

    case 233:
        // villsa [STRIFE] objective given after revealing the computer
        if(!EV_DoDoor(line, vld_splitOpen))
            return true;

        P_ChangeSwitchTexture(line, 1);
        GiveVoiceObjective("voc70", "log70", 0);
        
        // haleyjd: Strife used sprintf here, not a direct set.
        DEH_snprintf(usemessage, sizeof(usemessage), 
                     "Incoming Message from BlackBird...");
        thing->player->message = usemessage;

        break;

    case 234:
        // haleyjd 09/24/10: [STRIFE] SR Raise Door if Quest 3
        if(!(thing->player->questflags & QF_QUEST3)) // QUEST3 == Irale
        {
            // BUG: doesn't make sfx_oof sound like all other message-
            // giving door types. I highly doubt this was intentional.
            DEH_snprintf(usemessage, sizeof(usemessage), 
                         "That doesn't seem to work!");
            thing->player->message = usemessage;
        }
        else if(EV_DoDoor(line, vld_normal))
            P_ChangeSwitchTexture(line, 1);
        break;

    case 235:
        // haleyjd 09/25/10: [STRIFE] S1 Split Open Door if Have Sigil 4
        if(thing->player->sigiltype == 4)
        {
            if(EV_DoDoor(line, vld_splitOpen))
                P_ChangeSwitchTexture(line, 0);
        }
        break;

    case 666:
        // villsa [STRIFE] SR Move Wall
        P_MoveWall(line, thing);
        break;
    }

    return true;
}

