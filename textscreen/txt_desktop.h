// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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

#ifndef TXT_DESKTOP_H
#define TXT_DESKTOP_H

#include "txt_window.h"

void TXT_AddDesktopWindow(txt_window_t *win);
void TXT_RemoveDesktopWindow(txt_window_t *win);
void TXT_SetDesktopTitle(char *title);
void TXT_DrawDesktop(void);
void TXT_GUIMainLoop(void);
void TXT_DispatchEvents(void);
void TXT_ExitMainLoop(void);
void TXT_DrawWindow(txt_window_t *window, int selected);
void TXT_WindowKeyPress(txt_window_t *window, int c);

#endif /* #ifndef TXT_DESKTOP_T */


