// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
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
//-----------------------------------------------------------------------------
//
// Parses "Frame" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "info.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "deh_mapping.h"
#include "deh_htic.h"

#include "p_action.h"

// Offsets of action pointers within the Heretic 1.0 executable.
// (Seriously Greg, was this really necessary?  What was wrong with the
// "copying action pointer from another frame" technique used in dehacked?)

static const struct
{
    int offset;
    void (*func)();
} action_ptrs[] = {
    { 0, NULL },
    { 69200, A_Look },
    { 69328, A_Chase },
    { 69872, A_FaceTarget },
    { 69984, A_Pain },
    { 70016, A_DripBlood },
    { 70160, A_KnightAttack },
    { 70304, A_ImpExplode },
    { 70480, A_BeastPuff },
    { 70592, A_ImpMeAttack },
    { 70672, A_ImpMsAttack },
    { 70880, A_ImpMsAttack2 },
    { 70976, A_ImpDeath },
    { 71024, A_ImpXDeath1 },
    { 71072, A_ImpXDeath2 },
    { 71376, A_ChicAttack },
    { 71456, A_ChicLook },
    { 71488, A_ChicChase },
    { 71520, A_ChicPain },
    { 71568, A_Feathers },
    { 71808, A_MummyAttack },
    { 71920, A_MummyAttack2 },
    { 72016, A_MummyFX1Seek },
    { 72048, A_MummySoul },
    { 72096, A_Sor1Pain },
    { 72144, A_Sor1Chase },
    { 72192, A_Srcr1Attack },
    { 72480, A_SorcererRise },
    { 72816, A_Srcr2Decide },
    { 72896, A_Srcr2Attack },
    { 73120, A_BlueSpark },
    { 73232, A_GenWizard },
    { 73392, A_Sor2DthInit },
    { 73424, A_Sor2DthLoop },
    { 73456, A_SorZap },
    { 73488, A_SorRise },
    { 73520, A_SorDSph },
    { 73552, A_SorDExp },
    { 73584, A_SorDBon },
    { 73616, A_SorSightSnd },
    { 73648, A_MinotaurAtk1 },
    { 73760, A_MinotaurDecide },
    { 74032, A_MinotaurCharge },
    { 74112, A_MinotaurAtk2 },
    { 74352, A_MinotaurAtk3 },
    { 74528, A_MntrFloorFire },
    { 74640, A_BeastAttack },
    { 74752, A_HeadAttack },
    { 75168, A_WhirlwindSeek },
    { 75328, A_HeadIceImpact },
    { 75488, A_HeadFireGrow },
    { 75632, A_SnakeAttack },
    { 75712, A_SnakeAttack2 },
    { 75792, A_ClinkAttack },
    { 75872, A_GhostOff },
    { 75888, A_WizAtk1 },
    { 75920, A_WizAtk2 },
    { 75952, A_WizAtk3 },
    { 76144, A_Scream },
    { 76400, A_NoBlocking },
    { 76784, A_Explode },
    { 76896, A_PodPain },
    { 77056, A_RemovePod },
    { 77104, A_MakePod },
    { 77344, A_BossDeath },
    { 77472, A_ESound },
    { 77520, A_SpawnTeleGlitter },
    { 77600, A_SpawnTeleGlitter2 },
    { 77680, A_AccTeleGlitter },
    { 77728, A_InitKeyGizmo },
    { 77824, A_VolcanoSet },
    { 77856, A_VolcanoBlast },
    { 78080, A_VolcBallImpact },
    { 78288, A_SkullPop },
    { 78448, A_CheckSkullFloor },
    { 78480, A_CheckSkullDone },
    { 78512, A_FreeTargMobj },
    { 78608, A_AddPlayerCorpse },
    { 78704, A_FlameSnd },
    { 78736, A_HideThing },
    { 78752, A_UnHideThing },
    { 81952, A_RestoreArtifact },
    { 82048, A_RestoreSpecialThing1 },
    { 82128, A_RestoreSpecialThing2 },
    { 108432, A_ContMobjSound },
    { 111168, A_WeaponReady },
    { 111568, A_BeakReady },
    { 111696, A_ReFire },
    { 111760, A_Lower },
    { 111856, A_BeakRaise },
    { 111920, A_Raise },
    { 112272, A_BeakAttackPL1 },
    { 112448, A_BeakAttackPL2 },
    { 112640, A_StaffAttackPL1 },
    { 112784, A_StaffAttackPL2 },
    { 112928, A_FireBlasterPL1 },
    { 113072, A_FireBlasterPL2 },
    { 113152, A_FireGoldWandPL1 },
    { 113296, A_FireGoldWandPL2 },
    { 113760, A_FireMacePL1 },
    { 113904, A_MacePL1Check },
    { 114032, A_MaceBallImpact },
    { 114192, A_MaceBallImpact2 },
    { 114624, A_FireMacePL2 },
    { 114752, A_DeathBallImpact },
    { 115088, A_SpawnRippers },
    { 115232, A_FireCrossbowPL1 },
    { 115312, A_FireCrossbowPL2 },
    { 115456, A_BoltSpark },
    { 115568, A_FireSkullRodPL1 },
    { 115648, A_FireSkullRodPL2 },
    { 115776, A_SkullRodPL2Seek },
    { 115808, A_AddPlayerRain },
    { 115984, A_SkullRodStorm },
    { 116272, A_RainImpact },
    { 116336, A_HideInCeiling },
    { 116368, A_FirePhoenixPL1 },
    { 116480, A_RemovedPhoenixFunc },
    { 116496, A_PhoenixPuff },
    { 116720, A_InitPhoenixPL2 },
    { 116736, A_FirePhoenixPL2 },
    { 117104, A_ShutdownPhoenixPL2 },
    { 117120, A_FlameEnd },
    { 117152, A_FloatPuff },
    { 117184, A_GauntletAttack },
    { 117648, A_Light0 }
};

