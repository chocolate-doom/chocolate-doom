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

#include "txt_separator.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_utf8.h"
#include "txt_window.h"

static void TXT_SeparatorSizeCalc(TXT_UNCAST_ARG(separator))
{
    TXT_CAST_ARG(txt_separator_t, separator);

    if (separator->label != NULL)
    {
        // Minimum width is the string length + two spaces for padding

        separator->widget.w = TXT_UTF8_Strlen(separator->label) + 2;
    }
    else
    {
        separator->widget.w = 0;
    }

    separator->widget.h = 1;
}

static void TXT_SeparatorDrawer(TXT_UNCAST_ARG(separator))
{
    TXT_CAST_ARG(txt_separator_t, separator);
    int x, y;
    int w;

    w = separator->widget.w;

    TXT_GetXY(&x, &y);

    // Draw separator.  Go back one character and draw two extra
    // to overlap the window borders.

    TXT_DrawSeparator(x-2, y, w + 4);

    if (separator->label != NULL)
    {
        TXT_GotoXY(x, y);

        TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
        TXT_DrawString(" ");
        TXT_DrawString(separator->label);
        TXT_DrawString(" ");
    }
}

static void TXT_SeparatorDestructor(TXT_UNCAST_ARG(separator))
{
    TXT_CAST_ARG(txt_separator_t, separator);

    free(separator->label);
}

void TXT_SetSeparatorLabel(txt_separator_t *separator, char *label)
{
    free(separator->label);

    if (label != NULL)
    {
        separator->label = strdup(label);
    }
    else
    {
        separator->label = NULL;
    }
}

txt_widget_class_t txt_separator_class =
{
    TXT_NeverSelectable,
    TXT_SeparatorSizeCalc,
    TXT_SeparatorDrawer,
    NULL,
    TXT_SeparatorDestructor,
    NULL,
    NULL,
};

txt_separator_t *TXT_NewSeparator(char *label)
{
    txt_separator_t *separator;

    separator = malloc(sizeof(txt_separator_t));

    TXT_InitWidget(separator, &txt_separator_class);

    separator->label = NULL;
    TXT_SetSeparatorLabel(separator, label);

    return separator;
}

