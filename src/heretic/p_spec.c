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

// P_Spec.c

#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

// Macros

#define MAX_AMBIENT_SFX 8       // Per level

// Types

typedef enum
{
    afxcmd_play,                // (sound)
    afxcmd_playabsvol,          // (sound, volume)
    afxcmd_playrelvol,          // (sound, volume)
    afxcmd_delay,               // (ticks)
    afxcmd_delayrand,           // (andbits)
    afxcmd_end                  // ()
} afxcmd_t;

// Data

int *LevelAmbientSfx[MAX_AMBIENT_SFX];
int *AmbSfxPtr;
int AmbSfxCount;
int AmbSfxTics;
int AmbSfxVolume;

int AmbSndSeqInit[] = {         // Startup
    afxcmd_end
};
int AmbSndSeq1[] = {            // Scream
    afxcmd_play, sfx_amb1,
    afxcmd_end
};
int AmbSndSeq2[] = {            // Squish
    afxcmd_play, sfx_amb2,
    afxcmd_end
};
int AmbSndSeq3[] = {            // Drops
    afxcmd_play, sfx_amb3,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, sfx_amb7,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, sfx_amb3,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, sfx_amb7,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, sfx_amb3,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_play, sfx_amb7,
    afxcmd_delay, 16,
    afxcmd_delayrand, 31,
    afxcmd_end
};
int AmbSndSeq4[] = {            // SlowFootSteps
    afxcmd_play, sfx_amb4,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb4, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb4, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb4, -3,
    afxcmd_delay, 15,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_end
};
int AmbSndSeq5[] = {            // Heartbeat
    afxcmd_play, sfx_amb5,
    afxcmd_delay, 35,
    afxcmd_play, sfx_amb5,
    afxcmd_delay, 35,
    afxcmd_play, sfx_amb5,
    afxcmd_delay, 35,
    afxcmd_play, sfx_amb5,
    afxcmd_end
};
int AmbSndSeq6[] = {            // Bells
    afxcmd_play, sfx_amb6,
    afxcmd_delay, 17,
    afxcmd_playrelvol, sfx_amb6, -8,
    afxcmd_delay, 17,
    afxcmd_playrelvol, sfx_amb6, -8,
    afxcmd_delay, 17,
    afxcmd_playrelvol, sfx_amb6, -8,
    afxcmd_end
};
int AmbSndSeq7[] = {            // Growl
    afxcmd_play, sfx_bstsit,
    afxcmd_end
};
int AmbSndSeq8[] = {            // Magic
    afxcmd_play, sfx_amb8,
    afxcmd_end
};
int AmbSndSeq9[] = {            // Laughter
    afxcmd_play, sfx_amb9,
    afxcmd_delay, 16,
    afxcmd_playrelvol, sfx_amb9, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, sfx_amb9, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, sfx_amb10, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, sfx_amb10, -4,
    afxcmd_delay, 16,
    afxcmd_playrelvol, sfx_amb10, -4,
    afxcmd_end
};
int AmbSndSeq10[] = {           // FastFootsteps
    afxcmd_play, sfx_amb4,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb4, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb4, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb4, -3,
    afxcmd_delay, 8,
    afxcmd_playrelvol, sfx_amb11, -3,
    afxcmd_end
};

int *AmbientSfx[] = {
    AmbSndSeq1,                 // Scream
    AmbSndSeq2,                 // Squish
    AmbSndSeq3,                 // Drops
    AmbSndSeq4,                 // SlowFootsteps
    AmbSndSeq5,                 // Heartbeat
    AmbSndSeq6,                 // Bells
    AmbSndSeq7,                 // Growl
    AmbSndSeq8,                 // Magic
    AmbSndSeq9,                 // Laughter
    AmbSndSeq10                 // FastFootsteps
};

animdef_t animdefs[] = {
    // false = flat
    // true = texture
    {false, "FLTWAWA3", "FLTWAWA1", 8}, // Water
    {false, "FLTSLUD3", "FLTSLUD1", 8}, // Sludge
    {false, "FLTTELE4", "FLTTELE1", 6}, // Teleport
    {false, "FLTFLWW3", "FLTFLWW1", 9}, // River - West
    {false, "FLTLAVA4", "FLTLAVA1", 8}, // Lava
    {false, "FLATHUH4", "FLATHUH1", 8}, // Super Lava
    {true, "LAVAFL3", "LAVAFL1", 6},    // Texture: Lavaflow
    {true, "WATRWAL3", "WATRWAL1", 4},  // Texture: Waterfall
    {-1}
};

