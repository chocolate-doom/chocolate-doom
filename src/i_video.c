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
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "icon.c"

#include "config.h"
#include "deh_str.h"
#include "doomtype.h"
#include "doomkeys.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_scale.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_diskicon.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Lookup table for mapping ASCII characters to their equivalent when
// shift is pressed on an American layout keyboard:

static const char shiftxform[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};

// Non aspect ratio-corrected modes (direct multiples of 320x200)

static screen_mode_t *screen_modes[] = {
    &mode_scale_1x,
    &mode_scale_2x,
    &mode_scale_3x,
    &mode_scale_4x,
    &mode_scale_5x,
};

// Aspect ratio corrected modes (4:3 ratio)

static screen_mode_t *screen_modes_corrected[] = {

    // Vertically stretched modes (320x200 -> 320x240 and multiples)

    &mode_stretch_1x,
    &mode_stretch_2x,
    &mode_stretch_3x,
    &mode_stretch_4x,
    &mode_stretch_5x,

    // Horizontally squashed modes (320x200 -> 256x200 and multiples)

    &mode_squash_1x,
    &mode_squash_2x,
    &mode_squash_3x,
    &mode_squash_4x,
};

// SDL video driver name

char *video_driver = "";

// Window position:

static char *window_position = "";

// SDL surface for the screen.

static SDL_Surface *screen;

// Window title

static char *window_title = "";

// Intermediate 8-bit buffer that we draw to instead of 'screen'.
// This is used when we are rendering in 32-bit screen mode.
// When in a real 8-bit screen mode, screenbuffer == screen.

static SDL_Surface *screenbuffer = NULL;

// palette

static SDL_Color palette[256];
static boolean palette_to_set;

// display has been set up?

static boolean initialized = false;

// disable mouse?

static boolean nomouse = false;
int usemouse = 1;

// Bit mask of mouse button state.

static unsigned int mouse_button_state = 0;

// Disallow mouse and joystick movement to cause forward/backward
// motion.  Specified with the '-novert' command line parameter.
// This is an int to allow saving to config file

int novert = 0;

// Save screenshots in PNG format.

int png_screenshots = 0;

// if true, I_VideoBuffer is screen->pixels

static boolean native_surface;

// Screen width and height, from configuration file.

int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;

// Color depth.

int screen_bpp = 0;

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

boolean screenvisible;

// If true, we display dots at the bottom of the screen to 
// indicate FPS.

static boolean display_fps_dots;

// If this is true, the screen is rendered but not blitted to the
// video buffer.

static boolean noblit;

// Callback function to invoke to determine whether to grab the 
// mouse pointer.

static grabmouse_callback_t grabmouse_callback = NULL;

static boolean window_focused;

// Empty mouse cursor

static SDL_Cursor *cursors[2];

// The screen mode and scale functions being used

static screen_mode_t *screen_mode;

// Window resize state.

static boolean need_resize = false;
static unsigned int resize_w, resize_h;
static unsigned int last_resize_time;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;

// Is the shift key currently down?

static int shiftdown = 0;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

static void ApplyWindowResize(unsigned int w, unsigned int h);

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

// Update the value of window_focused when we get a focus event
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we dont move the mouse around if we aren't focused either.

static void UpdateFocus(void)
{
    Uint8 state;

    state = SDL_GetAppState();

    // We should have input (keyboard) focus and be visible 
    // (not minimized)

    window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);

    // Should the screen be grabbed?

    screenvisible = (state & SDL_APPACTIVE) != 0;
}

// Show or hide the mouse cursor. We have to use different techniques
// depending on the OS.

static void SetShowCursor(boolean show)
{
    // On Windows, using SDL_ShowCursor() adds lag to the mouse input,
    // so work around this by setting an invisible cursor instead. On
    // other systems, it isn't possible to change the cursor, so this
    // hack has to be Windows-only. (Thanks to entryway for this)

#ifdef _WIN32
    if (show)
    {
        SDL_SetCursor(cursors[1]);
    }
    else
    {
        SDL_SetCursor(cursors[0]);
    }
#else
    SDL_ShowCursor(show);
#endif

    // When the cursor is hidden, grab the input.

    if (!screensaver_mode)
    {
        SDL_WM_GrabInput(!show);
    }
}

//
// Translates the SDL key
//

