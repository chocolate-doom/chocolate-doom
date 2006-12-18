// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005,2006 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//     Screen scale-up code: 
//         1x,2x,3x,4x pixel doubling
//         Aspect ratio-correcting stretch functions
//
//-----------------------------------------------------------------------------



#include "doomdef.h"
#include "doomtype.h"

#include "z_zone.h"

// Should be screens[0]

static byte *src_buffer;

// Destination buffer, ie. screen->pixels.

static byte *dest_buffer;

// Pitch of destination buffer, ie. screen->pitch.

static int dest_pitch;

// Lookup tables used for aspect ratio correction stretching code.
// stretch_tables[0] : 20% / 80%
// stretch_tables[1] : 40% / 60%
// All other combinations can be reached from these two tables.

static byte *stretch_tables[2];

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

void I_Scale1x(int x1, int y1, int x2, int y2)
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
}

void I_Scale2x(int x1, int y1, int x2, int y2)
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
}

void I_Scale3x(int x1, int y1, int x2, int y2)
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
}

void I_Scale4x(int x1, int y1, int x2, int y2)
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
}

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

void I_InitStretchTables(byte *palette)
{
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

// 
// Aspect ratio correcting scale up functions.
//
// These double up pixels to stretch the screen when using a 4:3
// screen mode.
//

static void WriteBlendedLine1x(byte *dest, byte *src1, byte *src2, 
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

void I_Stretch1x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return;
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
}

static void WriteLine2x(byte *dest, byte *src)
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

static void WriteBlendedLine2x(byte *dest, byte *src1, byte *src2, 
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

// Scale 2x, correcting aspect ratio in the process

void I_Stretch2x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return;
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
}

static void WriteLine3x(byte *dest, byte *src)
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

static void WriteBlendedLine3x(byte *dest, byte *src1, byte *src2, 
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

void I_Stretch3x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return;
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
}

static void WriteLine4x(byte *dest, byte *src)
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

static void WriteBlendedLine4x(byte *dest, byte *src1, byte *src2, 
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

void I_Stretch4x(int x1, int y1, int x2, int y2)
{
    byte *bufp, *screenp;
    int y;

    // Only works with full screen update

    if (x1 != 0 || y1 != 0 || x2 != SCREENWIDTH || y2 != SCREENHEIGHT)
    {
        return;
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
}

