//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	DOOM graphics stuff for SDL.
//


#include "SDL.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "icon.c"

#include "config.h"
#include "deh_str.h"
#include "doomtype.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// SDL video driver name

char *video_driver = "";

// Window position:

static char *window_position = "";

// These are (1) the window (or the full screen) that our game is rendered to
// and (2) the renderer that scales the texture (see below) into this window.

static SDL_Window *screen;
static SDL_Renderer *renderer;

// Window title

static char *window_title = "";

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds I_VideoBuffer), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.

static SDL_Surface *screenbuffer = NULL;
static SDL_Surface *rgbabuffer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Texture *texture_upscaled = NULL;
int destscreen = 0;
byte *destpixels = NULL;
int currentscreen = 0;
byte *currentpixels = NULL;
byte *screenpixels[3] = { NULL, NULL, NULL };

byte *tempscreen = NULL;

static boolean mode_y = false;

static SDL_Rect blit_rect = {
    0,
    0,
    SCREENWIDTH,
    SCREENHEIGHT
};

static SDL_Rect page_rect = {
    0,
    0,
    SCREENWIDTH,
    SCREENHEIGHT
};

// palette

static SDL_Color palette[256];
static boolean palette_to_set;

// display has been set up?

static boolean initialized = false;

// disable mouse?

static boolean nomouse = false;
int usemouse = 1;

// Save screenshots in PNG format.

int png_screenshots = 0;

// Screen width and height, from configuration file.

int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;

// Automatically adjust video settings if the selected mode is 
// not a valid video mode.

static int autoadjust_video_settings = 1;

// Run in full screen mode?  (int type for config code)

int fullscreen = true;

// Aspect ratio correction mode

int aspect_ratio_correct = true;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 1000;

// Grab the mouse? (int type for config code)

static int grabmouse = true;

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible = true;

// If true, we display dots at the bottom of the screen to 
// indicate FPS.

static boolean display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static boolean noblit;

// Callback function to invoke to determine whether to grab the 
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = NULL;

// Does the window currently have focus?

static boolean window_focused = true;

// Window resize state.

static boolean need_resize = false;
static unsigned int resize_w, resize_h;
static unsigned int last_resize_time;

// Gamma correction level to use

int usegamma = 0;

// Disk icon variables

int show_diskicon = 1;

static int diskicon_pos_x = 0;
static int diskicon_pos_y = 0;

static boolean MouseShouldBeGrabbed()
{
    // never grab the mouse when in screensaver mode
   
    if (screensaver_mode)
        return false;

    // if the window doesn't have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to 
    // see the mouse pointer)

    if (fullscreen)
        return true;

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
        return false;

    // if we specify not to grab the mouse, never grab

    if (!grabmouse)
        return false;

    // Invoke the grabmouse callback function to determine whether
    // the mouse should be grabbed

    if (grabmouse_callback != NULL)
    {
        return grabmouse_callback();
    }
    else
    {
        return true;
    }
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    grabmouse_callback = func;
}

// Set the variable controlling FPS dots.

void I_DisplayFPSDots(boolean dots_on)
{
    display_fps_dots = dots_on;
}

// Show or hide the mouse cursor. We have to use different techniques
// depending on the OS.

static void SetShowCursor(boolean show)
{
    if (!screensaver_mode)
    {
        // When the cursor is hidden, grab the input.
        // Relative mode implicitly hides the cursor.
        SDL_SetRelativeMouseMode(!show);
        SDL_GetRelativeMouseState(NULL, NULL);
    }
}

