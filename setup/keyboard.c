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
#include "doomtype.h"

#include "execute.h"
#include "txt_keyinput.h"

#include "joystick.h"
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

// Menu keys:

int key_menu_activate  = KEY_ESCAPE;
int key_menu_up        = KEY_UPARROW;
int key_menu_down      = KEY_DOWNARROW;
int key_menu_left      = KEY_LEFTARROW;
int key_menu_right     = KEY_RIGHTARROW;
int key_menu_back      = KEY_BACKSPACE;
int key_menu_forward   = KEY_ENTER;
int key_menu_confirm   = 'y';
int key_menu_abort     = 'n';

int key_menu_help      = KEY_F1;
int key_menu_save      = KEY_F2;
int key_menu_load      = KEY_F3;
int key_menu_volume    = KEY_F4;
int key_menu_detail    = KEY_F5;
int key_menu_qsave     = KEY_F6;
int key_menu_endgame   = KEY_F7;
int key_menu_messages  = KEY_F8;
int key_menu_qload     = KEY_F9;
int key_menu_quit      = KEY_F10;
int key_menu_gamma     = KEY_F11;

int key_menu_incscreen = KEY_EQUALS;
int key_menu_decscreen = KEY_MINUS;

int vanilla_keyboard_mapping = 1;

static int always_run = 0;

// Keys within these groups cannot have the same value.

static int *controls[] = { &key_left, &key_right, &key_up, &key_down,
                           &key_strafeleft, &key_straferight, &key_fire,
                           &key_use, &key_strafe, &key_speed, NULL };

static int *menu_nav[] = { &key_menu_activate, &key_menu_up, &key_menu_down,
                           &key_menu_left, &key_menu_right, &key_menu_back,
                           &key_menu_forward, NULL };

static int *shortcuts[] = { &key_menu_help, &key_menu_save, &key_menu_load,
                            &key_menu_volume, &key_menu_detail, &key_menu_qsave,
                            &key_menu_endgame, &key_menu_messages,
                            &key_menu_qload, &key_menu_quit, &key_menu_gamma,
                            &key_menu_incscreen, &key_menu_decscreen, NULL };

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

static int VarInGroup(int *variable, int **group)
{
    unsigned int i;

    for (i=0; group[i] != NULL; ++i)
    {
        if (group[i] == variable)
        {
            return 1;
        }
    }

    return 0;
}

static void CheckKeyGroup(int *variable, int **group)
{
    unsigned int i;

    // Don't check unless the variable is in this group.

    if (!VarInGroup(variable, group))
    {
        return;
    }

    // If another variable has the same value as the new value, reset it.

    for (i=0; group[i] != NULL; ++i)
    {
        if (*variable == *group[i] && group[i] != variable)
        {
            // A different key has the same value.  Clear the existing
            // value. This ensures that no two keys can have the same
            // value.

            *group[i] = 0;
        }
    }
}

// Callback invoked when a key control is set

static void KeySetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);

    CheckKeyGroup(variable, controls);
    CheckKeyGroup(variable, menu_nav);
    CheckKeyGroup(variable, shortcuts);
}

// Add a label and keyboard input to the specified table.

static void AddKeyControl(txt_table_t *table, char *name, int *var)
{
    txt_key_input_t *key_input;

    TXT_AddWidget(table, TXT_NewLabel(name));
    key_input = TXT_NewKeyInput(var);
    TXT_AddWidget(table, key_input);

    TXT_SignalConnect(key_input, "set", KeySetCallback, var);
}

static void MenuKeysDialog(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_table_t *table;
    txt_scrollpane_t *scrollpane;

    window = TXT_NewWindow("Menu keys");

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 25, 10);

    AddKeyControl(table, "Activate menu",         &key_menu_activate);
    AddKeyControl(table, "Move cursor up",        &key_menu_up);
    AddKeyControl(table, "Move cursor down",      &key_menu_down);
    AddKeyControl(table, "Move slider left",      &key_menu_left);
    AddKeyControl(table, "Move slider right",     &key_menu_right);
    AddKeyControl(table, "Go to previous menu",   &key_menu_back);
    AddKeyControl(table, "Activate menu item",    &key_menu_forward);
    AddKeyControl(table, "Confirm action",        &key_menu_confirm);
    AddKeyControl(table, "Cancel action",         &key_menu_abort);

    AddKeyControl(table, "Help screen",           &key_menu_help);
    AddKeyControl(table, "Save game",             &key_menu_save);
    AddKeyControl(table, "Load game",             &key_menu_load);
    AddKeyControl(table, "Sound volume",          &key_menu_volume);
    AddKeyControl(table, "Toggle detail",         &key_menu_detail);
    AddKeyControl(table, "Quick save",            &key_menu_qsave);
    AddKeyControl(table, "End game",              &key_menu_endgame);
    AddKeyControl(table, "Toggle messages",       &key_menu_messages);
    AddKeyControl(table, "Quick load",            &key_menu_qload);
    AddKeyControl(table, "Quit game",             &key_menu_quit);
    AddKeyControl(table, "Toggle gamma",          &key_menu_gamma);

    AddKeyControl(table, "Increase screen size",  &key_menu_incscreen);
    AddKeyControl(table, "Decrease screen size",  &key_menu_decscreen);

    scrollpane = TXT_NewScrollPane(0, 10, table);

    TXT_AddWidget(window, scrollpane);
}

void ConfigKeyboard(void)
{
    txt_window_t *window;
    txt_table_t *movement_table;
    txt_table_t *action_table;
    txt_checkbox_t *run_control;

    always_run = joybspeed >= 20;

    window = TXT_NewWindow("Keyboard configuration");

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Movement"),
                   movement_table = TXT_NewTable(4),

                   TXT_NewSeparator("Action"),
                   action_table = TXT_NewTable(4),
                   TXT_NewButton2("Menu keys...", MenuKeysDialog, NULL),
                   NULL);

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Misc."),
                   run_control = TXT_NewCheckBox("Always run", &always_run),
                   TXT_NewInvertedCheckBox("Use native keyboard mapping", 
                                           &vanilla_keyboard_mapping),
                   NULL);

    TXT_SetColumnWidths(movement_table, 15, 4, 15, 4);

    TXT_SignalConnect(run_control, "changed", UpdateJoybSpeed, NULL);

    AddKeyControl(movement_table, "Move Forward", &key_up);
    AddKeyControl(movement_table, " Strafe Left", &key_strafeleft);
    AddKeyControl(movement_table, "Move Backward", &key_down);
    AddKeyControl(movement_table, " Strafe Right", &key_straferight);
    AddKeyControl(movement_table, "Turn Left", &key_left);
    AddKeyControl(movement_table, " Speed On", &key_speed);
    AddKeyControl(movement_table, "Turn Right", &key_right);
    AddKeyControl(movement_table, " Strafe On", &key_strafe);

    TXT_SetColumnWidths(action_table, 15, 4, 15, 4);

    AddKeyControl(action_table, "Fire/Attack", &key_fire);
    AddKeyControl(action_table, " Use", &key_use);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
}

