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

/*
	TODO 20161024:
	- Conditionally write/read with crispy_extsaveg, defaulting to true
	- Handling of wadfilename != maplumpinfo->wad_file->name
	- Automap markers?
	- moving platforms stopped by linedef 54/89?
	- respawn/fast/nomonsters/deathmatch/altdeath?
	- spawn spots?
	- fireflicker (sector type 17)?
*/

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

static void P_WritePackageTarname (const char *key)
{
	M_snprintf(line, sizeof(line), "%s %s\n", key, PACKAGE_VERSION);
	fprintf(save_stream, "%s", line);
}

static void P_WriteWadFileName (const char *key)
{
	M_snprintf(line, sizeof(line), "%s %s\n", key, maplumpinfo->wad_file->name);
	fprintf(save_stream, "%s", line);
}

// extrakills

static void P_WriteExtraKills (const char *key)
{
	M_snprintf(line, sizeof(line), "%s %d\n", key, extrakills);
	fprintf(save_stream, "%s", line);
}

static void P_ReadExtraKills (const char *key)
{
	int value;

	if (sscanf(line, "%s %d", string, &value) == 2 &&
	    !strncmp(string, key, sizeof(string)))
	{
		extrakills = value;
	}
}

// totalleveltimes

static void P_WriteTotalLevelTimes (const char *key)
{
	M_snprintf(line, sizeof(line), "%s %d\n", key, totalleveltimes);
	fprintf(save_stream, "%s", line);
}

static void P_ReadTotalLevelTimes (const char *key)
{
	int value;

	if (sscanf(line, "%s %d", string, &value) == 2 &&
	    !strncmp(string, key, sizeof(string)))
	{
		totalleveltimes = value;
	}
}

// players[]->lookdir

static void P_WritePlayersLookdir (const char *key)
{
	int i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].lookdir)
		{
			M_snprintf(line, sizeof(line), "%s %d %d\n", key, i, players[i].lookdir);
			fprintf(save_stream, "%s", line);
		}
	}
}

static void P_ReadPlayersLookdir (const char *key)
{
	int i, value;

	if (sscanf(line, "%s %d %d", string, &i, &value) == 3 &&
	    !strncmp(string, key, sizeof(string)) &&
	    i < MAXPLAYERS)
	{
		players[i].lookdir = value;
	}
}

typedef struct
{
	const char *key;
	void (* extsavegwritefn) (const char *key);
	void (* extsavegreadfn) (const char *key);
} extsavegdata_t;

static const extsavegdata_t extsavegdata[] =
{
	{PACKAGE_TARNAME, P_WritePackageTarname, NULL},
	{"wadfilename", P_WriteWadFileName, NULL},
	{"extrakills", P_WriteExtraKills, P_ReadExtraKills},
	{"totalleveltimes", P_WriteTotalLevelTimes, P_ReadTotalLevelTimes},
	{"playerslookdir", P_WritePlayersLookdir, P_ReadPlayersLookdir},
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
					extsavegdata[i].extsavegreadfn(extsavegdata[i].key);
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