void I_ShutdownGraphics(void)
{
    if (initialized)
    {
        SetShowCursor(true);

        SDL_QuitSubSystem(SDL_INIT_VIDEO);

        initialized = false;
    }
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

static void HandleWindowEvent(SDL_WindowEvent *event)
{
    switch (event->event)
    {
#if 0 // SDL2-TODO
        case SDL_ACTIVEEVENT:
            // need to update our focus state
            UpdateFocus();
            break;
#endif
        case SDL_WINDOWEVENT_EXPOSED:
            palette_to_set = true;
            break;

        case SDL_WINDOWEVENT_RESIZED:
            need_resize = true;
            resize_w = event->data1;
            resize_h = event->data2;
            last_resize_time = SDL_GetTicks();
            break;

        // Don't render the screen when the window is minimized:

        case SDL_WINDOWEVENT_MINIMIZED:
            screenvisible = false;
            break;

        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
            screenvisible = true;
            break;

        // Update the value of window_focused when we get a focus event
        //
        // We try to make ourselves be well-behaved: the grab on the mouse
        // is removed if we lose focus (such as a popup window appearing),
        // and we dont move the mouse around if we aren't focused either.

        case SDL_WINDOWEVENT_ENTER:
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            window_focused = true;
            break;

        case SDL_WINDOWEVENT_LEAVE:
        case SDL_WINDOWEVENT_FOCUS_LOST:
            window_focused = false;
            break;

        default:
            break;
    }
}

void I_GetEvent(void)
{
    extern void I_HandleKeyboardEvent(SDL_Event *sdlevent);
    extern void I_HandleMouseEvent(SDL_Event *sdlevent);
    SDL_Event sdlevent;

    SDL_PumpEvents();

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
		I_HandleKeyboardEvent(&sdlevent);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (usemouse && !nomouse && window_focused)
                {
                    I_HandleMouseEvent(&sdlevent);
                }
                break;

            case SDL_QUIT:
                if (screensaver_mode)
                {
                    I_Quit();
                }
                else
                {
                    event_t event;
                    event.type = ev_quit;
                    D_PostEvent(&event);
                }
                break;

            case SDL_WINDOWEVENT:
                if (sdlevent.window.windowID == SDL_GetWindowID(screen))
                {
                    HandleWindowEvent(&sdlevent.window);
                }
                break;

            default:
                break;
        }
    }
}

//
// I_StartTic
//
void I_StartTic (void)
{
    if (!initialized)
    {
        return;
    }

    I_GetEvent();

    if (usemouse && !nomouse)
    {
        I_ReadMouse();
    }

    I_UpdateJoystick();
}

//
// I_UpdateBox
//
static void I_UpdateBox(int x, int y, int w, int h)
{
    int i, j;
    if (x < 0 || y < 0 || w <= 0 || h <= 0
     || x + w > SCREENWIDTH || y + h > SCREENHEIGHT)
    {
        I_Error("Bad I_UpdateBox (%i, %i, %i, %i)", x, y, w, h);
    }

    for (i = y; i < y + h; i++)
    {
        for (j = x; j < x + w; j++)
        {
            destpixels[i * SCREENWIDTH + j] =
                I_VideoBuffer[i * SCREENWIDTH + j];
        }
    }
}

//
// I_UpdateNoBlit
//
static int olddirtybox[2][4];
void I_UpdateNoBlit (void)
{
    int realdirtybox[4];
    int x, y, w, h;

    if (!initialized || noblit || !mode_y)
    {
        return;
    }

    //Set current screen
    currentscreen = destscreen;
    currentpixels = destpixels;
    page_rect.y = currentscreen * SCREENHEIGHT;

    // Update dirtybox size
    realdirtybox[BOXTOP] = dirtybox[BOXTOP];
    if (realdirtybox[BOXTOP] < olddirtybox[0][BOXTOP])
    {
        realdirtybox[BOXTOP] = olddirtybox[0][BOXTOP];
    }
    if (realdirtybox[BOXTOP] < olddirtybox[1][BOXTOP])
    {
        realdirtybox[BOXTOP] = olddirtybox[1][BOXTOP];
    }

    realdirtybox[BOXRIGHT] = dirtybox[BOXRIGHT];
    if (realdirtybox[BOXRIGHT] < olddirtybox[0][BOXRIGHT])
    {
        realdirtybox[BOXRIGHT] = olddirtybox[0][BOXRIGHT];
    }
    if (realdirtybox[BOXRIGHT] < olddirtybox[1][BOXRIGHT])
    {
        realdirtybox[BOXRIGHT] = olddirtybox[1][BOXRIGHT];
    }

    realdirtybox[BOXBOTTOM] = dirtybox[BOXBOTTOM];
    if (realdirtybox[BOXBOTTOM] > olddirtybox[0][BOXBOTTOM])
    {
        realdirtybox[BOXBOTTOM] = olddirtybox[0][BOXBOTTOM];
    }
    if (realdirtybox[BOXBOTTOM] > olddirtybox[1][BOXBOTTOM])
    {
        realdirtybox[BOXBOTTOM] = olddirtybox[1][BOXBOTTOM];
    }

    realdirtybox[BOXLEFT] = dirtybox[BOXLEFT];
    if (realdirtybox[BOXLEFT] > olddirtybox[0][BOXLEFT])
    {
        realdirtybox[BOXLEFT] = olddirtybox[0][BOXLEFT];
    }
    if (realdirtybox[BOXLEFT] > olddirtybox[1][BOXLEFT])
    {
        realdirtybox[BOXLEFT] = olddirtybox[1][BOXLEFT];
    }

    // Leave current box for next update
    memcpy(olddirtybox[0], olddirtybox[1], 4 * sizeof(int));
    memcpy(olddirtybox[1], dirtybox, 4 * sizeof(int));

    // Update screen
    if (realdirtybox[BOXBOTTOM] <= realdirtybox[BOXTOP])
    {
        x = realdirtybox[BOXLEFT];
        y = realdirtybox[BOXBOTTOM];
        w = realdirtybox[BOXRIGHT] - realdirtybox[BOXLEFT] + 1;
        h = realdirtybox[BOXTOP] - realdirtybox[BOXBOTTOM] + 1;
        I_UpdateBox(x, y, w, h);
    }
    // Clear box
    M_ClearBox(dirtybox);
}

