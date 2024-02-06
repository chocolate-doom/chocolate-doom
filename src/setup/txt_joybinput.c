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

#include "SDL_joystick.h"
#include "SDL_gamecontroller.h"

#include "doomkeys.h"
#include "joystick.h"
#include "i_joystick.h"
#include "i_system.h"
#include "m_controls.h"
#include "m_misc.h"

#include "txt_joybinput.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_label.h"
#include "txt_sdl.h"
#include "txt_utf8.h"
#include "txt_window.h"

#define JOYSTICK_INPUT_WIDTH 10


// Joystick button variables.
// The ordering of this array is important. We will always try to map
// each variable to the virtual button with the same array index. For
// example: joybfire should always be 0, and then we change
// joystick_physical_buttons[0] to point to the physical joystick
// button that the user wants to use for firing. We do this so that
// the menus work (the game code is hard coded to interpret
// button #0 = select menu item, button #1 = go back to previous menu).
static int *all_joystick_buttons[NUM_VIRTUAL_BUTTONS] =
{
    &joybfire,
    &joybuse,
    &joybstrafe,
    &joybspeed,
    &joybstrafeleft,
    &joybstraferight,
    &joybprevweapon,
    &joybnextweapon,
    &joybjump,
    &joybmenu,
    &joybautomap,
    &joybuseartifact,
    &joybinvleft,
    &joybinvright,
    &joybflyup,
    &joybflydown,
    &joybflycenter,
};

// For indirection so that we're not dependent on item ordering in the
// SDL_GameControllerButton enum.
static const int gamepad_buttons[GAMEPAD_BUTTON_MAX] =
{
   SDL_CONTROLLER_BUTTON_A,
   SDL_CONTROLLER_BUTTON_B,
   SDL_CONTROLLER_BUTTON_X,
   SDL_CONTROLLER_BUTTON_Y,
   SDL_CONTROLLER_BUTTON_BACK,
   SDL_CONTROLLER_BUTTON_GUIDE,
   SDL_CONTROLLER_BUTTON_START,
   SDL_CONTROLLER_BUTTON_LEFTSTICK,
   SDL_CONTROLLER_BUTTON_RIGHTSTICK,
   SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
   SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
   SDL_CONTROLLER_BUTTON_DPAD_UP,
   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
   SDL_CONTROLLER_BUTTON_DPAD_LEFT,
   SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
   SDL_CONTROLLER_BUTTON_MISC1,
   SDL_CONTROLLER_BUTTON_PADDLE1,
   SDL_CONTROLLER_BUTTON_PADDLE2,
   SDL_CONTROLLER_BUTTON_PADDLE3,
   SDL_CONTROLLER_BUTTON_PADDLE4,
   SDL_CONTROLLER_BUTTON_TOUCHPAD,
   GAMEPAD_BUTTON_TRIGGERLEFT,
   GAMEPAD_BUTTON_TRIGGERRIGHT,
};

// Items in the following button lists are ordered according to gamepad_buttons
// above.
static const char *xbox360_buttons[GAMEPAD_BUTTON_MAX] =
{
    "A",
    "B",
    "X",
    "Y",
    "BACK",
    "GUIDE",
    "START",
    "LSB",
    "RSB",
    "LB",
    "RB",
    "DPAD U",
    "DPAD D",
    "DPAD L",
    "DPAD R",
    "",
    "",
    "",
    "",
    "",
    "",
    "LT",
    "RT",
};

static const char *xboxone_buttons[GAMEPAD_BUTTON_MAX] =
{
    "A",
    "B",
    "X",
    "Y",
    "VIEW",
    "XBOX",
    "MENU",
    "LSB",
    "RSB",
    "LB",
    "RB",
    "DPAD U",
    "DPAD D",
    "DPAD L",
    "DPAD R",
    "PROFILE",
    "P1",
    "P2",
    "P3",
    "P4",
    "",
    "LT",
    "RT",
};

static const char *ps3_buttons[GAMEPAD_BUTTON_MAX] =
{
    "X",
    "CIRCLE",
    "SQUARE",
    "TRIANGLE",
    "SELECT",
    "PS",
    "START",
    "L3",
    "R3",
    "L1",
    "R1",
    "DPAD U",
    "DPAD D",
    "DPAD L",
    "DPAD R",
    "",
    "",
    "",
    "",
    "",
    "",
    "L2",
    "R2",
};

static const char *ps4_buttons[GAMEPAD_BUTTON_MAX] =
{
    "X",
    "CIRCLE",
    "SQUARE",
    "TRIANGLE",
    "SHARE",
    "PS",
    "OPTIONS",
    "L3",
    "R3",
    "L1",
    "R1",
    "DPAD U",
    "DPAD D",
    "DPAD L",
    "DPAD R",
    "",
    "",
    "",
    "",
    "",
    "TOUCH",
    "L2",
    "R2",
};

