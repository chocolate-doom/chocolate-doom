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

/**
 * @file txt_desktop.h
 *
 * Textscreen desktop.
 */

#include "txt_window.h"

void TXT_AddDesktopWindow(txt_window_t *win);
void TXT_RemoveDesktopWindow(txt_window_t *win);
void TXT_DrawDesktop(void);
void TXT_DispatchEvents(void);
void TXT_DrawWindow(txt_window_t *window, int selected);
void TXT_WindowKeyPress(txt_window_t *window, int c);

/**
 * Set the title displayed at the top of the screen.
 *
 * @param title         The title to display.
 */

void TXT_SetDesktopTitle(char *title);

/**
 * Exit the currently-running main loop and return from the
 * @ref TXT_GUIMainLoop function.
 */

void TXT_ExitMainLoop(void);

/**
 * Start the main event loop.  At least one window must have been
 * opened prior to running this function.  When no windows are left
 * open, the event loop exits.
 *
 * It is possible to trigger an exit from this function using the
 * @ref TXT_ExitMainLoop function.
 */

void TXT_GUIMainLoop(void);

/**
 * Get the top window on the desktop that is currently receiving
 * inputs.
 *
 * @return    The active window, or NULL if no windows are present.
 */

txt_window_t *TXT_GetActiveWindow(void);

#endif /* #ifndef TXT_DESKTOP_H */


