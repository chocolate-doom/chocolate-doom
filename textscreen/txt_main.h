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
//
// Base interface that abstracts the text mode screen.
//

#ifndef TXT_MAIN_H
#define TXT_MAIN_H

// For the moment, txt_sdl.c is the only implementation of the base 
// text mode screen API:

#include "txt_sdl.h"

// textscreen key values:
// Key values are difficult because we have to support multiple conflicting
// address spaces.
// First, Doom's key constants use 0-127 as ASCII and extra values from
// 128-255 to represent special keys. Second, mouse buttons are represented
// as buttons. Finally, we want to be able to support Unicode.
//
// So we define different ranges:
// 0-255:    Doom key constants, including ASCII.
// 256-511:  Mouse buttons and other reserved.
// >=512:    Unicode values greater than 127 are offset up into this range.

// Special keypress values that correspond to mouse button clicks

#define TXT_MOUSE_BASE         256
#define TXT_MOUSE_LEFT         (TXT_MOUSE_BASE + 0)
#define TXT_MOUSE_RIGHT        (TXT_MOUSE_BASE + 1)
#define TXT_MOUSE_MIDDLE       (TXT_MOUSE_BASE + 2)
#define TXT_MOUSE_SCROLLUP     (TXT_MOUSE_BASE + 3)
#define TXT_MOUSE_SCROLLDOWN   (TXT_MOUSE_BASE + 4)
#define TXT_MAX_MOUSE_BUTTONS  16

#define TXT_KEY_TO_MOUSE_BUTTON(x)                                        \
        ( (x) >= TXT_MOUSE_BASE                                           \
       && (x) < TXT_MOUSE_BASE + TXT_MAX_MOUSE_BUTTONS ?                  \
          (x) - TXT_MOUSE_BASE : -1 )

// Unicode offset. Unicode values from 128 onwards are offset up into
// this range, so TXT_UNICODE_BASE = Unicode character #128, and so on.

#define TXT_UNICODE_BASE       512

// Convert a key value to a Unicode character:

#define TXT_KEY_TO_UNICODE(x)                                             \
        ( (x) < 128 ? (x) :                                               \
          (x) >= TXT_UNICODE_BASE ? ((x) - TXT_UNICODE_BASE + 128) : 0 )

// Screen size

#define TXT_SCREEN_W 80
#define TXT_SCREEN_H 25

#define TXT_COLOR_BLINKING (1 << 3)

typedef enum
{
    TXT_COLOR_BLACK,
    TXT_COLOR_BLUE,
    TXT_COLOR_GREEN,
    TXT_COLOR_CYAN,
    TXT_COLOR_RED,
    TXT_COLOR_MAGENTA,
    TXT_COLOR_BROWN,
    TXT_COLOR_GREY,
    TXT_COLOR_DARK_GREY,
    TXT_COLOR_BRIGHT_BLUE,
    TXT_COLOR_BRIGHT_GREEN,
    TXT_COLOR_BRIGHT_CYAN,
    TXT_COLOR_BRIGHT_RED,
    TXT_COLOR_BRIGHT_MAGENTA,
    TXT_COLOR_YELLOW,
    TXT_COLOR_BRIGHT_WHITE,
} txt_color_t;

// Modifier keys.

typedef enum
{
    TXT_MOD_SHIFT,
    TXT_MOD_CTRL,
    TXT_MOD_ALT,
    TXT_NUM_MODIFIERS
} txt_modifier_t;

// Initialize the screen
// Returns 1 if successful, 0 if failed.
int TXT_Init(void);

// Shut down text mode emulation
void TXT_Shutdown(void);

// Get a pointer to the buffer containing the raw screen data.
unsigned char *TXT_GetScreenData(void);

// Update an area of the screen
void TXT_UpdateScreenArea(int x, int y, int w, int h);

// Update the whole screen
void TXT_UpdateScreen(void);

// Read a character from the keyboard
int TXT_GetChar(void);

// Read the current state of modifier keys that are held down.
int TXT_GetModifierState(txt_modifier_t mod);

// Provides a short description of a key code, placing into the 
// provided buffer.
void TXT_GetKeyDescription(int key, char *buf, size_t buf_len);

// Retrieve the current position of the mouse
void TXT_GetMousePosition(int *x, int *y);

// Sleep until an event is received or the screen needs updating
// Optional timeout in ms (timeout == 0 : sleep forever)
void TXT_Sleep(int timeout);

// Controls whether keys are returned from TXT_GetChar based on keyboard
// mapping, or raw key code.
void TXT_EnableKeyMapping(int enable);

// Set the window title of the window containing the text mode screen
void TXT_SetWindowTitle(char *title);

// Safe string copy.
void TXT_StringCopy(char *dest, const char *src, size_t dest_len);

// Safe string concatenate.
void TXT_StringConcat(char *dest, const char *src, size_t dest_len);

// Safe version of vsnprintf().
int TXT_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);

// Safe version of snprintf().
int TXT_snprintf(char *buf, size_t buf_len, const char *s, ...);

#endif /* #ifndef TXT_MAIN_H */

