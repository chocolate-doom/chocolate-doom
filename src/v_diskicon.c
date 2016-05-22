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
#include "i_swap.h"
#include "i_video.h"
#include "m_argv.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "v_diskicon.h"

// Only display the disk icon if more then this much bytes have been read
// during the previous tic.

static const int diskicon_threshold = 20*1024;

// disk image patch (either STDISK or STCDROM)

static patch_t *disk;
static byte *saved_background;

static int loading_disk_xoffs = 0;
static int loading_disk_yoffs = 0;

// Number of bytes read since the last call to V_DrawDiskIcon().
static size_t recent_bytes_read = 0;
static boolean disk_drawn;

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
    saved_background = Z_Malloc(SHORT(disk->width) * SHORT(disk->height),
                                PU_STATIC, NULL);
}

void V_BeginRead(size_t nbytes)
{
    recent_bytes_read += nbytes;
}

static void CopyRegion(byte *dest, int dest_pitch,
                       byte *src, int src_pitch,
                       int w, int h)
{
    byte *s, *d;
    int y;

    s = src; d = dest;
    for (y = 0; y < h; ++y)
    {
        memcpy(d, s, w);
        s += src_pitch;
        d += dest_pitch;
    }
}

static byte *DiskRegionPointer(void)
{
    int x, y;

    x = loading_disk_xoffs + SHORT(disk->leftoffset);
    y = loading_disk_yoffs + SHORT(disk->topoffset);
    return I_VideoBuffer + y * SCREENWIDTH + x;
}

void V_DrawDiskIcon(void)
{
    if (disk != NULL && recent_bytes_read > diskicon_threshold)
    {
        // Save the background behind the disk before we draw it.
        CopyRegion(saved_background, SHORT(disk->width),
                   DiskRegionPointer(), SCREENWIDTH,
                   SHORT(disk->width), SHORT(disk->height));

        // Draw the disk to the screen
        V_DrawPatch(loading_disk_xoffs, loading_disk_yoffs, disk);
        disk_drawn = true;
    }

    recent_bytes_read = 0;
}

void V_RestoreDiskBackground(void)
{
    if (disk_drawn)
    {
        // Restore the background.
        CopyRegion(DiskRegionPointer(), SCREENWIDTH,
                   saved_background, SHORT(disk->width),
                   SHORT(disk->width), SHORT(disk->height));

        disk_drawn = false;
    }
}

