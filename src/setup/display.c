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
    { 256,  200 },
    { 320,  240 },
    { 512,  400 },
    { 640,  480 },
    { 800,  600 },
    { 960,  720 },
    { 1024, 800 },
    { 1280, 960 },
    { 1600, 1200 },
    { 0, 0},
};

// List of fullscreen modes generated at runtime

static screen_mode_t *screen_modes_fullscreen = NULL;
static int num_screen_modes_fullscreen;

static int vidmode = 0;

static char *video_driver = "";
static char *window_position = "";
static int autoadjust_video_settings = 1;
static int aspect_ratio_correct = 1;
static int fullscreen = 1;
static int screen_width = 320;
static int screen_height = 200;
static int startup_delay = 1000;
static int usegamma = 0;

int graphical_startup = 1;
int show_endoom = 1;
int png_screenshots = 0;

// These are the last screen width/height values that were chosen by the
// user.  These are used when finding the "nearest" mode, so when 
// changing the fullscreen / aspect ratio options, the setting does not
// jump around.

static int selected_screen_width = 0, selected_screen_height;

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

    screen_width = mode->w;
    screen_height = mode->h;

    // This is now the most recently selected screen width

    selected_screen_width = screen_width;
    selected_screen_height = screen_height;
}

static int GoodFullscreenMode(screen_mode_t *mode)
{
    int w, h;

    w = mode->w;
    h = mode->h;

    // 320x200 and 640x400 are always good (special case)

    if ((w == 320 && h == 200) || (w == 640 && h == 400))
    {
        return 1;
    }

    // Special case: 320x240 letterboxed mode is okay (but not aspect
    // ratio corrected 320x240)

    if (w == 320 && h == 240 && !aspect_ratio_correct)
    {
        return 1;
    }

    // Ignore all modes less than 640x480

    return w >= 640 && h >= 480;
}

// Build screen_modes_fullscreen

static void BuildFullscreenModesList(void)
{
    screen_mode_t *m1;
    screen_mode_t *m2;
    screen_mode_t m;
    int display = 0;  // SDL2-TODO
    int num_modes;
    int i;

    // Free the existing modes list, if one exists

    if (screen_modes_fullscreen != NULL)
    {
        free(screen_modes_fullscreen);
    }

    num_modes = SDL_GetNumDisplayModes(display);
    screen_modes_fullscreen = calloc(num_modes, sizeof(screen_mode_t) + 1);

    for (i = 0; i < SDL_GetNumDisplayModes(display); ++i)
    {
        SDL_DisplayMode mode;

        SDL_GetDisplayMode(display, i, &mode);
        screen_modes_fullscreen[i].w = mode.w;
        screen_modes_fullscreen[i].h = mode.h;
        // SDL2-TODO: Deal with duplicate modes due to different pixel formats.
    }

    screen_modes_fullscreen[num_modes].w = 0;
    screen_modes_fullscreen[num_modes].h = 0;

    // Reverse the order of the modes list (smallest modes first)

    for (i=0; i<num_modes / 2; ++i)
    {
        m1 = &screen_modes_fullscreen[i];
        m2 = &screen_modes_fullscreen[num_modes - 1 - i];

        memcpy(&m, m1, sizeof(screen_mode_t));
        memcpy(m1, m2, sizeof(screen_mode_t));
        memcpy(m2, &m, sizeof(screen_mode_t));
    }

    num_screen_modes_fullscreen = num_modes;
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
        if (fullscreen && !GoodFullscreenMode(&modes[i]))
        {
            continue;
        }

        diff = (selected_screen_width - modes[i].w)
                  * (selected_screen_width - modes[i].w) 
             + (selected_screen_height - modes[i].h)
                  * (selected_screen_height - modes[i].h);

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

    if (fullscreen)
    {
        if (screen_modes_fullscreen == NULL)
        {
            BuildFullscreenModesList();
        }

        modes = screen_modes_fullscreen;
    }
    else if (aspect_ratio_correct) 
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
        // Skip bad fullscreen modes

        if (fullscreen && !GoodFullscreenMode(&modes[i]))
        {
            continue;
        }

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
        screen_width = modes[vidmode].w;
        screen_height = modes[vidmode].h;
    }
}

