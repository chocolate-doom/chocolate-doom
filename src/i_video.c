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
#include "SDL_opengl.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "icon.c"

#include "config.h"
#include "d_loop.h"
#include "deh_str.h"
#include "doomtype.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_diskicon.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

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

static SDL_Rect blit_rect = {
    0,
    0,
    SCREENWIDTH,
    SCREENHEIGHT
};

static uint32_t pixel_format;

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

// SDL video driver name

char *video_driver = "";

// Window position:

char *window_position = "center";

// SDL display number on which to run.

int video_display = 0;

// Screen width and height, from configuration file.

int window_width = SCREENWIDTH * 2;
int window_height = SCREENHEIGHT_4_3 * 2;

// Fullscreen mode, 0x0 for SDL_WINDOW_FULLSCREEN_DESKTOP.

int fullscreen_width = 0, fullscreen_height = 0;

// Maximum number of pixels to use for intermediate scale buffer.

static int max_scaling_buffer_pixels = 16000000;

// Run in full screen mode?  (int type for config code)

int fullscreen = true;

// Aspect ratio correction mode

int aspect_ratio_correct = true;
static int actualheight;

// Force integer scales for resolution-independent rendering

int integer_scaling = false;

// VGA Porch palette change emulation

int vga_porch_flash = false;

// Force software rendering, for systems which lack effective hardware
// acceleration

int force_software_renderer = false;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

static int startup_delay = 1000;

// Grab the mouse? (int type for config code). nograbmouse_override allows
// this to be temporarily disabled via the command line.

static int grabmouse = true;
static boolean nograbmouse_override = false;

// The screen buffer; this is modified to draw things to the screen

pixel_t *I_VideoBuffer = NULL;

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
static unsigned int last_resize_time;
#define RESIZE_DELAY 500

// Gamma correction level to use

int usegamma = 0;

// Joystick/gamepad hysteresis
unsigned int joywait = 0;

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

    if (nograbmouse_override || !grabmouse)
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

// Adjust window_width / window_height variables to be an an aspect
// ratio consistent with the aspect_ratio_correct variable.
static void AdjustWindowSize(void)
{
    if (window_width * actualheight <= window_height * SCREENWIDTH)
    {
        // We round up window_height if the ratio is not exact; this leaves
        // the result stable.
        window_height = (window_width * actualheight + SCREENWIDTH - 1) / SCREENWIDTH;
    }
    else
    {
        window_width = window_height * SCREENWIDTH / actualheight;
    }
}

static void HandleWindowEvent(SDL_WindowEvent *event)
{
    int i;

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

        case SDL_WINDOWEVENT_FOCUS_GAINED:
            window_focused = true;
            break;

        case SDL_WINDOWEVENT_FOCUS_LOST:
            window_focused = false;
            break;

        // We want to save the user's preferred monitor to use for running the
        // game, so that next time we're run we start on the same display. So
        // every time the window is moved, find which display we're now on and
        // update the video_display config variable.

        case SDL_WINDOWEVENT_MOVED:
            i = SDL_GetWindowDisplayIndex(screen);
            if (i >= 0)
            {
                video_display = i;
            }
            break;

        default:
            break;
    }
}

static boolean ToggleFullScreenKeyShortcut(SDL_Keysym *sym)
{
    Uint16 flags = (KMOD_LALT | KMOD_RALT);
#if defined(__MACOSX__)
    flags |= (KMOD_LGUI | KMOD_RGUI);
#endif
    return sym->scancode == SDL_SCANCODE_RETURN && (sym->mod & flags) != 0;
}

