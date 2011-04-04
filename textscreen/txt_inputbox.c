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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_inputbox.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void SetBufferFromValue(txt_inputbox_t *inputbox);

static void TXT_InputBoxSizeCalc(TXT_UNCAST_ARG(inputbox))
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    // Enough space for the box + cursor

    inputbox->widget.w = inputbox->size + 1;
    inputbox->widget.h = 1;
}

static void TXT_InputBoxDrawer(TXT_UNCAST_ARG(inputbox), int selected)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);
    int i;
    int chars;
    int w;

    w = inputbox->widget.w;

    // Select the background color based on whether we are currently
    // editing, and if not, whether the widget is selected.

    if (inputbox->editing && selected)
    {
        TXT_BGColor(TXT_COLOR_BLACK, 0);
    }
    else
    {
        TXT_SetWidgetBG(inputbox, selected);
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if (!inputbox->editing)
    {
        // If not editing, use the current value from inputbox->value.
 
        SetBufferFromValue(inputbox);
    }
    
    TXT_DrawString(inputbox->buffer);

    chars = strlen(inputbox->buffer);

    if (chars < w && inputbox->editing && selected)
    {
        TXT_BGColor(TXT_COLOR_BLACK, 1);
        TXT_DrawString("_");
        ++chars;
    }
    
    for (i=chars; i < w; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_InputBoxDestructor(TXT_UNCAST_ARG(inputbox))
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    free(inputbox->buffer);
}

static void Backspace(txt_inputbox_t *inputbox)
{
    if (strlen(inputbox->buffer) > 0)
    {
        inputbox->buffer[strlen(inputbox->buffer) - 1] = '\0';
    }
}

static void AddCharacter(txt_inputbox_t *inputbox, int key)
{
    if (strlen(inputbox->buffer) < inputbox->size)
    {
        // Add character to the buffer

        inputbox->buffer[strlen(inputbox->buffer) + 1] = '\0';
        inputbox->buffer[strlen(inputbox->buffer)] = key;
    }
}

static int TXT_InputBoxKeyPress(TXT_UNCAST_ARG(inputbox), int key)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    if (!inputbox->editing)
    {
        if (key == KEY_ENTER)
        {
            SetBufferFromValue(inputbox);
            inputbox->editing = 1;
            return 1;
        }

        return 0;
    }

    if (key == KEY_ENTER)
    {
        free(*((char **)inputbox->value));
        *((char **) inputbox->value) = strdup(inputbox->buffer);

        TXT_EmitSignal(&inputbox->widget, "changed");

        inputbox->editing = 0;
    }

    if (key == KEY_ESCAPE)
    {
        inputbox->editing = 0;
    }

    if (isprint(key))
    {
        // Add character to the buffer

        AddCharacter(inputbox, key);
    }

    if (key == KEY_BACKSPACE)
    {
        Backspace(inputbox);
    }
    
    return 1;
}

static int TXT_IntInputBoxKeyPress(TXT_UNCAST_ARG(inputbox), int key)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    if (!inputbox->editing)
    {
        if (key == KEY_ENTER)
        {
            strcpy(inputbox->buffer, "");
            inputbox->editing = 1;
            return 1;
        }

        return 0;
    }

    if (key == KEY_ENTER)
    {
        *((int *) inputbox->value) = atoi(inputbox->buffer);

        inputbox->editing = 0;
    }

    if (key == KEY_ESCAPE)
    {
        inputbox->editing = 0;
    }

    if (isdigit(key))
    {
        // Add character to the buffer

        AddCharacter(inputbox, key);
    }

    if (key == KEY_BACKSPACE)
    {
        Backspace(inputbox);
    }
    
    return 1;
}

static void TXT_InputBoxMousePress(TXT_UNCAST_ARG(inputbox),
                                   int x, int y, int b)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    if (b == TXT_MOUSE_LEFT)
    {
        // Make mouse clicks start editing the box

        if (!inputbox->editing)
        {
            // Send a simulated keypress to start editing

            TXT_WidgetKeyPress(inputbox, KEY_ENTER);
        }
    }
}

txt_widget_class_t txt_inputbox_class =
{
    TXT_AlwaysSelectable,
    TXT_InputBoxSizeCalc,
    TXT_InputBoxDrawer,
    TXT_InputBoxKeyPress,
    TXT_InputBoxDestructor,
    TXT_InputBoxMousePress,
    NULL,
};

txt_widget_class_t txt_int_inputbox_class =
{
    TXT_AlwaysSelectable,
    TXT_InputBoxSizeCalc,
    TXT_InputBoxDrawer,
    TXT_IntInputBoxKeyPress,
    TXT_InputBoxDestructor,
    TXT_InputBoxMousePress,
    NULL,
};

static void SetBufferFromValue(txt_inputbox_t *inputbox)
{
    if (inputbox->widget.widget_class == &txt_inputbox_class)
    {
        char **value = (char **) inputbox->value;

        if (*value != NULL)
        {
            strncpy(inputbox->buffer, *value, inputbox->size);
            inputbox->buffer[inputbox->size] = '\0';
        }
        else
        {
            strcpy(inputbox->buffer, "");
        }
    }
    else if (inputbox->widget.widget_class == &txt_int_inputbox_class)
    {
        int *value = (int *) inputbox->value;
        sprintf(inputbox->buffer, "%i", *value);
    }
}

txt_inputbox_t *TXT_NewInputBox(char **value, int size)
{
    txt_inputbox_t *inputbox;

    inputbox = malloc(sizeof(txt_inputbox_t));

    TXT_InitWidget(inputbox, &txt_inputbox_class);
    inputbox->value = value;
    inputbox->size = size;
    inputbox->buffer = malloc(size + 1);
    inputbox->editing = 0;

    return inputbox;
}

txt_inputbox_t *TXT_NewIntInputBox(int *value, int size)
{
    txt_inputbox_t *inputbox;

    inputbox = malloc(sizeof(txt_inputbox_t));

    TXT_InitWidget(inputbox, &txt_int_inputbox_class);
    inputbox->value = value;
    inputbox->size = size;
    inputbox->buffer = malloc(15);
    inputbox->editing = 0;
    return inputbox;
}

