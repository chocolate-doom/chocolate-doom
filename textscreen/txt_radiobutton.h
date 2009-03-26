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

#ifndef TXT_RADIOBUTTON_H
#define TXT_RADIOBUTTON_H

/**
 * @file txt_radiobutton.h
 *
 * Radio button widget.
 */

/**
 * A radio button widget.
 *
 * Radio buttons are typically used in groups, to allow a value to be
 * selected from a range of options.  Each radio button corresponds
 * to a particular option that may be selected.  A radio button
 * has an indicator to indicate whether it is the currently-selected
 * value, and a text label.
 *
 * Internally, a radio button tracks an integer variable that may take
 * a range of different values.  Each radio button has a particular
 * value associated with it; if the variable is equal to that value,
 * the radio button appears selected.  If a radio button is pressed
 * by the user through the GUI, the variable is set to its value.
 *
 * When a radio button is selected, the "selected" signal is emitted.
 */

typedef struct txt_radiobutton_s txt_radiobutton_t;

#include "txt_widget.h"

struct txt_radiobutton_s
{
    txt_widget_t widget;
    char *label;
    int *variable;
    int value;
};

/**
 * Create a new radio button widget.
 *
 * @param label          The label to display next to the radio button.
 * @param variable       Pointer to the variable tracking whether this
 *                       radio button is selected.
 * @param value          If the variable is equal to this value, the
 *                       radio button appears selected.
 * @return               Pointer to the new radio button widget.
 */

txt_radiobutton_t *TXT_NewRadioButton(char *label, int *variable, int value);

/**
 * Set the label on a radio button.
 *
 * @param radiobutton    The radio button.
 * @param value          The new label.
 */

void TXT_SetRadioButtonLabel(txt_radiobutton_t *radiobutton, char *value);

#endif /* #ifndef TXT_RADIOBUTTON_H */


