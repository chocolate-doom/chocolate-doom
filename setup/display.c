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

static int vidmode = 0;

char *video_driver = "";
int autoadjust_video_settings = 1;
int aspect_ratio_correct = 1;
int fullscreen = 1;
int screen_width = 320;
int screen_height = 200;
int startup_delay = 1000;
int show_endoom = 1;

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

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

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
    TXT_SetColumnWidths(modes_table, 15, 15, 15);

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

    screen_width = modes[vidmode].w;
    screen_height = modes[vidmode].h;
}

#if defined(_WIN32) && !defined(_WIN32_WCE)

static int win32_video_driver = 0;

static char *win32_video_drivers[] = 
{
    "DirectX",
    "Windows GDI",
};

// Restart the textscreen library.  Used when the video_driver variable
// is changed.

static void RestartTextscreen(void)
{
    TXT_Shutdown();

    SetDisplayDriver();

    TXT_Init();
}

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

static void UpdateVideoDriver(TXT_UNCAST_ARG(widget), 
                              TXT_UNCAST_ARG(modes_table))
{
    TXT_CAST_ARG(txt_table_t, modes_table);
    char *drivers[] = 
    {
        "",
        "windib",
    };

    video_driver = drivers[win32_video_driver != 0];

    // When the video driver is changed, we need to restart the textscreen 
    // library.

    RestartTextscreen();

    // Rebuild the video modes list

    BuildFullscreenModesList();
    GenerateModesTable(NULL, modes_table);
}

#endif


void ConfigDisplay(void)
{
    txt_window_t *window;
    txt_table_t *modes_table;
    txt_checkbox_t *fs_checkbox;
    txt_checkbox_t *ar_checkbox;

    // First time in? Initialise selected_screen_{width,height}

    if (selected_screen_width == 0)
    {
        selected_screen_width = screen_width;
        selected_screen_height = screen_height;
    }

    // Open the window
    
    window = TXT_NewWindow("Display Configuration");

    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP, 
                                  TXT_SCREEN_W / 2, 5);

    TXT_AddWidgets(window, 
                   fs_checkbox = TXT_NewCheckBox("Fullscreen", &fullscreen),
                   ar_checkbox = TXT_NewCheckBox("Correct aspect ratio",
                                                 &aspect_ratio_correct),
                   NULL);

    modes_table = TXT_NewTable(3);

    // On Windows, there is an extra control to change between 
    // the Windows GDI and DirectX video drivers.

#if defined(_WIN32) && !defined(_WIN32_WCE)
    {
        txt_table_t *driver_table;
        txt_dropdown_list_t *driver_list;

        driver_table = TXT_NewTable(2);

        TXT_SetColumnWidths(driver_table, 20, 0);

        TXT_AddWidgets(driver_table,
                       TXT_NewLabel("Video driver"),
                       driver_list = TXT_NewDropdownList(&win32_video_driver,
                                                         win32_video_drivers,
                                                         2),
                       NULL);

        TXT_SignalConnect(driver_list, "changed",
                          UpdateVideoDriver, modes_table);
        SetWin32VideoDriver();

        TXT_AddWidget(window, driver_table);
    }
#endif

    // Screen modes list

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Screen mode"),
                   modes_table,
                   TXT_NewSeparator("Misc."),
                   TXT_NewCheckBox("Show ENDOOM screen", &show_endoom),
                   NULL);

    TXT_SignalConnect(fs_checkbox, "changed", GenerateModesTable, modes_table);
    TXT_SignalConnect(ar_checkbox, "changed", GenerateModesTable, modes_table);

    GenerateModesTable(NULL, modes_table);
}

