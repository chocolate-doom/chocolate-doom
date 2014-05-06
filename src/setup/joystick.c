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

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "i_joystick.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "textscreen.h"

#include "execute.h"
#include "joystick.h"
#include "mode.h"
#include "txt_joybinput.h"

typedef enum
{
    CALIBRATE_CENTER,
    CALIBRATE_LEFT,
    CALIBRATE_UP,

    // These are only used when defining button axes:
    CALIBRATE_RIGHT,
    CALIBRATE_DOWN,
} calibration_stage_t;

typedef struct
{
    char *name;  // Config file name
    int value;
} joystick_config_t;

typedef struct
{
    char *name;
    int axes, buttons, hats;
    const joystick_config_t *configs;
} known_joystick_t;

// SDL joystick successfully initialized?

static int joystick_initted = 0;

// Joystick enable/disable

static int usejoystick = 0;

// Joystick to use, as an SDL joystick index:

int joystick_index = -1;

// Calibration button. This is the button the user pressed at the
// start of the calibration sequence. They *must* press this button
// for each subsequent sequence.

static int calibrate_button = -1;

// "Bad" joystick axes. Sometimes an axis can be stuck or "bad". An
// example I found is that if you unplug the nunchuck extension from
// a Wii remote, the axes from the nunchuck can be stuck at one of
// the maximum values. These have to be ignored, so when we ask the
// user to center the joystick, we look for bad axes that are not
// close to zero.

static boolean *bad_axis = NULL;

// Which joystick axis to use for horizontal movement, and whether to
// invert the direction:

static int joystick_x_axis = 0;
static int joystick_x_invert = 0;

// Which joystick axis to use for vertical movement, and whether to
// invert the direction:

static int joystick_y_axis = 1;
static int joystick_y_invert = 0;

// Strafe axis.

static int joystick_strafe_axis = -1;
static int joystick_strafe_invert = 0;

