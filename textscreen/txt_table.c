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

txt_widget_t txt_table_overflow_right;
txt_widget_t txt_table_overflow_down;
txt_widget_t txt_table_eol;
txt_widget_t txt_table_empty;

// Returns true if the given widget in the table's widgets[] array refers
// to an actual widget - not NULL, or one of the special overflow pointers.
static int IsActualWidget(txt_widget_t *widget)
{
    return widget != NULL
        && widget != &txt_table_overflow_right
        && widget != &txt_table_overflow_down;
}

// Remove all entries from a table

void TXT_ClearTable(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    int i;

    // Free all widgets
    // Skip over the first (num_columns) widgets in the array, as these
    // are the column struts used to control column width

    for (i=table->columns; i<table->num_widgets; ++i)
    {
        if (IsActualWidget(table->widgets[i]))
        {
            TXT_DestroyWidget(table->widgets[i]);
        }
    }

    // Shrink the table to just the column strut widgets

    table->num_widgets = table->columns;
}

static void TXT_TableDestructor(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);

    TXT_ClearTable(table);
}

static int TableRows(txt_table_t *table)
{
    return (table->num_widgets + table->columns - 1) / table->columns;
}

// Most widgets occupy just one cell of a table, but if the special
// overflow constants are used, they can occupy multiple cells.
// This function figures out for a widget in a given cell, which
// cells it should actually occupy (always a rectangle).
static void CellOverflowedSize(txt_table_t *table, int x, int y,
                               int *w, int *h)
{
    txt_widget_t *widget;
    int x1, y1;

    if (!IsActualWidget(table->widgets[y * table->columns + x]))
    {
        *w = 0; *h = 0;
        return;
    }

    *w = table->columns - x;
    *h = 0;
    for (y1 = y; y1 < TableRows(table); ++y1)
    {
        // Every overflow cell must point to either (x, y) or another
        // overflow cell. This means the first in every row must be
        // txt_table_overflow_down.

        if (y1 * table->columns + x >= table->num_widgets)
        {
            break;
        }

        widget = table->widgets[y1 * table->columns + x];

        if (y1 != y && widget != &txt_table_overflow_down)
        {
            break;
        }

        for (x1 = x + 1; x1 < x + *w; ++x1)
        {
            if (y1 * table->columns + x1 >= table->num_widgets)
            {
                break;
            }

            // Can be either type of overflow, except on the first row.
            // Otherwise we impose a limit on the width.
            widget = table->widgets[y1 * table->columns + x1];
            if (widget != &txt_table_overflow_right
             && (widget != &txt_table_overflow_down || y1 == y))
            {
                *w = x1 - x;
                break;
            }
        }

        ++*h;
    }
}

static int IsOverflowingCell(txt_table_t *table, int x, int y)
{
    int w, h;
    CellOverflowedSize(table, x, y, &w, &h);
    return w > 1 || h > 1;
}

