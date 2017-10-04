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

#ifndef TXT_DROPDOWN_H
#define TXT_DROPDOWN_H

/**
 * @file txt_dropdown.h
 *
 * Dropdown list widget.
 */

/**
 * Dropdown list widget.
 *
 * A dropdown list allows the user to select from a list of values,
 * which appears when the list is selected.
 *
 * When the value of a dropdown list is changed, the "changed" signal
 * is emitted.
 */

typedef struct txt_dropdown_list_s txt_dropdown_list_t;

#include "txt_widget.h"

//
// Drop-down list box.
// 

struct txt_dropdown_list_s
{
    txt_widget_t widget;
    int *variable;
    char **values;
    int num_values;
};

/**
 * Create a new dropdown list widget.
 *
 * The parameters specify a list of string labels, and a pointer to an
 * integer variable.  The variable contains the current "value" of the
 * list, as an index within the list of labels.
 *
 * @param variable        Pointer to the variable containing the
 *                        list's value.
 * @param values          Pointer to an array of strings containing
 *                        the labels to use for the list (UTF-8 format).
 * @param num_values      The number of variables in the list.
 */

txt_dropdown_list_t *TXT_NewDropdownList(int *variable, 
                                         char **values, int num_values);

#endif /* #ifndef TXT_DROPDOWN_H */


