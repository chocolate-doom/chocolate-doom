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
#include <math.h>

#include "doomkeys.h"

#include "txt_spinctrl.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

// Number of characters needed to represent a character 

static int IntWidth(int val)
{
    char buf[15];

    sprintf(buf, "%i", val);

    return strlen(buf);
}

// Returns the minimum width of the input box

static int SpinControlWidth(txt_spincontrol_t *spincontrol)
{
    int minw, maxw;

    minw = IntWidth(spincontrol->min);
    maxw = IntWidth(spincontrol->max);
    
    // Choose the wider of the two values.  Add one so that there is always
    // space for the cursor when editing.

    if (minw > maxw)
    {
        return minw;
    }
    else
    {
        return maxw;
    }
}

static void TXT_SpinControlSizeCalc(TXT_UNCAST_ARG(spincontrol))
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);

    spincontrol->widget.w = SpinControlWidth(spincontrol) + 5;
    spincontrol->widget.h = 1;
}

static void TXT_SpinControlDrawer(TXT_UNCAST_ARG(spincontrol), int selected)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);
    char buf[15];
    unsigned int i;
    unsigned int padding;

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_BGColor(TXT_COLOR_BLUE, 0);

    TXT_DrawString("< ");
    
    // Choose background color

    if (selected && spincontrol->editing)
    {
        TXT_BGColor(TXT_COLOR_BLACK, 0);
    }
    else if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }
    else
    {
        TXT_BGColor(TXT_COLOR_BLUE, 0);
    }

    if (spincontrol->editing)
    {
        strcpy(buf, spincontrol->buffer);
    }
    else
    {
        sprintf(buf, "%i", *spincontrol->value);
    }

    i = 0;

    padding = spincontrol->widget.w - strlen(buf) - 4;

    while (i < padding)
    {
        TXT_DrawString(" ");
        ++i;
    }

    TXT_DrawString(buf);
    i += strlen(buf);

    while (i < spincontrol->widget.w - 4)
    {
        TXT_DrawString(" ");
        ++i;
    }

    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_DrawString(" >");
}

static void TXT_SpinControlDestructor(TXT_UNCAST_ARG(spincontrol))
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);

    free(spincontrol->buffer);
}

static void AddCharacter(txt_spincontrol_t *spincontrol, int key)
{
    if (strlen(spincontrol->buffer) < SpinControlWidth(spincontrol))
    {
        spincontrol->buffer[strlen(spincontrol->buffer) + 1] = '\0';
        spincontrol->buffer[strlen(spincontrol->buffer)] = key;
    }
}

static void Backspace(txt_spincontrol_t *spincontrol)
{
    if (strlen(spincontrol->buffer) > 0)
    {
        spincontrol->buffer[strlen(spincontrol->buffer) - 1] = '\0';
    }
}

static void EnforceLimits(txt_spincontrol_t *spincontrol)
{
    if (*spincontrol->value > spincontrol->max)
        *spincontrol->value = spincontrol->max;
    else if (*spincontrol->value < spincontrol->min)
        *spincontrol->value = spincontrol->min;
}

static int TXT_SpinControlKeyPress(TXT_UNCAST_ARG(spincontrol), int key)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);

    // Enter to enter edit mode

    if (spincontrol->editing)
    {
        if (key == KEY_ENTER)
        {
            *spincontrol->value = atoi(spincontrol->buffer);
            spincontrol->editing = 0;
            EnforceLimits(spincontrol);
            return 1;
        }

        if (key == KEY_ESCAPE)
        {
            // Abort without saving value
            spincontrol->editing = 0;
            return 1;
        }

        if (isdigit(key) || key == '-')
        {
            AddCharacter(spincontrol, key);
            return 1;
        }

        if (key == KEY_BACKSPACE)
        {
            Backspace(spincontrol);
            return 1;
        }
    }
    else
    {
        // Non-editing mode

        if (key == KEY_ENTER)
        {
            sprintf(spincontrol->buffer, "%i", *spincontrol->value);
            spincontrol->editing = 1;
            return 1;
        }
        if (key == KEY_LEFTARROW)
        {
            --*spincontrol->value;

            EnforceLimits(spincontrol);

            return 1;
        }
        
        if (key == KEY_RIGHTARROW)
        {
            ++*spincontrol->value;

            EnforceLimits(spincontrol);

            return 1;
        }
    }

    return 0;
}

static void TXT_SpinControlMousePress(TXT_UNCAST_ARG(spincontrol),
                                   int x, int y, int b)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);
    unsigned int rel_x;

    rel_x = x - spincontrol->widget.x;

    if (rel_x < 2)
    {
        TXT_SpinControlKeyPress(spincontrol, KEY_LEFTARROW);
    }
    else if (rel_x >= spincontrol->widget.w - 2)
    {
        TXT_SpinControlKeyPress(spincontrol, KEY_RIGHTARROW);
    }
}

txt_widget_class_t txt_spincontrol_class =
{
    TXT_SpinControlSizeCalc,
    TXT_SpinControlDrawer,
    TXT_SpinControlKeyPress,
    TXT_SpinControlDestructor,
    TXT_SpinControlMousePress,
    NULL,
};

txt_spincontrol_t *TXT_NewSpinControl(int *value, int min, int max)
{
    txt_spincontrol_t *spincontrol;

    spincontrol = malloc(sizeof(txt_spincontrol_t));

    TXT_InitWidget(spincontrol, &txt_spincontrol_class);
    spincontrol->value = value;
    spincontrol->min = min;
    spincontrol->max = max;
    spincontrol->buffer = malloc(15);
    strcpy(spincontrol->buffer, "");
    spincontrol->editing = 0;

    return spincontrol;
}


