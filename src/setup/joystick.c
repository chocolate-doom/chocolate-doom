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
#include <string.h>

#include "doomtype.h"
#include "i_joystick.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "textscreen.h"

#include "execute.h"
#include "joystick.h"
#include "mode.h"
#include "txt_joyaxis.h"
#include "txt_joybinput.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-gamepad"

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

// GUID and index of joystick to use.

char *joystick_guid = "";
int joystick_index = -1;

// Calibration button. This is the button the user pressed at the
// start of the calibration sequence. They *must* press this button
// for each subsequent sequence.

static int calibrate_button = -1;

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

// Look axis.

static int joystick_look_axis = -1;
static int joystick_look_invert = 0;

// Virtual to physical mapping.
int joystick_physical_buttons[NUM_VIRTUAL_BUTTONS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

static txt_button_t *joystick_button;
static txt_joystick_axis_t *x_axis_widget;
static txt_joystick_axis_t *y_axis_widget;

//
// Calibration
//

static txt_window_t *calibration_window;
static SDL_Joystick **all_joysticks = NULL;
static int all_joysticks_len = 0;

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
    {"joystick_x_axis",            -1},
    {"joystick_x_invert",          0},
    {"joystick_y_axis",            -1},
    {"joystick_y_invert",          0},
    {"joystick_strafe_axis",       -1},
    {"joystick_strafe_invert",     0},
    {"joystick_look_axis",         -1},
    {"joystick_look_invert",       0},
    {"joyb_fire",                  -1},
    {"joyb_use",                   -1},
    {"joyb_strafe",                -1},
    {"joyb_speed",                 -1},
    {"joyb_strafeleft",            -1},
    {"joyb_straferight",           -1},
    {"joyb_prevweapon",            -1},
    {"joyb_nextweapon",            -1},
    {"joyb_jump",                  -1},
    {"joyb_menu_activate",         -1},
    {"joyb_toggle_automap",        -1},
    {"joystick_physical_button0",  0},
    {"joystick_physical_button1",  1},
    {"joystick_physical_button2",  2},
    {"joystick_physical_button3",  3},
    {"joystick_physical_button4",  4},
    {"joystick_physical_button5",  5},
    {"joystick_physical_button6",  6},
    {"joystick_physical_button7",  7},
    {"joystick_physical_button8",  8},
    {"joystick_physical_button9",  9},
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

// Playstation 4 Dual Shock 4 (DS4)
static const joystick_config_t ps4_ds4_controller[] =
{
    {"joystick_x_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_HORIZONTAL)},
    {"joystick_y_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_VERTICAL)},
    {"joyb_fire",              0},  // Square
    {"joyb_speed",             1},  // X
    {"joyb_use",               2},  // Circle
    {"joyb_jump",              3},  // Triangle
    {"joyb_strafeleft",        6},  // Bottom shoulder buttons
    {"joyb_straferight",       7},
    {"joyb_prevweapon",        4},  // Top shoulder buttons
    {"joyb_nextweapon",        5},
    {"joyb_menu_activate",     12}, // Playstation logo button
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

// Xbox 360 controller under Linux.
static const joystick_config_t xbox360_controller_linux[] =
{
    {"joystick_x_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_HORIZONTAL)},
    {"joystick_y_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_VERTICAL)},
    // Ideally we'd like the trigger buttons to be strafe left/right
    // But Linux presents each trigger button as its own axis, which
    // we can't really work with. So we have to settle for a
    // suboptimal setup.
    {"joyb_fire",              2},  // X
    {"joyb_speed",             0},  // A
    {"joyb_jump",              3},  // Y
    {"joyb_use",               1},  // B
    {"joyb_strafeleft",        4},  // LB
    {"joyb_straferight",       5},  // RB
    {"joyb_menu_activate",     7},  // Start
    {"joyb_prevweapon",        6},  // Back
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

// Multilaser JS030 gamepad, similar to a PS2 controller.
static const joystick_config_t multilaser_js030_controller[] =
{
    {"joystick_x_axis",        0},   // Left stick / D-pad
    {"joystick_y_axis",        1},
    {"joyb_fire",              3},   // Square
    {"joyb_speed",             2},   // X
    {"joyb_use",               1},   // Circle
    {"joyb_jump",              0},   // Triangle
    {"joyb_strafeleft",        6},   // Bottom shoulder buttons
    {"joyb_straferight",       7},
    {"joyb_prevweapon",        4},   // Top shoulder buttons
    {"joyb_nextweapon",        5},
    {"joyb_menu_activate",     9},   // Start
    {NULL, 0},
};

// Buffalo Classic USB Gamepad (thanks Fabian Greffrath).
static const joystick_config_t buffalo_classic_controller[] =
{
    {"joystick_x_axis",        0},
    {"joystick_y_axis",        1},
    {"joyb_use",               0},    // A
    {"joyb_speed",             1},    // B
    {"joyb_jump",              2},    // X
    {"joyb_fire",              3},    // Y
    {"joyb_strafeleft",        4},    // Left shoulder
    {"joyb_straferight",       5},    // Right shoulder
    {"joyb_prevweapon",        6},    // Select
    {"joyb_menu_activate",     7},    // Start
    {NULL, 0},
};

// Config for if the user is actually using an old PC joystick or gamepad,
// probably via a USB-Gameport adapter.
static const joystick_config_t pc_gameport_controller[] =
{
    {"joystick_x_axis",        0},
    {"joystick_y_axis",        1},
    // Button configuration is the default as used for Vanilla Doom,
    // Heretic and Hexen. When playing with a Gravis Gamepad, this
    // layout should also be vaguely similar to the standard layout
    // described above.
    {"joyb_fire",              0},
    {"joyb_strafe",            1},
    {"joyb_use",               3},
    {"joyb_speed",             2},
    {NULL, 0},
};

// http://www.8bitdo.com/nes30pro/ and http://www.8bitdo.com/fc30pro/
static const joystick_config_t nes30_pro_controller[] =
{
    {"joystick_x_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_HORIZONTAL)},
    {"joystick_y_axis",        CREATE_HAT_AXIS(0, HAT_AXIS_VERTICAL)},
    {"joyb_fire",              4},  // Y
    {"joyb_speed",             1},  // B
    {"joyb_jump",              3},  // X
    {"joyb_use",               0},  // A
    {"joyb_strafeleft",        6},  // L1
    {"joyb_straferight",       7}, // R1
    {"joyb_prevweapon",        8},  // L2
    {"joyb_nextweapon",        9},  // R2
    {"joyb_menu_activate",     11}, // Start
    {"joyb_toggle_automap",    10}, // Select
    {NULL, 0},
};

// http://www.8bitdo.com/sfc30/ or http://www.8bitdo.com/snes30/
// and http://www.nes30.com/ and http://www.fc30.com/
static const joystick_config_t sfc30_controller[] =
{
    {"joystick_x_axis",        0},
    {"joystick_y_axis",        1},
    {"joyb_fire",              4}, // Y
    {"joyb_speed",             1}, // B
    {"joyb_jump",              3}, // X
    {"joyb_use",               0}, // A
    {"joyb_strafeleft",        6}, // L
    {"joyb_straferight",       7}, // R
    {"joyb_menu_activate",    11}, // Start
    {"joyb_toggle_automap",   10}, // Select
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
        "Controller (XBOX One For Windows)",
        5, 10, 1,
        xbox360_controller,
    },

    // Xbox 360 controller as it appears on Linux.
    {
        "Microsoft X-Box 360 pad",
        6, 11, 1,
        xbox360_controller_linux,
    },

    // Xbox One controller as it appears on Linux.
    {
        "Microsoft X-Box One pad",
        6, 11, 1,
        xbox360_controller_linux,
    },

    {
        "Logitech Dual Action",
        4, 12, 1,
        logitech_f310_controller,
    },

    {
        "USB Vibration Joystick",
        4, 12, 1,
        multilaser_js030_controller,
    },

    {
        "USB,2-axis 8-button gamepad  ",
        2, 8, 0,
        buffalo_classic_controller,
    },

    // PS4 controller just appears as the generic-sounding "Wireless
    // Controller". Hopefully the number of buttons/axes/hats should be
    // enough to distinguish it from other gamepads.
    {
        "Wireless Controller",
        6, 14, 1,
        ps4_ds4_controller,
    },

    // This is the configuration for the USB-Gameport adapter listed on
    // this page as the "Mayflash USB to Gameport Adapter" (though the
    // device is labeled as "Super Joy Box 7"):
    // https://sites.google.com/site/joystickrehab/itemcatal
    // TODO: Add extra configurations here for other USB-Gameport adapters,
    // which should just be the same configuration.
    {
        "WiseGroup.,Ltd Gameport to USB Controller",
        4, 8, 1,
        pc_gameport_controller,
    },

    // How the Super Joy Box 7 appears on Mac OS X.
    {
        "Gameport to USB Controller",
        2, 8, 1,
        pc_gameport_controller,
    },

    // 8Bitdo NES30 Pro, http://www.8bitdo.com/nes30pro/
    {
        "8Bitdo NES30 Pro",
        4, 16, 1,
        nes30_pro_controller,
    },

    // the above, NES variant, via USB on Linux/Raspbian (odd values)
    {
        "8Bitdo NES30 Pro*",
        6, 15, 1,
        nes30_pro_controller,
    },

    // the above, NES variant, connected over bluetooth
    {
        "8Bitdo NES30 Pro",
        6, 16, 1,
        nes30_pro_controller,
    },

    // 8bitdo NES30 Pro, in joystick mode (R1+Power), swaps the D-Pad
    // and analog stick inputs.  Only applicable over Bluetooth. On USB,
    // this mode registers the device as an Xbox 360 pad.
    {
        "8Bitdo NES30 Pro Joystick",
        6, 16, 1,
        nes30_pro_controller,
    },

    // variant of the above, via USB on Mac
    // Note: untested, but theorized to exist based on us comparing
    // a NES30 Pro tested on Linux with a FC30 Pro tested with Mac & Linux
    {
        "8Bitdo NES30 Pro",
        4, 15, 1,
        nes30_pro_controller,
    },


    // 8Bitdo FC30 Pro, http://8bitdo.cn/fc30pro/
    // connected over bluetooth
    {
        "8Bitdo FC30 Pro",
        4, 16, 1,
        nes30_pro_controller,
    },

    // variant of the above, via USB on Linux/Raspbian
    {
        "8Bitdo FC30 Pro*",
        6, 15, 1,
        nes30_pro_controller,
    },

    // variant of the above, Linux/bluetooth
    {
        "8Bitdo FC30 Pro",
	6, 16, 1,
	nes30_pro_controller,
    },

    // variant of the above, via USB on Mac
    {
        "8Bitdo FC30 Pro",
        4, 15, 1,
        nes30_pro_controller,
    },

    // 8Bitdo SFC30 SNES replica controller
    // in default mode and in controller mode (Start+R)
    // the latter suffixes "Joystick" to the name
    // http://www.8bitdo.com/sfc30/
    {
        "8Bitdo SFC30 GamePad*",
        4, 16, 1,
        sfc30_controller,
    },

    // As above, but as detected on RHEL Linux (odd extra axes)
    {
        "8Bitdo SFC30 GamePad*",
        6, 16, 1,
        sfc30_controller,
    },

    // SNES30 colour variation of the above
    // http://www.8bitdo.com/snes30/
    {
        "8Bitdo SNES30 GamePad*",
        4, 16, 1,
        sfc30_controller,
    },

    // 8Bitdo SFC30 SNES replica controller in USB controller mode
    // tested with firmware V2.68 (Beta); latest stable V2.65 doesn't work on
    // OS X in USB controller mode
    // Names seen so far:
    //     'SFC30 Joystick' (OS X)
    //     'SFC30              SFC30 Joystick' (Fedora 24; RHEL7)
    // XXX: there is probably a SNES30 variant of this too
    {
        "SFC30 *",
        4, 12, 1,
        sfc30_controller,
    },

    // NES30 (not pro), tested in default and "hold R whilst turning on"
    // mode, with whatever firmware it came with out of the box. Latter
    // mode puts " Joystick" suffix on the name string
    {
        "8Bitdo NES30 GamePad*",
        4, 16, 1,
        sfc30_controller, // identical to SFC30
    },
    // FC30 variant of the above
    {
        "8Bitdo FC30 GamePad*",
        4, 16, 1,
        sfc30_controller, // identical to SFC30
    },

    // NES30 in USB mode
    {
        "NES30*",
        4, 12, 1,
        sfc30_controller, // identical to SFC30
    },
    // FC30 variant of the above
    {
        "FC30*",
        4, 12, 1,
        sfc30_controller, // identical to SFC30
    },
};

static const known_joystick_t *GetJoystickType(int index)
{
    SDL_Joystick *joystick;
    const char *name;
    int axes, buttons, hats;
    int i;

    joystick = all_joysticks[index];
    name = SDL_JoystickName(joystick);
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

static void InitJoystick(void)
{
    if (!joystick_initted)
    {
        joystick_initted = SDL_Init(SDL_INIT_JOYSTICK) >= 0;
    }
}

static void UnInitJoystick(void)
{
    if (joystick_initted)
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        joystick_initted = 0;
    }
}

// We identify joysticks using GUID where possible, but joystick_index
// is used to distinguish between different devices. As the index can
// change, UpdateJoystickIndex() checks to see if it is still valid and
// updates it as appropriate.
static void UpdateJoystickIndex(void)
{
    SDL_JoystickGUID guid, dev_guid;
    int i;

    guid = SDL_JoystickGetGUIDFromString(joystick_guid);

    // Is joystick_index already correct?
    if (joystick_index >= 0 && joystick_index < SDL_NumJoysticks())
    {
        dev_guid = SDL_JoystickGetDeviceGUID(joystick_index);
        if (!memcmp(&guid, &dev_guid, sizeof(SDL_JoystickGUID)))
        {
            return;
        }
    }

    // If index is not correct, look for the first device with the
    // expected GUID. It may have moved to a different index.
    for (i = 0; i < SDL_NumJoysticks(); ++i)
    {
        dev_guid = SDL_JoystickGetDeviceGUID(i);
        if (!memcmp(&guid, &dev_guid, sizeof(SDL_JoystickGUID)))
        {
            joystick_index = i;
            return;
        }
    }

    // Not found; it's possible the device is disconnected. Do not
    // reset joystick_guid or joystick_index in case they are
    // reconnected later.
}

// Set the label showing the name of the currently selected joystick
static void SetJoystickButtonLabel(void)
{
    SDL_JoystickGUID guid, dev_guid;
    const char *name;

    if (!usejoystick || !strcmp(joystick_guid, ""))
    {
        name = "None set";
    }
    else
    {
        name = "Not found (device disconnected?)";

        // Use the device name if the GUID and index match.
        if (joystick_index >= 0 && joystick_index < SDL_NumJoysticks())
        {
            guid = SDL_JoystickGetGUIDFromString(joystick_guid);
            dev_guid = SDL_JoystickGetDeviceGUID(joystick_index);
            if (!memcmp(&guid, &dev_guid, sizeof(SDL_JoystickGUID)))
            {
                name = SDL_JoystickNameForIndex(joystick_index);
            }
        }
    }

    TXT_SetButtonLabel(joystick_button, (char *) name);
}

// Try to open all joysticks visible to SDL.

static int OpenAllJoysticks(void)
{
    int i;
    int result;

    InitJoystick();

    // SDL_JoystickOpen() all joysticks.

    all_joysticks_len = SDL_NumJoysticks();
    all_joysticks = calloc(all_joysticks_len, sizeof(SDL_Joystick *));

    result = 0;

    for (i = 0; i < all_joysticks_len; ++i)
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

    for (i = 0; i < all_joysticks_len; ++i)
    {
        if (all_joysticks[i] != NULL)
        {
            SDL_JoystickClose(all_joysticks[i]);
        }
    }

    SDL_JoystickEventState(SDL_DISABLE);

    free(all_joysticks);
    all_joysticks = NULL;

    UnInitJoystick();
}

static void CalibrateXAxis(void)
{
    TXT_ConfigureJoystickAxis(x_axis_widget, calibrate_button, NULL);
}

// Given the SDL_JoystickID instance ID from a button event, set the
// joystick_guid and joystick_index config variables.
static boolean SetJoystickGUID(SDL_JoystickID joy_id)
{
    SDL_JoystickGUID guid;
    int i;

    for (i = 0; i < all_joysticks_len; ++i)
    {
        if (SDL_JoystickInstanceID(all_joysticks[i]) == joy_id)
        {
            guid = SDL_JoystickGetGUID(all_joysticks[i]);
            joystick_guid = malloc(33);
            SDL_JoystickGetGUIDString(guid, joystick_guid, 33);
            joystick_index = i;
            return true;
        }
    }

    return false;
}

static int CalibrationEventCallback(SDL_Event *event, void *user_data)
{
    if (event->type != SDL_JOYBUTTONDOWN)
    {
        return 0;
    }

    if (!SetJoystickGUID(event->jbutton.which))
    {
        return 0;
    }

    // At this point, we have a button press.
    // In the first "center" stage, we're just trying to work out which
    // joystick is being configured and which button the user is pressing.
    usejoystick = 1;
    calibrate_button = event->jbutton.button;

    // If the joystick is a known one, auto-load default
    // config for it. Otherwise, proceed with calibration.
    if (IsKnownJoystick(joystick_index))
    {
        LoadKnownConfiguration();
        TXT_CloseWindow(calibration_window);
    }
    else
    {
        TXT_CloseWindow(calibration_window);

        // Calibrate joystick axes: Y axis first, then X axis once
        // completed.
        TXT_ConfigureJoystickAxis(y_axis_widget, calibrate_button,
                                  CalibrateXAxis);
    }

    return 1;
}

static void NoJoystick(void)
{
    TXT_MessageBox(NULL, "No gamepads or joysticks could be found.\n\n"
                         "Try configuring your controller from within\n"
                         "your OS first. Maybe you need to install\n"
                         "some drivers or otherwise configure it.");

    usejoystick = 0;
    joystick_index = -1;
    SetJoystickButtonLabel();
}

static void CalibrateWindowClosed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    TXT_SDL_SetEventCallback(NULL, NULL);
    SetJoystickButtonLabel();
    CloseAllJoysticks();
}

static void CalibrateJoystick(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    // Try to open all available joysticks.  If none are opened successfully,
    // bomb out with an error.

    if (!OpenAllJoysticks())
    {
        NoJoystick();
        return;
    }

    calibration_window = TXT_NewWindow("Gamepad/Joystick calibration");

    TXT_AddWidgets(calibration_window,
                   TXT_NewStrut(0, 1),
                   TXT_NewLabel("Center the D-pad or joystick,\n"
                                "and press a button."),
                   TXT_NewStrut(0, 1),
                   NULL);

    TXT_SetWindowAction(calibration_window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(calibration_window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(calibration_window));
    TXT_SetWindowAction(calibration_window, TXT_HORIZ_RIGHT, NULL);

    TXT_SDL_SetEventCallback(CalibrationEventCallback, NULL);

    TXT_SignalConnect(calibration_window, "closed", CalibrateWindowClosed, NULL);

    // Start calibration
    usejoystick = 0;
    joystick_index = -1;
}

//
// GUI
//

static void AddJoystickControl(TXT_UNCAST_ARG(table), char *label, int *var)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_joystick_input_t *joy_input;

    joy_input = TXT_NewJoystickInput(var);

    TXT_AddWidgets(table,
                   TXT_NewLabel(label),
                   joy_input,
                   TXT_TABLE_EMPTY,
                   NULL);
}

void ConfigJoystick(void)
{
    txt_window_t *window;

    window = TXT_NewWindow("Gamepad/Joystick configuration");
    TXT_SetTableColumns(window, 6);
    TXT_SetColumnWidths(window, 18, 10, 1, 15, 10, 0);
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewLabel("Controller"),
                   joystick_button = TXT_NewButton("zzzz"),
                   TXT_TABLE_EOL,

                   TXT_NewSeparator("Axes"),
                   TXT_NewLabel("Forward/backward"),
                   y_axis_widget = TXT_NewJoystickAxis(&joystick_y_axis,
                                                       &joystick_y_invert,
                                                       JOYSTICK_AXIS_VERTICAL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
                   TXT_TABLE_EMPTY,

                   TXT_NewLabel("Turn left/right"),
                   x_axis_widget =
                        TXT_NewJoystickAxis(&joystick_x_axis,
                                            &joystick_x_invert,
                                            JOYSTICK_AXIS_HORIZONTAL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
                   TXT_TABLE_EMPTY,

                   TXT_NewLabel("Strafe left/right"),
                   TXT_NewJoystickAxis(&joystick_strafe_axis,
                                       &joystick_strafe_invert,
                                        JOYSTICK_AXIS_HORIZONTAL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
                   TXT_TABLE_EMPTY,
                   NULL);

    if (gamemission == heretic || gamemission == hexen || gamemission == strife)
    {
        TXT_AddWidgets(window,
                   TXT_NewLabel("Look up/down"),
                   TXT_NewJoystickAxis(&joystick_look_axis,
                                       &joystick_look_invert,
                                        JOYSTICK_AXIS_VERTICAL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
                   TXT_TABLE_EMPTY,
                   NULL);
    }

    TXT_AddWidget(window, TXT_NewSeparator("Buttons"));

    AddJoystickControl(window, "Fire/Attack", &joybfire);
    AddJoystickControl(window, "Strafe Left", &joybstrafeleft);

    AddJoystickControl(window, "Use", &joybuse);
    AddJoystickControl(window, "Strafe Right", &joybstraferight);

    AddJoystickControl(window, "Previous weapon", &joybprevweapon);
    AddJoystickControl(window, "Strafe", &joybstrafe);

    AddJoystickControl(window, "Next weapon", &joybnextweapon);

    // High values of joybspeed are used to activate the "always run mode"
    // trick in Vanilla Doom.  If this has been enabled, not only is the
    // joybspeed value meaningless, but the control itself is useless.

    if (joybspeed < 20)
    {
        AddJoystickControl(window, "Speed", &joybspeed);
    }

    if (gamemission == hexen || gamemission == strife)
    {
        AddJoystickControl(window, "Jump", &joybjump);
    }

    AddJoystickControl(window, "Activate menu", &joybmenu);

    AddJoystickControl(window, "Toggle Automap", &joybautomap);

    TXT_SignalConnect(joystick_button, "pressed", CalibrateJoystick, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());

    InitJoystick();
    UpdateJoystickIndex();
    SetJoystickButtonLabel();
    UnInitJoystick();
}

void BindJoystickVariables(void)
{
    int i;

    M_BindIntVariable("use_joystick",           &usejoystick);
    M_BindStringVariable("joystick_guid",       &joystick_guid);
    M_BindIntVariable("joystick_index",         &joystick_index);
    M_BindIntVariable("joystick_x_axis",        &joystick_x_axis);
    M_BindIntVariable("joystick_y_axis",        &joystick_y_axis);
    M_BindIntVariable("joystick_strafe_axis",   &joystick_strafe_axis);
    M_BindIntVariable("joystick_x_invert",      &joystick_x_invert);
    M_BindIntVariable("joystick_y_invert",      &joystick_y_invert);
    M_BindIntVariable("joystick_strafe_invert", &joystick_strafe_invert);
    M_BindIntVariable("joystick_look_axis",   &joystick_look_axis);
    M_BindIntVariable("joystick_look_invert", &joystick_look_invert);

    for (i = 0; i < NUM_VIRTUAL_BUTTONS; ++i)
    {
        char name[32];
        M_snprintf(name, sizeof(name), "joystick_physical_button%i", i);
        M_BindIntVariable(name, &joystick_physical_buttons[i]);
    }
}

