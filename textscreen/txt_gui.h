// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
// Revision 1.1  2006/01/13 23:56:00  fraggle
// Add text-mode I/O functions.
// Use text-mode screen for the waiting screen.
//
// Revision 1.2  2006/01/13 18:23:28  fraggle
// Textscreen getchar() function; remove SDL code from I_Endoom.
//
// Revision 1.1  2005/10/02 03:16:03  fraggle
// Text mode emulation code
//
//
//-----------------------------------------------------------------------------
//
// Text mode emulation in SDL
//
//-----------------------------------------------------------------------------

#ifndef TXT_GUI_H
#define TXT_GUI_H

void TXT_DrawDesktopBackground(char *title);
void TXT_DrawWindowFrame(char *title, int x, int y, int w, int h);
void TXT_DrawSeparator(int x, int y, int w);

#endif /* #ifndef TXT_GUI_H */

