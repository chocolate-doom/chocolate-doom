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
//     Screen scale-up code: 
//         1x,2x,3x,4x pixel doubling
//         Aspect ratio-correcting stretch functions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"

#include "i_video.h"
#include "m_argv.h"
#include "z_zone.h"

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

// Should be I_VideoBuffer

static byte *src_buffer;

// Destination buffer, ie. screen->pixels.

static byte *dest_buffer;

// Pitch of destination buffer, ie. screen->pitch.

static int dest_pitch;

// Lookup tables used for aspect ratio correction stretching code.
// stretch_tables[0] : 20% / 80%
// stretch_tables[1] : 40% / 60%
// All other combinations can be reached from these two tables.

static byte *stretch_tables[2] = { NULL, NULL };

// 50%/50% stretch table, for 800x600 squash mode

static byte *half_stretch_table = NULL;

// Called to set the source and destination buffers before doing the
// scale.

void I_InitScale(byte *_src_buffer, byte *_dest_buffer, int _dest_pitch)
{
    src_buffer = _src_buffer;
    dest_buffer = _dest_buffer;
    dest_pitch = _dest_pitch;
}

//
// Pixel doubling scale-up functions.
//

// 1x scale doesn't really do any scaling: it just copies the buffer
// a line at a time for when pitch != SCREENWIDTH (!native_surface)

