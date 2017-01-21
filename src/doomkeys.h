//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//       Key definitions
//

#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW	0xae
#define KEY_LEFTARROW	0xac
#define KEY_UPARROW	0xad
#define KEY_DOWNARROW	0xaf
#define KEY_ESCAPE	27
#define KEY_ENTER	13
#define KEY_TAB		9
#define KEY_F1		(0x80+0x3b)
#define KEY_F2		(0x80+0x3c)
#define KEY_F3		(0x80+0x3d)
#define KEY_F4		(0x80+0x3e)
#define KEY_F5		(0x80+0x3f)
#define KEY_F6		(0x80+0x40)
#define KEY_F7		(0x80+0x41)
#define KEY_F8		(0x80+0x42)
#define KEY_F9		(0x80+0x43)
#define KEY_F10		(0x80+0x44)
#define KEY_F11		(0x80+0x57)
#define KEY_F12		(0x80+0x58)

#define KEY_BACKSPACE	0x7f
#define KEY_PAUSE	0xff

#define KEY_EQUALS	0x3d
#define KEY_MINUS	0x2d

#define KEY_RSHIFT	(0x80+0x36)
#define KEY_RCTRL	(0x80+0x1d)
#define KEY_RALT	(0x80+0x38)

#define KEY_LALT	KEY_RALT

// new keys:

#define KEY_CAPSLOCK    (0x80+0x3a)
#define KEY_NUMLOCK     (0x80+0x45)
#define KEY_SCRLCK      (0x80+0x46)
#define KEY_PRTSCR      (0x80+0x59)

#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)

#define KEYP_0          0
#define KEYP_1          KEY_END
#define KEYP_2          KEY_DOWNARROW
#define KEYP_3          KEY_PGDN
#define KEYP_4          KEY_LEFTARROW
#define KEYP_5          '5'
#define KEYP_6          KEY_RIGHTARROW
#define KEYP_7          KEY_HOME
#define KEYP_8          KEY_UPARROW
#define KEYP_9          KEY_PGUP

#define KEYP_DIVIDE     '/'
#define KEYP_PLUS       '+'
#define KEYP_MINUS      '-'
#define KEYP_MULTIPLY   '*'
#define KEYP_PERIOD     0
#define KEYP_EQUALS     KEY_EQUALS
#define KEYP_ENTER      KEY_ENTER

#define SCANCODE_TO_KEYS_ARRAY {                                            \
    0,   0,   0,   0,   'a',                                  /* 0-9 */     \
    'b', 'c', 'd', 'e', 'f',                                                \
    'g', 'h', 'i', 'j', 'k',                                  /* 10-19 */   \
    'l', 'm', 'n', 'o', 'p',                                                \
    'q', 'r', 's', 't', 'u',                                  /* 20-29 */   \
    'v', 'w', 'x', 'y', 'z',                                                \
    '1', '2', '3', '4', '5',                                  /* 30-39 */   \
    '6', '7', '8', '9', '0',                                                \
    KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, ' ',       /* 40-49 */   \
    KEY_MINUS, KEY_EQUALS, '[', ']', '\\',                                  \
    0,   ';', '\'', '`', ',',                                 /* 50-59 */   \
    '.', '/', KEY_CAPSLOCK, KEY_F1, KEY_F2,                                 \
    KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,                   /* 60-69 */   \
    KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,                              \
    KEY_PRTSCR, KEY_SCRLCK, KEY_PAUSE, KEY_INS, KEY_HOME,     /* 70-79 */   \
    KEY_PGUP, KEY_DEL, KEY_END, KEY_PGDN, KEY_RIGHTARROW,                   \
    KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW,                /* 80-89 */   \
    KEY_NUMLOCK, KEYP_DIVIDE,                                               \
    KEYP_MULTIPLY, KEYP_MINUS, KEYP_PLUS, KEYP_ENTER, KEYP_1,               \
    KEYP_2, KEYP_3, KEYP_4, KEYP_5, KEYP_6,                   /* 90-99 */   \
    KEYP_7, KEYP_8, KEYP_9, KEYP_0, KEYP_PERIOD,                            \
    0, 0, 0, KEYP_EQUALS,                                     /* 100-103 */ \
}

// Default names for keys, to use in English or as fallback.
#define KEY_NAMES_ARRAY {                                            \
    { KEY_BACKSPACE,  "BACKSP" },   { KEY_TAB,        "TAB" },       \
    { KEY_INS,        "INS" },      { KEY_DEL,        "DEL" },       \
    { KEY_PGUP,       "PGUP" },     { KEY_PGDN,       "PGDN" },      \
    { KEY_ENTER,      "ENTER" },    { KEY_ESCAPE,     "ESC" },       \
    { KEY_F1,         "F1" },       { KEY_F2,         "F2" },        \
    { KEY_F3,         "F3" },       { KEY_F4,         "F4" },        \
    { KEY_F5,         "F5" },       { KEY_F6,         "F6" },        \
    { KEY_F7,         "F7" },       { KEY_F8,         "F8" },        \
    { KEY_F9,         "F9" },       { KEY_F10,        "F10" },       \
    { KEY_F11,        "F11" },      { KEY_F12,        "F12" },       \
    { KEY_HOME,       "HOME" },     { KEY_END,        "END" },       \
    { KEY_MINUS,      "-" },        { KEY_EQUALS,     "=" },         \
    { KEY_NUMLOCK,    "NUMLCK" },   { KEY_SCRLCK,     "SCRLCK" },    \
    { KEY_PAUSE,      "PAUSE" },    { KEY_PRTSCR,     "PRTSC" },     \
    { KEY_UPARROW,    "UP" },       { KEY_DOWNARROW,  "DOWN" },      \
    { KEY_LEFTARROW,  "LEFT" },     { KEY_RIGHTARROW, "RIGHT" },     \
    { KEY_RALT,       "ALT" },      { KEY_LALT,       "ALT" },       \
    { KEY_RSHIFT,     "SHIFT" },    { KEY_CAPSLOCK,   "CAPS" },      \
    { KEY_RCTRL,      "CTRL" },     { ' ',            "SPACE" },     \
    { 'a', "A" },   { 'b', "B" },   { 'c', "C" },   { 'd', "D" },    \
    { 'e', "E" },   { 'f', "F" },   { 'g', "G" },   { 'h', "H" },    \
    { 'i', "I" },   { 'j', "J" },   { 'k', "K" },   { 'l', "L" },    \
    { 'm', "M" },   { 'n', "N" },   { 'o', "O" },   { 'p', "P" },    \
    { 'q', "Q" },   { 'r', "R" },   { 's', "S" },   { 't', "T" },    \
    { 'u', "U" },   { 'v', "V" },   { 'w', "W" },   { 'x', "X" },    \
    { 'y', "Y" },   { 'z', "Z" },   { '0', "0" },   { '1', "1" },    \
    { '2', "2" },   { '3', "3" },   { '4', "4" },   { '5', "5" },    \
    { '6', "6" },   { '7', "7" },   { '8', "8" },   { '9', "9" },    \
    { '[', "[" },   { ']', "]" },   { ';', ";" },   { '`', "`" },    \
    { ',', "," },   { '.', "." },   { '/', "/" },   { '\\', "\\" },  \
    { '\'', "\'" },                                                  \
}

#endif          // __DOOMKEYS__

