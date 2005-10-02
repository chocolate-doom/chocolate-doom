// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Parses "Thing" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "doomdef.h"
#include "doomtype.h"

#include "deh_defs.h"
#include "deh_main.h"

#include "info.h"

static void *DEH_ThingStart(deh_context_t *context, char *line)
{
    int thing_number = 0;
    mobjinfo_t *mobj;
    
    sscanf(line, "Thing %i", &thing_number);

    // dehacked files are indexed from 1
    --thing_number;

    if (thing_number < 0 || thing_number >= NUMMOBJTYPES)
        return NULL;
    
    mobj = &mobjinfo[thing_number];
    
    return mobj;
}

static void DEH_ThingEnd(deh_context_t *context, void *tag)
{
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

        return;
    }
    
//    printf("Set %s to %s for mobj\n", variable_name, value);

    // all values are integers

    ivalue = atoi(value);
    
    // set the appropriate field

    if (!strcasecmp(variable_name, "ID #"))
    {
        mobj->doomednum = ivalue;
    }
    else if (!strcasecmp(variable_name, "Initial frame"))
    {
        mobj->spawnstate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Hit points"))
    {
        mobj->spawnhealth = ivalue;
    }
    else if (!strcasecmp(variable_name, "First moving frame"))
    {
        mobj->seestate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Alert sound"))
    {
        mobj->seesound = ivalue;
    }
    else if (!strcasecmp(variable_name, "Reaction time"))
    {
        mobj->reactiontime = ivalue;
    }
    else if (!strcasecmp(variable_name, "Attack sound"))
    {
        mobj->attacksound = ivalue;
    }
    else if (!strcasecmp(variable_name, "Injury frame"))
    {
        mobj->painstate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Pain chance"))
    {
        mobj->painchance = ivalue;
    }
    else if (!strcasecmp(variable_name, "Pain sound"))
    {
        mobj->painsound = ivalue;
    }
    else if (!strcasecmp(variable_name, "Close attack frame"))
    {
        mobj->meleestate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Far attack frame"))
    {
        mobj->missilestate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Death frame"))
    {
        mobj->deathstate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Exploding frame"))
    {
        mobj->xdeathstate = ivalue;
    }
    else if (!strcasecmp(variable_name, "Death sound"))
    {
        mobj->deathsound = ivalue;
    }
    else if (!strcasecmp(variable_name, "Speed"))
    {
        mobj->speed = ivalue;
    }
    else if (!strcasecmp(variable_name, "Width"))
    {
        mobj->radius = ivalue;
    }
    else if (!strcasecmp(variable_name, "Height"))
    {
        mobj->height = ivalue;
    }
    else if (!strcasecmp(variable_name, "Mass"))
    {
        mobj->mass = ivalue;
    }
    else if (!strcasecmp(variable_name, "Missile damage"))
    {
        mobj->damage = ivalue;
    }
    else if (!strcasecmp(variable_name, "Action sound"))
    {
        mobj->activesound = ivalue;
    }
    else if (!strcasecmp(variable_name, "Bits"))
    {
        mobj->flags = ivalue;
    }
    else if (!strcasecmp(variable_name, "Respawn frame"))
    {
        mobj->raisestate = ivalue;
    }
    else
    {
        printf("Unknown variable name %s\n", variable_name);
    }
}

deh_section_t deh_section_thing =
{
    "Thing",
    NULL,
    DEH_ThingStart,
    DEH_ThingParseLine,
    DEH_ThingEnd,
};

