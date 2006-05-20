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
#include "txt_table.h"

static void TXT_TableDestructor(txt_widget_t *widget)
{
    txt_table_t *table = (txt_table_t *) widget;
    int i;

    // Free all widgets

    for (i=0; i<table->num_widgets; ++i)
    {
        TXT_DestroyWidget(table->widgets[i]);
    }
    
    // Free table resources

    free(table->widgets);
}

// -------------

static int TableRows(txt_table_t *table)
{
    return (table->num_widgets + table->columns - 1) / table->columns;
}

static void CalcRowColSizes(txt_table_t *table, 
                            int *row_heights, 
                            int *col_widths)
{
    int table_height;
    int x, y;
    int rows;
    int ww, wh;

    rows = TableRows(table);

    memset(col_widths, 0, sizeof(int) * table->columns);

    for (y=0; y<rows; ++y)
    {
        row_heights[y] = 0;

        for (x=0; x<table->columns; ++x)
        {
            if (y * table->columns + x >= table->num_widgets)
                break;

            TXT_CalcWidgetSize(table->widgets[y * table->columns + x],
                               &ww, &wh);

            if (wh > row_heights[y])
                row_heights[y] = wh;
            if (ww > col_widths[x])
                col_widths[x] = ww;
        }
    }
}

static void TXT_CalcTableSize(txt_widget_t *widget, int *w, int *h)
{
    txt_table_t *table = (txt_table_t *) widget;
    int *column_widths;
    int *row_heights;
    int x, y;
    int rows;

    rows = TableRows(table);

    row_heights = malloc(sizeof(int) * rows);
    column_widths = malloc(sizeof(int) * table->columns);

    CalcRowColSizes(table, row_heights, column_widths);

    *w = 0;

    for (x=0; x<table->columns; ++x)
    {
        *w += column_widths[x];
    }

    *h = 0;

    for (y=0; y<rows; ++y)
    {
        *h += row_heights[y];
    }

    free(row_heights);
    free(column_widths);
}

static void TXT_TableDrawer(txt_widget_t *widget, int w, int selected)
{
    txt_table_t *table = (txt_table_t *) widget;
    int *column_widths;
    int *row_heights;
    int origin_x, origin_y;
    int draw_x, draw_y;
    int x, y;
    int rows;

    TXT_GetXY(&origin_x, &origin_y);

    // Work out the column widths and row heights

    rows = TableRows(table);

    column_widths = malloc(sizeof(int) * table->columns);
    row_heights = malloc(sizeof(int) * rows);

    CalcRowColSizes(table, row_heights, column_widths);

    // Draw all cells
    
    draw_y = origin_y;
    
    for (y=0; y<rows; ++y)
    {
        draw_x = origin_x;

        for (x=0; x<table->columns; ++x)
        {
            if (y * table->columns + x >= table->num_widgets)
                break;

            TXT_GotoXY(draw_x, draw_y);

            TXT_DrawWidget(table->widgets[y * table->columns + x],
                           column_widths[x],
                           selected && x == table->selected_x
                                    && y == table->selected_y);

            draw_x += column_widths[x];
        }

        draw_y += row_heights[y];
    }

    free(row_heights);
    free(column_widths);
}

void TXT_AddTableWidget(void *uncast_table, void *uncast_widget)
{
    txt_widget_t *widget;
    txt_table_t *table;

    table = (txt_table_t *) uncast_table;
    widget = (txt_widget_t *) uncast_widget;

    if (table->num_widgets > 0)
    {
        txt_widget_t *last_widget;

        last_widget = table->widgets[table->num_widgets - 1];

        if (widget->widget_class == &txt_separator_class
         && last_widget->widget_class == &txt_separator_class)
        {
            // The previous widget added was a separator; replace 
            // it with this one.
            //
            // This way, if the first widget added to a window is 
            // a separator, it replaces the "default" separator that
            // the window itself adds on creation.

            table->widgets[table->num_widgets - 1] = widget;

            TXT_DestroyWidget(last_widget);

            return;
        }
    }

    table->widgets = realloc(table->widgets,
                             sizeof(txt_widget_t *) * (table->num_widgets + 1));
    table->widgets[table->num_widgets] = widget;
    ++table->num_widgets;
}

