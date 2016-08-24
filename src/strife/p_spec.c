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
//	Implements special effects:
//	Texture animation, height or lighting changes
//	 according to adjacent sectors, respective
//	 utility functions, etc.
//	Line Tag handling. Line and Sector triggers.
//


#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"

#include "deh_main.h"
#include "i_system.h"
#include "z_zone.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_random.h"
#include "w_wad.h"

#include "r_local.h"
#include "p_local.h"

#include "g_game.h"

#include "s_sound.h"

// State.
#include "r_state.h"

// Data.
#include "sounds.h"

// [STRIFE]
#include "hu_stuff.h"
#include "p_dialog.h"


//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
    boolean	istexture;
    int		picnum;
    int		basepic;
    int		numpics;
    int		speed;
    
} anim_t;

//
//      source animation definition
//
typedef struct
{
    int 	istexture;	// if false, it is a flat
    char	endname[9];
    char	startname[9];
    int		speed;
} animdef_t;


// haleyjd 08/30/10: [STRIFE] MAXANIMS raised from 32 to 40
#define MAXANIMS                40

//
// P_InitPicAnims
//

// Floor/ceiling animation sequences,
//  defined by first and last frame,
//  i.e. the flat (64x64 tile) name to
//  be used.
// The full animation sequence is given
//  using all the flats between the start
//  and end entry, in the order found in
//  the WAD file.
//
// haleyjd 08/29/10: [STRIFE] Changed animdefs.
//
animdef_t               animdefs[] =
{
    { false, "F_SCANR8", "F_SCANR5",  4},
    { false, "F_WATR03", "F_WATR01",  8},
    { false, "F_PWATR3", "F_PWATR1", 11},
    { false, "F_SCANR4", "F_SCANR1",  4},
    { true,  "SCAN08",   "SCAN05",    4},
    { true,  "SWTRMG03", "SWTRMG01",  4},
    { true,  "SCAN04",   "SCAN01",    4},
    { true,  "COMP04",   "COMP01",    4},
    { true,  "COMP08",   "COMP05",    6},
    { true,  "COMP12",   "COMP09",   11},
    { true,  "COMP16",   "COMP13",   12},
    { true,  "COMP20",   "COMP17",   12},
    { true,  "COMP24",   "COMP21",   12},
    { true,  "COMP28",   "COMP25",   12},
    { true,  "COMP32",   "COMP29",   12},
    { true,  "COMP37",   "COMP33",   12},
    { true,  "COMP41",   "COMP38",   12},
    { true,  "COMP49",   "COMP42",   10},
    { true,  "BRKGRY16", "BRKGRY13", 10},
    { true,  "BRNSCN04", "BRNSCN01", 10},
    { true,  "CONCRT12", "CONCRT09", 11},
    { true,  "CONCRT25", "CONCRT22", 11},
    { true,  "WALPMP02", "WALPMP01", 16},
    { true,  "WALTEK17", "WALTEK16",  8},
    { true,  "FORCE04",  "FORCE01",   4},
    { true,  "FORCE08",  "FORCE05",   4},
    { true,  "FAN02",    "FAN01",     4},
    { false, "F_VWATR3", "P_VWATR1",  4},
    { false, "F_HWATR3", "F_HWATR1",  4},
    { false, "F_TELE2",  "F_TELE1",   4},
    { false, "F_FAN2",   "F_FAN1",    4},
    { false, "F_CONVY2", "F_CONVY1",  4},
    { false, "F_RDALN4", "F_RDALN1",  4},
    { -1,    "",         "",          0},
};

anim_t  anims[MAXANIMS];
anim_t* lastanim;

//
//      Animating line specials
//
// haleyjd 08/29/10: [STRIFE] MAXLINEANIMS raised from 64 to 96
#define MAXLINEANIMS            96

extern  short   numlinespecials;
extern  line_t* linespeciallist[MAXLINEANIMS];



void P_InitPicAnims (void)
{
    int		i;

    
    //	Init animation
    lastanim = anims;
    for (i=0 ; animdefs[i].istexture != -1 ; i++)
    {
        char *startname, *endname;

        startname = DEH_String(animdefs[i].startname);
        endname = DEH_String(animdefs[i].endname);

        if (animdefs[i].istexture)
        {
            // different episode ?
            if (R_CheckTextureNumForName(startname) == -1)
                continue;	

            lastanim->picnum = R_TextureNumForName(endname);
            lastanim->basepic = R_TextureNumForName(startname);
        }
        else
        {
            if (W_CheckNumForName(startname) == -1)
                continue;

            lastanim->picnum = R_FlatNumForName(endname);
            lastanim->basepic = R_FlatNumForName(startname);
        }

        lastanim->istexture = animdefs[i].istexture;
        lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;

        if (lastanim->numpics < 2)
            I_Error ("P_InitPicAnims: bad cycle from %s to %s",
            startname, endname);

        lastanim->speed = animdefs[i].speed;
        lastanim++;
    }

}

// villsa [STRIFE] terrain type definitions
typedef struct
{
    char*   flat;
    int     type;
    int     num;
} terraintype_t;

terraintype_t   terraintypes[] =
{
    {   "F_WATR03", FLOOR_WATER,    -1  },
    {   "F_WATR02", FLOOR_WATER,    -1  },
    {   "F_WATR01", FLOOR_WATER,    -1  },
    {   "F_VWATR3", FLOOR_WATER,    -1  },
    {   "F_VWATR2", FLOOR_WATER,    -1  },
    {   "P_VWATR1", FLOOR_WATER,    -1  },
    {   "F_HWATR3", FLOOR_WATER,    -1  },
    {   "F_HWATR2", FLOOR_WATER,    -1  },
    {   "F_HWATR1", FLOOR_WATER,    -1  },
    {   "F_PWATR3", FLOOR_SLIME,    -1  },
    {   "F_PWATR2", FLOOR_SLIME,    -1  },
    {   "F_PWATR1", FLOOR_SLIME,    -1  },
    {   "END",      FLOOR_END,      -1  },
};

//
// P_GetTerrainType
// villsa [STRIFE] new function
//

