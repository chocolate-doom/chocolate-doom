//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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



// HEADER FILES ------------------------------------------------------------

#include <stdarg.h>

#include "config.h"

#include "h2def.h"
#include "i_system.h"
#include "i_video.h"
#include "i_videohr.h"
#include "s_sound.h"
#include "st_start.h"


// MACROS ------------------------------------------------------------------
#define ST_MAX_NOTCHES		32
#define ST_NOTCH_WIDTH		16
#define ST_NOTCH_HEIGHT		23
#define ST_PROGRESS_X		64      // Start of notches x screen pos.
#define ST_PROGRESS_Y		441     // Start of notches y screen pos.

#define ST_NETPROGRESS_X		288
#define ST_NETPROGRESS_Y		32
#define ST_NETNOTCH_WIDTH		8
#define ST_NETNOTCH_HEIGHT		16
#define ST_MAX_NETNOTCHES		8

byte *ST_LoadScreen(void);
void ST_UpdateNotches(int notchPosition);
void ST_UpdateNetNotches(int notchPosition);

// PRIVATE DATA DEFINITIONS ------------------------------------------------
static const byte *bitmap = NULL;
int graphical_startup = 1;
static boolean using_graphical_startup;

static const byte notchTable[] = {
    // plane 0
    0x00, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40,
    0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x03, 0xC0,
    0x0F, 0x90, 0x1B, 0x68, 0x3D, 0xBC, 0x3F, 0xFC, 0x20, 0x08, 0x20, 0x08,
    0x2F, 0xD8, 0x37, 0xD8, 0x37, 0xF8, 0x1F, 0xF8, 0x1C, 0x50,

    // plane 1
    0x00, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x02, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0xA0,
    0x30, 0x6C, 0x24, 0x94, 0x42, 0x4A, 0x60, 0x0E, 0x60, 0x06, 0x7F, 0xF6,
    0x7F, 0xF6, 0x7F, 0xF6, 0x5E, 0xF6, 0x38, 0x16, 0x23, 0xAC,

    // plane 2
    0x00, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x02, 0x40, 0x02, 0x40,
    0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x03, 0xE0,
    0x30, 0x6C, 0x24, 0x94, 0x52, 0x6A, 0x7F, 0xFE, 0x60, 0x0E, 0x60, 0x0E,
    0x6F, 0xD6, 0x77, 0xD6, 0x56, 0xF6, 0x38, 0x36, 0x23, 0xAC,

    // plane 3
    0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
    0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0x80, 0x02, 0x40,
    0x0F, 0x90, 0x1B, 0x68, 0x3D, 0xB4, 0x1F, 0xF0, 0x1F, 0xF8, 0x1F, 0xF8,
    0x10, 0x28, 0x08, 0x28, 0x29, 0x08, 0x07, 0xE8, 0x1C, 0x50
};


