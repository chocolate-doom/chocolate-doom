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

#ifndef TXT_WINDOW_H
#define TXT_WINDOW_H

/**
 * @file txt_window.h
 *
 * Windows.
 */

/**
 * A window.
 *
 * A window contains widgets, and may also be treated as a table
 * (@ref txt_table_t) containing a single column.
 *
 * Windows can be created using @ref TXT_NewWindow and closed using
 * @ref TXT_CloseWindow.  When a window is closed, it emits the
 * "closed" signal.
 *
 * In addition to the widgets within a window, windows also have
 * a "tray" area at their bottom containing window action widgets.
 * These widgets allow keyboard shortcuts to trigger common actions.
 * Each window has three slots for keyboard shortcuts. By default,
 * the left slot contains an action to close the window when the
 * escape button is pressed, while the right slot contains an
 * action to activate the currently-selected widget.
 */

typedef struct txt_window_s txt_window_t;

#include "txt_widget.h" 
#include "txt_table.h"
#include "txt_window_action.h"

// Callback function for window key presses

typedef int (*TxtWindowKeyPress)(txt_window_t *window, int key, void *user_data);
typedef int (*TxtWindowMousePress)(txt_window_t *window, 
                                   int x, int y, int b, 
                                   void *user_data);

struct txt_window_s
{
    // Base class: all windows are tables with one column.

    txt_table_t table;
    
    // Window title

    char *title;

    // Screen coordinates of the window

    txt_vert_align_t vert_align;
    txt_horiz_align_t horiz_align;
    int x, y;

    // Actions that appear in the box at the bottom of the window

    txt_window_action_t *actions[3];

    // Callback functions to invoke when keys/mouse buttons are pressed

    TxtWindowKeyPress key_listener;
    void *key_listener_data;
    TxtWindowMousePress mouse_listener;
    void *mouse_listener_data;

    // These are set automatically when the window is drawn

    int window_x, window_y;
    unsigned int window_w, window_h;
};

/**
 * Open a new window.
 *
 * @param title        Title to display in the titlebar of the new window.
 * @return             Pointer to a new @ref txt_window_t structure
 *                     representing the new window.
 */

txt_window_t *TXT_NewWindow(char *title);

/**
 * Close a window.
 *
 * @param window       Tine window to close.
 */

void TXT_CloseWindow(txt_window_t *window);

/**
 * Set the position of a window on the screen.
 *
 * The window is specified as coordinates relative to a predefined
 * position on the screen (eg. center of the screen, top left of the
 * screen, etc).
 *
 * @param window       The window.
 * @param horiz_align  Horizontal position on the screen to which the
 *                     coordinates are relative (left side, right side
 *                     or center).
 * @param vert_align   Vertical position on the screen to which the
 *                     coordinates are relative (top, bottom or center).
 * @param x            X coordinate (horizonal axis) for window position.
 * @param y            Y coordinate (vertical axis) for window position.
 */

void TXT_SetWindowPosition(txt_window_t *window,
                           txt_horiz_align_t horiz_align,
                           txt_vert_align_t vert_align,
                           int x, int y);

/**
 * Set a window action for a given window.
 *
 * Each window can have up to three window actions, which provide
 * keyboard shortcuts that can be used within a given window.
 *
 * @param window      The window.
 * @param position    The window action slot to set (left, center or right).
 * @param action      The window action widget.  If this is NULL, any
 *                    current window action in the given slot is removed.
 */

void TXT_SetWindowAction(txt_window_t *window, txt_horiz_align_t position, 
                         txt_window_action_t *action);

/**
 * Set a callback function to be invoked whenever a key is pressed within
 * a window.
 *
 * @param window        The window.
 * @param key_listener  Callback function.
 * @param user_data     User-specified pointer to pass to the callback
 *                      function.
 */

void TXT_SetKeyListener(txt_window_t *window,
                        TxtWindowKeyPress key_listener,
                        void *user_data);

/**
 * Set a callback function to be invoked whenever a mouse button is pressed
 * within a window.
 *
 * @param window          The window.
 * @param mouse_listener  Callback function.
 * @param user_data       User-specified pointer to pass to the callback
 *                        function.
 */

void TXT_SetMouseListener(txt_window_t *window,
                          TxtWindowMousePress mouse_listener,
                          void *user_data);

#endif /* #ifndef TXT_WINDOW_T */