anim_t anims[MAXANIMS];
anim_t *lastanim;

int *TerrainTypes;
struct
{
    char *name;
    int type;
} TerrainTypeDefs[] =
{
    { "FLTWAWA1", FLOOR_WATER },
    { "FLTFLWW1", FLOOR_WATER },
    { "FLTLAVA1", FLOOR_LAVA },
    { "FLATHUH1", FLOOR_LAVA },
    { "FLTSLUD1", FLOOR_SLUDGE },
    { "END", -1 }
};

mobj_t LavaInflictor;

//----------------------------------------------------------------------------
//
// PROC P_InitLava
//
//----------------------------------------------------------------------------

void P_InitLava(void)
{
    memset(&LavaInflictor, 0, sizeof(mobj_t));
    LavaInflictor.type = MT_PHOENIXFX2;
    LavaInflictor.flags2 = MF2_FIREDAMAGE | MF2_NODMGTHRUST;
}

//----------------------------------------------------------------------------
//
// PROC P_InitTerrainTypes
//
//----------------------------------------------------------------------------

void P_InitTerrainTypes(void)
{
    int i;
    int lump;
    int size;

    size = (numflats + 1) * sizeof(int);
    TerrainTypes = Z_Malloc(size, PU_STATIC, 0);
    memset(TerrainTypes, 0, size);
    for (i = 0; TerrainTypeDefs[i].type != -1; i++)
    {
        lump = W_CheckNumForName(TerrainTypeDefs[i].name);
        if (lump != -1)
        {
            TerrainTypes[lump - firstflat] = TerrainTypeDefs[i].type;
        }
    }
}

//----------------------------------------------------------------------------
//
// PROC P_InitPicAnims
//
//----------------------------------------------------------------------------

void P_InitPicAnims(void)
{
    char *startname;
    char *endname;
    int i;

    lastanim = anims;
    for (i = 0; animdefs[i].istexture != -1; i++)
    {
        startname = DEH_String(animdefs[i].startname);
        endname = DEH_String(animdefs[i].endname);

        if (animdefs[i].istexture)
        {                       // Texture animation
            if (R_CheckTextureNumForName(startname) == -1)
            {                   // Texture doesn't exist
                continue;
            }
            lastanim->picnum = R_TextureNumForName(endname);
            lastanim->basepic = R_TextureNumForName(startname);
        }
        else
        {                       // Flat animation
            if (W_CheckNumForName(startname) == -1)
            {                   // Flat doesn't exist
                continue;
            }
            lastanim->picnum = R_FlatNumForName(endname);
            lastanim->basepic = R_FlatNumForName(startname);
        }
        lastanim->istexture = animdefs[i].istexture;
        lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;
        if (lastanim->numpics < 2)
        {
            I_Error("P_InitPicAnims: bad cycle from %s to %s",
                    startname, endname);
        }
        lastanim->speed = animdefs[i].speed;
        lastanim++;
    }
}

/*
==============================================================================

							UTILITIES

==============================================================================
*/

//
//      Will return a side_t* given the number of the current sector,
//              the line number, and the side (0/1) that you want.
//
side_t *getSide(int currentSector, int line, int side)
{
    return &sides[(sectors[currentSector].lines[line])->sidenum[side]];
}

//
//      Will return a sector_t* given the number of the current sector,
//              the line number and the side (0/1) that you want.
//
sector_t *getSector(int currentSector, int line, int side)
{
    return sides[(sectors[currentSector].lines[line])->sidenum[side]].sector;
}

//
//      Given the sector number and the line number, will tell you whether
//              the line is two-sided or not.
//
int twoSided(int sector, int line)
{
    return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

//==================================================================
//
//      Return sector_t * of sector next to current. NULL if not two-sided line
//
//==================================================================
sector_t *getNextSector(line_t * line, sector_t * sec)
{
    if (!(line->flags & ML_TWOSIDED))
        return NULL;

    if (line->frontsector == sec)
        return line->backsector;

    return line->frontsector;
}

//==================================================================
//
//      FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindLowestFloorSurrounding(sector_t * sec)
{
    int i;
    line_t *check;
    sector_t *other;
    fixed_t floor = sec->floorheight;

    for (i = 0; i < sec->linecount; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check, sec);
        if (!other)
            continue;
        if (other->floorheight < floor)
            floor = other->floorheight;
    }
    return floor;
}

