// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_video.c 283 2006-01-12 01:34:48Z fraggle $
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
// $Log$
// Revision 1.43  2006/01/12 01:34:48  fraggle
// Combine mouse motion for tics into single events.
//
// Revision 1.42  2006/01/02 20:27:45  fraggle
// Clear the screen AFTER initialising the loading disk buffer, so that
// bits of loading disk are not visible on the initial screen melt.
//
// Revision 1.41  2006/01/02 00:06:30  fraggle
// Make functions static.  Remove unused variable.
//
// Revision 1.40  2005/10/17 19:46:22  fraggle
// Guard against multiple video shutdowns better.  Fix crash due to improper
// screen clear at startup.
//
// Revision 1.39  2005/10/16 20:55:50  fraggle
// Fix the '-cdrom' command-line option.
//
// Revision 1.38  2005/10/15 22:50:57  fraggle
// Fix pink icon on startup
//
// Revision 1.37  2005/10/15 15:59:14  fraggle
// Map mouse buttons correctly.
//
// Revision 1.36  2005/10/15 15:45:03  fraggle
// Check the return code from SDL_LockSurface to ensure a surface has been
// properly locked.  Fixes crash when switching applications while running
// fullscreen.
//
// Revision 1.35  2005/10/02 03:16:29  fraggle
// ENDOOM support using text mode emulation
//
// Revision 1.34  2005/10/02 03:03:40  fraggle
// Make sure loading disk is only shown if the display is initialised
//
// Revision 1.33  2005/09/27 22:33:42  fraggle
// Always use SDL_Flip to update the screen.  Fixes problems in Windows when
// running fullscreen, introduced by fixes to the disk icon code.
//
// Revision 1.32  2005/09/26 21:44:30  fraggle
// Fix melting crap on startup - oops
//
// Revision 1.31  2005/09/25 00:31:32  fraggle
// Fix disk icon appearing before palette is set (pink disk!)
// Cleanup and commenting
//
// Revision 1.30  2005/09/24 23:44:49  fraggle
// Enforce sane screenmultiply values
//
// Revision 1.29  2005/09/24 23:41:07  fraggle
// Fix "loading" icon for all video modes
//
// Revision 1.28  2005/09/24 22:04:03  fraggle
// Add application icon to running program
//
// Revision 1.27  2005/09/17 20:50:46  fraggle
// Mouse acceleration code to emulate old DOS drivers
//
// Revision 1.26  2005/09/14 21:55:47  fraggle
// Lock surfaces properly when we have to (fixes crash under Windows 98)
//
// Revision 1.25  2005/09/11 20:25:56  fraggle
// Second configuration file to allow chocolate doom-specific settings.
// Adjust some existing command line logic (for graphics settings and
// novert) to adjust for this.
//
// Revision 1.24  2005/09/08 22:05:17  fraggle
// Allow alt-tab away while running fullscreen
//
// Revision 1.23  2005/09/07 20:44:23  fraggle
// Fix up names of functions
// Make the quit button work (pops up the "quit doom?" prompt).
// Fix focus detection to release the mouse and ignore mouse events
// when window is not focused.
//
// Revision 1.22  2005/09/04 23:18:30  fraggle
// Remove dead code.  Cope with the screen not having width == pitch.  Lock
// the SDL screen surface properly. Rewrite 2x scaling code.
//
// Revision 1.21  2005/09/01 00:01:36  fraggle
// -nograbmouse option
//
// Revision 1.20  2005/08/31 23:58:28  fraggle
// smarter mouse grabbing for windowed mode
//
// Revision 1.19  2005/08/31 21:35:42  fraggle
// Display the game name in the title bar.  Move game start code to later
// in initialisation because of the IWAD detection changes.
//
// Revision 1.18  2005/08/10 08:45:35  fraggle
// Remove "if (french)" stuff, FRENCH define, detect french wad automatically
//
// Revision 1.17  2005/08/07 20:01:00  fraggle
// Clear the screen on startup
//
// Revision 1.16  2005/08/07 03:09:33  fraggle
// Fix gamma correction
//
// Revision 1.15  2005/08/07 02:59:23  fraggle
// Clear disk image when loading at startup
//
// Revision 1.14  2005/08/06 17:30:30  fraggle
// Only change palette on screen updates
//
// Revision 1.13  2005/08/04 22:23:07  fraggle
// Use zone memory function.  Add command line options
//
// Revision 1.12  2005/08/04 19:54:56  fraggle
// Use keysym value rather than unicode value (fixes problems with shift
// key)
//
// Revision 1.11  2005/08/04 18:42:15  fraggle
// Silence compiler warnings
//
// Revision 1.10  2005/08/04 01:13:46  fraggle
// Loading disk
//
// Revision 1.9  2005/08/03 22:19:52  fraggle
// Set some flags to fix palette and improve performance
//
// Revision 1.8  2005/08/03 21:58:02  fraggle
// Working scale*2
//
// Revision 1.7  2005/07/25 20:50:55  fraggle
// mouse
//
// Revision 1.6  2005/07/24 02:14:04  fraggle
// Move to SDL for graphics.
// Translate key scancodes to correct internal format when reading
// settings from config file - backwards compatible with config files
// for original exes
//
// Revision 1.5  2005/07/23 21:32:47  fraggle
// Add missing errno.h, fix crash on startup when no IWAD present
//
// Revision 1.4  2005/07/23 19:17:11  fraggle
// Use ANSI-standard limit constants.  Remove LINUX define.
//
// Revision 1.3  2005/07/23 17:27:04  fraggle
// Stop crash on shutdown
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:19:58  fraggle
// Initial import
//
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_video.c 283 2006-01-12 01:34:48Z fraggle $";

