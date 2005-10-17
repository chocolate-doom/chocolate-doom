// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_misc.c 207 2005-10-17 20:49:42Z fraggle $
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
// alone.  See P_GiveBody in p_inter.c

int deh_max_health = 100;

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

int deh_max_soulsphere;             // TODO
int deh_soulsphere_health;          // TODO
int deh_megasphere_health;          // TODO
int deh_god_mode_health;            // TODO
int deh_idfa_armor;                 // TODO
int deh_idfa_armor_class;           // TODO
int deh_idkfa_armor;                // TODO
int deh_idkfa_armor_class;          // TODO

// Dehacked: "BFG Cells/Shot"
// This is the number of CELLs firing the BFG uses up.
// See P_CheckAmmo and A_FireBFG in p_pspr.c

int deh_bfg_cells_per_shot = 40;

int deh_monsters_infight;           // TODO

static struct
{
    char *deh_name;
    int *value;
    boolean functional;
} misc_settings[] = {
    {"Initial Health",      &deh_initial_health,        true},
    {"Initial Bullets",     &deh_initial_bullets,       true},
    {"Max Health",          &deh_max_health,            true},
    {"Max Armor",           &deh_max_armor,             true},
    {"Green Armor Class",   &deh_green_armor_class,     true},
    {"Blue Armor Class",    &deh_blue_armor_class,      true},
    {"Max Soulsphere",      &deh_max_soulsphere},
    {"Soulsphere Health",   &deh_soulsphere_health},
    {"Megasphere Health",   &deh_megasphere_health},
    {"God Mode Health",     &deh_god_mode_health},
    {"IDFA Armor",          &deh_idfa_armor},
    {"IDFA Armor Class",    &deh_idfa_armor_class},
    {"IDKFA Armor",         &deh_idkfa_armor},
    {"IDKFA Armor Class",   &deh_idkfa_armor_class},
    {"BFG Cells/Shot",      &deh_bfg_cells_per_shot,    true},
    {"Monsters Infight",    &deh_monsters_infight},
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

    for (i=0; i<sizeof(misc_settings) / sizeof(*misc_settings); ++i)
    {
        if (!strcasecmp(variable_name, misc_settings[i].deh_name))
        {
            if (!misc_settings[i].functional)
            {
                DEH_Warning(context, "Misc variable '%s' is not yet functional",
                            variable_name);
            }
            
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