static boolean I_Scale1x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;
    int w = x2 - x1;
    
    // Need to byte-copy from buffer into the screen buffer

    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + y1 * dest_pitch + x1;

    for (y=y1; y<y2; ++y)
    {
        memcpy(screenp, bufp, w);
        screenp += dest_pitch;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_scale_1x = {
    SCREENWIDTH, SCREENHEIGHT,
    NULL,
    I_Scale1x,
    false,
};

// 2x scale (640x400)

static boolean I_Scale2x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp, *screenp2;
    int x, y;
    int multi_pitch;

    multi_pitch = dest_pitch * 2;
    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + (y1 * dest_pitch + x1) * 2;
    screenp2 = screenp + dest_pitch;

    for (y=y1; y<y2; ++y)
    {
        byte *sp, *sp2, *bp;
        sp = screenp;
        sp2 = screenp2;
        bp = bufp;

        for (x=x1; x<x2; ++x)
        {
            *sp++ = *bp;  *sp++ = *bp;
            *sp2++ = *bp; *sp2++ = *bp;
            ++bp;
        }
        screenp += multi_pitch;
        screenp2 += multi_pitch;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_scale_2x = {
    SCREENWIDTH * 2, SCREENHEIGHT * 2,
    NULL,
    I_Scale2x,
    false,
};

// 3x scale (960x600)

static boolean I_Scale3x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp, *screenp2, *screenp3;
    int x, y;
    int multi_pitch;

    multi_pitch = dest_pitch * 3;
    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + (y1 * dest_pitch + x1) * 3;
    screenp2 = screenp + dest_pitch;
    screenp3 = screenp + dest_pitch * 2;

    for (y=y1; y<y2; ++y)
    {
        byte *sp, *sp2, *sp3, *bp;
        sp = screenp;
        sp2 = screenp2;
        sp3 = screenp3;
        bp = bufp;

        for (x=x1; x<x2; ++x)
        {
            *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;
            *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp;
            *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp;
            ++bp;
        }
        screenp += multi_pitch;
        screenp2 += multi_pitch;
        screenp3 += multi_pitch;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_scale_3x = {
    SCREENWIDTH * 3, SCREENHEIGHT * 3,
    NULL,
    I_Scale3x,
    false,
};

// 4x scale (1280x800)

static boolean I_Scale4x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp, *screenp2, *screenp3, *screenp4;
    int x, y;
    int multi_pitch;

    multi_pitch = dest_pitch * 4;
    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + (y1 * dest_pitch + x1) * 4;
    screenp2 = screenp + dest_pitch;
    screenp3 = screenp + dest_pitch * 2;
    screenp4 = screenp + dest_pitch * 3;

    for (y=y1; y<y2; ++y)
    {
        byte *sp, *sp2, *sp3, *sp4, *bp;
        sp = screenp;
        sp2 = screenp2;
        sp3 = screenp3;
        sp4 = screenp4;
        bp = bufp;

        for (x=x1; x<x2; ++x)
        {
            *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;
            *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp;
            *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp;
            *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp;
            ++bp;
        }
        screenp += multi_pitch;
        screenp2 += multi_pitch;
        screenp3 += multi_pitch;
        screenp4 += multi_pitch;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_scale_4x = {
    SCREENWIDTH * 4, SCREENHEIGHT * 4,
    NULL,
    I_Scale4x,
    false,
};

// 5x scale (1600x1000)

static boolean I_Scale5x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp, *screenp2, *screenp3, *screenp4, *screenp5;
    int x, y;
    int multi_pitch;

    multi_pitch = dest_pitch * 5;
    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + (y1 * dest_pitch + x1) * 5;
    screenp2 = screenp + dest_pitch;
    screenp3 = screenp + dest_pitch * 2;
    screenp4 = screenp + dest_pitch * 3;
    screenp5 = screenp + dest_pitch * 4;

    for (y=y1; y<y2; ++y)
    {
        byte *sp, *sp2, *sp3, *sp4, *sp5, *bp;
        sp = screenp;
        sp2 = screenp2;
        sp3 = screenp3;
        sp4 = screenp4;
        sp5 = screenp5;
        bp = bufp;

        for (x=x1; x<x2; ++x)
        {
            *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;
            *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp;
            *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp;
            *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp;
            *sp5++ = *bp; *sp5++ = *bp; *sp5++ = *bp; *sp5++ = *bp; *sp5++ = *bp;
            ++bp;
        }
        screenp += multi_pitch;
        screenp2 += multi_pitch;
        screenp3 += multi_pitch;
        screenp4 += multi_pitch;
        screenp5 += multi_pitch;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_scale_5x = {
    SCREENWIDTH * 5, SCREENHEIGHT * 5,
    NULL,
    I_Scale5x,
    false,
};


// Search through the given palette, finding the nearest color that matches
// the given color.

static int FindNearestColor(byte *palette, int r, int g, int b)
{
    byte *col;
    int best;
    int best_diff;
    int diff;
    int i;

    best = 0;
    best_diff = INT_MAX;

    for (i=0; i<256; ++i)
    {
        col = palette + i * 3;
        diff = (r - col[0]) * (r - col[0])
             + (g - col[1]) * (g - col[1])
             + (b - col[2]) * (b - col[2]);

        if (diff == 0)
        {
            return i;
        }
        else if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }
    }

    return best;
}

// Create a stretch table.  This is a lookup table for blending colors.
// pct specifies the bias between the two colors: 0 = all y, 100 = all x.
// NB: This is identical to the lookup tables used in other ports for
// translucency.

static byte *GenerateStretchTable(byte *palette, int pct)
{
    byte *result;
    int x, y;
    int r, g, b;
    byte *col1;
    byte *col2;

    result = Z_Malloc(256 * 256, PU_STATIC, NULL);

    for (x=0; x<256; ++x)
    {
        for (y=0; y<256; ++y)
        {
            col1 = palette + x * 3;
            col2 = palette + y * 3;
            r = (((int) col1[0]) * pct + ((int) col2[0]) * (100 - pct)) / 100;
            g = (((int) col1[1]) * pct + ((int) col2[1]) * (100 - pct)) / 100;
            b = (((int) col1[2]) * pct + ((int) col2[2]) * (100 - pct)) / 100;
            result[x * 256 + y] = FindNearestColor(palette, r, g, b);
        }
    }

    return result;
}

// Called at startup to generate the lookup tables for aspect ratio
// correcting scale up.

static void I_InitStretchTables(byte *palette)
{
    if (stretch_tables[0] != NULL)
    {
        return;
    }

    // We only actually need two lookup tables:
    //
    // mix 0%   =  just write line 1
    // mix 20%  =  stretch_tables[0]
    // mix 40%  =  stretch_tables[1]
    // mix 60%  =  stretch_tables[1] used backwards
    // mix 80%  =  stretch_tables[0] used backwards
    // mix 100% =  just write line 2

    printf("I_InitStretchTables: Generating lookup tables..");
    fflush(stdout);
    stretch_tables[0] = GenerateStretchTable(palette, 20);
    printf(".."); fflush(stdout);
    stretch_tables[1] = GenerateStretchTable(palette, 40);
    puts("");
}

// Create 50%/50% table for 800x600 squash mode

static void I_InitSquashTable(byte *palette)
{
    if (half_stretch_table != NULL)
    {
        return;
    }

    printf("I_InitSquashTable: Generating lookup table..");
    fflush(stdout);
    half_stretch_table = GenerateStretchTable(palette, 50);
    puts("");
}

// Destroy the scaling lookup tables. This should only ever be called
// if switching to a completely different palette from the normal one
// (in which case the mappings no longer make any sense).

void I_ResetScaleTables(byte *palette)
{
    if (stretch_tables[0] != NULL)
    {
        Z_Free(stretch_tables[0]);
        Z_Free(stretch_tables[1]);

        printf("I_ResetScaleTables: Regenerating lookup tables..\n");
        stretch_tables[0] = GenerateStretchTable(palette, 20);
        stretch_tables[1] = GenerateStretchTable(palette, 40);
    }

    if (half_stretch_table != NULL)
    {
        Z_Free(half_stretch_table);

        printf("I_ResetScaleTables: Regenerating lookup table..\n");

        half_stretch_table = GenerateStretchTable(palette, 50);
    }
}


// 
// Aspect ratio correcting scale up functions.
//
// These double up pixels to stretch the screen when using a 4:3
// screen mode.
//

static inline void WriteBlendedLine1x(byte *dest, byte *src1, byte *src2, 
                               byte *stretch_table)
{
    int x;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        *dest = stretch_table[*src1 * 256 + *src2];
        ++dest;
        ++src1;
        ++src2;
    }
} 

// 1x stretch (320x240)

static boolean I_Stretch1x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    // Need to byte-copy from buffer into the screen buffer

    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + y1 * dest_pitch + x1;

    // For every 5 lines of src_buffer, 6 lines are written to dest_buffer
    // (200 -> 240)

    for (y=0; y<SCREENHEIGHT; y += 5)
    {
        // 100% line 0
        memcpy(screenp, bufp, SCREENWIDTH);
        screenp += dest_pitch;

        // 20% line 0, 80% line 1
        WriteBlendedLine1x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 40% line 1, 60% line 2
        WriteBlendedLine1x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 60% line 2, 40% line 3
        WriteBlendedLine1x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 80% line 3, 20% line 4
        WriteBlendedLine1x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 4
        memcpy(screenp, bufp, SCREENWIDTH);
        screenp += dest_pitch; bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_stretch_1x = {
    SCREENWIDTH, SCREENHEIGHT_4_3,
    I_InitStretchTables,
    I_Stretch1x,
    true,
};

static inline void WriteLine2x(byte *dest, byte *src)
{
    int x;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        dest[0] = *src;
        dest[1] = *src;
        dest += 2;
        ++src;
    }
}

static inline void WriteBlendedLine2x(byte *dest, byte *src1, byte *src2, 
                               byte *stretch_table)
{
    int x;
    int val;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        val = stretch_table[*src1 * 256 + *src2];
        dest[0] = val;
        dest[1] = val;
        dest += 2;
        ++src1;
        ++src2;
    }
} 

// 2x stretch (640x480)

static boolean I_Stretch2x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    // Need to byte-copy from buffer into the screen buffer

    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + y1 * dest_pitch + x1;

    // For every 5 lines of src_buffer, 12 lines are written to dest_buffer.
    // (200 -> 480)

    for (y=0; y<SCREENHEIGHT; y += 5)
    {
        // 100% line 0
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 40% line 0, 60% line 1
        WriteBlendedLine2x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 1
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 80% line 1, 20% line 2
        WriteBlendedLine2x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 2
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 2
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 20% line 2, 80% line 3
        WriteBlendedLine2x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 3
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 60% line 3, 40% line 4
        WriteBlendedLine2x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 4
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 4
        WriteLine2x(screenp, bufp);
        screenp += dest_pitch; bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_stretch_2x = {
    SCREENWIDTH * 2, SCREENHEIGHT_4_3 * 2,
    I_InitStretchTables,
    I_Stretch2x,
    false,
};

static inline void WriteLine3x(byte *dest, byte *src)
{
    int x;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        dest[0] = *src;
        dest[1] = *src;
        dest[2] = *src;
        dest += 3;
        ++src;
    }
}

static inline void WriteBlendedLine3x(byte *dest, byte *src1, byte *src2, 
                               byte *stretch_table)
{
    int x;
    int val;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        val = stretch_table[*src1 * 256 + *src2];
        dest[0] = val;
        dest[1] = val;
        dest[2] = val;
        dest += 3;
        ++src1;
        ++src2;
    }
} 

// 3x stretch (960x720)

static boolean I_Stretch3x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    // Need to byte-copy from buffer into the screen buffer

    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + y1 * dest_pitch + x1;

    // For every 5 lines of src_buffer, 18 lines are written to dest_buffer.
    // (200 -> 720)

    for (y=0; y<SCREENHEIGHT; y += 5)
    {
        // 100% line 0
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 60% line 0, 40% line 1
        WriteBlendedLine3x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 1
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 1
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 1
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 20% line 1, 80% line 2
        WriteBlendedLine3x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 2
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 2
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 80% line 2, 20% line 3
        WriteBlendedLine3x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 3
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 3
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 3
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 40% line 3, 60% line 4
        WriteBlendedLine3x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 4
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 4
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 4
        WriteLine3x(screenp, bufp);
        screenp += dest_pitch; bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_stretch_3x = {
    SCREENWIDTH * 3, SCREENHEIGHT_4_3 * 3,
    I_InitStretchTables,
    I_Stretch3x,
    false,
};

static inline void WriteLine4x(byte *dest, byte *src)
{
    int x;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        dest[0] = *src;
        dest[1] = *src;
        dest[2] = *src;
        dest[3] = *src;
        dest += 4;
        ++src;
    }
}

static inline void WriteBlendedLine4x(byte *dest, byte *src1, byte *src2, 
                               byte *stretch_table)
{
    int x;
    int val;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        val = stretch_table[*src1 * 256 + *src2];
        dest[0] = val;
        dest[1] = val;
        dest[2] = val;
        dest[3] = val;
        dest += 4;
        ++src1;
        ++src2;
    }
} 

