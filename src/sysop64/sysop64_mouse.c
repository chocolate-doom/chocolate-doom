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
//     Sysop-64 mouse discovery, Linux input-event reading, and Doom mouse
//     event posting.
//

#include "config.h"
#include "d_event.h"
#include "doomtype.h"
#include "sysop64_backend.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#define SYSOP_MOUSE_DEV_INPUT_DIR "/dev/input"
#define SYSOP_MOUSE_KEY_MAX 0x2ff
#define SYSOP_MOUSE_REL_MAX 0x0f
#define SYSOP_MOUSE_ABS_MAX 0x3f

#define SYSOP_EV_KEY 0x01
#define SYSOP_EV_REL 0x02
#define SYSOP_EV_ABS 0x03

#define SYSOP_REL_X 0x00
#define SYSOP_REL_Y 0x01
#define SYSOP_REL_HWHEEL 0x06
#define SYSOP_REL_WHEEL 0x08

#define SYSOP_KEY_ESC 0x01
#define SYSOP_BTN_LEFT 0x110
#define SYSOP_BTN_MOUSE 0x110
#define SYSOP_BTN_RIGHT 0x111
#define SYSOP_BTN_MIDDLE 0x112
#define SYSOP_BTN_SIDE 0x113
#define SYSOP_BTN_EXTRA 0x114
#define SYSOP_BTN_GAMEPAD 0x130

#define SYSOP_MOUSE_BUTTON_LEFT 0
#define SYSOP_MOUSE_BUTTON_RIGHT 1
#define SYSOP_MOUSE_BUTTON_MIDDLE 2
#define SYSOP_MOUSE_BUTTON_WHEELUP 3
#define SYSOP_MOUSE_BUTTON_WHEELDOWN 4
#define SYSOP_MOUSE_BUTTON_SIDE 5
#define SYSOP_MOUSE_BUTTON_EXTRA 6

#define SYSOP_EVIOCGBIT(ev, len) _IOC(_IOC_READ, 'E', 0x20 + (ev), len)

#define SYSOP_MOUSE_BITS_PER_LONG (sizeof(unsigned long) * 8)
#define SYSOP_MOUSE_NBITS(x) ((((x) - 1) / SYSOP_MOUSE_BITS_PER_LONG) + 1)
#define SYSOP_MOUSE_OFF(x) ((x) % SYSOP_MOUSE_BITS_PER_LONG)
#define SYSOP_MOUSE_LONG(x) ((x) / SYSOP_MOUSE_BITS_PER_LONG)

typedef struct
{
    struct timeval time;
    unsigned short type;
    unsigned short code;
    int value;
} sysop_input_event_t;

static int g_sysop_mouse_fd = -1;
static char g_sysop_mouse_path[PATH_MAX];
static unsigned int g_sysop_mouse_buttons = 0;

// Test one Linux input capability bit from an EVIOCGBIT bitset.
static int sysop_mouse_test_bit(int bit, const unsigned long *array)
{
    return (array[SYSOP_MOUSE_LONG(bit)] >> SYSOP_MOUSE_OFF(bit)) & 1UL;
}

