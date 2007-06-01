// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007 Simon Howard
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
#include "txt_joybinput.h"

#include "joystick.h"

// Joystick enable/disable

int usejoystick = 0;

// Button mappings

int joybfire = 0;
int joybstrafe = 1;
int joybuse = 2;
int joybspeed = 3;

// Joystick to use, as an SDL joystick index:

int joystick_index = -1;

// Which joystick axis to use for horizontal movement, and whether to
// invert the direction:

int joystick_x_axis = 0;
int joystick_x_invert = 0;

// Which joystick axis to use for vertical movement, and whether to
// invert the direction:

int joystick_y_axis = 1;
int joystick_y_invert = 0;

static txt_button_t *joystick_button;

static void SetJoystickButtonLabel(void)
{
    char *name;

    name = "None set";

    if (joystick_index >= 0 && joystick_index < SDL_NumJoysticks())
    {
        name = (char *) SDL_JoystickName(joystick_index);
    }

    TXT_SetButtonLabel(joystick_button, name);
}

void ConfigJoystick(void)
{
    txt_window_t *window;
    txt_table_t *button_table;
    txt_table_t *joystick_table;

    SDL_Init(SDL_INIT_JOYSTICK);

    window = TXT_NewWindow("Joystick configuration");

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable joystick", &usejoystick),
                   joystick_table = TXT_NewTable(2),
                   TXT_NewSeparator("Joystick buttons"),
                   button_table = TXT_NewTable(2),
                   NULL);

    TXT_SetColumnWidths(joystick_table, 20, 15);

    TXT_AddWidgets(joystick_table,
                   TXT_NewLabel("Current joystick"),
                   joystick_button = TXT_NewButton("zzzz"),
                   NULL);

    TXT_SetColumnWidths(button_table, 20, 15);

    TXT_AddWidgets(button_table,
                   TXT_NewLabel("Fire"),
                   TXT_NewJoystickInput(&joybfire),
                   TXT_NewLabel("Use"),
                   TXT_NewJoystickInput(&joybuse),
                   TXT_NewLabel("Strafe"),
                   TXT_NewJoystickInput(&joybstrafe),
                   NULL);

    // High values of joybspeed are used to activate the "always run mode"
    // trick in Vanilla Doom.  If this has been enabled, not only is the
    // joybspeed value meaningless, but the control itself is useless.

    if (joybspeed < 20)
    {
        TXT_AddWidgets(button_table,
                       TXT_NewLabel("Speed"),
                       TXT_NewJoystickInput(&joybspeed),
                       NULL);
    }

    SetJoystickButtonLabel();
}