static int TranslateKey(SDL_keysym *sym)
{
    switch(sym->sym)
    {
      case SDLK_LEFT:	return KEY_LEFTARROW;
      case SDLK_RIGHT:	return KEY_RIGHTARROW;
      case SDLK_DOWN:	return KEY_DOWNARROW;
      case SDLK_UP:	return KEY_UPARROW;
      case SDLK_ESCAPE:	return KEY_ESCAPE;
      case SDLK_RETURN:	return KEY_ENTER;
      case SDLK_TAB:	return KEY_TAB;
      case SDLK_F1:	return KEY_F1;
      case SDLK_F2:	return KEY_F2;
      case SDLK_F3:	return KEY_F3;
      case SDLK_F4:	return KEY_F4;
      case SDLK_F5:	return KEY_F5;
      case SDLK_F6:	return KEY_F6;
      case SDLK_F7:	return KEY_F7;
      case SDLK_F8:	return KEY_F8;
      case SDLK_F9:	return KEY_F9;
      case SDLK_F10:	return KEY_F10;
      case SDLK_F11:	return KEY_F11;
      case SDLK_F12:	return KEY_F12;
      case SDLK_PRINT:  return KEY_PRTSCR;

      case SDLK_BACKSPACE: return KEY_BACKSPACE;
      case SDLK_DELETE:	return KEY_DEL;

      case SDLK_PAUSE:	return KEY_PAUSE;

      case SDLK_EQUALS: return KEY_EQUALS;

      case SDLK_MINUS:          return KEY_MINUS;

      case SDLK_LSHIFT:
      case SDLK_RSHIFT:
	return KEY_RSHIFT;
	
      case SDLK_LCTRL:
      case SDLK_RCTRL:
	return KEY_RCTRL;
	
      case SDLK_LALT:
      case SDLK_RALT:
      case SDLK_LMETA:
      case SDLK_RMETA:
        return KEY_RALT;

      case SDLK_CAPSLOCK: return KEY_CAPSLOCK;
      case SDLK_SCROLLOCK: return KEY_SCRLCK;
      case SDLK_NUMLOCK: return KEY_NUMLOCK;

      case SDLK_KP0: return KEYP_0;
      case SDLK_KP1: return KEYP_1;
      case SDLK_KP2: return KEYP_2;
      case SDLK_KP3: return KEYP_3;
      case SDLK_KP4: return KEYP_4;
      case SDLK_KP5: return KEYP_5;
      case SDLK_KP6: return KEYP_6;
      case SDLK_KP7: return KEYP_7;
      case SDLK_KP8: return KEYP_8;
      case SDLK_KP9: return KEYP_9;

      case SDLK_KP_PERIOD:   return KEYP_PERIOD;
      case SDLK_KP_MULTIPLY: return KEYP_MULTIPLY;
      case SDLK_KP_PLUS:     return KEYP_PLUS;
      case SDLK_KP_MINUS:    return KEYP_MINUS;
      case SDLK_KP_DIVIDE:   return KEYP_DIVIDE;
      case SDLK_KP_EQUALS:   return KEYP_EQUALS;
      case SDLK_KP_ENTER:    return KEYP_ENTER;

      case SDLK_HOME: return KEY_HOME;
      case SDLK_INSERT: return KEY_INS;
      case SDLK_END: return KEY_END;
      case SDLK_PAGEUP: return KEY_PGUP;
      case SDLK_PAGEDOWN: return KEY_PGDN;

#ifdef SDL_HAVE_APP_KEYS
        case SDLK_APP1:        return KEY_F1;
        case SDLK_APP2:        return KEY_F2;
        case SDLK_APP3:        return KEY_F3;
        case SDLK_APP4:        return KEY_F4;
        case SDLK_APP5:        return KEY_F5;
        case SDLK_APP6:        return KEY_F6;
#endif

      default:
        return tolower(sym->sym);
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

static void UpdateMouseButtonState(unsigned int button, boolean on)
{
    event_t event;

    if (button < SDL_BUTTON_LEFT || button > MAX_MOUSE_BUTTONS)
    {
        return;
    }

    // Note: button "0" is left, button "1" is right,
    // button "2" is middle for Doom.  This is different
    // to how SDL sees things.

    switch (button)
    {
        case SDL_BUTTON_LEFT:
            button = 0;
            break;

        case SDL_BUTTON_RIGHT:
            button = 1;
            break;

        case SDL_BUTTON_MIDDLE:
            button = 2;
            break;

        default:
            // SDL buttons are indexed from 1.
            --button;
            break;
    }

    // Turn bit representing this button on or off.

    if (on)
    {
        mouse_button_state |= (1 << button);
    }
    else
    {
        mouse_button_state &= ~(1 << button);
    }

    // Post an event with the new button state.

    event.type = ev_mouse;
    event.data1 = mouse_button_state;
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
}

static int AccelerateMouse(int val)
{
    if (val < 0)
        return -AccelerateMouse(-val);

    if (val > mouse_threshold)
    {
        return (int)((val - mouse_threshold) * mouse_acceleration + mouse_threshold);
    }
    else
    {
        return val;
    }
}

// Get the equivalent ASCII (Unicode?) character for a keypress.

static int GetTypedChar(SDL_Event *event)
{
    int key;

    // If Vanilla keyboard mapping enabled, the keyboard
    // scan code is used to give the character typed.
    // This does not change depending on keyboard layout.
    // If you have a German keyboard, pressing 'z' will
    // give 'y', for example.  It is desirable to be able
    // to fix this so that people with non-standard 
    // keyboard mappings can type properly.  If vanilla
    // mode is disabled, use the properly translated 
    // version.

    if (vanilla_keyboard_mapping)
    {
        key = TranslateKey(&event->key.keysym);

        // Is shift held down?  If so, perform a translation.

        if (shiftdown > 0)
        {
            if (key >= 0 && key < arrlen(shiftxform))
            {
                key = shiftxform[key];
            }
            else
            {
                key = 0;
            }
        }

        return key;
    }
    else
    {
        // Unicode value, from key layout.

        return tolower(event->key.keysym.unicode);
    }
}

static void UpdateShiftStatus(SDL_Event *event)
{
    int change;

    if (event->type == SDL_KEYDOWN)
    {
        change = 1;
    }
    else if (event->type == SDL_KEYUP)
    {
        change = -1;
    }
    else
    {
        return;
    }

    if (event->key.keysym.sym == SDLK_LSHIFT 
     || event->key.keysym.sym == SDLK_RSHIFT)
    {
        shiftdown += change;
    }
}

void I_GetEvent(void)
{
    SDL_Event sdlevent;
    event_t event;

    // possibly not needed
    
    SDL_PumpEvents();

    // put event-grabbing stuff in here
    
    while (SDL_PollEvent(&sdlevent))
    {
        // ignore mouse events when the window is not focused

        if (!window_focused 
         && (sdlevent.type == SDL_MOUSEMOTION
          || sdlevent.type == SDL_MOUSEBUTTONDOWN
          || sdlevent.type == SDL_MOUSEBUTTONUP))
        {
            continue;
        }

        if (screensaver_mode && sdlevent.type == SDL_QUIT)
        {
            I_Quit();
        }

        UpdateShiftStatus(&sdlevent);

        // process event
        
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                // data1 has the key pressed, data2 has the character
                // (shift-translated, etc)
                event.type = ev_keydown;
                event.data1 = TranslateKey(&sdlevent.key.keysym);
                event.data2 = GetTypedChar(&sdlevent);

                if (event.data1 != 0)
                {
                    D_PostEvent(&event);
                }
                break;

            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = TranslateKey(&sdlevent.key.keysym);

                // data2 is just initialized to zero for ev_keyup.
                // For ev_keydown it's the shifted Unicode character
                // that was typed, but if something wants to detect
                // key releases it should do so based on data1
                // (key ID), not the printable char.

                event.data2 = 0;

                if (event.data1 != 0)
                {
                    D_PostEvent(&event);
                }
                break;

                /*
            case SDL_MOUSEMOTION:
                event.type = ev_mouse;
                event.data1 = mouse_button_state;
                event.data2 = AccelerateMouse(sdlevent.motion.xrel);
                event.data3 = -AccelerateMouse(sdlevent.motion.yrel);
                D_PostEvent(&event);
                break;
                */

            case SDL_MOUSEBUTTONDOWN:
		if (usemouse && !nomouse)
		{
                    UpdateMouseButtonState(sdlevent.button.button, true);
		}
                break;

            case SDL_MOUSEBUTTONUP:
		if (usemouse && !nomouse)
		{
                    UpdateMouseButtonState(sdlevent.button.button, false);
		}
                break;

            case SDL_QUIT:
                event.type = ev_quit;
                D_PostEvent(&event);
                break;

            case SDL_ACTIVEEVENT:
                // need to update our focus state
                UpdateFocus();
                break;

            case SDL_VIDEOEXPOSE:
                palette_to_set = true;
                break;

            case SDL_RESIZABLE:
                need_resize = true;
                resize_w = sdlevent.resize.w;
                resize_h = sdlevent.resize.h;
                last_resize_time = SDL_GetTicks();
                break;

            default:
                break;
        }
    }
}

// Warp the mouse back to the middle of the screen

static void CenterMouse(void)
{
    // Warp the the screen center

    SDL_WarpMouse(screen->w / 2, screen->h / 2);

    // Clear any relative movement caused by warping

    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}

//
// Read the change in mouse state to generate mouse motion events
//
// This is to combine all mouse movement for a tic into one mouse
// motion event.

static void I_ReadMouse(void)
{
    int x, y;
    event_t ev;

    SDL_GetRelativeMouseState(&x, &y);

    if (x != 0 || y != 0) 
    {
        ev.type = ev_mouse;
        ev.data1 = mouse_button_state;
        ev.data2 = AccelerateMouse(x);

        if (!novert)
        {
            ev.data3 = -AccelerateMouse(y);
        }
        else
        {
            ev.data3 = 0;
        }
        
        D_PostEvent(&ev);
    }

    if (MouseShouldBeGrabbed())
    {
        CenterMouse();
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
        CenterMouse();
    }
    else if (!grab && currently_grabbed)
    {
        SetShowCursor(true);

        // When releasing the mouse from grab, warp the mouse cursor to
        // the bottom-right of the screen. This is a minimally distracting
        // place for it to appear - we may only have released the grab
        // because we're at an end of level intermission screen, for
        // example.

        SDL_WarpMouse(screen->w - 16, screen->h - 16);
        SDL_PumpEvents();
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;

}

// Update a small portion of the screen
//
// Does stretching and buffer blitting if neccessary
//
// Return true if blit was successful.

static boolean BlitArea(int x1, int y1, int x2, int y2)
{
    int x_offset, y_offset;
    boolean result;

    // No blit needed on native surface

    if (native_surface)
    {
	return true;
    }

    x_offset = (screenbuffer->w - screen_mode->width) / 2;
    y_offset = (screenbuffer->h - screen_mode->height) / 2;

    if (SDL_LockSurface(screenbuffer) >= 0)
    {
        I_InitScale(I_VideoBuffer,
                    (byte *) screenbuffer->pixels
                                + (y_offset * screenbuffer->pitch)
                                + x_offset,
                    screenbuffer->pitch);
        result = screen_mode->DrawScreen(x1, y1, x2, y2);
      	SDL_UnlockSurface(screenbuffer);
    }
    else
    {
        result = false;
    }

    return result;
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    static int	lasttic;
    int		tics;
    int		i;

    if (!initialized)
        return;

    if (noblit)
        return;

    if (need_resize && SDL_GetTicks() > last_resize_time + 500)
    {
        ApplyWindowResize(resize_w, resize_h);
        need_resize = false;
        palette_to_set = true;
    }

    UpdateGrab();

    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;

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

    // draw to screen

    BlitArea(0, 0, SCREENWIDTH, SCREENHEIGHT);

    if (palette_to_set)
    {
        SDL_SetColors(screenbuffer, palette, 0, 256);
        palette_to_set = false;

        // In native 8-bit mode, if we have a palette to set, the act
        // of setting the palette updates the screen

        if (screenbuffer == screen)
        {
            return;
        }
    }

    // In 8in32 mode, we must blit from the fake 8-bit screen buffer
    // to the real screen before doing a screen flip.

    if (screenbuffer != screen)
    {
        SDL_Rect dst_rect;

        // Center the buffer within the full screen space.

        dst_rect.x = (screen->w - screenbuffer->w) / 2;
        dst_rect.y = (screen->h - screenbuffer->h) / 2;

        SDL_BlitSurface(screenbuffer, NULL, screen, &dst_rect);
    }

    SDL_Flip(screen);

    // Restore background and undo the disk indicator, if it was drawn.
    V_RestoreDiskBackground();
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
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
    SDL_WM_SetCaption(buf, NULL);
    free(buf);
}

// Set the application icon

void I_InitWindowIcon(void)
{
    SDL_Surface *surface;
    Uint8 *mask;
    int i;

    // Generate the mask

    mask = malloc(icon_w * icon_h / 8);
    memset(mask, 0, icon_w * icon_h / 8);

    for (i=0; i<icon_w * icon_h; ++i)
    {
        if (icon_data[i * 3] != 0x00
         || icon_data[i * 3 + 1] != 0x00
         || icon_data[i * 3 + 2] != 0x00)
        {
            mask[i / 8] |= 1 << (7 - i % 8);
        }
    }

    surface = SDL_CreateRGBSurfaceFrom(icon_data,
                                       icon_w,
                                       icon_h,
                                       24,
                                       icon_w * 3,
                                       0xff << 0,
                                       0xff << 8,
                                       0xff << 16,
                                       0);

    SDL_WM_SetIcon(surface, mask);
    SDL_FreeSurface(surface);
    free(mask);
}

// Pick the modes list to use:

static void GetScreenModes(screen_mode_t ***modes_list, int *num_modes)
{
    if (aspect_ratio_correct)
    {
        *modes_list = screen_modes_corrected;
        *num_modes = arrlen(screen_modes_corrected);
    }
    else
    {
        *modes_list = screen_modes;
        *num_modes = arrlen(screen_modes);
    }
}

// Find which screen_mode_t to use for the given width and height.

static screen_mode_t *I_FindScreenMode(int w, int h)
{
    screen_mode_t **modes_list;
    screen_mode_t *best_mode;
    int modes_list_length;
    int num_pixels;
    int best_num_pixels;
    int i;

    // Special case: 320x200 and 640x400 are available even if aspect 
    // ratio correction is turned on.  These modes have non-square
    // pixels.

    if (fullscreen)
    {
        if (w == SCREENWIDTH && h == SCREENHEIGHT)
        {
            return &mode_scale_1x;
        }
        else if (w == SCREENWIDTH*2 && h == SCREENHEIGHT*2)
        {
            return &mode_scale_2x;
        }
    }

    GetScreenModes(&modes_list, &modes_list_length);

    // Find the biggest screen_mode_t in the list that fits within these 
    // dimensions

    best_mode = NULL;
    best_num_pixels = 0;

    for (i=0; i<modes_list_length; ++i) 
    {
        // Will this fit within the dimensions? If not, ignore.

        if (modes_list[i]->width > w || modes_list[i]->height > h)
        {
            continue;
        }

        num_pixels = modes_list[i]->width * modes_list[i]->height;

        if (num_pixels > best_num_pixels)
        {
            // This is a better mode than the current one

            best_mode = modes_list[i];
            best_num_pixels = num_pixels;
        }
    }

    return best_mode;
}

// Adjust to an appropriate fullscreen mode.
// Returns true if successful.

static boolean AutoAdjustFullscreen(void)
{
    SDL_Rect **modes;
    SDL_Rect *best_mode;
    screen_mode_t *screen_mode;
    int diff, best_diff;
    int i;

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

    // No fullscreen modes available at all?

    if (modes == NULL || modes == (SDL_Rect **) -1 || *modes == NULL)
    {
        return false;
    }

    // Find the best mode that matches the mode specified in the
    // configuration file

    best_mode = NULL;
    best_diff = INT_MAX;

    for (i=0; modes[i] != NULL; ++i)
    {
        //printf("%ix%i?\n", modes[i]->w, modes[i]->h);

        // What screen_mode_t would be used for this video mode?

        screen_mode = I_FindScreenMode(modes[i]->w, modes[i]->h);

        // Never choose a screen mode that we cannot run in, or
        // is poor quality for fullscreen

        if (screen_mode == NULL || screen_mode->poor_quality)
        {
        //    printf("\tUnsupported / poor quality\n");
            continue;
        }

        // Do we have the exact mode?
        // If so, no autoadjust needed

        if (screen_width == modes[i]->w && screen_height == modes[i]->h)
        {
        //    printf("\tExact mode!\n");
            return true;
        }

        // Is this mode better than the current mode?

        diff = (screen_width - modes[i]->w) * (screen_width - modes[i]->w)
             + (screen_height - modes[i]->h) * (screen_height - modes[i]->h);

        if (diff < best_diff)
        {
        //    printf("\tA valid mode\n");
            best_mode = modes[i];
            best_diff = diff;
        }
    }

    if (best_mode == NULL)
    {
        // Unable to find a valid mode!

        return false;
    }

    printf("I_InitGraphics: %ix%i mode not supported on this machine.\n",
           screen_width, screen_height);

    screen_width = best_mode->w;
    screen_height = best_mode->h;

    return true;
}

// Auto-adjust to a valid windowed mode.

static void AutoAdjustWindowed(void)
{
    screen_mode_t *best_mode;

    // Find a screen_mode_t to fit within the current settings

    best_mode = I_FindScreenMode(screen_width, screen_height);

    if (best_mode == NULL)
    {
        // Nothing fits within the current settings.
        // Pick the closest to 320x200 possible.

        best_mode = I_FindScreenMode(SCREENWIDTH, SCREENHEIGHT_4_3);
    }

    // Switch to the best mode if necessary.

    if (best_mode->width != screen_width || best_mode->height != screen_height)
    {
        printf("I_InitGraphics: Cannot run at specified mode: %ix%i\n",
               screen_width, screen_height);

        screen_width = best_mode->width;
        screen_height = best_mode->height;
    }
}

// Auto-adjust to a valid color depth.

static void AutoAdjustColorDepth(void)
{
    SDL_Rect **modes;
    SDL_PixelFormat format;
    const SDL_VideoInfo *info;
    int flags;

    // If screen_bpp=0, we should use the current (default) pixel depth.
    // Fetch it from SDL.

    if (screen_bpp == 0)
    {
        info = SDL_GetVideoInfo();

        if (info != NULL && info->vfmt != NULL)
        {
            screen_bpp = info->vfmt->BitsPerPixel;
        }
    }

    if (fullscreen)
    {
        flags = SDL_FULLSCREEN;
    }
    else
    {
        flags = 0;
    }

    format.BitsPerPixel = screen_bpp;
    format.BytesPerPixel = (screen_bpp + 7) / 8;

    // Are any screen modes supported at the configured color depth?

    modes = SDL_ListModes(&format, flags);

    // If not, we must autoadjust to something sensible.

    if (modes == NULL)
    {
        printf("I_InitGraphics: %ibpp color depth not supported.\n",
               screen_bpp);

        info = SDL_GetVideoInfo();

        if (info != NULL && info->vfmt != NULL)
        {
            screen_bpp = info->vfmt->BitsPerPixel;
        }
    }
}

// If the video mode set in the configuration file is not available,
// try to choose a different mode.

static void I_AutoAdjustSettings(void)
{
    int old_screen_w, old_screen_h, old_screen_bpp;

    old_screen_w = screen_width;
    old_screen_h = screen_height;
    old_screen_bpp = screen_bpp;

    // Possibly adjust color depth.

    AutoAdjustColorDepth();

    // If we are running fullscreen, try to autoadjust to a valid fullscreen
    // mode.  If this is impossible, switch to windowed.

    if (fullscreen && !AutoAdjustFullscreen())
    {
        fullscreen = 0;
    }

    // If we are running windowed, pick a valid window size.

    if (!fullscreen)
    {
        AutoAdjustWindowed();
    }

    // Have the settings changed?  Show a message.

    if (screen_width != old_screen_w || screen_height != old_screen_h
     || screen_bpp != old_screen_bpp)
    {
        printf("I_InitGraphics: Auto-adjusted to %ix%ix%ibpp.\n",
               screen_width, screen_height, screen_bpp);

        printf("NOTE: Your video settings have been adjusted.  "
               "To disable this behavior,\n"
               "set autoadjust_video_settings to 0 in your "
               "configuration file.\n");
    }
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
    // @category video
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
    // @arg <bpp>
    //
    // Specify the color depth of the screen, in bits per pixel.
    //

    i = M_CheckParmWithArgs("-bpp", 1);

    if (i > 0)
    {
        screen_bpp = atoi(myargv[i + 1]);
    }

    // Because we love Eternity:

    //!
    // @category video
    //
    // Set the color depth of the screen to 32 bits per pixel.
    //

    if (M_CheckParm("-8in32"))
    {
        screen_bpp = 32;
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

    //!
    // @category video
    //
    // Disable vertical mouse movement.
    //

    if (M_CheckParm("-novert"))
    {
        novert = true;
    }

    //!
    // @category video
    //
    // Enable vertical mouse movement.
    //

    if (M_CheckParm("-nonovert"))
    {
        novert = false;
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

static void CreateCursors(void)
{
    static Uint8 empty_cursor_data = 0;

    // Save the default cursor so it can be recalled later

    cursors[1] = SDL_GetCursor();

    // Create an empty cursor

    cursors[0] = SDL_CreateCursor(&empty_cursor_data,
                                  &empty_cursor_data,
                                  1, 1, 0, 0);
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

static char *WindowBoxType(screen_mode_t *mode, int w, int h)
{
    if (mode->width != w && mode->height != h) 
    {
        return "Windowboxed";
    }
    else if (mode->width == w) 
    {
        return "Letterboxed";
    }
    else if (mode->height == h)
    {
        return "Pillarboxed";
    }
    else
    {
        return "...";
    }
}

static void SetVideoMode(screen_mode_t *mode, int w, int h)
{
    byte *doompal;
    int flags = 0;

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);

    // If we are already running and in a true color mode, we need
    // to free the screenbuffer surface before setting the new mode.

    if (screenbuffer != NULL && screen != screenbuffer)
    {
        SDL_FreeSurface(screenbuffer);
    }

    // Generate lookup tables before setting the video mode.

    if (mode != NULL && mode->InitMode != NULL)
    {
        mode->InitMode(doompal);
    }

    // Set the video mode.

    flags |= SDL_SWSURFACE | SDL_DOUBLEBUF;

    if (screen_bpp == 8)
    {
        flags |= SDL_HWPALETTE;
    }

    if (fullscreen)
    {
        flags |= SDL_FULLSCREEN;
    }
    else
    {
        // In windowed mode, the window can be resized while the game is
        // running.  This feature is disabled on OS X, as it adds an ugly
        // scroll handle to the corner of the screen.

#ifndef __MACOSX__
        flags |= SDL_RESIZABLE;
#endif
    }

    screen = SDL_SetVideoMode(w, h, screen_bpp, flags);

    if (screen == NULL)
    {
        I_Error("Error setting video mode %ix%ix%ibpp: %s\n",
                w, h, screen_bpp, SDL_GetError());
    }

    // Blank out the full screen area in case there is any junk in
    // the borders that won't otherwise be overwritten.

    SDL_FillRect(screen, NULL, 0);

    // If mode was not set, it must be set now that we know the
    // screen size.

    if (mode == NULL)
    {
        mode = I_FindScreenMode(screen->w, screen->h);

        if (mode == NULL)
        {
            I_Error("I_InitGraphics: Unable to find a screen mode small "
                    "enough for %ix%i", screen->w, screen->h);
        }

        // Generate lookup tables before setting the video mode.

        if (mode->InitMode != NULL)
        {
            mode->InitMode(doompal);
        }
    }

    // Create the screenbuffer surface; if we have a real 8-bit palettized
    // screen, then we can use the screen as the screenbuffer.

    if (screen->format->BitsPerPixel == 8)
    {
        screenbuffer = screen;
    }
    else
    {
        screenbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                            mode->width, mode->height, 8,
                                            0, 0, 0, 0);

        SDL_FillRect(screenbuffer, NULL, 0);
    }

    // Save screen mode.

    screen_mode = mode;
}

static void ApplyWindowResize(unsigned int w, unsigned int h)
{
    screen_mode_t *mode;

    // Find the biggest screen mode that will fall within these
    // dimensions, falling back to the smallest mode possible if
    // none is found.

    mode = I_FindScreenMode(w, h);

    if (mode == NULL)
    {
        mode = I_FindScreenMode(SCREENWIDTH, SCREENHEIGHT);
    }

    // Reset mode to resize window.

    printf("Resize to %ix%i\n", mode->width, mode->height);
    SetVideoMode(mode, mode->width, mode->height);

    // Save settings.

    screen_width = mode->width;
    screen_height = mode->height;
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
    SetWindowPositionVars();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error("Failed to initialize video: %s", SDL_GetError());
    }

    // Set up title and icon.  Windows cares about the ordering; this
    // has to be done before the call to SDL_SetVideoMode.

    I_InitWindowTitle();
    I_InitWindowIcon();

    // Warning to OS X users... though they might never see it :(
#ifdef __MACOSX__
    if (fullscreen)
    {
        printf("Some old versions of OS X might crash in fullscreen mode.\n"
               "If this happens to you, switch back to windowed mode.\n");
    }
#endif

    //
    // Enter into graphics mode.
    //
    // When in screensaver mode, run full screen and auto detect
    // screen dimensions (don't change video mode)
    //

    if (screensaver_mode)
    {
        SetVideoMode(NULL, 0, 0);
    }
    else
    {
        int w, h;

        if (autoadjust_video_settings)
        {
            I_AutoAdjustSettings();
        }

        w = screen_width;
        h = screen_height;

        screen_mode = I_FindScreenMode(w, h);

        if (screen_mode == NULL)
        {
            I_Error("I_InitGraphics: Unable to find a screen mode small "
                    "enough for %ix%i", w, h);
        }

        if (w != screen_mode->width || h != screen_mode->height)
        {
            printf("I_InitGraphics: %s (%ix%i within %ix%i)\n",
                   WindowBoxType(screen_mode, w, h),
                   screen_mode->width, screen_mode->height, w, h);
        }

        SetVideoMode(screen_mode, w, h);
    }

    // Start with a clear black screen
    // (screen will be flipped after we set the palette)

    SDL_FillRect(screenbuffer, NULL, 0);

    // Set the palette

    doompal = W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE);
    I_SetPalette(doompal);
    SDL_SetColors(screenbuffer, palette, 0, 256);

    CreateCursors();

    UpdateFocus();
    UpdateGrab();

    // On some systems, it takes a second or so for the screen to settle
    // after changing modes.  We include the option to add a delay when
    // setting the screen mode, so that the game doesn't start immediately
    // with the player unable to see anything.

    if (fullscreen && !screensaver_mode)
    {
        SDL_Delay(startup_delay);
    }

    // Check if we have a native surface we can use
    // If we have to lock the screen, draw to a buffer and copy
    // Likewise if the screen pitch is not the same as the width
    // If we have to multiply, drawing is done to a separate 320x200 buf

    native_surface = screen == screenbuffer
                  && !SDL_MUSTLOCK(screen)
                  && screen_mode == &mode_scale_1x
                  && screen->pitch == SCREENWIDTH
                  && aspect_ratio_correct;

    // If not, allocate a buffer and copy from that buffer to the
    // screen when we do an update

    if (native_surface)
    {
	I_VideoBuffer = (unsigned char *) screen->pixels;

        I_VideoBuffer += (screen->h - SCREENHEIGHT) / 2;
    }
    else
    {
	I_VideoBuffer = (unsigned char *) Z_Malloc (SCREENWIDTH * SCREENHEIGHT, 
                                                    PU_STATIC, NULL);
    }

    V_RestoreBuffer();

    // Clear the screen to black.

    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

    // We need SDL to give us translated versions of keys as well

    SDL_EnableUNICODE(1);

    // Repeat key presses - this is what Vanilla Doom does
    // Not sure about repeat rate - probably dependent on which DOS
    // driver is used.  This is good enough though.

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

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
    M_BindIntVariable("autoadjust_video_settings", &autoadjust_video_settings);
    M_BindIntVariable("fullscreen",                &fullscreen);
    M_BindIntVariable("aspect_ratio_correct",      &aspect_ratio_correct);
    M_BindIntVariable("startup_delay",             &startup_delay);
    M_BindIntVariable("screen_width",              &screen_width);
    M_BindIntVariable("screen_height",             &screen_height);
    M_BindIntVariable("screen_bpp",                &screen_bpp);
    M_BindIntVariable("grabmouse",                 &grabmouse);
    M_BindFloatVariable("mouse_acceleration",      &mouse_acceleration);
    M_BindIntVariable("mouse_threshold",           &mouse_threshold);
    M_BindStringVariable("video_driver",           &video_driver);
    M_BindStringVariable("window_position",        &window_position);
    M_BindIntVariable("usegamma",                  &usegamma);
    M_BindIntVariable("vanilla_keyboard_mapping",  &vanilla_keyboard_mapping);
    M_BindIntVariable("novert",                    &novert);
    M_BindIntVariable("png_screenshots",           &png_screenshots);

    // Windows Vista or later?  Set screen color depth to
    // 32 bits per pixel, as 8-bit palettized screen modes
    // don't work properly in recent versions.

#if defined(_WIN32) && !defined(_WIN32_WCE)
    {
        OSVERSIONINFOEX version_info;

        ZeroMemory(&version_info, sizeof(OSVERSIONINFOEX));
        version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        GetVersionEx((OSVERSIONINFO *) &version_info);

        if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT
         && version_info.dwMajorVersion >= 6)
        {
            screen_bpp = 32;
        }
    }
#endif

    // Disable fullscreen by default on OS X, as there is an SDL bug
    // where some old versions of OS X (<= Snow Leopard) crash.

#ifdef __MACOSX__
    fullscreen = 0;
    screen_width = 800;
    screen_height = 600;
#endif
}
