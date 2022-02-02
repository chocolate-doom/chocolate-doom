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
// Parses "Thing" sections in dehacked files
//

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"

#include "deh_defs.h"
#include "deh_main.h"
#include "deh_mapping.h"

#include "info.h"
#include "p_mobj.h" // [crispy] MF_*

typedef struct {
    const char *flag;
    int bits;
} bex_thingbits_t;

static const bex_thingbits_t bex_thingbitstable[] = {
    {"SPECIAL", MF_SPECIAL},
    {"SOLID", MF_SOLID},
    {"SHOOTABLE", MF_SHOOTABLE},
    {"NOSECTOR", MF_NOSECTOR},
    {"NOBLOCKMAP", MF_NOBLOCKMAP},
    {"AMBUSH", MF_AMBUSH},
    {"JUSTHIT", MF_JUSTHIT},
    {"JUSTATTACKED", MF_JUSTATTACKED},
    {"SPAWNCEILING", MF_SPAWNCEILING},
    {"NOGRAVITY", MF_NOGRAVITY},
    {"DROPOFF", MF_DROPOFF},
    {"PICKUP", MF_PICKUP},
    {"NOCLIP", MF_NOCLIP},
    {"SLIDE", MF_SLIDE},
    {"FLOAT", MF_FLOAT},
    {"TELEPORT", MF_TELEPORT},
    {"MISSILE", MF_MISSILE},
    {"DROPPED", MF_DROPPED},
    {"SHADOW", MF_SHADOW},
    {"NOBLOOD", MF_NOBLOOD},
    {"CORPSE", MF_CORPSE},
    {"INFLOAT", MF_INFLOAT},
    {"COUNTKILL", MF_COUNTKILL},
    {"COUNTITEM", MF_COUNTITEM},
    {"SKULLFLY", MF_SKULLFLY},
    {"NOTDMATCH", MF_NOTDMATCH},
    {"TRANSLUCENT", MF_TRANSLUCENT},
    // [NS] Beta projectile bouncing.
    {"BOUNCES", MF_BOUNCES},
    // TRANSLATION consists of 2 bits, not 1
    {"TRANSLATION", 0x04000000},
    {"TRANSLATION1", 0x04000000},
    {"TRANSLATION2", 0x08000000},
    // unused bits, for Boom compatibility
    {"UNUSED1", 0x08000000},
    {"UNUSED2", 0x10000000},
    {"UNUSED3", 0x20000000},
    {"UNUSED4", 0x40000000},
};

DEH_BEGIN_MAPPING(thing_mapping, mobjinfo_t)
  DEH_MAPPING("ID #",                doomednum)
  DEH_MAPPING("Initial frame",       spawnstate)
  DEH_MAPPING("Hit points",          spawnhealth)
  DEH_MAPPING("First moving frame",  seestate)
  DEH_MAPPING("Alert sound",         seesound)
  DEH_MAPPING("Reaction time",       reactiontime)
  DEH_MAPPING("Attack sound",        attacksound)
  DEH_MAPPING("Injury frame",        painstate)
  DEH_MAPPING("Pain chance",         painchance)
  DEH_MAPPING("Pain sound",          painsound)
  DEH_MAPPING("Close attack frame",  meleestate)
  DEH_MAPPING("Far attack frame",    missilestate)
  DEH_MAPPING("Death frame",         deathstate)
  DEH_MAPPING("Exploding frame",     xdeathstate)
  DEH_MAPPING("Death sound",         deathsound)
  DEH_MAPPING("Speed",               speed)
  DEH_MAPPING("Width",               radius)
  DEH_MAPPING("Height",              height)
  DEH_MAPPING("Mass",                mass)
  DEH_MAPPING("Missile damage",      damage)
  DEH_MAPPING("Action sound",        activesound)
  DEH_MAPPING("Bits",                flags)
  DEH_MAPPING("Respawn frame",       raisestate)
  // [crispy] Thing id to drop after death
  DEH_MAPPING("Dropped item",        droppeditem)
  // [crispy] Distance to switch from missile to melee attack
  DEH_MAPPING("Melee threshold",     meleethreshold)
  // [crispy] Maximum distance range to start shooting (zero for unlimited)
  DEH_MAPPING("Max target range",    maxattackrange)
  // [crispy] Minimum chance for firing a missile
  DEH_MAPPING("Min missile chance",  minmissilechance)
  // [crispy] Multiplies the chance of firing a missile (65536 = normal chance)
  DEH_MAPPING("Missile chance multiplier",  missilechancemult)
