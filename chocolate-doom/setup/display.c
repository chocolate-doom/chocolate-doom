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

#include <string.h>

#include "textscreen.h"

#include "display.h"

typedef struct 
{
    char *description;
    char *description_4_3;
    int screenmultiply;
    txt_radiobutton_t *widget;
} vidmode_t;

enum
{
    RATIO_CORRECT_NONE,
    RATIO_CORRECT_LETTERBOX,
    RATIO_CORRECT_STRETCH,
    NUM_RATIO_CORRECT,
};

static vidmode_t modes[] = 
{
    { "320x200",  "320x240",  1, NULL },
    { "960x600",  "960x720",  3, NULL },
    { "640x400",  "640x480",  2, NULL },
    { "1280x800", "1280x960", 4, NULL },
    { "1600x1000", "1600x1200", 5, NULL },
    { NULL,       NULL,       0, NULL },
};

static char *aspect_ratio_strings[] =
{
    "Disabled",
    "Letterbox mode",
    "Stretch to 4:3",
};

static int vidmode = 0;

char *video_driver = "";
int autoadjust_video_settings = 1;
int aspect_ratio_correct = RATIO_CORRECT_NONE;
int fullscreen = 1;
int screenmultiply = 1;
int startup_delay = 0;
int show_endoom = 1;

#ifdef _WIN32

static int win32_video_driver = 0;

static char *win32_video_drivers[] = 
{
    "DirectX",
    "Windows GDI",
};

static void SetWin32VideoDriver(void)
{
    if (!strcmp(video_driver, "windib"))
    {
        win32_video_driver = 1;
    }
    else
    {
        win32_video_driver = 0;
    }
}

static void UpdateVideoDriver(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    char *drivers[] = 
    {
        "",
        "windib",
    };

    video_driver = drivers[win32_video_driver];
}

#endif

// Given the video settings (fullscreen, screenmultiply, etc), find the
// current video mode

static void SetCurrentMode(void)
{
    int i;

    vidmode = 0;

    for (i=0; modes[i].description != NULL; ++i)
    {
        if (screenmultiply == modes[i].screenmultiply)
        {
            vidmode = i;
            break;
        }
    }
}

static void ModeSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(mode))
{
    TXT_CAST_ARG(vidmode_t, mode);

    screenmultiply = mode->screenmultiply;
}

static void UpdateModes(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    int i;

    for (i=0; modes[i].description != NULL; ++i)
    {
        if (aspect_ratio_correct == RATIO_CORRECT_NONE)
        {
            TXT_SetRadioButtonLabel(modes[i].widget, modes[i].description);
        }
        else
        {
            TXT_SetRadioButtonLabel(modes[i].widget, modes[i].description_4_3);
        }
    }
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *ar_table;
    txt_table_t *modes_table;
    txt_radiobutton_t *rbutton;
    txt_dropdown_list_t *ar_dropdown;
    int i;

    // Find the current mode

    SetCurrentMode();

    // Open the window
    
    window = TXT_NewWindow("Display Configuration");

    TXT_AddWidgets(window, 
                   TXT_NewCheckBox("Fullscreen", &fullscreen),
                   ar_table = TXT_NewTable(2),
                   TXT_NewSeparator("Screen mode"),
                   modes_table = TXT_NewTable(2),
                   TXT_NewSeparator("Misc."),
                   TXT_NewCheckBox("Show ENDOOM screen", &show_endoom),
                   NULL);

    TXT_SetColumnWidths(ar_table, 25, 0);

#ifdef _WIN32
    {
        txt_dropdown_list_t *driver_list;

        driver_list = TXT_NewDropdownList(&win32_video_driver,
                                          win32_video_drivers,
                                          2);

        TXT_SignalConnect(driver_list, "changed", UpdateVideoDriver, NULL);
        SetWin32VideoDriver();

        TXT_AddWidgets(ar_table,
                       TXT_NewLabel("Video driver"),
                       driver_list,
                       NULL);
    }
#endif

    TXT_AddWidgets(ar_table,
                   TXT_NewLabel("Aspect ratio correction"),
                   ar_dropdown = TXT_NewDropdownList(&aspect_ratio_correct,
                                                     aspect_ratio_strings,
                                                     NUM_RATIO_CORRECT),
                   NULL);

    TXT_SignalConnect(ar_dropdown, "changed", UpdateModes, NULL);

    TXT_SetColumnWidths(modes_table, 14, 14);

    for (i=0; modes[i].description != NULL; ++i)
    {
        rbutton = TXT_NewRadioButton(modes[i].description, &vidmode, i);
        modes[i].widget = rbutton;
        TXT_AddWidget(modes_table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }

    UpdateModes(NULL, NULL);
}

