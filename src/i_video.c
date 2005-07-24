// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_video.c 26 2005-07-24 02:14:04Z fraggle $
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
rcsid[] = "$Id: i_video.c 26 2005-07-24 02:14:04Z fraggle $";

#include <ctype.h>
#include <SDL.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

SDL_Surface *screen;

#define POINTER_WARP_COUNTDOWN	1

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
boolean		grabMouse;
int		doPointerWarp = POINTER_WARP_COUNTDOWN;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;

//
// Translates the SDL key
//

int xlatekey(SDL_keysym *sym)
{
    int rc;

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
        return tolower(sym->unicode);
    }
}

void I_ShutdownGraphics(void)
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

static int	lastmousex = 0;
static int	lastmousey = 0;
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
                D_PostEvent(&event);
                break;
            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = xlatekey(&sdlevent.key.keysym);
                D_PostEvent(&event);
                break;
#if 0
          case ButtonPress:
            event.type = ev_mouse;
            event.data1 =
                (X_event.xbutton.state & Button1Mask)
                | (X_event.xbutton.state & Button2Mask ? 2 : 0)
                | (X_event.xbutton.state & Button3Mask ? 4 : 0)
                | (X_event.xbutton.button == Button1)
                | (X_event.xbutton.button == Button2 ? 2 : 0)
                | (X_event.xbutton.button == Button3 ? 4 : 0);
            event.data2 = event.data3 = 0;
            D_PostEvent(&event);
            // fprintf(stderr, "b");
            break;
          case ButtonRelease:
            event.type = ev_mouse;
            event.data1 =
                (X_event.xbutton.state & Button1Mask)
                | (X_event.xbutton.state & Button2Mask ? 2 : 0)
                | (X_event.xbutton.state & Button3Mask ? 4 : 0);
            // suggest parentheses around arithmetic in operand of |
            event.data1 =
                event.data1
                ^ (X_event.xbutton.button == Button1 ? 1 : 0)
                ^ (X_event.xbutton.button == Button2 ? 2 : 0)
                ^ (X_event.xbutton.button == Button3 ? 4 : 0);
            event.data2 = event.data3 = 0;
            D_PostEvent(&event);
            // fprintf(stderr, "bu");
            break;
          case MotionNotify:
            event.type = ev_mouse;
            event.data1 =
                (X_event.xmotion.state & Button1Mask)
                | (X_event.xmotion.state & Button2Mask ? 2 : 0)
                | (X_event.xmotion.state & Button3Mask ? 4 : 0);
            event.data2 = (X_event.xmotion.x - lastmousex) << 2;
            event.data3 = (lastmousey - X_event.xmotion.y) << 2;

            if (event.data2 || event.data3)
            {
                lastmousex = X_event.xmotion.x;
                lastmousey = X_event.xmotion.y;
                if (X_event.xmotion.x != X_width/2 &&
                    X_event.xmotion.y != X_height/2)
                {
                    D_PostEvent(&event);
                    // fprintf(stderr, "m");
                    mousemoved = false;
                } else
                {
                    mousemoved = true;
                }
            }
            break;
            
          case Expose:
          case ConfigureNotify:
            break;
            
          default:
            if (doShm && X_event.type == X_shmeventtype) shmFinished = true;
            break;
#endif
        }
    }
}
//
// I_StartTic
//
void I_StartTic (void)
{
    I_GetEvent();
#if 0

    if (!X_display)
	return;

    while (XPending(X_display))
	I_GetEvent();

    // Warp the pointer back to the middle of the window
    //  or it will wander off - that is, the game will
    //  loose input focus within X11.
    if (grabMouse)
    {
	if (!--doPointerWarp)
	{
	    XWarpPointer( X_display,
			  None,
			  X_mainWindow,
			  0, 0,
			  0, 0,
			  X_width/2, X_height/2);

	    doPointerWarp = POINTER_WARP_COUNTDOWN;
	}
    }

    mousemoved = false;
#endif
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
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
#if 0
    // scales the screen size before blitting it
    if (multiply == 2)
    {
	unsigned int *olineptrs[2];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int twoopixels;
	unsigned int twomoreopixels;
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<2 ; i++)
	    olineptrs[i] = (unsigned int *) &image->data[i*X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		twoopixels =	(fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xffff00)
		    |	((fouripixels>>16) & 0xff);
		twomoreopixels =	((fouripixels<<16) & 0xff000000)
		    |	((fouripixels<<8) & 0xffff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
#else
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
#endif
	    } while (x-=4);
	    olineptrs[0] += X_width/4;
	    olineptrs[1] += X_width/4;
	}

    }
    else if (multiply == 3)
    {
	unsigned int *olineptrs[3];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int fouropixels[3];
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<3 ; i++)
	    olineptrs[i] = (unsigned int *) &image->data[i*X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels[0] = (fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xff0000)
		    |	((fouripixels>>16) & 0xffff);
		fouropixels[1] = ((fouripixels<<8) & 0xff000000)
		    |	(fouripixels & 0xffff00)
		    |	((fouripixels>>8) & 0xff);
		fouropixels[2] = ((fouripixels<<16) & 0xffff0000)
		    |	((fouripixels<<8) & 0xff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
#else
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
#endif
	    } while (x-=4);
	    olineptrs[0] += 2*X_width/4;
	    olineptrs[1] += 2*X_width/4;
	    olineptrs[2] += 2*X_width/4;
	}

    }
    else if (multiply == 4)
    {
	// Broken. Gotta fix this some day.
	void Expand4(unsigned *, double *);
  	Expand4 ((unsigned *)(screens[0]), (double *) (image->data));
    }