DEH_END_MAPPING

// [crispy] initialize Thing extra properties (keeping vanilla props in info.c)
static void DEH_InitThingProperties (void)
{
	int i;

	for (i = 0; i < NUMMOBJTYPES; i++)
	{
		// [crispy] mobj id for item dropped on death
		switch (i)
		{
			case MT_WOLFSS:
			case MT_POSSESSED:
			mobjinfo[i].droppeditem = MT_CLIP;
			break;

			case MT_SHOTGUY:
			mobjinfo[i].droppeditem = MT_SHOTGUN;
			break;

			case MT_CHAINGUY:
			mobjinfo[i].droppeditem = MT_CHAINGUN;
			break;

			default:
			mobjinfo[i].droppeditem = MT_NULL;
		}

		// [crispy] distance to switch from missile to melee attack (generaliz. for Revenant)
		if (i == MT_UNDEAD)
			mobjinfo[i].meleethreshold = 196;
		else
			mobjinfo[i].meleethreshold = 0;

		// [crispy] maximum distance range to start shooting (generaliz. for Arch Vile)
		if (i == MT_VILE)
			mobjinfo[i].maxattackrange = 14*64;
		else
			mobjinfo[i].maxattackrange = 0; // unlimited

		// [crispy] minimum likelihood of a missile attack (generaliz. for Cyberdemon)
		if (i == MT_CYBORG)
			mobjinfo[i].minmissilechance = 160;
		else
			mobjinfo[i].minmissilechance = 200;

		// [crispy] multiplier for missile firing chance (generaliz. from vanilla)
		if (i == MT_CYBORG
		   || i == MT_SPIDER
		   || i == MT_UNDEAD
		   || i == MT_SKULL)
			mobjinfo[i].missilechancemult = FRACUNIT/2;
		else
			mobjinfo[i].missilechancemult = FRACUNIT;
	}
}

static void *DEH_ThingStart(deh_context_t *context, char *line)
{
    int thing_number = 0;
    mobjinfo_t *mobj;
    
    if (sscanf(line, "Thing %i", &thing_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    // dehacked files are indexed from 1
    --thing_number;

    if (thing_number < 0 || thing_number >= NUMMOBJTYPES)
    {
        DEH_Warning(context, "Invalid thing number: %i", thing_number);
        return NULL;
    }
    
    mobj = &mobjinfo[thing_number];
    
    return mobj;
}

static void DEH_ThingParseLine(deh_context_t *context, char *line, void *tag)
{
    mobjinfo_t *mobj;
    char *variable_name, *value;
    int ivalue;
    
    if (tag == NULL)
       return;

    mobj = (mobjinfo_t *) tag;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }
    
//    printf("Set %s to %s for mobj\n", variable_name, value);

    // all values are integers

    ivalue = atoi(value);
    
    // [crispy] support BEX bits mnemonics in Things fields
    if (!ivalue && !strcasecmp(variable_name, "bits"))
    {
	for ( ; (value = strtok(value, ",+| \t\f\r")); value = NULL)
	{
	    int i;
	    for (i = 0; i < arrlen(bex_thingbitstable); i++)
		if (!strcasecmp(value, bex_thingbitstable[i].flag))
		{
		    ivalue |= bex_thingbitstable[i].bits;
		    break;
		}
	}
    }
    // [crispy] Thing ids in dehacked are 1-based, convert dropped item to 0-based
    if (!strcasecmp(variable_name, "dropped item"))
    {
        ivalue -= 1;
    }

    // Set the field value

    DEH_SetMapping(context, &thing_mapping, mobj, variable_name, ivalue);
}

static void DEH_ThingSHA1Sum(sha1_context_t *context)
{
    int i;

    for (i=0; i<NUMMOBJTYPES; ++i)
    {
        DEH_StructSHA1Sum(context, &thing_mapping, &mobjinfo[i]);
    }
}

deh_section_t deh_section_thing =
{
    "Thing",
    DEH_InitThingProperties, // [crispy] initialize Thing extra properties
    DEH_ThingStart,
    DEH_ThingParseLine,
    NULL,
    DEH_ThingSHA1Sum,
};

