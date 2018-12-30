//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2005-2006 Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] Support MUSINFO lump (dynamic music changing)
//

#ifndef __S_MUSINFO__
#define __S_MUSINFO__

#include "p_mobj.h"

#define MAX_MUS_ENTRIES 65 // [crispy] 0 to 64 inclusive

typedef struct musinfo_s
{
	mobj_t *mapthing;
	mobj_t *lastmapthing;
	int tics;
	int current_item;
	int items[MAX_MUS_ENTRIES];
	boolean from_savegame;
} musinfo_t;

extern musinfo_t musinfo;

extern void S_ParseMusInfo (const char *mapid);
extern void T_MusInfo (void);

#endif
