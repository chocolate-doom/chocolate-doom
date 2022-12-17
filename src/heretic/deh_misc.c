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
// Parses "Misc" sections in dehacked files
//

#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "deh_misc.h"

// Dehacked: "RNG Table"
// This is the RNG table for the games randomness
// See rndtable in m_random.c
int[] deh_rngtable = DEH_DEAFULT_RNG

static struct
{
    const char *deh_name;
    int *value;
} misc_settings[] = {
    {"RNG Table",           &deh_rngtable}
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

static void DEH_MiscSHA1Sum(sha1_context_t *context)
{
    unsigned int i;

    for (i=0; i<arrlen(misc_settings); ++i)
    {
        SHA1_UpdateInt32(context, *misc_settings[i].value);
    }
}

deh_section_t deh_section_misc =
{
    "Misc",
    NULL,
    DEH_MiscStart,
    DEH_MiscParseLine,
    NULL,
    DEH_MiscSHA1Sum,
};