// Check whether an input-event device has relative X/Y motion and is not
// obviously a keyboard or gamepad.
static int sysop_mouse_looks_like_mouse(int fd)
{
    unsigned long keybits[SYSOP_MOUSE_NBITS(SYSOP_MOUSE_KEY_MAX)] = {0};
    unsigned long relbits[SYSOP_MOUSE_NBITS(SYSOP_MOUSE_REL_MAX)] = {0};
    unsigned long absbits[SYSOP_MOUSE_NBITS(SYSOP_MOUSE_ABS_MAX)] = {0};
    int has_rel_x;
    int has_rel_y;
    int has_btn_left;
    int has_btn_mouse;
    int has_key_esc;
    int has_gamepad;

    if (fd < 0) {
        return 0;
    }

    if (ioctl(fd, SYSOP_EVIOCGBIT(SYSOP_EV_REL, sizeof(relbits)), relbits) < 0) {
        return 0;
    }

    if (ioctl(fd, SYSOP_EVIOCGBIT(SYSOP_EV_KEY, sizeof(keybits)), keybits) < 0) {
        return 0;
    }

    (void) ioctl(fd, SYSOP_EVIOCGBIT(SYSOP_EV_ABS, sizeof(absbits)), absbits);

    has_rel_x = sysop_mouse_test_bit(SYSOP_REL_X, relbits);
    has_rel_y = sysop_mouse_test_bit(SYSOP_REL_Y, relbits);
    has_btn_left = sysop_mouse_test_bit(SYSOP_BTN_LEFT, keybits);
    has_btn_mouse = sysop_mouse_test_bit(SYSOP_BTN_MOUSE, keybits);
    has_key_esc = sysop_mouse_test_bit(SYSOP_KEY_ESC, keybits);
    has_gamepad = sysop_mouse_test_bit(SYSOP_BTN_GAMEPAD, keybits);

    if (has_key_esc || has_gamepad) {
        return 0;
    }

    if (has_rel_x && has_rel_y && (has_btn_left || has_btn_mouse)) {
        return 1;
    }

    return has_rel_x && has_rel_y;
}

// Scan /dev/input/event* and open the first relative mouse-like device.
static int sysop_mouse_open_first(char *out_path, size_t out_path_size)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(SYSOP_MOUSE_DEV_INPUT_DIR);
    if (dir == NULL) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[PATH_MAX];
        int fd;

        if (strncmp(entry->d_name, "event", 5) != 0) {
            continue;
        }

        if (snprintf(full_path, sizeof(full_path), "%s/%s",
                     SYSOP_MOUSE_DEV_INPUT_DIR, entry->d_name) >= (int) sizeof(full_path)) {
            continue;
        }

        fd = open(full_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }

        if (sysop_mouse_looks_like_mouse(fd)) {
            if (out_path != NULL && out_path_size > 0) {
                snprintf(out_path, out_path_size, "%s", full_path);
            }
            closedir(dir);
            return fd;
        }

        close(fd);
    }

    closedir(dir);
    return -1;
}

// Apply Chocolate Doom's configured mouse acceleration while preserving sign.
static int sysop_mouse_accelerate(int val)
{
    if (val < 0) {
        return -sysop_mouse_accelerate(-val);
    }

    if (val > mouse_threshold) {
        return (int) ((val - mouse_threshold) * mouse_acceleration + mouse_threshold);
    }

    return val;
}

// Post one accumulated mouse movement/button event into Chocolate Doom.
static void sysop_mouse_post_event(unsigned int buttons, int x, int y)
{
    event_t event;

    memset(&event, 0, sizeof(event));
    event.type = ev_mouse;
    event.data1 = (int) buttons;
    event.data2 = x;
    event.data3 = y;
    D_PostEvent(&event);
}

// Send wheel movement as a press/release pair because Doom represents wheel
// directions as transient mouse buttons.
static void sysop_mouse_post_wheel(int button)
{
    unsigned int wheel_bit = 1U << button;

    sysop_mouse_post_event(g_sysop_mouse_buttons | wheel_bit, 0, 0);
    sysop_mouse_post_event(g_sysop_mouse_buttons, 0, 0);
}

// Update the tracked mouse button bitmask and report whether it changed.
static int sysop_mouse_set_button(int button, int pressed)
{
    unsigned int old_buttons = g_sysop_mouse_buttons;
    unsigned int bit = 1U << button;

    if (pressed) {
        g_sysop_mouse_buttons |= bit;
    } else {
        g_sysop_mouse_buttons &= ~bit;
    }

    return old_buttons != g_sysop_mouse_buttons;
}

// Close the currently opened input-event mouse device.
static void sysop_mouse_close_fd(void)
{
    if (g_sysop_mouse_fd >= 0) {
        close(g_sysop_mouse_fd);
        g_sysop_mouse_fd = -1;
    }
}

