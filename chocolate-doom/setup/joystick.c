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

typedef enum
{
    CALIBRATE_CENTER,
    CALIBRATE_LEFT,
    CALIBRATE_UP,
} calibration_stage_t;

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

//
// Calibration 
//

static txt_window_t *calibration_window;
static txt_label_t *calibration_label;
static calibration_stage_t calibrate_stage;
static SDL_Joystick **all_joysticks = NULL;

// Try to open all joysticks visible to SDL.

static int OpenAllJoysticks(void)
{
    int i;
    int num_joysticks;
    int result;

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

    if (!result)
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
                      "center, and press fire.";
            break;
        case CALIBRATE_UP:
            message = "Move the joystick up,\n"
                      "and press fire.";
            break;
        case CALIBRATE_LEFT:
            message = "Move the joystick to the\n"
                      "left, and press fire.";
            break;
    }

    TXT_SetLabel(calibration_label, message);
}

static int CalibrationEventCallback(SDL_Event *event, void *user_data)
{
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
}

static void CalibrateWindowClosed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    CloseAllJoysticks();
    TXT_SDL_SetEventCallback(NULL, NULL);
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

    TXT_SetWidgetAlign(calibration_label, TXT_HORIZ_CENTER);
    SetCalibrationLabel();
    TXT_SDL_SetEventCallback(CalibrationEventCallback, NULL);

    TXT_SignalConnect(calibration_window, "closed", CalibrateWindowClosed, NULL);
}


// 
// GUI
//

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

    TXT_SignalConnect(joystick_button, "pressed", CalibrateJoystick, NULL);

    SetJoystickButtonLabel();
}