#define SELECTABLE_WIDGET(w) ((w)->selectable && (w)->visible)

// Tries to locate a selectable widget in the given row, returning
// the column number of the first column available or -1 if none are 
// available in the given row.
//
// Starts from start_col, then searches nearby columns.

static int FindSelectableColumn(txt_table_t *table, int row, int start_col)
{
    int x;
    int i;

    for (x=0; x<table->columns; ++x)
    {
        // Search to the right

        i = table->columns * row + start_col + x;

        if (i >= 0 && i < table->num_widgets
         && SELECTABLE_WIDGET(table->widgets[i]))
        {
            return start_col + x;
        }

        // Search to the left

        i = table->columns * row + start_col - x;

        if (i >= 0 && i < table->num_widgets
         && SELECTABLE_WIDGET(table->widgets[i]))
        {
            return start_col - x;
        }
    }

    // None available

    return -1;
}

static int TXT_TableKeyPress(txt_widget_t *widget, int key)
{
    txt_table_t *table = (txt_table_t *) widget;
    int selected;
    int rows;

    rows = TableRows(table);

    // Send to the currently selected widget first

    selected = table->selected_y * table->columns + table->selected_x;

    if (selected >= 0 && selected < table->num_widgets)
    {
        if (TXT_WidgetKeyPress(table->widgets[selected], key))
        {
            return 1;
        }
    }

    if (key == KEY_DOWNARROW)
    {
        int new_x, new_y;

        // Move cursor down to the next selectable widget

        for (new_y = table->selected_y + 1; new_y < rows; ++new_y)
        {
            new_x = FindSelectableColumn(table, new_y, table->selected_x);
                            
            if (new_x >= 0)
            {
                // Found a selectable widget in this column!

                table->selected_x = new_x;
                table->selected_y = new_y;

                return 1;
            }
        } 
    }

    if (key == KEY_UPARROW)
    {
        int new_x, new_y;

        // Move cursor up to the next selectable widget

        for (new_y = table->selected_y - 1; new_y >= 0; --new_y)
        {
            new_x = FindSelectableColumn(table, new_y, table->selected_x);
                            
            if (new_x >= 0)
            {
                // Found a selectable widget in this column!

                table->selected_x = new_x;
                table->selected_y = new_y;

                return 1;
            }
        } 
    }

    if (key == KEY_LEFTARROW)
    {
        int new_x;
        int i;

        // Move cursor left

        for (new_x = table->selected_x - 1; new_x >= 0; --new_x)
        {
            i = table->selected_y * table->columns + new_x;
            
            if (i >= 0 && i < table->num_widgets
             && SELECTABLE_WIDGET(table->widgets[i]))
            {
                // Found a selectable widget!

                table->selected_x = new_x;

                return 1;
            }
        }
    }

    if (key == KEY_RIGHTARROW)
    {
        int new_x;
        int i;

        // Move cursor left

        for (new_x = table->selected_x + 1; new_x < table->columns; ++new_x)
        {
            i = table->selected_y * table->columns + new_x;
            
            if (i >= 0 && i < table->num_widgets
             && SELECTABLE_WIDGET(table->widgets[i]))
            {
                // Found a selectable widget!

                table->selected_x = new_x;

                return 1;
            }
        }
    }

    return 0;
}

txt_widget_class_t txt_table_class =
{
    TXT_CalcTableSize,
    TXT_TableDrawer,
    TXT_TableKeyPress,
    TXT_TableDestructor,
};

void TXT_InitTable(txt_table_t *table, int columns)
{
    table->columns = columns;
    table->widgets = NULL;
    table->num_widgets = 0;
    table->widget.widget_class = &txt_table_class;
    table->widget.visible = 1;
    table->widget.selectable = 1;
}

txt_table_t *TXT_NewTable(int columns)
{
    txt_table_t *table;

    table = malloc(sizeof(txt_table_t));

    TXT_InitTable(table, columns);

    return table;
}