//==================================================================
//
//      FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindHighestFloorSurrounding(sector_t * sec)
{
    int i;
    line_t *check;
    sector_t *other;
    fixed_t floor = -500 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check, sec);
        if (!other)
            continue;
        if (other->floorheight > floor)
            floor = other->floorheight;
    }
    return floor;
}

//==================================================================
//
//      FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindNextHighestFloor(sector_t * sec, int currentheight)
{
    int i;
    int h;
    fixed_t min;
    line_t *check;
    sector_t *other;
    fixed_t height = currentheight;

    min = INT_MAX;

    for (i = 0, h = 0; i < sec->linecount; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check, sec);

        if (other != NULL && other->floorheight > height)
        {
            if (other->floorheight < min)
            {
                min = other->floorheight;
            }

            ++h;
        }
    }

    // Compatibility note, in case of demo desyncs.

    if (h > 20)
    {
        fprintf(stderr, "P_FindNextHighestFloor: exceeded Vanilla limit\n");
    }

    return min;
}

//==================================================================
//
//      FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindLowestCeilingSurrounding(sector_t * sec)
{
    int i;
    line_t *check;
    sector_t *other;
    fixed_t height = INT_MAX;

    for (i = 0; i < sec->linecount; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check, sec);
        if (!other)
            continue;
        if (other->ceilingheight < height)
            height = other->ceilingheight;
    }
    return height;
}

//==================================================================
//
//      FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindHighestCeilingSurrounding(sector_t * sec)
{
    int i;
    line_t *check;
    sector_t *other;
    fixed_t height = 0;

    for (i = 0; i < sec->linecount; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check, sec);
        if (!other)
            continue;
        if (other->ceilingheight > height)
            height = other->ceilingheight;
    }
    return height;
}

//==================================================================
//
//      RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
//==================================================================
int P_FindSectorFromLineTag(line_t * line, int start)
{
    int i;

    for (i = start + 1; i < numsectors; i++)
        if (sectors[i].tag == line->tag)
            return i;
    return -1;
}

//==================================================================
//
//      Find minimum light from an adjacent sector
//
//==================================================================
int P_FindMinSurroundingLight(sector_t * sector, int max)
{
    int i;
    int min;
    line_t *line;
    sector_t *check;

    min = max;
    for (i = 0; i < sector->linecount; i++)
    {
        line = sector->lines[i];
        check = getNextSector(line, sector);
        if (!check)
            continue;
        if (check->lightlevel < min)
            min = check->lightlevel;
    }
    return min;
}

/*
==============================================================================

							EVENTS

Events are operations triggered by using, crossing, or shooting special lines, or by timed thinkers

==============================================================================
*/



/*
===============================================================================
=
= P_CrossSpecialLine - TRIGGER
=
= Called every time a thing origin is about to cross
= a line with a non 0 special
=
===============================================================================
*/

