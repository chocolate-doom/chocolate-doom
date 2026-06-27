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
//     C64 keyboard matrix scanning and Chocolate Doom key event translation
//     for the Sysop-64 backend.
//

#include "config.h"
#include "d_event.h"
#include "doomkeys.h"
#include "doomtype.h"
#include "sysop64.h"
#include "sysop64_backend.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Reads the raw active-low C64 keyboard matrix from the Sysop-64 hardware.
uint64_t sysop_read_key_data(void);

// Resets the attached C64; only exposed through the display tuning controls.
void c64_reset(void);

#define KEYQUEUE_SIZE 64
#define MAX_KEY_EVENTS_PER_SCAN 64

typedef enum
{
    SYSOP_C64_KEY_STOP = 0,
    SYSOP_C64_KEY_Q,
    SYSOP_C64_KEY_CMD,
    SYSOP_C64_KEY_SPACE,
    SYSOP_C64_KEY_2,
    SYSOP_C64_KEY_CTRL,
    SYSOP_C64_KEY_ESC,
    SYSOP_C64_KEY_1,

    SYSOP_C64_KEY_SLASH,
    SYSOP_C64_KEY_ARROW_UP,
    SYSOP_C64_KEY_EQUAL,
    SYSOP_C64_KEY_RIGHT_SHIFT,
    SYSOP_C64_KEY_HOME,
    SYSOP_C64_KEY_SEMICOLON,
    SYSOP_C64_KEY_STAR,
    SYSOP_C64_KEY_LBS,

    SYSOP_C64_KEY_COMMA,
    SYSOP_C64_KEY_AT,
    SYSOP_C64_KEY_COLON,
    SYSOP_C64_KEY_PERIOD,
    SYSOP_C64_KEY_MINUS,
    SYSOP_C64_KEY_L,
    SYSOP_C64_KEY_P,
    SYSOP_C64_KEY_PLUS,

    SYSOP_C64_KEY_N,
    SYSOP_C64_KEY_O,
    SYSOP_C64_KEY_K,
    SYSOP_C64_KEY_M,
    SYSOP_C64_KEY_0,
    SYSOP_C64_KEY_J,
    SYSOP_C64_KEY_I,
    SYSOP_C64_KEY_9,

    SYSOP_C64_KEY_V,
    SYSOP_C64_KEY_U,
    SYSOP_C64_KEY_H,
    SYSOP_C64_KEY_B,
    SYSOP_C64_KEY_8,
    SYSOP_C64_KEY_G,
    SYSOP_C64_KEY_Y,
    SYSOP_C64_KEY_7,

    SYSOP_C64_KEY_X,
    SYSOP_C64_KEY_T,
    SYSOP_C64_KEY_F,
    SYSOP_C64_KEY_C,
    SYSOP_C64_KEY_6,
    SYSOP_C64_KEY_D,
    SYSOP_C64_KEY_R,
    SYSOP_C64_KEY_5,

    SYSOP_C64_KEY_LEFT_SHIFT,
    SYSOP_C64_KEY_E,
    SYSOP_C64_KEY_S,
    SYSOP_C64_KEY_Z,
    SYSOP_C64_KEY_4,
    SYSOP_C64_KEY_A,
    SYSOP_C64_KEY_W,
    SYSOP_C64_KEY_3,

    SYSOP_C64_KEY_CRSR_DOWN,
    SYSOP_C64_KEY_F5,
    SYSOP_C64_KEY_F3,
    SYSOP_C64_KEY_F1,
    SYSOP_C64_KEY_F7,
    SYSOP_C64_KEY_CRSR_RIGHT,
    SYSOP_C64_KEY_RETURN,
    SYSOP_C64_KEY_DELETE,

    SYSOP_C64_KEY_CRSR_UP,
    SYSOP_C64_KEY_CRSR_LEFT,
    SYSOP_C64_KEY_COUNT
} SysopC64KeyCode;

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;
static int sysop_keyboard_aliases_enabled = 1;

// Converts the hardware matrix row/column into the local key enum ordering.
static uint8_t Sysop_C64KeyForMatrixPosition(int row, int col)
{
    return (uint8_t)((7 - row) * 8 + (7 - col));
}

// Checks both physical shift keys because C64 cursor direction depends on shift.
static int Sysop_C64ShiftIsDown(const uint8_t *scan_key_state)
{
    return scan_key_state[SYSOP_C64_KEY_LEFT_SHIFT]
        || scan_key_state[SYSOP_C64_KEY_RIGHT_SHIFT];
}

// Rewrites shifted cursor key presses into explicit up/left pseudo keys.
static void Sysop_NormalizeC64CursorKeys(uint8_t *scan_key_state)
{
    if (!Sysop_C64ShiftIsDown(scan_key_state)) {
        return;
    }

    if (scan_key_state[SYSOP_C64_KEY_CRSR_DOWN]) {
        scan_key_state[SYSOP_C64_KEY_CRSR_DOWN] = 0;
        scan_key_state[SYSOP_C64_KEY_CRSR_UP] = 1;
    }

    if (scan_key_state[SYSOP_C64_KEY_CRSR_RIGHT]) {
        scan_key_state[SYSOP_C64_KEY_CRSR_RIGHT] = 0;
        scan_key_state[SYSOP_C64_KEY_CRSR_LEFT] = 1;
    }
}

