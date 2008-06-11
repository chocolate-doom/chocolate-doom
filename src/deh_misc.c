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
// Parses "Misc" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "deh_misc.h"

// Dehacked: "Initial Health" 
// This is the initial health a player has when starting anew.
// See G_PlayerReborn in g_game.c

int deh_initial_health = DEH_DEFAULT_INITIAL_HEALTH;

// Dehacked: "Initial bullets"
// This is the number of bullets the player has when starting anew.
// See G_PlayerReborn in g_game.c

int deh_initial_bullets = DEH_DEFAULT_INITIAL_BULLETS;

// Dehacked: "Max Health"
// This is the maximum health that can be reached using medikits 
// alone.  See P_TouchSpecialThing in p_inter.c

int deh_max_health = DEH_DEFAULT_MAX_HEALTH;

// Dehacked: "Max Armor"
// This is the maximum armor which can be reached by picking up
// armor helmets. See P_TouchSpecialThing in p_inter.c

int deh_max_armor = DEH_DEFAULT_MAX_ARMOR;

// Dehacked: "Green Armor Class"
// This is the armor class that is given when picking up the green 
// armor or an armor helmet. See P_TouchSpecialThing in p_inter.c
//
// DOS dehacked only modifies the behavior of the green armor shirt,
// the armor class set by armor helmets is not affected.

int deh_green_armor_class = DEH_DEFAULT_GREEN_ARMOR_CLASS;

// Dehacked: "Blue Armor Class"
// This is the armor class that is given when picking up the blue 
// armor or a megasphere. See P_TouchSpecialThing in p_inter.c
//
// DOS dehacked only modifies the MegaArmor behavior and not
// the MegaSphere, which always gives armor type 2.

int deh_blue_armor_class = DEH_DEFAULT_BLUE_ARMOR_CLASS;

// Dehacked: "Max soulsphere"
// The maximum health which can be reached by picking up the
// soulsphere.  See P_TouchSpecialThing in p_inter.c

int deh_max_soulsphere = DEH_DEFAULT_MAX_SOULSPHERE;

// Dehacked: "Soulsphere health"
// The amount of health bonus that picking up a soulsphere
// gives.  See P_TouchSpecialThing in p_inter.c

int deh_soulsphere_health = DEH_DEFAULT_SOULSPHERE_HEALTH;

// Dehacked: "Megasphere health"
// This is what the health is set to after picking up a 
// megasphere.  See P_TouchSpecialThing in p_inter.c

int deh_megasphere_health = DEH_DEFAULT_MEGASPHERE_HEALTH;

// Dehacked: "God mode health"
// This is what the health value is set to when cheating using
// the IDDQD god mode cheat.  See ST_Responder in st_stuff.c

int deh_god_mode_health = DEH_DEFAULT_GOD_MODE_HEALTH;

// Dehacked: "IDFA Armor"
// This is what the armor is set to when using the IDFA cheat.
// See ST_Responder in st_stuff.c

int deh_idfa_armor = DEH_DEFAULT_IDFA_ARMOR;

// Dehacked: "IDFA Armor Class"
// This is what the armor class is set to when using the IDFA cheat.
// See ST_Responder in st_stuff.c

int deh_idfa_armor_class = DEH_DEFAULT_IDFA_ARMOR_CLASS;

// Dehacked: "IDKFA Armor"
// This is what the armor is set to when using the IDKFA cheat.
// See ST_Responder in st_stuff.c

int deh_idkfa_armor = DEH_DEFAULT_IDKFA_ARMOR;

// Dehacked: "IDKFA Armor Class"
// This is what the armor class is set to when using the IDKFA cheat.
// See ST_Responder in st_stuff.c

int deh_idkfa_armor_class = DEH_DEFAULT_IDKFA_ARMOR_CLASS;

// Dehacked: "BFG Cells/Shot"
// This is the number of CELLs firing the BFG uses up.
// See P_CheckAmmo and A_FireBFG in p_pspr.c

int deh_bfg_cells_per_shot = DEH_DEFAULT_BFG_CELLS_PER_SHOT;

// Dehacked: "Monsters infight"
// This controls whether monsters can harm other monsters of the same 
// species.  For example, whether an imp fireball will damage other
// imps.  The value of this in dehacked patches is weird - '202' means
// off, while '221' means on.
//
// See PIT_CheckThing in p_map.c

int deh_species_infighting = DEH_DEFAULT_SPECIES_INFIGHTING;

static struct
{
    char *deh_name;
    int *value;
} misc_settings[] = {
    {"Initial Health",      &deh_initial_health},
    {"Initial Bullets",     &deh_initial_bullets},
    {"Max Health",          &deh_max_health},
    {"Max Armor",           &deh_max_armor},
    {"Green Armor Class",   &deh_green_armor_class},
    {"Blue Armor Class",    &deh_blue_armor_class},
    {"Max Soulsphere",      &deh_max_soulsphere},
    {"Soulsphere Health",   &deh_soulsphere_health},
    {"Megasphere Health",   &deh_megasphere_health},
    {"God Mode Health",     &deh_god_mode_health},
    {"IDFA Armor",          &deh_idfa_armor},
    {"IDFA Armor Class",    &deh_idfa_armor_class},
    {"IDKFA Armor",         &deh_idkfa_armor},
    {"IDKFA Armor Class",   &deh_idkfa_armor_class},
    {"BFG Cells/Shot",      &deh_bfg_cells_per_shot},
};

static void *DEH_MiscStart(deh_context_t *context, char *line)
{
    return NULL;
}

static void DEH_MiscParseLine(deh_context_t *context, char *line, void *tag)
{
    char *variable_name, *value;
    int ivalue;
    size_t i;

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    ivalue = atoi(value);

    if (!strcasecmp(variable_name, "Monsters Infight"))
    {
        // See notes above.
 
        if (ivalue == 202)
        {
            deh_species_infighting = 0;
        }
        else if (ivalue == 221)
        {
            deh_species_infighting = 1;
        }
        else
        {
            DEH_Warning(context, 
                        "Invalid value for 'Monsters Infight': %i", ivalue);
        }
        
        return;
    }

    for (i=0; i<arrlen(misc_settings); ++i)
    {
        if (!strcasecmp(variable_name, misc_settings[i].deh_name))
        {
            *misc_settings[i].value = ivalue;
            return;
        }
    }

    DEH_Warning(context, "Unknown Misc variable '%s'", variable_name);
}

static void DEH_MiscMD5Sum(md5_context_t *context)
{
    unsigned int i;

    for (i=0; i<arrlen(misc_settings); ++i)
    {
        MD5_UpdateInt32(context, *misc_settings[i].value);
    }
}

deh_section_t deh_section_misc =
{
    "Misc",
    NULL,
    DEH_MiscStart,
    DEH_MiscParseLine,
    NULL,
    DEH_MiscMD5Sum,
};