void P_CrossSpecialLine(int linenum, int side, mobj_t * thing)
{
    line_t *line;

    line = &lines[linenum];
    if (!thing->player)
    {                           // Check if trigger allowed by non-player mobj
        switch (line->special)
        {
            case 39:           // Trigger_TELEPORT
            case 97:           // Retrigger_TELEPORT
            case 4:            // Trigger_Raise_Door
                //case 10:      // PLAT DOWN-WAIT-UP-STAY TRIGGER
                //case 88:      // PLAT DOWN-WAIT-UP-STAY RETRIGGER
                break;
            default:
                return;
                break;
        }
    }
    switch (line->special)
    {
            //====================================================
            // TRIGGERS
            //====================================================
        case 2:                // Open Door
            EV_DoDoor(line, vld_open, VDOORSPEED);
            line->special = 0;
            break;
        case 3:                // Close Door
            EV_DoDoor(line, vld_close, VDOORSPEED);
            line->special = 0;
            break;
        case 4:                // Raise Door
            EV_DoDoor(line, vld_normal, VDOORSPEED);
            line->special = 0;
            break;
        case 5:                // Raise Floor
            EV_DoFloor(line, raiseFloor);
            line->special = 0;
            break;
        case 6:                // Fast Ceiling Crush & Raise
            EV_DoCeiling(line, fastCrushAndRaise);
            line->special = 0;
            break;
        case 8:                // Trigger_Build_Stairs (8 pixel steps)
            EV_BuildStairs(line, 8 * FRACUNIT);
            line->special = 0;
            break;
        case 106:              // Trigger_Build_Stairs_16 (16 pixel steps)
            EV_BuildStairs(line, 16 * FRACUNIT);
            line->special = 0;
            break;
        case 10:               // PlatDownWaitUp
            EV_DoPlat(line, downWaitUpStay, 0);
            line->special = 0;
            break;
        case 12:               // Light Turn On - brightest near
            EV_LightTurnOn(line, 0);
            line->special = 0;
            break;
        case 13:               // Light Turn On 255
            EV_LightTurnOn(line, 255);
            line->special = 0;
            break;
        case 16:               // Close Door 30
            EV_DoDoor(line, vld_close30ThenOpen, VDOORSPEED);
            line->special = 0;
            break;
        case 17:               // Start Light Strobing
            EV_StartLightStrobing(line);
            line->special = 0;
            break;
        case 19:               // Lower Floor
            EV_DoFloor(line, lowerFloor);
            line->special = 0;
            break;
        case 22:               // Raise floor to nearest height and change texture
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            line->special = 0;
            break;
        case 25:               // Ceiling Crush and Raise
            EV_DoCeiling(line, crushAndRaise);
            line->special = 0;
            break;
        case 30:               // Raise floor to shortest texture height
            // on either side of lines
            EV_DoFloor(line, raiseToTexture);
            line->special = 0;
            break;
        case 35:               // Lights Very Dark
            EV_LightTurnOn(line, 35);
            line->special = 0;
            break;
        case 36:               // Lower Floor (TURBO)
            EV_DoFloor(line, turboLower);
            line->special = 0;
            break;
        case 37:               // LowerAndChange
            EV_DoFloor(line, lowerAndChange);
            line->special = 0;
            break;
        case 38:               // Lower Floor To Lowest
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
            break;
        case 39:               // TELEPORT!
            EV_Teleport(line, side, thing);
            line->special = 0;
            break;
        case 40:               // RaiseCeilingLowerFloor
            EV_DoCeiling(line, raiseToHighest);
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
            break;
        case 44:               // Ceiling Crush
            EV_DoCeiling(line, lowerAndCrush);
            line->special = 0;
            break;
        case 52:               // EXIT!
            G_ExitLevel();
            line->special = 0;
            break;
        case 53:               // Perpetual Platform Raise
            EV_DoPlat(line, perpetualRaise, 0);
            line->special = 0;
            break;
        case 54:               // Platform Stop
            EV_StopPlat(line);
            line->special = 0;
            break;
        case 56:               // Raise Floor Crush
            EV_DoFloor(line, raiseFloorCrush);
            line->special = 0;
            break;
        case 57:               // Ceiling Crush Stop
            EV_CeilingCrushStop(line);
            line->special = 0;
            break;
        case 58:               // Raise Floor 24
            EV_DoFloor(line, raiseFloor24);
            line->special = 0;
            break;
        case 59:               // Raise Floor 24 And Change
            EV_DoFloor(line, raiseFloor24AndChange);
            line->special = 0;
            break;
        case 104:              // Turn lights off in sector(tag)
            EV_TurnTagLightsOff(line);
            line->special = 0;
            break;
        case 105:              // Trigger_SecretExit
            G_SecretExitLevel();
            line->special = 0;
            break;

            //====================================================
            // RE-DOABLE TRIGGERS
            //====================================================

        case 72:               // Ceiling Crush
            EV_DoCeiling(line, lowerAndCrush);
            break;
        case 73:               // Ceiling Crush and Raise
            EV_DoCeiling(line, crushAndRaise);
            break;
        case 74:               // Ceiling Crush Stop
            EV_CeilingCrushStop(line);
            break;
        case 75:               // Close Door
            EV_DoDoor(line, vld_close, VDOORSPEED);
            break;
        case 76:               // Close Door 30
            EV_DoDoor(line, vld_close30ThenOpen, VDOORSPEED);
            break;
        case 77:               // Fast Ceiling Crush & Raise
            EV_DoCeiling(line, fastCrushAndRaise);
            break;
        case 79:               // Lights Very Dark
            EV_LightTurnOn(line, 35);
            break;
        case 80:               // Light Turn On - brightest near
            EV_LightTurnOn(line, 0);
            break;
        case 81:               // Light Turn On 255
            EV_LightTurnOn(line, 255);
            break;
        case 82:               // Lower Floor To Lowest
            EV_DoFloor(line, lowerFloorToLowest);
            break;
        case 83:               // Lower Floor
            EV_DoFloor(line, lowerFloor);
            break;
        case 84:               // LowerAndChange
            EV_DoFloor(line, lowerAndChange);
            break;
        case 86:               // Open Door
            EV_DoDoor(line, vld_open, VDOORSPEED);
            break;
        case 87:               // Perpetual Platform Raise
            EV_DoPlat(line, perpetualRaise, 0);
            break;
        case 88:               // PlatDownWaitUp
            EV_DoPlat(line, downWaitUpStay, 0);
            break;
        case 89:               // Platform Stop
            EV_StopPlat(line);
            break;
        case 90:               // Raise Door
            EV_DoDoor(line, vld_normal, VDOORSPEED);
            break;
        case 100:              // Retrigger_Raise_Door_Turbo
            EV_DoDoor(line, vld_normal, VDOORSPEED * 3);
            break;
        case 91:               // Raise Floor
            EV_DoFloor(line, raiseFloor);
            break;
        case 92:               // Raise Floor 24
            EV_DoFloor(line, raiseFloor24);
            break;
        case 93:               // Raise Floor 24 And Change
            EV_DoFloor(line, raiseFloor24AndChange);
            break;
        case 94:               // Raise Floor Crush
            EV_DoFloor(line, raiseFloorCrush);
            break;
        case 95:               // Raise floor to nearest height and change texture
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            break;
        case 96:               // Raise floor to shortest texture height
            // on either side of lines
            EV_DoFloor(line, raiseToTexture);
            break;
        case 97:               // TELEPORT!
            EV_Teleport(line, side, thing);
            break;
        case 98:               // Lower Floor (TURBO)
            EV_DoFloor(line, turboLower);
            break;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_ShootSpecialLine
//
// Called when a thing shoots a special line.
//
//----------------------------------------------------------------------------

void P_ShootSpecialLine(mobj_t * thing, line_t * line)
{
    if (!thing->player)
    {                           // Check if trigger allowed by non-player mobj
        switch (line->special)
        {
            case 46:           // Impact_OpenDoor
                break;
            default:
                return;
                break;
        }
    }
    switch (line->special)
    {
        case 24:               // Impact_RaiseFloor
            EV_DoFloor(line, raiseFloor);
            P_ChangeSwitchTexture(line, 0);
            break;
        case 46:               // Impact_OpenDoor
            EV_DoDoor(line, vld_open, VDOORSPEED);
            P_ChangeSwitchTexture(line, 1);
            break;
        case 47:               // Impact_RaiseFloorNear&Change
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            P_ChangeSwitchTexture(line, 0);
            break;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerInSpecialSector
//
// Called every tic frame that the player origin is in a special sector.
//
//----------------------------------------------------------------------------

void P_PlayerInSpecialSector(player_t * player)
{
    sector_t *sector;
    static int pushTab[5] = {
        2048 * 5,
        2048 * 10,
        2048 * 25,
        2048 * 30,
        2048 * 35
    };

    sector = player->mo->subsector->sector;
    if (player->mo->z != sector->floorheight)
    {                           // Player is not touching the floor
        return;
    }
    switch (sector->special)
    {
        case 7:                // Damage_Sludge
            if (!(leveltime & 31))
            {
                P_DamageMobj(player->mo, NULL, NULL, 4);
            }
            break;
        case 5:                // Damage_LavaWimpy
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 5);
                P_HitFloor(player->mo);
            }
            break;
        case 16:               // Damage_LavaHefty
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 8);
                P_HitFloor(player->mo);
            }
            break;
        case 4:                // Scroll_EastLavaDamage
            P_Thrust(player, 0, 2048 * 28);
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 5);
                P_HitFloor(player->mo);
            }
            break;
        case 9:                // SecretArea
            player->secretcount++;
            sector->special = 0;
            break;
        case 11:               // Exit_SuperDamage (DOOM E1M8 finale)
            /*
               player->cheats &= ~CF_GODMODE;
               if(!(leveltime&0x1f))
               {
               P_DamageMobj(player->mo, NULL, NULL, 20);
               }
               if(player->health <= 10)
               {
               G_ExitLevel();
               }
             */
            break;

        case 25:
        case 26:
        case 27:
        case 28:
        case 29:               // Scroll_North
            P_Thrust(player, ANG90, pushTab[sector->special - 25]);
            break;
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:               // Scroll_East
            P_Thrust(player, 0, pushTab[sector->special - 20]);
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:               // Scroll_South
            P_Thrust(player, ANG270, pushTab[sector->special - 30]);
            break;
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:               // Scroll_West
            P_Thrust(player, ANG180, pushTab[sector->special - 35]);
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
            // Wind specials are handled in (P_mobj):P_XYMovement
            break;

        case 15:               // Friction_Low
            // Only used in (P_mobj):P_XYMovement and (P_user):P_Thrust
            break;

        default:
            I_Error("P_PlayerInSpecialSector: "
                    "unknown special %i", sector->special);
    }
}