static const char *ps5_buttons[GAMEPAD_BUTTON_MAX] =
{
    "X",
    "CIRCLE",
    "SQUARE",
    "TRIANGLE",
    "SHARE",
    "PS",
    "OPTIONS",
    "L3",
    "R3",
    "L1",
    "R1",
    "DPAD U",
    "DPAD D",
    "DPAD L",
    "DPAD R",
    "MUTE",
    "",
    "",
    "",
    "",
    "TOUCH",
    "L2",
    "R2",
};

static const char *switchpro_buttons[GAMEPAD_BUTTON_MAX] =
{
    "B",
    "A",
    "Y",
    "X",
    "MINUS",
    "HOME",
    "PLUS",
    "LSB",
    "RSB",
    "L",
    "R",
    "DPAD U",
    "DPAD D",
    "DPAD L",
    "DPAD R",
    "CAPTURE",
    "",
    "",
    "",
    "",
    "",
    "ZL",
    "ZR",
};

static int PhysicalForVirtualButton(int vbutton)
{
    if (vbutton < NUM_VIRTUAL_BUTTONS)
    {
        return joystick_physical_buttons[vbutton];
    }
    else
    {
        return vbutton;
    }
}

// Get the virtual button number for the given variable, ie. the
// variable's index in all_joystick_buttons[NUM_VIRTUAL_BUTTONS].
static int VirtualButtonForVariable(int *variable)
{
    int i;

    for (i = 0; i < arrlen(all_joystick_buttons); ++i)
    {
        if (variable == all_joystick_buttons[i])
        {
            return i;
        }
    }

    I_Error("Couldn't find virtual button");
    return -1;
}

// Rearrange joystick button configuration to be in "canonical" form:
// each joyb* variable should have a value equal to its index in
// all_joystick_buttons[NUM_VIRTUAL_BUTTONS] above.
static void CanonicalizeButtons(void)
{
    int new_mapping[NUM_VIRTUAL_BUTTONS];
    int vbutton;
    int i;

    for (i = 0; i < arrlen(all_joystick_buttons); ++i)
    {
        vbutton = *all_joystick_buttons[i];

        // Don't remap the speed key if it's bound to "always run".
        // Also preserve "unbound" variables.
        if ((all_joystick_buttons[i] == &joybspeed &&
             vbutton >= MAX_VIRTUAL_BUTTONS) ||
            vbutton < 0)
        {
            new_mapping[i] = i;
        }
        else
        {
            new_mapping[i] = PhysicalForVirtualButton(vbutton);
            *all_joystick_buttons[i] = i;
        }
    }

    for (i = 0; i < NUM_VIRTUAL_BUTTONS; ++i)
    {
        joystick_physical_buttons[i] = new_mapping[i];
    }
}

// Check all existing buttons and clear any using the specified physical
// button.
static void ClearVariablesUsingButton(int physbutton)
{
    int vbutton;
    int i;

    for (i = 0; i < arrlen(all_joystick_buttons); ++i)
    {
        vbutton = *all_joystick_buttons[i];

        if (vbutton >= 0 && physbutton == PhysicalForVirtualButton(vbutton))
        {
            *all_joystick_buttons[i] = -1;
        }
    }
}

// Called in response to SDL events when the prompt window is open:

static int EventCallback(SDL_Event *event, TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    // Got the joystick button press?

    if (event->type == SDL_JOYBUTTONDOWN)
    {
        int vbutton, physbutton;

        // Before changing anything, remap button configuration into
        // canonical form, to avoid conflicts.
        CanonicalizeButtons();

        vbutton = VirtualButtonForVariable(joystick_input->variable);
        physbutton = event->jbutton.button;

        if (joystick_input->check_conflicts)
        {
            ClearVariablesUsingButton(physbutton);
        }

        // Set mapping.
        *joystick_input->variable = vbutton;
        joystick_physical_buttons[vbutton] = physbutton;

        TXT_CloseWindow(joystick_input->prompt_window);
        return 1;
    }

    return 0;
}