static void UpdateModeSeparator(TXT_UNCAST_ARG(widget),
                                TXT_UNCAST_ARG(separator))
{
    TXT_CAST_ARG(txt_separator_t, separator);

    if (fullscreen)
    {
        TXT_SetSeparatorLabel(separator, "Screen mode");
    }
    else
    {
        TXT_SetSeparatorLabel(separator, "Window size");
    }
}

static void AdvancedDisplayConfig(TXT_UNCAST_ARG(widget),
                                  TXT_UNCAST_ARG(modes_table))
{
    TXT_CAST_ARG(txt_table_t, modes_table);
    txt_window_t *window;
    txt_checkbox_t *ar_checkbox;

    window = TXT_NewWindow("Advanced display options");

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

#ifdef HAVE_LIBPNG
    TXT_AddWidget(window,
                  TXT_NewCheckBox("Save screenshots in PNG format",
                                  &png_screenshots));
#endif

    TXT_SignalConnect(ar_checkbox, "changed", GenerateModesTable, modes_table);
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *modes_table;
    txt_separator_t *modes_separator;
    txt_window_action_t *advanced_button;
    txt_checkbox_t *fs_checkbox;
    int num_columns;
    int num_rows;
    int window_y;

    // First time in? Initialise selected_screen_{width,height}

    if (selected_screen_width == 0)
    {
        selected_screen_width = screen_width;
        selected_screen_height = screen_height;
    }

    // Open the window

    window = TXT_NewWindow("Display Configuration");

    // Some machines can have lots of video modes.  This tries to
    // keep a limit of six lines by increasing the number of
    // columns.  In extreme cases, the window is moved up slightly.

    BuildFullscreenModesList();

    if (num_screen_modes_fullscreen <= 24)
    {
        num_columns = 3;
    }
    else if (num_screen_modes_fullscreen <= 40)
    {
        num_columns = 4;
    }
    else
    {
        num_columns = 5;
    }

    modes_table = TXT_NewTable(num_columns);

    // Build window:

    TXT_AddWidget(window,
                  fs_checkbox = TXT_NewCheckBox("Full screen", &fullscreen));

    TXT_AddWidgets(window,
                   modes_separator = TXT_NewSeparator(""),
                   modes_table,
                   NULL);

    TXT_SignalConnect(fs_checkbox, "changed",
                      GenerateModesTable, modes_table);
    TXT_SignalConnect(fs_checkbox, "changed",
                      UpdateModeSeparator, modes_separator);

    // How many rows high will the configuration window be?
    // Need to take into account number of fullscreen modes, and also
    // number of supported pixel depths.
    // The windowed modes list is four rows, so take the maximum of
    // windowed and fullscreen.

    num_rows = (num_screen_modes_fullscreen + num_columns - 1) / num_columns;

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
    UpdateModeSeparator(NULL, modes_separator);

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
    M_BindVariable("autoadjust_video_settings", &autoadjust_video_settings);
    M_BindVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindVariable("fullscreen",                &fullscreen);
    M_BindVariable("screen_width",              &screen_width);
    M_BindVariable("screen_height",             &screen_height);
    M_BindVariable("startup_delay",             &startup_delay);
    M_BindVariable("video_driver",              &video_driver);
    M_BindVariable("window_position",           &window_position);
    M_BindVariable("usegamma",                  &usegamma);
    M_BindVariable("png_screenshots",           &png_screenshots);


    if (gamemission == doom || gamemission == heretic
     || gamemission == strife)
    {
        M_BindVariable("show_endoom",               &show_endoom);
    }

    if (gamemission == heretic || gamemission == hexen || gamemission == strife)
    {
        M_BindVariable("graphical_startup",        &graphical_startup);
    }

    // Disable fullscreen by default on OS X, as there is an SDL bug
    // where some old versions of OS X (<= Snow Leopard) crash.

#ifdef __MACOSX__
    fullscreen = 0;
    screen_width = 800;
    screen_height = 600;
#endif
}