// Red Network Progress notches
static const byte netnotchTable[] = {
    // plane 0
    0x80, 0x50, 0xD0, 0xf0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xD0, 0xF0, 0xC0,
    0x70, 0x50, 0x80, 0x60,

    // plane 1
    0x60, 0xE0, 0xE0, 0xA0, 0xA0, 0xA0, 0xE0, 0xA0, 0xA0, 0xA0, 0xE0, 0xA0,
    0xA0, 0xE0, 0x60, 0x00,

    // plane 2
    0x80, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
    0x10, 0x10, 0x80, 0x60,

    // plane 3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

// CODE --------------------------------------------------------------------



//--------------------------------------------------------------------------
//
// Startup Screen Functions
//
//--------------------------------------------------------------------------


//==========================================================================
//
// ST_Init - Do the startup screen
//
//==========================================================================

void ST_Init(void)
{
    byte *pal;
    byte *buffer;
    
    using_graphical_startup = false;

    if (graphical_startup && !debugmode && !testcontrols)
    {
        I_SetWindowTitleHR("Hexen startup - " PACKAGE_STRING);

        // Set 640x480x16 mode
        if (I_SetVideoModeHR())
        {
            using_graphical_startup = true;
            I_InitWindowIcon();

            S_StartSongName("orb", true);

            I_ClearScreenHR();
            I_InitPaletteHR();
            I_BlackPaletteHR();

            // Load graphic
            buffer = ST_LoadScreen();
            pal = buffer;
            bitmap = buffer + 16 * 3;

            I_SlamHR(bitmap);
            I_FadeToPaletteHR(pal);
            Z_Free(buffer);
        }
    }
}

void ST_Done(void)
{
    if (using_graphical_startup)
    {
        I_ClearScreenHR();
        I_UnsetVideoModeHR();
    }
}


//==========================================================================
//
// ST_UpdateNotches
//
//==========================================================================

void ST_UpdateNotches(int notchPosition)
{
    int x = ST_PROGRESS_X + notchPosition * ST_NOTCH_WIDTH;
    int y = ST_PROGRESS_Y;
    I_SlamBlockHR(x, y, ST_NOTCH_WIDTH, ST_NOTCH_HEIGHT, notchTable);
}


//==========================================================================
//
// ST_UpdateNetNotches - indicates network progress
//
//==========================================================================

void ST_UpdateNetNotches(int notchPosition)
{
    int x = ST_NETPROGRESS_X + notchPosition * ST_NETNOTCH_WIDTH;
    int y = ST_NETPROGRESS_Y;
    I_SlamBlockHR(x, y, ST_NETNOTCH_WIDTH, ST_NETNOTCH_HEIGHT, netnotchTable);
}


//==========================================================================
//
// ST_Progress - increments progress indicator
//
//==========================================================================

void ST_Progress(void)
{
    // Check for ESC press -- during startup all events eaten here
    if (I_CheckAbortHR())
    {
        I_Quit();
    }

    if (using_graphical_startup)
    {
        static int notchPosition = 0;

        if (notchPosition < ST_MAX_NOTCHES)
        {
            ST_UpdateNotches(notchPosition);
            S_StartSound(NULL, SFX_STARTUP_TICK);
            //I_Sleep(1000);
            notchPosition++;
        }
    }

    printf(".");
}


//==========================================================================
//
// ST_NetProgress - indicates network progress
//
//==========================================================================

void ST_NetProgress(void)
{
    printf("*");

    if (using_graphical_startup)
    {
        static int netnotchPosition = 0;

        if (netnotchPosition < ST_MAX_NETNOTCHES)
        {
            ST_UpdateNetNotches(netnotchPosition);
            S_StartSound(NULL, SFX_DRIP);
            netnotchPosition++;
        }
    }
}


//==========================================================================
//
// ST_NetDone - net progress complete
//
//==========================================================================
void ST_NetDone(void)
{
    if (using_graphical_startup)
    {
        S_StartSound(NULL, SFX_PICKUP_WEAPON);
    }
}


//==========================================================================
//
// ST_Message - gives debug message
//
//==========================================================================

void ST_Message(char *message, ...)
{
    va_list argptr;

    va_start(argptr, message);
    vprintf(message, argptr);
    va_end(argptr);
}

//==========================================================================
//
// ST_RealMessage - gives user message
//
//==========================================================================

void ST_RealMessage(char *message, ...)
{
    va_list argptr;

    va_start(argptr, message);
    vprintf(message, argptr);
    va_end(argptr);
}



//==========================================================================
//
// ST_LoadScreen - loads startup graphic
//
//==========================================================================


byte *ST_LoadScreen(void)
{
    int length, lump;
    byte *buffer;

    lump = W_GetNumForName("STARTUP");
    length = W_LumpLength(lump);
    buffer = (byte *) Z_Malloc(length, PU_STATIC, NULL);
    W_ReadLump(lump, buffer);
    return (buffer);
}

