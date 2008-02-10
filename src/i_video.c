// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------


#include "SDL.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "icon.c"

#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_scale.h"
#include "m_argv.h"
#include "s_sound.h"
#include "sounds.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

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
    &mode_squash_5x,
};

extern void M_QuitDOOM();

// SDL video driver name

char *video_driver = "";

static SDL_Surface *screen;

// palette

static SDL_Color palette[256];
static boolean palette_to_set;

static int windowwidth, windowheight;

// display has been set up?

static boolean initialised = false;

// disable mouse?

static boolean nomouse = false;
extern int usemouse;

// if true, screens[0] is screen->pixel

static boolean native_surface;

// Screen width and height, from configuration file.

int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;

// Automatically adjust video settings if the selected mode is 
// not a valid video mode.

int autoadjust_video_settings = 1;

// Run in full screen mode?  (int type for config code)

int fullscreen = true;

// Aspect ratio correction mode

int aspect_ratio_correct = true;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

int startup_delay = 0;

// Grab the mouse? (int type for config code)

int grabmouse = true;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// disk image data and background overwritten by the disk to be
// restored by EndRead

static byte *disk_image = NULL;
static int disk_image_w, disk_image_h;
static byte *saved_background;
static boolean window_focused;

// Empty mouse cursor

static SDL_Cursor *cursors[2];

// The screen mode and scale functions being used

static screen_mode_t *screen_mode;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;

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

static boolean MouseShouldBeGrabbed()
{
    // never grab the mouse when in screensaver mode
   
    if (screensaver_mode)
        return false;

    // if the window doesnt have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to 
    // see the mouse pointer)

    if (fullscreen)
        return true;

    // Don't grab the mouse if mouse input is disabled

    if (!usemouse || nomouse)
        return false;

    // Drone players don't need mouse focus

    if (drone)
        return false;

    // if we specify not to grab the mouse, never grab
 
    if (!grabmouse)
        return false;

    // when menu is active or game is paused, release the mouse 
 
    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback;
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
    // (not minimised)

    window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);

    // Should the screen be grabbed?

    screenvisible = (state & SDL_APPACTIVE) != 0;
}

static void LoadDiskImage(void)
{
    patch_t *disk;
    int y;
    char buf[20];

    SDL_VideoDriverName(buf, 15);

    if (!strcmp(buf, "Quartz"))
    {
        // MacOS Quartz gives us pageflipped graphics that screw up the 
        // display when we use the loading disk.  Disable it.
        // This is a gross hack.

        return;
    }

    if (M_CheckParm("-cdrom") > 0)
        disk = (patch_t *) W_CacheLumpName(DEH_String("STCDROM"), PU_STATIC);
    else
        disk = (patch_t *) W_CacheLumpName(DEH_String("STDISK"), PU_STATIC);

    V_DrawPatch(0, 0, 0, disk);
    disk_image_w = SHORT(disk->width);
    disk_image_h = SHORT(disk->height);

    disk_image = Z_Malloc(disk_image_w * disk_image_h, PU_STATIC, NULL);
    saved_background = Z_Malloc(disk_image_w * disk_image_h, PU_STATIC, NULL);

    for (y=0; y<disk_image_h; ++y) 
    {
        memcpy(disk_image + disk_image_w * y,
               screens[0] + SCREENWIDTH * y,
               disk_image_w);
        memset(screens[0] + SCREENWIDTH * y, 0, disk_image_w);
    }

    Z_Free(disk);
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
      case SDLK_LMETA:
      case SDLK_RALT:
      case SDLK_RMETA:
        return KEY_RALT;

      case SDLK_CAPSLOCK: return KEY_CAPSLOCK;
      case SDLK_SCROLLOCK: return KEY_SCRLCK;

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

      default:
        return tolower(sym->sym);
    }
}

