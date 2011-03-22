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

#ifdef _WIN32_WCE
#include "libc_wince.h"
#endif

#include "textscreen.h"

#include "display.h"

extern void RestartTextscreen(void);

typedef struct
{
    char *description;
    int bpp;
} pixel_depth_t;

// List of supported pixel depths.

static pixel_depth_t pixel_depths[] =
{
    { "8-bit",    8 },
    { "16-bit",   16 },
    { "24-bit",   24 },
    { "32-bit",   32 },
};

// List of strings containing supported pixel depths.

static char **supported_bpps;
static int num_supported_bpps;

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
    { 1280, 1000 },
    { 1600, 1200 },
    { 0, 0},
};

// List of fullscreen modes generated at runtime

static screen_mode_t *screen_modes_fullscreen = NULL;
static int num_screen_modes_fullscreen;

static int vidmode = 0;

char *video_driver = "";
int autoadjust_video_settings = 1;
int aspect_ratio_correct = 1;
int fullscreen = 1;
int screen_width = 320;
int screen_height = 200;
int screen_bpp = 8;
int startup_delay = 1000;
int show_endoom = 1;

// These are the last screen width/height values that were chosen by the
// user.  These are used when finding the "nearest" mode, so when 
// changing the fullscreen / aspect ratio options, the setting does not
// jump around.

static int selected_screen_width = 0, selected_screen_height;

// Index into the supported_bpps of the selected pixel depth.

static int selected_bpp = 0;

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

        env_string = malloc(strlen(video_driver) + 30);
        sprintf(env_string, "SDL_VIDEODRIVER=%s", video_driver);
        putenv(env_string);
        free(env_string);
    }
    else
    {
#if defined(_WIN32) && !defined(_WIN32_WCE)
        // On Windows, use DirectX over windib by default.

        putenv("SDL_VIDEODRIVER=directx");
#endif
    }
}

// Query SDL as to whether any fullscreen modes are available for the
// specified pixel depth.

static int PixelDepthSupported(int bpp)
{
    SDL_PixelFormat format;
    SDL_Rect **modes;

    format.BitsPerPixel = bpp;
    format.BytesPerPixel = (bpp + 7) / 8;

    modes = SDL_ListModes(&format, SDL_FULLSCREEN);

    return modes != NULL;
}

// Query SDL and populate the supported_bpps array.

static void IdentifyPixelDepths(void)
{
    unsigned int i;
    unsigned int num_depths = sizeof(pixel_depths) / sizeof(*pixel_depths);

    if (supported_bpps != NULL)
    {
        free(supported_bpps);
    }

    supported_bpps = malloc(sizeof(char *) * num_depths);
    num_supported_bpps = 0;

    // Check each bit depth to determine if modes are available.

    for (i = 0; i < num_depths; ++i)
    {
        // If modes are available, add this bit depth to the list.

        if (PixelDepthSupported(pixel_depths[i].bpp))
        {
            supported_bpps[num_supported_bpps] = pixel_depths[i].description;
            ++num_supported_bpps;
        }
    }

    // No supported pixel depths?  That's kind of a problem.  Add 8bpp
    // as a fallback.

    if (num_supported_bpps == 0)
    {
        supported_bpps[0] = pixel_depths[0].description;
        ++num_supported_bpps;
    }
}

// Get the screen pixel depth corresponding to what selected_bpp is set to.

static int GetSelectedBPP(void)
{
    unsigned int num_depths = sizeof(pixel_depths) / sizeof(*pixel_depths);
    unsigned int i;

    // Find which pixel depth is selected, and set screen_bpp.

    for (i = 0; i < num_depths; ++i)
    {
        if (pixel_depths[i].description == supported_bpps[selected_bpp])
        {
            return pixel_depths[i].bpp;
        }
    }

    // Default fallback value.

    return 8;
}

// Get the index into supported_bpps of the specified pixel depth string.

static int GetSupportedBPPIndex(char *description)
{
    unsigned int i;

    for (i = 0; i < num_supported_bpps; ++i)
    {
        if (supported_bpps[i] == description)
        {
            return i;
        }
    }

    return -1;
}

// Set selected_bpp to match screen_bpp.

static int TrySetSelectedBPP(void)
{
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

        sprintf(buf, "%ix%i", modes[i].w, modes[i].h);
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

// Callback invoked when the BPP selector is changed.

static void UpdateBPP(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(modes_table))
{
    TXT_CAST_ARG(txt_table_t, modes_table);

    screen_bpp = GetSelectedBPP();

    // Rebuild list of fullscreen modes.

    BuildFullscreenModesList();
    GenerateModesTable(NULL, modes_table);
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

#if defined(_WIN32) && !defined(_WIN32_WCE)

static int use_directx = 1;

static void SetWin32VideoDriver(void)
{
    if (!strcmp(video_driver, "windib"))
    {
        use_directx = 0;
    }
    else
    {
        use_directx = 1;
    }
}

static void UpdateVideoDriver(TXT_UNCAST_ARG(widget), 
                              TXT_UNCAST_ARG(modes_table))
{
    TXT_CAST_ARG(txt_table_t, modes_table);

    if (use_directx)
    {
        video_driver = "";
    }
    else
    {
        video_driver = "windib";
    }

    // When the video driver is changed, we need to restart the textscreen 
    // library.

    RestartTextscreen();

    // Rebuild the list of supported pixel depths.

    IdentifyPixelDepths();
    SetSelectedBPP();

    // Rebuild the video modes list

    BuildFullscreenModesList();
    GenerateModesTable(NULL, modes_table);
}

#endif

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
                   TXT_NewCheckBox("Show ENDOOM screen on exit", &show_endoom),
                   NULL);

    TXT_SignalConnect(ar_checkbox, "changed", GenerateModesTable, modes_table);

    // On Windows, there is an extra control to change between 
    // the Windows GDI and DirectX video drivers.

#if defined(_WIN32) && !defined(_WIN32_WCE)
    {
        txt_radiobutton_t *dx_button, *gdi_button;

        TXT_AddWidgets(window,
                       TXT_NewSeparator("Windows video driver"),
                       dx_button = TXT_NewRadioButton("DirectX",
                                                      &use_directx, 1),
                       gdi_button = TXT_NewRadioButton("Windows GDI",
                                                       &use_directx, 0),
                       NULL);

        TXT_SignalConnect(dx_button, "selected",
                          UpdateVideoDriver, modes_table);
        TXT_SignalConnect(gdi_button, "selected",
                          UpdateVideoDriver, modes_table);
        SetWin32VideoDriver();
    }
#endif
}

void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *modes_table;
    txt_separator_t *modes_separator;
    txt_table_t *bpp_table;
    txt_window_action_t *advanced_button;
    txt_checkbox_t *fs_checkbox;
    int i;
    int num_columns;
    int num_rows;
    int window_y;

    // What color depths are supported?  Generate supported_bpps array
    // and set selected_bpp to match the current value of screen_bpp.

    IdentifyPixelDepths();
    SetSelectedBPP();

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

    if (num_supported_bpps > 1)
    {
        TXT_AddWidgets(window,
                       TXT_NewSeparator("Color depth"),
                       bpp_table = TXT_NewTable(4),
                       NULL);

        for (i = 0; i < num_supported_bpps; ++i)
        {
            txt_radiobutton_t *button;

            button = TXT_NewRadioButton(supported_bpps[i],
                                        &selected_bpp, i);

            TXT_AddWidget(bpp_table, button);
            TXT_SignalConnect(button, "selected", UpdateBPP, modes_table);
        }
    }

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

    if (num_supported_bpps > 1)
    {
        num_rows += 2;
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