// Using the given column/row size tables, calculate the size of the given
// widget, storing the result in (w, h).
static void CalculateWidgetDimensions(txt_table_t *table,
                                      int x, int y,
                                      unsigned int *column_widths,
                                      unsigned int *row_heights,
                                      unsigned int *w, unsigned int *h)
{
    int cell_w, cell_h;
    int x1, y1;

    // Find which cells this widget occupies.
    CellOverflowedSize(table, x, y, &cell_w, &cell_h);

    // Add up column / row widths / heights to get the actual dimensions.
    *w = 0;
    for (x1 = x; x1 < x + cell_w; ++x1)
    {
        *w += column_widths[x1];
    }

    *h = 0;
    for (y1 = y; y1 < y + cell_h; ++y1)
    {
        *h += row_heights[y1];
    }
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

    for (y = 0; y < rows; ++y)
    {
        row_heights[y] = 0;

        for (x = 0; x < table->columns; ++x)
        {
            if (y * table->columns + x >= table->num_widgets)
                break;

            widget = table->widgets[y * table->columns + x];

            if (IsActualWidget(widget))
            {
                TXT_CalcWidgetSize(widget);
            }

            // In the first pass we ignore overflowing cells.
            if (IsOverflowingCell(table, x, y))
            {
                continue;
            }

            // NULL represents an empty spacer
            if (IsActualWidget(widget))
            {
                if (widget->h > row_heights[y])
                    row_heights[y] = widget->h;
                if (widget->w > col_widths[x])
                    col_widths[x] = widget->w;
            }
        }
    }

    // In the second pass, we go through again and process overflowing
    // widgets, to ensure that they will fit.
    for (y = 0; y < rows; ++y)
    {
        for (x = 0; x < table->columns; ++x)
        {
            unsigned int w, h;

            if (y * table->columns + x >= table->num_widgets)
                break;

            widget = table->widgets[y * table->columns + x];
            if (!IsActualWidget(widget))
            {
                continue;
            }

            // Expand column width and row heights as needed.
            CalculateWidgetDimensions(table, x, y, col_widths, row_heights,
                                      &w, &h);
            if (w < widget->w)
            {
                col_widths[x] += widget->w - w;
            }
            if (h < widget->h)
            {
                row_heights[y] += widget->h - h;
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

static void FillRowToEnd(txt_table_t *table)
{
    while ((table->num_widgets % table->columns) != 0)
    {
        TXT_AddWidget(table, &txt_table_overflow_right);
    }
}

void TXT_AddWidget(TXT_UNCAST_ARG(table), TXT_UNCAST_ARG(widget))
{
    TXT_CAST_ARG(txt_table_t, table);
    TXT_CAST_ARG(txt_widget_t, widget);
    int is_separator;
    int i;

    // Convenience alias for NULL:
    if (widget == &txt_table_empty)
    {
        widget = NULL;
    }
    else if (widget == &txt_table_eol)
    {
        FillRowToEnd(table);
        return;
    }

    // We have special handling for the separator widget:
    is_separator = IsActualWidget(widget)
                && widget->widget_class == &txt_separator_class;

    // If we add two separators consecutively, the new separator replaces the
    // first. This allows us to override the "implicit" separator that is
    // added at the top of a window when it is created.
    if (is_separator)
    {
        for (i = table->num_widgets - 1; i >= 0; --i)
        {
            txt_widget_t *last_widget;
            last_widget = table->widgets[i];

            if (IsActualWidget(last_widget)
             && widget->widget_class == &txt_separator_class
             && last_widget->widget_class == &txt_separator_class)
            {
                table->widgets[i] = widget;
                TXT_DestroyWidget(last_widget);
                return;
            }
            else if (last_widget != &txt_table_overflow_right)
            {
                break;
            }
        }
    }

    // Separators begin on a new line.
    if (is_separator)
    {
        FillRowToEnd(table);
    }

    table->widgets = realloc(table->widgets,
                             sizeof(txt_widget_t *) * (table->num_widgets + 1));
    table->widgets[table->num_widgets] = widget;
    ++table->num_widgets;

    // Maintain parent pointer.
    if (IsActualWidget(widget))
    {
        widget->parent = &table->widget;
    }

    // Separators always take up the entire line.
    if (is_separator)
    {
        FillRowToEnd(table);
    }
}

// Add multiple widgets to a table.

void TXT_AddWidgets(TXT_UNCAST_ARG(table), ...)
{
    TXT_CAST_ARG(txt_table_t, table);
    va_list args;
    txt_widget_t *widget;

    va_start(args, TXT_UNCAST_ARG_NAME(table));

    // Keep adding widgets until a NULL is reached.
   
    for (;;) 
    {
        widget = va_arg(args, txt_widget_t *);

        if (widget == NULL)
        {
            break;
        }

        TXT_AddWidget(table, widget);
    }

    va_end(args);
}

static int SelectableCell(txt_table_t *table, int x, int y)
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
        return IsActualWidget(widget)
            && TXT_SelectableWidget(widget)
            && widget->visible;
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

        if (SelectableCell(table, start_col + x, row))
        {
            return start_col + x;
        }

        // Search to the left

        if (SelectableCell(table, start_col - x, row))
        {
            return start_col - x;
        }
    }

    // None available

    return -1;
}

// Change the selected widget.

static void ChangeSelection(txt_table_t *table, int x, int y)
{
    txt_widget_t *cur_widget;
    txt_widget_t *new_widget;
    int i;

    // No change?

    if (x == table->selected_x && y == table->selected_y)
    {
        return;
    }

    // Unfocus current widget:

    i = table->selected_y * table->columns + table->selected_x;

    if (i < table->num_widgets)
    {
        cur_widget = table->widgets[i];

        if (table->widget.focused && IsActualWidget(cur_widget))
        {
            TXT_SetWidgetFocus(cur_widget, 0);
        }
    }

    // Focus new widget.

    new_widget = table->widgets[y * table->columns + x];

    table->selected_x = x;
    table->selected_y = y;

    if (table->widget.focused && new_widget != NULL)
    {
        TXT_SetWidgetFocus(new_widget, 1);
    }
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
        if (IsActualWidget(table->widgets[selected])
         && TXT_SelectableWidget(table->widgets[selected])
         && TXT_WidgetKeyPress(table->widgets[selected], key))
        {
            return 1;
        }
    }

    if (key == KEY_TAB)
    {
        int dir;
        int i;

        dir = TXT_GetModifierState(TXT_MOD_SHIFT) ? -1 : 1;

        // Cycle through all widgets until we find one that can be selected.
        for (i = table->selected_y * table->columns + table->selected_x + dir;
             i >= 0 && i < table->num_widgets;
             i += dir)
        {
            if (IsActualWidget(table->widgets[i])
             && TXT_SelectableWidget(table->widgets[i]))
            {
                ChangeSelection(table, i % table->columns, i / table->columns);
                return 1;
            }
        }

        return 0;
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

                ChangeSelection(table, new_x, new_y);

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

                ChangeSelection(table, new_x, new_y);

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
            if (SelectableCell(table, new_x, table->selected_y))
            {
                // Found a selectable widget!

                ChangeSelection(table, new_x, table->selected_y);

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
            if (SelectableCell(table, new_x, table->selected_y))
            {
                // Found a selectable widget!

                ChangeSelection(table, new_x, table->selected_y);

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

            ChangeSelection(table, new_x, new_y);

            break;
        }
    }
}

static void LayoutCell(txt_table_t *table, int x, int y,
                       int draw_x, int draw_y)
{
    txt_widget_t *widget;
    int col_width;

    widget = table->widgets[y * table->columns + x];

    col_width = widget->w;

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
    txt_widget_t *widget;
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

            widget = table->widgets[i];

            if (IsActualWidget(widget))
            {
                CalculateWidgetDimensions(table, x, y,
                                          column_widths, row_heights,
                                          &widget->w, &widget->h);
                LayoutCell(table, x, y, draw_x, draw_y);
            }

            draw_x += column_widths[x];
        }

        draw_y += row_heights[y];
    }

    free(row_heights);
    free(column_widths);
}