terraintype_e P_GetTerrainType(mobj_t* mobj)
{
    int i = 0;
    subsector_t* ss = mobj->subsector;

    if(mobj->z <= ss->sector->floorheight &&
        terraintypes[0].type != FLOOR_END)
    {
        while(ss->sector->floorpic != terraintypes[i].num)
        {
            if(terraintypes[i+1].type == FLOOR_END)
                return FLOOR_SOLID;

            i++;
        }

        return terraintypes[i].type;
    }

    return FLOOR_SOLID;
}

//
// P_InitTerrainTypes
// villsa [STRIFE] new function
// Initialize terrain types
//

void P_InitTerrainTypes(void)
{
    int i = 0;

    if(terraintypes[0].type != FLOOR_END)
    {
        while(terraintypes[i].type != FLOOR_END)
        {
            terraintypes[i].num = R_FlatNumForName(terraintypes[i].flat);
            i++;
        }
    }
}



//
// UTILITIES
//



//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t*
getSide
( int		currentSector,
  int		line,
  int		side )
{
    return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}


//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t*
getSector
( int		currentSector,
  int		line,
  int		side )
{
    return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}


//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
int
twoSided
( int	sector,
  int	line )
{
    return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}




//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
sector_t*
getNextSector
( line_t*	line,
  sector_t*	sec )
{
    if (!(line->flags & ML_TWOSIDED))
	return NULL;
		
    if (line->frontsector == sec)
	return line->backsector;
	
    return line->frontsector;
}



//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindLowestFloorSurrounding(sector_t* sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		floor = sec->floorheight;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;
	
	if (other->floorheight < floor)
	    floor = other->floorheight;
    }
    return floor;
}



//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		floor = -500*FRACUNIT;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);
	
	if (!other)
	    continue;
	
	if (other->floorheight > floor)
	    floor = other->floorheight;
    }
    return floor;
}



//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
// Note: this should be doable w/o a fixed array.

// Thanks to entryway for the Vanilla overflow emulation.

// 20 adjoining sectors max!
#define MAX_ADJOINING_SECTORS     20

fixed_t
P_FindNextHighestFloor
( sector_t* sec,
  int       currentheight )
{
    int         i;
    int         h;
    int         min;
    line_t*     check;
    sector_t*   other;
    fixed_t     height = currentheight;
    fixed_t     heightlist[MAX_ADJOINING_SECTORS + 2];

    for (i=0, h=0; i < sec->linecount; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check,sec);

        if (!other)
            continue;
        
        if (other->floorheight > height)
        {
            // Emulation of memory (stack) overflow
            if (h == MAX_ADJOINING_SECTORS + 1)
            {
                height = other->floorheight;
            }
            else if (h == MAX_ADJOINING_SECTORS + 2)
            {
                // Fatal overflow: game crashes at 22 sectors
                I_Error("Sector with more than 22 adjoining sectors. "
                        "Vanilla will crash here");
            }

            heightlist[h++] = other->floorheight;
        }
    }
    
    // Find lowest height in list
    if (!h)
    {
        return currentheight;
    }
        
    min = heightlist[0];
    
    // Range checking? 
    for (i = 1; i < h; i++)
    {
        if (heightlist[i] < min)
        {
            min = heightlist[i];
        }
    }

    return min;
}

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t
P_FindLowestCeilingSurrounding(sector_t* sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		height = INT_MAX;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;

	if (other->ceilingheight < height)
	    height = other->ceilingheight;
    }
    return height;
}


//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t	P_FindHighestCeilingSurrounding(sector_t* sec)
{
    int		i;
    line_t*	check;
    sector_t*	other;
    fixed_t	height = 0;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;

	if (other->ceilingheight > height)
	    height = other->ceilingheight;
    }
    return height;
}



//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
int
P_FindSectorFromLineTag
( line_t*	line,
  int		start )
{
    int	i;
	
    for (i=start+1;i<numsectors;i++)
	if (sectors[i].tag == line->tag)
	    return i;
    
    return -1;
}




//
// Find minimum light from an adjacent sector
//
int
P_FindMinSurroundingLight
( sector_t*	sector,
  int		max )
{
    int		i;
    int		min;
    line_t*	line;
    sector_t*	check;
	
    min = max;
    for (i=0 ; i < sector->linecount ; i++)
    {
	line = sector->lines[i];
	check = getNextSector(line,sector);

	if (!check)
	    continue;

	if (check->lightlevel < min)
	    min = check->lightlevel;
    }
    return min;
}



//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

// [STRIFE]
static char crosslinestr[90];