//----------------------------------------------------------------------------
//
// PROC P_UpdateSpecials
//
// Animate planes, scroll walls, etc.
//
//----------------------------------------------------------------------------

void P_UpdateSpecials(void)
{
    int i;
    int pic;
    anim_t *anim;
    line_t *line;

    // Animate flats and textures
    for (anim = anims; anim < lastanim; anim++)
    {
        for (i = anim->basepic; i < anim->basepic + anim->numpics; i++)
        {
            pic =
                anim->basepic +
                ((leveltime / anim->speed + i) % anim->numpics);
            if (anim->istexture)
            {
                texturetranslation[i] = pic;
            }
            else
            {
                flattranslation[i] = pic;
            }
        }
    }
    // Update scrolling texture offsets
    for (i = 0; i < numlinespecials; i++)
    {
        line = linespeciallist[i];
        switch (line->special)
        {
            case 48:           // Effect_Scroll_Left
                sides[line->sidenum[0]].textureoffset += FRACUNIT;
                break;
            case 99:           // Effect_Scroll_Right
                sides[line->sidenum[0]].textureoffset -= FRACUNIT;
                break;
        }
    }
    // Handle buttons
    for (i = 0; i < MAXBUTTONS; i++)
    {
        if (buttonlist[i].btimer)
        {
            buttonlist[i].btimer--;
            if (!buttonlist[i].btimer)
            {
                switch (buttonlist[i].where)
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
                S_StartSound(buttonlist[i].soundorg, sfx_switch);
                memset(&buttonlist[i], 0, sizeof(button_t));
            }
        }
    }
}

