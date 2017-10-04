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

#ifndef TXT_WINDOW_ACTION_H
#define TXT_WINDOW_ACTION_H

/**
 * @file txt_window_action.h
 *
 * Window action widget.
 */

/**
 * Window action widget.
 *
 * A window action is attached to a window and corresponds to a
 * keyboard shortcut that is active within that window.  When the
 * key is pressed, the action is triggered.
 *
 * When a window action is triggered, the "pressed" signal is emitted.
 */

typedef struct txt_window_action_s txt_window_action_t;

#include "txt_widget.h"
#include "txt_window.h"

struct txt_window_action_s
{
    txt_widget_t widget;
    char *label;
    int key;
};

/**
 * Create a new window action.
 *
 * @param key           The keyboard key that triggers this action.
 * @param label         Label to display for this action in the tray
 *                      at the bottom of the window (UTF-8 format).
 * @return              Pointer to the new window action widget.
 */

txt_window_action_t *TXT_NewWindowAction(int key, const char *label);

/**
 * Create a new window action that closes the window when the
 * escape key is pressed.  The label "Close" is used.
 *
 * @param window        The window to close.
 * @return              Pointer to the new window action widget.
 */

txt_window_action_t *TXT_NewWindowEscapeAction(txt_window_t *window);

/**
 * Create a new window action that closes the window when the
 * escape key is pressed.  The label "Abort" is used.
 *
 * @param window        The window to close.
 * @return              Pointer to the new window action widget.
 */

txt_window_action_t *TXT_NewWindowAbortAction(txt_window_t *window);

/**
 * Create a new "select" window action.  This does not really do
 * anything, but reminds the user that "enter" can be pressed to
 * activate the currently-selected widget.
 *
 * @param window        The window.
 * @return              Pointer to the new window action widget.
 */

txt_window_action_t *TXT_NewWindowSelectAction(txt_window_t *window);

#endif /* #ifndef TXT_WINDOW_ACTION_H */