static void I_ToggleFullScreen(void)
{
    unsigned int flags = 0;

    // TODO: Consider implementing fullscreen toggle for SDL_WINDOW_FULLSCREEN
    // (mode-changing) setup. This is hard because we have to shut down and
    // restart again.
    if (fullscreen_width != 0 || fullscreen_height != 0)
    {
        return;
    }

    fullscreen = !fullscreen;

    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_SetWindowFullscreen(screen, flags);

    if (!fullscreen)
    {
        AdjustWindowSize();
        SDL_SetWindowSize(screen, window_width, window_height);
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
                if (ToggleFullScreenKeyShortcut(&sdlevent.key.keysym))
                {
                    I_ToggleFullScreen();
                    break;
                }
                // deliberate fall-though

            case SDL_KEYUP:
		I_HandleKeyboardEvent(&sdlevent);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
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

    if (joywait < I_GetTime())
    {
        I_UpdateJoystick();
    }
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
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

static void LimitTextureSize(int *w_upscale, int *h_upscale)
{
    SDL_RendererInfo rinfo;
    int orig_w, orig_h;

    orig_w = *w_upscale;
    orig_h = *h_upscale;

    // Query renderer and limit to maximum texture dimensions of hardware:
    if (SDL_GetRendererInfo(renderer, &rinfo) != 0)
    {
        I_Error("CreateUpscaledTexture: SDL_GetRendererInfo() call failed: %s",
                SDL_GetError());
    }

    while (*w_upscale * SCREENWIDTH > rinfo.max_texture_width)
    {
        --*w_upscale;
    }
    while (*h_upscale * SCREENHEIGHT > rinfo.max_texture_height)
    {
        --*h_upscale;
    }

    if ((*w_upscale < 1 && rinfo.max_texture_width > 0) ||
        (*h_upscale < 1 && rinfo.max_texture_height > 0))
    {
        I_Error("CreateUpscaledTexture: Can't create a texture big enough for "
                "the whole screen! Maximum texture size %dx%d",
                rinfo.max_texture_width, rinfo.max_texture_height);
    }

    // We limit the amount of texture memory used for the intermediate buffer,
    // since beyond a certain point there are diminishing returns. Also,
    // depending on the hardware there may be performance problems with very
    // huge textures, so the user can use this to reduce the maximum texture
    // size if desired.

    if (max_scaling_buffer_pixels < SCREENWIDTH * SCREENHEIGHT)
    {
        I_Error("CreateUpscaledTexture: max_scaling_buffer_pixels too small "
                "to create a texture buffer: %d < %d",
                max_scaling_buffer_pixels, SCREENWIDTH * SCREENHEIGHT);
    }

    while (*w_upscale * *h_upscale * SCREENWIDTH * SCREENHEIGHT
           > max_scaling_buffer_pixels)
    {
        if (*w_upscale > *h_upscale)
        {
            --*w_upscale;
        }
        else
        {
            --*h_upscale;
        }
    }

    if (*w_upscale != orig_w || *h_upscale != orig_h)
    {
        printf("CreateUpscaledTexture: Limited texture size to %dx%d "
               "(max %d pixels, max texture size %dx%d)\n",
               *w_upscale * SCREENWIDTH, *h_upscale * SCREENHEIGHT,
               max_scaling_buffer_pixels,
               rinfo.max_texture_width, rinfo.max_texture_height);
    }
}

static void CreateUpscaledTexture(boolean force)
{
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

    LimitTextureSize(&w_upscale, &h_upscale);

    // Create a new texture only if the upscale factors have actually changed.

    if (h_upscale == h_upscale_old && w_upscale == w_upscale_old && !force)
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
                                pixel_format,
                                SDL_TEXTUREACCESS_TARGET,
                                w_upscale*SCREENWIDTH,
                                h_upscale*SCREENHEIGHT);
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

    if (need_resize)
    {
        if (SDL_GetTicks() > last_resize_time + RESIZE_DELAY)
        {
            int flags;
            // When the window is resized (we're not in fullscreen mode),
            // save the new window size.
            flags = SDL_GetWindowFlags(screen);
            if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0)
            {
                SDL_GetWindowSize(screen, &window_width, &window_height);

                // Adjust the window by resizing again so that the window
                // is the right aspect ratio.
                AdjustWindowSize();
                SDL_SetWindowSize(screen, window_width, window_height);
            }
            CreateUpscaledTexture(false);
            need_resize = false;
            palette_to_set = true;
        }
        else
        {
            return;
        }
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
	    I_VideoBuffer[ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*4 ; i+=4)
	    I_VideoBuffer[ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    }

    // Draw disk icon before blit, if necessary.
    V_DrawDiskIcon();

    if (palette_to_set)
    {
        SDL_SetPaletteColors(screenbuffer->format->palette, palette, 0, 256);
        palette_to_set = false;

        if (vga_porch_flash)
        {
            // "flash" the pillars/letterboxes with palette changes, emulating
            // VGA "porch" behaviour (GitHub issue #832)
            SDL_SetRenderDrawColor(renderer, palette[0].r, palette[0].g,
                palette[0].b, SDL_ALPHA_OPAQUE);
        }
    }

    // Blit from the paletted 8-bit screen buffer to the intermediate
    // 32-bit RGBA buffer that we can load into the texture.

    SDL_LowerBlit(screenbuffer, &blit_rect, rgbabuffer, &blit_rect);

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

    // Restore background and undo the disk indicator, if it was drawn.
    V_RestoreDiskBackground();
}


//
// I_ReadScreen
//
void I_ReadScreen (pixel_t* scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH*SCREENHEIGHT*sizeof(*scr));
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
    // Pick 320x200 or 320x240, depending on aspect ratio correct

    window_width = factor * SCREENWIDTH;
    window_height = factor * actualheight;
    fullscreen = false;
}

