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

#ifndef TXT_WIDGET_H
#define TXT_WIDGET_H

/**
 * @file txt_widget.h
 *
 * Base "widget" GUI component class.
 */

#ifndef DOXYGEN

#define TXT_UNCAST_ARG_NAME(name) uncast_ ## name
#define TXT_UNCAST_ARG(name)   void * TXT_UNCAST_ARG_NAME(name)
#define TXT_CAST_ARG(type, name)  type *name = (type *) uncast_ ## name

#else

#define TXT_UNCAST_ARG(name) txt_widget_t *name

#endif

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

/**
 * A GUI widget.
 *
 * A widget is an individual component of a GUI.  Various different widget
 * types exist.
 *
 * Widgets may emit signals.  The types of signal emitted by a widget
 * depend on the type of the widget.  It is possible to be notified
 * when a signal occurs using the @ref TXT_SignalConnect function.
 */

typedef struct txt_widget_s txt_widget_t;

typedef struct txt_widget_class_s txt_widget_class_t;
typedef struct txt_callback_table_s txt_callback_table_t;

typedef void (*TxtWidgetSizeCalc)(TXT_UNCAST_ARG(widget));
typedef void (*TxtWidgetDrawer)(TXT_UNCAST_ARG(widget), int selected);
typedef void (*TxtWidgetDestroy)(TXT_UNCAST_ARG(widget));
typedef int (*TxtWidgetKeyPress)(TXT_UNCAST_ARG(widget), int key);
typedef void (*TxtWidgetSignalFunc)(TXT_UNCAST_ARG(widget), void *user_data);
typedef void (*TxtMousePressFunc)(TXT_UNCAST_ARG(widget), int x, int y, int b);
typedef void (*TxtWidgetLayoutFunc)(TXT_UNCAST_ARG(widget));
typedef int (*TxtWidgetSelectableFunc)(TXT_UNCAST_ARG(widget));

struct txt_widget_class_s
{
    TxtWidgetSelectableFunc selectable;
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
    int visible;
    txt_horiz_align_t align;

    // These are set automatically when the window is drawn and should
    // not be set manually.

    int x, y;
    unsigned int w, h;

    // Pointer up to parent widget that contains this widget.

    txt_widget_t *parent;
};

void TXT_InitWidget(TXT_UNCAST_ARG(widget), txt_widget_class_t *widget_class);
void TXT_CalcWidgetSize(TXT_UNCAST_ARG(widget));
void TXT_DrawWidget(TXT_UNCAST_ARG(widget), int selected);
void TXT_EmitSignal(TXT_UNCAST_ARG(widget), const char *signal_name);
int TXT_WidgetKeyPress(TXT_UNCAST_ARG(widget), int key);
void TXT_WidgetMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b);
void TXT_DestroyWidget(TXT_UNCAST_ARG(widget));
void TXT_LayoutWidget(TXT_UNCAST_ARG(widget));
int TXT_AlwaysSelectable(TXT_UNCAST_ARG(widget));
int TXT_NeverSelectable(TXT_UNCAST_ARG(widget));

/**
 * Set a callback function to be invoked when a signal occurs.
 *
 * @param widget       The widget to watch.
 * @param signal_name  The signal to watch.
 * @param func         The callback function to invoke.
 * @param user_data    User-specified pointer to pass to the callback function.
 */

void TXT_SignalConnect(TXT_UNCAST_ARG(widget), const char *signal_name,
                       TxtWidgetSignalFunc func, void *user_data);

/**
 * Set the policy for how a widget should be aligned within a table.
 * By default, widgets are aligned to the left of the column.
 *
 * @param widget       The widget.
 * @param horiz_align  The alignment to use.
 */

void TXT_SetWidgetAlign(TXT_UNCAST_ARG(widget), txt_horiz_align_t horiz_align);

/**
 * Query whether a widget is selectable with the cursor.
 *
 * @param widget       The widget.
 * @return             Non-zero if the widget is selectable.
 */

int TXT_SelectableWidget(TXT_UNCAST_ARG(widget));

/**
 * Query whether the mouse is hovering over the specified widget.
 *
 * @param widget       The widget.
 * @return             Non-zero if the mouse cursor is over the widget.
 */

int TXT_HoveringOverWidget(TXT_UNCAST_ARG(widget));

/**
 * Set the background to draw the specified widget, depending on
 * whether it is selected and the mouse is hovering over it.
 *
 * @param widget       The widget.
 * @param selected     Whether the widget is selected.
 */

void TXT_SetWidgetBG(TXT_UNCAST_ARG(widget), int selected);

/**
 * Query whether the specified widget is contained within another
 * widget.
 *
 * @param haystack     The widget that might contain needle.
 * @param needle       The widget being queried.
 */

int TXT_ContainsWidget(TXT_UNCAST_ARG(haystack), TXT_UNCAST_ARG(needle));

#endif /* #ifndef TXT_WIDGET_H */

