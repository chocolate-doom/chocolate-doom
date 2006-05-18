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

#include "txt_gui.h"
#include "txt_separator.h"
#include "txt_window.h"

#define MAXWINDOWS 128

static txt_window_t *all_windows[MAXWINDOWS];
static int num_windows = 0;

static void AddWindow(txt_window_t *win)
{
    all_windows[num_windows] = win;
    ++num_windows;
}

static void RemoveWindow(txt_window_t *win)
{
    int from, to;

    for (from=0, to=0; from<num_windows; ++from)
    {
        if (all_windows[from] != win)
        {
            all_windows[to] = all_windows[from];
            ++to;
        }
    }
    
    num_windows = to;
}

txt_window_t *TXT_NewWindow(char *title, int x, int y)
{
    txt_window_t *win;

    win = malloc(sizeof(txt_window_t));

    win->title = strdup(title);
    win->x = x;
    win->y = y;
    win->widgets = NULL;
    win->num_widgets = 0;
    win->selected = 0;

    AddWindow(win);

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

    RemoveWindow(window);
}

static void TXT_WindowSize(txt_window_t *window, int *w, int *h)
{
    int max_width;
    int i;
    int ww;

    max_width = 10;

    for (i=0; i<window->num_widgets; ++i)
    {
        ww = TXT_WidgetWidth(window->widgets[i]);

        if (ww > max_width)
            max_width = ww;
    }

    *w = max_width;
    *h = window->num_widgets;
}

static void DrawWindow(txt_window_t *window)
{
    int widgets_w, widgets_h;
    int window_w, window_h;
    int x, y;
    int window_x, window_y;
    int i;
    
    TXT_WindowSize(window, &widgets_w, &widgets_h);

    // Actual window size after padding

    window_w = widgets_w + 2;
    window_h = widgets_h + 5;

    // Use the x,y position as the centerpoint and find the location to 
    // draw the window.

    window_x = window->x - (window_w / 2);
    window_y = window->y - (window_h / 2);

    // Draw the window

    TXT_DrawWindow(window->title, window_x, window_y, window_w, window_h);

    // Draw all widgets

    for (i=0; i<window->num_widgets; ++i)
    {
        TXT_GotoXY(window_x + 1, window_y + 2 + i);
        TXT_DrawWidget(window->widgets[i], widgets_w, i == window->selected);
    }

    // Separator for action area

    TXT_DrawSeparator(window_x, window_y + 2 + window->num_widgets, window_w);
}

void TXT_DrawAllWindows(void)
{
    int i;

    TXT_DrawDesktop("Not Chocolate Doom setup");

    for (i=0; i<num_windows; ++i)
    {
        DrawWindow(all_windows[i]);
    }

    TXT_UpdateScreen();
}

void TXT_AddWidget(txt_window_t *window, txt_widget_t *widget)
{
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

