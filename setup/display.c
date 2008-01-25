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
#include "textscreen.h"

static int vidmode = 1;
static int fullscreen = 0;
static int grabmouse = 1;

static char *modes[] = { "320x200", "640x400" };

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *box;
    
    window = TXT_NewWindow("Display Configuration");

    box = TXT_NewTable(2);
    TXT_AddWidget(box, TXT_NewLabel("Screen mode: "));
    TXT_AddWidget(box, TXT_NewDropdownList(&vidmode, modes, 2));
    TXT_AddWidget(window, box);

    TXT_AddWidget(window, TXT_NewCheckBox("Fullscreen", &fullscreen));
    TXT_AddWidget(window, TXT_NewCheckBox("Grab mouse", &grabmouse));
}

