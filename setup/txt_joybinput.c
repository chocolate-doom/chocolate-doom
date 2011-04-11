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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_joystick.h"

#include "doomkeys.h"
#include "joystick.h"

#include "txt_joybinput.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_label.h"
#include "txt_sdl.h"
#include "txt_window.h"

#define JOYSTICK_INPUT_WIDTH 10

// Called in response to SDL events when the prompt window is open:

static int EventCallback(SDL_Event *event, TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    // Got the joystick button press?

    if (event->type == SDL_JOYBUTTONDOWN)
    {
        *joystick_input->variable = event->jbutton.button;

        if (joystick_input->check_conflicts)
        {
            TXT_EmitSignal(joystick_input, "set");
        }

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

static void OpenErrorWindow(void)
{
    txt_window_t *window;

    window = TXT_NewWindow(NULL);

    TXT_AddWidget(window, TXT_NewLabel("Please configure a joystick first!"));

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowEscapeAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
}

static void OpenPromptWindow(txt_joystick_input_t *joystick_input)
{
    txt_window_t *window;
    txt_label_t *label;
    SDL_Joystick *joystick;

    // Silently update when the shift button is held down.

    joystick_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
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

    window = TXT_NewWindow(NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
    
    label = TXT_NewLabel("Press the new joystick button...");

    TXT_AddWidget(window, label);
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_SDL_SetEventCallback(EventCallback, joystick_input);
    TXT_SignalConnect(window, "closed", PromptWindowClosed, joystick);
    joystick_input->prompt_window = window;

    SDL_JoystickEventState(SDL_ENABLE);
}

static void TXT_JoystickInputSizeCalc(TXT_UNCAST_ARG(joystick_input))
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    // All joystickinputs are the same size.

    joystick_input->widget.w = JOYSTICK_INPUT_WIDTH;
    joystick_input->widget.h = 1;
}

static void GetJoystickButtonDescription(int button, char *buf)
{
    sprintf(buf, "BUTTON #%i", button + 1);
}

static void TXT_JoystickInputDrawer(TXT_UNCAST_ARG(joystick_input), int selected)
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);
    char buf[20];
    int i;

    if (*joystick_input->variable < 0)
    {
        strcpy(buf, "(none)");
    }
    else
    {
        GetJoystickButtonDescription(*joystick_input->variable, buf);
    }

    TXT_SetWidgetBG(joystick_input, selected);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    
    TXT_DrawString(buf);
    
    for (i=strlen(buf); i<JOYSTICK_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_JoystickInputDestructor(TXT_UNCAST_ARG(joystick_input))
{
}

static int TXT_JoystickInputKeyPress(TXT_UNCAST_ARG(joystick_input), int joystick)
{
    TXT_CAST_ARG(txt_joystick_input_t, joystick_input);

    if (joystick == KEY_ENTER)
    {
        // Open a window to prompt for the new joystick press

        OpenPromptWindow(joystick_input);

        return 1;
    }

    return 0;
}

static void TXT_JoystickInputMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b)
{
    TXT_CAST_ARG(txt_joystick_input_t, widget);
            
    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_JoystickInputKeyPress(widget, KEY_ENTER);
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

txt_joystick_input_t *TXT_NewJoystickInput(int *variable)
{
    txt_joystick_input_t *joystick_input;

    joystick_input = malloc(sizeof(txt_joystick_input_t));

    TXT_InitWidget(joystick_input, &txt_joystick_input_class);
    joystick_input->variable = variable;

    return joystick_input;
}

