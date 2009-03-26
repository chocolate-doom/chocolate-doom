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
//
//-----------------------------------------------------------------------------



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "i_joystick.h"
#include "i_timer.h"
#include "i_video.h"
#include "s_sound.h"

#include "d_net.h"
#include "g_game.h"

#include "i_system.h"
#include "txt_main.h"


#include "w_wad.h"
#include "z_zone.h"

int mb_used = 16;
int show_endoom = 1;

// Tactile feedback function, probably used for the Logitech Cyberman

void I_Tactile(int on, int off, int total)
{
}

int  I_GetHeapSize (void)
{
    int p;

    //!
    // @arg <mb>
    //
    // Specify the heap size, in MiB (default 16).
    //

    p = M_CheckParm("-mb");
    
    if (p > 0)
    {
        mb_used = atoi(myargv[p+1]);
    }
    
    return mb_used*1024*1024;
}

byte *I_ZoneBase (int *size)
{
    byte *zonemem;

    *size = I_GetHeapSize();

    zonemem = malloc(*size);
    
    printf("zone memory: %p, %x allocated for zone\n", 
           zonemem, *size);

    return zonemem;
}

// 
// I_ConsoleStdout
//
// Returns true if stdout is a real console, false if it is a file
//

boolean I_ConsoleStdout(void)
{
#ifdef _WIN32
    // SDL "helpfully" always redirects stdout to a file.
    return 0;
#else
    return isatty(fileno(stdout));
#endif
}

//
// I_Init
//
void I_Init (void)
{
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
}

// 
// Displays the text mode ending screen after the game quits
//

void I_Endoom(void)
{
    unsigned char *endoom_data;
    unsigned char *screendata;

    endoom_data = W_CacheLumpName(DEH_String("ENDOOM"), PU_STATIC);

    // Set up text mode screen

    TXT_Init();

    // Make sure the new window has the right title and icon
 
    I_SetWindowCaption();
    I_SetWindowIcon();
    
    // Write the data to the screen memory
  
    screendata = TXT_GetScreenData();
    memcpy(screendata, endoom_data, 4000);

    // Wait for a keypress

    while (true)
    {
        TXT_UpdateScreen();

        if (TXT_GetChar() >= 0)
        {
            break;
        }
        
        TXT_Sleep(0);
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
    G_CheckDemoStatus();
    S_Shutdown();

    if (!screensaver_mode)
    {
        M_SaveDefaults ();
    }

    I_ShutdownGraphics();

    if (show_endoom && !testcontrols && !screensaver_mode)
    {
        I_Endoom();
    }

    exit(0);
}

void I_WaitVBL(int count)
{
    I_Sleep((count * 1000) / 70);
}

//
// I_Error
//
extern boolean demorecording;

static boolean already_quitting = false;

void I_Error (char *error, ...)
{
    va_list	argptr;

    if (already_quitting)
    {
        fprintf(stderr, "Warning: recursive call to I_Error detected.\n");
        exit(-1);
    }
    else
    {
        already_quitting = true;
    }
    
    // Message first.
    va_start(argptr, error);
    fprintf(stderr, "\nError: ");
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
    fflush(stderr);

    // Shutdown. Here might be other errors.

    if (demorecording)
    {
	G_CheckDemoStatus();
    }

    D_QuitNetGame ();
    I_ShutdownGraphics();
    S_Shutdown();
    
#ifdef _WIN32
    // On Windows, pop up a dialog box with the error message.
    {
        char msgbuf[512];

        va_start(argptr, error);
        memset(msgbuf, 0, sizeof(msgbuf));
        vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
        va_end(argptr);

        MessageBox(NULL, msgbuf, "Error", MB_OK);
    }
#endif

    // abort();

    exit(-1);
}

