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
#include "doomtype.h"

#include "execute.h"
#include "txt_mouseinput.h"

#include "mouse.h"

int usemouse = 1;

int novert = 0;
int mouseSensitivity = 5;
float mouse_acceleration = 2.0;
int mouse_threshold = 10;
int grabmouse = 1;

int mousebfire = 0;
int mousebforward = 1;
int mousebstrafe = 2;
int mousebstrafeleft = -1;
int mousebstraferight = -1;
int mousebbackward = -1;
int mousebuse = -1;
int mousebprevweapon = -1;
int mousebnextweapon = -1;

int dclick_use = 1;

static int *all_mouse_buttons[] = {
    &mousebfire,
    &mousebstrafe,
    &mousebforward,
    &mousebstrafeleft,
    &mousebstraferight,
    &mousebbackward,
    &mousebuse,
    &mousebprevweapon,
    &mousebnextweapon
};

static void MouseSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    unsigned int i;

    // Check if the same mouse button is used for a different action
    // If so, set the other action(s) to -1 (unset)

    for (i=0; i<arrlen(all_mouse_buttons); ++i)
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

static void ConfigExtraButtons(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_table_t *buttons_table;

    window = TXT_NewWindow("Additional mouse buttons");

    TXT_AddWidgets(window,
                   buttons_table = TXT_NewTable(2),
                   NULL);

    TXT_SetColumnWidths(buttons_table, 29, 5);

    AddMouseControl(buttons_table, "Move backward", &mousebbackward);
    AddMouseControl(buttons_table, "Use", &mousebuse);
    AddMouseControl(buttons_table, "Strafe left", &mousebstrafeleft);
    AddMouseControl(buttons_table, "Strafe right", &mousebstraferight);
    AddMouseControl(buttons_table, "Previous weapon", &mousebprevweapon);
    AddMouseControl(buttons_table, "Next weapon", &mousebnextweapon);
}

void ConfigMouse(void)
{
    txt_window_t *window;
    txt_table_t *motion_table;
    txt_table_t *buttons_table;
    txt_button_t *more_buttons;

    window = TXT_NewWindow("Mouse configuration");

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable mouse", &usemouse),
                   TXT_NewInvertedCheckBox("Allow vertical mouse movement", 
                                           &novert),
                   TXT_NewCheckBox("Grab mouse in windowed mode", 
                                   &grabmouse),
                   TXT_NewCheckBox("Double click acts as \"use\"",
                                   &dclick_use),

                   TXT_NewSeparator("Mouse motion"),
                   motion_table = TXT_NewTable(2),
    
                   TXT_NewSeparator("Buttons"),
                   buttons_table = TXT_NewTable(2),
                   more_buttons = TXT_NewButton("More buttons..."),

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

    TXT_SetColumnWidths(buttons_table, 27, 5);

    AddMouseControl(buttons_table, "Move forward", &mousebforward);
    AddMouseControl(buttons_table, "Strafe on", &mousebstrafe);
    AddMouseControl(buttons_table, "Fire weapon", &mousebfire);
    
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());

    TXT_SignalConnect(more_buttons, "pressed", ConfigExtraButtons, NULL);
}