void I_ShutdownGraphics(void)
{
    if (initialised)
    {
        SDL_ShowCursor(1);
        SDL_WM_GrabInput(SDL_GRAB_OFF);

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    
        initialised = false;
    }
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

static int MouseButtonState(void)
{
    Uint8 state = SDL_GetMouseState(NULL, NULL);
    int result = 0;

    // Note: button "0" is left, button "1" is right,
    // button "2" is middle for Doom.  This is different
    // to how SDL sees things.

    if (state & SDL_BUTTON(1))
        result |= 1;
    if (state & SDL_BUTTON(3))
        result |= 2;
    if (state & SDL_BUTTON(2))
        result |= 4;

    return result;
}

static int AccelerateMouse(int val)
{
    if (val < 0)
        return -AccelerateMouse(-val);

    if (val > mouse_threshold)
    {
        return (val - mouse_threshold) * mouse_acceleration + mouse_threshold;
    }
    else
    {
        return val;
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

        // process event
        
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                event.type = ev_keydown;
                event.data1 = TranslateKey(&sdlevent.key.keysym);

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
                    event.data2 = event.data1;
                }
                else
                {
                    event.data2 = sdlevent.key.keysym.unicode;
                }

                D_PostEvent(&event);
                break;

            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = TranslateKey(&sdlevent.key.keysym);
                D_PostEvent(&event);
                break;

                /*
            case SDL_MOUSEMOTION:
                event.type = ev_mouse;
                event.data1 = MouseButtonState();
                event.data2 = AccelerateMouse(sdlevent.motion.xrel);
                event.data3 = -AccelerateMouse(sdlevent.motion.yrel);
                D_PostEvent(&event);
                break;
                */

            case SDL_MOUSEBUTTONDOWN:
		if (usemouse && !nomouse)
		{
                    event.type = ev_mouse;
                    event.data1 = MouseButtonState();
                    event.data2 = event.data3 = 0;
                    D_PostEvent(&event);
		}
                break;

            case SDL_MOUSEBUTTONUP:
		if (usemouse && !nomouse)
		{
                    event.type = ev_mouse;
                    event.data1 = MouseButtonState();
                    event.data2 = event.data3 = 0;
                    D_PostEvent(&event);
		}
                break;

            case SDL_QUIT:
                // bring up the "quit doom?" prompt
                S_StartSound(NULL,sfx_swtchn);
                M_QuitDOOM(0);
                break;

            case SDL_ACTIVEEVENT:
                // need to update our focus state
                UpdateFocus();
                break;

            case SDL_VIDEOEXPOSE:
                palette_to_set = true;
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
        ev.data1 = MouseButtonState();
        ev.data2 = AccelerateMouse(x);
        ev.data3 = -AccelerateMouse(y);
        
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

        SDL_SetCursor(cursors[0]);
    }
    else if (grab && !currently_grabbed)
    {
        SDL_SetCursor(cursors[0]);
        SDL_WM_GrabInput(SDL_GRAB_ON);
    }
    else if (!grab && currently_grabbed)
    {
        SDL_SetCursor(cursors[1]);
        SDL_WM_GrabInput(SDL_GRAB_OFF);
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

    x_offset = (screen->w - screen_mode->width) / 2;
    y_offset = (screen->h - screen_mode->height) / 2;

    if (SDL_LockSurface(screen) >= 0)
    {
        I_InitScale(screens[0], 
                    (byte *) screen->pixels + (y_offset * screen->pitch)
                                            + x_offset, 
                    screen->pitch);
        result = screen_mode->DrawScreen(x1, y1, x2, y2);
      	SDL_UnlockSurface(screen);
    }
    else
    {
        result = false;
    }

    return result;
}

static void UpdateRect(int x1, int y1, int x2, int y2)
{
    int x1_scaled, x2_scaled, y1_scaled, y2_scaled;

    // Do stretching and blitting

    if (BlitArea(x1, y1, x2, y2))
    {
        // Update the area

        x1_scaled = (x1 * screen_mode->width) / SCREENWIDTH;
        y1_scaled = (y1 * screen_mode->height) / SCREENHEIGHT;
        x2_scaled = (x2 * screen_mode->width) / SCREENWIDTH;
        y2_scaled = (y2 * screen_mode->height) / SCREENHEIGHT;

        SDL_UpdateRect(screen,
                       x1_scaled, y1_scaled,
                       x2_scaled - x1_scaled,
                       y2_scaled - y1_scaled);
    }
}

void I_BeginRead(void)
{
    int y;

    if (!initialised || disk_image == NULL)
        return;

    // save background and copy the disk image in

    for (y=0; y<disk_image_h; ++y)
    {
        byte *screenloc = 
               screens[0] 
                 + (SCREENHEIGHT - 1 - disk_image_h + y) * SCREENWIDTH
                 + (SCREENWIDTH - 1 - disk_image_w);

        memcpy(saved_background + y * disk_image_w,
               screenloc,
               disk_image_w);
        memcpy(screenloc, disk_image + y * disk_image_w, disk_image_w);
    }

    UpdateRect(SCREENWIDTH - disk_image_w, SCREENHEIGHT - disk_image_h,
               SCREENWIDTH, SCREENHEIGHT);
}

void I_EndRead(void)
{
    int y;

    if (!initialised || disk_image == NULL)
        return;

    // save background and copy the disk image in

    for (y=0; y<disk_image_h; ++y)
    {
        byte *screenloc = 
               screens[0] 
                 + (SCREENHEIGHT - 1 - disk_image_h + y) * SCREENWIDTH
                 + (SCREENWIDTH - 1 - disk_image_w);

        memcpy(screenloc, saved_background + y * disk_image_w, disk_image_w);
    }

    UpdateRect(SCREENWIDTH - disk_image_w, SCREENHEIGHT - disk_image_h,
               SCREENWIDTH, SCREENHEIGHT);
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    if (!initialised)
        return;

    if (noblit)
        return;
    
    UpdateGrab();

    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away 
    // while fullscreen.

    if (!(SDL_GetAppState() & SDL_APPACTIVE))
        return;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=4)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*4 ; i+=4)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    
    }

    // draw to screen

    BlitArea(0, 0, SCREENWIDTH, SCREENHEIGHT);
    
    // If we have a palette to set, the act of setting the palette
    // updates the screen

    if (palette_to_set)
    {
        SDL_SetColors(screen, palette, 0, 256);
        palette_to_set = false;
    }
    else
    {
        SDL_Flip(screen);
    }
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte *doompalette)
{
    int i;

    for (i=0; i<256; ++i) 
    {
        palette[i].r = gammatable[usegamma][*doompalette++];
        palette[i].g = gammatable[usegamma][*doompalette++];
        palette[i].b = gammatable[usegamma][*doompalette++];
    }

    palette_to_set = true;
}

