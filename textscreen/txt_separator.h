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

#ifndef TXT_SEPARATOR_H
#define TXT_SEPARATOR_H

/**
 * @file txt_separator.h
 *
 * Horizontal separator widget.
 */

/**
 * Horizontal separator.
 *
 * A horizontal separator appears as a horizontal line divider across
 * the length of the window in which it is added.  An optional label
 * allows the separator to be used as a section divider for grouping
 * related controls.
 */

typedef struct txt_separator_s txt_separator_t;

#include "txt_widget.h"

struct txt_separator_s
{
    txt_widget_t widget;
    char *label;
};

extern txt_widget_class_t txt_separator_class;

/**
 * Create a new horizontal separator widget.
 *
 * @param label         Label to display on the separator (UTF-8 format).
 *                      If this is set to NULL, no label is displayed.
 * @return              The new separator widget.
 */

txt_separator_t *TXT_NewSeparator(char *label);

/**
 * Change the label on a separator.
 *
 * @param separator     The separator.
 * @param label         The new label (UTF-8 format).
 */

void TXT_SetSeparatorLabel(txt_separator_t *separator, char *label);

#endif /* #ifndef TXT_SEPARATOR_H */