//============================================================
//
//      Special Stuff that can't be categorized
//
//============================================================
int EV_DoDonut(line_t * line)
{
    sector_t *s1;
    sector_t *s2;
    sector_t *s3;
    int secnum;
    int rtn;
    int i;
    floormove_t *floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        s1 = &sectors[secnum];

        //      ALREADY MOVING?  IF SO, KEEP GOING...
        if (s1->specialdata)
            continue;

        rtn = 1;
        s2 = getNextSector(s1->lines[0], s1);
        for (i = 0; i < s2->linecount; i++)
        {
            // Note: This was originally part of the following test:
            //   (!s2->lines[i]->flags & ML_TWOSIDED) ||
            // Due to the apparent mistaken formatting, this can never be
            // true.

            if (s2->lines[i]->backsector == s1)
                continue;
            s3 = s2->lines[i]->backsector;

            //
            //      Spawn rising slime
            //
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            P_AddThinker(&floor->thinker);
            s2->specialdata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = donutRaise;
            floor->crush = false;
            floor->direction = 1;
            floor->sector = s2;
            floor->speed = FLOORSPEED / 2;
            floor->texture = s3->floorpic;
            floor->newspecial = 0;
            floor->floordestheight = s3->floorheight;

            //
            //      Spawn lowering donut-hole
            //
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            P_AddThinker(&floor->thinker);
            s1->specialdata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = lowerFloor;
            floor->crush = false;
            floor->direction = -1;
            floor->sector = s1;
            floor->speed = FLOORSPEED / 2;
            floor->floordestheight = s3->floorheight;
            break;
        }
    }
    return rtn;
}

