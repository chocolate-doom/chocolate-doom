//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Disk load indicator.
//

#ifndef __V_DISKICON__
#define __V_DISKICON__

// Dimensions of the flashing "loading" disk icon

#define LOADING_DISK_W 16
#define LOADING_DISK_H 16

typedef enum
{
    disk_off,
    disk_on,
    disk_dirty
} disk_indicator_e;

extern disk_indicator_e disk_indicator;

extern void V_EnableLoadingDisk (int xoffs, int yoffs);
extern void V_BeginRead (void);

#endif