// 
// Set the window caption
//

void I_SetWindowCaption(void)
{
    char *buf;

    buf = Z_Malloc(strlen(gamedescription) + strlen(PACKAGE_STRING) + 10, 
                   PU_STATIC, NULL);
    sprintf(buf, "%s - %s", gamedescription, PACKAGE_STRING);

    SDL_WM_SetCaption(buf, NULL);

    Z_Free(buf);
}

// Set the application icon

void I_SetWindowIcon(void)
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom(icon_data,
                                       icon_w,
                                       icon_h,
                                       24,
                                       icon_w * 3,
                                       0xff << 0,
                                       0xff << 8,
                                       0xff << 16,
                                       0);

    SDL_WM_SetIcon(surface, NULL);
    SDL_FreeSurface(surface);
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

// If the video mode set in the configuration file is not available,
// try to choose a different mode.

static void I_AutoAdjustSettings(void)
{
    if (fullscreen)
    {
        SDL_Rect **modes;
        SDL_Rect *best_mode;
        screen_mode_t *screen_mode;
        int target_pixels, diff, best_diff;
        int i;

        modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

        // Find the best mode that matches the mode specified in the
        // configuration file

        best_mode = NULL;
        best_diff = INT_MAX;
        target_pixels = screen_width * screen_height;

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
                return;
            }

            // Is this mode better than the current mode?

            diff = (screen_width - modes[i]->w) 
                     * (screen_width - modes[i]->w)
                 + (screen_height - modes[i]->h)
                     * (screen_height - modes[i]->h);

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

            I_Error("Unable to find any valid video mode at all!");
        }

        printf("I_InitGraphics: %ix%i mode not supported on this machine.\n",
               screen_width, screen_height);

        screen_width = best_mode->w;
        screen_height = best_mode->h;

    }
    else
    {
        screen_mode_t *best_mode;

        //
        // Windowed mode.
        //
        // Find a screen_mode_t to fit within the current settings
        //

        best_mode = I_FindScreenMode(screen_width, screen_height);

        if (best_mode == NULL) 
        {
            // Nothing fits within the current settings.
            // Pick the closest to 320x200 possible.

            best_mode = I_FindScreenMode(SCREENWIDTH, SCREENHEIGHT_4_3);
        }

        // Do we have the exact mode already?

        if (best_mode->width == screen_width 
         && best_mode->height == screen_height)
        {
            return;
        }

        printf("I_InitGraphics: Cannot run at specified mode: %ix%i\n",
               screen_width, screen_height);

        screen_width = best_mode->width;
        screen_height = best_mode->height;
    }

    printf("I_InitGraphics: Auto-adjusted to %ix%i.\n",
           screen_width, screen_height);

    printf("NOTE: Your video settings have been adjusted.  "
           "To disable this behavior,\n"
           "set autoadjust_video_settings to 0 in your "
           "configuration file.\n");
}