#include <SDL.h>
#include <ctype.h>
#include <math.h>

#include "chocolate_doom_icon.c"

#include "config.h"
#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_swap.h"
#include "s_sound.h"
#include "sounds.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

extern void M_QuitDOOM();

static SDL_Surface *screen;

// palette
static SDL_Color palette[256];
static boolean palette_to_set;

static int windowwidth, windowheight;

// display has been set up?

static boolean initialised = false;

// if true, screens[0] is screen->pixel

static boolean native_surface;

// Run in full screen mode?  (int type for config code)
int fullscreen = true;

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

// mouse acceleration
// We accelerate the mouse by raising the mouse movement values to
// the power of this value, to simulate the acceleration in DOS
// mouse drivers
//
// TODO: See what is a sensible default value for this

float mouse_acceleration = 1.5;

static boolean MouseShouldBeGrabbed()
{
    // if the window doesnt have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to 
    // see the mouse pointer)

    if (fullscreen)
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

    if (M_CheckParm("-cdrom") > 0)
        disk = (patch_t *) W_CacheLumpName("STCDROM", PU_STATIC);
    else
        disk = (patch_t *) W_CacheLumpName("STDISK", PU_STATIC);

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

int TranslateKey(SDL_keysym *sym)
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

    return (int) pow(val, mouse_acceleration) / 5;
}

void I_GetEvent(void)
{
    SDL_Event sdlevent;
    event_t event;

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
                event.type = ev_mouse;
                event.data1 = MouseButtonState();
                event.data2 = event.data3 = 0;
                D_PostEvent(&event);
                break;
            case SDL_MOUSEBUTTONUP:
                event.type = ev_mouse;
                event.data1 = MouseButtonState();
                event.data2 = event.data3 = 0;
                D_PostEvent(&event);
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
}

//
// I_StartTic
//
void I_StartTic (void)
{
    I_GetEvent();
    I_ReadMouse();
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

    if (screenmultiply == 1 && !native_surface)
    {
        byte *bufp, *screenp;
        int y;
        int pitch;

        if (SDL_LockSurface(screen) >= 0)
        {
            pitch = screen->pitch;
            bufp = screens[0] + y1 * SCREENWIDTH + x1;
            screenp = (byte *) screen->pixels + y1 * pitch + x1;
    
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
            screenp = (byte *) screen->pixels + (y1 * pitch) +  (x1 * 2);
            screenp2 = screenp + screen->pitch;
    
            for (y=y1; y<y2; ++y)
            {
                byte *sp, *sp2, *bp;
                sp = screenp;
                sp2 = screenp2;
                bp = bufp;
    
                for (x=x1; x<x2; ++x)
                {
                    *sp2++ = *bp;
                    *sp2++ = *bp;
                    *sp++ = *bp;
                    *sp++ = *bp++;
                }
                screenp += pitch;
                screenp2 += pitch;
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

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
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

void I_InitGraphics(void)
{
    SDL_Event dummy;
    int flags = 0;

    SDL_Init(SDL_INIT_VIDEO);

    flags |= SDL_SWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

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
        fullscreen = false;
    }
    else if (M_CheckParm("-fullscreen"))
    {
        fullscreen = true;
    }

    if (fullscreen)
    {
        flags |= SDL_FULLSCREEN;
    }

    // scale-by-2 mode
 
    if (M_CheckParm("-1"))
    {
        screenmultiply = 1;
    }
    else if (M_CheckParm("-2"))
    {
        screenmultiply = 2;
    }

    if (screenmultiply < 1)
        screenmultiply = 1;
    if (screenmultiply > 2)
        screenmultiply = 2;

    windowwidth = SCREENWIDTH * screenmultiply;
    windowheight = SCREENHEIGHT * screenmultiply;

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

    I_SetPalette (W_CacheLumpName ("PLAYPAL",PU_CACHE));
    SDL_SetColors(screen, palette, 0, 256);

    // Setup title and icon

    I_SetWindowCaption();
    I_SetWindowIcon();

    UpdateFocus();
    UpdateGrab();

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
	screens[0] = (unsigned char *) (screen->pixels);
    else
	screens[0] = (unsigned char *) Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    // Loading from disk icon

    LoadDiskImage();

    memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);

    // We need SDL to give us translated versions of keys as well

    SDL_EnableUNICODE(1);

    // clear out any events waiting at the start
  
    while (SDL_PollEvent(&dummy));

    initialised = true;
}

