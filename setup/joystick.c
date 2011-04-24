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

#include "doomtype.h"
#include "textscreen.h"
#include "txt_joybinput.h"

#include "execute.h"
#include "joystick.h"

typedef enum
{
    CALIBRATE_CENTER,
    CALIBRATE_LEFT,
    CALIBRATE_UP,
} calibration_stage_t;

// SDL joystick successfully initialized?

static int joystick_initted = 0;

// Joystick enable/disable

int usejoystick = 0;

// Button mappings

int joybfire = 0;
int joybstrafe = 1;
int joybuse = 3;
int joybspeed = 2;
int joybstrafeleft = -1;
int joybstraferight = -1;
int joybprevweapon = -1;
int joybnextweapon = -1;

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

static int *all_joystick_buttons[] = {
    &joybstraferight, &joybstrafeleft, &joybfire, &joybspeed,
    &joybuse, &joybstrafe, &joybprevweapon, &joybnextweapon
};

//
// Calibration 
//

static txt_window_t *calibration_window;
static txt_label_t *calibration_label;
static calibration_stage_t calibrate_stage;
static SDL_Joystick **all_joysticks = NULL;

// Set the label showing the name of the currently selected joystick

static void SetJoystickButtonLabel(void)
{
    char *name;

    name = "None set";

    if (joystick_initted 
     && joystick_index >= 0 && joystick_index < SDL_NumJoysticks())
    {
        name = (char *) SDL_JoystickName(joystick_index);
    }

    TXT_SetButtonLabel(joystick_button, name);
}

// Try to open all joysticks visible to SDL.

static int OpenAllJoysticks(void)
{
    int i;
    int num_joysticks;
    int result;

    if (!joystick_initted)
    {
        return 0;
    }

    // SDL_JoystickOpen() all joysticks.

    num_joysticks = SDL_NumJoysticks();

    all_joysticks = malloc(sizeof(SDL_Joystick *) * num_joysticks);

    result = 0;

    for (i=0; i<num_joysticks; ++i) 
    {
        all_joysticks[i] = SDL_JoystickOpen(i);

        // If any joystick is successfully opened, return true.

        if (all_joysticks[i] != NULL)
        {
            result = 1;
        }
    }

    // Success? Turn on joystick events.

    if (result)
    {
        SDL_JoystickEventState(SDL_ENABLE);
    }
    else
    {
        free(all_joysticks);
        all_joysticks = NULL;
    }

    return result;
}

// Close all the joysticks opened with OpenAllJoysticks()

static void CloseAllJoysticks(void)
{
    int i;
    int num_joysticks;

    num_joysticks = SDL_NumJoysticks();

    for (i=0; i<num_joysticks; ++i)
    {
        if (all_joysticks[i] != NULL)
        {
            SDL_JoystickClose(all_joysticks[i]);
        }
    }

    SDL_JoystickEventState(SDL_DISABLE);

    free(all_joysticks);
    all_joysticks = NULL;
}

static void SetCalibrationLabel(void)
{
    char *message = "???";

    switch (calibrate_stage)
    {
        case CALIBRATE_CENTER:
            message = "Move the joystick to the\n"
                      "center, and press a button.";
            break;
        case CALIBRATE_UP:
            message = "Move the joystick up,\n"
                      "and press a button.";
            break;
        case CALIBRATE_LEFT:
            message = "Move the joystick to the\n"
                      "left, and press a button.";
            break;
    }

    TXT_SetLabel(calibration_label, message);
}

static void CalibrateAxis(int *axis_index, int *axis_invert)
{
    SDL_Joystick *joystick;
    int best_axis;
    int best_value;
    int best_invert;
    Sint16 axis_value;
    int i;

    joystick = all_joysticks[joystick_index];

    // Check all axes to find which axis has the largest value.  We test
    // for one axis at a time, so eg. when we prompt to push the joystick 
    // left, whichever axis has the largest value is the left axis.

    best_axis = 0;
    best_value = 0;
    best_invert = 0;

    for (i=0; i<SDL_JoystickNumAxes(joystick); ++i)
    {
        axis_value = SDL_JoystickGetAxis(joystick, i);
    
        if (abs(axis_value) > best_value)
        {
            best_value = abs(axis_value);
            best_invert = axis_value > 0;
            best_axis = i;
        }
    }

    // Save the best values we have found

    *axis_index = best_axis;
    *axis_invert = best_invert;
}