// 4x stretch (1280x960)

static boolean I_Stretch4x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    // Need to byte-copy from buffer into the screen buffer

    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + y1 * dest_pitch + x1;

    // For every 5 lines of src_buffer, 24 lines are written to dest_buffer.
    // (200 -> 960)

    for (y=0; y<SCREENHEIGHT; y += 5)
    {
        // 100% line 0
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 90% line 0, 20% line 1
        WriteBlendedLine4x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 1
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 1
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 1
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 1
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 60% line 1, 40% line 2
        WriteBlendedLine4x(screenp, bufp + SCREENWIDTH, bufp, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 2
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 2
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 2
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 2
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 40% line 2, 60% line 3
        WriteBlendedLine4x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[1]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 3
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 3
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 3
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 3
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 20% line 3, 80% line 4
        WriteBlendedLine4x(screenp, bufp, bufp + SCREENWIDTH, stretch_tables[0]);
        screenp += dest_pitch; bufp += SCREENWIDTH;

        // 100% line 4
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 4
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 4
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 4
        WriteLine4x(screenp, bufp);
        screenp += dest_pitch; bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_stretch_4x = {
    SCREENWIDTH * 4, SCREENHEIGHT_4_3 * 4,
    I_InitStretchTables,
    I_Stretch4x,
    false,
};

static inline void WriteLine5x(byte *dest, byte *src)
{
    int x;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        dest[0] = *src;
        dest[1] = *src;
        dest[2] = *src;
        dest[3] = *src;
        dest[4] = *src;
        dest += 5;
        ++src;
    }
}

// 5x stretch (1600x1200)

static boolean I_Stretch5x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    // Need to byte-copy from buffer into the screen buffer

    bufp = src_buffer + y1 * SCREENWIDTH + x1;
    screenp = (byte *) dest_buffer + y1 * dest_pitch + x1;

    // For every 1 line of src_buffer, 6 lines are written to dest_buffer.
    // (200 -> 1200)

    for (y=0; y<SCREENHEIGHT; y += 1)
    {
        // 100% line 0
        WriteLine5x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine5x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine5x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine5x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine5x(screenp, bufp);
        screenp += dest_pitch;

        // 100% line 0
        WriteLine5x(screenp, bufp);
        screenp += dest_pitch; bufp += SCREENWIDTH;
    }

    // test hack for Porsche Monty... scan line simulation:
    // See here: http://www.doomworld.com/vb/post/962612

    if (M_CheckParm("-scanline") > 0)
    {
        screenp = (byte *) dest_buffer + 2 * dest_pitch;

        for (y=0; y<1198; y += 3)
        {
            memset(screenp, 0, 1600);

            screenp += dest_pitch * 3;
        }
    }

    return true;
}

