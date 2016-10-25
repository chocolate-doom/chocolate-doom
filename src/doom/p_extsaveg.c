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
	TODO 20161025:
	- Handling of wadfilename != maplumpinfo->wad_file->name
	- Automap markers?
	- moving platforms stopped by linedef 54/89?
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

// gameoptions

static void P_WriteGameOptions (const char *key)
{
	int value;

	value = (respawnparm ? 1 : 0) |
	        (fastparm ? 2 : 0) |
	        (nomonsters ? 4 : 0) |
	        (deathmatch << 4);

	M_snprintf(line, sizeof(line), "%s %d\n", key, value);
	fprintf(save_stream, "%s", line);
}

static void P_ReadGameOptions (const char *key)
{
	int value;

	if (sscanf(line, "%s %d", string, &value) == 2 &&
	    !strncmp(string, key, sizeof(string)))
	{
		respawnparm = value & 1;
		fastparm = (value >> 1) & 1;
		nomonsters = (value >> 2) & 1;
		deathmatch = (value >> 4) & 2;
	}
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
	const int pass;
	void (* extsavegwritefn) (const char *key);
	void (* extsavegreadfn) (const char *key);
} extsavegdata_t;

static const extsavegdata_t extsavegdata[] =
{
	{PACKAGE_TARNAME, 0, P_WritePackageTarname, NULL},
	{"wadfilename", 0, P_WriteWadFileName, NULL},
	{"gameoptions", 0, P_WriteGameOptions, P_ReadGameOptions},
	{"extrakills", 1, P_WriteExtraKills, P_ReadExtraKills},
	{"totalleveltimes", 1, P_WriteTotalLevelTimes, P_ReadTotalLevelTimes},
	{"playerslookdir", 1, P_WritePlayersLookdir, P_ReadPlayersLookdir},
};

void P_WriteExtendedSaveGameData (void)
{
	int i;

	if (crispy_extsaveg)
	{
		for (i = 0; i < arrlen(extsavegdata); i++)
		{
			extsavegdata[i].extsavegwritefn(extsavegdata[i].key);
		}
	}
}

static void P_ReadKeyValuePairs (int pass)
{
	while (fgets(line, sizeof(line), save_stream))
	{
		if (sscanf(line, "%s", string) == 1)
		{
			int i;

			for (i = 1; i < arrlen(extsavegdata); i++)
			{
				if (extsavegdata[i].extsavegreadfn &&
				    extsavegdata[i].pass == pass &&
				    !strncmp(string, extsavegdata[i].key, sizeof(string)))
				{
					extsavegdata[i].extsavegreadfn(extsavegdata[i].key);
				}
			}
		}
	}
}

static void P_ReadFirstPass (void)
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
				P_ReadKeyValuePairs(0);
				break;
			}
		}
	}

	fseek(save_stream, curpos, SEEK_SET);
}

void P_ReadExtendedSaveGameData (int pass)
{
	if (crispy_extsaveg)
	{
		if (pass)
		{
			P_ReadKeyValuePairs(pass);
		}
		else
		{
			P_ReadFirstPass();
		}
	}
}
