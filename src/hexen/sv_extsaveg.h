//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

#ifndef __SV_EXTSAVEG__
#define __SV_EXTSAVEG__

typedef enum
{
    EXTSAVEG_MAP = 1,
    EXTSAVEG_GAME,
    EXTSAVEG_BOTH // for checks only
} savetarget_t;

extern void SV_WriteExtendedSaveGameData (savetarget_t location);
extern void SV_ReadExtendedSaveGameData (savetarget_t location);

#endif
