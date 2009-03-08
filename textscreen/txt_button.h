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

#ifndef TXT_BUTTON_H
#define TXT_BUTTON_H

/**
 * @file txt_button.h
 *
 * Button widget.
 */

/**
 * Button widget.
 *
 * A button is a widget that can be selected to perform some action.
 * When a button is pressed, it emits the "pressed" signal.
 */

typedef struct txt_button_s txt_button_t;

#include "txt_widget.h"

struct txt_button_s
{
    txt_widget_t widget;
    char *label;
};

/**
 * Create a new button widget.
 *
 * @param label        The label to use on the new button.
 * @return             Pointer to the new button widget.
 */

txt_button_t *TXT_NewButton(char *label);

/**
 * Create a new button widget, binding the "pressed" signal to a
 * specified callback function.
 *
 * @param label        The label to use on the new button.
 * @param func         The callback function to invoke.
 * @param user_data    User-specified pointer to pass to the callback.
 * @return             Pointer to the new button widget.
 */

txt_button_t *TXT_NewButton2(char *label, TxtWidgetSignalFunc func,
                             void *user_data);

/**
 * Change the label used on a button.
 *
 * @param button       The button.
 * @param label        The new label.
 */

void TXT_SetButtonLabel(txt_button_t *button, char *label);

#endif /* #ifndef TXT_BUTTON_H */