static void UpdateGrab(void)
{
    static boolean currently_grabbed = false;
    boolean grab;

    grab = MouseShouldBeGrabbed();

    if (screensaver_mode)
    {
        // Hide the cursor in screensaver mode

        SetShowCursor(false);
    }
    else if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
    }
    else if (!grab && currently_grabbed)
    {
        int screen_w, screen_h;

        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_GetWindowSize(screen, &screen_w, &screen_h);
        SDL_WarpMouseInWindow(screen, screen_w - 16, screen_h - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;

}

static void CreateUpscaledTexture(void)
{
    const int actualheight = aspect_ratio_correct ?
                             SCREENHEIGHT_4_3 :
                             SCREENHEIGHT;
    int w, h;
    int h_upscale, w_upscale;
    static int h_upscale_old, w_upscale_old;

    // Get the size of the renderer output. The units this gives us will be
    // real world pixels, which are not necessarily equivalent to the screen's
    // window size (because of highdpi).
    if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0)
    {
        I_Error("Failed to get renderer output size: %s", SDL_GetError());
    }

    // When the screen or window dimensions do not match the aspect ratio
    // of the texture, the rendered area is scaled down to fit. Calculate
    // the actual dimensions of the rendered area.

    if (w * actualheight < h * SCREENWIDTH)
    {
        // Tall window.

        h = w * actualheight / SCREENWIDTH;
    }
    else
    {
        // Wide window.

        w = h * SCREENWIDTH / actualheight;
    }

    // Pick texture size the next integer multiple of the screen dimensions.
    // If one screen dimension matches an integer multiple of the original
    // resolution, there is no need to overscale in this direction.

    w_upscale = (w + SCREENWIDTH - 1) / SCREENWIDTH;
    h_upscale = (h + SCREENHEIGHT - 1) / SCREENHEIGHT;

    // Minimum texture dimensions of 320x200.

    if (w_upscale < 1)
    {
        w_upscale = 1;
    }
    if (h_upscale < 1)
    {
        h_upscale = 1;
    }

    // Limit maximum texture dimensions to 1600x1200.
    // It's really diminishing returns at this point.

    if (w_upscale > 5)
    {
        w_upscale = 5;
    }
    if (h_upscale > 6)
    {
        h_upscale = 6;
    }

    // Create a new texture only if the upscale factors have actually changed.

    if (h_upscale == h_upscale_old && w_upscale == w_upscale_old)
    {
        return;
    }

    h_upscale_old = h_upscale;
    w_upscale_old = w_upscale;

    if (texture_upscaled)
    {
        SDL_DestroyTexture(texture_upscaled);
    }

    // Set the scaling quality for rendering the upscaled texture to "linear",
    // which looks much softer and smoother than "nearest" but does a better
    // job at downscaling from the upscaled texture to screen.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    texture_upscaled = SDL_CreateTexture(renderer,
                                SDL_GetWindowPixelFormat(screen),
                                SDL_TEXTUREACCESS_TARGET,
                                w_upscale*SCREENWIDTH,
                                h_upscale*SCREENHEIGHT);
}

