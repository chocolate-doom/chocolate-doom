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

// Base GUI "widget" class that all widgets inherit from.

#ifndef TXT_WIDGET_H
#define TXT_WIDGET_H

typedef struct txt_widget_class_s txt_widget_class_t;
typedef struct txt_widget_s txt_widget_t;
typedef struct txt_callback_table_s txt_callback_table_t;

typedef void (*TxtWidgetSizeCalc)(txt_widget_t *widget, int *w, int *h);
typedef void (*TxtWidgetDrawer)(txt_widget_t *widget, int w, int selected);
typedef void (*TxtWidgetDestroy)(txt_widget_t *widget);
typedef int (*TxtWidgetKeyPress)(txt_widget_t *widget, int key);
typedef void (*TxtWidgetSignalFunc)(txt_widget_t *widget, void *user_data);

struct txt_widget_class_s
{
    TxtWidgetSizeCalc size_calc;
    TxtWidgetDrawer drawer;
    TxtWidgetKeyPress key_press;
    TxtWidgetDestroy destructor;
};

struct txt_widget_s
{
    txt_widget_class_t *widget_class;
    txt_callback_table_t *callback_table;
    int selectable;
    int visible;
};

void TXT_InitWidget(void *widget, txt_widget_class_t *widget_class);
void TXT_CalcWidgetSize(txt_widget_t *widget, int *w, int *h);
void TXT_DrawWidget(txt_widget_t *widget, int w, int selected);
void TXT_SignalConnect(txt_widget_t *widget, char *signal_name,
                       TxtWidgetSignalFunc func, void *user_data);
void TXT_EmitSignal(txt_widget_t *widget, char *signal_name);
int TXT_WidgetKeyPress(txt_widget_t *widget, int key);
void TXT_DestroyWidget(txt_widget_t *widget);

#endif /* #ifndef TXT_WIDGET_H */


