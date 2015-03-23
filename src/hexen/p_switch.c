//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


#include "h2def.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"

//==================================================================
//
//      CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
//==================================================================
switchlist_t alphSwitchListDemo[] = {
    {"SW_1_UP", "SW_1_DN", SFX_SWITCH1},
    {"SW_2_UP", "SW_2_DN", SFX_SWITCH1},
    {"SW52_OFF", "SW52_ON", SFX_SWITCH2},
    {"\0", "\0", 0}
};

switchlist_t alphSwitchListFull[] = {
    {"SW_1_UP", "SW_1_DN", SFX_SWITCH1},
    {"SW_2_UP", "SW_2_DN", SFX_SWITCH1},
    {"VALVE1", "VALVE2", SFX_VALVE_TURN},
    {"SW51_OFF", "SW51_ON", SFX_SWITCH2},
    {"SW52_OFF", "SW52_ON", SFX_SWITCH2},
    {"SW53_UP", "SW53_DN", SFX_ROPE_PULL},
    {"PUZZLE5", "PUZZLE9", SFX_SWITCH1},
    {"PUZZLE6", "PUZZLE10", SFX_SWITCH1},
    {"PUZZLE7", "PUZZLE11", SFX_SWITCH1},
    {"PUZZLE8", "PUZZLE12", SFX_SWITCH1},
    {"\0", "\0", 0}
};

switchlist_t *alphSwitchList = NULL;

int switchlist[MAXSWITCHES * 2];
int numswitches;
button_t buttonlist[MAXBUTTONS];

/*
===============
=
= P_InitSwitchList
=
= Only called at game initialization
=
===============
*/

void P_InitSwitchList(void)
{
    int i;
    int index;

    if (!alphSwitchList)
    {
        if (gamemode == shareware)
        {
            alphSwitchList = alphSwitchListDemo;
        }
        else
        {
            alphSwitchList = alphSwitchListFull;
        }
    }

    for (index = 0, i = 0; i < MAXSWITCHES; i++)
    {
        if (!alphSwitchList[i].soundID)
        {
            numswitches = index / 2;
            switchlist[index] = -1;
            break;
        }
        switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name1);
        switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name2);
    }
}

//==================================================================
//
//      Start a button counting down till it turns off.
//
//==================================================================
void P_StartButton(line_t * line, bwhere_e w, int texture, int time)
{
    int i;

    for (i = 0; i < MAXBUTTONS; i++)
    {
        if (!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = (mobj_t *) & line->frontsector->soundorg;
            return;
        }
    }
    I_Error("P_StartButton: no button slots left!");
}

//==================================================================
//
//      Function that changes wall texture.
//      Tell it if switch is ok to use again (1=yes, it's a button).
//
//==================================================================
void P_ChangeSwitchTexture(line_t * line, int useAgain)
{
    int texTop;
    int texMid;
    int texBot;
    int i;

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    for (i = 0; i < numswitches * 2; i++)
    {
        if (switchlist[i] == texTop)
        {
            S_StartSound((mobj_t *) & line->frontsector->soundorg,
                         alphSwitchList[i / 2].soundID);
            sides[line->sidenum[0]].toptexture = switchlist[i ^ 1];
            if (useAgain)
            {
                P_StartButton(line, SWTCH_TOP, switchlist[i], BUTTONTIME);
            }
            return;
        }
        else if (switchlist[i] == texMid)
        {
            S_StartSound((mobj_t *) & line->frontsector->soundorg,
                         alphSwitchList[i / 2].soundID);
            sides[line->sidenum[0]].midtexture = switchlist[i ^ 1];
            if (useAgain)
            {
                P_StartButton(line, SWTCH_MIDDLE, switchlist[i], BUTTONTIME);
            }
            return;
        }
        else if (switchlist[i] == texBot)
        {
            S_StartSound((mobj_t *) & line->frontsector->soundorg,
                         alphSwitchList[i / 2].soundID);
            sides[line->sidenum[0]].bottomtexture = switchlist[i ^ 1];
            if (useAgain)
            {
                P_StartButton(line, SWTCH_BOTTOM, switchlist[i], BUTTONTIME);
            }
            return;
        }
    }
}
