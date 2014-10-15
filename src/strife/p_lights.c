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
//	Handle Sector base lighting effects.
//	Muzzle flash?
//



#include "z_zone.h"
#include "m_random.h"

#include "doomdef.h"
#include "p_local.h"


// State.
#include "r_state.h"

//
// FIRELIGHT FLICKER
//

//
// T_FireFlicker
//
// [STRIFE] 
// haleyjd 2011023: Changes to amount and duration of flicker
//
void T_FireFlicker (fireflicker_t* flick)
{
    int	amount;

    if (--flick->count)
        return;

    amount = (P_Random() & 3) * 8; // [STRIFE] 16 -> 8

    if (flick->sector->lightlevel - amount < flick->minlight)
        flick->sector->lightlevel = flick->minlight;
    else
        flick->sector->lightlevel = flick->maxlight - amount;

    // [STRIFE] flicker count made random!
    flick->count = (P_Random() & 3) + 1;
}



//
// P_SpawnFireFlicker
//
// [STRIFE] 
// haleyjd 2011023: Changes to minimum light level and initial duration
//
void P_SpawnFireFlicker (sector_t*      sector)
{
    fireflicker_t*      flick;

    // Note that we are resetting sector attributes.
    // Nothing special about it during gameplay.
    sector->special = 0; 

    flick = Z_Malloc ( sizeof(*flick), PU_LEVSPEC, 0);

    P_AddThinker (&flick->thinker);

    flick->thinker.function.acp1 = (actionf_p1) T_FireFlicker;
    flick->sector = sector;
    flick->maxlight = sector->lightlevel;
    flick->minlight = sector->lightlevel - 32; // [STRIFE] changed from min surrounding+16
    flick->count = 2;                          // [STRIFE]: Initial count 4 -> 2
}



//
// BROKEN LIGHT FLASHING
//


//
// T_LightFlash
// Do flashing lights.
//
// [STRIFE] Verified unmodified
//
void T_LightFlash (lightflash_t* flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->maxlight)
    {
        flash->sector->lightlevel = flash->minlight;
        flash->count = (P_Random()&flash->mintime)+1;
    }
    else
    {
        flash->sector->lightlevel = flash->maxlight;
        flash->count = (P_Random()&flash->maxtime)+1;
    }
}




//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
// [STRIFE] Verified unmodified
//
void P_SpawnLightFlash (sector_t*	sector)
{
    lightflash_t*	flash;

    // nothing special about it during gameplay
    sector->special = 0;

    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);

    P_AddThinker (&flash->thinker);

    flash->thinker.function.acp1 = (actionf_p1) T_LightFlash;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;

    flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    flash->maxtime = 64;
    flash->mintime = 7;
    flash->count = (P_Random()&flash->maxtime)+1;
}



//
// STROBE LIGHT FLASHING
//


//
// T_StrobeFlash
//
// [STRIFE] Verified unmodified
//
void T_StrobeFlash (strobe_t*           flash)
{
    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->minlight)
    {
        flash-> sector->lightlevel = flash->maxlight;
        flash->count = flash->brighttime;
    }
    else
    {
        flash-> sector->lightlevel = flash->minlight;
        flash->count =flash->darktime;
    }
}



//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
// [STRIFE] Verified unmodified
//
void
P_SpawnStrobeFlash
( sector_t*     sector,
  int           fastOrSlow,
  int           inSync )
{
    strobe_t*   flash;

    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);

    P_AddThinker (&flash->thinker);

    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->thinker.function.acp1 = (actionf_p1) T_StrobeFlash;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

    if (flash->minlight == flash->maxlight)
        flash->minlight = 0;

    // nothing special about it during gameplay
    sector->special = 0;

    if (!inSync)
        flash->count = (P_Random()&7)+1;
    else
        flash->count = 1;
}


//
// Start strobing lights (usually from a trigger)
//
// [STRIFE] Verified unmodified
//
void EV_StartLightStrobing(line_t*      line)
{
    int         secnum;
    sector_t*   sec;

    secnum = -1;
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        P_SpawnStrobeFlash (sec, SLOWDARK, 0);
    }
}



//
// TURN LINE'S TAG LIGHTS OFF
//
// [STRIFE] Verified unmodified
//
void EV_TurnTagLightsOff(line_t* line)
{
    int             i;
    int             j;
    int             min;
    sector_t*       sector;
    sector_t*       tsec;
    line_t*         templine;

    sector = sectors;

    for (j = 0;j < numsectors; j++, sector++)
    {
        if (sector->tag == line->tag)
        {
            min = sector->lightlevel;
            for (i = 0;i < sector->linecount; i++)
            {
                templine = sector->lines[i];
                tsec = getNextSector(templine,sector);
                if (!tsec)
                    continue;
                if (tsec->lightlevel < min)
                    min = tsec->lightlevel;
            }
            sector->lightlevel = min;
        }
    }
}


//
// TURN LINE'S TAG LIGHTS ON
//
// [STRIFE] Verified unmodified
//
void
EV_LightTurnOn
( line_t*       line,
  int           bright )
{
    int         i;
    int         j;
    sector_t*   sector;
    sector_t*   temp;
    line_t*     templine;

    sector = sectors;

    for (i=0;i<numsectors;i++, sector++)
    {
        if (sector->tag == line->tag)
        {
            // bright = 0 means to search
            // for highest light level
            // surrounding sector
            if (!bright)
            {
                for (j = 0;j < sector->linecount; j++)
                {
                    templine = sector->lines[j];
                    temp = getNextSector(templine,sector);

                    if (!temp)
                        continue;

                    if (temp->lightlevel > bright)
                        bright = temp->lightlevel;
                }
            }
            sector-> lightlevel = bright;
        }
    }
}

    
//
// Spawn glowing light
//
// [STRIFE] Verified unmodified
//
void T_Glow(glow_t*     g)
{
    switch(g->direction)
    {
    case -1:
        // DOWN
        g->sector->lightlevel -= GLOWSPEED;
        if (g->sector->lightlevel <= g->minlight)
        {
            g->sector->lightlevel += GLOWSPEED;
            g->direction = 1;
        }
        break;

    case 1:
        // UP
        g->sector->lightlevel += GLOWSPEED;
        if (g->sector->lightlevel >= g->maxlight)
        {
            g->sector->lightlevel -= GLOWSPEED;
            g->direction = -1;
        }
        break;
    }
}

//
// [STRIFE] Verified unmodified
//
void P_SpawnGlowingLight(sector_t*	sector)
{
    glow_t*	g;

    g = Z_Malloc( sizeof(*g), PU_LEVSPEC, 0);

    P_AddThinker(&g->thinker);

    g->sector = sector;
    g->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    g->maxlight = sector->lightlevel;
    g->thinker.function.acp1 = (actionf_p1) T_Glow;
    g->direction = -1;

    sector->special = 0;
}

