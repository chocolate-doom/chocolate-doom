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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_mouseinput.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_label.h"
#include "txt_window.h"

#define MOUSE_INPUT_WIDTH 8

static int MousePressCallback(txt_window_t *window, 
                              int x, int y, int b,
                              TXT_UNCAST_ARG(mouse_input))
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);

    // Got the mouse press.  Save to the variable and close the window.

    *mouse_input->variable = b - TXT_MOUSE_BASE;

    if (mouse_input->check_conflicts)
    {
        TXT_EmitSignal(mouse_input, "set");
    }

    TXT_CloseWindow(window);

    return 1;
}

static void OpenPromptWindow(txt_mouse_input_t *mouse_input)
{
    txt_window_t *window;
    txt_label_t *label;

    // Silently update when the shift key is held down.
    mouse_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

    window = TXT_NewWindow(NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
    
    label = TXT_NewLabel("Press the new mouse button...");

    TXT_AddWidget(window, label);
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);

    TXT_SetMouseListener(window, MousePressCallback, mouse_input);
}

static void TXT_MouseInputSizeCalc(TXT_UNCAST_ARG(mouse_input))
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);

    // All mouseinputs are the same size.

    mouse_input->widget.w = MOUSE_INPUT_WIDTH;
    mouse_input->widget.h = 1;
}

static void GetMouseButtonDescription(int button, char *buf)
{
    switch (button)
    {
        case 0:
            strcpy(buf, "LEFT");
            break;
        case 1:
            strcpy(buf, "RIGHT");
            break;
        case 2:
            strcpy(buf, "MID");
            break;
        default:
            sprintf(buf, "BUTTON #%i", button + 1);
            break;
    }
}

static void TXT_MouseInputDrawer(TXT_UNCAST_ARG(mouse_input), int selected)
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);
    char buf[20];
    int i;

    if (*mouse_input->variable < 0)
    {
        strcpy(buf, "(none)");
    }
    else
    {
        GetMouseButtonDescription(*mouse_input->variable, buf);
    }

    TXT_SetWidgetBG(mouse_input, selected);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    
    TXT_DrawString(buf);
    
    for (i=strlen(buf); i<MOUSE_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_MouseInputDestructor(TXT_UNCAST_ARG(mouse_input))
{
}

static int TXT_MouseInputKeyPress(TXT_UNCAST_ARG(mouse_input), int mouse)
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);

    if (mouse == KEY_ENTER)
    {
        // Open a window to prompt for the new mouse press

        OpenPromptWindow(mouse_input);

        return 1;
    }

    return 0;
}

static void TXT_MouseInputMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b)
{
    TXT_CAST_ARG(txt_mouse_input_t, widget);

    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_MouseInputKeyPress(widget, KEY_ENTER);
    }
}

txt_widget_class_t txt_mouse_input_class =
{
    TXT_AlwaysSelectable,
    TXT_MouseInputSizeCalc,
    TXT_MouseInputDrawer,
    TXT_MouseInputKeyPress,
    TXT_MouseInputDestructor,
    TXT_MouseInputMousePress,
    NULL,
};

txt_mouse_input_t *TXT_NewMouseInput(int *variable)
{
    txt_mouse_input_t *mouse_input;

    mouse_input = malloc(sizeof(txt_mouse_input_t));

    TXT_InitWidget(mouse_input, &txt_mouse_input_class);
    mouse_input->variable = variable;

    return mouse_input;
}

