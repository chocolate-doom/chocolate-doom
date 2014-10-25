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
// Parses [STRINGS] sections in BEX files
//

#include <stdio.h>
#include <string.h>

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"

#include "dstrings.h"

typedef struct {
    char *macro;
    char *string;
} bex_string_t;

// mnemonic keys table
static const bex_string_t bex_stringtable[] = {
    // part 1 - general initialization and prompts
    {"D_DEVSTR", D_DEVSTR},
    {"D_CDROM", D_CDROM},
    {"QUITMSG", QUITMSG},
    {"LOADNET", LOADNET},
    {"QLOADNET", QLOADNET},
    {"QSAVESPOT", QSAVESPOT},
    {"SAVEDEAD", SAVEDEAD},
    {"QSPROMPT", QSPROMPT},
    {"QLPROMPT", QLPROMPT},
    {"NEWGAME", NEWGAME},
    {"NIGHTMARE", NIGHTMARE},
    {"SWSTRING", SWSTRING},
    {"MSGOFF", MSGOFF},
    {"MSGON", MSGON},
    {"NETEND", NETEND},
    {"ENDGAME", ENDGAME},
    {"DETAILHI", DETAILHI},
    {"DETAILLO", DETAILLO},
    {"GAMMALVL0", GAMMALVL0},
    {"GAMMALVL1", GAMMALVL1},
    {"GAMMALVL2", GAMMALVL2},
    {"GAMMALVL3", GAMMALVL3},
    {"GAMMALVL4", GAMMALVL4},
    {"EMPTYSTRING", EMPTYSTRING},
    {"GGSAVED", GGSAVED},
    {"SAVEGAMENAME", SAVEGAMENAME},
    // part 2 - messages when the player gets things
    {"GOTARMOR", GOTARMOR},
    {"GOTMEGA", GOTMEGA},
    {"GOTHTHBONUS", GOTHTHBONUS},
    {"GOTARMBONUS", GOTARMBONUS},
    {"GOTSTIM", GOTSTIM},
    {"GOTMEDINEED", GOTMEDINEED},
    {"GOTMEDIKIT", GOTMEDIKIT},
    {"GOTSUPER", GOTSUPER},
    {"GOTBLUECARD", GOTBLUECARD},
    {"GOTYELWCARD", GOTYELWCARD},
    {"GOTREDCARD", GOTREDCARD},
    {"GOTBLUESKUL", GOTBLUESKUL},
    {"GOTYELWSKUL", GOTYELWSKUL},
    {"GOTREDSKULL", GOTREDSKULL},
    {"GOTINVUL", GOTINVUL},
    {"GOTBERSERK", GOTBERSERK},
    {"GOTINVIS", GOTINVIS},
    {"GOTSUIT", GOTSUIT},
    {"GOTMAP", GOTMAP},
    {"GOTVISOR", GOTVISOR},
    {"GOTMSPHERE", GOTMSPHERE},
    {"GOTCLIP", GOTCLIP},
    {"GOTCLIPBOX", GOTCLIPBOX},
    {"GOTROCKET", GOTROCKET},
    {"GOTROCKBOX", GOTROCKBOX},
    {"GOTCELL", GOTCELL},
    {"GOTCELLBOX", GOTCELLBOX},
    {"GOTSHELLS", GOTSHELLS},
    {"GOTSHELLBOX", GOTSHELLBOX},
    {"GOTBACKPACK", GOTBACKPACK},
    {"GOTBFG9000", GOTBFG9000},
    {"GOTCHAINGUN", GOTCHAINGUN},
    {"GOTCHAINSAW", GOTCHAINSAW},
    {"GOTLAUNCHER", GOTLAUNCHER},
    {"GOTPLASMA", GOTPLASMA},
    {"GOTSHOTGUN", GOTSHOTGUN},
    {"GOTSHOTGUN2", GOTSHOTGUN2},
    // part 3 - messages when keys are needed
    {"PD_BLUEO", PD_BLUEO},
    {"PD_REDO", PD_REDO},
    {"PD_YELLOWO", PD_YELLOWO},
    {"PD_BLUEK", PD_BLUEK},
    {"PD_REDK", PD_REDK},
    {"PD_YELLOWK", PD_YELLOWK},
    // part 4 - multiplayer messaging
    {"HUSTR_MSGU", HUSTR_MSGU},
    {"HUSTR_MESSAGESENT", HUSTR_MESSAGESENT},
    {"HUSTR_CHATMACRO0", HUSTR_CHATMACRO0},
    {"HUSTR_CHATMACRO1", HUSTR_CHATMACRO1},
    {"HUSTR_CHATMACRO2", HUSTR_CHATMACRO2},
    {"HUSTR_CHATMACRO3", HUSTR_CHATMACRO3},
    {"HUSTR_CHATMACRO4", HUSTR_CHATMACRO4},
    {"HUSTR_CHATMACRO5", HUSTR_CHATMACRO5},
    {"HUSTR_CHATMACRO6", HUSTR_CHATMACRO6},
    {"HUSTR_CHATMACRO7", HUSTR_CHATMACRO7},
    {"HUSTR_CHATMACRO8", HUSTR_CHATMACRO8},
    {"HUSTR_CHATMACRO9", HUSTR_CHATMACRO9},
    {"HUSTR_TALKTOSELF1", HUSTR_TALKTOSELF1},
    {"HUSTR_TALKTOSELF2", HUSTR_TALKTOSELF2},
    {"HUSTR_TALKTOSELF3", HUSTR_TALKTOSELF3},
    {"HUSTR_TALKTOSELF4", HUSTR_TALKTOSELF4},
    {"HUSTR_TALKTOSELF5", HUSTR_TALKTOSELF5},
    {"HUSTR_PLRGREEN", HUSTR_PLRGREEN},
    {"HUSTR_PLRINDIGO", HUSTR_PLRINDIGO},
    {"HUSTR_PLRBROWN", HUSTR_PLRBROWN},
    {"HUSTR_PLRRED", HUSTR_PLRRED},
    // part 5 - level names in the automap
    {"HUSTR_E1M1", HUSTR_E1M1},
    {"HUSTR_E1M2", HUSTR_E1M2},
    {"HUSTR_E1M3", HUSTR_E1M3},
    {"HUSTR_E1M4", HUSTR_E1M4},
    {"HUSTR_E1M5", HUSTR_E1M5},
    {"HUSTR_E1M6", HUSTR_E1M6},
    {"HUSTR_E1M7", HUSTR_E1M7},
    {"HUSTR_E1M8", HUSTR_E1M8},
    {"HUSTR_E1M9", HUSTR_E1M9},
    {"HUSTR_E2M1", HUSTR_E2M1},
    {"HUSTR_E2M2", HUSTR_E2M2},
    {"HUSTR_E2M3", HUSTR_E2M3},
    {"HUSTR_E2M4", HUSTR_E2M4},
    {"HUSTR_E2M5", HUSTR_E2M5},
    {"HUSTR_E2M6", HUSTR_E2M6},
    {"HUSTR_E2M7", HUSTR_E2M7},
    {"HUSTR_E2M8", HUSTR_E2M8},
    {"HUSTR_E2M9", HUSTR_E2M9},
    {"HUSTR_E3M1", HUSTR_E3M1},
    {"HUSTR_E3M2", HUSTR_E3M2},
    {"HUSTR_E3M3", HUSTR_E3M3},
    {"HUSTR_E3M4", HUSTR_E3M4},
    {"HUSTR_E3M5", HUSTR_E3M5},
    {"HUSTR_E3M6", HUSTR_E3M6},
    {"HUSTR_E3M7", HUSTR_E3M7},
    {"HUSTR_E3M8", HUSTR_E3M8},
    {"HUSTR_E3M9", HUSTR_E3M9},
    {"HUSTR_E4M1", HUSTR_E4M1},
    {"HUSTR_E4M2", HUSTR_E4M2},
    {"HUSTR_E4M3", HUSTR_E4M3},
    {"HUSTR_E4M4", HUSTR_E4M4},
    {"HUSTR_E4M5", HUSTR_E4M5},
    {"HUSTR_E4M6", HUSTR_E4M6},
    {"HUSTR_E4M7", HUSTR_E4M7},
    {"HUSTR_E4M8", HUSTR_E4M8},
    {"HUSTR_E4M9", HUSTR_E4M9},
    {"HUSTR_1", HUSTR_1},
    {"HUSTR_2", HUSTR_2},
    {"HUSTR_3", HUSTR_3},
    {"HUSTR_4", HUSTR_4},
    {"HUSTR_5", HUSTR_5},
    {"HUSTR_6", HUSTR_6},
    {"HUSTR_7", HUSTR_7},
    {"HUSTR_8", HUSTR_8},
    {"HUSTR_9", HUSTR_9},
    {"HUSTR_10", HUSTR_10},
    {"HUSTR_11", HUSTR_11},
    {"HUSTR_12", HUSTR_12},
    {"HUSTR_13", HUSTR_13},
    {"HUSTR_14", HUSTR_14},
    {"HUSTR_15", HUSTR_15},
    {"HUSTR_16", HUSTR_16},
    {"HUSTR_17", HUSTR_17},
    {"HUSTR_18", HUSTR_18},
    {"HUSTR_19", HUSTR_19},
    {"HUSTR_20", HUSTR_20},
    {"HUSTR_21", HUSTR_21},
    {"HUSTR_22", HUSTR_22},
    {"HUSTR_23", HUSTR_23},
    {"HUSTR_24", HUSTR_24},
    {"HUSTR_25", HUSTR_25},
    {"HUSTR_26", HUSTR_26},
    {"HUSTR_27", HUSTR_27},
    {"HUSTR_28", HUSTR_28},
    {"HUSTR_29", HUSTR_29},
    {"HUSTR_30", HUSTR_30},
    {"HUSTR_31", HUSTR_31},
    {"HUSTR_32", HUSTR_32},
    {"PHUSTR_1", PHUSTR_1},
    {"PHUSTR_2", PHUSTR_2},
    {"PHUSTR_3", PHUSTR_3},
    {"PHUSTR_4", PHUSTR_4},
    {"PHUSTR_5", PHUSTR_5},
    {"PHUSTR_6", PHUSTR_6},
    {"PHUSTR_7", PHUSTR_7},
    {"PHUSTR_8", PHUSTR_8},
    {"PHUSTR_9", PHUSTR_9},
    {"PHUSTR_10", PHUSTR_10},
    {"PHUSTR_11", PHUSTR_11},
    {"PHUSTR_12", PHUSTR_12},
    {"PHUSTR_13", PHUSTR_13},
    {"PHUSTR_14", PHUSTR_14},
    {"PHUSTR_15", PHUSTR_15},
    {"PHUSTR_16", PHUSTR_16},
    {"PHUSTR_17", PHUSTR_17},
    {"PHUSTR_18", PHUSTR_18},
    {"PHUSTR_19", PHUSTR_19},
    {"PHUSTR_20", PHUSTR_20},
    {"PHUSTR_21", PHUSTR_21},
    {"PHUSTR_22", PHUSTR_22},
    {"PHUSTR_23", PHUSTR_23},
    {"PHUSTR_24", PHUSTR_24},
    {"PHUSTR_25", PHUSTR_25},
    {"PHUSTR_26", PHUSTR_26},
    {"PHUSTR_27", PHUSTR_27},
    {"PHUSTR_28", PHUSTR_28},
    {"PHUSTR_29", PHUSTR_29},
    {"PHUSTR_30", PHUSTR_30},
    {"PHUSTR_31", PHUSTR_31},
    {"PHUSTR_32", PHUSTR_32},
    {"THUSTR_1", THUSTR_1},
    {"THUSTR_2", THUSTR_2},
    {"THUSTR_3", THUSTR_3},
    {"THUSTR_4", THUSTR_4},
    {"THUSTR_5", THUSTR_5},
    {"THUSTR_6", THUSTR_6},
    {"THUSTR_7", THUSTR_7},
    {"THUSTR_8", THUSTR_8},
    {"THUSTR_9", THUSTR_9},
    {"THUSTR_10", THUSTR_10},
    {"THUSTR_11", THUSTR_11},
    {"THUSTR_12", THUSTR_12},
    {"THUSTR_13", THUSTR_13},
    {"THUSTR_14", THUSTR_14},
    {"THUSTR_15", THUSTR_15},
    {"THUSTR_16", THUSTR_16},
    {"THUSTR_17", THUSTR_17},
    {"THUSTR_18", THUSTR_18},
    {"THUSTR_19", THUSTR_19},
    {"THUSTR_20", THUSTR_20},
    {"THUSTR_21", THUSTR_21},
    {"THUSTR_22", THUSTR_22},
    {"THUSTR_23", THUSTR_23},
    {"THUSTR_24", THUSTR_24},
    {"THUSTR_25", THUSTR_25},
    {"THUSTR_26", THUSTR_26},
    {"THUSTR_27", THUSTR_27},
    {"THUSTR_28", THUSTR_28},
    {"THUSTR_29", THUSTR_29},
    {"THUSTR_30", THUSTR_30},
    {"THUSTR_31", THUSTR_31},
    {"THUSTR_32", THUSTR_32},
    // part 6 - messages as a result of toggling states
    {"AMSTR_FOLLOWON", AMSTR_FOLLOWON},
    {"AMSTR_FOLLOWOFF", AMSTR_FOLLOWOFF},
    {"AMSTR_GRIDON", AMSTR_GRIDON},
    {"AMSTR_GRIDOFF", AMSTR_GRIDOFF},
    {"AMSTR_MARKEDSPOT", AMSTR_MARKEDSPOT},
    {"AMSTR_MARKSCLEARED", AMSTR_MARKSCLEARED},
    {"STSTR_MUS", STSTR_MUS},
    {"STSTR_NOMUS", STSTR_NOMUS},
    {"STSTR_DQDON", STSTR_DQDON},
    {"STSTR_DQDOFF", STSTR_DQDOFF},
    {"STSTR_KFAADDED", STSTR_KFAADDED},
    {"STSTR_FAADDED", STSTR_FAADDED},
    {"STSTR_NCON", STSTR_NCON},
    {"STSTR_NCOFF", STSTR_NCOFF},
    {"STSTR_BEHOLD", STSTR_BEHOLD},
    {"STSTR_BEHOLDX", STSTR_BEHOLDX},
    {"STSTR_CHOPPERS", STSTR_CHOPPERS},
    {"STSTR_CLEV", STSTR_CLEV},
    // part 7 - episode intermission texts
    {"E1TEXT", E1TEXT},
    {"E2TEXT", E2TEXT},
    {"E3TEXT", E3TEXT},
    {"E4TEXT", E4TEXT},
    {"C1TEXT", C1TEXT},
    {"C2TEXT", C2TEXT},
    {"C3TEXT", C3TEXT},
    {"C4TEXT", C4TEXT},
    {"C5TEXT", C5TEXT},
    {"C6TEXT", C6TEXT},
    {"P1TEXT", P1TEXT},
    {"P2TEXT", P2TEXT},
    {"P3TEXT", P3TEXT},
    {"P4TEXT", P4TEXT},
    {"P5TEXT", P5TEXT},
    {"P6TEXT", P6TEXT},
    {"T1TEXT", T1TEXT},
    {"T2TEXT", T2TEXT},
    {"T3TEXT", T3TEXT},
    {"T4TEXT", T4TEXT},
    {"T5TEXT", T5TEXT},
    {"T6TEXT", T6TEXT},
    // part 8 - creature names for the finale
    {"CC_ZOMBIE", CC_ZOMBIE},
    {"CC_SHOTGUN", CC_SHOTGUN},
    {"CC_HEAVY", CC_HEAVY},
    {"CC_IMP", CC_IMP},
    {"CC_DEMON", CC_DEMON},
    {"CC_LOST", CC_LOST},
    {"CC_CACO", CC_CACO},
    {"CC_HELL", CC_HELL},
    {"CC_BARON", CC_BARON},
    {"CC_ARACH", CC_ARACH},
    {"CC_PAIN", CC_PAIN},
    {"CC_REVEN", CC_REVEN},
    {"CC_MANCU", CC_MANCU},
    {"CC_ARCH", CC_ARCH},
    {"CC_SPIDER", CC_SPIDER},
    {"CC_CYBER", CC_CYBER},
    {"CC_HERO", CC_HERO},
    // part 9 - intermission tiled backgrounds
    {"BGFLATE1", "FLOOR4_8"},
    {"BGFLATE2", "SFLR6_1"},
    {"BGFLATE3", "MFLR8_4"},
    {"BGFLATE4", "MFLR8_3"},
    {"BGFLAT06", "SLIME16"},
    {"BGFLAT11", "RROCK14"},
    {"BGFLAT20", "RROCK07"},
    {"BGFLAT30", "RROCK17"},
    {"BGFLAT15", "RROCK13"},
    {"BGFLAT31", "RROCK19"},
    {"BGCASTCALL", "BOSSBACK"},
};

static void *DEH_BEXStrStart(deh_context_t *context, char *line)
{
    char s[10];

    if (sscanf(line, "%9s", s) == 0 || strncmp("[STRINGS]", s, sizeof(s)))
    {
	DEH_Warning(context, "Parse error on section start");
    }

    return NULL;
}

static void DEH_BEXStrParseLine(deh_context_t *context, char *line, void *tag)
{
    char *variable_name, *value;
    int i;

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
	DEH_Warning(context, "Failed to parse assignment");
	return;
    }

    for (i = 0; i < arrlen(bex_stringtable); i++)
    {
	if (!strcmp(bex_stringtable[i].macro, variable_name))
	{
	    DEH_AddStringReplacement(bex_stringtable[i].string, value);
	}
    }
}

deh_section_t deh_section_bexstr =
{
    "[STRINGS]",
    NULL,
    DEH_BEXStrStart,
    DEH_BEXStrParseLine,
    NULL,
    NULL,
};