static void TXT_TableDrawer(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_widget_t *widget;
    int i;

    // Check the table's current selection points at something valid before
    // drawing.

    CheckValidSelection(table);

    // Draw all cells

    for (i=0; i<table->num_widgets; ++i)
    {
        widget = table->widgets[i];

        if (IsActualWidget(widget))
        {
            TXT_GotoXY(widget->x, widget->y);
            TXT_DrawWidget(widget);
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

        if (IsActualWidget(widget))
        {
            if (x >= widget->x && x < (signed) (widget->x + widget->w)
             && y >= widget->y && y < (signed) (widget->y + widget->h))
            {
                // This is the widget that was clicked!

                // Select the cell if the widget is selectable

                if (TXT_SelectableWidget(widget))
                {
                    ChangeSelection(table, i % table->columns,
                                           i / table->columns);
                }

                // Propagate click

                TXT_WidgetMousePress(widget, x, y, b);

                break;
            }
        }
    }
}

// Determine whether the table is selectable.

static int TXT_TableSelectable(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    int i;

    // Is the currently-selected cell selectable?

    if (SelectableCell(table, table->selected_x, table->selected_y))
    {
        return 1;
    }

    // Find the first selectable cell and set selected_x, selected_y.

    for (i = 0; i < table->num_widgets; ++i)
    {
        if (IsActualWidget(table->widgets[i])
         && TXT_SelectableWidget(table->widgets[i]))
        {
            ChangeSelection(table, i % table->columns, i / table->columns);
            return 1;
        }
    }

    // No selectable widgets exist within the table.

    return 0;
}

// Need to pass through focus changes to the selected child widget.

static void TXT_TableFocused(TXT_UNCAST_ARG(table), int focused)
{
    TXT_CAST_ARG(txt_table_t, table);
    int i;

    i = table->selected_y * table->columns + table->selected_x;

    if (i < table->num_widgets)
    {
        if (IsActualWidget(table->widgets[i]))
        {
            TXT_SetWidgetFocus(table->widgets[i], focused);
        }
    }
}

txt_widget_class_t txt_table_class =
{
    TXT_TableSelectable,
    TXT_CalcTableSize,
    TXT_TableDrawer,
    TXT_TableKeyPress,
    TXT_TableDestructor,
    TXT_TableMousePress,
    TXT_TableLayout,
    TXT_TableFocused,
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

// Alternative to TXT_NewTable() that allows a list of widgets to be
// provided in its arguments.
txt_table_t *TXT_MakeTable(int columns, TXT_UNCAST_ARG(first_widget), ...)
{
    TXT_CAST_ARG(txt_widget_t, first_widget);
    txt_table_t *table;
    va_list args;

    table = TXT_NewTable(columns);
    TXT_AddWidget(table, first_widget);

    va_start(args, TXT_UNCAST_ARG_NAME(first_widget));

    for (;;)
    {
        txt_widget_t *widget;
        widget = va_arg(args, txt_widget_t *);

        if (widget == NULL)
        {
            break;
        }

        TXT_AddWidget(table, widget);
    }

    va_end(args);

    return table;
}

// Create a horizontal table from a list of widgets.

txt_table_t *TXT_NewHorizBox(TXT_UNCAST_ARG(first_widget), ...)
{
    TXT_CAST_ARG(txt_widget_t, first_widget);
    txt_table_t *result;
    va_list args;
    int num_args;

    // First, find the number of arguments to determine the width of
    // the box.

    va_start(args, TXT_UNCAST_ARG_NAME(first_widget));

    num_args = 1;

    for (;;)
    {
        txt_widget_t *widget;

        widget = va_arg(args, txt_widget_t *);

        if (widget == NULL)
        {
            // End of list

            break;
        }
        else
        {
            ++num_args;
        }
    }
    
    va_end(args);

    // Create the table.

    result = TXT_NewTable(num_args);
    TXT_AddWidget(result, first_widget);

    // Go through the list again and add each widget.

    va_start(args, TXT_UNCAST_ARG_NAME(first_widget));

    for (;;)
    {
        txt_widget_t *widget;

        widget = va_arg(args, txt_widget_t *);

        if (widget == NULL)
        {
            // End of list

            break;
        }
        else
        {
            TXT_AddWidget(result, widget);
        }
    }
    
    va_end(args);

    return result;
}

// Get the currently-selected widget in a table, recursively searching
// through sub-tables if necessary.

txt_widget_t *TXT_GetSelectedWidget(TXT_UNCAST_ARG(table))
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_widget_t *result;
    int index;

    index = table->selected_y * table->columns + table->selected_x;

    result = NULL;

    if (index >= 0 && index < table->num_widgets)
    {
        result = table->widgets[index];

        if (!IsActualWidget(result))
        {
            result = NULL;
        }
    }

    if (result != NULL && result->widget_class == &txt_table_class)
    {
        result = TXT_GetSelectedWidget(result);
    }

    return result;
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
        if (!IsActualWidget(table->widgets[i]))
        {
            continue;
        }

        if (table->widgets[i] == widget)
        {
            // Found the item!  Select it and return.

            ChangeSelection(table, i % table->columns, i / table->columns);

            return 1;
        }

        if (table->widgets[i]->widget_class == &txt_table_class)
        {
            // This item is a subtable.  Recursively search this table.

            if (TXT_SelectWidget(table->widgets[i], widget))
            {
                // Found it in the subtable.  Select this subtable and return.

                ChangeSelection(table, i % table->columns, i / table->columns);

                return 1;
            }
        }
    }

    // Not found.

    return 0;
}

