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

#ifndef TXT_TABLE_H
#define TXT_TABLE_H

typedef struct txt_table_s txt_table_t;

#include "txt_widget.h" 

struct txt_table_s
{
    txt_widget_t widget;

    // Widgets in this table
    // The widget at (x,y) in the table is widgets[columns * y + x]

    txt_widget_t **widgets;
    int num_widgets;

    // Number of columns

    int columns;

    // Currently selected 

    int selected_x;
    int selected_y;
};

extern txt_widget_class_t txt_table_class;

txt_table_t *TXT_NewTable(int columns);
txt_table_t *TXT_NewHorizBox(TXT_UNCAST_ARG(first_widget), ...);
void TXT_InitTable(txt_table_t *table, int columns);
txt_widget_t *TXT_GetSelectedWidget(TXT_UNCAST_ARG(table));
void TXT_AddWidget(TXT_UNCAST_ARG(table), TXT_UNCAST_ARG(widget));
void TXT_AddWidgets(TXT_UNCAST_ARG(table), ...);
int TXT_SelectWidget(TXT_UNCAST_ARG(table), TXT_UNCAST_ARG(widget));
void TXT_SetColumnWidths(TXT_UNCAST_ARG(table), ...);
void TXT_ClearTable(TXT_UNCAST_ARG(table));

#endif /* #ifndef TXT_TABLE_T */


