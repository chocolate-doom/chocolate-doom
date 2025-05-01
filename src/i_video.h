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
//	System specific interface stuff.
//


#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"
#include "i_truecolor.h"
#include "m_fixed.h" // [crispy] fixed_t
#include "crispy.h"

// Screen width and height.

#define ORIGWIDTH  320 // [crispy]
#define ORIGHEIGHT 200 // [crispy]

#define MAXWIDTH  (ORIGWIDTH << 2) // [crispy]
#define MAXHEIGHT (ORIGHEIGHT << 1) // [crispy]

extern int SCREENWIDTH;
extern int SCREENHEIGHT;
extern int NONWIDEWIDTH; // [crispy] non-widescreen SCREENWIDTH
extern int WIDESCREENDELTA; // [crispy] horizontal widescreen offset
void I_GetScreenDimensions (void); // [crispy] re-calculate WIDESCREENDELTA
void I_ToggleVsync (void); // [crispy] calls native SDL vsync toggle

// Screen height used when aspect_ratio_correct=true.

#define ORIGHEIGHT_4_3 240 // [crispy]
#define MAXHEIGHT_4_3 (ORIGHEIGHT_4_3 << 1) // [crispy]

extern int SCREENHEIGHT_4_3;

typedef boolean (*grabmouse_callback_t)(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);

void I_GraphicsCheckCommandLine(void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
#ifndef CRISPY_TRUECOLOR
void I_SetPalette (byte* palette);
int I_GetPaletteIndex(int r, int g, int b);
#else
void I_SetPalette (int palette);
extern const pixel_t I_MapRGB (const uint8_t r, const uint8_t g, const uint8_t b);
#endif

extern byte gamma2table[18][256];
void I_SetGammaTable (void);

extern fixed_t fractionaltic;

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

void I_ReadScreen (pixel_t* scr);

void I_BeginRead (void);

void I_SetWindowTitle(const char *title);

void I_CheckIsScreensaver(void);
void I_SetGrabMouseCallback(grabmouse_callback_t func);

void I_DisplayFPSDots(boolean dots_on);
void I_BindVideoVariables(void);

void I_InitWindowTitle(void);
void I_RegisterWindowIcon(const unsigned int *icon, int width, int height);
void I_InitWindowIcon(void);

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

void I_StartFrame (void);

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

void I_StartTic (void);

void I_UpdateFracTic (void); // [crispy]
void I_StartDisplay (void); // [crispy]

// Enable the loading disk image displayed when reading from disk.

void I_EnableLoadingDisk(int xoffs, int yoffs);

extern char *video_driver;
extern boolean screenvisible;

extern int vanilla_keyboard_mapping;
extern boolean screensaver_mode;
extern int usegamma;
extern pixel_t *I_VideoBuffer;

extern int screen_width;
extern int screen_height;
extern int fullscreen;
extern int aspect_ratio_correct;
extern int integer_scaling;
extern int smooth_pixel_scaling;
extern int vga_porch_flash;
extern int force_software_renderer;

extern int png_screenshots;

extern char *window_position;
void I_GetWindowPosition(int *x, int *y, int w, int h);

// Joystic/gamepad hysteresis
extern unsigned int joywait;

extern int usemouse;


#endif