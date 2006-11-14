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


#include <SDL.h>
#include <ctype.h>
#include <math.h>

#include "chocolate_doom_icon.c"

#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_swap.h"
#include "s_sound.h"
#include "sounds.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Alternate screenheight for letterbox mode

#define LETTERBOX_SCREENHEIGHT 240

enum
{
    FULLSCREEN_OFF,
    FULLSCREEN_ON,
    FULLSCREEN_LETTERBOX,
};

extern void M_QuitDOOM();

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

// Automatically adjust video settings if the selected mode is 
// not a valid video mode.

int autoadjust_video_settings = 1;

// Run in full screen mode?  (int type for config code)

int fullscreen = FULLSCREEN_ON;

// Time to wait for the screen to settle on startup before starting the
// game (ms)

int startup_delay = 0;

// Grab the mouse? (int type for config code)

int grabmouse = true;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Blocky mode,
// replace each 320x200 pixel with screenmultiply*screenmultiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....

int screenmultiply = 1;

// disk image data and background overwritten by the disk to be
// restored by EndRead

static byte *disk_image = NULL;
static int disk_image_w, disk_image_h;
static byte *saved_background;
static boolean window_focused;

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

    if (fullscreen != FULLSCREEN_OFF)
        return true;

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

      case SDLK_EQUALS:
      case SDLK_KP_EQUALS:	return KEY_EQUALS;

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

      case SDLK_HOME: return KEY_HOME;
      case SDLK_INSERT: return KEY_INS;
      case SDLK_END: return KEY_END;
      case SDLK_PAGEUP: return KEY_PGUP;
      case SDLK_PAGEDOWN: return KEY_PGDN;
      case SDLK_KP_MULTIPLY: return KEYP_MULTIPLY;
      case SDLK_KP_PLUS: return KEYP_PLUS;
      case SDLK_KP_MINUS: return KEYP_MINUS;
      case SDLK_KP_DIVIDE: return KEYP_DIVIDE;

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
		event.data2 = sdlevent.key.keysym.unicode;
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

    if (grab && !currently_grabbed)
    {
        SDL_ShowCursor(0);
        SDL_WM_GrabInput(SDL_GRAB_ON);
    }

    if (!grab && currently_grabbed)
    {
        SDL_ShowCursor(1);
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    }

    currently_grabbed = grab;

}

// Update a small portion of the screen
//
// Does 2x stretching and buffer blitting if neccessary

