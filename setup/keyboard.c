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
#include "textscreen.h"

#include "txt_keyinput.h"

int key_left = KEY_LEFTARROW;
int key_right = KEY_RIGHTARROW;
int key_up = KEY_UPARROW;
int key_down = KEY_DOWNARROW;
int key_strafeleft = ',';
int key_straferight = '.';
int key_fire = KEY_RCTRL;
int key_use = ' ';
int key_strafe = KEY_RALT;
int key_speed = KEY_RSHIFT;

static int *allkeys[] = {&key_left, &key_right, &key_up, &key_down, 
                         &key_strafeleft, &key_straferight, &key_fire, 
                         &key_use, &key_strafe, &key_speed};

// Callback invoked when a key control is set

static void KeySetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    int i;

    for (i=0; i<sizeof(allkeys) / sizeof(*allkeys); ++i)
    {
        if (*variable == *allkeys[i] && allkeys[i] != variable)
        {
            // A different key has the same value.  Clear the existing
            // value. This ensures that no two keys can have the same
            // value.

            *allkeys[i] = 0;
        }
    }
}

static void AddKeyControl(txt_table_t *table, char *name, int *var)
{
    txt_key_input_t *key_input;

    TXT_AddWidget(table, TXT_NewLabel(name));
    key_input = TXT_NewKeyInput(var);
    TXT_AddWidget(table, key_input);
    TXT_SignalConnect(key_input, "set", KeySetCallback, var);
}

void ConfigKeyboard(void)
{
    txt_window_t *window;
    txt_table_t *table;

    window = TXT_NewWindow("Keyboard configuration");

    TXT_AddWidget(window, TXT_NewSeparator("Movement"));

    table = TXT_NewTable(2);
    TXT_SetColumnWidths(table, 20, 8);

    AddKeyControl(table, "Move Forward", &key_up);
    AddKeyControl(table, "Move Backward", &key_down);
    AddKeyControl(table, "Turn Left", &key_left);
    AddKeyControl(table, "Turn Right", &key_right);
    AddKeyControl(table, "Strafe Left", &key_strafeleft);
    AddKeyControl(table, "Strafe Right", &key_straferight);
    AddKeyControl(table, "Speed On", &key_speed);
    AddKeyControl(table, "Strafe On", &key_strafe);

    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator("Action"));

    table = TXT_NewTable(2);
    TXT_SetColumnWidths(table, 20, 8);

    AddKeyControl(table, "Use", &key_use);
    AddKeyControl(table, "Fire", &key_fire);

    TXT_AddWidget(window, table);
}