//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void
P_CrossSpecialLine
( int           linenum,
  int           side,
  mobj_t*       thing )
{
    line_t*     line;
    side_t*     sidedef; // [STRIFE]
    int         flag;    // [STRIFE]
    int         ok;

    line = &lines[linenum];

    // haleyjd 09/21/10: corpses and missiles cannot activate any cross-over
    // line types, *except* 182 (which is for the sake of missiles).
    if((thing->flags & (MF_MISSILE|MF_CORPSE)) && line->special != 182)
        return;

    //	Triggers that other things can activate
    if (!thing->player)
    {
        // Things that should NOT trigger specials...
        // villsa [STRIFE] unused
        //   haleyjd: removed dead switch. Strife only excludes missiles and
        //   corpses, which is handled above.
 
        ok = 0;

        // [STRIFE] Added several line types. Removed none.
        switch(line->special)
        {
        case 97:        // TELEPORT RETRIGGER
        case 185:       // haleyjd: [STRIFE] Silent Teleport (used for Converter)
        case 195:       // haleyjd: [STRIFE] Silent Teleport and Change Zombie
        case 231:       // haleyjd: [STRIFE] WR Teleport (Silent at Source)
        case 125:       // TELEPORT MONSTERONLY TRIGGER
        case 126:       // TELEPORT MONSTERONLY RETRIGGER
        case 182:       // haleyjd: [STRIFE] Break glass - it's a W1 type too!
        case 10:        // PLAT DOWN-WAIT-UP-STAY TRIGGER
        case 39:        // TELEPORT TRIGGER
        case 88:        // PLAT DOWN-WAIT-UP-STAY RETRIGGER
        case 4:         // RAISE DOOR
            ok = 1;
            break;
        }
        if (!ok)
            return;
    }

    
    // Note: could use some const's here.
    switch (line->special)
    {
    //
    // TRIGGERS.
    // All from here to RETRIGGERS.
    //
    case 230:
        // haleyjd 09/21/10: [STRIFE] W1 Open Door if Quest
        sidedef = &sides[line->sidenum[0]];
        flag = (sidedef->rowoffset >> FRACBITS) - 1;

        if(!(thing->player->questflags & (1 << flag)))
            break;
        // fall-through:
    case 2:
        // Open Door - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_open);
        line->special = 0;
        break;

    case 227:
        // haleyjd 09/21/10: [STRIFE] W1 Close Door if Quest
        sidedef = &sides[line->sidenum[0]];
        flag = (sidedef->rowoffset >> FRACBITS) - 1;

        if(!(thing->player->questflags & (1 << flag)))
            break;
        // fall-through:
    case 3:
        // Close Door - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_close);
        line->special = 0;
        break;

    case 4:
        // Raise Door - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_normal);
        line->special = 0;
        break;

    case 5:
        // Raise Floor - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloor);
        line->special = 0;
        break;

    case 6:
        // Fast Ceiling Crush & Raise - [STRIFE] Verified unmodified.
        EV_DoCeiling(line,fastCrushAndRaise);
        line->special = 0;
        break;

    case 8:
        // Build Stairs - [STRIFE] Verified unmodified.
        EV_BuildStairs(line,build8);
        line->special = 0;
        break;

    case 10:
        // PlatDownWaitUp - [STRIFE] Verified unmodified.
        EV_DoPlat(line,downWaitUpStay,0);
        line->special = 0;
        break;

    case 12:
        // Light Turn On - brightest near - [STRIFE] Verified unmodified.
        EV_LightTurnOn(line,0);
        line->special = 0;
        break;

    case 13:
        // Light Turn On 255 - [STRIFE] Verified unmodified.
        EV_LightTurnOn(line,255);
        line->special = 0;
        break;

    case 16:
        // Close Door 30 - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_close30ThenOpen);
        line->special = 0;
        break;

    case 17:
        // Start Light Strobing - [STRIFE] Verified unmodified.
        EV_StartLightStrobing(line);
        line->special = 0;
        break;

    case 19:
        // Lower Floor - [STRIFE] Verified unmodified.
        EV_DoFloor(line,lowerFloor);
        line->special = 0;
        break;

    case 22:
        // villsa [STRIFE] Verified unmodified.
        // Raise floor to nearest height and change texture
        EV_DoPlat(line,raiseToNearestAndChange,0);
        line->special = 0;
        break;

    case 25:
        // Ceiling Crush and Raise - [STRIFE] Verified unmodified.
        EV_DoCeiling(line,crushAndRaise);
        line->special = 0;
        break;

    case 30:
        // Raise floor to shortest texture height - [STRIFE] Verified unmodified.
        //  on either side of lines.
        EV_DoFloor(line,raiseToTexture);
        line->special = 0;
        break;

    case 35:
        // Lights Very Dark - [STRIFE] Verified unmodified.
        EV_LightTurnOn(line,35);
        line->special = 0;
        break;

    case 36:
        // Lower Floor (TURBO) - [STRIFE] Verified unmodified.
        EV_DoFloor(line,turboLower);
        line->special = 0;
        break;

    case 37:
        // LowerAndChange - [STRIFE] Verified unmodified.
        EV_DoFloor(line,lowerAndChange);
        line->special = 0;
        break;

    case 193:
        // haleyjd 09/21/10: [STRIFE] W1 Floor Lower to Lowest if Quest
        sidedef = &sides[line->sidenum[0]];
        flag = (sidedef->rowoffset >> FRACBITS) - 1; // note is fixed_t

        // must have the questflag indicated in the line's y offset
        if(!(thing->player->questflags & (1 << flag)))
            break;
        // fall-through:
    case 38: 
        // Lower Floor To Lowest - [STRIFE] Verified unmodified.
        EV_DoFloor( line, lowerFloorToLowest );
        line->special = 0;
        break;

    case 39:
        // TELEPORT! - [STRIFE] Verified unmodified (except for 0 flags param)
        EV_Teleport( line, side, thing, TF_NORMAL );
        line->special = 0;
        break;

        /*case 40:
        // RaiseCeilingLowerFloor
        EV_DoCeiling( line, raiseToHighest );
        EV_DoFloor( line, lowerFloorToLowest );
        line->special = 0;
        break;*/

    case 44:
        // Ceiling Crush - [STRIFE] Verified unmodified.
        EV_DoCeiling( line, lowerAndCrush );
        line->special = 0;
        break;

    case 52:
        // EXIT! - haleyjd 09/21/10: [STRIFE] Exit to level tag/100
        G_ExitLevel (line->tag / 100);
        break;

    case 53:
        // Perpetual Platform Raise - [STRIFE] Verified unmodified.
        EV_DoPlat(line,perpetualRaise,0);
        line->special = 0;
        break;

    case 54:
        // Platform Stop - [STRIFE] Verified unmodified.
        EV_StopPlat(line);
        line->special = 0;
        break;

    case 56:
        // Raise Floor Crush - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloorCrush);
        line->special = 0;
        break;

    case 57:
        // Ceiling Crush Stop - [STRIFE] Verified unmodified.
        EV_CeilingCrushStop(line);
        line->special = 0;
        break;

    case 58:
        // [STRIFE] raiseFloor24 was modified into raiseFloor64
        // Raise Floor 64 
        EV_DoFloor(line,raiseFloor64);
        line->special = 0;
        break;

    case 59:
        // Raise Floor 24 And Change - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloor24AndChange);
        line->special = 0;
        break;

    case 104:
        // Turn lights off in sector(tag) - [STRIFE] Verified unmodified.
        EV_TurnTagLightsOff(line);
        line->special = 0;
        break;

    case 108:
        // Blazing Door Raise (faster than TURBO!) - [STRIFE] Verified unmodified.
        EV_DoDoor (line,vld_blazeRaise);
        line->special = 0;
        break;

    case 109:
        // Blazing Door Open (faster than TURBO!) - [STRIFE] Verified unmodified.
        EV_DoDoor (line,vld_blazeOpen);
        line->special = 0;
        break;

    case 100:
        // Build Stairs Turbo 16 - [STRIFE] Verified unmodified.
        EV_BuildStairs(line,turbo16);
        line->special = 0;
        break;

    case 197:
        // haleyjd 09/21/10: [STRIFE] Blazing Door Close if Has Sigil B
        if(thing->player->sigiltype <= 0)
            break;
        // fall-through:
    case 110:
        // Blazing Door Close (faster than TURBO!) - [STRIFE] Verified unmodified.
        EV_DoDoor (line,vld_blazeClose);
        line->special = 0;
        break;

    case 119:
        // Raise floor to nearest surr. floor - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloorToNearest);
        line->special = 0;
        break;

    case 121:
        // villsa [STRIFE] Verified unmodified.
        // Blazing PlatDownWaitUpStay
        EV_DoPlat(line,blazeDWUS,0);
        line->special = 0;
        break;

    case 124:
        // haleyjd 09/21/10: [STRIFE] W1 Start Finale
        // Altered from G_SecretExitLevel.
        G_StartFinale();
        break;

    case 125:
        // TELEPORT MonsterONLY - [STRIFE] Verified unmodified
        //    (except for 0 flags parameter)
        if (!thing->player)
        {
            EV_Teleport( line, side, thing, TF_NORMAL );
            line->special = 0;
        }
        break;

    case 130:
        // Raise Floor Turbo - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloorTurbo);
        line->special = 0;
        break;

    case 141:
        // Silent Ceiling Crush & Raise - [STRIFE] Verified unmodified.
        EV_DoCeiling(line,silentCrushAndRaise);
        line->special = 0;
        break;

    case 174:
        // villsa [STRIFE] Split Open
        EV_DoDoor(line, vld_splitOpen);
        line->special = 0;
        break;

    case 183:
        // villsa [STRIFE] Split Raise Nearest
        EV_DoDoor(line, vld_splitRaiseNearest);
        line->special = 0;
        break;

    case 178: 
        // haleyjd 09/24/10: [STRIFE] W1 Build Stairs Down 16
        EV_BuildStairs(line, buildDown16);
        line->special = 0;
        break;

    case 179: 
        // haleyjd 09/25/10: [STRIFE] W1 Ceiling Lower to Floor
        EV_DoCeiling(line, lowerToFloor);
        line->special = 0;
        break;

    case 182:
        // haleyjd 09/21/10: [STRIFE] Break Glass
        // 182 is a unique linetype in that it is both a G1 and a W1 linetype,
        // but only missiles may activate it as a W1 type.
        if(thing->flags & MF_MISSILE)
            P_ChangeSwitchTexture(line, 1); // why 1? it will be cleared anyway.
        break;

    case 187:
        // haleyjd 09/21/10: [STRIFE] W1 Clear Force Fields if Quest
        sidedef = &sides[line->sidenum[0]];
        flag = (sidedef->rowoffset >> FRACBITS) - 1; // note is fixed_t

        // must have the questflag indicated in the line's y offset
        if(!(thing->player->questflags & (1 << flag)))
            break;

        // Do it!
        EV_ClearForceFields(line);
        line->special = 0;
        break;

    case 188:
        // haleyjd 09/21/10: [STRIFE] W1 Open Door if Quest 16 (Gate Mechanism 
        // Destroyed)
        if(!(thing->player->questflags & QF_QUEST16))
            break;
        EV_DoDoor(line, vld_open);
        line->special = 0;
        break;

    case 196:
        // haleyjd 09/26/10: [STRIFE] W1 Floor Lower to Lowest if Sigil Type > 0
        if(thing->player->sigiltype > 0)
        {
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
        }
        break;

    case 200:
        // haleyjd 09/21/10: [STRIFE] W1 Open Door if Sigil Owned
        if(!(thing->player->weaponowned[wp_sigil]))
            break;
        EV_DoDoor(line, vld_open);
        line->special = 0;
        break;

    case 201:
        // haleyjd 09/21/10: [STRIFE] W1 Voiced Objective (First Side Only)
        if(side == 1)
            break;
        // fall-through:
    case 202:
        // haleyjd 09/21/10: [STRIFE] W1 Voiced Objective (Tag = VOC/LOG #)
        // must be consoleplayer
        if(thing->player != &players[consoleplayer])
            break;

        // must have comm unit
        if(!(thing->player->powers[pw_communicator]))
            break;

        // load voice
        DEH_snprintf(crosslinestr, sizeof(crosslinestr), "voc%i", line->tag);
        I_StartVoice(crosslinestr);

        // load objective
        DEH_snprintf(crosslinestr, sizeof(crosslinestr), "log%i", line->tag);
        GiveObjective(crosslinestr, 0);

        // Put up a message
        thing->player->message = DEH_String("Incoming Message...");
        line->special = 0;
        break;

    case 210:
        // haleyjd 09/21/10: [STRIFE] W1 Voiced Objective if Flamethrower????
        // I don't think this is actually used anywhere o_O
        // must be player 1...
        if(thing->player != &players[0])
            break;

        // must have comm unit
        if(!(thing->player->powers[pw_communicator]))
            break;

        // must have... the flamethrower?!
        if(!(thing->player->weaponowned[wp_flame]))
            break;

        // load voice
        DEH_snprintf(crosslinestr, sizeof(crosslinestr), "voc%i", line->tag);
        I_StartVoice(crosslinestr);

        // load objective
        DEH_snprintf(crosslinestr, sizeof(crosslinestr), "log%i", line->tag);
        GiveObjective(crosslinestr, 0);

        // Put up a message
        thing->player->message = DEH_String("Incoming Message from BlackBird...");
        line->special = 0;
        break;

    case 212:
        // haleyjd 09/25/10: [STRIFE] W1 Floor Lower to Lowest if Have Flamethrower
        if(thing->player->weaponowned[wp_flame])
        {
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
        }
        break;

    case 215:
        // haleyjd 09/21/10: [STRIFE] W1 Voiced Objective if Quest (Tag/100, Tag%100)
        // must be player 1...
        if(thing->player != &players[0])
            break;

        // must have comm unit
        if(!(thing->player->powers[pw_communicator]))
            break;

        if(line->tag != 0)
        {
            // test for questflag
            if(!(thing->player->questflags & (1 << (line->tag % 100 - 1))))
                break;
        }

        // start voice
        DEH_snprintf(crosslinestr, sizeof(crosslinestr), "voc%i", line->tag/100);
        I_StartVoice(crosslinestr);

        // give objective
        DEH_snprintf(crosslinestr, sizeof(crosslinestr), "log%i", line->tag/100);
        GiveObjective(crosslinestr, 0);

        // Put up a message
        thing->player->message = DEH_String("Incoming Message from BlackBird...");
        line->special = 0;
        break;

    case 204:
        // haleyjd 09/21/10: [STRIFE] W1 Change Music (unused!)
        if(thing->player != &players[0])
            break;
        S_ChangeMusic(line->tag, 1);
        line->special = 0;
        break;

    case 228:
        // haleyjd 09/21/10: [STRIFE] W1 Entity Voice?
        if(!(thing->player->questflags & QF_QUEST24)) // Not killed Macil???
            break; // STRIFE-TODO: verify...

        if(!(thing->player->questflags & QF_QUEST28)) // ????? STRIFE-TODO
            I_StartVoice(DEH_String("voc128"));
        else
            I_StartVoice(DEH_String("voc130"));
        
        line->special = 0;
        break;

        //
        // RETRIGGERS.  All from here till end.
        //
    case 72:
        // Ceiling Crush - [STRIFE] Verified unmodified.
        EV_DoCeiling( line, lowerAndCrush );
        break;

    case 73:
        // Ceiling Crush and Raise - [STRIFE] Verified unmodified.
        EV_DoCeiling(line,crushAndRaise);
        break;

    case 74:
        // Ceiling Crush Stop - [STRIFE] Verified unmodified.
        EV_CeilingCrushStop(line);
        break;

    case 75:
        // Close Door - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_close);
        break;

    case 76:
        // Close Door 30 - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_close30ThenOpen);
        break;

    case 77:
        // Fast Ceiling Crush & Raise - [STRIFE] Verified unmodified.
        EV_DoCeiling(line,fastCrushAndRaise);
        break;

    case 79:
        // Lights Very Dark - [STRIFE] Verified unmodified.
        EV_LightTurnOn(line,35);
        break;

    case 80:
        // Light Turn On - brightest near - [STRIFE] Verified unmodified.
        EV_LightTurnOn(line,0);
        break;

    case 81:
        // Light Turn On 255 - [STRIFE] Verified unmodified.
        EV_LightTurnOn(line,255);
        break;

    case 82:
        // Lower Floor To Lowest - [STRIFE] Verified unmodified.
        EV_DoFloor( line, lowerFloorToLowest );
        break;

    case 83:
        // Lower Floor - [STRIFE] Verified unmodified.
        EV_DoFloor(line,lowerFloor);
        break;

    case 84:
        // LowerAndChange - [STRIFE] Verified unmodified.
        EV_DoFloor(line,lowerAndChange);
        break;

    case 86:
        // Open Door - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_open);
        break;

    case 87:
        // Perpetual Platform Raise - [STRIFE] Verified unmodified.
        EV_DoPlat(line,perpetualRaise,0);
        break;

    case 88:
        // PlatDownWaitUp - [STRIFE] Verified unmodified.
        EV_DoPlat(line,downWaitUpStay,0);
        break;

    case 89:
        // Platform Stop - [STRIFE] Verified unmodified.
        EV_StopPlat(line);
        break;

    case 216:
        // haleyjd 09/21/10: [STRIFE] WR Raise Door if Quest
        sidedef = &sides[line->sidenum[0]];
        flag = (sidedef->rowoffset >> FRACBITS) - 1; // note is fixed_t.

        if(!(thing->player->questflags & (1 << flag)))
            break;
        // fall-through:
    case 90: 
        // Raise Door - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_normal);
        break;

    case 91:
        // Raise Floor - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloor);
        break;

    case 92:
        // [STRIFE] raiseFloor24 changed to raiseFloor64
        // Raise Floor 64
        EV_DoFloor(line,raiseFloor64);
        break;

    case 93:
        // Raise Floor 24 And Change - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloor24AndChange);
        break;

    case 94:
        // Raise Floor Crush - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloorCrush);
        break;

    case 95:
        // villsa [STRIFE] Verified unmodified.
        // Raise floor to nearest height
        // and change texture.
        EV_DoPlat(line,raiseToNearestAndChange,0);
        break;

    case 96:
        // Raise floor to shortest texture height - [STRIFE] Verified unmodified.
        // on either side of lines.
        EV_DoFloor(line,raiseToTexture);
        break;

    case 97:
        // TELEPORT! - [STRIFE] Verified unmodified (except for 0 flags param)
        EV_Teleport( line, side, thing, TF_NORMAL );
        break;

    case 98:
        // Lower Floor (TURBO) - [STRIFE] Verified unmodified.
        EV_DoFloor(line,turboLower);
        break;

    case 105:
        // Blazing Door Raise (faster than TURBO!) - [STRIFE] Verified unmodified.
        EV_DoDoor (line,vld_blazeRaise);
        break;

    case 106:
        // Blazing Door Open (faster than TURBO!) - [STRIFE] Verified unmodified.
        EV_DoDoor (line,vld_blazeOpen);
        break;

    case 107:
        // Blazing Door Close (faster than TURBO!) - [STRIFE] Verified unmodified.
        EV_DoDoor (line,vld_blazeClose);
        break;

    case 120:
        // villsa [STRIFE] Verified unmodified.
        // Blazing PlatDownWaitUpStay.
        EV_DoPlat(line,blazeDWUS,0);
        break;

    case 126:
        // TELEPORT MonsterONLY. - [STRIFE] Verified unmodified (except for 0 flags param)
        if (!thing->player)
            EV_Teleport( line, side, thing, TF_NORMAL );
        break;

    case 128:
        // Raise To Nearest Floor - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloorToNearest);
        break;

    case 129:
        // Raise Floor Turbo - [STRIFE] Verified unmodified.
        EV_DoFloor(line,raiseFloorTurbo);
        break;

    case 186:
        // haleyjd [STRIFE] Exit Level to Spot, First Side Only
        if(side == 1)
            break;
        // fall-through:
    case 145:
        // haleyjd [STRIFE] Exit Level to Spot
        thing->momx = thing->momy = thing->momz = 0;
        {
            int map  = line->tag / 100;
            int spot = line->tag % 100;

            if(thing->player->weaponowned[wp_sigil])
            {
                if(map == 3)
                    map = 30;
                else if(map == 7)
                    map = 10;
            }

            DEH_snprintf(crosslinestr, sizeof(crosslinestr), 
                         "Entering%s", 
                         DEH_String(mapnames[map - 1]) + 8);
            thing->player->message = crosslinestr;

            if(netgame && deathmatch)
            {
                if(levelTimer && levelTimeCount != 0)
                {
                    DEH_snprintf(crosslinestr, sizeof(crosslinestr), 
                                 "%d min left", 
                                 (levelTimeCount/TICRATE)/60);
                    break;
                }

                // raise switch from floor
                EV_DoFloor(line, raiseFloor64);
            }
            else
            {
                // normal single-player exit

                // BUG: Here is the opening for a flaming player to cross past
                // the exit line and hit a deathmatch switch ;) It's not so much
                // that this is incorrect, as that they forgot to add such a 
                // check to the other kind of exit lines too ;)
                if(thing->player->health <= 0)
                    break;

                G_RiftExitLevel(map, spot, thing->angle);
            }
        }
        break;

    case 175:
        // haleyjd 09/21/10: [STRIFE] WR Raise Alarm if < 16 Above Floor
        if(thing->z < thing->floorz + 16 * FRACUNIT)
            P_NoiseAlert(thing->player->mo, thing->player->mo);
        break;

    case 198:
        // haleyjd 09/21/10: [STRIFE] WR Raise Alarm if No Guard Uniform
        if(P_PlayerHasItem(thing->player, MT_QUEST_GUARD_UNIFORM))
            break;
        // fall-through:
    case 150:
        // haleyjd 09/21/10: [STRIFE] WR Raise Alarm
        P_NoiseAlert(thing->player->mo, thing->player->mo);
        break;

    case 208:
        // haleyjd 09/21/10: [STRIFE] WR Raise Alarm if Have Flamethrower
        // O_o - this is definitely unused. Was an entire flamethrower quest
        // cut out of the game before release?
        if(thing->player->weaponowned[wp_flame])
            P_NoiseAlert(thing->player->mo, thing->player->mo);
        break;

    case 206:
        // haleyjd 09/21/10: [STRIFE] WR Raise Alarm if Have Chalice
        // This *is* used, inside the Tavern in Tarnhill. Oddly there is also
        // one just randomly placed outside the entrance to the Power Station.
        if(P_PlayerHasItem(thing->player, MT_INV_CHALICE))
            P_NoiseAlert(thing->player->mo, thing->player->mo);
        break;

    case 184:
        // villsa [STRIFE] plat up wait down stay
        if(EV_DoPlat(line, upWaitDownStay, 0))
            P_ChangeSwitchTexture(line, 1); // In P_CrossSpecialLine? Copypasta error?
        break;

    case 185:
        // haleyjd 09/21/10: [STRIFE] Silent Teleport (used for Converter)
        EV_Teleport(line, side, thing, TF_FULLSILENCE);
        break;

    case 195:
        // haleyjd 09/21/10: [STRIFE] Silent Teleport and Change Zombie
        EV_Teleport(line, side, thing, TF_FULLSILENCE);
        P_SetMobjState(thing, S_AGRD_00); // 419
        break;

    case 203:
        // haleyjd 09/21/10: [STRIFE] WR Change Music
        if(thing->player != &players[0])
            break;
        S_ChangeMusic(line->tag, 1);
        break;

    case 231:
        // haleyjd 09/21/10: [STRIFE] WR Teleport (Silent at Source)
        EV_Teleport(line, side, thing, TF_SRCSILENCE);
        break;
        
        // haleyjd 09/21/10: Moved one-time-use lines up above with the others.
    }
}



