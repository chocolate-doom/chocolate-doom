// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_system.c 289 2006-01-13 18:23:28Z fraggle $
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
// Revision 1.17  2006/01/13 18:23:27  fraggle
// Textscreen getchar() function; remove SDL code from I_Endoom.
//
// Revision 1.16  2006/01/08 18:13:32  fraggle
// show_endoom config file option to disable the endoom screen
//
// Revision 1.15  2005/12/30 18:50:53  fraggle
// Millisecond clock function
//
// Revision 1.14  2005/11/17 09:41:24  fraggle
// Catch SDL_QUIT event on ENDOOM display
//
// Revision 1.13  2005/10/09 20:19:21  fraggle
// Handle blinking text in ENDOOM lumps properly.
//
// Revision 1.12  2005/10/02 03:23:54  fraggle
// Fix the length of time that ENDOOM is displayed for
//
// Revision 1.11  2005/10/02 03:16:29  fraggle
// ENDOOM support using text mode emulation
//
// Revision 1.10  2005/09/22 13:13:47  fraggle
// Remove external statistics driver support (-statcopy):
// nonfunctional on modern systems and never used.
// Fix for systems where sizeof(int) != sizeof(void *)
//
// Revision 1.9  2005/09/08 22:10:40  fraggle
// Delay calls so we don't use the entire CPU
//
// Revision 1.8  2005/09/06 22:39:43  fraggle
// Restore -nosound, -nosfx, -nomusic
//
// Revision 1.7  2005/08/04 18:42:15  fraggle
// Silence compiler warnings
//
// Revision 1.6  2005/08/04 01:14:37  fraggle
// Begin/EndRead now in i_video.c
//
// Revision 1.5  2005/07/25 20:41:59  fraggle
// Port timer code to SDL
//
// Revision 1.4  2005/07/23 19:42:56  fraggle
// Startup messages as in the DOS exes
//
// Revision 1.3  2005/07/23 18:56:07  fraggle
// Remove unneccessary pragmas
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:39  fraggle
// Initial import
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_system.c 289 2006-01-13 18:23:28Z fraggle $";


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <SDL.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#include "i_system.h"
#include "txt_main.h"


#include "w_wad.h"
#include "z_zone.h"


int	mb_used = 6;
int     show_endoom = 1;

void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
{
    byte *zonemem;
    *size = mb_used*1024*1024;
    zonemem = malloc (*size);
    printf("zone memory: %p, %x allocated for zone\n", 
           zonemem, *size);
    return zonemem;
}



//
// I_GetTime
// returns time in 1/35th second tics
//

static Uint32 basetime = 0;

int  I_GetTime (void)
{
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    ticks -= basetime;

    return (ticks * 35) / 1000;    
}

//
// Same as I_GetTime, but returns time in milliseconds
//

int I_GetTimeMS(void)
{
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    return ticks - basetime;
}



//
// I_Init
//
void I_Init (void)
{
    I_InitSound();
    I_InitMusic();

    // initialise timer

    SDL_Init(SDL_INIT_TIMER);
}

// 
// Displays the text mode ending screen after the game quits
//

void I_Endoom(void)
{
    unsigned char *endoom_data;
    unsigned char *screendata;
    unsigned int start_ms;
    boolean waiting;

    endoom_data = W_CacheLumpName("ENDOOM", PU_STATIC);

    // Set up text mode screen

    TXT_Init();

    // Make sure the new window has the right title and icon
 
    I_SetWindowCaption();
    I_SetWindowIcon();
    
    // Write the data to the screen memory
  
    screendata = TXT_GetScreenData();
    memcpy(screendata, endoom_data, 4000);

    // Wait for 10 seconds, or until a keypress or mouse click

    waiting = true;
    start_ms = I_GetTime();

    while (waiting && I_GetTime() < start_ms + 350)
    {
        TXT_UpdateScreen();

        if (TXT_GetChar() >= 0)
        {
            break;
        }
        
        I_Sleep(50);
    }
    
    // Shut down text mode screen

    TXT_Shutdown();
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();

    if (show_endoom)
    {
        I_Endoom();
    }

    exit(0);
}

void I_WaitVBL(int count)
{
    SDL_Delay((count * 1000) / 70);
}

// Sleep for a specified number of ms

void I_Sleep(int ms)
{
    SDL_Delay(ms);
}

byte*	I_AllocLow(int length)
{
    byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
extern boolean demorecording;

void I_Error (char *error, ...)
{
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();
    
    exit(-1);
}

