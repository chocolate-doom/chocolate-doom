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

#include "doomtype.h"
#include "deh_str.h"
#include "m_argv.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "v_diskicon.h"

// disk image patch (either STDISK or STCDROM)

static patch_t *disk;

static int loading_disk_xoffs = 0;
static int loading_disk_yoffs = 0;

disk_indicator_e disk_indicator = disk_off;

void V_EnableLoadingDisk(int xoffs, int yoffs)
{
    char *disk_name;

    loading_disk_xoffs = xoffs;
    loading_disk_yoffs = yoffs;

    if (M_CheckParm("-cdrom") > 0)
        disk_name = DEH_String("STCDROM");
    else
        disk_name = DEH_String("STDISK");

    disk = W_CacheLumpName(disk_name, PU_STATIC);
}

void V_BeginRead(void)
{
    if (disk == NULL)
        return;

    // Draw the disk to the screen
    V_DrawPatch(loading_disk_xoffs, loading_disk_yoffs, disk);

    disk_indicator = disk_dirty;
}