//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void
P_ShootSpecialLine
( mobj_t*       thing,
  line_t*       line )
{
    int         ok;

    //	Impacts that other things can activate.
    if (!thing->player)
    {
        ok = 0;
        switch(line->special)
        {
        case 46:  // OPEN DOOR IMPACT
        case 182: // villsa [STRIFE] for windows
            ok = 1;
            break;
        }
        if (!ok)
            return;
    }

    switch(line->special)
    {
    case 24:
        // RAISE FLOOR - [STRIFE] Verified unmodified
        EV_DoFloor(line,raiseFloor);
        P_ChangeSwitchTexture(line,0);
        break;

    case 46:
        // OPEN DOOR - [STRIFE] Verified unmodified.
        EV_DoDoor(line,vld_open);
        P_ChangeSwitchTexture(line,1);
        break;

    case 47:
        // villsa [STRIFE] Verified unmodified.
        // RAISE FLOOR NEAR AND CHANGE
        EV_DoPlat(line,raiseToNearestAndChange,0);
        P_ChangeSwitchTexture(line,0);
        break;

    case 180:
        // haleyjd 09/22/10: [STRIFE] G1 Raise Floor 512 & Change
        EV_DoFloor(line, raiseFloor512AndChange);
        P_ChangeSwitchTexture(line, 0);
        break;

    case 182:
        // villsa [STRIFE] G1 Break Glass
        //   haleyjd: note that 182 is also a W1 type in P_CrossSpecialLine, but
        //   can only be activated in that manner by an MF_MISSILE object.
        P_ChangeSwitchTexture(line, 0);
        break;
    }
}



