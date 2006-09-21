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

// Base GUI "widget" class that all widgets inherit from.

#ifndef TXT_WIDGET_H
#define TXT_WIDGET_H

#define TXT_UNCAST_ARG_NAME(name) uncast_ ## name
#define TXT_UNCAST_ARG(name)   void * TXT_UNCAST_ARG_NAME(name)
#define TXT_CAST_ARG(type, name)  type *name = (type *) uncast_ ## name

typedef enum
{
    TXT_VERT_TOP,
    TXT_VERT_CENTER,
    TXT_VERT_BOTTOM,
} txt_vert_align_t;

typedef enum
{
    TXT_HORIZ_LEFT,
    TXT_HORIZ_CENTER,
    TXT_HORIZ_RIGHT,
} txt_horiz_align_t;

typedef struct txt_widget_class_s txt_widget_class_t;
typedef struct txt_widget_s txt_widget_t;
typedef struct txt_callback_table_s txt_callback_table_t;

typedef void (*TxtWidgetSizeCalc)(TXT_UNCAST_ARG(widget));
typedef void (*TxtWidgetDrawer)(TXT_UNCAST_ARG(widget), int selected);
typedef void (*TxtWidgetDestroy)(TXT_UNCAST_ARG(widget));
typedef int (*TxtWidgetKeyPress)(TXT_UNCAST_ARG(widget), int key);
typedef void (*TxtWidgetSignalFunc)(TXT_UNCAST_ARG(widget), void *user_data);
typedef void (*TxtMousePressFunc)(TXT_UNCAST_ARG(widget), int x, int y, int b);
typedef void (*TxtWidgetLayoutFunc)(TXT_UNCAST_ARG(widget));

struct txt_widget_class_s
{
    TxtWidgetSizeCalc size_calc;
    TxtWidgetDrawer drawer;
    TxtWidgetKeyPress key_press;
    TxtWidgetDestroy destructor;
    TxtMousePressFunc mouse_press;
    TxtWidgetLayoutFunc layout;
};

struct txt_widget_s
{
    txt_widget_class_t *widget_class;
    txt_callback_table_t *callback_table;
    int selectable;
    int visible;
    txt_horiz_align_t align;

    // These are set automatically when the window is drawn and should
    // not be set manually.

    int x, y;
    unsigned int w, h;
};

void TXT_InitWidget(TXT_UNCAST_ARG(widget), txt_widget_class_t *widget_class);
void TXT_CalcWidgetSize(TXT_UNCAST_ARG(widget));
void TXT_DrawWidget(TXT_UNCAST_ARG(widget), int selected);
void TXT_SignalConnect(TXT_UNCAST_ARG(widget), char *signal_name,
                       TxtWidgetSignalFunc func, void *user_data);
void TXT_EmitSignal(TXT_UNCAST_ARG(widget), char *signal_name);
int TXT_WidgetKeyPress(TXT_UNCAST_ARG(widget), int key);
void TXT_WidgetMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b);
void TXT_DestroyWidget(TXT_UNCAST_ARG(widget));
void TXT_SetWidgetAlign(TXT_UNCAST_ARG(widget), txt_horiz_align_t horiz_align);
void TXT_LayoutWidget(TXT_UNCAST_ARG(widget));

#endif /* #ifndef TXT_WIDGET_H */