// Set video size to a particular scale factor (1x, 2x, 3x, etc.)

static void SetScaleFactor(int factor)
{
    if (fullscreen)
    {
        // In fullscreen, find a mode that will provide this scale factor

        SDL_Rect **modes;
        SDL_Rect *best_mode;
        screen_mode_t *scrmode;
        int best_num_pixels, num_pixels;
        int i;

        modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

        best_mode = NULL;
        best_num_pixels = INT_MAX;

        for (i=0; modes[i] != NULL; ++i)
        {
            // What screen_mode_t will this use?

            scrmode = I_FindScreenMode(modes[i]->w, modes[i]->h);

            if (scrmode == NULL)
            {
                continue;
            }

            // Only choose modes that fit the requested scale factor.
            //
            // Note that this allows 320x240 as valid for 1x scale, as 
            // 240/200 is rounded down to 1 by integer division.

            if ((scrmode->width / SCREENWIDTH) != factor
             || (scrmode->height / SCREENHEIGHT) != factor)
            {
                continue;
            }

            // Is this a better mode than what we currently have?

            num_pixels = modes[i]->w * modes[i]->h;

            if (num_pixels < best_num_pixels)
            {
                best_num_pixels = num_pixels;
                best_mode = modes[i];
            }
        }

        if (best_mode == NULL)
        {
            I_Error("No fullscreen graphics mode available to support "
                    "%ix scale factor!", factor);
        }

        screen_width = best_mode->w;
        screen_height = best_mode->h;
    }
    else
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
}

