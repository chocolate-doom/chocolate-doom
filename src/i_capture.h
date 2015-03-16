//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2015 Fabian Greffrath
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
//	Video capturing stuff.
//

#ifndef __I_CAPTURE__
#define __I_CAPTURE__

extern void *I_VideoCapture;

void I_InitCapture (const char *filename);
void I_CaptureEncode (void);
void I_SetCapturePalette (void *palette);
void I_QuitCapture (void);

#endif
