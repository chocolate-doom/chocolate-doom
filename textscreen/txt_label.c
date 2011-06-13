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

#include "txt_label.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_LabelSizeCalc(TXT_UNCAST_ARG(label))
{
    TXT_CAST_ARG(txt_label_t, label);

    label->widget.w = label->w;
    label->widget.h = label->h;
}

static void TXT_LabelDrawer(TXT_UNCAST_ARG(label), int selected)
{
    TXT_CAST_ARG(txt_label_t, label);
    unsigned int x, y;
    int origin_x, origin_y;
    unsigned int align_indent = 0;
    unsigned int w;

    w = label->widget.w;

    TXT_BGColor(label->bgcolor, 0);
    TXT_FGColor(label->fgcolor);

    TXT_GetXY(&origin_x, &origin_y);

    for (y=0; y<label->h; ++y)
    {
        // Calculate the amount to indent this line due to the align 
        // setting

        switch (label->widget.align)
        {
            case TXT_HORIZ_LEFT:
                align_indent = 0;
                break;
            case TXT_HORIZ_CENTER:
                align_indent = (label->w - strlen(label->lines[y])) / 2;
                break;
            case TXT_HORIZ_RIGHT:
                align_indent = label->w - strlen(label->lines[y]);
                break;
        }
        
        // Draw this line

        TXT_GotoXY(origin_x, origin_y + y);

        // Gap at the start

        for (x=0; x<align_indent; ++x)
        {
            TXT_DrawString(" ");
        }

        // The string itself

        TXT_DrawString(label->lines[y]);
        x += strlen(label->lines[y]);

        // Gap at the end

        for (; x<w; ++x)
        {
            TXT_DrawString(" ");
        }
    }
}

static void TXT_LabelDestructor(TXT_UNCAST_ARG(label))
{
    TXT_CAST_ARG(txt_label_t, label);

    free(label->label);
    free(label->lines);
}

txt_widget_class_t txt_label_class =
{
    TXT_NeverSelectable,
    TXT_LabelSizeCalc,
    TXT_LabelDrawer,
    NULL,
    TXT_LabelDestructor,
    NULL,
    NULL,
};

void TXT_SetLabel(txt_label_t *label, char *value)
{
    char *p;
    unsigned int y;

    // Free back the old label

    free(label->label);
    free(label->lines);

    // Set the new value 

    label->label = strdup(value);

    // Work out how many lines in this label

    label->h = 1;

    for (p = value; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            ++label->h;
        }
    }

    // Split into lines

    label->lines = malloc(sizeof(char *) * label->h);
    label->lines[0] = label->label;
    y = 1;
    
    for (p = label->label; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            label->lines[y] = p + 1;
            *p = '\0';
            ++y;
        }
    }

    label->w = 0;

    for (y=0; y<label->h; ++y)
    {
        if (strlen(label->lines[y]) > label->w)
            label->w = strlen(label->lines[y]);
    }
}

txt_label_t *TXT_NewLabel(char *text)
{
    txt_label_t *label;

    label = malloc(sizeof(txt_label_t));

    TXT_InitWidget(label, &txt_label_class);
    label->label = NULL;
    label->lines = NULL;

    // Default colors

    label->bgcolor = TXT_WINDOW_BACKGROUND;
    label->fgcolor = TXT_COLOR_BRIGHT_WHITE;

    TXT_SetLabel(label, text);

    return label;
}

void TXT_SetFGColor(txt_label_t *label, txt_color_t color)
{
    label->fgcolor = color;
}

void TXT_SetBGColor(txt_label_t *label, txt_color_t color)
{
    label->bgcolor = color;
}

