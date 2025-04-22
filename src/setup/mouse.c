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

#include "crispy.h"
#include "textscreen.h"
#include "doomtype.h"
#include "m_config.h"
#include "m_controls.h"

#include "execute.h"
#include "txt_mouseinput.h"

#include "mode.h"
#include "mouse.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-mouse"

static int usemouse = 1;

static int mouseSensitivity = 5;
static int mouseSensitivity_x2 = 5; // [crispy]
static float mouse_acceleration = 2.0;
static int mouse_threshold = 10;
static int mouseSensitivity_y = 5; // [crispy]
static float mouse_acceleration_y = 1.0; // [crispy]
static int mouse_threshold_y = 0; // [crispy]
static int mouse_y_invert = 0; // [crispy]
static int grabmouse = 1;

int novert = 1;

static int *game_mouse_buttons[] = {
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
    &mousebmouselook, // [crispy]
    &mousebreverse, // [crispy]
    &mousebspeed,
    &mousebinvleft,
    &mousebinvright,
    &mousebuseartifact,
    &mousebinvuse, // [crispy]
    &mousebturnleft,
    &mousebturnright,
};

// [crispy]
static int *map_mouse_buttons[] = {
    &mousebmapzoomin,
    &mousebmapzoomout,
    &mousebmapmaxzoom,
    &mousebmapfollow,
};

static void MouseSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    unsigned int i;

    // Check if the same mouse button is used for a different action
    // If so, set the other action(s) to -1 (unset)

    for (i=0; i<arrlen(game_mouse_buttons); ++i)
    {
        if (*game_mouse_buttons[i] == *variable
         && game_mouse_buttons[i] != variable)
        {
            *game_mouse_buttons[i] = -1;
        }
    }
}

static void MouseMapSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    unsigned int i;

    // Check if the same mouse button is used for a different action
    // If so, set the other action(s) to -1 (unset)

    for (i=0; i<arrlen(map_mouse_buttons); ++i)
    {
        if (*map_mouse_buttons[i] == *variable
         && map_mouse_buttons[i] != variable)
        {
            *map_mouse_buttons[i] = -1;
        }
    }
}

static void AddMouseControl(TXT_UNCAST_ARG(table), const char *label, int *var)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_mouse_input_t *mouse_input;

    TXT_AddWidget(table, TXT_NewLabel(label));

    mouse_input = TXT_NewMouseInput(var);
    TXT_AddWidget(table, mouse_input);

    TXT_SignalConnect(mouse_input, "set", MouseSetCallback, var);
}

static void AddMouseMapControl(TXT_UNCAST_ARG(table), const char *label, int *var)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_mouse_input_t *mouse_input;

    TXT_AddWidget(table, TXT_NewLabel(label));

    mouse_input = TXT_NewMouseInput(var);
    TXT_AddWidget(table, mouse_input);

    TXT_SignalConnect(mouse_input, "set", MouseMapSetCallback, var);
}