screen_mode_t mode_stretch_5x = {
    SCREENWIDTH * 5, SCREENHEIGHT_4_3 * 5,
    I_InitStretchTables,
    I_Stretch5x,
    false,
};

//
// Aspect ratio correcting "squash" functions. 
//
// These do the opposite of the "stretch" functions above: while the
// stretch functions increase the vertical dimensions, the squash
// functions decrease the horizontal dimensions for the same result.
//
// The same blend tables from the stretch functions are reused; as 
// a result, the dimensions are *slightly* wrong (eg. 320x200 should
// squash to 266x200, but actually squashes to 256x200).
//

// 
// 1x squashed scale (256x200)
//

static inline void WriteSquashedLine1x(byte *dest, byte *src)
{
    int x;

    for (x=0; x<SCREENWIDTH; )
    {
        // Draw in blocks of 5

        // 80% pixel 0,   20% pixel 1

        *dest++ = stretch_tables[0][src[1] * 256 + src[0]];

        // 60% pixel 1,   40% pixel 2

        *dest++ = stretch_tables[1][src[2] * 256 + src[1]];

        // 40% pixel 2,   60% pixel 3

        *dest++ = stretch_tables[1][src[2] * 256 + src[3]];

        // 20% pixel 3,   80% pixel 4

        *dest++ = stretch_tables[0][src[3] * 256 + src[4]];

        x += 5;
        src += 5;
    }
}