// Open the Sysop mouse device when --sysop-mouse enabled this input path.
void Sysop_MouseInit(void)
{
    if (!g_sysop_mouse_enabled || g_sysop_mouse_fd >= 0) {
        return;
    }

    g_sysop_mouse_path[0] = '\0';
    g_sysop_mouse_fd = sysop_mouse_open_first(g_sysop_mouse_path, sizeof(g_sysop_mouse_path));

    if (g_sysop_mouse_fd >= 0) {
        printf("Sysop mouse: using %s\n", g_sysop_mouse_path);
    } else {
        printf("Sysop mouse: enabled, but no relative mouse was found in %s\n",
               SYSOP_MOUSE_DEV_INPUT_DIR);
    }
}

// Close the mouse device and clear held button state.
void Sysop_MouseShutdown(void)
{
    sysop_mouse_close_fd();
    g_sysop_mouse_buttons = 0;
}

// Drain pending Linux input events, translate them to Doom mouse state, and
// post at most one accumulated movement event.
void Sysop_MouseRead(void)
{
    int raw_x = 0;
    int raw_y = 0;
    int button_changed = 0;
    int saw_input = 0;

    if (!g_sysop_mouse_enabled || g_sysop_mouse_fd < 0) {
        return;
    }

    for (;;) {
        sysop_input_event_t ev;
        ssize_t result = read(g_sysop_mouse_fd, &ev, sizeof(ev));

        if (result == sizeof(ev)) {
            saw_input = 1;

            if (ev.type == SYSOP_EV_REL) {
                if (ev.code == SYSOP_REL_X) {
                    raw_x += ev.value;
                } else if (ev.code == SYSOP_REL_Y) {
                    raw_y += ev.value;
                } else if (ev.code == SYSOP_REL_WHEEL) {
                    if (ev.value > 0) {
                        sysop_mouse_post_wheel(SYSOP_MOUSE_BUTTON_WHEELUP);
                    } else if (ev.value < 0) {
                        sysop_mouse_post_wheel(SYSOP_MOUSE_BUTTON_WHEELDOWN);
                    }
                } else if (ev.code == SYSOP_REL_HWHEEL) {
                    // Horizontal wheel input is ignored for now.
                }
            } else if (ev.type == SYSOP_EV_KEY) {
                int pressed = ev.value != 0;

                if (ev.code == SYSOP_BTN_LEFT) {
                    button_changed |= sysop_mouse_set_button(SYSOP_MOUSE_BUTTON_LEFT, pressed);
                } else if (ev.code == SYSOP_BTN_RIGHT) {
                    button_changed |= sysop_mouse_set_button(SYSOP_MOUSE_BUTTON_RIGHT, pressed);
                } else if (ev.code == SYSOP_BTN_MIDDLE) {
                    button_changed |= sysop_mouse_set_button(SYSOP_MOUSE_BUTTON_MIDDLE, pressed);
                } else if (ev.code == SYSOP_BTN_SIDE) {
                    button_changed |= sysop_mouse_set_button(SYSOP_MOUSE_BUTTON_SIDE, pressed);
                } else if (ev.code == SYSOP_BTN_EXTRA) {
                    button_changed |= sysop_mouse_set_button(SYSOP_MOUSE_BUTTON_EXTRA, pressed);
                }
            }

            continue;
        }

        if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            printf("Sysop mouse: lost %s (%s)\n",
                   g_sysop_mouse_path[0] ? g_sysop_mouse_path : "mouse device",
                   strerror(errno));
            sysop_mouse_close_fd();
        }

        break;
    }

    if (saw_input) {
        int x = sysop_mouse_accelerate(raw_x);
        int y = novert ? 0 : -sysop_mouse_accelerate(raw_y);

        if (x != 0 || y != 0 || button_changed) {
            sysop_mouse_post_event(g_sysop_mouse_buttons, x, y);
        }
    }
}
