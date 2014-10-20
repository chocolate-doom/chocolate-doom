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

#include <stdlib.h>

#include "textscreen.h"
#include "doomtype.h"
#include "m_config.h"
#include "m_controls.h"

#include "execute.h"
#include "txt_mouseinput.h"

#include "mode.h"
#include "mouse.h"

static int usemouse = 1;

static int mouseSensitivity = 5;
static float mouse_acceleration = 2.0;
static int mouse_threshold = 10;
static int mouseSensitivity_y = 5;
static float mouse_acceleration_y = 1.0;
static int mouse_threshold_y = 0;
static int mouse_y_invert = 0;
static int grabmouse = 1;

int novert = 1;

static int *all_mouse_buttons[] = {
    &mousebfire,
    &mousebstrafe,
    &mousebforward,
    &mousebstrafeleft,
    &mousebstraferight,
    &mousebbackward,
    &mousebuse,
    &mousebjump,
    &mousebprevweapon,
    &mousebnextweapon,
    &mousebmouselook
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

    if (gamemission == hexen)
    {
        AddMouseControl(buttons_table, "Jump", &mousebjump);
    }

    AddMouseControl(buttons_table, "Previous weapon", &mousebprevweapon);
    AddMouseControl(buttons_table, "Next weapon", &mousebnextweapon);
    if (gamemission == doom) // Crispy
    {
    AddMouseControl(buttons_table, "Free look [*]", &mousebmouselook);
    }
}

void ConfigMouse(void)
{
    txt_window_t *window;
    txt_table_t *motion_table;
    txt_table_t *buttons_table;

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
                   TXT_NewButton2("More controls...",
                                  ConfigExtraButtons,
                                  NULL),
                   NULL);

    TXT_SetColumnWidths(motion_table, 27, 5);

    if (gamemission == doom) // Crispy
    {
    TXT_AddWidgets(motion_table,
                   TXT_NewLabel("Speed (h)"),
                   TXT_NewSpinControl(&mouseSensitivity, 1, 256),
                   TXT_NewLabel("Acceleration (h)"),
                   TXT_NewFloatSpinControl(&mouse_acceleration, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold (h)"),
                   TXT_NewSpinControl(&mouse_threshold, 0, 32),
                   TXT_NewLabel("Speed (v)"),
                   TXT_NewSpinControl(&mouseSensitivity_y, 1, 256),
                   TXT_NewLabel("Acceleration (v)"),
                   TXT_NewFloatSpinControl(&mouse_acceleration_y, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold (v)"),
                   TXT_NewSpinControl(&mouse_threshold_y, 0, 32),
                   TXT_NewCheckBox("Invert mouse (v)", &mouse_y_invert),
                   NULL);
    }
    else
    {
    TXT_AddWidgets(motion_table,
                   TXT_NewLabel("Speed"),
                   TXT_NewSpinControl(&mouseSensitivity, 1, 256),
                   TXT_NewLabel("Acceleration"),
                   TXT_NewFloatSpinControl(&mouse_acceleration, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold"),
                   TXT_NewSpinControl(&mouse_threshold, 0, 32),
                   NULL);
    }

    TXT_SetColumnWidths(buttons_table, 27, 5);

    AddMouseControl(buttons_table, "Fire/Attack", &mousebfire);
    AddMouseControl(buttons_table, "Move forward", &mousebforward);
    AddMouseControl(buttons_table, "Strafe on", &mousebstrafe);
    
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
}

void BindMouseVariables(void)
{
    M_BindVariable("use_mouse",            &usemouse);
    M_BindVariable("novert",               &novert);
    M_BindVariable("mouse_sensitivity",    &mouseSensitivity);
    M_BindVariable("mouse_acceleration",   &mouse_acceleration);
    M_BindVariable("mouse_threshold",      &mouse_threshold);
    if (gamemission == doom) // Crispy
    {
    M_BindVariable("mouse_sensitivity_y",  &mouseSensitivity_y);
    M_BindVariable("mouse_acceleration_y", &mouse_acceleration_y);
    M_BindVariable("mouse_threshold_y",    &mouse_threshold_y);
    M_BindVariable("mouse_y_invert",       &mouse_y_invert);
    }
    M_BindVariable("grabmouse",            &grabmouse);
}
