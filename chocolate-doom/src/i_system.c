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
rcsid[] = "$Id$";


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




int	mb_used = 6;


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
    printf("zone memory: %x, %x allocated for zone\n", 
	   (int) zonemem, *size);
    return zonemem;
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
    static Uint32 basetime = 0;
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    ticks -= basetime;

    return (ticks * 35) / 1000;    
}



//
// I_Init
//
void I_Init (void)
{
    I_InitSound();

    // initialise timer

    SDL_Init(SDL_INIT_TIMER);
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
    exit(0);
}

void I_WaitVBL(int count)
{
    SDL_Delay((count * 1000) / 70);
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
