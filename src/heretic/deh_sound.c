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
// Parses "Sound" sections in dehacked files
//

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "deh_defs.h"
#include "deh_main.h"
#include "deh_mapping.h"

#include "doomdef.h"
#include "i_sound.h"

#include "sounds.h"

DEH_BEGIN_MAPPING(sound_mapping, sfxinfo_t)
    DEH_MAPPING_STRING("Name", name)
    DEH_UNSUPPORTED_MAPPING("Special")
    DEH_MAPPING("Value", priority)
    DEH_MAPPING("Unknown 1", usefulness)
    DEH_UNSUPPORTED_MAPPING("Unknown 2")
    DEH_UNSUPPORTED_MAPPING("Unknown 3")
    DEH_MAPPING("One/Two", numchannels)
DEH_END_MAPPING

static void *DEH_SoundStart(deh_context_t *context, char *line)
{
    int sound_number = 0;
    
    if (sscanf(line, "Sound %i", &sound_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    if (sound_number < 0 || sound_number >= NUMSFX)
    {
        DEH_Warning(context, "Invalid sound number: %i", sound_number);
        return NULL;
    }

    if (sound_number >= DEH_VANILLA_NUMSFX)
    {
        DEH_Warning(context, "Attempt to modify SFX %i.  This will cause "
                             "problems in Vanilla dehacked.", sound_number); 
    }

    return &S_sfx[sound_number];
}

static void DEH_SoundParseLine(deh_context_t *context, char *line, void *tag)
{
    sfxinfo_t *sfx;
    char *variable_name, *value;

    if (tag == NULL)
       return;

    sfx = (sfxinfo_t *) tag;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse
        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    // Set the field value:

    if (!strcasecmp(variable_name, "Name"))
    {
        DEH_SetStringMapping(context, &sound_mapping, sfx,
                             variable_name, value);
    }
    else
    {
        DEH_SetMapping(context, &sound_mapping, sfx,
                       variable_name, atoi(value));
    }
}

deh_section_t deh_section_sound =
{
    "Sound",
    NULL,
    DEH_SoundStart,
    DEH_SoundParseLine,
    NULL,
    NULL,
};