static int EventCallbackGamepad(SDL_Event *event,
                                TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    // Got the joystick button press?

    if (event->type == SDL_CONTROLLERBUTTONDOWN ||
        event->type == SDL_CONTROLLERAXISMOTION)
    {
        int vbutton, physbutton, axis;

        // Before changing anything, remap button configuration into
        // canonical form, to avoid conflicts.
        CanonicalizeButtons();

        vbutton = VirtualButtonForVariable(joystick_input->variable);
        axis = event->caxis.axis;

        if (event->type == SDL_CONTROLLERAXISMOTION &&
            (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
             axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) &&
            event->caxis.value > TRIGGER_THRESHOLD)
        {
            if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
            {
                physbutton = GAMEPAD_BUTTON_TRIGGERLEFT;
            }
            else
            {
                physbutton = GAMEPAD_BUTTON_TRIGGERRIGHT;
            }
        }
        else if (event->type == SDL_CONTROLLERBUTTONDOWN)
        {
            physbutton = event->cbutton.button;
        }
        else
        {
            return 0;
        }

        if (joystick_input->check_conflicts)
        {
            ClearVariablesUsingButton(physbutton);
        }

        // Set mapping.
        *joystick_input->variable = vbutton;
        joystick_physical_buttons[vbutton] = physbutton;

        TXT_CloseWindow(joystick_input->prompt_window);
        return 1;
    }

    return 0;
}

// When the prompt window is closed, disable the event callback function;
// we are no longer interested in receiving notification of events.

static void PromptWindowClosed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(joystick))
{
    TXT_CAST_ARG(SDL_Joystick, joystick);

    SDL_JoystickClose(joystick);
    TXT_SDL_SetEventCallback(NULL, NULL);
    SDL_JoystickEventState(SDL_DISABLE);
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

static void PromptWindowClosedGamepad(TXT_UNCAST_ARG(widget),
                                      TXT_UNCAST_ARG(joystick))
{
    TXT_CAST_ARG(SDL_GameController, joystick);

    SDL_GameControllerClose(joystick);
    TXT_SDL_SetEventCallback(NULL, NULL);
    SDL_JoystickEventState(SDL_DISABLE);
    SDL_GameControllerEventState(SDL_DISABLE);
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

static void OpenErrorWindow(void)
{
    TXT_MessageBox(NULL, "Please configure a controller first!");
}

static void OpenPromptWindow(txt_joystick_input_t *joystick_input)
{
    txt_window_t *window;
    SDL_Joystick *joystick;

    // Silently update when the shift button is held down.

    joystick_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
    {
        return;
    }

    // Check the current joystick is valid

    joystick = SDL_JoystickOpen(joystick_index);

    if (joystick == NULL)
    {
        OpenErrorWindow();
        return;
    }

    // Open the prompt window

    window = TXT_MessageBox(NULL, "Press the new button on the controller...");

    TXT_SDL_SetEventCallback(EventCallback, joystick_input);
    TXT_SignalConnect(window, "closed", PromptWindowClosed, joystick);
    joystick_input->prompt_window = window;

    SDL_JoystickEventState(SDL_ENABLE);
}

static void OpenPromptWindowGamepad(txt_joystick_input_t *joystick_input)
{
    txt_window_t *window;
    SDL_GameController *gamepad;

    // Silently update when the shift button is held down.

    joystick_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
    {
        return;
    }

    // Check the current joystick is valid

    gamepad = SDL_GameControllerOpen(joystick_index);

    if (gamepad == NULL)
    {
        OpenErrorWindow();
        return;
    }

    // Open the prompt window

    window = TXT_MessageBox(NULL, "Press the new button on the controller...");

    TXT_SDL_SetEventCallback(EventCallbackGamepad, joystick_input);
    TXT_SignalConnect(window, "closed", PromptWindowClosedGamepad, gamepad);
    joystick_input->prompt_window = window;

    // GameController events do not fire if Joystick events are disabled.
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);
}

static void TXT_JoystickInputSizeCalc(TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    // All joystickinputs are the same size.

    joystick_input->widget.w = JOYSTICK_INPUT_WIDTH;
    joystick_input->widget.h = 1;
}

static void GetJoystickButtonDescription(int vbutton, char *buf,
                                         size_t buf_len)
{
    M_snprintf(buf, buf_len, "BUTTON #%i",
               PhysicalForVirtualButton(vbutton) + 1);
}

static void TXT_JoystickInputDrawer(TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);
    char buf[20];
    int i;

    if (*joystick_input->variable < 0)
    {
        M_StringCopy(buf, "(none)", sizeof(buf));
    }
    else
    {
        GetJoystickButtonDescription(*joystick_input->variable,
                                     buf, sizeof(buf));
    }

    TXT_SetWidgetBG(joystick_input);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_DrawString(buf);

    for (i = TXT_UTF8_Strlen(buf); i < JOYSTICK_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static int GetGamepadButtonIndex(int button)
{
    int i;

    for (i = 0; i < arrlen(gamepad_buttons); ++i)
    {
        if (button == gamepad_buttons[i])
        {
            return i;
        }
    }

    return -1;
}

static void GetGamepadButtonDescription(int vbutton, char *buf, size_t buf_len)
{
    int index;

    index = GetGamepadButtonIndex(PhysicalForVirtualButton(vbutton));

    if (index < 0)
    {
        M_StringCopy(buf, "(unknown)", buf_len);
        return;
    }

    switch (gamepad_type)
    {
        case SDL_CONTROLLER_TYPE_XBOX360:
            M_snprintf(buf, buf_len, "%s", xbox360_buttons[index]);
            break;

        case SDL_CONTROLLER_TYPE_XBOXONE:
            M_snprintf(buf, buf_len, "%s", xboxone_buttons[index]);
            break;

        case SDL_CONTROLLER_TYPE_PS3:
            M_snprintf(buf, buf_len, "%s", ps3_buttons[index]);
            break;

        case SDL_CONTROLLER_TYPE_PS4:
            M_snprintf(buf, buf_len, "%s", ps4_buttons[index]);
            break;

        case SDL_CONTROLLER_TYPE_PS5:
            M_snprintf(buf, buf_len, "%s", ps5_buttons[index]);
            break;

        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
            M_snprintf(buf, buf_len, "%s", switchpro_buttons[index]);
            break;

        default:
            M_snprintf(buf, buf_len, "BUTTON #%i",
                       PhysicalForVirtualButton(vbutton) + 1);
            break;
    }
}

static void TXT_GamepadInputDrawer(TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);
    char buf[20]; // Need to fit "BUTTON #XX"
    int i;

    if (*joystick_input->variable < 0)
    {
        M_StringCopy(buf, "(none)", sizeof(buf));
    }
    else
    {
        GetGamepadButtonDescription(*joystick_input->variable, buf,
                                    sizeof(buf));
    }

    TXT_SetWidgetBG(joystick_input);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_DrawString(buf);

    for (i = TXT_UTF8_Strlen(buf); i < JOYSTICK_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_JoystickInputDestructor(TXT_UNCAST_ARG(joystick_input))
{
}

static int TXT_JoystickInputKeyPress(TXT_UNCAST_ARG(joystick_input), int key)
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    if (key == KEY_ENTER)
    {
        // Open a window to prompt for the new joystick press
        OpenPromptWindow(joystick_input);

        return 1;
    }

    if (key == KEY_BACKSPACE || key == KEY_DEL)
    {
        *joystick_input->variable = -1;
    }

    return 0;
}

static int TXT_GamepadInputKeyPress(TXT_UNCAST_ARG(joystick_input), int key)
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    if (key == KEY_ENTER)
    {
        // Open a window to prompt for the new joystick press
        OpenPromptWindowGamepad(joystick_input);

        return 1;
    }

    if (key == KEY_BACKSPACE || key == KEY_DEL)
    {
        *joystick_input->variable = -1;
    }

    return 0;
}

static void TXT_JoystickInputMousePress(TXT_UNCAST_ARG(widget),
                                        int x, int y, int b)
{
    TXT_CAST_ARG(txt_joystick_input_t, widget);

    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_JoystickInputKeyPress(widget, KEY_ENTER);
    }
}