// 1x squashed (256x200)

static boolean I_Squash1x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    bufp = src_buffer;
    screenp = (byte *) dest_buffer;

    for (y=0; y<SCREENHEIGHT; ++y) 
    {
        WriteSquashedLine1x(screenp, bufp);

        screenp += dest_pitch;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_squash_1x = {
    SCREENWIDTH_4_3, SCREENHEIGHT,
    I_InitStretchTables,
    I_Squash1x,
    true,
};


//
// 2x squashed scale (512x400)
//

#define DRAW_PIXEL2 \
      *dest++ = *dest2++ = c;

static inline void WriteSquashedLine2x(byte *dest, byte *src)
{
    byte *dest2;
    int x, c;

    dest2 = dest + dest_pitch;

    for (x=0; x<SCREENWIDTH; )
    {
        // Draw in blocks of 5

        // 100% pixel 0

        c = src[0];
        DRAW_PIXEL2;

        // 60% pixel 0, 40% pixel 1

        c = stretch_tables[1][src[1] * 256 + src[0]];
        DRAW_PIXEL2;

        // 100% pixel 1

        c = src[1];
        DRAW_PIXEL2;

        // 20% pixel 1, 80% pixel 2

        c = stretch_tables[0][src[1] * 256 + src[2]];
        DRAW_PIXEL2;

        // 80% pixel 2, 20% pixel 3

        c = stretch_tables[0][src[3] * 256 + src[2]];
        DRAW_PIXEL2;

        // 100% pixel 3

        c = src[3];
        DRAW_PIXEL2;

        // 40% pixel 3, 60% pixel 4

        c = stretch_tables[1][src[3] * 256 + src[4]];
        DRAW_PIXEL2;

        // 100% pixel 4

        c = src[4];
        DRAW_PIXEL2;

        x += 5;
        src += 5;
    }
}

// 2x squash (512x400)

static boolean I_Squash2x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    bufp = src_buffer;
    screenp = (byte *) dest_buffer;

    for (y=0; y<SCREENHEIGHT; ++y) 
    {
        WriteSquashedLine2x(screenp, bufp);

        screenp += dest_pitch * 2;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_squash_2x = {
    SCREENWIDTH_4_3 * 2, SCREENHEIGHT * 2,
    I_InitStretchTables,
    I_Squash2x,
    false,
};


#define DRAW_PIXEL3 \
        *dest++ = *dest2++ = *dest3++ = c

static inline void WriteSquashedLine3x(byte *dest, byte *src)
{
    byte *dest2, *dest3;
    int x, c;

    dest2 = dest + dest_pitch;
    dest3 = dest + dest_pitch * 2;

    for (x=0; x<SCREENWIDTH; )
    {
        // Every 2 pixels is expanded to 5 pixels

        // 100% pixel 0 x2

        c = src[0];

        DRAW_PIXEL3;
        DRAW_PIXEL3;

        // 50% pixel 0, 50% pixel 1

        c = half_stretch_table[src[0] * 256 + src[1]];

        DRAW_PIXEL3;

        // 100% pixel 1

        c = src[1];

        DRAW_PIXEL3;
        DRAW_PIXEL3;

        x += 2;
        src += 2;
    }
}


//
// 3x scale squashed (800x600)
//
// This is a special case that uses the half_stretch_table (50%) rather
// than the normal stretch_tables(20,40%), to scale up to 800x600 
// exactly.
//

static boolean I_Squash3x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    bufp = src_buffer;
    screenp = (byte *) dest_buffer;

    for (y=0; y<SCREENHEIGHT; ++y) 
    {
        WriteSquashedLine3x(screenp, bufp);

        screenp += dest_pitch * 3;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_squash_3x = {
    800, 600,
    I_InitSquashTable,
    I_Squash3x,
    false,
};

#define DRAW_PIXEL4 \
        *dest++ = *dest2++ = *dest3++ = *dest4++ = c;
      
static inline void WriteSquashedLine4x(byte *dest, byte *src)
{
    int x;
    int c;
    byte *dest2, *dest3, *dest4;

    dest2 = dest + dest_pitch;
    dest3 = dest + dest_pitch * 2;
    dest4 = dest + dest_pitch * 3;

    for (x=0; x<SCREENWIDTH; )
    {
        // Draw in blocks of 5

        // 100% pixel 0  x3

        c = src[0];
        DRAW_PIXEL4;
        DRAW_PIXEL4;
        DRAW_PIXEL4;

        // 20% pixel 0,  80% pixel 1

        c = stretch_tables[0][src[0] * 256 + src[1]];
        DRAW_PIXEL4;

        // 100% pixel 1 x2

        c = src[1];
        DRAW_PIXEL4;
        DRAW_PIXEL4;

        // 40% pixel 1, 60% pixel 2

        c = stretch_tables[1][src[1] * 256 + src[2]];
        DRAW_PIXEL4;

        // 100% pixel 2 x2

        c = src[2];
        DRAW_PIXEL4;
        DRAW_PIXEL4;

        // 60% pixel 2, 40% pixel 3

        c = stretch_tables[1][src[3] * 256 + src[2]];
        DRAW_PIXEL4;

        // 100% pixel 3 x2

        c = src[3];
        DRAW_PIXEL4;
        DRAW_PIXEL4;

        // 80% pixel 3, 20% pixel 4

        c = stretch_tables[0][src[4] * 256 + src[3]];
        DRAW_PIXEL4;

        // 100% pixel 4

        c = src[4];
        DRAW_PIXEL4;
        DRAW_PIXEL4;
        DRAW_PIXEL4;

        x += 5;
        src += 5;
    }
}

//
// 4x squashed (1024x800)
//

static boolean I_Squash4x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    bufp = src_buffer;
    screenp = (byte *) dest_buffer;

    for (y=0; y<SCREENHEIGHT; ++y) 
    {
        WriteSquashedLine4x(screenp, bufp);

        screenp += dest_pitch * 4;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_squash_4x = {
    SCREENWIDTH_4_3 * 4, SCREENHEIGHT * 4,
    I_InitStretchTables,
    I_Squash4x,
    false,
};

#define DRAW_PIXEL5 \
        *dest++ = *dest2++ = *dest3++ = *dest4++ = *dest5++ = c

static inline void WriteSquashedLine5x(byte *dest, byte *src)
{
    int x;
    int c;
    byte *dest2, *dest3, *dest4, *dest5;

    dest2 = dest + dest_pitch;
    dest3 = dest + dest_pitch * 2;
    dest4 = dest + dest_pitch * 3;
    dest5 = dest + dest_pitch * 4;

    for (x=0; x<SCREENWIDTH; ++x)
    {
        // Draw in blocks of 5

        // 100% pixel 0  x4

        c = *src++;
        DRAW_PIXEL5;
        DRAW_PIXEL5;
        DRAW_PIXEL5;
        DRAW_PIXEL5;
    }
}

//
// 5x squashed (1280x1000)
//

static boolean I_Squash5x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return false;
    }    

    bufp = src_buffer;
    screenp = (byte *) dest_buffer;

    for (y=0; y<SCREENHEIGHT; ++y) 
    {
        WriteSquashedLine5x(screenp, bufp);

        screenp += dest_pitch * 5;
        bufp += SCREENWIDTH;
    }

    return true;
}

screen_mode_t mode_squash_5x = {
    SCREENWIDTH_4_3 * 5, SCREENHEIGHT * 5,
    I_InitStretchTables,
    I_Squash5x,
    false,
};