void I_GraphicsCheckCommandLine(void)
{
    int i;

    //!
    // @category video
    // @vanilla
    //
    // Disable blitting the screen.
    //

    noblit = M_CheckParm ("-noblit");

    //!
    // @category video 
    //
    // Don't grab the mouse when running in windowed mode.
    //

    nograbmouse_override = M_ParmExists("-nograbmouse");

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
    // Specify the screen width, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-width", 1);

    if (i > 0)
    {
        window_width = atoi(myargv[i + 1]);
        window_height = window_width * 2;
        AdjustWindowSize();
        fullscreen = false;
    }

    //!
    // @category video
    // @arg <y>
    //
    // Specify the screen height, in pixels. Implies -window.
    //

    i = M_CheckParmWithArgs("-height", 1);

    if (i > 0)
    {
        window_height = atoi(myargv[i + 1]);
        window_width = window_height * 2;
        AdjustWindowSize();
        fullscreen = false;
    }

    //!
    // @category video
    // @arg <WxY>
    //
    // Specify the dimensions of the window. Implies -window.
    //

    i = M_CheckParmWithArgs("-geometry", 1);

    if (i > 0)
    {
        int w, h, s;

        s = sscanf(myargv[i + 1], "%ix%i", &w, &h);
        if (s == 2)
        {
            window_width = w;
            window_height = h;
            fullscreen = false;
        }
    }

    //!
    // @category video
    //
    // Don't scale up the screen. Implies -window.
    //

    if (M_CheckParm("-1")) 
    {
        SetScaleFactor(1);
    }

    //!
    // @category video
    //
    // Double up the screen to 2x its normal size. Implies -window.
    //

    if (M_CheckParm("-2")) 
    {
        SetScaleFactor(2);
    }

    //!
    // @category video
    //
    // Double up the screen to 3x its normal size. Implies -window.
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

// Check the display bounds of the display referred to by 'video_display' and
// set x and y to a location that places the window in the center of that
// display.
static void CenterWindow(int *x, int *y, int w, int h)
{
    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(video_display, &bounds) < 0)
    {
        fprintf(stderr, "CenterWindow: Failed to read display bounds "
                        "for display #%d!\n", video_display);
        return;
    }

    *x = bounds.x + SDL_max((bounds.w - w) / 2, 0);
    *y = bounds.y + SDL_max((bounds.h - h) / 2, 0);
}

