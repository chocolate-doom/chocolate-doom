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
// DESCRIPTION:
//       SDL Joystick code.
//
//-----------------------------------------------------------------------------


#include "SDL.h"
#include "SDL_joystick.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joystick.h"

// When an axis is within the dead zone, it is set to zero.
// This is 5% of the full range:

#define DEAD_ZONE (32768 / 20)

static SDL_Joystick *joystick = NULL;

// Configuration variables:

// Standard default.cfg Joystick enable/disable

extern int usejoystick;

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

void I_InitJoystick(void)
{
    int num_axes;

    if (!usejoystick)
    {
        return;
    }

    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
        return;
    }

    if (joystick_index < 0 || joystick_index >= SDL_NumJoysticks())
    {
        printf("I_InitJoystick: Invalid joystick ID: %i\n", joystick_index);
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        return;
    }

    // Open the joystick

    joystick = SDL_JoystickOpen(joystick_index);

    if (joystick == NULL)
    {
        printf("I_InitJoystick: Failed to open joystick #%i\n",
               joystick_index);
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        return;
    }

    num_axes = SDL_JoystickNumAxes(joystick);

    if (joystick_x_axis < 0 || joystick_x_axis >= num_axes
     || joystick_y_axis < 0 || joystick_y_axis >= num_axes)
    {
        printf("I_InitJoystick: Invalid joystick axis for joystick #%i "
               "(run joystick setup again)\n",
               joystick_index);

        SDL_JoystickClose(joystick);
        joystick = NULL;
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }

    SDL_JoystickEventState(SDL_ENABLE);

    // Initialized okay!

    printf("I_InitJoystick: %s\n", SDL_JoystickName(joystick_index));
}

void I_ShutdownJoystick(void)
{
    if (joystick != NULL) 
    {
        SDL_JoystickClose(joystick);
        joystick = NULL;
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }
}

// Get a bitmask of all currently-pressed buttons

static int GetButtonState(void)
{
    int i;
    int result;

    result = 0;

    for (i=0; i<SDL_JoystickNumButtons(joystick); ++i) 
    {
        if (SDL_JoystickGetButton(joystick, i))
        {
            result |= 1 << i;
        }
    }

    return result;
}

// Read the state of an axis, inverting if necessary.

static int GetAxisState(int axis, int invert)
{
    int result;

    result = SDL_JoystickGetAxis(joystick, axis);

    if (invert)
    {
        result = -result;
    }

    if (result < DEAD_ZONE && result > -DEAD_ZONE)
    {
        result = 0;
    }

    return result;
}

void I_UpdateJoystick(void)
{
    if (joystick != NULL)
    {
        event_t ev;

        ev.type = ev_joystick;
        ev.data1 = GetButtonState();
        ev.data2 = GetAxisState(joystick_x_axis, joystick_x_invert);
        ev.data3 = GetAxisState(joystick_y_axis, joystick_y_invert);

        D_PostEvent(&ev);
    }
}

