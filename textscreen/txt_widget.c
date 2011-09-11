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

#include <stdlib.h>
#include <string.h>

#include "txt_io.h"
#include "txt_widget.h"
#include "txt_gui.h"
#include "txt_desktop.h"

typedef struct
{
    char *signal_name;
    TxtWidgetSignalFunc func;
    void *user_data;
} txt_callback_t;

struct txt_callback_table_s
{
    int refcount;
    txt_callback_t *callbacks;
    int num_callbacks;
};

txt_callback_table_t *TXT_NewCallbackTable(void)
{
    txt_callback_table_t *table;

    table = malloc(sizeof(txt_callback_table_t));
    table->callbacks = NULL;
    table->num_callbacks = 0;
    table->refcount = 1;

    return table;
}

void TXT_RefCallbackTable(txt_callback_table_t *table)
{
    ++table->refcount;
}

void TXT_UnrefCallbackTable(txt_callback_table_t *table)
{
    int i;

    --table->refcount;

    if (table->refcount == 0)
    {
        // No more references to this table

        for (i=0; i<table->num_callbacks; ++i)
        {
            free(table->callbacks[i].signal_name);
        }
    
        free(table->callbacks);
        free(table);
    }
}

void TXT_InitWidget(TXT_UNCAST_ARG(widget), txt_widget_class_t *widget_class)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->widget_class = widget_class;
    widget->callback_table = TXT_NewCallbackTable();
    widget->parent = NULL;

    // Visible by default.

    widget->visible = 1;

    // Align left by default

    widget->align = TXT_HORIZ_LEFT;
}

void TXT_SignalConnect(TXT_UNCAST_ARG(widget),
                       const char *signal_name,
                       TxtWidgetSignalFunc func, 
                       void *user_data)
{
    TXT_CAST_ARG(txt_widget_t, widget);
    txt_callback_table_t *table;
    txt_callback_t *callback;

    table = widget->callback_table;

    // Add a new callback to the table

    table->callbacks 
            = realloc(table->callbacks,
                      sizeof(txt_callback_t) * (table->num_callbacks + 1));
    callback = &table->callbacks[table->num_callbacks];
    ++table->num_callbacks;

    callback->signal_name = strdup(signal_name);
    callback->func = func;
    callback->user_data = user_data;
}

void TXT_EmitSignal(TXT_UNCAST_ARG(widget), const char *signal_name)
{
    TXT_CAST_ARG(txt_widget_t, widget);
    txt_callback_table_t *table;
    int i;

    table = widget->callback_table;

    // Don't destroy the table while we're searching through it
    // (one of the callbacks may destroy this window)

    TXT_RefCallbackTable(table);

    // Search the table for all callbacks with this name and invoke
    // the functions.

    for (i=0; i<table->num_callbacks; ++i)
    {
        if (!strcmp(table->callbacks[i].signal_name, signal_name))
        {
            table->callbacks[i].func(widget, table->callbacks[i].user_data);
        }
    }

    // Finished using the table

    TXT_UnrefCallbackTable(table);
}

void TXT_CalcWidgetSize(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->widget_class->size_calc(widget);
}

void TXT_DrawWidget(TXT_UNCAST_ARG(widget), int selected)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    // For convenience...

    TXT_GotoXY(widget->x, widget->y);

    // Call drawer method
 
    widget->widget_class->drawer(widget, selected);
}

void TXT_DestroyWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->widget_class->destructor(widget);
    TXT_UnrefCallbackTable(widget->callback_table);
    free(widget);
}

int TXT_WidgetKeyPress(TXT_UNCAST_ARG(widget), int key)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->key_press != NULL)
    {
        return widget->widget_class->key_press(widget, key);
    }

    return 0;
}

void TXT_SetWidgetAlign(TXT_UNCAST_ARG(widget), txt_horiz_align_t horiz_align)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    widget->align = horiz_align;
}

void TXT_WidgetMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->mouse_press != NULL)
    {
        widget->widget_class->mouse_press(widget, x, y, b);
    }
}

void TXT_LayoutWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->layout != NULL)
    {
        widget->widget_class->layout(widget);
    }
}

int TXT_AlwaysSelectable(TXT_UNCAST_ARG(widget))
{
    return 1;
}

int TXT_NeverSelectable(TXT_UNCAST_ARG(widget))
{
    return 0;
}

int TXT_SelectableWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (widget->widget_class->selectable != NULL)
    {
        return widget->widget_class->selectable(widget);
    }
    else
    {
        return 0;
    }
}

int TXT_ContainsWidget(TXT_UNCAST_ARG(haystack), TXT_UNCAST_ARG(needle))
{
    TXT_CAST_ARG(txt_widget_t, haystack);
    TXT_CAST_ARG(txt_widget_t, needle);

    while (needle != NULL)
    {
        if (needle == haystack)
        {
            return 1;
        }

        needle = needle->parent;
    }

    return 0;
}

int TXT_HoveringOverWidget(TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_widget_t, widget);
    txt_window_t *active_window;
    int x, y;

    // We can only be hovering over widgets in the active window.

    active_window = TXT_GetActiveWindow();

    if (active_window == NULL || !TXT_ContainsWidget(active_window, widget))
    {
        return 0;
    }

    // Is the mouse cursor within the bounds of the widget?

    TXT_GetMousePosition(&x, &y);

    return (x >= widget->x && x < widget->x + widget->w
         && y >= widget->y && y < widget->y + widget->h);
}

void TXT_SetWidgetBG(TXT_UNCAST_ARG(widget), int selected)
{
    TXT_CAST_ARG(txt_widget_t, widget);

    if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }
    else if (TXT_HoveringOverWidget(widget))
    {
        TXT_BGColor(TXT_HOVER_BACKGROUND, 0);
    }
    else
    {
        TXT_BGColor(TXT_WINDOW_BACKGROUND, 0);
    }
}

