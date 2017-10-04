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
//  { 320,  200 }, // hires
    { 640,  400 },
//  { 960,  600 }, // hires
    { 1280, 800 },
//  { 1600, 1000 }, // hires
    { 1920, 1200 }, // hires * 3
    { 2560, 1600 }, // hires * 4
    { 3200, 2000 }, // hires * 5
    { 0, 0},
};

// List of aspect ratio-corrected window sizes:
static window_size_t window_sizes_scaled[] =
{
<<<<<<< HEAD
//  { 256,  200 }, // hires
//  { 320,  240 }, // hires
=======
    { 320,  240 },
>>>>>>> upstream/sdl2-branch
    { 512,  400 },
    { 640,  480 },
    { 800,  600 },
//  { 960,  720 }, // hires
    { 1024, 800 },
    { 1280, 960 },
    { 1600, 1200 },
<<<<<<< HEAD
    { 1920, 1440 }, // hires * 3
    { 2560, 1920 }, // hires * 4
    { 3200, 2400 }, // hires * 5
=======
    { 1920, 1440 },
>>>>>>> upstream/sdl2-branch
    { 0, 0},
};

static char *video_driver = "";
static char *window_position = "";
static int aspect_ratio_correct = 1;
static int integer_scaling = 0;
static int vga_porch_flash = 0;
static int force_software_renderer = 0;
static int fullscreen = 1;
<<<<<<< HEAD
static int screen_width = 640;
static int screen_height = 400;
static int screen_bpp = 0;
=======
static int fullscreen_width = 0, fullscreen_height = 0;
static int window_width = 640, window_height = 480;
>>>>>>> upstream/sdl2-branch
static int startup_delay = 1000;
static int max_scaling_buffer_pixels = 16000000;
static int usegamma = 0;

int graphical_startup = 0;
int show_endoom = 0;
int show_diskicon = 1;
int png_screenshots = 1;

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
<<<<<<< HEAD
    unsigned int num_depths = sizeof(pixel_depths) / sizeof(*pixel_depths);
    unsigned int i;

    // Search pixel_depths, find the bpp that corresponds to screen_bpp,
    // then set selected_bpp to match.

    for (i = 0; i < num_depths; ++i)
    {
        if (pixel_depths[i].bpp == screen_bpp)
        {
            selected_bpp = GetSupportedBPPIndex(pixel_depths[i].description);

            if (selected_bpp >= 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

static void SetSelectedBPP(void)
{
    const SDL_VideoInfo *info;

    if (TrySetSelectedBPP())
    {
        return;
    }

    // screen_bpp does not match any supported pixel depth.  Query SDL
    // to find out what it recommends using.

    info = SDL_GetVideoInfo();

    if (info != NULL && info->vfmt != NULL)
    {
        screen_bpp = info->vfmt->BitsPerPixel;
    }

    // Try again.

    if (!TrySetSelectedBPP())
    {
        // Give up and just use the first in the list.

        selected_bpp = 0;
        screen_bpp = GetSelectedBPP();
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

    if ((w == 320 && h == 200 && 0) || (w == 640 && h == 400)) // hires
    {
        return 1;
    }

    // Special case: 320x240 letterboxed mode is okay (but not aspect
    // ratio corrected 320x240)

    if (w == 640 && h == 480 && !aspect_ratio_correct) // hires
    {
        return 1;
    }

    // Ignore all modes less than 640x480

    return w >= 640 && h >= 480;
}

// Build screen_modes_fullscreen

static void BuildFullscreenModesList(void)
{
    SDL_PixelFormat format;
    SDL_Rect **modes;
    screen_mode_t *m1;
    screen_mode_t *m2;
    screen_mode_t m;
    int num_modes;
    int i;

    // Free the existing modes list, if one exists

    if (screen_modes_fullscreen != NULL)
    {
        free(screen_modes_fullscreen);
    }

    // Get a list of fullscreen modes and find out how many
    // modes are in the list.

    format.BitsPerPixel = screen_bpp;
    format.BytesPerPixel = (screen_bpp + 7) / 8;

    modes = SDL_ListModes(&format, SDL_FULLSCREEN);

    if (modes == NULL || modes == (SDL_Rect **) -1)
    {
        num_modes = 0;
    }
    else
    {
        for (num_modes=0; modes[num_modes] != NULL; ++num_modes);
    }

    // Build the screen_modes_fullscreen array

    screen_modes_fullscreen = malloc(sizeof(screen_mode_t) * (num_modes + 1));

    for (i=0; i<num_modes; ++i)
    {
        screen_modes_fullscreen[i].w = modes[i]->w;
        screen_modes_fullscreen[i].h = modes[i]->h;
    }

    screen_modes_fullscreen[i].w = 0;
    screen_modes_fullscreen[i].h = 0;

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
=======
    char buf[15];
    txt_radiobutton_t *result;
>>>>>>> upstream/sdl2-branch

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
<<<<<<< HEAD
 
    TXT_ClearTable(modes_table);
    TXT_SetColumnWidths(modes_table, 14, 14, 14, 14, 14);

    for (i=0; modes[i].w != 0; ++i) 
    {
        // Skip bad fullscreen modes

        if (fullscreen && !GoodFullscreenMode(&modes[i]))
        {
            continue;
        }

        if (modes[i].w < 640 || modes[i].h < 400) // hires
        {
            continue;
        }

        M_snprintf(buf, sizeof(buf), "%ix%i", modes[i].w, modes[i].h);
        rbutton = TXT_NewRadioButton(buf, &vidmode, i);
        TXT_AddWidget(modes_table, rbutton);
        TXT_SignalConnect(rbutton, "selected", ModeSelected, &modes[i]);
    }
=======
    TXT_ClearTable(sizes_table);
    TXT_SetColumnWidths(sizes_table, 14, 14, 14);
>>>>>>> upstream/sdl2-branch

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