//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
// [STRIFE] Modified for new sector types and changes to old ones.
//
void P_PlayerInSpecialSector (player_t* player)
{
    sector_t*	sector;

    sector = player->mo->subsector->sector;

    // Falling, not all the way down yet?
    if (player->mo->z != sector->floorheight)
        return;	

    // Has hitten ground.
    switch (sector->special)
    {
    case 5:
        // HELLSLIME DAMAGE
        // [STRIFE] +2 to nukagecount
        if(!player->powers[pw_ironfeet])
            player->nukagecount += 2;
        break;
    
    case 16:
        // [STRIFE] +4 to nukagecount
        if(!player->powers[pw_ironfeet])
            player->nukagecount += 4;
        break;

    case 4:
    case 7:
        // [STRIFE] Immediate 5 damage every 31 tics
        if(!player->powers[pw_ironfeet])
            if(!(leveltime & 0x1f))
                P_DamageMobj(player->mo, NULL, NULL, 5);
        break;

    case 9:
        // SECRET SECTOR
        //player->secretcount++; [STRIFE] Don't have a secret count.
        sector->special = 0;
        if(player - players == consoleplayer)
            S_StartSound(NULL, sfx_yeah);
        break;

    case 11:
        // EXIT SUPER DAMAGE! (for E1M8 finale)
        player->cheats &= ~CF_GODMODE;

        if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 20);

        if (player->health <= 10)
            G_ExitLevel(0);
        break;

    case 15:
        // haleyjd 08/30/10: [STRIFE] "Instant" Death sector
        P_DamageMobj(player->mo, NULL, NULL, 999);
        break;


    case 18:
        // haleyjd 08/30/10: [STRIFE] Water current
        {
            int tagval = sector->tag - 100;
            fixed_t force;
            angle_t angle;

            if(player->cheats & CF_NOCLIP)
                return;

            force = (tagval % 10) << 12;
            angle = (tagval / 10) << 29;

            P_Thrust(player, angle, force);
        }
        break;

    default:
        I_Error ("P_PlayerInSpecialSector: "
                 "unknown special %i",
                 sector->special);
        break;
    };
}