static void TXT_GamepadInputMousePress(TXT_UNCAST_ARG(widget), int x, int y,
                                       int b)
{
    TXT_CAST_ARG(txt_joystick_input_t, widget);

    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_GamepadInputKeyPress(widget, KEY_ENTER);
    }
}

txt_widget_class_t txt_joystick_input_class =
{
    TXT_AlwaysSelectable,
    TXT_JoystickInputSizeCalc,
    TXT_JoystickInputDrawer,
    TXT_JoystickInputKeyPress,
    TXT_JoystickInputDestructor,
    TXT_JoystickInputMousePress,
    NULL,
};

txt_widget_class_t txt_gamepad_input_class =
{
    TXT_AlwaysSelectable,
    TXT_JoystickInputSizeCalc,
    TXT_GamepadInputDrawer,
    TXT_GamepadInputKeyPress,
    TXT_JoystickInputDestructor,
    TXT_GamepadInputMousePress,
    NULL,
};

txt_joystick_input_t *TXT_NewJoystickInput(int *variable)
{
    txt_joystick_input_t *joystick_input;

    joystick_input = malloc(sizeof(txt_joystick_input_t));

    if (use_gamepad)
    {
        TXT_InitWidget(joystick_input, &txt_gamepad_input_class);
    }
    else
    {
        TXT_InitWidget(joystick_input, &txt_joystick_input_class);
    }
    joystick_input->variable = variable;

    return joystick_input;
}

