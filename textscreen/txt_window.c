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

void TXT_SetWindowAction(txt_window_t *window,
                         txt_horiz_align_t position, 
                         txt_window_action_t *action)
{
    if (window->actions[position] != NULL)
    {
        TXT_DestroyWidget(window->actions[position]);
    }

    window->actions[position] = action;
}

txt_window_t *TXT_NewWindow(char *title)
{
    int i;
    txt_window_action_t *cancel_action;
    txt_window_action_t *accept_action;

    txt_window_t *win;

    win = malloc(sizeof(txt_window_t));

    TXT_InitTable(&win->table, 1);

    win->title = strdup(title);
    win->x = TXT_SCREEN_W / 2;
    win->y = TXT_SCREEN_H / 2;
    win->horiz_align = TXT_HORIZ_CENTER;
    win->vert_align = TXT_VERT_CENTER;

    TXT_AddWidget(win, TXT_NewSeparator(NULL));

    for (i=0; i<3; ++i)
        win->actions[i] = NULL;

    TXT_AddDesktopWindow(win);

    // Default actions

    cancel_action = TXT_NewWindowAction(KEY_ESCAPE, "Abort");
    TXT_SetWindowAction(win, TXT_HORIZ_LEFT, cancel_action);
    accept_action = TXT_NewWindowAction(KEY_ENTER, "Accept");
    TXT_SetWindowAction(win, TXT_HORIZ_RIGHT, accept_action);

    return win;
}

void TXT_CloseWindow(txt_window_t *window)
{
    int i;

    free(window->title);

    // Destroy all actions

    for (i=0; i<3; ++i)
    {
        if (window->actions[i] != NULL)
        {
            TXT_DestroyWidget(window->actions[i]);
        }
    }

    // Destroy table and window

    TXT_DestroyWidget(window);
    
    TXT_RemoveDesktopWindow(window);
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

static void DrawActionArea(txt_window_t *window,
                           int window_x, int window_y,
                           int window_w, int window_h)
{
    int ww, wh;

    // Draw the left action

    if (window->actions[TXT_HORIZ_LEFT] != NULL)
    {
        TXT_GotoXY(window_x + 2, window_y + window_h - 2);
        TXT_DrawWidget(window->actions[TXT_HORIZ_LEFT], 0, 0);
    }

    // Draw the center action

    if (window->actions[TXT_HORIZ_CENTER] != NULL)
    {
        TXT_CalcWidgetSize(window->actions[TXT_HORIZ_CENTER], &ww, &wh);
        TXT_GotoXY(window_x + (window_w - ww - 2) / 2, window_y + window_h - 2);
        TXT_DrawWidget(window->actions[TXT_HORIZ_CENTER], 0, 0);
    }

    // Draw the right action

    if (window->actions[TXT_HORIZ_RIGHT] != NULL)
    {
        TXT_CalcWidgetSize(window->actions[TXT_HORIZ_RIGHT], &ww, &wh);
        TXT_GotoXY(window_x + window_w - 2 - ww, window_y + window_h - 2);
        TXT_DrawWidget(window->actions[TXT_HORIZ_RIGHT], 0, 0);
    }
}

static int CalcActionAreaWidth(txt_window_t *window, int *widgets_w)
{
    int ww, wh;
    int w;
    int i;

    w = 1;

    // Calculate the width of all the action widgets and use this
    // to create an overall min. width of the action area

    for (i=0; i<3; ++i)
    {
        if (window->actions[i] != NULL)
        {
            TXT_CalcWidgetSize(window->actions[i], &ww, &wh);
            w += ww + 1;
        }
    }

    // If this is larger than the window size, adjust the window to fit.

    if (w > *widgets_w)
        *widgets_w = w;
}

void TXT_DrawWindow(txt_window_t *window)
{
    int widgets_w, widgets_h;
    int window_w, window_h;
    int x, y;
    int window_x, window_y;
    int i;
    int ww, wh;
    
    TXT_CalcWidgetSize(window, &widgets_w, &widgets_h);
    CalcActionAreaWidth(window, &widgets_w);

    // Actual window size after padding

    window_w = widgets_w + 2;
    window_h = widgets_h + 5;

    // Use the x,y position as the centerpoint and find the location to 
    // draw the window.

    CalcWindowPosition(window, &window_x, &window_y, window_w, window_h);

    // Draw the window

    TXT_DrawWindowFrame(window->title, window_x, window_y, window_w, window_h);

    // Draw all widgets

    TXT_GotoXY(window_x + 1, window_y + 2);
    TXT_DrawWidget(window, widgets_w, 1);

    // Separator for action area

    TXT_DrawSeparator(window_x, window_y + 2 + widgets_h, window_w);

    // Action area at the window bottom

    DrawActionArea(window, window_x, window_y, window_w, window_h);
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
    int i;
    
    // Send to the currently selected widget first

    if (TXT_WidgetKeyPress(window, c))
    {
        return;
    }

    // Try all of the action buttons

    for (i=0; i<3; ++i)
    {
        if (window->actions[i] != NULL
         && TXT_WidgetKeyPress(window->actions[i], c))
        {
            return;
        }
    }
}

