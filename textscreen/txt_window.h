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

typedef int (*TxtWindowKeyPress)(txt_window_t *window, int key,
                                 void *user_data);
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

    // URL of a webpage with help about this window. If set, a help key
    // indicator is shown while this window is active.
    char *help_url;
};

/**
 * Open a new window.
 *
 * @param title        Title to display in the titlebar of the new window
 *                     (UTF-8 format).
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
 * The position is specified as a pair of x, y, coordinates on the
 * screen. These specify the position of a particular point on the
 * window. The following are some examples:
 *
 * <code>
 *   // Centered on the screen:
 *
 *   TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_CENTER,
 *                                 TXT_SCREEN_W / 2, TXT_SCREEN_H / 2);
 *
 *   // Horizontally centered, with top of the window on line 6:
 *
 *   TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
 *                                 TXT_SCREEN_W / 2, 6);
 *
 *   // Top-left of window at 20, 6:
 *
 *   TXT_SetWindowPosition(window, TXT_HORIZ_LEFT, TXT_VERT_TOP, 20, 6);
 * </code>
 *
 * @param window       The window.
 * @param horiz_align  Horizontal location on the window for the X
 *                     position.
 * @param vert_align   Vertical location on the window for the Y position.
 * @param x            X coordinate (horizontal axis) for window position.
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

/**
 * Open a window displaying a message.
 *
 * @param title           Title of the window (UTF-8 format).
 * @param message         The message to display in the window (UTF-8 format).
 * @return                The new window.
 */

txt_window_t *TXT_MessageBox(char *title, char *message, ...);

/**
 * Set the help URL for the given window.
 *
 * @param window          The window.
 * @param help_url        String containing URL of the help page for this
 *                        window, or NULL to set no help for this window.
 */

void TXT_SetWindowHelpURL(txt_window_t *window, char *help_url);

/**
 * Open the help URL for the given window, if one is set.
 *
 * @param window          The window.
 */

void TXT_OpenWindowHelpURL(txt_window_t *window);

#endif /* #ifndef TXT_WINDOW_H */

