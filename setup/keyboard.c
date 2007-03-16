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

#include "execute.h"
#include "txt_keyinput.h"

#include "keyboard.h"

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
int joybspeed = 3;

int vanilla_keyboard_mapping = 1;

static int always_run = 0;

static int *allkeys[] = {&key_left, &key_right, &key_up, &key_down, 
                         &key_strafeleft, &key_straferight, &key_fire, 
                         &key_use, &key_strafe, &key_speed};

static void UpdateJoybSpeed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(var))
{
    if (always_run)
    {
        /*
         <Janizdreg> if you want to pick one for chocolate doom to use, 
                     pick 29, since that is the most universal one that 
                     also works with heretic, hexen and strife =P

         NB. This choice also works with original, ultimate and final exes.
        */

        joybspeed = 29;
    }
    else
    {
       joybspeed = 0;
    }
}

// Callback invoked when a key control is set

static void KeySetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    unsigned int i;

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
    txt_table_t *movement_table;
    txt_table_t *action_table;
    txt_checkbox_t *run_control;

    always_run = joybspeed >= 10;

    window = TXT_NewWindow("Keyboard configuration");

    TXT_AddWidgets(window, 
                   TXT_NewSeparator("Movement"),
                   movement_table = TXT_NewTable(2),

                   TXT_NewSeparator("Action"),
                   action_table = TXT_NewTable(2),

                   TXT_NewSeparator("Misc."),
                   run_control = TXT_NewCheckBox("Always run", &always_run),
                   TXT_NewInvertedCheckBox("Use native keyboard mapping", 
                                           &vanilla_keyboard_mapping),
                   NULL);

    TXT_SetColumnWidths(movement_table, 20, 8);

    TXT_SignalConnect(run_control, "changed", UpdateJoybSpeed, NULL);

    AddKeyControl(movement_table, "Move Forward", &key_up);
    AddKeyControl(movement_table, "Move Backward", &key_down);
    AddKeyControl(movement_table, "Turn Left", &key_left);
    AddKeyControl(movement_table, "Turn Right", &key_right);
    AddKeyControl(movement_table, "Strafe Left", &key_strafeleft);
    AddKeyControl(movement_table, "Strafe Right", &key_straferight);
    AddKeyControl(movement_table, "Speed On", &key_speed);
    AddKeyControl(movement_table, "Strafe On", &key_strafe);

    TXT_SetColumnWidths(action_table, 20, 8);

    AddKeyControl(action_table, "Use", &key_use);
    AddKeyControl(action_table, "Fire", &key_fire);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
}