void I_DrawScreen(int screen)
{

    if (palette_to_set)
    {
        SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
        palette_to_set = false;
    }

    // Blit from the paletted 8-bit screen buffer to the intermediate
    // 32-bit RGBA buffer that we can load into the texture.

    SDL_LowerBlit(screenbuffer, &page_rect, rgbabuffer, &blit_rect);

    // Update the intermediate texture with the contents of the RGBA buffer.

    SDL_UpdateTexture(texture, NULL, rgbabuffer->pixels, rgbabuffer->pitch);

    // Make sure the pillarboxes are kept clear each frame.

    SDL_RenderClear(renderer);

    // Render this intermediate texture into the upscaled texture
    // using "nearest" integer scaling.

    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Finally, render this upscaled texture to screen using linear scaling.

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);

    // Draw!

    SDL_RenderPresent(renderer);
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    static int lasttic;
    int tics;
    int i;

    if (!initialized)
        return;

    if (noblit)
        return;

    if (need_resize && SDL_GetTicks() > last_resize_time + 1000/TICRATE)
    {
        CreateUpscaledTexture();
        screen_width = resize_w;
        screen_height = resize_h;
        need_resize = false;
        palette_to_set = true;
    }

    UpdateGrab();

#if 0 // SDL2-TODO
    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;
#endif

    // draws little dots on the bottom of the screen

    if (display_fps_dots)
    {
	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*4 ; i+=4)
        destpixels[ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*4 ; i+=4)
        destpixels[ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    }

    I_DrawScreen(destscreen);

    if (mode_y)
    {
        destscreen = (destscreen + 1) % 3;
        destpixels = screenpixels[destscreen];
    }
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy(scr, currentpixels, SCREENWIDTH*SCREENHEIGHT*sizeof(*scr));
}


//
// I_SetPalette
//
void I_SetPalette (byte *doompalette)
{
    int i;

    for (i=0; i<256; ++i)
    {
        // Zero out the bottom two bits of each channel - the PC VGA
        // controller only supports 6 bits of accuracy.

        palette[i].r = gammatable[usegamma][*doompalette++] & ~3;
        palette[i].g = gammatable[usegamma][*doompalette++] & ~3;
        palette[i].b = gammatable[usegamma][*doompalette++] & ~3;
    }

    palette_to_set = true;
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex(int r, int g, int b)
{
    int best, best_diff, diff;
    int i;

    best = 0; best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
        diff = (r - palette[i].r) * (r - palette[i].r)
             + (g - palette[i].g) * (g - palette[i].g)
             + (b - palette[i].b) * (b - palette[i].b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

// 
// Set the window title
//

void I_SetWindowTitle(char *title)
{
    window_title = title;
}

//
// Call the SDL function to set the window title, based on 
// the title set with I_SetWindowTitle.
//

void I_InitWindowTitle(void)
{
    char *buf;

    buf = M_StringJoin(window_title, " - ", PACKAGE_STRING, NULL);
    SDL_SetWindowTitle(screen, buf);
    free(buf);
}

// Set the application icon

void I_InitWindowIcon(void)
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *) icon_data, icon_w, icon_h,
                                       32, icon_w * 4,
                                       0xff << 24, 0xff << 16,
                                       0xff << 8, 0xff << 0);

    SDL_SetWindowIcon(screen, surface);
    SDL_FreeSurface(surface);
}

// Set video size to a particular scale factor (1x, 2x, 3x, etc.)

static void SetScaleFactor(int factor)
{
    int w, h;

    // Pick 320x200 or 320x240, depending on aspect ratio correct

    if (aspect_ratio_correct)
    {
        w = SCREENWIDTH;
        h = SCREENHEIGHT_4_3;
    }
    else
    {
        w = SCREENWIDTH;
        h = SCREENHEIGHT;
    }

    screen_width = w * factor;
    screen_height = h * factor;
}

