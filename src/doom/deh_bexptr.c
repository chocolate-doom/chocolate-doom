//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath
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
// Parses [CODEPTR] sections in BEX files
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "info.h"

#include "deh_io.h"
#include "deh_main.h"

extern void A_Light0();
extern void A_WeaponReady();
extern void A_Lower();
extern void A_Raise();
extern void A_Punch();
extern void A_ReFire();
extern void A_FirePistol();
extern void A_Light1();
extern void A_FireShotgun();
extern void A_Light2();
extern void A_FireShotgun2();
extern void A_CheckReload();
extern void A_OpenShotgun2();
extern void A_LoadShotgun2();
extern void A_CloseShotgun2();
extern void A_FireCGun();
extern void A_GunFlash();
extern void A_FireMissile();
extern void A_Saw();
extern void A_FirePlasma();
extern void A_BFGsound();
extern void A_FireBFG();
extern void A_BFGSpray();
extern void A_Explode();
extern void A_Pain();
extern void A_PlayerScream();
extern void A_Fall();
extern void A_XScream();
extern void A_Look();
extern void A_Chase();
extern void A_FaceTarget();
extern void A_PosAttack();
extern void A_Scream();
extern void A_SPosAttack();
extern void A_VileChase();
extern void A_VileStart();
extern void A_VileTarget();
extern void A_VileAttack();
extern void A_StartFire();
extern void A_Fire();
extern void A_FireCrackle();
extern void A_Tracer();
extern void A_SkelWhoosh();
extern void A_SkelFist();
extern void A_SkelMissile();
extern void A_FatRaise();
extern void A_FatAttack1();
extern void A_FatAttack2();
extern void A_FatAttack3();
extern void A_BossDeath();
extern void A_CPosAttack();
extern void A_CPosRefire();
extern void A_TroopAttack();
extern void A_SargAttack();
extern void A_HeadAttack();
extern void A_BruisAttack();
extern void A_SkullAttack();
extern void A_Metal();
extern void A_SpidRefire();
extern void A_BabyMetal();
extern void A_BspiAttack();
extern void A_Hoof();
extern void A_CyberAttack();
extern void A_PainAttack();
extern void A_PainDie();
extern void A_KeenDie();
extern void A_BrainPain();
extern void A_BrainScream();
extern void A_BrainDie();
extern void A_BrainAwake();
extern void A_BrainSpit();
extern void A_SpawnSound();
extern void A_SpawnFly();
extern void A_BrainExplode();

typedef struct {
    char *mnemonic;
    actionf_t pointer;
} bex_codeptr_t;

