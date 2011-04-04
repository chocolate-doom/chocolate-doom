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

#include "txt_checkbox.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_CheckBoxSizeCalc(TXT_UNCAST_ARG(checkbox))
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    // Minimum width is the string length + right-side space for padding

    checkbox->widget.w = strlen(checkbox->label) + 5;
    checkbox->widget.h = 1;
}

static void TXT_CheckBoxDrawer(TXT_UNCAST_ARG(checkbox), int selected)
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);
    int i;
    int w;

    w = checkbox->widget.w;

    TXT_BGColor(TXT_WINDOW_BACKGROUND, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString("(");

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if ((*checkbox->variable != 0) ^ checkbox->inverted)
    {
        TXT_DrawString("\x07");
    }
    else
    {
        TXT_DrawString(" ");
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);

    TXT_DrawString(") ");

    TXT_SetWidgetBG(checkbox, selected);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_DrawString(checkbox->label);
    
    for (i=strlen(checkbox->label); i < w-5; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_CheckBoxDestructor(TXT_UNCAST_ARG(checkbox))
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    free(checkbox->label);
}

static int TXT_CheckBoxKeyPress(TXT_UNCAST_ARG(checkbox), int key)
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    if (key == KEY_ENTER || key == ' ')
    {
        *checkbox->variable = !*checkbox->variable;
        TXT_EmitSignal(checkbox, "changed");
        return 1;
    }
    
    return 0;
}

static void TXT_CheckBoxMousePress(TXT_UNCAST_ARG(checkbox), int x, int y, int b)
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    if (b == TXT_MOUSE_LEFT)
    {
        // Equivalent to pressing enter

        TXT_CheckBoxKeyPress(checkbox, KEY_ENTER);
    }
}

txt_widget_class_t txt_checkbox_class =
{
    TXT_AlwaysSelectable,
    TXT_CheckBoxSizeCalc,
    TXT_CheckBoxDrawer,
    TXT_CheckBoxKeyPress,
    TXT_CheckBoxDestructor,
    TXT_CheckBoxMousePress,
    NULL,
};

txt_checkbox_t *TXT_NewCheckBox(char *label, int *variable)
{
    txt_checkbox_t *checkbox;

    checkbox = malloc(sizeof(txt_checkbox_t));

    TXT_InitWidget(checkbox, &txt_checkbox_class);
    checkbox->label = strdup(label);
    checkbox->variable = variable;
    checkbox->inverted = 0;

    return checkbox;
}

txt_checkbox_t *TXT_NewInvertedCheckBox(char *label, int *variable)
{
    txt_checkbox_t *result;

    result = TXT_NewCheckBox(label, variable);
    result->inverted = 1;

    return result;
}

