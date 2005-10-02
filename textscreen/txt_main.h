// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: txt_main.h 146 2005-10-02 03:16:03Z fraggle $
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
// $Log$
// Revision 1.1  2005/10/02 03:16:03  fraggle
// Text mode emulation code
//
//
//-----------------------------------------------------------------------------
//
// Text mode emulation in SDL
//
//-----------------------------------------------------------------------------

#ifndef TXT_MAIN_H
#define TXT_MAIN_H

// Initialise the screen
// Returns 1 if successful, 0 if failed.

int TXT_Init(void);

// Shut down text mode emulation

void TXT_Shutdown(void);

// Get a pointer to the buffer containing the raw screen data.

unsigned char *TXT_GetScreenData(void);

// Update an area of the screen

void TXT_UpdateScreenArea(int x, int y, int w, int h);

// Update the whole screen

void TXT_UpdateScreen(void);

#endif /* #ifndef TXT_MAIN_H */