static void BlitArea(int x1, int y1, int x2, int y2)
{
    int w = x2 - x1;
    int x_offset, y_offset;

    x_offset = ((screen->w / screenmultiply) - SCREENWIDTH) / 2;
    y_offset = ((screen->h / screenmultiply) - SCREENHEIGHT) / 2;
    
    // Need to byte-copy from buffer into the screen buffer

    if (screenmultiply == 1 && !native_surface)
    {
        byte *bufp, *screenp;
        int y;
        int pitch;

        if (SDL_LockSurface(screen) >= 0)
        {
            pitch = screen->pitch;
            bufp = screens[0] + y1 * SCREENWIDTH + x1;
            screenp = (byte *) screen->pixels + (y1 + y_offset) * pitch 
                                              + x1 + x_offset;
    
            for (y=y1; y<y2; ++y)
            {
                memcpy(screenp, bufp, w);
                screenp += pitch;
                bufp += SCREENWIDTH;
            }
    
            SDL_UnlockSurface(screen);
        }
    }

    // scales the screen size before blitting it

    if (screenmultiply == 2)
    {
        byte *bufp, *screenp, *screenp2;
        int x, y;
        int pitch;

        if (SDL_LockSurface(screen) >= 0)
        {
            pitch = screen->pitch * 2;
            bufp = screens[0] + y1 * SCREENWIDTH + x1;
            screenp = (byte *) screen->pixels 
                    + (y1 + y_offset) * pitch 
                    + (x1 + x_offset) * 2;
            screenp2 = screenp + screen->pitch;
    
            for (y=y1; y<y2; ++y)
            {
                byte *sp, *sp2, *bp;
                sp = screenp;
                sp2 = screenp2;
                bp = bufp;
    
                for (x=x1; x<x2; ++x)
                {
                    *sp++ = *bp;  *sp++ = *bp;
                    *sp2++ = *bp; *sp2++ = *bp;
                    ++bp;
                }
                screenp += pitch;
                screenp2 += pitch;
                bufp += SCREENWIDTH;
            }
    
            SDL_UnlockSurface(screen);
        }
    }

    if (screenmultiply == 3)
    {
        byte *bufp, *screenp, *screenp2, *screenp3;
        int x, y;
        int pitch;

        if (SDL_LockSurface(screen) >= 0)
        {
            pitch = screen->pitch * 3;
            bufp = screens[0] + y1 * SCREENWIDTH + x1;
            screenp = (byte *) screen->pixels 
                    + (y1 + y_offset) * pitch 
                    + (x1 + x_offset) * 3;
            screenp2 = screenp + screen->pitch;
            screenp3 = screenp2 + screen->pitch;
    
            for (y=y1; y<y2; ++y)
            {
                byte *sp, *sp2, *sp3, *bp;
                sp = screenp;
                sp2 = screenp2;
                sp3 = screenp3;
                bp = bufp;
    
                for (x=x1; x<x2; ++x)
                {
                    *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;
                    *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp;
                    *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp;
                    ++bp;
                }
                screenp += pitch;
                screenp2 += pitch;
                screenp3 += pitch;
                bufp += SCREENWIDTH;
            }
    
            SDL_UnlockSurface(screen);
        }
    }

    if (screenmultiply == 4)
    {
        byte *bufp, *screenp, *screenp2, *screenp3, *screenp4;
        int x, y;
        int pitch;

        if (SDL_LockSurface(screen) >= 0)
        {
            pitch = screen->pitch * 4;
            bufp = screens[0] + y1 * SCREENWIDTH + x1;
            screenp = (byte *) screen->pixels 
                    + (y1 + y_offset) * pitch 
                    + (x1 + x_offset) * 4;
            screenp2 = screenp + screen->pitch;
            screenp3 = screenp2 + screen->pitch;
            screenp4 = screenp3 + screen->pitch;
    
            for (y=y1; y<y2; ++y)
            {
                byte *sp, *sp2, *sp3, *sp4, *bp;
                sp = screenp;
                sp2 = screenp2;
                sp3 = screenp3;
                sp4 = screenp4;
                bp = bufp;
    
                for (x=x1; x<x2; ++x)
                {
                    *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;  *sp++ = *bp;
                    *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp; *sp2++ = *bp;
                    *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp; *sp3++ = *bp;
                    *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp; *sp4++ = *bp;
                    ++bp;
                }
                screenp += pitch;
                screenp2 += pitch;
                screenp3 += pitch;
                screenp4 += pitch;
                bufp += SCREENWIDTH;
            }
    
            SDL_UnlockSurface(screen);
        }
    }
}

static void UpdateRect(int x1, int y1, int x2, int y2)
{
    // Do stretching and blitting

    BlitArea(x1, y1, x2, y2);

    // Update the area

    SDL_UpdateRect(screen, 
                   x1 * screenmultiply, 
                   y1 * screenmultiply, 
                   (x2-x1) * screenmultiply, 
                   (y2-y1) * screenmultiply);
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
        palette_to_set = 0;
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

    palette_to_set = 1;
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

    surface = SDL_CreateRGBSurfaceFrom(chocolate_doom_data,
                                       chocolate_doom_w,
                                       chocolate_doom_h,
                                       24,
                                       chocolate_doom_w * 3,
                                       0xff << 0,
                                       0xff << 8,
                                       0xff << 16,
                                       0);

    SDL_WM_SetIcon(surface, NULL);
    SDL_FreeSurface(surface);
}

// Get window dimensions for the current settings

static void GetWindowDimensions(int *windowwidth, int *windowheight)
{
    *windowwidth = SCREENWIDTH * screenmultiply;

    if (fullscreen == FULLSCREEN_LETTERBOX)
        *windowheight = LETTERBOX_SCREENHEIGHT * screenmultiply;
    else
        *windowheight = SCREENHEIGHT * screenmultiply;
}