void I_GraphicsCheckCommandLine(void)
{
    int i;

    //!
    // @vanilla
    //
    // Disable blitting the screen.
    //

    noblit = M_CheckParm ("-noblit"); 

    //!
    // @category video 
    //
    // Grab the mouse when running in windowed mode.
    //

    if (M_CheckParm("-grabmouse"))
    {
        grabmouse = true;
    }

    //!
    // @category video 
    //
    // Don't grab the mouse when running in windowed mode.
    //

    if (M_CheckParm("-nograbmouse"))
    {
        grabmouse = false;
    }

    // default to fullscreen mode, allow override with command line
    // nofullscreen because we love prboom

    //!
    // @category video 
    //
    // Run in a window.
    //

    if (M_CheckParm("-window") || M_CheckParm("-nofullscreen"))
    {
        fullscreen = false;
    }

    //!
    // @category video 
    //
    // Run in fullscreen mode.
    //

    if (M_CheckParm("-fullscreen"))
    {
        fullscreen = true;
    }

    //!
    // @category video 
    //
    // Disable the mouse.
    //

    nomouse = M_CheckParm("-nomouse") > 0;

    //!
    // @category video
    // @arg <x>
    //
    // Specify the screen width, in pixels.
    //

    i = M_CheckParmWithArgs("-width", 1);

    if (i > 0)
    {
        screen_width = atoi(myargv[i + 1]);
    }

    //!
    // @category video
    // @arg <y>
    //
    // Specify the screen height, in pixels.
    //

    i = M_CheckParmWithArgs("-height", 1);

    if (i > 0)
    {
        screen_height = atoi(myargv[i + 1]);
    }

    //!
    // @category video
    // @arg <WxY>[wf]
    //
    // Specify the dimensions of the window or fullscreen mode.  An
    // optional letter of w or f appended to the dimensions selects
    // windowed or fullscreen mode.

    i = M_CheckParmWithArgs("-geometry", 1);

    if (i > 0)
    {
        int w, h, s;
        char f;

        s = sscanf(myargv[i + 1], "%ix%i%1c", &w, &h, &f);
        if (s == 2 || s == 3)
        {
            screen_width = w;
            screen_height = h;

            if (s == 3 && f == 'f')
            {
                fullscreen = true;
            }
            else if (s == 3 && f == 'w')
            {
                fullscreen = false;
            }
        }
    }

    //!
    // @category video
    //
    // Don't scale up the screen.
    //

    if (M_CheckParm("-1")) 
    {
        SetScaleFactor(1);
    }

    //!
    // @category video
    //
    // Double up the screen to 2x its normal size.
    //

    if (M_CheckParm("-2")) 
    {
        SetScaleFactor(2);
    }

    //!
    // @category video
    //
    // Double up the screen to 3x its normal size.
    //

    if (M_CheckParm("-3")) 
    {
        SetScaleFactor(3);
    }
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver(void)
{
    char *env;

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        screensaver_mode = true;
    }
}

static void SetSDLVideoDriver(void)
{
    // Allow a default value for the SDL video driver to be specified
    // in the configuration file.

    if (strcmp(video_driver, "") != 0)
    {
        char *env_string;

        env_string = M_StringJoin("SDL_VIDEODRIVER=", video_driver, NULL);
        putenv(env_string);
        free(env_string);
    }
}

static void SetWindowPositionVars(void)
{
    char buf[64];
    int x, y;

    if (window_position == NULL || !strcmp(window_position, ""))
    {
        return;
    }

    if (!strcmp(window_position, "center"))
    {
        putenv("SDL_VIDEO_CENTERED=1");
    }
    else if (sscanf(window_position, "%i,%i", &x, &y) == 2)
    {
        M_snprintf(buf, sizeof(buf), "SDL_VIDEO_WINDOW_POS=%i,%i", x, y);
        putenv(buf);
    }
}

