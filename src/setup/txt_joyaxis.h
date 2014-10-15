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

#ifndef TXT_JOY_AXIS_H
#define TXT_JOY_AXIS_H

typedef struct txt_joystick_axis_s txt_joystick_axis_t;

typedef enum
{
    JOYSTICK_AXIS_HORIZONTAL,
    JOYSTICK_AXIS_VERTICAL,
} txt_joystick_axis_direction_t;

typedef enum
{
    CONFIG_CENTER,      // "Center the joystick and press a button..."
    CONFIG_STAGE1,      // "Top or left and press a button..."
    CONFIG_STAGE2,      // [Optional] "Bottom or right and press a button..."
} txt_joystick_axis_stage_t;

// Callback invoked when calibration is completed.
typedef void (*txt_joystick_axis_callback_t)(void);

#include "txt_widget.h"
#include "txt_window.h"

#include "SDL.h"

//
// A joystick axis.
//

struct txt_joystick_axis_s
{
    txt_widget_t widget;
    int *axis, *invert;
    txt_joystick_axis_direction_t dir;

    // Only used when configuring:

    // Configuration prompt window and label.
    txt_window_t *config_window;
    txt_label_t *config_label;

    // SDL joystick handle for reading joystick state.
    SDL_Joystick *joystick;

    // "Bad" joystick axes. Sometimes an axis can be stuck or "bad". An
    // example I found is that if you unplug the nunchuck extension from
    // a Wii remote, the axes from the nunchuck can be stuck at one of
    // the maximum values. These have to be ignored, so when we ask the
    // user to center the joystick, we look for bad axes that are not
    // close to zero.
    boolean *bad_axis;

    // Stage we have reached in configuring joystick axis.
    txt_joystick_axis_stage_t config_stage;

    // Button to press to advance to next stage.
    int config_button;

    // Callback invoked when the axis is calibrated.
    txt_joystick_axis_callback_t callback;
};

txt_joystick_axis_t *TXT_NewJoystickAxis(int *axis, int *invert,
                                         txt_joystick_axis_direction_t dir);

// Configure a joystick axis widget.
//   axis: The axis widget to configure.
//   using_button: If non-negative, use this joystick button as the button
//       to expect from the user. Otherwise, ask.
void TXT_ConfigureJoystickAxis(txt_joystick_axis_t *axis, int using_button,
                               txt_joystick_axis_callback_t callback);

#endif /* #ifndef TXT_JOY_AXIS_H */