// Check if the screen mode for the current settings is in the list available
// Not all machines support running in 320x200/640x400 (only support 4:3)
// Some don't even support modes below 640x480.

static boolean CheckValidFSMode(void)
{
    SDL_Rect **modes;
    int i;
    int w, h;

    GetWindowDimensions(&w, &h);

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

    for (i=0; modes[i]; ++i)
    {
        if (w == modes[i]->w && h == modes[i]->h)
            return true;
    }

    // not found

    return false;
}

static void CheckCommandLine(void)
{
    // mouse grabbing

    if (M_CheckParm("-grabmouse"))
    {
        grabmouse = true;
    }
    else if (M_CheckParm("-nograbmouse"))
    {
        grabmouse = false;
    }

    // default to fullscreen mode, allow override with command line
    // nofullscreen because we love prboom

    if (M_CheckParm("-window") || M_CheckParm("-nofullscreen"))
    {
        fullscreen = FULLSCREEN_OFF;
    }
    else if (M_CheckParm("-fullscreen"))
    {
        fullscreen = FULLSCREEN_ON;
    }

    nomouse = M_CheckParm("-nomouse") > 0;

    // scale-by-2 mode
 
    if (M_CheckParm("-1"))
    {
        screenmultiply = 1;
    }
    else if (M_CheckParm("-2"))
    {
        screenmultiply = 2;
    }
    else if (M_CheckParm("-3"))
    {
        screenmultiply = 3;
    }
    else if (M_CheckParm("-4"))
    {
        screenmultiply = 4;
    }

    if (screenmultiply < 1)
        screenmultiply = 1;
    if (screenmultiply > 4)
        screenmultiply = 4;
}

static void AutoAdjustSettings(void)
{
    int oldw, oldh;
    int old_fullscreen, old_screenmultiply;

    GetWindowDimensions(&oldw, &oldh);
    old_screenmultiply = screenmultiply;
    old_fullscreen = fullscreen;

    if (!CheckValidFSMode() && screenmultiply == 1 
     && fullscreen == FULLSCREEN_ON)
    {
        // 320x200 is not valid.

        // Try turning on letterbox mode - avoid doubling up
        // the screen if possible

        fullscreen = FULLSCREEN_LETTERBOX;

        if (!CheckValidFSMode())
        {
            // That doesn't work. Change it back.

            fullscreen = FULLSCREEN_ON;
        }
    }

    if (!CheckValidFSMode() && screenmultiply == 1)
    {
        // Try doubling up the screen to 640x400

        screenmultiply = 2;
    }

    if (!CheckValidFSMode() && fullscreen == FULLSCREEN_ON)
    {
        // This is not a valid mode.  Try turning on letterbox mode

        fullscreen = FULLSCREEN_LETTERBOX;
    }

    if (old_fullscreen != fullscreen 
     || old_screenmultiply != screenmultiply)
    {
        printf("I_InitGraphics: %ix%i resolution is not supported "
               "on this machine. \n", oldw, oldh);
        printf("I_InitGraphics: Video settings adjusted to "
               "compensate:\n");
        
        if (fullscreen != old_fullscreen)
            printf("\tletterbox mode on (fullscreen=2)\n");
        if (screenmultiply != old_screenmultiply)
            printf("\tscreenmultiply=%i\n", screenmultiply);
        
        printf("NOTE: Your video settings have been adjusted.  "
               "To disable this behavior,\n"
               "set autoadjust_video_settings to 0 in your "
               "configuration file.\n");
    }
    
    if (!CheckValidFSMode())
    {
        printf("I_InitGraphics: WARNING: Unable to find a valid "
               "fullscreen video mode to run in.\n");
    }
}

// Check if we have been invoked as a screensaver by xscreensaver.

void I_CheckIsScreensaver(void)
{
    char *env;

    env = SDL_getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        screensaver_mode = true;
    }
}

// In screensaver mode, pick a screenmultiply value that fits
// inside the screen.  It is okay to do this because settings
// are not saved in screensaver mode.