void I_GetWindowPosition(int *x, int *y, int w, int h)
{
    // Check that video_display corresponds to a display that really exists,
    // and if it doesn't, reset it.
    if (video_display < 0 || video_display >= SDL_GetNumVideoDisplays())
    {
        fprintf(stderr,
                "I_GetWindowPosition: We were configured to run on display #%d, "
                "but it no longer exists (max %d). Moving to display 0.\n",
                video_display, SDL_GetNumVideoDisplays() - 1);
        video_display = 0;
    }

    // in fullscreen mode, the window "position" still matters, because
    // we use it to control which display we run fullscreen on.

    if (fullscreen)
    {
        CenterWindow(x, y, w, h);
        return;
    }

    // in windowed mode, the desired window position can be specified
    // in the configuration file.

    if (window_position == NULL || !strcmp(window_position, ""))
    {
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
    else if (!strcmp(window_position, "center"))
    {
        // Note: SDL has a SDL_WINDOWPOS_CENTER, but this is useless for our
        // purposes, since we also want to control which display we appear on.
        // So we have to do this ourselves.
        CenterWindow(x, y, w, h);
    }
    else if (sscanf(window_position, "%i,%i", x, y) != 2)
    {
        // invalid format: revert to default
        fprintf(stderr, "I_GetWindowPosition: invalid window_position setting\n");
        *x = *y = SDL_WINDOWPOS_UNDEFINED;
    }
}

static void SetVideoMode(void)
{
    int w, h;
    int x, y;
    unsigned int rmask, gmask, bmask, amask;
    int unused_bpp;
    int window_flags = 0, renderer_flags = 0;
    SDL_DisplayMode mode;

    w = window_width;
    h = window_height;

    // In windowed mode, the window can be resized while the game is
    // running.
    window_flags = SDL_WINDOW_RESIZABLE;

    // Set the highdpi flag - this makes a big difference on Macs with
    // retina displays, especially when using small window sizes.
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    if (fullscreen)
    {
        if (fullscreen_width == 0 && fullscreen_height == 0)
        {
            // This window_flags means "Never change the screen resolution!
            // Instead, draw to the entire screen by scaling the texture
            // appropriately".
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            w = fullscreen_width;
            h = fullscreen_height;
            window_flags |= SDL_WINDOW_FULLSCREEN;
        }
    }

    I_GetWindowPosition(&x, &y, w, h);

    // Create window and renderer contexts. We set the window title
    // later anyway and leave the window position "undefined". If
    // "window_flags" contains the fullscreen flag (see above), then
    // w and h are ignored.

    if (screen == NULL)
    {
        screen = SDL_CreateWindow(NULL, x, y, w, h, window_flags);

        if (screen == NULL)
        {
            I_Error("Error creating window for video startup: %s",
            SDL_GetError());
        }

        pixel_format = SDL_GetWindowPixelFormat(screen);

        SDL_SetWindowMinimumSize(screen, SCREENWIDTH, actualheight);

        I_InitWindowTitle();
        I_InitWindowIcon();
    }

    // The SDL_RENDERER_TARGETTEXTURE flag is required to render the
    // intermediate texture into the upscaled texture.
    renderer_flags = SDL_RENDERER_TARGETTEXTURE;
	
    if (SDL_GetCurrentDisplayMode(video_display, &mode) != 0)
    {
        I_Error("Could not get display mode for video display #%d: %s",
        video_display, SDL_GetError());
    }

    // Turn on vsync if we aren't in a -timedemo
    if (!singletics && mode.refresh_rate > 0)
    {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    if (force_software_renderer)
    {
        renderer_flags |= SDL_RENDERER_SOFTWARE;
        renderer_flags &= ~SDL_RENDERER_PRESENTVSYNC;
    }

    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
    }

    renderer = SDL_CreateRenderer(screen, -1, renderer_flags);

    if (renderer == NULL)
    {
        I_Error("Error creating renderer for screen window: %s",
                SDL_GetError());
    }

    // Important: Set the "logical size" of the rendering context. At the same
    // time this also defines the aspect ratio that is preserved while scaling
    // and stretching the texture into the window.

    SDL_RenderSetLogicalSize(renderer,
                             SCREENWIDTH,
                             actualheight);

    // Force integer scales for resolution-independent rendering.

#if SDL_VERSION_ATLEAST(2, 0, 5)
    SDL_RenderSetIntegerScale(renderer, integer_scaling);
#endif

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Create the 8-bit paletted and the 32-bit RGBA screenbuffer surfaces.

    if (screenbuffer == NULL)
    {
        screenbuffer = SDL_CreateRGBSurface(0,
                                            SCREENWIDTH, SCREENHEIGHT, 8,
                                            0, 0, 0, 0);
        SDL_FillRect(screenbuffer, NULL, 0);
    }

    // Format of rgbabuffer must match the screen pixel format because we
    // import the surface data into the texture.
    if (rgbabuffer == NULL)
    {
        SDL_PixelFormatEnumToMasks(pixel_format, &unused_bpp,
                                   &rmask, &gmask, &bmask, &amask);
        rgbabuffer = SDL_CreateRGBSurface(0,
                                          SCREENWIDTH, SCREENHEIGHT, 32,
                                          rmask, gmask, bmask, amask);
        SDL_FillRect(rgbabuffer, NULL, 0);
    }

    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }

    // Set the scaling quality for rendering the intermediate texture into
    // the upscaled texture to "nearest", which is gritty and pixelated and
    // resembles software scaling pretty well.

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Create the intermediate texture that the RGBA surface gets loaded into.
    // The SDL_TEXTUREACCESS_STREAMING flag means that this texture's content
    // is going to change frequently.

    texture = SDL_CreateTexture(renderer,
                                pixel_format,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREENWIDTH, SCREENHEIGHT);

    // Initially create the upscaled texture for rendering to screen

    CreateUpscaledTexture(true);
}

