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

// Generate the format string to be used for displaying floats

static void FloatFormatString(float step, char *buf)
{
    int precision;

    precision = (int) ceil(-log(step) / log(10));

    if (precision > 0)
    {
        sprintf(buf, "%%.%if", precision);
    }
    else
    {
        strcpy(buf, "%.1f");
    }
}

// Number of characters needed to represent a character 

static unsigned int IntWidth(int val)
{
    char buf[25];

    sprintf(buf, "%i", val);

    return strlen(buf);
}

static unsigned int FloatWidth(float val, float step)
{
    unsigned int precision;
    unsigned int result;

    // Calculate the width of the int value

    result = IntWidth((int) val);

    // Add a decimal part if the precision specifies it

    precision = (unsigned int) ceil(-log(step) / log(10));

    if (precision > 0)
    {
        result += precision + 1;
    }    

    return result;
}

// Returns the minimum width of the input box

static unsigned int SpinControlWidth(txt_spincontrol_t *spincontrol)
{
    unsigned int minw, maxw;

    switch (spincontrol->type)
    {
        case TXT_SPINCONTROL_FLOAT:
            minw = FloatWidth(spincontrol->min.f, spincontrol->step.f);
            maxw = FloatWidth(spincontrol->max.f, spincontrol->step.f);
            break;

        default:
        case TXT_SPINCONTROL_INT:
            minw = IntWidth(spincontrol->min.i);
            maxw = IntWidth(spincontrol->max.i);
            break;

    }
    
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

static void SetBuffer(txt_spincontrol_t *spincontrol)
{
    char format[25];

    switch (spincontrol->type)
    {
        case TXT_SPINCONTROL_INT:
            sprintf(spincontrol->buffer, "%i", spincontrol->value->i);
            break;

        case TXT_SPINCONTROL_FLOAT:
            FloatFormatString(spincontrol->step.f, format);
            sprintf(spincontrol->buffer, format, spincontrol->value->f);
            break;
    }
}

static void TXT_SpinControlDrawer(TXT_UNCAST_ARG(spincontrol), int selected)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);
    unsigned int i;
    unsigned int padding;

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_BGColor(TXT_WINDOW_BACKGROUND, 0);

    TXT_DrawString("\x1b ");
    
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    // Choose background color

    if (selected && spincontrol->editing)
    {
        TXT_BGColor(TXT_COLOR_BLACK, 0);
    }
    else
    {
        TXT_SetWidgetBG(spincontrol, selected);
    }

    if (!spincontrol->editing)
    {
        SetBuffer(spincontrol);
    }
    
    i = 0;

    padding = spincontrol->widget.w - strlen(spincontrol->buffer) - 4;

    while (i < padding)
    {
        TXT_DrawString(" ");
        ++i;
    }

    TXT_DrawString(spincontrol->buffer);
    i += strlen(spincontrol->buffer);

    while (i < spincontrol->widget.w - 4)
    {
        TXT_DrawString(" ");
        ++i;
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_BGColor(TXT_WINDOW_BACKGROUND, 0);
    TXT_DrawString(" \x1a");
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
    switch (spincontrol->type)
    {
        case TXT_SPINCONTROL_INT:
            if (spincontrol->value->i > spincontrol->max.i)
                spincontrol->value->i = spincontrol->max.i;
            else if (spincontrol->value->i < spincontrol->min.i)
                spincontrol->value->i = spincontrol->min.i;
            break;

        case TXT_SPINCONTROL_FLOAT:
            if (spincontrol->value->f > spincontrol->max.f)
                spincontrol->value->f = spincontrol->max.f;
            else if (spincontrol->value->f < spincontrol->min.f)
                spincontrol->value->f = spincontrol->min.f;
            break;
    }
}

static int TXT_SpinControlKeyPress(TXT_UNCAST_ARG(spincontrol), int key)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);

    // Enter to enter edit mode

    if (spincontrol->editing)
    {
        if (key == KEY_ENTER)
        {
            switch (spincontrol->type)
            {
                case TXT_SPINCONTROL_INT:
                    spincontrol->value->i = atoi(spincontrol->buffer);
                    break;

                case TXT_SPINCONTROL_FLOAT:
                    spincontrol->value->f = (float) atof(spincontrol->buffer);
                    break;
            }

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

        if (isdigit(key) || key == '-' || key == '.')
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
            spincontrol->editing = 1;
            strcpy(spincontrol->buffer, "");
            return 1;
        }
        if (key == KEY_LEFTARROW)
        {
            switch (spincontrol->type)
            {
                case TXT_SPINCONTROL_INT:
                    spincontrol->value->i -= spincontrol->step.i;
                    break;

                case TXT_SPINCONTROL_FLOAT:
                    spincontrol->value->f -= spincontrol->step.f;
                    break;
            }

            EnforceLimits(spincontrol);

            return 1;
        }
        
        if (key == KEY_RIGHTARROW)
        {
            switch (spincontrol->type)
            {
                case TXT_SPINCONTROL_INT:
                    spincontrol->value->i += spincontrol->step.i;
                    break;

                case TXT_SPINCONTROL_FLOAT:
                    spincontrol->value->f += spincontrol->step.f;
                    break;
            }

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
    TXT_AlwaysSelectable,
    TXT_SpinControlSizeCalc,
    TXT_SpinControlDrawer,
    TXT_SpinControlKeyPress,
    TXT_SpinControlDestructor,
    TXT_SpinControlMousePress,
    NULL,
};

static txt_spincontrol_t *TXT_BaseSpinControl(void)
{
    txt_spincontrol_t *spincontrol;

    spincontrol = malloc(sizeof(txt_spincontrol_t));

    TXT_InitWidget(spincontrol, &txt_spincontrol_class);
    spincontrol->buffer = malloc(25);
    strcpy(spincontrol->buffer, "");
    spincontrol->editing = 0;

    return spincontrol;
}

txt_spincontrol_t *TXT_NewSpinControl(int *value, int min, int max)
{
    txt_spincontrol_t *spincontrol;

    spincontrol = TXT_BaseSpinControl();
    spincontrol->type = TXT_SPINCONTROL_INT;
    spincontrol->value = (void *) value;
    spincontrol->min.i = min;
    spincontrol->max.i = max;
    spincontrol->step.i = 1;

    return spincontrol;
}

txt_spincontrol_t *TXT_NewFloatSpinControl(float *value, float min, float max)
{
    txt_spincontrol_t *spincontrol;

    spincontrol = TXT_BaseSpinControl();
    spincontrol->type = TXT_SPINCONTROL_FLOAT;
    spincontrol->value = (void *) value;
    spincontrol->min.f = min;
    spincontrol->max.f = max;
    spincontrol->step.f = 0.1f;

    return spincontrol;
}

