// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
rcsid[] = "$Id$";

#include <ctype.h>
#include <SDL.h>

#include "config.h"
#include "w_wad.h"
#include "z_zone.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "m_swap.h"
#include "d_main.h"

#include "doomdef.h"

static SDL_Surface *screen;

#define POINTER_WARP_COUNTDOWN	1

// palette
static SDL_Color palette[256];
static boolean palette_to_set;

static int windowwidth, windowheight;

// if true, screens[0] is screen->pixel

static boolean native_surface;

boolean fullscreen = 1;
boolean grabmouse = 1;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;

// disk image data and background overwritten by the disk to be
// restored by EndRead

static byte *disk_image = NULL;
static int disk_image_w, disk_image_h;
static byte *saved_background;

static boolean mouse_should_be_grabbed()
{
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

void I_BeginRead(void)
{
    int y;

    if (disk_image == NULL)
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

    SDL_UpdateRect(screen, 
                   screen->w - disk_image_w, screen->h - disk_image_h, 
                   disk_image_w, disk_image_h);
}

void I_EndRead(void)
{
    int y;

    if (disk_image == NULL)
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

    SDL_UpdateRect(screen, 
                   screen->w - disk_image_w, screen->h - disk_image_h, 
                   disk_image_w, disk_image_h);
}

static void LoadDiskImage(void)
{
    patch_t *disk;
    int y;

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

int xlatekey(SDL_keysym *sym)
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
    SDL_ShowCursor(1);
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

static int mousebuttonstate(void)
{
    Uint8 state = SDL_GetMouseState(NULL, NULL);
    int result = 0;

    if (state & SDL_BUTTON(1))
        result |= 1;
    if (state & SDL_BUTTON(2))
        result |= 2;
    if (state & SDL_BUTTON(3))
        result |= 4;

    return result;
}

boolean		mousemoved = false;

void I_GetEvent(void)
{
    SDL_Event sdlevent;
    event_t event;

    // put event-grabbing stuff in here
    
    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                event.type = ev_keydown;
                event.data1 = xlatekey(&sdlevent.key.keysym);
		event.data2 = sdlevent.key.keysym.unicode;
                D_PostEvent(&event);
                break;
            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = xlatekey(&sdlevent.key.keysym);
                D_PostEvent(&event);
                break;
            case SDL_MOUSEMOTION:
                event.type = ev_mouse;
                event.data1 = mousebuttonstate();
                event.data2 = sdlevent.motion.xrel * 8;
                event.data3 = -sdlevent.motion.yrel * 8;
                D_PostEvent(&event);
                break;
            case SDL_MOUSEBUTTONDOWN:
                event.type = ev_mouse;
                event.data1 = mousebuttonstate();
                event.data2 = event.data3 = 0;
                D_PostEvent(&event);
                break;
            case SDL_MOUSEBUTTONUP:
                event.type = ev_mouse;
                event.data1 = mousebuttonstate();
                event.data2 = event.data3 = 0;
                D_PostEvent(&event);
                break;
        }
    }
}
//
// I_StartTic
//
void I_StartTic (void)
{
    I_GetEvent();
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

void UpdateGrab(void)
{
    static boolean currently_grabbed = false;
    boolean grab;

    grab = mouse_should_be_grabbed();

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

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    UpdateGrab();

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

    if (multiply == 1 && !native_surface)
    {
        byte *bufp, *screenp;
        int y;
        int pitch;

        SDL_LockSurface(screen);

        bufp = screens[0];
        screenp = (byte *) screen->pixels;
        pitch = screen->pitch;

        for (y=0; y<SCREENHEIGHT; ++y)
        {
            memcpy(screenp, bufp, SCREENWIDTH);
            screenp += pitch;
            bufp += SCREENWIDTH;
        }

        SDL_UnlockSurface(screen);
    }

    // scales the screen size before blitting it

    if (multiply == 2)
    {
        byte *bufp, *screenp, *screenp2;
        int x, y;
        int pitch;

        SDL_LockSurface(screen);

        bufp = screens[0];
        screenp = (byte *) screen->pixels;
        screenp2 = ((byte *) screen->pixels) + screen->pitch;
        pitch = screen->pitch * 2;

        for (y=0; y<SCREENHEIGHT; ++y)
        {
            byte *sp, *sp2, *bp;
            sp = screenp;
            sp2 = screenp2;
            bp = bufp;

            for (x=0; x<SCREENWIDTH; ++x)
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

    if (native_surface)
        SDL_UnlockSurface(screen);

    // draw to screen
    
    if (palette_to_set)
    {
        SDL_SetColors(screen, palette, 0, 256);
        palette_to_set = 0;
    }
    else
    {
        SDL_Flip(screen);
    }

    if (native_surface)
        SDL_LockSurface(screen);
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

static void SetCaption(void)
{
    char *buf;

    buf = Z_Malloc(strlen(gamedescription) + strlen(PACKAGE_STRING) + 10, 
                   PU_STATIC, NULL);
    sprintf(buf, "%s - %s", gamedescription, PACKAGE_STRING);

    SDL_WM_SetCaption(buf, NULL);

    Z_Free(buf);
}


void I_InitGraphics(void)
{
    SDL_Event dummy;
    int flags = 0;

    SDL_Init(SDL_INIT_VIDEO);

    flags |= SDL_SWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

    // mouse grabbing, defaults to on

    grabmouse = !M_CheckParm("-nograbmouse");

    // default to fullscreen mode, allow override with command line
    // nofullscreen because we love prboom

    fullscreen = !M_CheckParm("-window") && !M_CheckParm("-nofullscreen");

    if (fullscreen)
    {
        flags |= SDL_FULLSCREEN;
    }

    // scale-by-2 mode
 
    if (M_CheckParm("-2"))
    {
        multiply = 2;
    }

    windowwidth = SCREENWIDTH * multiply;
    windowheight = SCREENHEIGHT * multiply;

    screen = SDL_SetVideoMode(windowwidth, windowheight, 8, flags);

    if (screen == NULL)
    {
        I_Error("Error setting video mode: %s\n", SDL_GetError());
    }

    SetCaption();

    // Check if we have a native surface we can use

    native_surface = multiply == 1 && screen->pitch == SCREENWIDTH;

    // If not, allocate a buffer and copy from that buffer to the 
    // screen when we do an update

    if (native_surface)
	screens[0] = (unsigned char *) (screen->pixels);
    else
	screens[0] = (unsigned char *) Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    if (native_surface)
        SDL_LockSurface(screen);

    LoadDiskImage();

    SDL_EnableUNICODE(1);

    // start with a clear black screen

    memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);

    // clear out any events waiting at the start
  
    while (SDL_PollEvent(&dummy));
}