// Virtual to physical mapping.
int joystick_physical_buttons[NUM_VIRTUAL_BUTTONS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

static txt_button_t *joystick_button;

//
// Calibration
//

static txt_window_t *calibration_window;
static txt_label_t *calibration_label;
static calibration_stage_t calibrate_stage;
static SDL_Joystick **all_joysticks = NULL;

// Known controllers.
// There are lots of game controllers on the market. Try to configure
// them in a consistent way:
//
// * Use the D-pad rather than an analog stick. left/right turns the
//   player, up/down moves forward/backward - ie. a "traditional"
//   layout like Vanilla Doom rather than something more elaborate.
// * No strafe axis.
// * Fire and run keys together, on the main right-side buttons,
//   ideally arranged so both can be controlled/covered simultaneously
//   with the thumb.
// * Jump/use keys in the same cluster if possible.
// * Strafe left/right configured to map to shoulder buttons if they
//   are present. No "strafe on" key unless shoulder buttons not present.
// * If a second set of shoulder buttons are also present, these map
//   to prev weapon/next weapon.
// * Menu button mapped to start button.
//
// With the common right-side button arrangement that looks like this,
// which is similar to the Vanilla default configuration when using
// a Gravis Gamepad:
//
//    B        A = Fire
//  A   D      B = Jump
//    C        C = Speed
//             D = Use

// Always loaded before others, to get a known starting configuration.
static const joystick_config_t empty_defaults[] =
{
    {"joystick_x_axis",        -1},
    {"joystick_x_invert",      0},
    {"joystick_y_axis",        -1},
    {"joystick_y_invert",      0},
    {"joystick_strafe_axis",   -1},
    {"joystick_strafe_invert", 0},
    {"joyb_fire",              -1},
    {"joyb_use",               -1},
    {"joyb_strafe",            -1},
    {"joyb_speed",             -1},
    {"joyb_strafeleft",        -1},
    {"joyb_straferight",       -1},
    {"joyb_prevweapon",        -1},
    {"joyb_nextweapon",        -1},
    {"joyb_menu_activate",     -1},
    {NULL, 0},
};

static const joystick_config_t ps3_controller[] =
{
    {"joystick_x_axis",        CREATE_BUTTON_AXIS(7, 5)},
    {"joystick_y_axis",        CREATE_BUTTON_AXIS(4, 6)},
    {"joyb_fire",              15},  // Square
    {"joyb_speed",             14},  // X
    {"joyb_use",               13},  // Circle
    {"joyb_jump",              12},  // Triangle
    {"joyb_strafeleft",        8},   // Bottom shoulder buttons
    {"joyb_straferight",       9},
    {"joyb_prevweapon",        10},  // Top shoulder buttons
    {"joyb_nextweapon",        11},
    {"joyb_menu_activate",     3},   // Start
    {NULL, 0},
};

static const joystick_config_t airflo_controller[] =
{
    {"joystick_x_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_HORIZONTAL)},
    {"joystick_y_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_VERTICAL)},
    {"joyb_fire",              2},  // "3"
    {"joyb_speed",             0},  // "1"
    {"joyb_jump",              3},  // "4"
    {"joyb_use",               1},  // "2"
    {"joyb_strafeleft",        6},  // Bottom shoulder buttons
    {"joyb_straferight",       7},
    {"joyb_prevweapon",        4},  // Top shoulder buttons
    {"joyb_nextweapon",        5},
    {"joyb_menu_activate",     9},  // "10", where "Start" usually is.
    {NULL, 0},
};

// Wii controller is weird, so we have to take some liberties.
// Also it's not a HID device, so it won't appear the same everywhere.
// Assume there is no nunchuk or classic controller attached.

// When using WJoy on OS X.
static const joystick_config_t wii_controller_wjoy[] =
{
    {"joystick_x_axis",        CREATE_BUTTON_AXIS(2, 3)},
    {"joystick_y_axis",        CREATE_BUTTON_AXIS(1, 0)},
    {"joyb_fire",              9},  // Button 1
    {"joyb_speed",             10}, // Button 2
    {"joyb_use",               5},  // Button B (trigger)
    {"joyb_prevweapon",        7},  // -
    {"joyb_nextweapon",        6},  // +
    {"joyb_menu_activate",     9},  // Button A
    {NULL, 0},
};

// Xbox 360 controller. Thanks to Brad Harding for the details.
static const joystick_config_t xbox360_controller[] =
{
    {"joystick_x_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_HORIZONTAL)},
    {"joystick_y_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_VERTICAL)},
    {"joystick_strafe_axis",   2},  // Trigger buttons form an axis(???)
    {"joyb_fire",              2},  // X
    {"joyb_speed",             0},  // A
    {"joyb_jump",              3},  // Y
    {"joyb_use",               1},  // B
    {"joyb_prevweapon",        4},  // LB
    {"joyb_nextweapon",        5},  // RB
    {"joyb_menu_activate",     9},  // Start
    {NULL, 0},
};

// Logitech Dual Action (F310, F710). Thanks to Brad Harding for details.
static const joystick_config_t logitech_f310_controller[] =
{
    {"joystick_x_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_HORIZONTAL)},
    {"joystick_y_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_VERTICAL)},
    {"joyb_fire",              0},  // X
    {"joyb_speed",             1},  // A
    {"joyb_jump",              3},  // Y
    {"joyb_use",               2},  // B
    {"joyb_strafeleft",        6},  // LT
    {"joyb_straferight",       7},  // RT
    {"joyb_prevweapon",        4},  // LB
    {"joyb_nextweapon",        5},  // RB
    {"joyb_menu_activate",     11}, // Start
    {NULL, 0},
};

static const known_joystick_t known_joysticks[] =
{
    {
        "PLAYSTATION(R)3 Controller",
        4, 19, 0,
        ps3_controller,
    },

    {
        "AIRFLO             ",
        4, 13, 1,
        airflo_controller,
    },

    {
        "Wiimote (*",  // WJoy includes the Wiimote MAC address.
        6, 26, 0,
        wii_controller_wjoy,
    },

    {
        "Controller (XBOX 360 For Windows)",
        5, 10, 1,
        xbox360_controller,
    },

    {
        "Logitech Dual Action",
        4, 12, 1,
        logitech_f310_controller,
    },
};

static const known_joystick_t *GetJoystickType(int index)
{
    SDL_Joystick *joystick;
    const char *name;
    int axes, buttons, hats;
    int i;

    joystick = all_joysticks[index];
    name = SDL_JoystickName(index);
    axes = SDL_JoystickNumAxes(joystick);
    buttons = SDL_JoystickNumButtons(joystick);
    hats = SDL_JoystickNumHats(joystick);

    for (i = 0; i < arrlen(known_joysticks); ++i)
    {
        // Check for a name match. If the name ends in '*', this means
        // ignore the rest.
        if (M_StringEndsWith(known_joysticks[i].name, "*"))
        {
            if (strncmp(known_joysticks[i].name, name,
                        strlen(known_joysticks[i].name) - 1) != 0)
            {
                continue;
            }
        }
        else
        {
            if (strcmp(known_joysticks[i].name, name) != 0)
            {
                continue;
            }
        }

        if (known_joysticks[i].axes == axes
         && known_joysticks[i].buttons == buttons
         && known_joysticks[i].hats == hats)
        {
            return &known_joysticks[i];
        }
    }

    printf("Unknown joystick '%s' with %i axes, %i buttons, %i hats\n",
           name, axes, buttons, hats);
    printf("Please consider sending in details about your gamepad!\n");

    return NULL;
}

// Query if the joystick at the given index is a known joystick type.
static boolean IsKnownJoystick(int index)
{
    return GetJoystickType(index) != NULL;
}

// Load a configuration set.
static void LoadConfigurationSet(const joystick_config_t *configs)
{
    const joystick_config_t *config;
    char buf[10];
    int button;
    int i;

    button = 0;

    for (i = 0; configs[i].name != NULL; ++i)
    {
        config = &configs[i];

        // Don't overwrite autorun if it is set.
        if (!strcmp(config->name, "joyb_speed") && joybspeed >= 20)
        {
            continue;
        }

        // For buttons, set the virtual button mapping as well.
        if (M_StringStartsWith(config->name, "joyb_") && config->value >= 0)
        {
            joystick_physical_buttons[button] = config->value;
            M_snprintf(buf, sizeof(buf), "%i", button);
            M_SetVariable(config->name, buf);
            ++button;
        }
        else
        {
            M_snprintf(buf, sizeof(buf), "%i", config->value);
            M_SetVariable(config->name, buf);
        }
    }
}

// Load configuration for joystick_index based on known types.
static void LoadKnownConfiguration(void)
{
    const known_joystick_t *jstype;

    jstype = GetJoystickType(joystick_index);
    if (jstype == NULL)
    {
        return;
    }

    LoadConfigurationSet(empty_defaults);
    LoadConfigurationSet(jstype->configs);
}

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
            message = "Move the D-pad or joystick to the\n"
                      "center, and press a button.";
            break;
        case CALIBRATE_UP:
            message = "Push the D-pad or joystick up,\n"
                      "and press the button.";
            break;
        case CALIBRATE_LEFT:
            message = "Push the D-pad or joystick to the\n"
                      "left, and press the button.";
            break;
        case CALIBRATE_DOWN:
            message = "Push the D-pad or joystick down,\n"
                      "and press the button.";
            break;
        case CALIBRATE_RIGHT:
            message = "Push the D-pad or joystick to the\n"
                      "right, and press the button.";
            break;
    }

    TXT_SetLabel(calibration_label, message);
}

// Search all axes on joystick being configured; find a button that is
// pressed (other than the calibrate button). Returns the button number.

static int FindPressedAxisButton(void)
{
    SDL_Joystick *joystick;
    int i;

    joystick = all_joysticks[joystick_index];

    for (i = 0; i < SDL_JoystickNumButtons(joystick); ++i)
    {
        if (i == calibrate_button)
        {
            continue;
        }

        if (SDL_JoystickGetButton(joystick, i))
        {
            return i;
        }
    }

    return -1;
}

// Look for a hat that isn't centered. Returns the encoded hat axis.

static int FindUncenteredHat(int *axis_invert)
{
    SDL_Joystick *joystick;
    int i, hatval;

    joystick = all_joysticks[joystick_index];

    for (i = 0; i < SDL_JoystickNumHats(joystick); ++i)
    {
        hatval = SDL_JoystickGetHat(joystick, i);

        switch (hatval)
        {
            case SDL_HAT_LEFT:
            case SDL_HAT_RIGHT:
                *axis_invert = hatval != SDL_HAT_LEFT;
                return CREATE_HAT_AXIS(i, HAT_AXIS_HORIZONTAL);

            case SDL_HAT_UP:
            case SDL_HAT_DOWN:
                *axis_invert = hatval != SDL_HAT_UP;
                return CREATE_HAT_AXIS(i, HAT_AXIS_VERTICAL);

            // If the hat is centered, or is not pointing in a
            // definite direction, then ignore it. We don't accept
            // the hat being pointed to the upper-left for example,
            // because it's ambiguous.
            case SDL_HAT_CENTERED:
            default:
                break;
        }
    }

    // None found.
    return -1;
}

static boolean CalibrateAxis(int *axis_index, int *axis_invert)
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
        if (bad_axis[i])
        {
            continue;
        }

        axis_value = SDL_JoystickGetAxis(joystick, i);

        if (abs(axis_value) > best_value)
        {
            best_value = abs(axis_value);
            best_invert = axis_value > 0;
            best_axis = i;
        }
    }

    // Did we find one axis that had a significant value?

    if (best_value > 32768 / 4)
    {
        // Save the best values we have found

        *axis_index = best_axis;
        *axis_invert = best_invert;
        return true;
    }

    // Otherwise, maybe this is a "button axis", like the PS3 SIXAXIS
    // controller that exposes the D-pad as four individual buttons.
    // Search for a button.

    i = FindPressedAxisButton();

    if (i >= 0)
    {
        *axis_index = CREATE_BUTTON_AXIS(i, 0);
        *axis_invert = 0;
        return true;
    }

    // Maybe it's a D-pad that is presented as a hat. This sounds weird
    // but gamepads like this really do exist; an example is the
    // Nyko AIRFLO Ex.

    i = FindUncenteredHat(axis_invert);

    if (i >= 0)
    {
        *axis_index = i;
        return true;
    }

    // User pressed the button without pushing the joystick anywhere.
    return false;
}

static boolean SetButtonAxisPositive(int *axis_index)
{
    int button;

    button = FindPressedAxisButton();

    if (button >= 0)
    {
        *axis_index |= CREATE_BUTTON_AXIS(0, button);
        return true;
    }

    return false;
}

static int NextCalibrateStage(void)
{
    switch (calibrate_stage)
    {
        case CALIBRATE_CENTER:
            return CALIBRATE_LEFT;

        // After pushing to the left, there are two possibilities:
        // either it is a button axis, in which case we need to find
        // the other button, or we can just move on to the next axis.
        case CALIBRATE_LEFT:
            if (IS_BUTTON_AXIS(joystick_x_axis))
            {
                return CALIBRATE_RIGHT;
            }
            else
            {
                return CALIBRATE_UP;
            }

        case CALIBRATE_RIGHT:
            return CALIBRATE_UP;

        case CALIBRATE_UP:
            if (IS_BUTTON_AXIS(joystick_y_axis))
            {
                return CALIBRATE_DOWN;
            }
            else
            {
                // Finished.
                return CALIBRATE_CENTER;
            }

        case CALIBRATE_DOWN:
            // Finished.
            return CALIBRATE_CENTER;
    }
}

static void IdentifyBadAxes(void)
{
    SDL_Joystick *joystick;
    int i, val;

    free(bad_axis);

    joystick = all_joysticks[joystick_index];
    bad_axis = calloc(SDL_JoystickNumAxes(joystick), sizeof(boolean));

    // Look for uncentered axes.

    for (i = 0; i < SDL_JoystickNumAxes(joystick); ++i)
    {
        val = SDL_JoystickGetAxis(joystick, i);

        bad_axis[i] = abs(val) > (32768 / 5);

        if (bad_axis[i])
        {
            printf("Ignoring uncentered joystick axis #%i: %i\n", i, val);
        }
    }
}

static int CalibrationEventCallback(SDL_Event *event, void *user_data)
{
    boolean advance;

    if (event->type != SDL_JOYBUTTONDOWN)
    {
        return 0;
    }

    // At this point, we have a button press.
    // In the first "center" stage, we're just trying to work out which
    // joystick is being configured and which button the user is pressing.
    if (calibrate_stage == CALIBRATE_CENTER)
    {
        joystick_index = event->jbutton.which;
        calibrate_button = event->jbutton.button;
        IdentifyBadAxes();

        // If the joystick is a known one, auto-load default
        // config for it.
        if (IsKnownJoystick(joystick_index))
        {
            LoadKnownConfiguration();
            usejoystick = 1;
            TXT_CloseWindow(calibration_window);
        }
        else
        {
            // Advance to next stage.
            calibrate_stage = CALIBRATE_LEFT;
            SetCalibrationLabel();
        }

        return 1;
    }

    // In subsequent stages, the user is asked to push in a specific
    // direction and press the button. They must push the same button
    // as they did before; this is necessary to support button axes.
    if (event->jbutton.which == joystick_index
     && event->jbutton.button == calibrate_button)
    {
        switch (calibrate_stage)
        {
            default:
            case CALIBRATE_LEFT:
                advance = CalibrateAxis(&joystick_x_axis, &joystick_x_invert);
                break;

            case CALIBRATE_RIGHT:
                advance = SetButtonAxisPositive(&joystick_x_axis);
                break;

            case CALIBRATE_UP:
                advance = CalibrateAxis(&joystick_y_axis, &joystick_y_invert);
                break;

            case CALIBRATE_DOWN:
                advance = SetButtonAxisPositive(&joystick_y_axis);
                break;
        }

        // Advance to the next calibration stage?

        if (advance)
        {
            calibrate_stage = NextCalibrateStage();
            SetCalibrationLabel();

            // Finished?
            if (calibrate_stage == CALIBRATE_CENTER)
            {
                usejoystick = 1;
                TXT_CloseWindow(calibration_window);
            }

            return 1;
        }
    }

    return 0;
}

static void NoJoystick(void)
{
    TXT_MessageBox(NULL, "No joysticks or gamepads could be found.\n\n"
                         "Try configuring your controller from within\n"
                         "your OS first. Maybe you need to install\n"
                         "some drivers or otherwise configure it.");

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

    calibration_window = TXT_NewWindow("Gamepad/Joystick calibration");

    TXT_AddWidgets(calibration_window,
                   TXT_NewLabel("Please follow the following instructions\n"
                                "in order to calibrate your controller."),
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

    window = TXT_NewWindow("Gamepad/Joystick configuration");

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable gamepad/joystick", &usejoystick),
                   joystick_table = TXT_NewTable(2),
                   TXT_NewSeparator("Buttons"),
                   button_table = TXT_NewTable(2),
                   NULL);

    TXT_SetColumnWidths(joystick_table, 20, 15);

    TXT_AddWidgets(joystick_table,
                   TXT_NewLabel("Current controller"),
                   joystick_button = TXT_NewButton("zzzz"),
                   NULL);

    TXT_SetColumnWidths(button_table, 20, 15);

    AddJoystickControl(button_table, "Fire/Attack", &joybfire);
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

    if (gamemission == hexen || gamemission == strife)
    {
        AddJoystickControl(button_table, "Jump", &joybjump);
    }

    AddJoystickControl(button_table, "Activate menu", &joybmenu);

    TXT_SignalConnect(joystick_button, "pressed", CalibrateJoystick, NULL);
    TXT_SignalConnect(window, "closed", JoystickWindowClosed, NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());

    SetJoystickButtonLabel();
}

void BindJoystickVariables(void)
{
    int i;

    M_BindVariable("use_joystick",          &usejoystick);
    M_BindVariable("joystick_index",        &joystick_index);
    M_BindVariable("joystick_x_axis",       &joystick_x_axis);
    M_BindVariable("joystick_y_axis",       &joystick_y_axis);
    M_BindVariable("joystick_strafe_axis",  &joystick_strafe_axis);
    M_BindVariable("joystick_x_invert",     &joystick_x_invert);
    M_BindVariable("joystick_y_invert",     &joystick_y_invert);
    M_BindVariable("joystick_strafe_invert",&joystick_strafe_invert);

    for (i = 0; i < NUM_VIRTUAL_BUTTONS; ++i)
    {
        char name[32];
        M_snprintf(name, sizeof(name), "joystick_physical_button%i", i);
        M_BindVariable(name, &joystick_physical_buttons[i]);
    }
}