static void CheckCommandLine(void)
{
    int i;

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

    i = M_CheckParm("-width");

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

    i = M_CheckParm("-height");

    if (i > 0)
    {
        screen_height = atoi(myargv[i + 1]);
    }

    //!
    // @category video
    // @arg <WxY>
    //
    // Specify the screen mode (when running fullscreen) or the window
    // dimensions (when running in windowed mode).

    i = M_CheckParm("-geometry");

    if (i > 0)
    {
        int w, h;

        if (sscanf(myargv[i + 1], "%ix%i", &w, &h) == 2)
        {
            screen_width = w;
            screen_height = h;
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

        env_string = malloc(strlen(video_driver) + 30);
        sprintf(env_string, "SDL_VIDEODRIVER=%s", video_driver);
        putenv(env_string);
        free(env_string);
    }

#ifdef _WIN32

    // Allow -gdi as a shortcut for using the windib driver.

    //!
    // @category video 
    // @platform windows
    //
    // Use the Windows GDI driver instead of DirectX.
    //

    if (M_CheckParm("-gdi") > 0)
    {
        putenv("SDL_VIDEODRIVER=windib");
    }

    // From the SDL 1.2.10 release notes: 
    //
    // > The "windib" video driver is the default now, to prevent 
    // > problems with certain laptops, 64-bit Windows, and Windows 
    // > Vista. 
    //
    // The hell with that.

    if (getenv("SDL_VIDEODRIVER") == NULL)
    {
        putenv("SDL_VIDEODRIVER=directx");
    }
#endif
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

void I_InitGraphics(void)
{
    SDL_Event dummy;
    byte *doompal;
    int flags = 0;
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
        sprintf(winenv, "SDL_WINDOWID=%i", winid);

        putenv(winenv);
    }

    SetSDLVideoDriver();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error("Failed to initialise video: %s", SDL_GetError());
    }

    // Check for command-line video-related parameters.

    CheckCommandLine();

    doompal = W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE);

    if (screensaver_mode)
    {
        windowwidth = 0;
        windowheight = 0;
    }
    else
    {
        if (autoadjust_video_settings)
        {
            I_AutoAdjustSettings();
        }

        windowwidth = screen_width;
        windowheight = screen_height;

        screen_mode = I_FindScreenMode(windowwidth, windowheight);

        if (screen_mode == NULL)
        {
            I_Error("I_InitGraphics: Unable to find a screen mode small "
                    "enough for %ix%i", windowwidth, windowheight);
        }

        if (windowwidth != screen_mode->width
         || windowheight != screen_mode->height)
        {
            printf("I_InitGraphics: %s (%ix%i within %ix%i)\n",
                   WindowBoxType(screen_mode, windowwidth, windowheight),
                   screen_mode->width, screen_mode->height,
                   windowwidth, windowheight);
        }

        // Generate lookup tables before setting the video mode.

        if (screen_mode->InitMode != NULL)
        {
            screen_mode->InitMode(doompal);
        }
    }

    // Set the video mode.

    flags |= SDL_SWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

    if (fullscreen)
    {
        flags |= SDL_FULLSCREEN;
    }

    screen = SDL_SetVideoMode(windowwidth, windowheight, 8, flags);

    if (screen == NULL)
    {
        I_Error("Error setting video mode: %s\n", SDL_GetError());
    }

    // Start with a clear black screen
    // (screen will be flipped after we set the palette)

    if (SDL_LockSurface(screen) >= 0)
    {
        byte *screenpixels;
        int y;

        screenpixels = (byte *) screen->pixels;

        for (y=0; y<screen->h; ++y)
            memset(screenpixels + screen->pitch * y, 0, screen->w);

        SDL_UnlockSurface(screen);
    }

    // Set the palette

    I_SetPalette(doompal);
    SDL_SetColors(screen, palette, 0, 256);

    // Setup title and icon

    I_SetWindowCaption();
    I_SetWindowIcon();

    CreateCursors();

    UpdateFocus();
    UpdateGrab();

    // In screensaver mode, now find a screen_mode to use.

    if (screensaver_mode)
    {
        screen_mode = I_FindScreenMode(screen->w, screen->h);

        if (screen_mode == NULL)
        {
            I_Error("I_InitGraphics: Unable to find a screen mode small "
                    "enough for %ix%i", screen->w, screen->h);
        }

        // Generate lookup tables before setting the video mode.

        if (screen_mode->InitMode != NULL)
        {
            screen_mode->InitMode(doompal);
        }
    }
    
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

    native_surface = !SDL_MUSTLOCK(screen) 
                  && screen_mode == &mode_scale_1x
                  && screen->pitch == SCREENWIDTH
                  && aspect_ratio_correct;

    // If not, allocate a buffer and copy from that buffer to the 
    // screen when we do an update

    if (native_surface)
    {
	screens[0] = (unsigned char *) screen->pixels;

        screens[0] += (screen->h - SCREENHEIGHT) / 2;
    }
    else
    {
	screens[0] = (unsigned char *) Z_Malloc (SCREENWIDTH * SCREENHEIGHT, 
                                                 PU_STATIC, NULL);

        // Clear the screen to black.

        memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);
    }

    // "Loading from disk" icon

    LoadDiskImage();

    // We need SDL to give us translated versions of keys as well

    SDL_EnableUNICODE(1);

    // Repeat key presses - this is what Vanilla Doom does
    // Not sure about repeat rate - probably dependent on which DOS
    // driver is used.  This is good enough though.

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // clear out any events waiting at the start and center the mouse
  
    while (SDL_PollEvent(&dummy));

    if (usemouse && !nomouse && (fullscreen || grabmouse))
    {
        CenterMouse();
    }

    initialised = true;
}

