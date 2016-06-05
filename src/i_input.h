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
//    System-specific keyboard/mouse input.
//


#ifndef __I_INPUT__
#define __I_INPUT__

#include "doomtype.h"

#define MAX_MOUSE_BUTTONS 8

extern float mouse_acceleration;
extern int mouse_threshold;

void I_BindInputVariables(void);
void I_ReadMouse(void);

// I_StartTextInput begins text input, activating the on-screen keyboard
// (if one is used). The caller indicates that any entered text will be
// displayed in the rectangle given by the provided set of coordinates.
void I_StartTextInput(int x1, int y1, int x2, int y2);

// I_StopTextInput finishes text input, deactivating the on-screen keyboard
// (if one is used).
void I_StopTextInput(void);

#endif
