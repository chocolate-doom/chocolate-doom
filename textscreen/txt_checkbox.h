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

#ifndef TXT_CHECKBOX_H
#define TXT_CHECKBOX_H

/**
 * @file txt_checkbox.h
 *
 * Checkbox widget.
 */

/**
 * Checkbox widget.
 *
 * A checkbox is used to control boolean values that may be either on
 * or off.  The widget has a label that is displayed to the right of
 * the checkbox indicator.  The widget tracks an integer variable;
 * if the variable is non-zero, the checkbox is checked, while if it
 * is zero, the checkbox is unchecked.  It is also possible to
 * create "inverted" checkboxes where this logic is reversed.
 *
 * When a checkbox is changed, it emits the "changed" signal.
 */

typedef struct txt_checkbox_s txt_checkbox_t;

#include "txt_widget.h"

struct txt_checkbox_s
{
    txt_widget_t widget;
    char *label;
    int *variable;
    int inverted;
};

/**
 * Create a new checkbox.
 *
 * @param label         The label for the new checkbox (UTF-8 format).
 * @param variable      Pointer to the variable containing this checkbox's
 *                      value.
 * @return              Pointer to the new checkbox.
 */

txt_checkbox_t *TXT_NewCheckBox(char *label, int *variable);

/**
 * Create a new inverted checkbox.
 *
 * An inverted checkbox displays the opposite of a normal checkbox;
 * where it would be checked, it appears unchecked, and vice-versa.
 *
 * @param label         The label for the new checkbox (UTF-8 format).
 * @param variable      Pointer to the variable containing this checkbox's
 *                      value.
 * @return              Pointer to the new checkbox.
 */

txt_checkbox_t *TXT_NewInvertedCheckBox(char *label, int *variable);

#endif /* #ifndef TXT_CHECKBOX_H */


