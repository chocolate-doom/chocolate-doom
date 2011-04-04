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

#include "doomkeys.h"

#include "txt_button.h"
#include "txt_dropdown.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

typedef struct 
{
    txt_window_t *window;
    txt_dropdown_list_t *list;
    int item;
} callback_data_t;

// Check if the selected value for a list is valid

static int ValidSelection(txt_dropdown_list_t *list)
{
    return *list->variable >= 0 && *list->variable < list->num_values;
}

// Calculate the Y position for the selector window

static int SelectorWindowY(txt_dropdown_list_t *list)
{
    if (ValidSelection(list))
    {
        return list->widget.y - 1 - *list->variable;
    }
    else
    {
        return list->widget.y - 1 - (list->num_values / 2);
    }
}

// Called when a button in the selector window is pressed

static void ItemSelected(TXT_UNCAST_ARG(button), TXT_UNCAST_ARG(callback_data))
{
    TXT_CAST_ARG(callback_data_t, callback_data);

    // Set the variable

    *callback_data->list->variable = callback_data->item;

    TXT_EmitSignal(callback_data->list, "changed");

    // Close the window

    TXT_CloseWindow(callback_data->window);
}

// Free callback data when the window is closed

static void FreeCallbackData(TXT_UNCAST_ARG(list), 
                             TXT_UNCAST_ARG(callback_data))
{
    TXT_CAST_ARG(callback_data_t, callback_data);

    free(callback_data);
}

// Catch presses of escape and close the window.

static int SelectorWindowListener(txt_window_t *window, int key, void *user_data)
{
    if (key == KEY_ESCAPE)
    {
        TXT_CloseWindow(window);
        return 1;
    }

    return 0;
}

static int SelectorMouseListener(txt_window_t *window, int x, int y, int b,
                                 void *unused)
{
    txt_widget_t *win;

    win = (txt_widget_t *) window;

    if (x < win->x || x > win->x + win->w || y < win->y || y > win->y + win->h)
    {
        TXT_CloseWindow(window);
        return 1;
    }

    return 0;
}

// Open the dropdown list window to select an item

static void OpenSelectorWindow(txt_dropdown_list_t *list)
{
    txt_window_t *window;
    int i;

    // Open a simple window with no title bar or action buttons.

    window = TXT_NewWindow(NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);

    // Position the window so that the currently selected item appears
    // over the top of the list widget.

    TXT_SetWindowPosition(window, TXT_HORIZ_LEFT, TXT_VERT_TOP,
                          list->widget.x - 2, SelectorWindowY(list));

    // Add a button to the window for each option in the list.

    for (i=0; i<list->num_values; ++i)
    {
        txt_button_t *button;
        callback_data_t *data;

        button = TXT_NewButton(list->values[i]);

        TXT_AddWidget(window, button);

        // Callback struct

        data = malloc(sizeof(callback_data_t));
        data->list = list;
        data->window = window;
        data->item = i;
        
        // When the button is pressed, invoke the button press callback
       
        TXT_SignalConnect(button, "pressed", ItemSelected, data);
        
        // When the window is closed, free back the callback struct

        TXT_SignalConnect(window, "closed", FreeCallbackData, data);

        // Is this the currently-selected value?  If so, select the button
        // in the window as the default.
        
        if (i == *list->variable)
        {
            TXT_SelectWidget(window, button);
        }
    }

    // Catch presses of escape in this window and close it.

    TXT_SetKeyListener(window, SelectorWindowListener, NULL);
    TXT_SetMouseListener(window, SelectorMouseListener, NULL);
}

static int DropdownListWidth(txt_dropdown_list_t *list)
{
    int i;
    int result;

    // Find the maximum string width
 
    result = 0;

    for (i=0; i<list->num_values; ++i)
    {
        int w = strlen(list->values[i]);
        if (w > result) 
        {
            result = w;
        }
    }

    return result;
}

static void TXT_DropdownListSizeCalc(TXT_UNCAST_ARG(list))
{
    TXT_CAST_ARG(txt_dropdown_list_t, list);

    list->widget.w = DropdownListWidth(list);
    list->widget.h = 1;
}

static void TXT_DropdownListDrawer(TXT_UNCAST_ARG(list), int selected)
{
    TXT_CAST_ARG(txt_dropdown_list_t, list);
    unsigned int i;
    const char *str;

    // Set bg/fg text colors.

    TXT_SetWidgetBG(list, selected);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    // Select a string to draw from the list, if the current value is
    // in range.  Otherwise fall back to a default.

    if (ValidSelection(list))
    {
        str = list->values[*list->variable];
    }
    else
    {
        str = "???";
    }

    // Draw the string and fill to the end with spaces

    TXT_DrawString(str);

    for (i=strlen(str); i<list->widget.w; ++i) 
    {
        TXT_DrawString(" ");
    }
}

static void TXT_DropdownListDestructor(TXT_UNCAST_ARG(list))
{
}

static int TXT_DropdownListKeyPress(TXT_UNCAST_ARG(list), int key)
{
    TXT_CAST_ARG(txt_dropdown_list_t, list);

    if (key == KEY_ENTER)
    {
        OpenSelectorWindow(list);
        return 1;
    }
    
    return 0;
}

static void TXT_DropdownListMousePress(TXT_UNCAST_ARG(list), 
                                       int x, int y, int b)
{
    TXT_CAST_ARG(txt_dropdown_list_t, list);

    // Left mouse click does the same as selecting and pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_DropdownListKeyPress(list, KEY_ENTER);
    }
}

txt_widget_class_t txt_dropdown_list_class =
{
    TXT_AlwaysSelectable,
    TXT_DropdownListSizeCalc,
    TXT_DropdownListDrawer,
    TXT_DropdownListKeyPress,
    TXT_DropdownListDestructor,
    TXT_DropdownListMousePress,
    NULL,
};

txt_dropdown_list_t *TXT_NewDropdownList(int *variable, char **values, 
                                         int num_values)
{
    txt_dropdown_list_t *list;

    list = malloc(sizeof(txt_dropdown_list_t));

    TXT_InitWidget(list, &txt_dropdown_list_class);
    list->variable = variable;
    list->values = values;
    list->num_values = num_values;

    return list;
}

