// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

//==================================================================
//==================================================================
//
//                                                      CEILINGS
//
//==================================================================
//==================================================================

ceiling_t *activeceilings[MAXCEILINGS];

//==================================================================
//
//      T_MoveCeiling
//
//==================================================================
void T_MoveCeiling(ceiling_t * ceiling)
{
    result_e res;

    switch (ceiling->direction)
    {
        case 0:                // IN STASIS
            break;
        case 1:                // UP
            res = T_MovePlane(ceiling->sector, ceiling->speed,
                              ceiling->topheight, false, 1,
                              ceiling->direction);
            if (!(leveltime & 7))
                S_StartSound((mobj_t *) & ceiling->sector->soundorg,
                             sfx_dormov);
            if (res == pastdest)
                switch (ceiling->type)
                {
                    case raiseToHighest:
                        P_RemoveActiveCeiling(ceiling);
                        break;
                    case fastCrushAndRaise:
                    case crushAndRaise:
                        ceiling->direction = -1;
                        break;
                    default:
                        break;
                }
            break;
        case -1:               // DOWN
            res = T_MovePlane(ceiling->sector, ceiling->speed,
                              ceiling->bottomheight, ceiling->crush, 1,
                              ceiling->direction);
            if (!(leveltime & 7))
                S_StartSound((mobj_t *) & ceiling->sector->soundorg,
                             sfx_dormov);
            if (res == pastdest)
                switch (ceiling->type)
                {
                    case crushAndRaise:
                        ceiling->speed = CEILSPEED;
                    case fastCrushAndRaise:
                        ceiling->direction = 1;
                        break;
                    case lowerAndCrush:
                    case lowerToFloor:
                        P_RemoveActiveCeiling(ceiling);
                        break;
                    default:
                        break;
                }
            else if (res == crushed)
                switch (ceiling->type)
                {
                    case crushAndRaise:
                    case lowerAndCrush:
                        ceiling->speed = CEILSPEED / 8;
                        break;
                    default:
                        break;
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
int EV_DoCeiling(line_t * line, ceiling_e type)
{
    int secnum, rtn;
    sector_t *sec;
    ceiling_t *ceiling;

    secnum = -1;
    rtn = 0;

    //
    //      Reactivate in-stasis ceilings...for certain types.
    //
    switch (type)
    {
        case fastCrushAndRaise:
        case crushAndRaise:
            P_ActivateInStasisCeiling(line);
        default:
            break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        //
        // new door thinker
        //
        rtn = 1;
        ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, 0);
        P_AddThinker(&ceiling->thinker);
        sec->specialdata = ceiling;
        ceiling->thinker.function = T_MoveCeiling;
        ceiling->sector = sec;
        ceiling->crush = false;
        switch (type)
        {
            case fastCrushAndRaise:
                ceiling->crush = true;
                ceiling->topheight = sec->ceilingheight;
                ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED * 2;
                break;
            case crushAndRaise:
                ceiling->crush = true;
                ceiling->topheight = sec->ceilingheight;
            case lowerAndCrush:
            case lowerToFloor:
                ceiling->bottomheight = sec->floorheight;
                if (type != lowerToFloor)
                    ceiling->bottomheight += 8 * FRACUNIT;
                ceiling->direction = -1;
                ceiling->speed = CEILSPEED;
                break;
            case raiseToHighest:
                ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
                ceiling->direction = 1;
                ceiling->speed = CEILSPEED;
                break;
        }

        ceiling->tag = sec->tag;
        ceiling->type = type;
        P_AddActiveCeiling(ceiling);
    }
    return rtn;
}

//==================================================================
//
//              Add an active ceiling
//
//==================================================================
void P_AddActiveCeiling(ceiling_t * c)
{
    int i;
    for (i = 0; i < MAXCEILINGS; i++)
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
void P_RemoveActiveCeiling(ceiling_t * c)
{
    int i;

    for (i = 0; i < MAXCEILINGS; i++)
        if (activeceilings[i] == c)
        {
            activeceilings[i]->sector->specialdata = NULL;
            P_RemoveThinker(&activeceilings[i]->thinker);
            activeceilings[i] = NULL;
            break;
        }
}

//==================================================================
//
//              Restart a ceiling that's in-stasis
//
//==================================================================
void P_ActivateInStasisCeiling(line_t * line)
{
    int i;

    for (i = 0; i < MAXCEILINGS; i++)
        if (activeceilings[i] && (activeceilings[i]->tag == line->tag) &&
            (activeceilings[i]->direction == 0))
        {
            activeceilings[i]->direction = activeceilings[i]->olddirection;
            activeceilings[i]->thinker.function = T_MoveCeiling;
        }
}

//==================================================================
//
//              EV_CeilingCrushStop
//              Stop a ceiling from crushing!
//
//==================================================================
int EV_CeilingCrushStop(line_t * line)
{
    int i;
    int rtn;

    rtn = 0;
    for (i = 0; i < MAXCEILINGS; i++)
        if (activeceilings[i] && (activeceilings[i]->tag == line->tag) &&
            (activeceilings[i]->direction != 0))
        {
            activeceilings[i]->olddirection = activeceilings[i]->direction;
            activeceilings[i]->thinker.function = NULL;
            activeceilings[i]->direction = 0;   // in-stasis
            rtn = 1;
        }

    return rtn;
}