void TXT_SetTableColumns(TXT_UNCAST_ARG(table), int new_columns)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_widget_t **new_widgets;
    txt_widget_t *widget;
    int new_num_widgets;
    int i, j, x;

    // We need as many full rows as are in the current list, plus the
    // remainder from the last row.
    new_num_widgets = (table->num_widgets / table->columns) * new_columns
                    + (table->num_widgets % table->columns);
    new_widgets = calloc(new_num_widgets, sizeof(txt_widget_t *));

    // Reset and add one by one from the old table.
    new_num_widgets = 0;

    for (i = 0; i < table->num_widgets; ++i)
    {
        widget = table->widgets[i];
        x = i % table->columns;

        if (x < new_columns)
        {
            new_widgets[new_num_widgets] = widget;
            ++new_num_widgets;
        }
        else if (IsActualWidget(widget))
        {
            TXT_DestroyWidget(widget);
        }

        // When we reach the last column of a row, we must pad it out with
        // extra widgets to reach the next row.
        if (x == table->columns - 1)
        {
            for (j = table->columns; j < new_columns; ++j)
            {
                // First row? We need to add struts that are used to apply
                // the column widths.
                if (i < table->columns)
                {
                    widget = &TXT_NewStrut(0, 0)->widget;
                }
                else
                {
                    widget = &txt_table_overflow_right;
                }
                new_widgets[new_num_widgets] = widget;
                ++new_num_widgets;
            }
        }
    }

    free(table->widgets);
    table->widgets = new_widgets;
    table->num_widgets = new_num_widgets;
    table->columns = new_columns;
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

    va_end(args);
}

