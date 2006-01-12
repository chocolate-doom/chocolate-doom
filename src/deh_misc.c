// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_misc.c 282 2006-01-12 00:21:29Z fraggle $
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
// $Log$
// Revision 1.9  2006/01/12 00:21:29  fraggle
// Interpret the dehacked "max health" setting properly.
//
// Revision 1.8  2005/10/17 22:07:25  fraggle
// Fix "Monsters Infight"
//
// Revision 1.7  2005/10/17 21:20:27  fraggle
// Add note that the "Monsters Infight" setting is not supported.
//
// Revision 1.6  2005/10/17 21:09:01  fraggle
// Dehacked Misc support: Controls for the armor and armor class set when
// using the ammo cheats.
//
// Revision 1.5  2005/10/17 21:02:57  fraggle
// Dehacked Misc support: Max soulsphere, Soulsphere+Megasphere health bonus
// values, God mode health value
//
// Revision 1.4  2005/10/17 20:49:42  fraggle
// Add dehacked "Misc" implementations for max armor+health, blue+green
// armor classes
//
// Revision 1.3  2005/10/17 20:27:05  fraggle
// Start of Dehacked 'Misc' section support.  Initial Health+Bullets,
// and bfg cells/shot are supported.
//
// Revision 1.2  2005/10/08 20:54:16  fraggle
// Proper dehacked error/warning framework.  Catch a load more errors.
//
// Revision 1.1  2005/10/04 22:10:32  fraggle
// Dehacked "Misc" section parser (currently a dummy)
//
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

int deh_initial_health = 100;

// Dehacked: "Initial bullets"
// This is the number of bullets the player has when starting anew.
// See G_PlayerReborn in g_game.c

int deh_initial_bullets = 50;

// Dehacked: "Max Health"
// This is the maximum health that can be reached using medikits 
// alone.  See P_TouchSpecialThing in p_inter.c

int deh_max_health = 200;

// Dehacked: "Max Armor"
// This is the maximum armor which can be reached by picking up
// armor helmets. See P_TouchSpecialThing in p_inter.c

int deh_max_armor = 200;

// Dehacked: "Green Armor Class"
// This is the armor class that is given when picking up the green 
// armor or an armor helmet. See P_TouchSpecialThing in p_inter.c
//
// Question: Does DOS dehacked modify the armor helmet behavior 
// as well as the green armor behavior?  I am currently following
// the Boom behavior, which is "yes".

int deh_green_armor_class = 1;

// Dehacked: "Blue Armor Class"
// This is the armor class that is given when picking up the blue 
// armor or a megasphere. See P_TouchSpecialThing in p_inter.c
//
// Question: Does DOS dehacked modify the megasphere behavior 
// as well as the blue armor behavior?  I am currently following
// the Boom behavior, which is "yes".

int deh_blue_armor_class = 2;

// Dehacked: "Max soulsphere"
// The maximum health which can be reached by picking up the
// soulsphere.  See P_TouchSpecialThing in p_inter.c

int deh_max_soulsphere = 200;

// Dehacked: "Soulsphere health"
// The amount of health bonus that picking up a soulsphere
// gives.  See P_TouchSpecialThing in p_inter.c

int deh_soulsphere_health = 100;

// Dehacked: "Megasphere health"
// This is what the health is set to after picking up a 
// megasphere.  See P_TouchSpecialThing in p_inter.c

int deh_megasphere_health = 200;

// Dehacked: "God mode health"
// This is what the health value is set to when cheating using
// the IDDQD god mode cheat.  See ST_Responder in st_stuff.c

int deh_god_mode_health = 100;

// Dehacked: "IDFA Armor"
// This is what the armor is set to when using the IDFA cheat.
// See ST_Responder in st_stuff.c

int deh_idfa_armor = 200;

// Dehacked: "IDFA Armor Class"
// This is what the armor class is set to when using the IDFA cheat.
// See ST_Responder in st_stuff.c

int deh_idfa_armor_class = 2;

// Dehacked: "IDKFA Armor"
// This is what the armor is set to when using the IDKFA cheat.
// See ST_Responder in st_stuff.c

int deh_idkfa_armor = 200;

// Dehacked: "IDKFA Armor Class"
// This is what the armor class is set to when using the IDKFA cheat.
// See ST_Responder in st_stuff.c

int deh_idkfa_armor_class = 2;

// Dehacked: "BFG Cells/Shot"
// This is the number of CELLs firing the BFG uses up.
// See P_CheckAmmo and A_FireBFG in p_pspr.c

int deh_bfg_cells_per_shot = 40;

// Dehacked: "Monsters infight"
// This controls whether monsters can harm other monsters of the same 
// species.  For example, whether an imp fireball will damage other
// imps.  The value of this in dehacked patches is weird - '202' means
// off, while '221' means on.
//
// See PIT_CheckThing in p_map.c

int deh_species_infighting = 0;

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
    int i;

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

    for (i=0; i<sizeof(misc_settings) / sizeof(*misc_settings); ++i)
    {
        if (!strcasecmp(variable_name, misc_settings[i].deh_name))
        {
            *misc_settings[i].value = ivalue;
            return;
        }
    }

    DEH_Warning(context, "Unknown Misc variable '%s'", variable_name);
}

deh_section_t deh_section_misc =
{
    "Misc",
    NULL,
    DEH_MiscStart,
    DEH_MiscParseLine,
    NULL,
};

