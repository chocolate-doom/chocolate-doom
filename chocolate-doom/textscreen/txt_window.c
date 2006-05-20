// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1993-1996 Id Software, Inc.
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

#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_desktop.h"
#include "txt_gui.h"
#include "txt_main.h"
#include "txt_separator.h"
#include "txt_window.h"

txt_window_t *TXT_NewWindow(char *title)
{
    txt_window_t *win;

    win = malloc(sizeof(txt_window_t));

    win->title = strdup(title);
    win->x = TXT_SCREEN_W / 2;
    win->y = TXT_SCREEN_H / 2;
    win->horiz_align = TXT_HORIZ_CENTER;
    win->vert_align = TXT_VERT_CENTER;
    win->widgets = NULL;
    win->num_widgets = 0;
    win->selected = 0;

    TXT_AddDesktopWindow(win);

    return win;
}

void TXT_CloseWindow(txt_window_t *window)
{
    int i;

    // Free all widgets

    for (i=0; i<window->num_widgets; ++i)
    {
        TXT_DestroyWidget(window->widgets[i]);
    }
    
    // Free window resources

    free(window->widgets);
    free(window->title);
    free(window);

    TXT_RemoveDesktopWindow(window);
}

static void CalcWindowSize(txt_window_t *window, int *w, int *h)
{
    txt_widget_t *widget;
    int max_width;
    int height;
    int i;
    int ww;

    max_width = 10;
    height = 0;

    for (i=0; i<window->num_widgets; ++i)
    {
        ww = TXT_WidgetWidth(window->widgets[i]);

        if (ww > max_width)
            max_width = ww;

        if (window->widgets[i]->visible)
        {
            height += 1;
        }
    }

    *w = max_width;
    *h = height;
}

static void CalcWindowPosition(txt_window_t *window,
                               int *x, int *y,
                               int w, int h)
{
    switch (window->horiz_align)
    {
        case TXT_HORIZ_LEFT:
            *x = window->x;
            break;
        case TXT_HORIZ_CENTER:
            *x = window->x - (w / 2);
            break;
        case TXT_HORIZ_RIGHT:
            *x = window->x - (w - 1);
            break;
    }

    switch (window->vert_align)
    {
        case TXT_VERT_TOP:
            *y = window->y;
            break;
        case TXT_VERT_CENTER:
            *y = window->y - (h / 2);
            break;
        case TXT_VERT_BOTTOM:
            *y = window->y - (h - 1);
            break;
    }
}

void TXT_DrawWindow(txt_window_t *window)
{
    int widgets_w, widgets_h;
    int window_w, window_h;
    int x, y;
    int window_x, window_y;
    int i;
    
    CalcWindowSize(window, &widgets_w, &widgets_h);

    // Actual window size after padding

    window_w = widgets_w + 2;
    window_h = widgets_h + 5;

    // Use the x,y position as the centerpoint and find the location to 
    // draw the window.

    CalcWindowPosition(window, &window_x, &window_y, window_w, window_h);

    // Draw the window

    TXT_DrawWindowFrame(window->title, window_x, window_y, window_w, window_h);

    // Draw all widgets

    x = window_x + 1;
    y = window_y + 2;

    for (i=0; i<window->num_widgets; ++i)
    {
        TXT_GotoXY(x, y);
        TXT_DrawWidget(window->widgets[i], widgets_w, i == window->selected);

        if (window->widgets[i]->visible)
        {
            y += 1;
        }
    }

    // Separator for action area

    TXT_DrawSeparator(window_x, y, window_w);
}

void TXT_AddWidget(txt_window_t *window, void *uncast_widget)
{
    txt_widget_t *widget;

    widget = (txt_widget_t *) uncast_widget;

    if (window->num_widgets == 0)
    {
        // This is the first widget added.
        //
        // The first widget in a window should always be a separator.
        // If we are not adding a separator now, add one in first.

        if (widget->widget_class != &txt_separator_class)
        {
            txt_separator_t *separator;

            separator = TXT_NewSeparator(NULL);

            window->widgets = malloc(sizeof(txt_widget_t *));
            window->widgets[0] = &separator->widget;
            window->num_widgets = 1;
        }
    }
    
    window->widgets = realloc(window->widgets,
                              sizeof(txt_widget_t *) * (window->num_widgets + 1));
    window->widgets[window->num_widgets] = widget;
    ++window->num_widgets;
}

void TXT_SetWindowPosition(txt_window_t *window,
                           txt_horiz_align_t horiz_align,
                           txt_vert_align_t vert_align,
                           int x, int y)
{
    window->vert_align = vert_align;
    window->horiz_align = horiz_align;
    window->x = x;
    window->y = y;
}

void TXT_WindowKeyPress(txt_window_t *window, int c)
{
    // Send to the currently selected widget first

    if (window->selected > 0 && window->selected <= window->num_widgets)
    {
        if (TXT_WidgetKeyPress(window->widgets[window->selected], c))
        {
            return;
        }
    }

    if (c == KEY_DOWNARROW)
    {
        int newsel;

        // Move cursor down to the next selectable widget

        for (newsel = window->selected + 1;
             newsel < window->num_widgets;
             ++newsel)
        {
            if (window->widgets[newsel]->visible
             && window->widgets[newsel]->selectable)
            {
                window->selected = newsel;
                break;
            }
        } 
    }

    if (c == KEY_UPARROW)
    {
        int newsel;

        // Move cursor down to the next selectable widget

        for (newsel = window->selected - 1;
             newsel >= 0;
             --newsel)
        {
            if (window->widgets[newsel]->visible
             && window->widgets[newsel]->selectable)
            {
                window->selected = newsel;
                break;
            }
        } 
    }
}

