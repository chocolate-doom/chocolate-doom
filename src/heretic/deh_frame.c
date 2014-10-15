//
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
// Parses "Frame" sections in dehacked files
//

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

typedef struct
{
    int offsets[deh_hhe_num_versions];
    void (*func)();
} hhe_action_pointer_t;

// Offsets of action pointers within the Heretic executables.
// Different versions have different offsets.
// (Seriously Greg, was this really necessary?  What was wrong with the
// "copying action pointer from another frame" technique used in dehacked?)

//    Offset                      Action function
//        v1.0    v1.2    v1.3

static const hhe_action_pointer_t action_pointers[] =
{
    { {  77680,  80144,  80208 }, A_AccTeleGlitter },
    { {  78608,  81104,  81168 }, A_AddPlayerCorpse },
    { { 115808, 118000, 118240 }, A_AddPlayerRain },
    { { 112272, 114480, 114720 }, A_BeakAttackPL1 },
    { { 112448, 114656, 114896 }, A_BeakAttackPL2 },
    { { 111856, 114176, 114416 }, A_BeakRaise },
    { { 111568, 113888, 114128 }, A_BeakReady },
    { {  74640,  77120,  77184 }, A_BeastAttack },
    { {  70480,  72992,  73056 }, A_BeastPuff },
    { {  73120,  75600,  75664 }, A_BlueSpark },
    { { 115456, 117648, 117888 }, A_BoltSpark },
    { {  77344,  79808,  79872 }, A_BossDeath },
    { {  69328,  71856,  71920 }, A_Chase },
    { {      0,  80976,  81040 }, A_CheckBurnGone },
    { {  78480,  80944,  81008 }, A_CheckSkullDone },
    { {  78448,  80912,  80976 }, A_CheckSkullFloor },
    { {  71376,  73888,  73952 }, A_ChicAttack },
    { {  71488,  74000,  74064 }, A_ChicChase },
    { {  71456,  73968,  74032 }, A_ChicLook },
    { {  71520,  74032,  74096 }, A_ChicPain },
    { {  75792,  78208,  78272 }, A_ClinkAttack },
    { { 108432, 110816, 111056 }, A_ContMobjSound },
    { { 114752, 116944, 117184 }, A_DeathBallImpact },
    { {  70016,  72528,  72592 }, A_DripBlood },
    { {  77472,  79936,  80000 }, A_ESound },
    { {  76784,  79248,  79312 }, A_Explode },
    { {  69872,  72400,  72464 }, A_FaceTarget },
    { {  71568,  74080,  74144 }, A_Feathers },
    { { 112928, 115136, 115376 }, A_FireBlasterPL1 },
    { { 113072, 115280, 115520 }, A_FireBlasterPL2 },
    { { 115232, 117424, 117664 }, A_FireCrossbowPL1 },
    { { 115312, 117504, 117744 }, A_FireCrossbowPL2 },
    { { 113152, 115360, 115600 }, A_FireGoldWandPL1 },
    { { 113296, 115504, 115744 }, A_FireGoldWandPL2 },
    { { 113760, 115968, 116208 }, A_FireMacePL1 },
    { { 114624, 116816, 117056 }, A_FireMacePL2 },
    { { 116368, 118544, 118784 }, A_FirePhoenixPL1 },
    { { 116736, 118896, 119136 }, A_FirePhoenixPL2 },
    { { 115568, 117760, 118000 }, A_FireSkullRodPL1 },
    { { 115648, 117840, 118080 }, A_FireSkullRodPL2 },
    { { 117120, 119280, 119520 }, A_FlameEnd },
    { {  78704,  81200,  81264 }, A_FlameSnd },
    { { 117152, 119312, 119552 }, A_FloatPuff },
    { {  78512,  81008,  81072 }, A_FreeTargMobj },
    { { 117184, 119344, 119584 }, A_GauntletAttack },
    { {  73232,  75712,  75776 }, A_GenWizard },
    { {  75872,  78304,  78368 }, A_GhostOff },
    { {  74752,  77232,  77296 }, A_HeadAttack },
    { {  75488,  77984,  78048 }, A_HeadFireGrow },
    { {  75328,  77824,  77888 }, A_HeadIceImpact },
    { { 116336, 118512, 118752 }, A_HideInCeiling },
    { {  78736,  81232,  81296 }, A_HideThing },
    { {  70976,  73488,  73552 }, A_ImpDeath },
    { {  70304,  72816,  72880 }, A_ImpExplode },
    { {  70592,  73104,  73168 }, A_ImpMeAttack },
    { {  70672,  73184,  73248 }, A_ImpMsAttack },
    { {  70880,  73392,  73456 }, A_ImpMsAttack2 },
    { {  71024,  73536,  73600 }, A_ImpXDeath1 },
    { {  71072,  73584,  73648 }, A_ImpXDeath2 },
    { {  77728,  80192,  80256 }, A_InitKeyGizmo },
    { { 116720, 118880, 119120 }, A_InitPhoenixPL2 },
    { {  70160,  72672,  72736 }, A_KnightAttack },
    { { 117648, 119824, 120064 }, A_Light0 },
    { {  69200,  71728,  71792 }, A_Look },
    { { 111760, 114080, 114320 }, A_Lower },
    { { 114032, 116224, 116464 }, A_MaceBallImpact },
    { { 114192, 116384, 116624 }, A_MaceBallImpact2 },
    { { 113904, 116112, 116352 }, A_MacePL1Check },
    { {  77104,  79568,  79632 }, A_MakePod },
    { {  73648,  76128,  76192 }, A_MinotaurAtk1 },
    { {  74112,  76592,  76656 }, A_MinotaurAtk2 },
    { {  74352,  76832,  76896 }, A_MinotaurAtk3 },
    { {  74032,  76512,  76576 }, A_MinotaurCharge },
    { {  73760,  76240,  76304 }, A_MinotaurDecide },
    { {  74528,  77008,  77072 }, A_MntrFloorFire },
    { {  71808,  74288,  74352 }, A_MummyAttack },
    { {  71920,  74400,  74464 }, A_MummyAttack2 },
    { {  72016,  74496,  74560 }, A_MummyFX1Seek },
    { {  72048,  74528,  74592 }, A_MummySoul },
    { {  76400,  78832,  78896 }, A_NoBlocking },
    { {  69984,  72496,  72560 }, A_Pain },
    { { 116496, 118656, 118896 }, A_PhoenixPuff },
    { {  76896,  79360,  79424 }, A_PodPain },
    { { 116272, 118448, 118688 }, A_RainImpact },
    { { 111920, 114240, 114480 }, A_Raise },
    { { 111696, 114016, 114256 }, A_ReFire },
    { {  77056,  79520,  79584 }, A_RemovePod },
    { { 116480,      0,      0 }, A_RemovedPhoenixFunc },
    { {  81952,  84464,  84528 }, A_RestoreArtifact },
    { {  82048,  84544,  84608 }, A_RestoreSpecialThing1 },
    { {  82128,  84592,  84656 }, A_RestoreSpecialThing2 },
    { {  76144,  78576,  78640 }, A_Scream },
    { { 117104, 119264, 119504 }, A_ShutdownPhoenixPL2 },
    { {  78288,  80752,  80816 }, A_SkullPop },
    { { 115776, 117968, 118208 }, A_SkullRodPL2Seek },
    { { 115984, 118176, 118416 }, A_SkullRodStorm },
    { {  75632,  78048,  78112 }, A_SnakeAttack },
    { {  75712,  78128,  78192 }, A_SnakeAttack2 },
    { {  72144,  74624,  74688 }, A_Sor1Chase },
    { {  72096,  74576,  74640 }, A_Sor1Pain },
    { {  73392,  75872,  75936 }, A_Sor2DthInit },
    { {  73424,  75904,  75968 }, A_Sor2DthLoop },
    { {  73584,  76064,  76128 }, A_SorDBon },
    { {  73552,  76032,  76096 }, A_SorDExp },
    { {  73520,  76000,  76064 }, A_SorDSph },
    { {  73488,  75968,  76032 }, A_SorRise },
    { {  73616,  76096,  76160 }, A_SorSightSnd },
    { {  73456,  75936,  76000 }, A_SorZap },
    { {  72480,  74960,  75024 }, A_SorcererRise },
    { { 115088, 117280, 117520 }, A_SpawnRippers },
    { {  77520,  79984,  80048 }, A_SpawnTeleGlitter },
    { {  77600,  80064,  80128 }, A_SpawnTeleGlitter2 },
    { {  72192,  74672,  74736 }, A_Srcr1Attack },
    { {  72896,  75376,  75440 }, A_Srcr2Attack },
    { {  72816,  75296,  75360 }, A_Srcr2Decide },
    { { 112640, 114848, 115088 }, A_StaffAttackPL1 },
    { { 112784, 114992, 115232 }, A_StaffAttackPL2 },
    { {  78752,  81248,  81312 }, A_UnHideThing },
    { {  78080,  80544,  80608 }, A_VolcBallImpact },
    { {  77856,  80320,  80384 }, A_VolcanoBlast },
    { {  77824,  80288,  80352 }, A_VolcanoSet },
    { { 111168, 113488, 113728 }, A_WeaponReady },
    { {  75168,  77664,  77728 }, A_WhirlwindSeek },
    { {  75888,  78320,  78384 }, A_WizAtk1 },
    { {  75920,  78352,  78416 }, A_WizAtk2 },
    { {  75952,  78384,  78448 }, A_WizAtk3 },
};