//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
// [STRIFE] Modifications to support multiple scrolling line types.
//
boolean         levelTimer;
int             levelTimeCount;

void P_UpdateSpecials (void)
{
    anim_t*     anim;
    int         pic;
    int         i;
    line_t*     line;


    //  LEVEL TIMER
    if (levelTimer == true)
    {
        if(levelTimeCount) // [STRIFE] Does not allow to go negative
            levelTimeCount--;
        
        /*
        // [STRIFE] Not done here. Exit lines check this manually instead.
        if (!levelTimeCount)
            G_ExitLevel(0);
        */
    }

    //  ANIMATE FLATS AND TEXTURES GLOBALLY
    for (anim = anims ; anim < lastanim ; anim++)
    {
        for (i=anim->basepic ; i<anim->basepic+anim->numpics ; i++)
        {
            pic = anim->basepic + ( (leveltime/anim->speed + i)%anim->numpics );
            if (anim->istexture)
                texturetranslation[i] = pic;
            else
                flattranslation[i] = pic;
        }
    }

    
    //  ANIMATE LINE SPECIALS
    for (i = 0; i < numlinespecials; i++)
    {
        line = linespeciallist[i];
        switch(line->special)
        {
        case 48:
            // EFFECT FIRSTCOL SCROLL +
            sides[line->sidenum[0]].textureoffset += FRACUNIT;
            break;

        case 142:
            // haleyjd 09/25/10 [STRIFE] Scroll Up Slow
            sides[line->sidenum[0]].rowoffset += FRACUNIT;
            break;

        case 143:
            // haleyjd 09/25/10 [STRIFE] Scroll Down Fast (3 Units/Tic)
            sides[line->sidenum[0]].rowoffset -= 3*FRACUNIT;
            break;

        case 149:
            // haleyjd 09/25/10 [STRIFE] Scroll Down Slow
            sides[line->sidenum[0]].rowoffset -= FRACUNIT;
            break;
        }
    }

    
    //  DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
        if (buttonlist[i].btimer)
        {
            buttonlist[i].btimer--;
            if (!buttonlist[i].btimer)
            {
                switch(buttonlist[i].where)
                {
                case top:
                    sides[buttonlist[i].line->sidenum[0]].toptexture =
                        buttonlist[i].btexture;
                    break;

                case middle:
                    sides[buttonlist[i].line->sidenum[0]].midtexture =
                        buttonlist[i].btexture;
                    break;

                case bottom:
                    sides[buttonlist[i].line->sidenum[0]].bottomtexture =
                        buttonlist[i].btexture;
                    break;
                }
                S_StartSound(&buttonlist[i].soundorg,sfx_swtchn);
                memset(&buttonlist[i],0,sizeof(button_t));
            }
        }
}


