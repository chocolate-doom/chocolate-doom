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

static void TXT_TableDestructor(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    int i;

    // Free all widgets

    for (i=0; i<table->num_widgets; ++i)
    {
        if (table->widgets[i] != NULL)
        {
            TXT_DestroyWidget(table->widgets[i]);
        }
    }
    
    // Free table resources

    free(table->widgets);
}

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
    txt_widget_t *widget;

    rows = TableRows(table);

    memset(col_widths, 0, sizeof(int) * table->columns);

    for (y=0; y<rows; ++y)
    {
        row_heights[y] = 0;

        for (x=0; x<table->columns; ++x)
        {
            if (y * table->columns + x >= table->num_widgets)
                break;

            widget = table->widgets[y * table->columns + x];

            if (widget != NULL)
            {
                TXT_CalcWidgetSize(widget, &ww, &wh);
            }
            else
            {
                // Empty spacer if widget is NULL

                ww = 0;
                wh = 0;
            }

            if (wh > row_heights[y])
                row_heights[y] = wh;
            if (ww > col_widths[x])
                col_widths[x] = ww;
        }
    }
}

static void TXT_CalcTableSize(TXT_UNCAST_ARG(table), int *w, int *h)
{
    TXT_CAST_ARG(txt_table_t, table);
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

void TXT_AddWidget(TXT_UNCAST_ARG(table), TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_table_t, table);
    TXT_CAST_ARG(txt_widget_t, widget);

    if (table->num_widgets > 0)
    {
        txt_widget_t *last_widget;

        last_widget = table->widgets[table->num_widgets - 1];

        if (widget != NULL && last_widget != NULL
         && widget->widget_class == &txt_separator_class
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

static int SelectableWidget(txt_table_t *table, int x, int y)
{
    txt_widget_t *widget;
    int i;

    i = y * table->columns + x;

    if (i >= 0 && i < table->num_widgets)
    {
        widget = table->widgets[i];
        return widget != NULL && widget->selectable && widget->visible;
    }

    return 0;
}

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

        if (SelectableWidget(table, start_col + x, row))
        {
            return start_col + x;
        }

        // Search to the left

        if (SelectableWidget(table, start_col - x, row))
        {
            return start_col - x;
        }
    }

    // None available

    return -1;
}

static int TXT_TableKeyPress(TXT_UNCAST_ARG(table), int key)
{
    TXT_CAST_ARG(txt_table_t, table);
    int selected;
    int rows;

    rows = TableRows(table);

    // Send to the currently selected widget first

    selected = table->selected_y * table->columns + table->selected_x;

    if (selected >= 0 && selected < table->num_widgets)
    {
        if (table->widgets[selected] != NULL
         && TXT_WidgetKeyPress(table->widgets[selected], key))
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
            if (SelectableWidget(table, new_x, table->selected_y))
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
            if (SelectableWidget(table, new_x, table->selected_y))
            {
                // Found a selectable widget!

                table->selected_x = new_x;

                return 1;
            }
        }
    }

    return 0;
}

// Check the currently selected widget in the table is valid.

static void CheckValidSelection(txt_table_t *table)
{
    int rows;
    int new_x, new_y;

    rows = TableRows(table);

    for (new_y = table->selected_y; new_y < rows; ++new_y)
    {
        new_x = FindSelectableColumn(table, new_y, table->selected_x);

        if (new_x >= 0)
        {
            // Found a selectable column.

            table->selected_x = new_x;
            table->selected_y = new_y;

            break;
        }
    }
}

static void DrawCell(txt_table_t *table, int x, int y,
                     int draw_x, int draw_y, int w, int selected)
{
    txt_widget_t *widget;
    int cw, ch;

    widget = table->widgets[y * table->columns + x];

    switch (widget->align)
    {
        case TXT_HORIZ_LEFT:
            break;

        case TXT_HORIZ_CENTER:
            TXT_CalcWidgetSize(widget, &cw, &ch);
            
            // Separators are always drawn left-aligned.

            if (widget->widget_class != &txt_separator_class)
            {
                draw_x += (w - cw) / 2;
                w = cw;
            }
            
            break;

        case TXT_HORIZ_RIGHT:
            TXT_CalcWidgetSize(widget, &cw, &ch);
            
            if (widget->widget_class != &txt_separator_class)
            {
                draw_x += w - cw;
                w = cw;
            }
            
            break;
    }

    TXT_GotoXY(draw_x, draw_y);

    TXT_DrawWidget(widget, w, selected && x == table->selected_x
                                       && y == table->selected_y);
}

static void TXT_TableDrawer(TXT_UNCAST_ARG(table), int w, int selected)
{
    TXT_CAST_ARG(txt_table_t, table);
    int *column_widths;
    int *row_heights;
    int origin_x, origin_y;
    int draw_x, draw_y;
    int x, y;
    int i;
    int rows;

    // Check the table's current selection points at something valid before
    // drawing.

    CheckValidSelection(table);

    TXT_GetXY(&origin_x, &origin_y);

    // Work out the column widths and row heights

    rows = TableRows(table);

    column_widths = malloc(sizeof(int) * table->columns);
    row_heights = malloc(sizeof(int) * rows);

    CalcRowColSizes(table, row_heights, column_widths);

    // If this table only has one column, expand column size to fit
    // the display width.  Ensures that separators reach the window edges 
    // when drawing windows.

    if (table->columns == 1)
    {
        column_widths[0] = w;
    }

    // Draw all cells
    
    draw_y = origin_y;
    
    for (y=0; y<rows; ++y)
    {
        draw_x = origin_x;

        for (x=0; x<table->columns; ++x)
        {
            i = y * table->columns + x;

            if (i >= table->num_widgets)
                break;

            if (table->widgets[i] != NULL)
            {
                DrawCell(table, x, y, draw_x, draw_y, 
                         column_widths[x], selected);
            }

            draw_x += column_widths[x];
        }

        draw_y += row_heights[y];
    }

    free(row_heights);
    free(column_widths);
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
    TXT_InitWidget(table, &txt_table_class);
    table->columns = columns;
    table->widgets = NULL;
    table->num_widgets = 0;
    table->selected_x = 0;
    table->selected_y = 0;
}

txt_table_t *TXT_NewTable(int columns)
{
    txt_table_t *table;

    table = malloc(sizeof(txt_table_t));

    TXT_InitTable(table, columns);

    return table;
}