/*
==============================================================================

							SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecials
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/

short numlinespecials;
line_t *linespeciallist[MAXLINEANIMS];

void P_SpawnSpecials(void)
{
    sector_t *sector;
    int i;

    //
    //      Init special SECTORs
    //
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;
        switch (sector->special)
        {
            case 1:            // FLICKERING LIGHTS
                P_SpawnLightFlash(sector);
                break;
            case 2:            // STROBE FAST
                P_SpawnStrobeFlash(sector, FASTDARK, 0);
                break;
            case 3:            // STROBE SLOW
                P_SpawnStrobeFlash(sector, SLOWDARK, 0);
                break;
            case 4:            // STROBE FAST/DEATH SLIME
                P_SpawnStrobeFlash(sector, FASTDARK, 0);
                sector->special = 4;
                break;
            case 8:            // GLOWING LIGHT
                P_SpawnGlowingLight(sector);
                break;
            case 9:            // SECRET SECTOR
                totalsecret++;
                break;
            case 10:           // DOOR CLOSE IN 30 SECONDS
                P_SpawnDoorCloseIn30(sector);
                break;
            case 12:           // SYNC STROBE SLOW
                P_SpawnStrobeFlash(sector, SLOWDARK, 1);
                break;
            case 13:           // SYNC STROBE FAST
                P_SpawnStrobeFlash(sector, FASTDARK, 1);
                break;
            case 14:           // DOOR RAISE IN 5 MINUTES
                P_SpawnDoorRaiseIn5Mins(sector, i);
                break;
        }
    }


    //
    //      Init line EFFECTs
    //
    numlinespecials = 0;
    for (i = 0; i < numlines; i++)
        switch (lines[i].special)
        {
            case 48:           // Effect_Scroll_Left
            case 99:           // Effect_Scroll_Right
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
        }

    //
    //      Init other misc stuff
    //
    for (i = 0; i < MAXCEILINGS; i++)
        activeceilings[i] = NULL;
    for (i = 0; i < MAXPLATS; i++)
        activeplats[i] = NULL;
    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));
}

//----------------------------------------------------------------------------
//
// PROC P_InitAmbientSound
//
//----------------------------------------------------------------------------

void P_InitAmbientSound(void)
{
    AmbSfxCount = 0;
    AmbSfxVolume = 0;
    AmbSfxTics = 10 * TICRATE;
    AmbSfxPtr = AmbSndSeqInit;
}

//----------------------------------------------------------------------------
//
// PROC P_AddAmbientSfx
//
// Called by (P_mobj):P_SpawnMapThing during (P_setup):P_SetupLevel.
//
//----------------------------------------------------------------------------

void P_AddAmbientSfx(int sequence)
{
    if (AmbSfxCount == MAX_AMBIENT_SFX)
    {
        I_Error("Too many ambient sound sequences");
    }
    LevelAmbientSfx[AmbSfxCount++] = AmbientSfx[sequence];
}

//----------------------------------------------------------------------------
//
// PROC P_AmbientSound
//
// Called every tic by (P_tick):P_Ticker.
//
//----------------------------------------------------------------------------

void P_AmbientSound(void)
{
    afxcmd_t cmd;
    int sound;
    boolean done;

    if (!AmbSfxCount)
    {                           // No ambient sound sequences on current level
        return;
    }
    if (--AmbSfxTics)
    {
        return;
    }
    done = false;
    do
    {
        cmd = *AmbSfxPtr++;
        switch (cmd)
        {
            case afxcmd_play:
                AmbSfxVolume = P_Random() >> 2;
                S_StartSoundAtVolume(NULL, *AmbSfxPtr++, AmbSfxVolume);
                break;
            case afxcmd_playabsvol:
                sound = *AmbSfxPtr++;
                AmbSfxVolume = *AmbSfxPtr++;
                S_StartSoundAtVolume(NULL, sound, AmbSfxVolume);
                break;
            case afxcmd_playrelvol:
                sound = *AmbSfxPtr++;
                AmbSfxVolume += *AmbSfxPtr++;
                if (AmbSfxVolume < 0)
                {
                    AmbSfxVolume = 0;
                }
                else if (AmbSfxVolume > 127)
                {
                    AmbSfxVolume = 127;
                }
                S_StartSoundAtVolume(NULL, sound, AmbSfxVolume);
                break;
            case afxcmd_delay:
                AmbSfxTics = *AmbSfxPtr++;
                done = true;
                break;
            case afxcmd_delayrand:
                AmbSfxTics = P_Random() & (*AmbSfxPtr++);
                done = true;
                break;
            case afxcmd_end:
                AmbSfxTics = 6 * TICRATE + P_Random();
                AmbSfxPtr = LevelAmbientSfx[P_Random() % AmbSfxCount];
                done = true;
                break;
            default:
                I_Error("P_AmbientSound: Unknown afxcmd %d", cmd);
                break;
        }
    }
    while (done == false);
}
