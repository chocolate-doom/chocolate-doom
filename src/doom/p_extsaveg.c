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
	- spawn spots?
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
#include "z_zone.h"

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

// plats

static void P_WritePlats (const char *key)
{
	thinker_t* th;

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acv == (actionf_v)NULL)
		{
			int i;

			for (i = 0; i < MAXPLATS; i++)
			{
				if (activeplats[i] == (plat_t *)th)
				{
					break;
				}
			}

			if (i < MAXPLATS)
			{
				plat_t *plat = (plat_t *)th;

				M_snprintf(line, sizeof(line), "%s %d %d %d %d %d %d %d %d %d %d %d\n",
				           key,
				           (int)(plat->sector - sectors),
				           (int)plat->speed,
				           (int)plat->low,
				           (int)plat->high,
				           (int)plat->wait,
				           (int)plat->count,
				           (int)plat->status,
				           (int)plat->oldstatus,
				           (int)plat->crush,
				           (int)plat->tag,
				           (int)plat->type);
				fprintf(save_stream, "%s", line);
			}

			continue;
		}
	}
}

static void P_ReadPlats (const char *key)
{
	int sector, speed, low, high, wait, count, status, oldstatus, crush, tag, type;

	if (sscanf(line, "%s %d %d %d %d %d %d %d %d %d %d %d",
	           string,
	           &sector,
	           &speed,
	           &low,
	           &high,
	           &wait,
	           &count,
	           &status,
	           &oldstatus,
	           &crush,
	           &tag,
	           &type) == 12 &&
	    !strncmp(string, key, sizeof(string)))
	{
		plat_t *plat;

		plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);

		plat->sector = &sectors[sector];
		plat->speed = speed;
		plat->low = low;
		plat->high = high;
		plat->wait = wait;
		plat->count = count;
		plat->status = status;
		plat->oldstatus = oldstatus;
		plat->crush = crush;
		plat->tag = tag;
		plat->type = type;

		plat->sector->specialdata = plat;

		// [crispy] we only archive plats with plat->function.acv == NULL
		plat->thinker.function.acv = (actionf_v)NULL;

		P_AddThinker(&plat->thinker);
		P_AddActivePlat(plat);
	}
}

// fireflicker
extern void T_FireFlicker (fireflicker_t* flick);

static void P_WriteFireFlicker (const char *key)
{
	thinker_t* th;

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1)T_FireFlicker)
		{
			fireflicker_t *flick = (fireflicker_t *)th;

			M_snprintf(line, sizeof(line), "%s %d %d %d %d\n",
			           key,
			           (int)(flick->sector - sectors),
			           (int)flick->count,
			           (int)flick->maxlight,
			           (int)flick->minlight);
			fprintf(save_stream, "%s", line);
		}
	}
}

static void P_ReadFireFlicker (const char *key)
{
	int sector, count, maxlight, minlight;

	if (sscanf(line, "%s %d %d %d %d\n",
	           string,
	           &sector,
	           &count,
	           &maxlight,
	           &minlight) == 5 &&
	    !strncmp(string, key, sizeof(string)))
	{
		fireflicker_t *flick;

		flick = Z_Malloc(sizeof(*flick), PU_LEVEL, NULL);

		flick->sector = &sectors[sector];
		flick->count = count;
		flick->maxlight = maxlight;
		flick->minlight = minlight;

		flick->thinker.function.acp1 = (actionf_p1)T_FireFlicker;

		P_AddThinker(&flick->thinker);
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
	{"plats", 1, P_WritePlats, P_ReadPlats},
	{"fireflicker", 1, P_WriteFireFlicker, P_ReadFireFlicker},
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