//
// Donut overrun emulation
//
// Derived from the code from PrBoom+.  Thanks go to Andrey Budko (entryway)
// as usual :-)
//

#define DONUT_FLOORHEIGHT_DEFAULT 0x00000000
#define DONUT_FLOORPIC_DEFAULT 0x16

static void DonutOverrun(fixed_t *s3_floorheight, short *s3_floorpic,
                         line_t *line, sector_t *pillar_sector)
{
    static int first = 1;
    static int tmp_s3_floorheight;
    static int tmp_s3_floorpic;

    extern int numflats;

    if (first)
    {
        int p;

        // This is the first time we have had an overrun.
        first = 0;

        // Default values
        tmp_s3_floorheight = DONUT_FLOORHEIGHT_DEFAULT;
        tmp_s3_floorpic = DONUT_FLOORPIC_DEFAULT;

        //!
        // @category compat
        // @arg <x> <y>
        //
        // Use the specified magic values when emulating behavior caused
        // by memory overruns from improperly constructed donuts.
        // In Vanilla Strife this can differ depending on the operating
        // system.  The default (if this option is not specified) is to
        // emulate the behavior when running under Windows 98.

        p = M_CheckParmWithArgs("-donut", 2);

        if (p > 0)
        {
            // Dump of needed memory: (fixed_t)0000:0000 and (short)0000:0008
            //
            // C:\>debug
            // -d 0:0
            //
            // DOS 6.22:
            // 0000:0000    (57 92 19 00) F4 06 70 00-(16 00)
            // DOS 7.1:
            // 0000:0000    (9E 0F C9 00) 65 04 70 00-(16 00)
            // Win98:
            // 0000:0000    (00 00 00 00) 65 04 70 00-(16 00)
            // DOSBox under XP:
            // 0000:0000    (00 00 00 F1) ?? ?? ?? 00-(07 00)

            M_StrToInt(myargv[p + 1], &tmp_s3_floorheight);
            M_StrToInt(myargv[p + 2], &tmp_s3_floorpic);

            if (tmp_s3_floorpic >= numflats)
            {
                fprintf(stderr,
                        "DonutOverrun: The second parameter for \"-donut\" "
                        "switch should be greater than 0 and less than number "
                        "of flats (%d). Using default value (%d) instead. \n",
                        numflats, DONUT_FLOORPIC_DEFAULT);
                tmp_s3_floorpic = DONUT_FLOORPIC_DEFAULT;
            }
        }
    }

    /*
    fprintf(stderr,
            "Linedef: %d; Sector: %d; "
            "New floor height: %d; New floor pic: %d\n",
            line->iLineID, pillar_sector->iSectorID,
            tmp_s3_floorheight >> 16, tmp_s3_floorpic);
     */

    *s3_floorheight = (fixed_t) tmp_s3_floorheight;
    *s3_floorpic = (short) tmp_s3_floorpic;
}


