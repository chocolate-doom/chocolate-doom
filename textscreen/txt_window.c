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

static void DefaultCancelAction(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_CloseWindow(window);
}

txt_window_t *TXT_NewWindow(char *title)
{
    int i;
    txt_window_action_t *cancel_action;
    txt_window_action_t *accept_action;

    txt_window_t *win;

    win = malloc(sizeof(txt_window_t));

    TXT_InitTable(&win->table, 1);

    if (title == NULL)
    {
        win->title = NULL;
    }
    else
    {
        win->title = strdup(title);
    }

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
    TXT_SignalConnect(cancel_action, "pressed", DefaultCancelAction, win);
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

static void CalcWindowPosition(txt_window_t *window)
{
    switch (window->horiz_align)
    {
        case TXT_HORIZ_LEFT:
            window->window_x = window->x;
            break;
        case TXT_HORIZ_CENTER:
            window->window_x = window->x - (window->window_w / 2);
            break;
        case TXT_HORIZ_RIGHT:
            window->window_x = window->x - (window->window_w - 1);
            break;
    }

    switch (window->vert_align)
    {
        case TXT_VERT_TOP:
            window->window_y = window->y;
            break;
        case TXT_VERT_CENTER:
            window->window_y = window->y - (window->window_h / 2);
            break;
        case TXT_VERT_BOTTOM:
            window->window_y = window->y - (window->window_h - 1);
            break;
    }
}

static void LayoutActionArea(txt_window_t *window)
{
    txt_widget_t *widget;

    // Left action

    if (window->actions[TXT_HORIZ_LEFT] != NULL)
    {
        widget = (txt_widget_t *) window->actions[TXT_HORIZ_LEFT];

        TXT_CalcWidgetSize(widget);

        widget->x = window->window_x + 2;
        widget->y = window->window_y + window->window_h - 2;
    }

    // Draw the center action

    if (window->actions[TXT_HORIZ_CENTER] != NULL)
    {
        widget = (txt_widget_t *) window->actions[TXT_HORIZ_CENTER];

        TXT_CalcWidgetSize(widget);

        widget->x = window->window_x + (window->window_w - widget->w - 2) / 2;
        widget->y = window->window_y + window->window_h - 2;
    }

    // Draw the right action

    if (window->actions[TXT_HORIZ_RIGHT] != NULL)
    {
        widget = (txt_widget_t *) window->actions[TXT_HORIZ_RIGHT];

        TXT_CalcWidgetSize(widget);

        widget->x = window->window_x + window->window_w - 2 - widget->w;
        widget->y = window->window_y + window->window_h - 2;
    }
}

static void DrawActionArea(txt_window_t *window)
{
    int i;

    for (i=0; i<3; ++i)
    {
        if (window->actions[i] != NULL)
        {
            TXT_DrawWidget(window->actions[i], 0);
        }
    }
}

static int ActionAreaWidth(txt_window_t *window)
{
    txt_widget_t *widget;
    int w;
    int i;

    w = 1;

    // Calculate the width of all the action widgets and use this
    // to create an overall min. width of the action area

    for (i=0; i<3; ++i)
    {
        widget = (txt_widget_t *) window->actions[i];

        if (widget != NULL)
        {
            TXT_CalcWidgetSize(widget);
            w += widget->w + 1;
        }
    }

    return w;
}

// Sets size and position of all widgets in a window

void TXT_LayoutWindow(txt_window_t *window)
{
    txt_widget_t *widgets = (txt_widget_t *) window;
    int widgets_w;
    int actionarea_w;

    // Calculate size of table
    
    TXT_CalcWidgetSize(window);

    // Calculate the size of the action area
  
    actionarea_w = ActionAreaWidth(window);
    
    // Which one is larger?

    widgets_w = widgets->w;

    if (actionarea_w > widgets_w)
        widgets_w = actionarea_w;

    // Set the window size based on widgets_w
   
    window->window_w = widgets_w + 2;
    window->window_h = widgets->h + 3;

    // If the window has a title, add an extra two lines

    if (window->title != NULL)
    {
        window->window_h += 2;
    }

    // Use the x,y position as the centerpoint and find the location to 
    // draw the window.

    CalcWindowPosition(window);

    // Set the table size and position

    widgets->w = widgets_w;
    // widgets->h        (already set)
    widgets->x = window->window_x + 1;
    widgets->y = window->window_y + window->window_h - widgets->h - 3;

    // Layout the table and action area

    LayoutActionArea(window);
    TXT_LayoutWidget(widgets);
}

void TXT_DrawWindow(txt_window_t *window)
{
    txt_widget_t *widgets;
    int x, y;
    int i;
    int ww, wh;

    TXT_LayoutWindow(window);
    
    // Draw the window

    TXT_DrawWindowFrame(window->title, 
                        window->window_x, window->window_y,
                        window->window_w, window->window_h);

    // Draw all widgets

    TXT_DrawWidget(window, 1);

    // Separator for action area

    widgets = (txt_widget_t *) window;

    TXT_DrawSeparator(window->window_x, widgets->y + widgets->h, widgets->w);

    // Action area at the window bottom

    DrawActionArea(window);
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

static void MouseButtonPress(txt_window_t *window, int b)
{
    int x, y;
    txt_widget_t *widgets;

    // Lay out the window, set positions and sizes of all widgets

    TXT_LayoutWindow(window);
    
    // Get the current mouse position

    TXT_GetMousePosition(&x, &y);

    // Is it within the table range?

    widgets = (txt_widget_t *) window;

    if (x >= widgets->x && x < widgets->x + widgets->w
     && y >= widgets->y && y < widgets->y + widgets->h)
    {
        TXT_WidgetMousePress(window, x, y, b);
    }
}

void TXT_WindowKeyPress(txt_window_t *window, int c)
{
    int i;

    // Is this a mouse button ?
    
    if (c == TXT_MOUSE_LEFT)
    {
        MouseButtonPress(window, c);
        return;
    }
    
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