static void SetVideoMode(int w, int h)
{
    byte *doompal;
    int flags = 0;
    int i;

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);

    // If we are already running, we need to free the screenbuffer
    // surface before setting the new mode.

    if (screenbuffer != NULL)
    {
        SDL_FreeSurface(screenbuffer);
        screenbuffer = NULL;
    }

    // Close the current window.

    if (screen != NULL)
    {
        SDL_DestroyWindow(screen);
        screen = NULL;
    }

    // Set the video mode.

    flags = 0;

    if (fullscreen)
    {
        // This flags means "Never change the screen resolution! Instead,
        // draw to the entire screen by scaling the texture appropriately".
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else
    {
        // In windowed mode, the window can be resized while the game is
        // running.  This feature is disabled on OS X, as it adds an ugly
        // scroll handle to the corner of the screen.
        flags |= SDL_WINDOW_RESIZABLE;
    }

    // Set the highdpi flag - this makes a big difference on Macs with
    // retina displays, especially when using small window sizes.
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    // Create window and renderer contexts. We set the window title
    // later anyway and leave the window position "undefined". If "flags"
    // contains the fullscreen flag (see above), then w and h are ignored.

    screen = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    w, h, flags);

    if (screen == NULL)
    {
        I_Error("Error creating window for video mode %ix%i: %s\n",
                w, h, SDL_GetError());
    }

    // If we are running fullscreen, the whole screen is our "window".

    if (fullscreen)
    {
        SDL_DisplayMode mode;

        // We do not change the video mode to run fullscreen but scale to fill
        // the desktop that "screen" is assigned to. So, use desktop dimensions
        // to calculate the size of the upscaled texture.

        SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(screen), &mode);

        h = mode.h;
        w = mode.w;
    }

    // The SDL_RENDERER_TARGETTEXTURE flag is required to render the
    // intermediate texture into the upscaled texture.

    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_TARGETTEXTURE);

    if (renderer == NULL)
    {
        I_Error("Error creating renderer for video mode %ix%i: %s\n",
                w, h, SDL_GetError());
    }

    // Important: Set the "logical size" of the rendering context. At the same
    // time this also defines the aspect ratio that is preserved while scaling
    // and stretching the texture into the window.

    SDL_RenderSetLogicalSize(renderer,
                             SCREENWIDTH,
                             aspect_ratio_correct ? SCREENHEIGHT_4_3 : SCREENHEIGHT);

    I_InitWindowTitle();
    I_InitWindowIcon();

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Create the 8-bit paletted and the 32-bit RGBA screenbuffer surfaces.
    
    screenbuffer = SDL_CreateRGBSurface(0,
                                        SCREENWIDTH, SCREENHEIGHT * 3, 8,
                                        0, 0, 0, 0);
    SDL_FillRect(screenbuffer, NULL, 0);

    rgbabuffer = SDL_CreateRGBSurface(0,
                                      SCREENWIDTH, SCREENHEIGHT, 32,
                                      0, 0, 0, 0);
    SDL_FillRect(rgbabuffer, NULL, 0);

    // Set the scaling quality for rendering the intermediate texture into
    // the upscaled texture to "nearest", which is gritty and pixelated and
    // resembles software scaling pretty well.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create the intermediate texture that the RGBA surface gets loaded into.
    // The SDL_TEXTUREACCESS_STREAMING flag means that this texture's content
    // is going to change frequently.

    texture = SDL_CreateTexture(renderer,
                                SDL_GetWindowPixelFormat(screen),
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREENWIDTH, SCREENHEIGHT);

    // Initially create the upscaled texture for rendering to screen

    CreateUpscaledTexture();
}

void I_InitGraphics(boolean use_mode_y)
{
    SDL_Event dummy;
    byte *doompal;
    char *env;
    int i;

    mode_y = use_mode_y;

    // Pass through the XSCREENSAVER_WINDOW environment variable to 
    // SDL_WINDOWID, to embed the SDL window into the Xscreensaver
    // window.

    env = getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        char winenv[30];
        int winid;

        sscanf(env, "0x%x", &winid);
        M_snprintf(winenv, sizeof(winenv), "SDL_WINDOWID=%i", winid);

        putenv(winenv);
    }

    SetSDLVideoDriver();
    SetWindowPositionVars();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error("Failed to initialize video: %s", SDL_GetError());
    }

    //
    // Enter into graphics mode.
    //
    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    //

    if (screensaver_mode)
    {
        SetVideoMode(0, 0);
    }
    else
    {
        // SDL2-TODO: sanity check screen_width/screen_height against
        // screen modes listed as available by LibSDL, and auto-adjust
        // when the selected mode is not available.
        SetVideoMode(screen_width, screen_height);
    }

    // Start with a clear black screen
    // (screen will be flipped after we set the palette)

    SDL_FillRect(screenbuffer, NULL, 0);

    // Set the palette

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);
    I_SetPalette(doompal);

    SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);

    // SDL2-TODO UpdateFocus();
    UpdateGrab();

    // On some systems, it takes a second or so for the screen to settle
    // after changing modes.  We include the option to add a delay when
    // setting the screen mode, so that the game doesn't start immediately
    // with the player unable to see anything.

    if (fullscreen && !screensaver_mode)
    {
        SDL_Delay(startup_delay);
    }

    // The actual 320x200 canvas that we draw to. This is the pixel buffer of
    // the 8-bit paletted screen buffer that gets blit on an intermediate
    // 32-bit RGBA screen buffer that gets loaded into a texture that gets
    // finally rendered into our window or full screen in I_FinishUpdate().

    currentscreen = destscreen = 0;

    if (mode_y)
    {
        currentpixels = destpixels = screenbuffer->pixels;

        for (i = 0; i < 3; i++)
        {
            screenpixels[i] = (byte*)screenbuffer->pixels
                            + i * SCREENWIDTH * SCREENHEIGHT;
        }

        I_VideoBuffer = (byte*)Z_Malloc(SCREENWIDTH * SCREENHEIGHT,
                                        PU_STATIC, NULL);
    }
    else
    {
        currentpixels = destpixels = I_VideoBuffer = screenbuffer->pixels;
    }

    tempscreen = (byte*)Z_Malloc(SCREENWIDTH * SCREENHEIGHT,
                                 PU_STATIC, NULL);

    V_RestoreBuffer();

    // Clear the screen to black.

    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

    memset(tempscreen, 0, SCREENWIDTH * SCREENHEIGHT);

    // We need SDL to give us translated versions of keys as well

    // SDL2-TODO SDL_EnableUNICODE(1);

    // Repeat key presses - this is what Vanilla Doom does
    // Not sure about repeat rate - probably dependent on which DOS
    // driver is used.  This is good enough though.

    // SDL2-TODO SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // clear out any events waiting at the start and center the mouse
  
    while (SDL_PollEvent(&dummy));

    initialized = true;

    // Call I_ShutdownGraphics on quit

    I_AtExit(I_ShutdownGraphics, true);
}

