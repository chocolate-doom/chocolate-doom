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
// Parses "Sound" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "doomdef.h"
#include "doomfeatures.h"
#include "doomtype.h"
#include "deh_defs.h"
#include "deh_main.h"
#include "deh_mapping.h"
#include "sounds.h"

DEH_BEGIN_MAPPING(sound_mapping, sfxinfo_t)
    DEH_UNSUPPORTED_MAPPING("Offset")
    DEH_MAPPING("Zero/One", singularity)
    DEH_MAPPING("Value", priority)
    DEH_MAPPING("Zero 1", link)
    DEH_MAPPING("Zero 2", pitch)
    DEH_MAPPING("Zero 3", volume)
    DEH_MAPPING("Zero 4", data)
    DEH_MAPPING("Neg. One 1", usefulness)
    DEH_MAPPING("Neg. One 2", lumpnum)
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
    int ivalue;
    
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
    
    // all values are integers

    ivalue = atoi(value);
    
    // Set the field value

    DEH_SetMapping(context, &sound_mapping, sfx, variable_name, ivalue);

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

