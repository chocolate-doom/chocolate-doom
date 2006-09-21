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
    if (val < 0)
    {
        return ((int) log(-val)) + 2;
    }
    else
    {
        return ((int) log(val)) + 1;
    }
}

static void TXT_SpinControlSizeCalc(TXT_UNCAST_ARG(spincontrol))
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);
    int minw, maxw;
    int w;

    minw = IntWidth(spincontrol->min);
    maxw = IntWidth(spincontrol->max);
    
    if (minw > maxw)
    {
        w = minw;
    }
    else
    {
        w = maxw;
    }

    spincontrol->widget.w = w + 4;
    spincontrol->widget.h = 1;
}

static void TXT_SpinControlDrawer(TXT_UNCAST_ARG(spincontrol), int selected)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);
    char buf[20];
    int i;

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_BGColor(TXT_COLOR_BLUE, 0);

    TXT_DrawString("< ");
    
    // Choose background color

    if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }
    else
    {
        TXT_BGColor(TXT_COLOR_BLUE, 0);
    }

    sprintf(buf, "%i", *spincontrol->value);

    for (i=strlen(buf); i<spincontrol->widget.w - 4; ++i)
    {
        TXT_DrawString(" ");
    }

    TXT_DrawString(buf);

    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_DrawString(" >");
}

static void TXT_SpinControlDestructor(TXT_UNCAST_ARG(spincontrol))
{
}

static int TXT_SpinControlKeyPress(TXT_UNCAST_ARG(spincontrol), int key)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);

    if (key == KEY_LEFTARROW)
    {
        --*spincontrol->value;

        if (*spincontrol->value < spincontrol->min)
        {
            *spincontrol->value = spincontrol->min;
        }

        return 1;
    }
    
    if (key == KEY_RIGHTARROW)
    {
        ++*spincontrol->value;

        if (*spincontrol->value > spincontrol->max)
        {
            *spincontrol->value = spincontrol->max;
        }

        return 1;
    }
    
    return 0;
}

static void TXT_SpinControlMousePress(TXT_UNCAST_ARG(spincontrol),
                                   int x, int y, int b)
{
    TXT_CAST_ARG(txt_spincontrol_t, spincontrol);
    int rel_x;

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
    0,
};

txt_spincontrol_t *TXT_NewSpinControl(int *value, int min, int max)
{
    txt_spincontrol_t *spincontrol;

    spincontrol = malloc(sizeof(txt_spincontrol_t));

    TXT_InitWidget(spincontrol, &txt_spincontrol_class);
    spincontrol->value = value;
    spincontrol->min = min;
    spincontrol->max = max;

    return spincontrol;
}


