//
// Copyright(C) 2005-2014 Simon Howard
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

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "textscreen.h"
#include "m_config.h"
#include "m_misc.h"
#include "mode.h"

#include "display.h"
#include "config.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-display"

extern void RestartTextscreen(void);

typedef struct
{
    int w, h;
} screen_mode_t;

// List of aspect ratio-uncorrected modes

static screen_mode_t screen_modes_unscaled[] = 
{
    { 320,  200 },
    { 640,  400 },
    { 960,  600 },
    { 1280, 800 },
    { 1600, 1000 },
    { 0, 0},
};

// List of aspect ratio-corrected modes

static screen_mode_t screen_modes_scaled[] = 
{
    { 320,  240 },
    { 512,  400 },
    { 640,  480 },
    { 800,  600 },
    { 960,  720 },
    { 1024, 800 },
    { 1280, 960 },
    { 1600, 1200 },
    { 1920, 1440 },
    { 0, 0},
};

static int vidmode = 0;

static char *video_driver = "";
static char *window_position = "";
static int aspect_ratio_correct = 1;
static int fullscreen = 1;
static int fullscreen_width = 0, fullscreen_height = 0;
static int window_width = 640, window_height = 480;
static int startup_delay = 1000;
static int usegamma = 0;

int graphical_startup = 1;
int show_endoom = 1;
int show_diskicon = 1;
int png_screenshots = 0;

// These are the last window width/height values that were chosen by the
// user.  These are used when finding the "nearest" mode, so when
// changing the fullscreen / aspect ratio options, the setting does not
// jump around.

static int selected_window_width = 0, selected_window_height;

static int system_video_env_set;

// Set the SDL_VIDEODRIVER environment variable

void SetDisplayDriver(void)
{
    static int first_time = 1;

    if (first_time)
    {
        system_video_env_set = getenv("SDL_VIDEODRIVER") != NULL;

        first_time = 0;
    }
    
    // Don't override the command line environment, if it has been set.

    if (system_video_env_set)
    {
        return;
    }

    // Use the value from the configuration file, if it has been set.

    if (strcmp(video_driver, "") != 0)
    {
        char *env_string;

        env_string = M_StringJoin("SDL_VIDEODRIVER=", video_driver, NULL);
        putenv(env_string);
        free(env_string);
    }
}

static void ModeSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(mode))
{
    TXT_CAST_ARG(screen_mode_t, mode);

    window_width = mode->w;
    window_height = mode->h;

    // This is now the most recently selected screen width

    selected_window_width = window_width;
    selected_window_height = window_height;
}

static int FindBestMode(screen_mode_t *modes)
{
    int i;
    int best_mode;
    int best_mode_diff;
    int diff;

    best_mode = -1;
    best_mode_diff = 0;

    for (i=0; modes[i].w != 0; ++i)
    {
        diff = (selected_window_width - modes[i].w)
                  * (selected_window_width - modes[i].w) 
             + (selected_window_height - modes[i].h)
                  * (selected_window_height - modes[i].h);

        if (best_mode == -1 || diff < best_mode_diff)
        {
            best_mode_diff = diff;
            best_mode = i;
        }
    }

    return best_mode;
}