static const char *hw_emu_warning = 
"===========================================================================\n"
"WARNING: it looks like you are using a software GL implementation.\n"
"To improve performance, try setting force_software_renderer in your\n"
"configuration file.\n"
"===========================================================================\n";

static void CheckGLVersion(void)
{
    const char * version;
    typedef const GLubyte* (APIENTRY * glStringFn_t)(GLenum);
    glStringFn_t glfp = (glStringFn_t)SDL_GL_GetProcAddress("glGetString");

    if (glfp)
    {
        version = (const char *)glfp(GL_VERSION);

        if (version && strstr(version, "Mesa"))
        {
            printf("%s", hw_emu_warning);
        }
    }
}

void I_InitGraphics(void)
{
    SDL_Event dummy;
    byte *doompal;
    char *env;

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

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error("Failed to initialize video: %s", SDL_GetError());
    }

    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    if (screensaver_mode)
    {
        fullscreen = true;
    }

    if (aspect_ratio_correct)
    {
        actualheight = SCREENHEIGHT_4_3;
    }
    else
    {
        actualheight = SCREENHEIGHT;
    }

    // Create the game window; this may switch graphic modes depending
    // on configuration.
    AdjustWindowSize();
    SetVideoMode();

    // We might have poor performance if we are using an emulated
    // HW accelerator. Check for Mesa and warn if we're using it.
    CheckGLVersion();

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

    I_VideoBuffer = screenbuffer->pixels;
    V_RestoreBuffer();

    // Clear the screen to black.

    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

    // clear out any events waiting at the start and center the mouse
  
    while (SDL_PollEvent(&dummy));

    initialized = true;

    // Call I_ShutdownGraphics on quit

    I_AtExit(I_ShutdownGraphics, true);
}

// Bind all variables controlling video options into the configuration
// file system.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("use_mouse",                 &usemouse);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("video_display",             &video_display);
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("integer_scaling",           &integer_scaling);
    M_BindIntVariable("vga_porch_flash",           &vga_porch_flash);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindIntVariable("fullscreen_width",          &fullscreen_width);
    M_BindIntVariable("fullscreen_height",         &fullscreen_height);
    M_BindIntVariable("force_software_renderer",   &force_software_renderer);
    M_BindIntVariable("max_scaling_buffer_pixels", &max_scaling_buffer_pixels);
    M_BindIntVariable("window_width",              &window_width);
    M_BindIntVariable("window_height",             &window_height);
    M_BindIntVariable("grabmouse",                 &grabmouse);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindStringVariable("window_position",        &window_position);
    M_BindIntVariable("usegamma",                  &usegamma);
    M_BindIntVariable("png_screenshots",           &png_screenshots);
}
