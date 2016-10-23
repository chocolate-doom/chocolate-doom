//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016 Fabian Greffrath
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
// DESCRIPTION:
//	[crispy] Archiving: Extended SaveGame I/O.
//


#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "doomstat.h"
#include "doomtype.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_extsaveg.h"
#include "p_saveg.h"

#define LINELENGTH 80

static char line[LINELENGTH];
static char string[LINELENGTH];

static void P_WritePackageTarname (char *key)
{
    M_snprintf(line, sizeof(line), "%s %s\n", key, PACKAGE_VERSION);
    fprintf(save_stream, "%s", line);
}

static void P_WriteWadFileName (char *key)
{
    M_snprintf(line, sizeof(line), "%s %s\n", key, maplumpinfo->wad_file->name);
    fprintf(save_stream, "%s", line);
}

static void P_WriteExtraKills (char *key)
{
    M_snprintf(line, sizeof(line), "%s %d\n", key, extrakills);
    fprintf(save_stream, "%s", line);
}

static void P_WriteTotalLevelTimes (char *key)
{
    M_snprintf(line, sizeof(line), "%s %d\n", key, totalleveltimes);
    fprintf(save_stream, "%s", line);
}

typedef struct
{
    char *key;
    void (* extsavegwritefn) (char *key);
    void (* extsavegreadfn) (void);
} extsavegdata_t;

static const extsavegdata_t extsavegdata[] =
{
    {PACKAGE_TARNAME, P_WritePackageTarname, NULL},
    {"wadfilename", P_WriteWadFileName, NULL},
    {"extrakills", P_WriteExtraKills, NULL},
    {"totalleveltimes", P_WriteTotalLevelTimes, NULL},
};

void P_WriteExtendedSaveGameData (void)
{
    int i;

    for (i = 0; i < arrlen(extsavegdata); i++)
    {
	extsavegdata[i].extsavegwritefn(extsavegdata[i].key);
    }
}

static void P_ReadKeyValuePairs (void)
{
    while (fgets(line, sizeof(line), save_stream))
    {
	if (sscanf(line, "%s", string) == 1)
	{
	    int i;

	    for (i = 1; i < arrlen(extsavegdata); i++)
	    {
		if (!strncmp(string, extsavegdata[i].key, sizeof(string)) &&
		    extsavegdata[i].extsavegreadfn)
		{
		    puts(extsavegdata[i].key);
		    extsavegdata[i].extsavegreadfn();
		}
	    }
	}
    }
}

void P_ReadExtendedSaveGameData (void)
{
    long p, curpos, endpos;

    curpos = ftell(save_stream);

    fseek(save_stream, 0, SEEK_END);
    endpos = ftell(save_stream);

    for (p = endpos - 1; p > 0; p--)
    {
	byte curbyte;

	fseek(save_stream, p, SEEK_SET);

	if (fread(&curbyte, 1, 1, save_stream) < 1)
	{
	    break;
	}

	if (curbyte == SAVEGAME_EOF)
	{
	    if (!fgets(line, sizeof(line), save_stream))
	    {
		continue;
	    }

	    if (sscanf(line, "%s", string) == 1 &&
	        !strncmp(string, extsavegdata[0].key, sizeof(string)))
	    {
		P_ReadKeyValuePairs();
		break;
	    }
	}
    }

    fseek(save_stream, curpos, SEEK_SET);
}
