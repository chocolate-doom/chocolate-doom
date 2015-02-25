//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015      James Haley
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
//    Medusa Effect emulation
//

#include <stdlib.h>
#include <time.h>
#include "doomtype.h"
#include "r_defs.h"
#include "r_draw.h"

// The zone heap was allocated at a max of 6 MB; on average, a texture will be
// allocated near the middle of the heap due to constant purging of PU_CACHE
// resources. Use a 3 Megabyte buffer by default for emulation.
#define DEFAULT_MEDUSA_BUFFER 3*1024*1024

#define MEDUSA_COLUMN_STARTS 3

// Simulated zone block header, a pattern which will frequently occur
static const byte zoneblock[] = 
{ 
    0x48, 0x14, 0x00, 0x00, // 5192 == size of DOOR3_6 (conveniently; column ends with block)
    0x08, 0x16, 0x10, 0x00, // user (low on heap, wad dir created at startup)
    0x65, 0x00, 0x00, 0x00, // tag  (PU_CACHE)
    0x11, 0x4A, 0x1D, 0x00, // ZONEID
    0x20, 0x56, 0x10, 0x00, // next
    0xD8, 0x3D, 0x10, 0x00, // prev (next - (5192 + 1024), a reasonable gap size)
};

// TODO: allow customization?
static size_t r_medusabuffersize = DEFAULT_MEDUSA_BUFFER;

static byte *r_medusacolumnstarts[MEDUSA_COLUMN_STARTS];

byte *r_medusabuffer;

//
// Fill the Medusa buffer with simulated garbage.
//
static void R_FillMedusaBuffer(void)
{
    byte  *rover     = r_medusabuffer;
    size_t remaining = r_medusabuffersize - 2;
    int    numcolumnstarts = 0;

    r_medusacolumnstarts[0] = r_medusabuffer;
    r_medusacolumnstarts[1] = r_medusabuffer;
    r_medusacolumnstarts[2] = r_medusabuffer;

    while(remaining >= 0)
    {
        byte i;
        byte topdelta = (byte)(rand() % 254);
        byte length   = (byte)(rand() % 252);

        if(length + 4 > remaining)
            break;

        if(rand() % 37 == 0 && numcolumnstarts < MEDUSA_COLUMN_STARTS)
            r_medusacolumnstarts[numcolumnstarts++] = rover;

        // possible early termination
        if(rand() % 67 == 0 && topdelta >= 200)
            topdelta = 255;

        // post header
        *rover++ = topdelta;
        *rover++ = length;

        // pad bytes and column content
        if(rand() % 67 == 0)
        {
            // a memset or solid data region may occur sometimes
            byte content = rand() % 255;
            for(i = 0; i < length + 2; i++)
                *rover++ = content;
        }
        else if((rand() & 1) && length + 2 <= sizeof(zoneblock))
        {
            // sprinkle in some zone blocks.
            for(i = 0; i < length + 2; i++)
                *rover++ = zoneblock[i];
        }
        else
        {
            // noise
            for(i = 0; i < length + 2; i++)
            {
                if(rand() % 2 == 0) // 50% chance of low-value data
                    *rover++ = (byte)(rand() % 16);
                else
                    *rover++ = (byte)(rand() % 255);
            }
        }

        remaining -= (length + 4);
    }

    // guarantee a terminating post at the end of the Medusa buffer
    *rover++ = 255;
    *rover++ = 0;
}

//
// Allocate the Medusa buffer if it hasn't been allocated already
//
void R_InitMedusaBuffer(void)
{
    srand((unsigned int)time(NULL));
    
    if(!r_medusabuffer)
    {
        if(!(r_medusabuffer = malloc(r_medusabuffersize)))
            return; // well that's tough I guess, no emulation.
    }

    R_FillMedusaBuffer();
}

//
// Get a pointer to the Medusa Effect emulation buffer.
//
byte *R_GetMedusaBuffer(void)
{
    static int count;

    if(++count >= 100000 && (rand() & 1))
    {
        // the heap does change now and then.
        R_FillMedusaBuffer();
        count = 0;
    }

    return r_medusacolumnstarts[dc_x % MEDUSA_COLUMN_STARTS];
}

// EOF