#endif

    // draw to screen
    
    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
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
void I_SetPalette (byte* palette)
{
    SDL_Color sdl_palette[256];
    int i;

    for (i=0; i<256; ++i) 
    {
        sdl_palette[i].r = *palette++;
        sdl_palette[i].g = *palette++;
        sdl_palette[i].b = *palette++;
    }

    SDL_SetColors(screen, sdl_palette, 0, 256);
}



void I_InitGraphics(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    screen = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, 8, 0);

    if (multiply == 1)
	screens[0] = (unsigned char *) (screen->pixels);
    else
	screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);

    SDL_EnableUNICODE(1);
}


unsigned	exptable[256];

void InitExpand (void)
{
    int		i;
	
    for (i=0 ; i<256 ; i++)
	exptable[i] = i | (i<<8) | (i<<16) | (i<<24);
}

double		exptable2[256*256];

void InitExpand2 (void)
{
    int		i;
    int		j;
    // UNUSED unsigned	iexp, jexp;
    double*	exp;
    union
    {
	double 		d;
	unsigned	u[2];
    } pixel;
	
    printf ("building exptable2...\n");
    exp = exptable2;
    for (i=0 ; i<256 ; i++)
    {
	pixel.u[0] = i | (i<<8) | (i<<16) | (i<<24);
	for (j=0 ; j<256 ; j++)
	{
	    pixel.u[1] = j | (j<<8) | (j<<16) | (j<<24);
	    *exp++ = pixel.d;
	}
    }
    printf ("done.\n");
}

int	inited;

void
Expand4
( unsigned*	lineptr,
  double*	xline )
{
    double	dpixel;
    unsigned	x;
    unsigned 	y;
    unsigned	fourpixels;
    unsigned	step;
    double*	exp;
	
    exp = exptable2;
    if (!inited)
    {
	inited = 1;
	InitExpand2 ();
    }
		
		
    step = 3*SCREENWIDTH/2;
	
    y = SCREENHEIGHT-1;
    do
    {
	x = SCREENWIDTH;

	do
	{
	    fourpixels = lineptr[0];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[0] = dpixel;
	    xline[160] = dpixel;
	    xline[320] = dpixel;
	    xline[480] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[1] = dpixel;
	    xline[161] = dpixel;
	    xline[321] = dpixel;
	    xline[481] = dpixel;

	    fourpixels = lineptr[1];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[2] = dpixel;
	    xline[162] = dpixel;
	    xline[322] = dpixel;
	    xline[482] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[3] = dpixel;
	    xline[163] = dpixel;
	    xline[323] = dpixel;
	    xline[483] = dpixel;

	    fourpixels = lineptr[2];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[4] = dpixel;
	    xline[164] = dpixel;
	    xline[324] = dpixel;
	    xline[484] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[5] = dpixel;
	    xline[165] = dpixel;
	    xline[325] = dpixel;
	    xline[485] = dpixel;

	    fourpixels = lineptr[3];
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[6] = dpixel;
	    xline[166] = dpixel;
	    xline[326] = dpixel;
	    xline[486] = dpixel;
			
	    dpixel = *(double *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[7] = dpixel;
	    xline[167] = dpixel;
	    xline[327] = dpixel;
	    xline[487] = dpixel;

	    lineptr+=4;
	    xline+=8;
	} while (x-=16);
	xline += step;
    } while (y--);
}