static void GenerateModesTable(TXT_UNCAST_ARG(widget),
                               TXT_UNCAST_ARG(modes_table))
{
    TXT_CAST_ARG(txt_table_t, modes_table);
    char buf[15];
    screen_mode_t *modes;
    txt_radiobutton_t *rbutton;
    int i;

    // Pick which modes list to use

    if (aspect_ratio_correct)
    {
        modes = screen_modes_scaled;
    }
    else
    {
        modes = screen_modes_unscaled;
    }

    // Build the table
 
    TXT_ClearTable(modes_table);
    TXT_SetColumnWidths(modes_table, 14, 14, 14, 14, 14);

    for (i=0; modes[i].w != 0; ++i)
    {
        M_snprintf(buf, sizeof(buf), "%ix%i", modes[i].w, modes[i].h);
        rbutton = TXT_NewRadioButton(buf, &vidmode, i);
        TXT_AddWidget(modes_table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }

    // Find the nearest mode in the list that matches the current
    // settings

    vidmode = FindBestMode(modes);

    if (vidmode > 0)
    {
        window_width = modes[vidmode].w;
        window_height = modes[vidmode].h;
    }
}

static void AdvancedDisplayConfig(TXT_UNCAST_ARG(widget),
                                  TXT_UNCAST_ARG(modes_table))
{
    TXT_CAST_ARG(txt_table_t, modes_table);
    txt_window_t *window;
    txt_checkbox_t *ar_checkbox;

    window = TXT_NewWindow("Advanced display options");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 35);

    TXT_AddWidgets(window,
                   ar_checkbox = TXT_NewCheckBox("Fix aspect ratio",
                                                 &aspect_ratio_correct),
                   NULL);

    if (gamemission == heretic || gamemission == hexen || gamemission == strife)
    {
        TXT_AddWidget(window,
                      TXT_NewCheckBox("Graphical startup", &graphical_startup));
    }

    if (gamemission == doom || gamemission == heretic || gamemission == strife)
    {
        TXT_AddWidget(window,
                      TXT_NewCheckBox("Show ENDOOM screen on exit",
                                      &show_endoom));
    }

    TXT_AddWidget(window,
                  TXT_NewCheckBox("Save screenshots in PNG format",
                                  &png_screenshots));

    TXT_SignalConnect(ar_checkbox, "changed", GenerateModesTable, modes_table);
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *modes_table;
    txt_window_action_t *advanced_button;
    txt_checkbox_t *fs_checkbox;
    int num_columns;
    int num_rows;
    int window_y;

    // First time in? Initialise selected_screen_{width,height}

    if (selected_window_width == 0)
    {
        selected_window_width = window_width;
        selected_window_height = window_height;
    }

    // Open the window

    window = TXT_NewWindow("Display Configuration");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    // Some machines can have lots of video modes.  This tries to
    // keep a limit of six lines by increasing the number of
    // columns.  In extreme cases, the window is moved up slightly.

    num_columns = 3;
    modes_table = TXT_NewTable(num_columns);

    // Build window:

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Window size"),
                   modes_table,
                   TXT_NewSeparator("Options"),
                   fs_checkbox = TXT_NewCheckBox("Full screen", &fullscreen),
                   NULL);

    TXT_SignalConnect(fs_checkbox, "changed",
                      GenerateModesTable, modes_table);

    // How many rows high will the configuration window be?
    // Need to take into account number of fullscreen modes, and also
    // number of supported pixel depths.
    // The windowed modes list is four rows, so take the maximum of
    // windowed and fullscreen.

    num_rows = (num_columns - 1) / num_columns;

    if (num_rows < 4)
    {
        num_rows = 4;
    }

    if (num_rows < 14)
    {
        window_y = 8 - ((num_rows + 1) / 2);
    }
    else
    {
        window_y = 1;
    }

    // The window is set at a fixed vertical position.  This keeps
    // the top of the window stationary when switching between
    // fullscreen and windowed mode (which causes the window's
    // height to change).

    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP, 
                                  TXT_SCREEN_W / 2, window_y);

    GenerateModesTable(NULL, modes_table);

    // Button to open "advanced" window.
    // Need to pass a pointer to the modes table, as some of the options
    // in there trigger a rebuild of it.

    advanced_button = TXT_NewWindowAction('a', "Advanced");
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, advanced_button);
    TXT_SignalConnect(advanced_button, "pressed",
                      AdvancedDisplayConfig, modes_table);
}

void BindDisplayVariables(void)
{
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("fullscreen_width",          &fullscreen_width);
    M_BindIntVariable("fullscreen_height",         &fullscreen_height);
    M_BindIntVariable("window_width",              &window_width);
    M_BindIntVariable("window_height",             &window_height);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindStringVariable("window_position",        &window_position);
    M_BindIntVariable("usegamma",                  &usegamma);
    M_BindIntVariable("png_screenshots",           &png_screenshots);

    if (gamemission == doom || gamemission == heretic
     || gamemission == strife)
    {
        M_BindIntVariable("show_endoom",               &show_endoom);
    }

    if (gamemission == doom || gamemission == strife)
    {
        M_BindIntVariable("show_diskicon",             &show_diskicon);
    }

    if (gamemission == heretic || gamemission == hexen || gamemission == strife)
    {
        M_BindIntVariable("graphical_startup",        &graphical_startup);
    }
}