// Moves the select by at least the given number of characters.
// Currently quietly ignores pagex, as we don't use it.

int TXT_PageTable(TXT_UNCAST_ARG(table), int pagex, int pagey)
{
    TXT_CAST_ARG(txt_table_t, table);
    unsigned int *column_widths;
    unsigned int *row_heights;
    int rows;
    int changed = 0;

    rows = TableRows(table);

    row_heights = malloc(sizeof(int) * rows);
    column_widths = malloc(sizeof(int) * table->columns);

    CalcRowColSizes(table, row_heights, column_widths);

    if (pagex)
    {
        // @todo Jump selection to the left or right as needed
    }

    if (pagey)
    {
        int new_x, new_y;
        int distance = 0;
        int dir;

        // What direction are we moving?

        if (pagey > 0)
        {
            dir = 1;
        }
        else
        {
            dir = -1;
        }

        // Move the cursor until the desired distance is reached.

        new_y = table->selected_y;

        while (new_y >= 0 && new_y < rows)
        {
            // We are about to travel a distance equal to the height of the row
            // we are about to leave.

            distance += row_heights[new_y];

            // *Now* increment the loop.

            new_y += dir;

            new_x = FindSelectableColumn(table, new_y, table->selected_x);

            if (new_x >= 0)
            {
                // Found a selectable widget in this column!
                // Select it anyway in case we don't find something better.

                ChangeSelection(table, new_x, new_y);
                changed = 1;

                // ...but is it far enough away?

                if (distance >= abs(pagey))
                {
                    break;
                }
            }
        }
    }

    free(row_heights);
    free(column_widths);

    return changed;
}

