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

#ifndef TXT_SCROLLPANE_H
#define TXT_SCROLLPANE_H

/**
 * @file txt_scrollpane.h
 *
 * Scrollable pane widget.
 */

/**
 * Scrollable pane widget.
 *
 * A scrollable pane widget is a widget that contains another widget
 * that is larger than it.  Scroll bars appear on the side to allow
 * different areas of the contained widget to be seen.
 */

typedef struct txt_scrollpane_s txt_scrollpane_t;

#include "txt_widget.h"

struct txt_scrollpane_s
{
    txt_widget_t widget;
    int w, h;
    int x, y;
    int expand_w, expand_h;
    txt_widget_t *child;
};

/**
 * Create a new scroll pane widget.
 *
 * @param w               Width of the scroll pane, in characters.
 * @param h               Height of the scroll pane, in lines.
 * @param target          The target widget that the scroll pane will
 *                        contain.
 * @return                Pointer to the new scroll pane widget.
 */

txt_scrollpane_t *TXT_NewScrollPane(int w, int h, TXT_UNCAST_ARG(target));

#endif /* #ifndef TXT_SCROLLPANE_H */


