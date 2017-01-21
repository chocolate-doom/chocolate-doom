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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"
#include "m_misc.h"

#include "txt_mouseinput.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_label.h"
#include "txt_utf8.h"
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

    // Silently update when the shift key is held down.
    mouse_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

    window = TXT_MessageBox(NULL, "Press the new mouse button...");

    TXT_SetMouseListener(window, MousePressCallback, mouse_input);
}

static void TXT_MouseInputSizeCalc(TXT_UNCAST_ARG(mouse_input))
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);

    // All mouseinputs are the same size.

    mouse_input->widget.w = MOUSE_INPUT_WIDTH;
    mouse_input->widget.h = 1;
}

static void GetMouseButtonDescription(int button, char *buf, size_t buf_len)
{
    switch (button)
    {
        case 0:
            M_StringCopy(buf, "LEFT", buf_len);
            break;
        case 1:
            M_StringCopy(buf, "RIGHT", buf_len);
            break;
        case 2:
            M_StringCopy(buf, "MID", buf_len);
            break;
        default:
            M_snprintf(buf, buf_len, "BUTTON #%i", button + 1);
            break;
    }
}

static void TXT_MouseInputDrawer(TXT_UNCAST_ARG(mouse_input))
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);
    char buf[20];
    int i;

    if (*mouse_input->variable < 0)
    {
        M_StringCopy(buf, "(none)", sizeof(buf));
    }
    else
    {
        GetMouseButtonDescription(*mouse_input->variable, buf, sizeof(buf));
    }

    TXT_SetWidgetBG(mouse_input);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    
    TXT_DrawString(buf);
    
    for (i = TXT_UTF8_Strlen(buf); i < MOUSE_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_MouseInputDestructor(TXT_UNCAST_ARG(mouse_input))
{
}

static int TXT_MouseInputKeyPress(TXT_UNCAST_ARG(mouse_input), int key)
{
    TXT_CAST_ARG(txt_mouse_input_t, mouse_input);

    if (key == KEY_ENTER)
    {
        // Open a window to prompt for the new mouse press

        OpenPromptWindow(mouse_input);

        return 1;
    }

    if (key == KEY_BACKSPACE || key == KEY_DEL)
    {
        *mouse_input->variable = -1;
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

