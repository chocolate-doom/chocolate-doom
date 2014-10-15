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
// Parses INCLUDE directives in BEX files
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_misc.h"

#include "deh_io.h"
#include "deh_main.h"

static boolean bex_nested = false;

static void *DEH_BEXInclStart(deh_context_t *context, char *line)
{
    char *deh_file, *inc_file, *try_path;
    extern boolean bex_notext;

    if (!DEH_FileName(context))
    {
	DEH_Warning(context, "DEHACKED lumps may not include files");
	return NULL;
    }

    deh_file = DEH_FileName(context);

    if (bex_nested)
    {
	DEH_Warning(context, "Included files may not include other files");
	return NULL;
    }

    inc_file = malloc(sizeof(*inc_file) * strlen(line));

    if (sscanf(line, "INCLUDE NOTEXT %s", inc_file) == 1)
    {
	bex_notext = true;
    }
    else
    if (sscanf(line, "INCLUDE %s", inc_file) == 1)
    {
	// well, fine
    }
    else
    {
	DEH_Warning(context, "Parse error on section start");
	free(inc_file);
	return NULL;
    }

    // first, try loading the file right away
    try_path = inc_file;

    if (!M_FileExists(try_path))
    {
	// second, try loading the file in the directory of the current file
	char *dir;
	dir = M_DirName(deh_file);
	try_path = M_StringJoin(dir, DIR_SEPARATOR_S, inc_file, NULL);
	free(dir);
    }

    bex_nested = true;

    if (!M_FileExists(try_path) || !DEH_LoadFile(try_path))
    {
	DEH_Warning(context, "Could not include \"%s\"", inc_file);
    }

    bex_nested = false;
    bex_notext = false;

    if (try_path != inc_file)
	free(try_path);
    free(inc_file);

    return NULL;
}

static void DEH_BEXInclParseLine(deh_context_t *context, char *line, void *tag)
{
    // not used
}

deh_section_t deh_section_bexincl =
{
    "INCLUDE",
    NULL,
    DEH_BEXInclStart,
    DEH_BEXInclParseLine,
    NULL,
    NULL,
};
