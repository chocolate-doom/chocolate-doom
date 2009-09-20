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
#include <string.h>

#include "doomkeys.h"

#include "txt_window_action.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_WindowActionSizeCalc(TXT_UNCAST_ARG(action))
{
    TXT_CAST_ARG(txt_window_action_t, action);
    char buf[10];

    TXT_GetKeyDescription(action->key, buf);

    // Minimum width is the string length + two spaces for padding

    action->widget.w = strlen(action->label) + strlen(buf) + 1;
    action->widget.h = 1;
}

static void TXT_WindowActionDrawer(TXT_UNCAST_ARG(action), int selected)
{
    TXT_CAST_ARG(txt_window_action_t, action);
    char buf[10];

    TXT_GetKeyDescription(action->key, buf);

    TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
    TXT_DrawString(buf);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString("=");
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_DrawString(action->label);
}

static void TXT_WindowActionDestructor(TXT_UNCAST_ARG(action))
{
    TXT_CAST_ARG(txt_window_action_t, action);

    free(action->label);
}

static int TXT_WindowActionKeyPress(TXT_UNCAST_ARG(action), int key)
{
    TXT_CAST_ARG(txt_window_action_t, action);

    if (key == action->key)
    {
        TXT_EmitSignal(action, "pressed");
        return 1;
    }
    
    return 0;
}

static void TXT_WindowActionMousePress(TXT_UNCAST_ARG(action), 
                                       int x, int y, int b)
{
    TXT_CAST_ARG(txt_window_action_t, action);

    // Simulate a press of the key

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_WindowActionKeyPress(action, action->key);
    }
}

txt_widget_class_t txt_window_action_class =
{
    TXT_WindowActionSizeCalc,
    TXT_WindowActionDrawer,
    TXT_WindowActionKeyPress,
    TXT_WindowActionDestructor,
    TXT_WindowActionMousePress,
    NULL,
};

txt_window_action_t *TXT_NewWindowAction(int key, const char *label)
{
    txt_window_action_t *action;

    action = malloc(sizeof(txt_window_action_t));

    TXT_InitWidget(action, &txt_window_action_class);
    action->key = key;
    action->label = strdup(label);

    return action;
}

static void WindowCloseCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_CloseWindow(window);
}

static void WindowSelectCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_WidgetKeyPress(window, KEY_ENTER);
}

// An action with the name "close" the closes the window

txt_window_action_t *TXT_NewWindowEscapeAction(txt_window_t *window)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_ESCAPE, "Close");
    TXT_SignalConnect(action, "pressed", WindowCloseCallback, window);

    return action;
}

// Exactly the same as the above, but the button is named "abort"

txt_window_action_t *TXT_NewWindowAbortAction(txt_window_t *window)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_ESCAPE, "Abort");
    TXT_SignalConnect(action, "pressed", WindowCloseCallback, window);

    return action;
}

txt_window_action_t *TXT_NewWindowSelectAction(txt_window_t *window)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_ENTER, "Select");
    TXT_SignalConnect(action, "pressed", WindowSelectCallback, window);

    return action;
}