// Returns the printable ASCII character for C64 keys that map cleanly to text.
static uint8_t Sysop_C64KeyToAscii(uint8_t key)
{
    switch (key) {
        case SYSOP_C64_KEY_Q: return 'q';
        case SYSOP_C64_KEY_2: return '2';
        case SYSOP_C64_KEY_1: return '1';
        case SYSOP_C64_KEY_SLASH: return '/';
        case SYSOP_C64_KEY_ARROW_UP: return '^';
        case SYSOP_C64_KEY_EQUAL: return '=';
        case SYSOP_C64_KEY_SEMICOLON: return ';';
        case SYSOP_C64_KEY_STAR: return '*';
        case SYSOP_C64_KEY_COMMA: return ',';
        case SYSOP_C64_KEY_AT: return '@';
        case SYSOP_C64_KEY_COLON: return ':';
        case SYSOP_C64_KEY_PERIOD: return '.';
        case SYSOP_C64_KEY_MINUS: return '-';
        case SYSOP_C64_KEY_L: return 'l';
        case SYSOP_C64_KEY_P: return 'p';
        case SYSOP_C64_KEY_PLUS: return '+';
        case SYSOP_C64_KEY_N: return 'n';
        case SYSOP_C64_KEY_O: return 'o';
        case SYSOP_C64_KEY_K: return 'k';
        case SYSOP_C64_KEY_M: return 'm';
        case SYSOP_C64_KEY_0: return '0';
        case SYSOP_C64_KEY_J: return 'j';
        case SYSOP_C64_KEY_I: return 'i';
        case SYSOP_C64_KEY_9: return '9';
        case SYSOP_C64_KEY_V: return 'v';
        case SYSOP_C64_KEY_U: return 'u';
        case SYSOP_C64_KEY_H: return 'h';
        case SYSOP_C64_KEY_B: return 'b';
        case SYSOP_C64_KEY_8: return '8';
        case SYSOP_C64_KEY_G: return 'g';
        case SYSOP_C64_KEY_Y: return 'y';
        case SYSOP_C64_KEY_7: return '7';
        case SYSOP_C64_KEY_X: return 'x';
        case SYSOP_C64_KEY_T: return 't';
        case SYSOP_C64_KEY_F: return 'f';
        case SYSOP_C64_KEY_C: return 'c';
        case SYSOP_C64_KEY_6: return '6';
        case SYSOP_C64_KEY_D: return 'd';
        case SYSOP_C64_KEY_R: return 'r';
        case SYSOP_C64_KEY_5: return '5';
        case SYSOP_C64_KEY_E: return 'e';
        case SYSOP_C64_KEY_S: return 's';
        case SYSOP_C64_KEY_Z: return 'z';
        case SYSOP_C64_KEY_4: return '4';
        case SYSOP_C64_KEY_A: return 'a';
        case SYSOP_C64_KEY_W: return 'w';
        case SYSOP_C64_KEY_3: return '3';
        case SYSOP_C64_KEY_SPACE: return ' ';
        default: return 0;
    }
}

// Translates C64 key identities into the Doom key codes expected by the game.
static uint8_t Sysop_ConvertToDoomKey(uint8_t key)
{
    uint8_t ascii = Sysop_C64KeyToAscii(key);

    switch (key) {
        case SYSOP_C64_KEY_RETURN:
            return KEY_ENTER;
        case SYSOP_C64_KEY_DELETE:
            return KEY_BACKSPACE;
        case SYSOP_C64_KEY_ESC:
        case SYSOP_C64_KEY_STOP:
            return KEY_ESCAPE;
        case SYSOP_C64_KEY_CTRL:
            return KEY_TAB;
        case SYSOP_C64_KEY_CRSR_LEFT:
            return KEY_LEFTARROW;
        case SYSOP_C64_KEY_CRSR_RIGHT:
            return KEY_RIGHTARROW;
        case SYSOP_C64_KEY_CRSR_UP:
            return KEY_UPARROW;
        case SYSOP_C64_KEY_CRSR_DOWN:
            return KEY_DOWNARROW;
        case SYSOP_C64_KEY_CMD:
            return KEY_RCTRL;
        case SYSOP_C64_KEY_LEFT_SHIFT:
        case SYSOP_C64_KEY_RIGHT_SHIFT:
            return KEY_RSHIFT;
        case SYSOP_C64_KEY_PLUS:
        case SYSOP_C64_KEY_EQUAL:
            return KEY_EQUALS;
        case SYSOP_C64_KEY_F7:
            if (g_sysop_display_tune_enabled) {
                sysop_c64_reset();
            }
            return KEY_F7;
        default:
            break;
    }

    if (sysop_keyboard_aliases_enabled) {
        switch (ascii) {
            case 'j': return KEY_LEFTARROW;
            case 'l': return KEY_RIGHTARROW;
            case 'i':
            case '@': return KEY_UPARROW;
            case 'k':
            case ';': return KEY_DOWNARROW;
            default: break;
        }
    }

    if (ascii == 'z') {
        return ',';
    }

    if (ascii == 'x') {
        return '.';
    }

    return ascii ? ascii : 0;
}

