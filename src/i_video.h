// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

// Screen width and height.

#define SCREENWIDTH  320
#define SCREENHEIGHT 200

// Screen width used for "squash" scale functions

#define SCREENWIDTH_4_3 256

// Screen height used for "stretch" scale functions.

#define SCREENHEIGHT_4_3 240

#define MAX_MOUSE_BUTTONS 8

typedef struct
{
    // Screen width and height

    int width;
    int height;

    // Initialisation function to call when using this mode.
    // Called with a pointer to the Doom palette.
    //
    // If NULL, no init function is called.

    void (*InitMode)(byte *palette);
    
    // Function to call to draw the screen from the source buffer.
    // Return true if draw was successful.

    boolean (*DrawScreen)(int x1, int y1, int x2, int y2);

    // If true, this is a "poor quality" mode.  The autoadjust 
    // code should always attempt to use a different mode to this 
    // mode in fullscreen.

    boolean poor_quality;
} screen_mode_t;

typedef boolean (*grabmouse_callback_t)(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);


void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (byte* palette);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);
void I_EndRead (void);

void I_SetWindowTitle(char *title);

void I_CheckIsScreensaver(void);
void I_SetGrabMouseCallback(grabmouse_callback_t func);

void I_DisplayFPSDots(boolean dots_on);
void I_BindVideoVariables(void);

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

void I_StartFrame (void);

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

void I_StartTic (void);

// Enable the loading disk image displayed when reading from disk.

void I_EnableLoadingDisk(void);

extern char *video_driver;
extern boolean screenvisible;

extern float mouse_acceleration;
extern int mouse_threshold;
extern int vanilla_keyboard_mapping;
extern boolean screensaver_mode;
extern int usegamma;
extern byte *I_VideoBuffer;

#endif

