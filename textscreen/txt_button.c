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

#include "txt_button.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_ButtonSizeCalc(TXT_UNCAST_ARG(button))
{
    TXT_CAST_ARG(txt_button_t, button);

    button->widget.w = strlen(button->label);
    button->widget.h = 1;
}

static void TXT_ButtonDrawer(TXT_UNCAST_ARG(button), int selected)
{
    TXT_CAST_ARG(txt_button_t, button);
    int i;
    int w;

    w = button->widget.w;

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_SetWidgetBG(button, selected);

    TXT_DrawString(button->label);

    for (i=strlen(button->label); i < w; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_ButtonDestructor(TXT_UNCAST_ARG(button))
{
    TXT_CAST_ARG(txt_button_t, button);

    free(button->label);
}

static int TXT_ButtonKeyPress(TXT_UNCAST_ARG(button), int key)
{
    TXT_CAST_ARG(txt_button_t, button);

    if (key == KEY_ENTER)
    {
        TXT_EmitSignal(button, "pressed");
        return 1;
    }
    
    return 0;
}

static void TXT_ButtonMousePress(TXT_UNCAST_ARG(button), int x, int y, int b)
{
    TXT_CAST_ARG(txt_button_t, button);

    if (b == TXT_MOUSE_LEFT)
    {
        // Equivalent to pressing enter

        TXT_ButtonKeyPress(button, KEY_ENTER);
    }
}

txt_widget_class_t txt_button_class =
{
    TXT_AlwaysSelectable,
    TXT_ButtonSizeCalc,
    TXT_ButtonDrawer,
    TXT_ButtonKeyPress,
    TXT_ButtonDestructor,
    TXT_ButtonMousePress,
    NULL,
};

void TXT_SetButtonLabel(txt_button_t *button, char *label)
{
    free(button->label);
    button->label = strdup(label);
}

txt_button_t *TXT_NewButton(char *label)
{
    txt_button_t *button;

    button = malloc(sizeof(txt_button_t));

    TXT_InitWidget(button, &txt_button_class);
    button->label = strdup(label);

    return button;
}

// Button with a callback set automatically

txt_button_t *TXT_NewButton2(char *label, TxtWidgetSignalFunc func,
                             void *user_data)
{
    txt_button_t *button;

    button = TXT_NewButton(label);

    TXT_SignalConnect(button, "pressed", func, user_data);

    return button;
}

