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

typedef struct 
{
        char *description;
        int fullscreen;
        int screenmult;
} vidmode_t;

static vidmode_t modes[] = 
{
        { "320x200",  0, 1 },
        { "640x400",  0, 2 },
        { "960x600",  0, 3 },
        { "1280x800", 0, 4 },
        { "320x200",  1, 1 },
        { "320x240",  2, 1 },
        { "640x400",  1, 2 },
        { "640x480",  2, 2 },
        { "960x600",  1, 3 },
        { "960x720",  2, 3 },
        { "1280x800", 1, 4 },
        { "1280x960", 2, 4 },
        { NULL },
};

static int vidmode = 0;
static int fullscreen = 0;
static int screenmult = 1;
static int grabmouse = 1;

// Given the video settings (fullscreen, screenmult, etc), find the
// current video mode

static void SetCurrentMode(void)
{
        int i;

        vidmode = 0;

        for (i=0; modes[i].description != NULL; ++i)
        {
                if (fullscreen == modes[i].fullscreen
                 && screenmult == modes[i].screenmult)
                {
                        vidmode = i;
                        break;
                }
        }
}

static void ModeSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(mode))
{
        TXT_CAST_ARG(vidmode_t, mode);

        fullscreen = mode->fullscreen;
        screenmult = mode->screenmult;
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *table;
    txt_radiobutton_t *rbutton;
    int i;

    // Find the current mode

    SetCurrentMode();

    // Open the window
    
    window = TXT_NewWindow("Display Configuration");

    TXT_AddWidget(window, TXT_NewSeparator("Windowed modes"));

    table = TXT_NewTable(2);
    
    for (i=0; modes[i].fullscreen == 0; ++i)
    {
        rbutton = TXT_NewRadioButton(modes[i].description, &vidmode, i);
        TXT_AddWidget(table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }

    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator("Fullscreen modes"));

    table = TXT_NewTable(2);

    for (; modes[i].description != NULL; ++i)
    {
        rbutton = TXT_NewRadioButton(modes[i].description, &vidmode, i);
        TXT_AddWidget(table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }

    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator(NULL));
    TXT_AddWidget(window, TXT_NewCheckBox("Grab mouse", &grabmouse));
}

