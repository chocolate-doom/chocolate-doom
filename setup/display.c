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

#include "display.h"

typedef struct 
{
    char *description;
    int fullscreen;
    int screenmultiply;
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
    { NULL,       0, 0 },
};

static int vidmode = 0;

int fullscreen = 0;
int screenmultiply = 1;
int startup_delay = 0;
int show_endoom = 1;

// Given the video settings (fullscreen, screenmultiply, etc), find the
// current video mode

static void SetCurrentMode(void)
{
    int i;

    vidmode = 0;

    for (i=0; modes[i].description != NULL; ++i)
    {
        if (fullscreen == modes[i].fullscreen
         && screenmultiply == modes[i].screenmultiply)
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
    screenmultiply = mode->screenmultiply;
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *windowed_table;
    txt_table_t *fullscreen_table;
    txt_table_t *misc_table;
    txt_radiobutton_t *rbutton;
    int i;

    // Find the current mode

    SetCurrentMode();

    // Open the window
    
    window = TXT_NewWindow("Display Configuration");

    TXT_AddWidgets(window, 
                   TXT_NewSeparator("Fullscreen modes"),
                   fullscreen_table = TXT_NewTable(2),
                   TXT_NewSeparator("Windowed modes"),
                   windowed_table = TXT_NewTable(2),
                   TXT_NewSeparator("Misc."),
                   TXT_NewCheckBox("Show ENDOOM screen", &show_endoom),
                   misc_table = TXT_NewTable(2),
                   NULL);

    TXT_SetColumnWidths(windowed_table, 14, 14);
    
    for (i=0; modes[i].fullscreen == 0; ++i)
    {
        rbutton = TXT_NewRadioButton(modes[i].description, &vidmode, i);
        TXT_AddWidget(windowed_table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }

    TXT_SetColumnWidths(fullscreen_table, 14, 14);

    for (; modes[i].description != NULL; ++i)
    {
        rbutton = TXT_NewRadioButton(modes[i].description, &vidmode, i);
        TXT_AddWidget(fullscreen_table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }

    TXT_SetColumnWidths(misc_table, 22, 5);

    TXT_AddWidgets(misc_table,
                   TXT_NewLabel("Startup delay (ms)"),
                   TXT_NewIntInputBox(&startup_delay, 5),
                   NULL);
}