DEH_BEGIN_MAPPING(state_mapping, state_t)
  DEH_MAPPING("Sprite number",    sprite)
  DEH_MAPPING("Sprite subnumber", frame)
  DEH_MAPPING("Duration",         tics)
  DEH_MAPPING("Next frame",       nextstate)
  DEH_MAPPING("Unknown 1",        misc1)
  DEH_MAPPING("Unknown 2",        misc2)
DEH_END_MAPPING

static void DEH_FrameInit(void)
{
    // Bit of a hack here:
    DEH_HereticInit();
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

    // Special case.

    if (offset == 0)
    {
        *result = NULL;
        return true;
    }

    for (i=0; i<arrlen(action_pointers); ++i)
    {
        if (action_pointers[i].offsets[deh_hhe_version] == offset)
        {
            *result = action_pointers[i].func;
            return true;
        }
    }

    return false;
}

// If an invalid action pointer is specified, the patch may be for a
// different version from the version we are currently set to.  Try to
// suggest a different version to use.

static void SuggestOtherVersions(unsigned int offset)
{
    unsigned int i, v;

    for (i=0; i<arrlen(action_pointers); ++i)
    {
        for (v=0; v<deh_hhe_num_versions; ++v)
        {
            if (action_pointers[i].offsets[v] == offset)
            {
                DEH_SuggestHereticVersion(v);
            }
        }
    }
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
            SuggestOtherVersions(ivalue);
            DEH_Error(context, "Unknown action pointer: %i", ivalue);
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

static void DEH_FrameSHA1Sum(sha1_context_t *context)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        DEH_StructSHA1Sum(context, &state_mapping, &states[i]);
    }
}

deh_section_t deh_section_frame =
{
    "Frame",
    DEH_FrameInit,
    DEH_FrameStart,
    DEH_FrameParseLine,
    NULL,
    DEH_FrameSHA1Sum,
};