void I_InitDiskFlash(int x, int y, char *graphic)
{
    void *pic;
    byte *temp;

    if (!show_diskicon || !mode_y)
    {
        return;
    }

    diskicon_pos_x = x;
    diskicon_pos_y = y;

    pic = W_CacheLumpName(graphic, PU_CACHE);

    temp = destpixels;
    destpixels = tempscreen;

    V_DrawPatchDirect(SCREENWIDTH - LOADING_DISK_W,
                      SCREENHEIGHT - LOADING_DISK_H, pic);

    destpixels = temp;

    W_ReleaseLumpName(graphic);
}

void I_BeginRead()
{
    int i;
    byte *screenloc = currentpixels
                    + diskicon_pos_y * SCREENWIDTH
                    + diskicon_pos_x;

    byte *backuploc = tempscreen
                    + (SCREENHEIGHT - 2 * LOADING_DISK_H) * SCREENWIDTH
                    + (SCREENWIDTH - LOADING_DISK_W);

    byte *diskloc = tempscreen
                    + (SCREENHEIGHT - LOADING_DISK_H) * SCREENWIDTH
                    + (SCREENWIDTH - LOADING_DISK_W);

    if (!show_diskicon || !mode_y || !initialized)
    {
        return;
    }

    for (i = 0; i < LOADING_DISK_H; i++)
    {
        memcpy(backuploc + i * SCREENWIDTH,
               screenloc + i * SCREENWIDTH,
               LOADING_DISK_W);
    }

    for (i = 0; i < LOADING_DISK_H; i++)
    {
        memcpy(screenloc + i * SCREENWIDTH,
               diskloc + i * SCREENWIDTH,
               LOADING_DISK_W);
    }

    if (currentscreen != destscreen)
    {
        I_DrawScreen(currentscreen);
    }
}

void I_EndRead()
{
    int i;
    byte *screenloc = currentpixels
                    + diskicon_pos_y * SCREENWIDTH
                    + diskicon_pos_x;

    byte *backuploc = tempscreen
                    + (SCREENHEIGHT - 2 * LOADING_DISK_H) * SCREENWIDTH
                    + (SCREENWIDTH - LOADING_DISK_W);

    if (!show_diskicon || !mode_y || !initialized)
    {
        return;
    }

    for (i = 0; i < LOADING_DISK_H; i++)
    {
        memcpy(screenloc + i * SCREENWIDTH,
               backuploc + i * SCREENWIDTH,
               LOADING_DISK_W);
    }
    if (currentscreen != destscreen)
    {
        I_DrawScreen(currentscreen);
    }
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("use_mouse",                 &usemouse);
    M_BindIntVariable("autoadjust_video_settings", &autoadjust_video_settings);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindIntVariable("screen_width",              &screen_width);
    M_BindIntVariable("screen_height",             &screen_height);
    M_BindIntVariable("grabmouse",                 &grabmouse);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindStringVariable("window_position",        &window_position);
    M_BindIntVariable("usegamma",                  &usegamma);
    M_BindIntVariable("png_screenshots",           &png_screenshots);
}
