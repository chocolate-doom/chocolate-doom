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

#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_radiobutton.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_utf8.h"
#include "txt_window.h"

static void TXT_RadioButtonSizeCalc(TXT_UNCAST_ARG(radiobutton))
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    // Minimum width is the string length + right-side spaces for padding

    radiobutton->widget.w = TXT_UTF8_Strlen(radiobutton->label) + 5;
    radiobutton->widget.h = 1;
}

static void TXT_RadioButtonDrawer(TXT_UNCAST_ARG(radiobutton))
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);
    txt_saved_colors_t colors;
    int i;
    int w;

    w = radiobutton->widget.w;

    TXT_SaveColors(&colors);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString("(");

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if (*radiobutton->variable == radiobutton->value)
    {
        TXT_DrawCodePageString("\x07");
    }
    else
    {
        TXT_DrawString(" ");
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);

    TXT_DrawString(") ");

    TXT_RestoreColors(&colors);
    TXT_SetWidgetBG(radiobutton);

    TXT_DrawString(radiobutton->label);

    for (i=TXT_UTF8_Strlen(radiobutton->label); i < w-5; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_RadioButtonDestructor(TXT_UNCAST_ARG(radiobutton))
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    free(radiobutton->label);
}

static int TXT_RadioButtonKeyPress(TXT_UNCAST_ARG(radiobutton), int key)
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    if (key == KEY_ENTER || key == ' ')
    {
        if (*radiobutton->variable != radiobutton->value)
        {
            *radiobutton->variable = radiobutton->value;
            TXT_EmitSignal(radiobutton, "selected");
        }
        return 1;
    }
    
    return 0;
}

static void TXT_RadioButtonMousePress(TXT_UNCAST_ARG(radiobutton), 
                                      int x, int y, int b)
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    if (b == TXT_MOUSE_LEFT)
    {
        // Equivalent to pressing enter

        TXT_RadioButtonKeyPress(radiobutton, KEY_ENTER);
    }
}

txt_widget_class_t txt_radiobutton_class =
{
    TXT_AlwaysSelectable,
    TXT_RadioButtonSizeCalc,
    TXT_RadioButtonDrawer,
    TXT_RadioButtonKeyPress,
    TXT_RadioButtonDestructor,
    TXT_RadioButtonMousePress,
    NULL,
};

txt_radiobutton_t *TXT_NewRadioButton(char *label, int *variable, int value)
{
    txt_radiobutton_t *radiobutton;

    radiobutton = malloc(sizeof(txt_radiobutton_t));

    TXT_InitWidget(radiobutton, &txt_radiobutton_class);
    radiobutton->label = strdup(label);
    radiobutton->variable = variable;
    radiobutton->value = value;

    return radiobutton;
}

void TXT_SetRadioButtonLabel(txt_radiobutton_t *radiobutton, char *value)
{
    free(radiobutton->label);
    radiobutton->label = strdup(value);
}