static void ConfigExtraButtons(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_table_t *buttons_table, *am_buttons_table;

    window = TXT_NewWindow("Additional mouse buttons");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                   buttons_table = TXT_NewTable(4),
                   NULL);

    TXT_SetColumnWidths(buttons_table, 16, 11, 16, 10);

    AddMouseControl(buttons_table, "Move forward", &mousebforward);
    AddMouseControl(buttons_table, "Strafe left", &mousebstrafeleft);
    AddMouseControl(buttons_table, "Move backward", &mousebbackward);
    AddMouseControl(buttons_table, "Strafe right", &mousebstraferight);
    AddMouseControl(buttons_table, "Previous weapon", &mousebprevweapon);
    AddMouseControl(buttons_table, "Strafe on", &mousebstrafe);
    AddMouseControl(buttons_table, "Next weapon", &mousebnextweapon);
    AddMouseControl(buttons_table, "Run", &mousebspeed);
    
    if (gamemission == heretic || gamemission == hexen)
    {
      AddMouseControl(buttons_table, "Quick Reverse", &mousebreverse);
      AddMouseControl(buttons_table, "Mouselook", &mousebmouselook);
      AddMouseControl(buttons_table, "Inventory left", &mousebinvleft);
      AddMouseControl(buttons_table, "Inventory right", &mousebinvright);
      AddMouseControl(buttons_table, "Use artifact", &mousebuseartifact);
    }

    if (gamemission == strife) // [crispy]
    {
        AddMouseControl(buttons_table, "Quick Reverse", &mousebreverse);
        AddMouseControl(buttons_table, "Mouselook", &mousebmouselook);
        AddMouseControl(buttons_table, "Inventory left", &mousebinvleft);
        AddMouseControl(buttons_table, "Inventory right", &mousebinvright);
        AddMouseControl(buttons_table, "Use inventory", &mousebinvuse);
    }

    if (gamemission == hexen || gamemission == strife)
    {
        AddMouseControl(buttons_table, "Jump", &mousebjump);
    }

    if (gamemission == doom) // [crispy]
    {
        AddMouseControl(buttons_table, "Quick Reverse", &mousebreverse);
        AddMouseControl(buttons_table, "Mouse Look [*]", &mousebmouselook);
        AddMouseControl(buttons_table, "Jump [*]", &mousebjump);
    }

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Automap"),
                   am_buttons_table = TXT_NewTable(4),
                   NULL);

    TXT_SetColumnWidths(am_buttons_table, 16, 11, 16, 10);

    AddMouseMapControl(am_buttons_table, "Zoom in", &mousebmapzoomin);
    AddMouseMapControl(am_buttons_table, "Zoom out", &mousebmapzoomout);
    AddMouseMapControl(am_buttons_table, "Max zoom out", &mousebmapmaxzoom);
    AddMouseMapControl(am_buttons_table, "Toggle follow", &mousebmapfollow);
}

void ConfigMouse(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    window = TXT_NewWindow("Mouse configuration");

    TXT_SetTableColumns(window, 2);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable mouse", &usemouse),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewInvertedCheckBox("Allow vertical mouse movement", 
                                           &novert),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Grab mouse in windowed mode", 
                                   &grabmouse),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewCheckBox("Double click acts as \"use\"",
                                   &dclick_use),
                   TXT_TABLE_OVERFLOW_RIGHT,

                   TXT_NewSeparator("Mouse motion"),
                   TXT_NewLabel("Speed (h/turn)"),
                   TXT_NewSpinControl(&mouseSensitivity, 0, 255), // [crispy] extended range
                   TXT_NewLabel("Speed (h/strafe)"),
                   TXT_NewSpinControl(&mouseSensitivity_x2, 0, 255), // [crispy] extended range
                   TXT_NewLabel("Acceleration (h)"),
                   TXT_NewFloatSpinControl(&mouse_acceleration, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold (h)"),
                   TXT_NewSpinControl(&mouse_threshold, 0, 32),
                   TXT_NewLabel("Speed (v)"),
                   TXT_NewSpinControl(&mouseSensitivity_y, 0, 255), // [crispy] extended range
                   TXT_NewLabel("Acceleration (v)"),
                   TXT_NewFloatSpinControl(&mouse_acceleration_y, 1.0, 5.0),
                   TXT_NewLabel("Acceleration threshold (v)"),
                   TXT_NewSpinControl(&mouse_threshold_y, 0, 32),

                   TXT_NewSeparator("Buttons"),
                   NULL);

    AddMouseControl(window, "Fire/Attack", &mousebfire);
    AddMouseControl(window, "Use", &mousebuse);

    TXT_AddWidget(window,
                  TXT_NewButton2("More controls...", ConfigExtraButtons, NULL));
}

void BindMouseVariables(void)
{
    M_BindIntVariable("use_mouse",               &usemouse);
    M_BindIntVariable("novert",                  &novert);
    M_BindIntVariable("grabmouse",               &grabmouse);
    M_BindIntVariable("mouse_sensitivity",       &mouseSensitivity);
    M_BindIntVariable("mouse_threshold",         &mouse_threshold);
    M_BindFloatVariable("mouse_acceleration",    &mouse_acceleration);
    // [crispy]
    M_BindIntVariable("mouse_sensitivity_x2",    &mouseSensitivity_x2);
    M_BindIntVariable("mouse_sensitivity_y",     &mouseSensitivity_y);
    M_BindIntVariable("mouse_threshold_y",       &mouse_threshold_y);
    M_BindFloatVariable("mouse_acceleration_y",  &mouse_acceleration_y);
    M_BindIntVariable("mouse_y_invert",          &mouse_y_invert);
    M_BindIntVariable("crispy_mouselook",        &crispy->mouselook);
}
