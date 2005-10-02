// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_video.h 147 2005-10-02 03:16:29Z fraggle $
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



// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);


void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (byte* palette);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);
void I_EndRead (void);

void I_SetWindowCaption(void);
void I_SetWindowIcon(void);

extern boolean screenvisible;
extern int screenmultiply;
extern boolean fullscreen;
extern boolean grabmouse;
extern float mouse_acceleration;

#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.8  2005/10/02 03:16:29  fraggle
// ENDOOM support using text mode emulation
//
// Revision 1.7  2005/09/17 20:25:56  fraggle
// Set the default values for variables in their initialisers.  Remove the
// "defaultvalue" parameter and associated code from the configuration
// file parsing code.
//
// Revision 1.6  2005/09/11 20:25:56  fraggle
// Second configuration file to allow chocolate doom-specific settings.
// Adjust some existing command line logic (for graphics settings and
// novert) to adjust for this.
//
// Revision 1.5  2005/09/11 16:39:29  fraggle
// Fix declaration of I_Sleep (not I_Delay) and move to right header
//
// Revision 1.4  2005/09/11 16:35:04  fraggle
// Missing declarations
//
// Revision 1.3  2005/07/23 18:56:07  fraggle
// Remove unneccessary pragmas
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:19:58  fraggle
// Initial import
//
//
//-----------------------------------------------------------------------------
