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
// Parses [PARS] sections in BEX files
//

#include <stdio.h>
#include <string.h>

#include "deh_bexpars.h"
#include "deh_io.h"

int bex_pars[6][10] = {{0}};
int bex_cpars[32] = {0};

static void *DEH_BEXParsStart(deh_context_t *context, char *line)
{
    char s[7];

    if (sscanf(line, "%6s", s) == 0 || strcmp("[PARS]", s))
    {
	DEH_Warning(context, "Parse error on section start");
    }

    return NULL;
}

static void DEH_BEXParsParseLine(deh_context_t *context, char *line, void *tag)
{
    int episode, map, partime;

    if (sscanf(line, "par %32d %32d %32d", &episode, &map, &partime) == 3)
    {
	if (episode >= 1 && episode <= 5 && map >= 1 && map <= 9)
	    bex_pars[episode][map] = partime;
	else
	{
	    DEH_Warning(context, "Invalid episode or map: E%dM%d", episode, map);
	    return;
	}
    }
    else
    if (sscanf(line, "par %32d %32d", &map, &partime) == 2)
    {
	if (map >= 1 && map <= 32)
	    bex_cpars[map-1] = partime;
	else
	{
	    DEH_Warning(context, "Invalid map: MAP%02d", map);
	    return;
	}
    }
    else
    {
	DEH_Warning(context, "Failed to parse assignment");
	return;
    }
}

deh_section_t deh_section_bexpars =
{
    "[PARS]",
    NULL,
    DEH_BEXParsStart,
    DEH_BEXParsParseLine,
    NULL,
    NULL,
};
