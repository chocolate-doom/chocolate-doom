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

#define UNCAST(name)   void *uncast_ ## name
#define CAST(type, name)  type *name = (type *) uncast_ ## name

typedef struct txt_widget_class_s txt_widget_class_t;
typedef struct txt_widget_s txt_widget_t;
typedef struct txt_callback_table_s txt_callback_table_t;

typedef void (*TxtWidgetSizeCalc)(UNCAST(widget), int *w, int *h);
typedef void (*TxtWidgetDrawer)(UNCAST(widget), int w, int selected);
typedef void (*TxtWidgetDestroy)(UNCAST(widget));
typedef int (*TxtWidgetKeyPress)(UNCAST(widget), int key);
typedef void (*TxtWidgetSignalFunc)(UNCAST(widget), void *user_data);

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

void TXT_InitWidget(UNCAST(widget), txt_widget_class_t *widget_class);
void TXT_CalcWidgetSize(UNCAST(widget), int *w, int *h);
void TXT_DrawWidget(UNCAST(widget), int w, int selected);
void TXT_SignalConnect(UNCAST(widget), char *signal_name,
                       TxtWidgetSignalFunc func, void *user_data);
void TXT_EmitSignal(UNCAST(widget), char *signal_name);
int TXT_WidgetKeyPress(UNCAST(widget), int key);
void TXT_DestroyWidget(UNCAST(widget));

#endif /* #ifndef TXT_WIDGET_H */


