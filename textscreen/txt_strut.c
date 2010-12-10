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

#include "txt_strut.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_StrutSizeCalc(TXT_UNCAST_ARG(strut))
{
    TXT_CAST_ARG(txt_strut_t, strut);

    // Minimum width is the string length + two spaces for padding

    strut->widget.w = strut->width;
    strut->widget.h = strut->height;
}

static void TXT_StrutDrawer(TXT_UNCAST_ARG(strut), int selected)
{
    // Nothing is drawn for a strut.
}

static void TXT_StrutDestructor(TXT_UNCAST_ARG(strut))
{
}

static int TXT_StrutKeyPress(TXT_UNCAST_ARG(strut), int key)
{
    return 0;
}

txt_widget_class_t txt_strut_class =
{
    TXT_NeverSelectable,
    TXT_StrutSizeCalc,
    TXT_StrutDrawer,
    TXT_StrutKeyPress,
    TXT_StrutDestructor,
    NULL,
    NULL,
};

txt_strut_t *TXT_NewStrut(int width, int height)
{
    txt_strut_t *strut;

    strut = malloc(sizeof(txt_strut_t));

    TXT_InitWidget(strut, &txt_strut_class);
    strut->width = width;
    strut->height = height;

    return strut;
}

