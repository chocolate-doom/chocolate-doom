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

#include "textscreen.h"

#include "testconfig.h"
#include "txt_mouseinput.h"

#include "mouse.h"

int use_mouse = 1;

int novert = 0;
int mouseSensitivity = 5;
float mouse_acceleration = 1.0;
int mouse_threshold = 10;
int grabmouse = 1;

int mousebfire = 0;
int mousebforward = 1;
int mousebstrafe = 2;

static int *all_mouse_buttons[] = {&mousebfire, &mousebstrafe, 
                                   &mousebforward};

static void MouseSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    unsigned int i;

    // Check if the same mouse button is used for a different action
    // If so, set the other action(s) to -1 (unset)

    for (i=0; i<sizeof(all_mouse_buttons) / sizeof(*all_mouse_buttons); ++i)
    {
        if (*all_mouse_buttons[i] == *variable
         && all_mouse_buttons[i] != variable)
        {
            *all_mouse_buttons[i] = -1;
        }
    }
}

static void AddMouseControl(txt_table_t *table, char *label, int *var)
{
    txt_mouse_input_t *mouse_input;

    TXT_AddWidget(table, TXT_NewLabel(label));

    mouse_input = TXT_NewMouseInput(var);
    TXT_AddWidget(table, mouse_input);

    TXT_SignalConnect(mouse_input, "set", MouseSetCallback, var);
}

void ConfigMouse(void)
{
    txt_window_t *window;
    txt_table_t *motion_table;
    txt_table_t *button_table;

    window = TXT_NewWindow("Mouse configuration");

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable mouse", &use_mouse),
                   TXT_NewInvertedCheckBox("Allow vertical mouse movement", 
                                           &novert),
                   TXT_NewCheckBox("Grab mouse in windowed mode", 
                                          &grabmouse),

                   TXT_NewSeparator("Mouse motion"),
                   motion_table = TXT_NewTable(2),
    
                   TXT_NewSeparator("Mouse buttons"),

                   button_table = TXT_NewTable(2),
                   NULL);

    TXT_SetColumnWidths(motion_table, 27, 5);

    TXT_AddWidgets(motion_table,
                   TXT_NewLabel("Speed"),
                   TXT_NewSpinControl(&mouseSensitivity, 1, 256),
                   TXT_NewLabel("Acceleration"),
                   TXT_NewFloatSpinControl(&mouse_acceleration, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold"),
                   TXT_NewSpinControl(&mouse_threshold, 0, 32),
                   NULL);

    TXT_SetColumnWidths(button_table, 27, 5);

    AddMouseControl(button_table, "Fire weapon", &mousebfire);
    AddMouseControl(button_table, "Move forward", &mousebforward);
    AddMouseControl(button_table, "Strafe on", &mousebstrafe);
    
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
}

