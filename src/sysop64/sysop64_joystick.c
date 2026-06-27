//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
// DESCRIPTION:
//     C64 joystick port 2 input mapping for the Sysop-64 Doom backend.
//

#include "config.h"
#include "d_event.h"
#include "doomkeys.h"
#include "m_controls.h"
#include "sysop64.h"
#include "sysop64_backend.h"

#include <stdint.h>
#include <string.h>

// Reads the active-low state of the requested C64 joystick port.
uint8_t sysop_read_joystick(uint8_t joystick_number);

#define SYSOP_JOYSTICK_PORT 2
#define SYSOP_JOYSTICK_USE_HOLD_TICS 8
#define SYSOP_DOOM_NUM_KEYS 256

typedef struct {
    int key;
    int down;
} SysopJoystickKey;

static SysopJoystickKey joy_key_up;
static SysopJoystickKey joy_key_down;
static SysopJoystickKey joy_key_left;
static SysopJoystickKey joy_key_right;
static SysopJoystickKey joy_key_strafe_left;
static SysopJoystickKey joy_key_strafe_right;
static SysopJoystickKey joy_key_speed;
static SysopJoystickKey joy_key_fire;
static SysopJoystickKey joy_key_use;

static int previous_fire;
static int fire_hold_tics;
static int fire_shot_tics;

// Posts a synthetic key transition through Chocolate Doom's normal event path.
static void post_key_event(int key, int pressed)
{
    event_t event;

    if (key <= 0 || key >= SYSOP_DOOM_NUM_KEYS) {
        return;
    }

    memset(&event, 0, sizeof(event));
    event.type = pressed ? ev_keydown : ev_keyup;
    event.data1 = key;
    event.data2 = pressed ? key : 0;
    event.data3 = pressed ? key : 0;
    D_PostEvent(&event);
}

// Keeps one virtual key in sync without reposting duplicate keydown events.
static void set_virtual_key(SysopJoystickKey *state, int key, int down)
{
    if (state->down && (!down || state->key != key)) {
        post_key_event(state->key, 0);
        state->down = 0;
    }

    if (down && key > 0 && key < SYSOP_DOOM_NUM_KEYS && (!state->down || state->key != key)) {
        state->key = key;
        state->down = 1;
        post_key_event(state->key, 1);
    }
}

// Releases every synthetic key when joystick input is disabled or unavailable.
static void release_all_virtual_keys(void)
{
    set_virtual_key(&joy_key_up, 0, 0);
    set_virtual_key(&joy_key_down, 0, 0);
    set_virtual_key(&joy_key_left, 0, 0);
    set_virtual_key(&joy_key_right, 0, 0);
    set_virtual_key(&joy_key_strafe_left, 0, 0);
    set_virtual_key(&joy_key_strafe_right, 0, 0);
    set_virtual_key(&joy_key_speed, 0, 0);
    set_virtual_key(&joy_key_fire, 0, 0);
    set_virtual_key(&joy_key_use, 0, 0);
}

// Polls C64 joystick port 2 and maps it onto Doom movement/action controls.
void Sysop_JoystickRead(void)
{
    uint8_t joy;
    int up;
    int down;
    int left;
    int right;
    int fire;
    int directional;
    int use_held;

    if (!g_sysop_joystick_enabled || !Sysop_LibraryIsInitialized()) {
        release_all_virtual_keys();
        previous_fire = 0;
        fire_hold_tics = 0;
        fire_shot_tics = 0;
        return;
    }

    joy = sysop_read_joystick(SYSOP_JOYSTICK_PORT);
    if (joy == 0) {
        joy = 0xff;
    }

    // C64 joystick bits are active-low: a cleared bit means the control is down.
    up = (joy & 0x01) == 0;
    down = (joy & 0x02) == 0;
    left = (joy & 0x04) == 0;
    right = (joy & 0x08) == 0;
    fire = (joy & 0x10) == 0;
    directional = up || down || left || right;

    if (fire) {
        ++fire_hold_tics;
    } else {
        fire_hold_tics = 0;
        fire_shot_tics = 0;
    }

    if (fire && !previous_fire) {
        fire_shot_tics = 2;
    }

    // Fire alone becomes "use" after a short hold; fire plus motion modifies movement.
    use_held = fire && !directional && fire_hold_tics >= SYSOP_JOYSTICK_USE_HOLD_TICS;

    set_virtual_key(&joy_key_up, key_up, up);
    set_virtual_key(&joy_key_down, key_down, down);
    set_virtual_key(&joy_key_speed, key_speed, fire && (up || down));

    set_virtual_key(&joy_key_left, key_left, left && !fire);
    set_virtual_key(&joy_key_right, key_right, right && !fire);
    set_virtual_key(&joy_key_strafe_left, key_strafeleft, left && fire);
    set_virtual_key(&joy_key_strafe_right, key_straferight, right && fire);

    set_virtual_key(&joy_key_fire, key_fire, fire_shot_tics > 0 && !use_held);
    set_virtual_key(&joy_key_use, key_use, use_held);

    if (fire_shot_tics > 0) {
        --fire_shot_tics;
    }

    previous_fire = fire;
}