DEH_BEGIN_MAPPING(state_mapping, state_t)
  DEH_MAPPING("Sprite number",    sprite)
  DEH_MAPPING("Sprite subnumber", frame)
  DEH_MAPPING("Duration",         tics)
  DEH_MAPPING("Next frame",       nextstate)
  DEH_MAPPING("Unknown 1",        misc1)
  DEH_MAPPING("Unknown 2",        misc2)
DEH_END_MAPPING

// When a HHE patch is first loaded, we must apply a small change
// to the states[] table.  The table was changed between 1.0 and
// 1.3 to add two extra frames to the player "burning death"
// animation.
// If we are using an HHE patch, the table must behave like the
// Heretic 1.0 table.  We must therefore change the table to cut
// these out again.

static void DEH_FrameInit(void)
{
    states[S_PLAY_FDTH18].nextstate = S_NULL;
}

static void *DEH_FrameStart(deh_context_t *context, char *line)
{
    int frame_number = 0;
    int mapped_frame_number;
    state_t *state;

    if (sscanf(line, "Frame %i", &frame_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    // Map the HHE frame number (which assumes a Heretic 1.0 state table)
    // to the internal frame number (which is is the Heretic 1.3 state table):

    mapped_frame_number = DEH_MapHereticFrameNumber(frame_number);

    if (mapped_frame_number < 0 || mapped_frame_number >= DEH_HERETIC_NUMSTATES)
    {
        DEH_Warning(context, "Invalid frame number: %i", frame_number);
        return NULL;
    }

    state = &states[mapped_frame_number];

    return state;
}

static boolean GetActionPointerForOffset(int offset, void **result)
{
    int i;

    for (i=0; i<arrlen(action_ptrs); ++i)
    {
        if (action_ptrs[i].offset == offset)
        {
            *result = action_ptrs[i].func;
            return true;
        }
    }

    return false;
}

static void DEH_FrameParseLine(deh_context_t *context, char *line, void *tag)
{
    state_t *state;
    char *variable_name, *value;
    int ivalue;

    if (tag == NULL)
       return;

    state = (state_t *) tag;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    // all values are integers

    ivalue = atoi(value);

    // Action pointer field is a special case:

    if (!strcasecmp(variable_name, "Action pointer"))
    {
        void *func;

        if (!GetActionPointerForOffset(ivalue, &func))
        {
            DEH_Warning(context, "Unknown action pointer: %i", ivalue);
            return;
        }

        state->action = func;
    }
    else
    {
        // "Next frame" numbers need to undergo mapping.

        if (!strcasecmp(variable_name, "Next frame"))
        {
            ivalue = DEH_MapHereticFrameNumber(ivalue);
        }

        DEH_SetMapping(context, &state_mapping, state, variable_name, ivalue);
    }
}

static void DEH_FrameMD5Sum(md5_context_t *context)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        DEH_StructMD5Sum(context, &state_mapping, &states[i]);
    }
}

deh_section_t deh_section_frame =
{
    "Frame",
    DEH_FrameInit,
    DEH_FrameStart,
    DEH_FrameParseLine,
    NULL,
    DEH_FrameMD5Sum,
};

