//
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

#ifndef TXT_DESKTOP_H
#define TXT_DESKTOP_H

/**
 * @file txt_desktop.h
 *
 * Textscreen desktop.
 */

#include "txt_window.h"

typedef void (*TxtIdleCallback)(void *user_data);

void TXT_AddDesktopWindow(txt_window_t *win);
void TXT_RemoveDesktopWindow(txt_window_t *win);
void TXT_DrawDesktop(void);
void TXT_DispatchEvents(void);
void TXT_DrawWindow(txt_window_t *window);
void TXT_SetWindowFocus(txt_window_t *window, int focused);
int TXT_WindowKeyPress(txt_window_t *window, int c);

/**
 * Set the title displayed at the top of the screen.
 *
 * @param title         The title to display (UTF-8 format).
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

/**
 * Set a callback function to be invoked periodically by the main
 * loop code.
 *
 * @param callback      The callback to invoke, or NULL to cancel
 *                      an existing callback.
 * @param user_data     Extra data to pass to the callback.
 * @param period        Maximum time between invoking each callback.
 *                      eg. a value of 200 will cause the callback
 *                      to be invoked at least once every 200ms.
 */

void TXT_SetPeriodicCallback(TxtIdleCallback callback,
                             void *user_data,
                             unsigned int period);

/**
 * Raise the z-position of the given window relative to other windows.
 *
 * @param window        The window to raise.
 * @return              Non-zero if the window was raised successfully,
 *                      or zero if the window could not be raised further.
 */

int TXT_RaiseWindow(txt_window_t *window);

/**
 * Lower the z-position of the given window relative to other windows.
 *
 * @param window        The window to make inactive.
 * @return              Non-zero if the window was lowered successfully,
 *                      or zero if the window could not be lowered further.
 */

int TXT_LowerWindow(txt_window_t *window);

#endif /* #ifndef TXT_DESKTOP_H */