static int CalibrationEventCallback(SDL_Event *event, void *user_data)
{
    if (event->type == SDL_JOYBUTTONDOWN
     && (joystick_index == -1 || event->jbutton.which == joystick_index))
    {
        switch (calibrate_stage)
        {
            case CALIBRATE_CENTER:
                // Centering stage selects which joystick to use.
                joystick_index = event->jbutton.which;
                break;

            case CALIBRATE_LEFT:
                CalibrateAxis(&joystick_x_axis, &joystick_x_invert);
                break;

            case CALIBRATE_UP:
                CalibrateAxis(&joystick_y_axis, &joystick_y_invert);
                break;
        }

        if (calibrate_stage == CALIBRATE_UP)
        {
            // Final stage; close the window

            TXT_CloseWindow(calibration_window);
        }
        else
        {
            // Advance to the next calibration stage

            ++calibrate_stage;
            SetCalibrationLabel();
        }

        return 1;
    }

    return 0;
}

static void NoJoystick(void)
{
    txt_window_t *window;

    window = TXT_NewWindow(NULL);

    TXT_AddWidget(window,
                  TXT_NewLabel("No joysticks could be opened.\n\n"
                               "Try configuring your joystick from within\n"
                               "your OS first."));

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowEscapeAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);

    joystick_index = -1;
    SetJoystickButtonLabel();
}

static void CalibrateWindowClosed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    CloseAllJoysticks();
    TXT_SDL_SetEventCallback(NULL, NULL);
    SetJoystickButtonLabel();
}

static void CalibrateJoystick(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    calibrate_stage = CALIBRATE_CENTER;

    // Try to open all available joysticks.  If none are opened successfully,
    // bomb out with an error.

    if (!OpenAllJoysticks())
    {
        NoJoystick();
        return;
    }

    calibration_window = TXT_NewWindow("Joystick calibration");

    TXT_AddWidgets(calibration_window, 
                   TXT_NewLabel("Please follow the following instructions\n"
                                "in order to calibrate your joystick."),
                   TXT_NewStrut(0, 1),
                   calibration_label = TXT_NewLabel("zzz"),
                   TXT_NewStrut(0, 1),
                   NULL);

    TXT_SetWindowAction(calibration_window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(calibration_window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(calibration_window));
    TXT_SetWindowAction(calibration_window, TXT_HORIZ_RIGHT, NULL);

    TXT_SetWidgetAlign(calibration_label, TXT_HORIZ_CENTER);
    TXT_SDL_SetEventCallback(CalibrationEventCallback, NULL);

    TXT_SignalConnect(calibration_window, "closed", CalibrateWindowClosed, NULL);

    // Start calibration

    joystick_index = -1;
    calibrate_stage = CALIBRATE_CENTER;

    SetCalibrationLabel();
}

void JoyButtonSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    unsigned int i;

    // Only allow a button to be bound to one action at a time.  If 
    // we assign a key that another action is using, set that other action
    // to -1.

    for (i=0; i<arrlen(all_joystick_buttons); ++i)
    {
        if (variable != all_joystick_buttons[i]
         && *variable == *all_joystick_buttons[i])
        {
            *all_joystick_buttons[i] = -1;
        }
    }
}


// 
// GUI
//

static void JoystickWindowClosed(TXT_UNCAST_ARG(window), TXT_UNCAST_ARG(unused))
{
    if (joystick_initted)
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        joystick_initted = 0;
    }
}

static void AddJoystickControl(txt_table_t *table, char *label, int *var)
{
    txt_joystick_input_t *joy_input;

    joy_input = TXT_NewJoystickInput(var);

    TXT_AddWidget(table, TXT_NewLabel(label));
    TXT_AddWidget(table, joy_input);

    TXT_SignalConnect(joy_input, "set", JoyButtonSetCallback, var);
}

void ConfigJoystick(void)
{
    txt_window_t *window;
    txt_table_t *button_table;
    txt_table_t *joystick_table;

    if (!joystick_initted) 
    {
        joystick_initted = SDL_Init(SDL_INIT_JOYSTICK) >= 0;
    }

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

    AddJoystickControl(button_table, "Fire", &joybfire);
    AddJoystickControl(button_table, "Use", &joybuse);

    // High values of joybspeed are used to activate the "always run mode"
    // trick in Vanilla Doom.  If this has been enabled, not only is the
    // joybspeed value meaningless, but the control itself is useless.

    if (joybspeed < 20)
    {
        AddJoystickControl(button_table, "Speed", &joybspeed);
    }

    AddJoystickControl(button_table, "Strafe", &joybstrafe);

    AddJoystickControl(button_table, "Strafe Left", &joybstrafeleft);
    AddJoystickControl(button_table, "Strafe Right", &joybstraferight);
    AddJoystickControl(button_table, "Previous weapon", &joybprevweapon);
    AddJoystickControl(button_table, "Next weapon", &joybnextweapon);

    TXT_SignalConnect(joystick_button, "pressed", CalibrateJoystick, NULL);
    TXT_SignalConnect(window, "closed", JoystickWindowClosed, NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());

    SetJoystickButtonLabel();
}

