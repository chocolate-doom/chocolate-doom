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

#include <stdlib.h>
#include "textscreen.h"

#include "txt_mouseinput.h"

int novert;
int speed;
int accel;
int threshold;

int mouseb_fire;
int mouseb_strafe;
int mouseb_forward;

static int *all_mouse_buttons[] = {&mouseb_fire, &mouseb_strafe, 
                                   &mouseb_forward};

static void MouseSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
    TXT_CAST_ARG(int, variable);
    int i;

    // Check if the same mouse button is used for a different action
    // If so, set the other action(s) to -1 (unset)

    for (i=0; i<sizeof(all_mouse_buttons) / sizeof(*all_mouse_buttons); ++i)
    {
        if (*all_mouse_buttons[i] == *variable
         && all_mouse_buttons[i] != variable)
        {
            *all_mouse_buttons[i] = -1;
        }
    }
}

static void AddMouseControl(txt_table_t *table, char *label, int *var)
{
    txt_mouse_input_t *mouse_input;

    TXT_AddWidget(table, TXT_NewLabel(label));

    mouse_input = TXT_NewMouseInput(var);
    TXT_AddWidget(table, mouse_input);

    TXT_SignalConnect(mouse_input, "set", MouseSetCallback, var);
}

void ConfigMouse(void)
{
    txt_window_t *window;
    txt_table_t *table;

    window = TXT_NewWindow("Mouse configuration");

    TXT_AddWidget(window, TXT_NewSeparator("Mouse motion"));

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 25, 10);
    TXT_AddWidget(table, TXT_NewLabel("Speed"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&speed, 8));
    TXT_AddWidget(table, TXT_NewLabel("Acceleration"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&accel, 8));
    TXT_AddWidget(table, TXT_NewLabel("Acceleration threshold"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&threshold, 8));

    TXT_AddWidget(window, table);
    
    TXT_AddWidget(window, TXT_NewSeparator("Mouse buttons"));

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 25, 10);
    AddMouseControl(table, "Fire weapon", &mouseb_fire);
    AddMouseControl(table, "Move forward", &mouseb_forward);
    AddMouseControl(table, "Strafe on", &mouseb_strafe);
    
    TXT_AddWidget(window, table);
    
    TXT_AddWidget(window, TXT_NewSeparator(NULL));

    TXT_AddWidget(window, 
                  TXT_NewInvertedCheckBox("Allow vertical mouse movement", 
                                          &novert));

}