static const bex_codeptr_t bex_codeptrtable[] = {
    {"Light0", {A_Light0}},
    {"WeaponReady", {A_WeaponReady}},
    {"Lower", {A_Lower}},
    {"Raise", {A_Raise}},
    {"Punch", {A_Punch}},
    {"ReFire", {A_ReFire}},
    {"FirePistol", {A_FirePistol}},
    {"Light1", {A_Light1}},
    {"FireShotgun", {A_FireShotgun}},
    {"Light2", {A_Light2}},
    {"FireShotgun2", {A_FireShotgun2}},
    {"CheckReload", {A_CheckReload}},
    {"OpenShotgun2", {A_OpenShotgun2}},
    {"LoadShotgun2", {A_LoadShotgun2}},
    {"CloseShotgun2", {A_CloseShotgun2}},
    {"FireCGun", {A_FireCGun}},
    {"GunFlash", {A_GunFlash}},
    {"FireMissile", {A_FireMissile}},
    {"Saw", {A_Saw}},
    {"FirePlasma", {A_FirePlasma}},
    {"BFGsound", {A_BFGsound}},
    {"FireBFG", {A_FireBFG}},
    {"BFGSpray", {A_BFGSpray}},
    {"Explode", {A_Explode}},
    {"Pain", {A_Pain}},
    {"PlayerScream", {A_PlayerScream}},
    {"Fall", {A_Fall}},
    {"XScream", {A_XScream}},
    {"Look", {A_Look}},
    {"Chase", {A_Chase}},
    {"FaceTarget", {A_FaceTarget}},
    {"PosAttack", {A_PosAttack}},
    {"Scream", {A_Scream}},
    {"SPosAttack", {A_SPosAttack}},
    {"VileChase", {A_VileChase}},
    {"VileStart", {A_VileStart}},
    {"VileTarget", {A_VileTarget}},
    {"VileAttack", {A_VileAttack}},
    {"StartFire", {A_StartFire}},
    {"Fire", {A_Fire}},
    {"FireCrackle", {A_FireCrackle}},
    {"Tracer", {A_Tracer}},
    {"SkelWhoosh", {A_SkelWhoosh}},
    {"SkelFist", {A_SkelFist}},
    {"SkelMissile", {A_SkelMissile}},
    {"FatRaise", {A_FatRaise}},
    {"FatAttack1", {A_FatAttack1}},
    {"FatAttack2", {A_FatAttack2}},
    {"FatAttack3", {A_FatAttack3}},
    {"BossDeath", {A_BossDeath}},
    {"CPosAttack", {A_CPosAttack}},
    {"CPosRefire", {A_CPosRefire}},
    {"TroopAttack", {A_TroopAttack}},
    {"SargAttack", {A_SargAttack}},
    {"HeadAttack", {A_HeadAttack}},
    {"BruisAttack", {A_BruisAttack}},
    {"SkullAttack", {A_SkullAttack}},
    {"Metal", {A_Metal}},
    {"SpidRefire", {A_SpidRefire}},
    {"BabyMetal", {A_BabyMetal}},
    {"BspiAttack", {A_BspiAttack}},
    {"Hoof", {A_Hoof}},
    {"CyberAttack", {A_CyberAttack}},
    {"PainAttack", {A_PainAttack}},
    {"PainDie", {A_PainDie}},
    {"KeenDie", {A_KeenDie}},
    {"BrainPain", {A_BrainPain}},
    {"BrainScream", {A_BrainScream}},
    {"BrainDie", {A_BrainDie}},
    {"BrainAwake", {A_BrainAwake}},
    {"BrainSpit", {A_BrainSpit}},
    {"SpawnSound", {A_SpawnSound}},
    {"SpawnFly", {A_SpawnFly}},
    {"BrainExplode", {A_BrainExplode}},
    {"NULL", {NULL}},
};

extern actionf_t codeptrs[NUMSTATES];

static int CodePointerIndex(actionf_t *ptr)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        if (!memcmp(&codeptrs[i], ptr, sizeof(actionf_t)))
        {
            return i;
        }
    }

    return -1;
}

static void *DEH_BEXPtrStart(deh_context_t *context, char *line)
{
    char s[10];

    if (sscanf(line, "%9s", s) == 0 || strncmp("[CODEPTR]", s, sizeof(s)))
    {
	DEH_Warning(context, "Parse error on section start");
    }

    return NULL;
}

static void DEH_BEXPtrParseLine(deh_context_t *context, char *line, void *tag)
{
    state_t *state;
    char *variable_name, *value;
    int frame_number, i;

    // parse "FRAME nn = mnemonic", where
    // variable_name = "FRAME nn" and value = "mnemonic"
    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
	DEH_Warning(context, "Failed to parse assignment");
	return;
    }

    // parse "FRAME nn", where frame_number = "nn"
    if (sscanf(variable_name, "FRAME %d", &frame_number) != 1)
    {
	DEH_Warning(context, "Failed to parse assignment");
	return;
    }

    if (frame_number < 0 || frame_number >= NUMSTATES)
    {
	DEH_Warning(context, "Invalid frame number: %i", frame_number);
	return;
    }

    state = (state_t *) &states[frame_number];

    for (i = 0; i < arrlen(bex_codeptrtable); i++)
    {
	if (!strcmp(bex_codeptrtable[i].mnemonic, value))
	{
	    state->action = bex_codeptrtable[i].pointer;
	    return;
	}
    }

    DEH_Warning(context, "Invalid mnemonic '%s'", value);
}

static void DEH_BEXPtrSHA1Sum(sha1_context_t *context)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        SHA1_UpdateInt32(context, CodePointerIndex(&states[i].action));
    }
}

deh_section_t deh_section_bexptr =
{
    "[CODEPTR]",
    NULL,
    DEH_BEXPtrStart,
    DEH_BEXPtrParseLine,
    NULL,
    DEH_BEXPtrSHA1Sum,
};
