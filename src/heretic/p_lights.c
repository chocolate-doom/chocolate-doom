// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2008 Simon Howard
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
#include "m_random.h"
#include "p_local.h"
#include "v_video.h"

//==================================================================
//==================================================================
//
//                                                      BROKEN LIGHT FLASHING
//
//==================================================================
//==================================================================

//==================================================================
//
//      T_LightFlash
//
//      After the map has been loaded, scan each sector for specials
//      that spawn thinkers
//
//==================================================================
void T_LightFlash(lightflash_t * flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->maxlight)
    {
        flash->sector->lightlevel = flash->minlight;
        flash->count = (P_Random() & flash->mintime) + 1;
    }
    else
    {
        flash->sector->lightlevel = flash->maxlight;
        flash->count = (P_Random() & flash->maxtime) + 1;
    }

}


//==================================================================
//
//      P_SpawnLightFlash
//
//      After the map has been loaded, scan each sector for specials that spawn thinkers
//
//==================================================================
void P_SpawnLightFlash(sector_t * sector)
{
    lightflash_t *flash;

    sector->special = 0;        // nothing special about it during gameplay

    flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);
    P_AddThinker(&flash->thinker);
    flash->thinker.function = T_LightFlash;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;

    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    flash->maxtime = 64;
    flash->mintime = 7;
    flash->count = (P_Random() & flash->maxtime) + 1;
}

//==================================================================
//
//                                                      STROBE LIGHT FLASHING
//
//==================================================================

//==================================================================
//
//      T_StrobeFlash
//
//      After the map has been loaded, scan each sector for specials that spawn thinkers
//
//==================================================================
void T_StrobeFlash(strobe_t * flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->minlight)
    {
        flash->sector->lightlevel = flash->maxlight;
        flash->count = flash->brighttime;
    }
    else
    {
        flash->sector->lightlevel = flash->minlight;
        flash->count = flash->darktime;
    }

}

//==================================================================
//
//      P_SpawnLightFlash
//
//      After the map has been loaded, scan each sector for specials that spawn thinkers
//
//==================================================================
void P_SpawnStrobeFlash(sector_t * sector, int fastOrSlow, int inSync)
{
    strobe_t *flash;

    flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);
    P_AddThinker(&flash->thinker);
    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->thinker.function = T_StrobeFlash;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

    if (flash->minlight == flash->maxlight)
        flash->minlight = 0;
    sector->special = 0;        // nothing special about it during gameplay

    if (!inSync)
        flash->count = (P_Random() & 7) + 1;
    else
        flash->count = 1;
}

//==================================================================
//
//      Start strobing lights (usually from a trigger)
//
//==================================================================
void EV_StartLightStrobing(line_t * line)
{
    int secnum;
    sector_t *sec;

    secnum = -1;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        P_SpawnStrobeFlash(sec, SLOWDARK, 0);
    }
}

//==================================================================
//
//      TURN LINE'S TAG LIGHTS OFF
//
//==================================================================
void EV_TurnTagLightsOff(line_t * line)
{
    int i;
    int j;
    int min;
    sector_t *sector;
    sector_t *tsec;
    line_t *templine;

    sector = sectors;
    for (j = 0; j < numsectors; j++, sector++)
        if (sector->tag == line->tag)
        {
            min = sector->lightlevel;
            for (i = 0; i < sector->linecount; i++)
            {
                templine = sector->lines[i];
                tsec = getNextSector(templine, sector);
                if (!tsec)
                    continue;
                if (tsec->lightlevel < min)
                    min = tsec->lightlevel;
            }
            sector->lightlevel = min;
        }
}

//==================================================================
//
//      TURN LINE'S TAG LIGHTS ON
//
//==================================================================
void EV_LightTurnOn(line_t * line, int bright)
{
    int i;
    int j;
    sector_t *sector;
    sector_t *temp;
    line_t *templine;

    sector = sectors;

    for (i = 0; i < numsectors; i++, sector++)
        if (sector->tag == line->tag)
        {
            //
            // bright = 0 means to search for highest
            // light level surrounding sector
            //
            if (!bright)
            {
                for (j = 0; j < sector->linecount; j++)
                {
                    templine = sector->lines[j];
                    temp = getNextSector(templine, sector);
                    if (!temp)
                        continue;
                    if (temp->lightlevel > bright)
                        bright = temp->lightlevel;
                }
            }
            sector->lightlevel = bright;
        }
}

//==================================================================
//
//      Spawn glowing light
//
//==================================================================
void T_Glow(glow_t * g)
{
    switch (g->direction)
    {
        case -1:               // DOWN
            g->sector->lightlevel -= GLOWSPEED;
            if (g->sector->lightlevel <= g->minlight)
            {
                g->sector->lightlevel += GLOWSPEED;
                g->direction = 1;
            }
            break;
        case 1:                // UP
            g->sector->lightlevel += GLOWSPEED;
            if (g->sector->lightlevel >= g->maxlight)
            {
                g->sector->lightlevel -= GLOWSPEED;
                g->direction = -1;
            }
            break;
    }
}

void P_SpawnGlowingLight(sector_t * sector)
{
    glow_t *g;

    g = Z_Malloc(sizeof(*g), PU_LEVSPEC, 0);
    P_AddThinker(&g->thinker);
    g->sector = sector;
    g->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    g->maxlight = sector->lightlevel;
    g->thinker.function = T_Glow;
    g->direction = -1;

    sector->special = 0;
}
