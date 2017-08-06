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
} window_size_t;

// List of aspect ratio-uncorrected window sizes:
static window_size_t window_sizes_unscaled[] =
{
    { 320,  200 },
    { 640,  400 },
    { 960,  600 },
    { 1280, 800 },
    { 1600, 1000 },
    { 0, 0},
};

// List of aspect ratio-corrected window sizes:
static window_size_t window_sizes_scaled[] =
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

static char *video_driver = "";
static char *window_position = "";
static int aspect_ratio_correct = 1;
static int integer_scaling = 0;
static int vga_porch_flash = 0;
static int force_software_renderer = 0;
static int fullscreen = 1;
static int fullscreen_width = 0, fullscreen_height = 0;
static int window_width = 640, window_height = 480;
static int startup_delay = 1000;
static int max_scaling_buffer_pixels = 16000000;
static int usegamma = 0;

int graphical_startup = 1;
int show_endoom = 1;
int show_diskicon = 1;
int png_screenshots = 0;

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

static void WindowSizeSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(size))
{
    TXT_CAST_ARG(window_size_t, size);

    window_width = size->w;
    window_height = size->h;
}

static txt_radiobutton_t *SizeSelectButton(window_size_t *size)
{
    char buf[15];
    txt_radiobutton_t *result;

    M_snprintf(buf, sizeof(buf), "%ix%i", size->w, size->h);
    result = TXT_NewRadioButton(buf, &window_width, size->w);
    TXT_SignalConnect(result, "selected", WindowSizeSelected, size);

    return result;
}

static void GenerateSizesTable(TXT_UNCAST_ARG(widget),
                               TXT_UNCAST_ARG(sizes_table))
{
    TXT_CAST_ARG(txt_table_t, sizes_table);
    window_size_t *sizes;
    boolean have_size;
    int i;

    // Pick which window sizes list to use
    if (aspect_ratio_correct)
    {
        sizes = window_sizes_scaled;
    }
    else
    {
        sizes = window_sizes_unscaled;
    }

    // Build the table
    TXT_ClearTable(sizes_table);
    TXT_SetColumnWidths(sizes_table, 14, 14, 14);

    TXT_AddWidget(sizes_table, TXT_NewSeparator("Window size"));

    have_size = false;

    for (i = 0; sizes[i].w != 0; ++i)
    {
        TXT_AddWidget(sizes_table, SizeSelectButton(&sizes[i]));
        have_size = have_size || window_width == sizes[i].w;
    }

    // Windows can be any arbitrary size. We key off the width of the
    // window in pixels. If the current size is not in the list of
    // standard (integer multiply) sizes, create a special button to
    // mean "the current window size".
    if (!have_size)
    {
        static window_size_t current_size;
        current_size.w = window_width;
        current_size.h = window_height;
        TXT_AddWidget(sizes_table, SizeSelectButton(&current_size));
    }
}

static void AdvancedDisplayConfig(TXT_UNCAST_ARG(widget),
                                  TXT_UNCAST_ARG(sizes_table))
{
    TXT_CAST_ARG(txt_table_t, sizes_table);
    txt_window_t *window;
    txt_checkbox_t *ar_checkbox;

    window = TXT_NewWindow("Advanced display options");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 40);

    TXT_AddWidgets(window,
        ar_checkbox = TXT_NewCheckBox("Fix aspect ratio",
                                      &aspect_ratio_correct),
        TXT_If(gamemission == heretic || gamemission == hexen
            || gamemission == strife,
            TXT_NewCheckBox("Graphical startup", &graphical_startup)),
        TXT_If(gamemission == doom || gamemission == heretic
            || gamemission == strife,
            TXT_NewCheckBox("Show ENDOOM screen on exit",
                            &show_endoom)),
#ifdef HAVE_LIBPNG
        TXT_NewCheckBox("Save screenshots in PNG format",
                        &png_screenshots),
#endif
        NULL);

    TXT_SignalConnect(ar_checkbox, "changed", GenerateSizesTable, sizes_table);
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *sizes_table;
    txt_window_action_t *advanced_button;

    // Open the window
    window = TXT_NewWindow("Display Configuration");
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    // Build window:
    TXT_AddWidgets(window,
        TXT_NewCheckBox("Full screen", &fullscreen),
        TXT_NewConditional(&fullscreen, 0,
            sizes_table = TXT_NewTable(3)),
        NULL);

    TXT_SetColumnWidths(window, 42);

    // The window is set at a fixed vertical position.  This keeps
    // the top of the window stationary when switching between
    // fullscreen and windowed mode (which causes the window's
    // height to change).
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, 6);

    GenerateSizesTable(NULL, sizes_table);

    // Button to open "advanced" window.
    // Need to pass a pointer to the window sizes table, as some of the options
    // in there trigger a rebuild of it.
    advanced_button = TXT_NewWindowAction('a', "Advanced");
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, advanced_button);
    TXT_SignalConnect(advanced_button, "pressed",
                      AdvancedDisplayConfig, sizes_table);
}

void BindDisplayVariables(void)
{
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("integer_scaling",           &integer_scaling);
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
    M_BindIntVariable("vga_porch_flash",           &vga_porch_flash);
    M_BindIntVariable("force_software_renderer",   &force_software_renderer);
    M_BindIntVariable("max_scaling_buffer_pixels", &max_scaling_buffer_pixels);

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
