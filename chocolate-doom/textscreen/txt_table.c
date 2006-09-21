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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_desktop.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_separator.h"
#include "txt_strut.h"
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
                            unsigned int *row_heights, 
                            unsigned int *col_widths)
{
    int x, y;
    int rows;
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

            // NULL represents an empty spacer

            if (widget != NULL)
            {
                TXT_CalcWidgetSize(widget);
                if (widget->h > row_heights[y])
                    row_heights[y] = widget->h;
                if (widget->w > col_widths[x])
                    col_widths[x] = widget->w;
            }
        }
    }
}

static void TXT_CalcTableSize(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    unsigned int *column_widths;
    unsigned int *row_heights;
    int x, y;
    int rows;

    rows = TableRows(table);

    row_heights = malloc(sizeof(int) * rows);
    column_widths = malloc(sizeof(int) * table->columns);

    CalcRowColSizes(table, row_heights, column_widths);

    table->widget.w = 0;

    for (x=0; x<table->columns; ++x)
    {
        table->widget.w += column_widths[x];
    }

    table->widget.h = 0;

    for (y=0; y<rows; ++y)
    {
        table->widget.h += row_heights[y];
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

    if (x < 0 || x >= table->columns)
    {
        return 0;
    }

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
         && table->widgets[selected]->selectable
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

static void LayoutCell(txt_table_t *table, int x, int y, int col_width,
                       int draw_x, int draw_y)
{
    txt_widget_t *widget;

    widget = table->widgets[y * table->columns + x];

    // Adjust x position based on alignment property

    switch (widget->align)
    {
        case TXT_HORIZ_LEFT:
            widget->w = col_width;
            break;

        case TXT_HORIZ_CENTER:
            TXT_CalcWidgetSize(widget);
            
            // Separators are always drawn left-aligned.

            if (widget->widget_class != &txt_separator_class)
            {
                draw_x += (col_width - widget->w) / 2;
            }
            
            break;

        case TXT_HORIZ_RIGHT:
            TXT_CalcWidgetSize(widget);
            
            if (widget->widget_class != &txt_separator_class)
            {
                draw_x += col_width - widget->w;
            }
            
            break;
    }

    // Set the position for this widget

    widget->x = draw_x;
    widget->y = draw_y;

    // Recursively lay out any widgets contained in the widget

    TXT_LayoutWidget(widget);
}

static void TXT_TableLayout(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    unsigned int *column_widths;
    unsigned int *row_heights;
    int draw_x, draw_y;
    int x, y;
    int i;
    int rows;

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
        column_widths[0] = table->widget.w;
    }

    // Draw all cells
    
    draw_y = table->widget.y;
    
    for (y=0; y<rows; ++y)
    {
        draw_x = table->widget.x;

        for (x=0; x<table->columns; ++x)
        {
            i = y * table->columns + x;

            if (i >= table->num_widgets)
                break;

            if (table->widgets[i] != NULL)
            {
                LayoutCell(table, x, y, column_widths[x], 
                           draw_x, draw_y);
            }

            draw_x += column_widths[x];
        }

        draw_y += row_heights[y];
    }

    free(row_heights);
    free(column_widths);
}
                
static void TXT_TableDrawer(TXT_UNCAST_ARG(table), int selected)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_widget_t *widget;
    int selected_cell;
    int i;
    
    // Check the table's current selection points at something valid before
    // drawing.

    CheckValidSelection(table);

    // Find the index of the currently-selected widget.

    selected_cell = table->selected_y * table->columns + table->selected_x;
    
    // Draw all cells
    
    for (i=0; i<table->num_widgets; ++i)
    {
        widget = table->widgets[i];

        if (widget != NULL)
        {
            TXT_GotoXY(widget->x, widget->y);
            TXT_DrawWidget(widget, selected && i == selected_cell);
        }
    }
}

// Responds to mouse presses

static void TXT_TableMousePress(TXT_UNCAST_ARG(table), int x, int y, int b)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_widget_t *widget;
    int i;

    for (i=0; i<table->num_widgets; ++i)
    {
        widget = table->widgets[i];

        // NULL widgets are spacers

        if (widget != NULL)
        {
            if (x >= widget->x && x < widget->x + widget->w
             && y >= widget->y && y < widget->y + widget->h)
            {
                // This is the widget that was clicked!

                // Select the cell if the widget is selectable

                if (widget->selectable)
                {
                    table->selected_x = i % table->columns;
                    table->selected_y = i / table->columns;
                }

                // Propagate click

                TXT_WidgetMousePress(widget, x, y, b);

                break;
            }
        }
    }
}

txt_widget_class_t txt_table_class =
{
    TXT_CalcTableSize,
    TXT_TableDrawer,
    TXT_TableKeyPress,
    TXT_TableDestructor,
    TXT_TableMousePress,
    TXT_TableLayout,
};

void TXT_InitTable(txt_table_t *table, int columns)
{
    int i;

    TXT_InitWidget(table, &txt_table_class);
    table->columns = columns;
    table->widgets = NULL;
    table->num_widgets = 0;
    table->selected_x = 0;
    table->selected_y = 0;

    // Add a strut for each column at the start of the table. 
    // These are used by the TXT_SetColumnWidths function below:
    // the struts are created with widths of 0 each, but this
    // function changes them.

    for (i=0; i<columns; ++i)
    {
        TXT_AddWidget(table, TXT_NewStrut(0, 0));
    }
}

txt_table_t *TXT_NewTable(int columns)
{
    txt_table_t *table;

    table = malloc(sizeof(txt_table_t));

    TXT_InitTable(table, columns);

    return table;
}

// Selects a given widget in a table, recursively searching any tables
// within this table.  Returns 1 if successful, 0 if unsuccessful.

int TXT_SelectWidget(TXT_UNCAST_ARG(table), TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_table_t, table);
    TXT_CAST_ARG(txt_widget_t, widget);
    int i;

    for (i=0; i<table->num_widgets; ++i)
    {
        if (table->widgets[i] == NULL)
        {
            continue;
        }
        
        if (table->widgets[i] == widget)
        {
            // Found the item!  Select it and return.
            
            table->selected_x = i % table->columns;
            table->selected_y = i / table->columns;

            return 1;
        }
        
        if (table->widgets[i]->widget_class == &txt_table_class)
        {
            // This item is a subtable.  Recursively search this table.

            if (TXT_SelectWidget(table->widgets[i], widget))
            {
                // Found it in the subtable.  Select this subtable and return.

                table->selected_x = i % table->columns;
                table->selected_y = i / table->columns;

                return 1;
            }
        }
    }

    // Not found.

    return 0;
}

// Sets the widths of columns in a table.

void TXT_SetColumnWidths(TXT_UNCAST_ARG(table), ...)
{
    TXT_CAST_ARG(txt_table_t, table);
    va_list args;
    txt_strut_t *strut;
    int i;
    int width;

    va_start(args, TXT_UNCAST_ARG_NAME(table));

    for (i=0; i<table->columns; ++i)
    {
        width = va_arg(args, int);

        strut = (txt_strut_t *) table->widgets[i];
        strut->width = width;
    }
}