//
// Special Stuff that can not be categorized
//
int EV_DoDonut(line_t*	line)
{
    sector_t*		s1;
    sector_t*		s2;
    sector_t*		s3;
    int			secnum;
    int			rtn;
    int			i;
    floormove_t*	floor;
    fixed_t s3_floorheight;
    short s3_floorpic;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
	s1 = &sectors[secnum];

	// ALREADY MOVING?  IF SO, KEEP GOING...
	if (s1->specialdata)
	    continue;

	rtn = 1;
	s2 = getNextSector(s1->lines[0],s1);

        // Vanilla Doom does not check if the linedef is one sided.  The
        // game does not crash, but reads invalid memory and causes the
        // sector floor to move "down" to some unknown height.
        // DOSbox prints a warning about an invalid memory access.
        //
        // I'm not sure exactly what invalid memory is being read.  This
        // isn't something that should be done, anyway.
        // Just print a warning and return.

        if (s2 == NULL)
        {
            fprintf(stderr,
                    "EV_DoDonut: linedef had no second sidedef! "
                    "Unexpected behavior may occur in Vanilla Doom. \n");
	    break;
        }

	for (i = 0; i < s2->linecount; i++)
	{
	    s3 = s2->lines[i]->backsector;

	    if (s3 == s1)
		continue;

            if (s3 == NULL)
            {
                // e6y
                // s3 is NULL, so
                // s3->floorheight is an int at 0000:0000
                // s3->floorpic is a short at 0000:0008
                // Trying to emulate

                fprintf(stderr,
                        "EV_DoDonut: WARNING: emulating buffer overrun due to "
                        "NULL back sector. "
                        "Unexpected behavior may occur in Vanilla Doom.\n");

                DonutOverrun(&s3_floorheight, &s3_floorpic, line, s1);
            }
            else
            {
                s3_floorheight = s3->floorheight;
                s3_floorpic = s3->floorpic;
            }

	    //	Spawn rising slime
	    floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
	    P_AddThinker (&floor->thinker);
	    s2->specialdata = floor;
	    floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
	    floor->type = donutRaise;
	    floor->crush = false;
	    floor->direction = 1;
	    floor->sector = s2;
	    floor->speed = FLOORSPEED / 2;
	    floor->texture = s3_floorpic;
	    floor->newspecial = 0;
	    floor->floordestheight = s3_floorheight;
	    
	    //	Spawn lowering donut-hole
	    floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
	    P_AddThinker (&floor->thinker);
	    s1->specialdata = floor;
	    floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
	    floor->type = lowerFloor;
	    floor->crush = false;
	    floor->direction = -1;
	    floor->sector = s1;
	    floor->speed = FLOORSPEED / 2;
	    floor->floordestheight = s3_floorheight;
	    break;
	}
    }
    return rtn;
}



//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//
short		numlinespecials;
line_t*		linespeciallist[MAXLINEANIMS];


// Parses command line parameters.
//
// haleyjd 09/25/10: [STRIFE] Modifications for more scrolling line types and
// for initialization of sliding door resources.
//
void P_SpawnSpecials (void)
{
    sector_t*   sector;
    int         i;

    // See if -TIMER was specified.

    if (timelimit > 0 && deathmatch)
    {
        levelTimer = true;
        levelTimeCount = timelimit * 60 * TICRATE;
    }
    else
    {
        levelTimer = false;
    }

    //	Init special SECTORs - [STRIFE] Verified unmodified.
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
        if (!sector->special)
            continue;

        switch (sector->special)
        {
        case 1:
            // FLICKERING LIGHTS
            P_SpawnLightFlash (sector);
            break;

        case 2:
            // STROBE FAST
            P_SpawnStrobeFlash(sector,FASTDARK,0);
            break;

        case 3:
            // STROBE SLOW
            P_SpawnStrobeFlash(sector,SLOWDARK,0);
            break;

        case 4:
            // STROBE FAST/DEATH SLIME
            P_SpawnStrobeFlash(sector,FASTDARK,0);
            sector->special = 4;
            break;

        case 8:
            // GLOWING LIGHT
            P_SpawnGlowingLight(sector);
            break;
        case 9:
            // SECRET SECTOR
            totalsecret++;
            break;

        case 10:
            // DOOR CLOSE IN 30 SECONDS
            P_SpawnDoorCloseIn30 (sector);
            break;

        case 12:
            // SYNC STROBE SLOW
            P_SpawnStrobeFlash (sector, SLOWDARK, 1);
            break;

        case 13:
            // SYNC STROBE FAST
            P_SpawnStrobeFlash (sector, FASTDARK, 1);
            break;

        case 14:
            // DOOR RAISE IN 5 MINUTES
            P_SpawnDoorRaiseIn5Mins (sector, i);
            break;

        case 17:
            P_SpawnFireFlicker(sector);
            break;
        }
    }


    //	Init line EFFECTs
    numlinespecials = 0;
    for (i = 0;i < numlines; i++)
    {
        switch(lines[i].special)
        {
        case 48:  // EFFECT FIRSTCOL SCROLL+
        case 142:
        case 143:
        case 149:
            linespeciallist[numlinespecials] = &lines[i];
            numlinespecials++;
            break;
        }
    }

    //	Init other misc stuff
    for (i = 0;i < MAXCEILINGS;i++)
        activeceilings[i] = NULL;

    for (i = 0;i < MAXPLATS;i++)
        activeplats[i] = NULL;

    for (i = 0;i < MAXBUTTONS;i++)
        memset(&buttonlist[i],0,sizeof(button_t));

    // villsa [STRIFE]
    P_InitSlidingDoorFrames();
}