// Scans the C64 matrix and emits edge-triggered raw key up/down events.
static void Sysop_ReadKeyboardEvents(unsigned int *kb_events, unsigned char *event_count)
{
    static int keyboard_init_done = 0;
    static uint8_t key_state[256];
    uint8_t scan_key_state[256];
    uint64_t key_data;

    *event_count = 0;

    if (!keyboard_init_done) {
        memset(key_state, 0, sizeof(key_state));
        keyboard_init_done = 1;
    }

    memset(scan_key_state, 0, sizeof(scan_key_state));

    key_data = sysop_read_key_data();

    for (int row = 0; row < 8; row++) {
        uint8_t row_data = (key_data >> (row * 8)) & 0xff;

        if (row_data == 0xff) {
            continue;
        }

        for (int col = 0; col < 8; col++) {
            uint8_t mask = 1 << col;

            if ((row_data & mask) == 0) {
                uint8_t key_code = Sysop_C64KeyForMatrixPosition(row, col);
                scan_key_state[key_code] = 1;
            }
        }
    }

    Sysop_NormalizeC64CursorKeys(scan_key_state);

    for (int key_code = 0; key_code < SYSOP_C64_KEY_COUNT; key_code++) {
        if (scan_key_state[key_code] != key_state[key_code]) {
            if (*event_count < MAX_KEY_EVENTS_PER_SCAN) {
                kb_events[*event_count] = key_code;

                if (scan_key_state[key_code]) {
                    kb_events[*event_count] |= 256;
                }

                (*event_count)++;
            }

            key_state[key_code] = scan_key_state[key_code];
        }
    }
}

// Adds one translated key event to the ring buffer and handles tuning hotkeys.
static void Sysop_AddKeyToQueue(int pressed, uint8_t key_code)
{
    uint8_t key = Sysop_ConvertToDoomKey(key_code);
    uint8_t raw_char = Sysop_C64KeyToAscii(key_code);
    uint16_t key_data = (pressed << 8) | key;

    if (g_sysop_key_debug) {
        printf("[key] %s raw=0x%02x", pressed ? "down" : "up", key_code);
        if (isprint(raw_char)) {
            printf(" '%c'", raw_char);
        }
        printf(" -> doom=0x%02x", key);
        if (isprint(key)) {
            printf(" '%c'", key);
        }
        printf("\n");
        fflush(stdout);
    }

    if (pressed && g_sysop_display_tune_enabled) {
        switch (key_code) {
            case SYSOP_C64_KEY_F3:
                sysop_keyboard_aliases_enabled = !sysop_keyboard_aliases_enabled;
                printf("Sysop keyboard aliases: %s\n",
                       sysop_keyboard_aliases_enabled ? "on" : "off");
                return;
            default:
                break;
        }

        if (Sysop_HandleDisplayTuneKey(raw_char)) {
            return;
        }
    }

    if (key == 0) {
        return;
    }

    s_KeyQueue[s_KeyQueueWriteIndex] = key_data;
    s_KeyQueueWriteIndex = (s_KeyQueueWriteIndex + 1) % KEYQUEUE_SIZE;
}

// Pops one pending Doom key event from the ring buffer.
static int Sysop_DequeueKey(int *pressed, unsigned char *doom_key)
{
    uint16_t key_data;

    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
        return 0;
    }

    key_data = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex = (s_KeyQueueReadIndex + 1) % KEYQUEUE_SIZE;

    *pressed = (key_data >> 8) != 0;
    *doom_key = (unsigned char)(key_data & 0xff);

    return 1;
}

// Polls the Sysop-64 keyboard hardware and queues any changed key states.
void Sysop_KeyboardScan(void)
{
    uint32_t kb_events[MAX_KEY_EVENTS_PER_SCAN];
    uint8_t event_count = 0;

    Sysop_ReadKeyboardEvents(kb_events, &event_count);

    for (int i = 0; i < event_count; i++) {
        uint32_t event = kb_events[i];
        Sysop_AddKeyToQueue((event & 0x100) != 0, (uint8_t)(event & 0xff));
    }
}

// Posts queued key events into Chocolate Doom's normal event system.
void Sysop_PostQueuedKeyEvents(void)
{
    int pressed;
    unsigned char key;
    event_t event;

    while (Sysop_DequeueKey(&pressed, &key)) {
        if (key == 0) {
            continue;
        }

        memset(&event, 0, sizeof(event));
        event.type = pressed ? ev_keydown : ev_keyup;
        event.data1 = key;
        event.data2 = pressed ? key : 0;
        event.data3 = pressed ? key : 0;
        D_PostEvent(&event);
    }
}