static void FindScreensaverMultiply(void)
{
    int i;

    for (i=1; i<=4; ++i)
    {
        if (SCREENWIDTH * i <= screen->w
         && SCREENHEIGHT * i <= screen->h)
        {
            screenmultiply = i;
        }
    }
}

// Blank cursor so we don't see the mouse.  It is not okay to
// do SDL_ShowCursor(0) because this will hide the mouse in
// the configuration dialog.  Only show no mouse when over this
// window.

static void SetBlankCursor(void)
{
    Uint8 zero = zero;
    SDL_Cursor *cursor;

    cursor = SDL_CreateCursor(&zero, &zero, 1, 1, 0, 0);

    SDL_SetCursor(cursor);
}

void I_InitGraphics(void)
{
    SDL_Event dummy;
    int flags = 0;
    char *env;

    // Pass through the XSCREENSAVER_WINDOW environment variable to 
    // SDL_WINDOWID, to embed the SDL window into the Xscreensaver
    // window.

    env = SDL_getenv("XSCREENSAVER_WINDOW");

    if (env != NULL)
    {
        char winenv[30];
        int winid;

        sscanf(env, "0x%x", &winid);
        sprintf(winenv, "SDL_WINDOWID=%i", winid);

        SDL_putenv(winenv);
    }

#ifdef _WIN32

    // Allow -gdi as a shortcut for using the windib driver.

    if (M_CheckParm("-gdi") > 0)
    {
        SDL_putenv("SDL_VIDEODRIVER=windib");
    }

    // From the SDL 1.2.10 release notes: 
    //
    // > The "windib" video driver is the default now, to prevent 
    // > problems with certain laptops, 64-bit Windows, and Windows 
    // > Vista. 
    //
    // The hell with that.

    if (SDL_getenv("SDL_VIDEODRIVER") == NULL)
    {
        SDL_putenv("SDL_VIDEODRIVER=directx");
    }

#endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        I_Error("Failed to initialise video: %s", SDL_GetError());
    }

    // Check for command-line video-related parameters.

    CheckCommandLine();

    if (fullscreen && autoadjust_video_settings)
    {
        // Check that the fullscreen mode we are trying to use is valid;
        // if not, try to automatically select a more appropriate one.

        AutoAdjustSettings();
    }

    GetWindowDimensions(&windowwidth, &windowheight);

    flags |= SDL_SWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

    if (fullscreen != FULLSCREEN_OFF)
    {
        flags |= SDL_FULLSCREEN;
    }

    if (screensaver_mode)
    {
        windowwidth = 0;
        windowheight = 0;
    }

    screen = SDL_SetVideoMode(windowwidth, windowheight, 8, flags);

    if (screen == NULL)
    {
        I_Error("Error setting video mode: %s\n", SDL_GetError());
    }

    // In screensaver mode, screenmultiply as large as possible
    // and set a blank cursor.

    if (screensaver_mode)
    {
        FindScreensaverMultiply();
        SetBlankCursor();
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

    I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
    SDL_SetColors(screen, palette, 0, 256);

    // Setup title and icon

    I_SetWindowCaption();
    I_SetWindowIcon();

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

    native_surface = !SDL_MUSTLOCK(screen) 
                  && screenmultiply == 1 
                  && screen->pitch == SCREENWIDTH;

    // If not, allocate a buffer and copy from that buffer to the 
    // screen when we do an update

    if (native_surface)
    {
	screens[0] = (unsigned char *) (screen->pixels);

        if (fullscreen == FULLSCREEN_LETTERBOX)
        {
            screens[0] += ((LETTERBOX_SCREENHEIGHT - SCREENHEIGHT) * screen->pitch) / 2;
        }
    }
    else
    {
	screens[0] = (unsigned char *) Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
    }

    // "Loading from disk" icon

    LoadDiskImage();

    // Clear the screen to black.

    memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);

    // We need SDL to give us translated versions of keys as well

    SDL_EnableUNICODE(1);

    // Repeat key presses - this is what Vanilla Doom does
    // Not sure about repeat rate - probably dependent on which DOS
    // driver is used.  This is good enough though.

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // clear out any events waiting at the start and center the mouse
  
    while (SDL_PollEvent(&dummy));
    CenterMouse();

    initialised = true;
}

