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

#include "config.h"

#include "deh_str.h"
#include "doomtype.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "i_joystick.h"
#include "i_sound.h"
#include "i_timer.h"
#include "i_video.h"

#include "i_system.h"

#include "w_wad.h"
#include "z_zone.h"

#define DEFAULT_RAM 16 /* MiB */
#define MIN_RAM     4  /* MiB */


typedef struct atexit_listentry_s atexit_listentry_t;

struct atexit_listentry_s
{
    atexit_func_t func;
    boolean run_on_error;
    atexit_listentry_t *next;
};

static atexit_listentry_t *exit_funcs = NULL;

void I_AtExit(atexit_func_t func, boolean run_on_error)
{
    atexit_listentry_t *entry;

    entry = malloc(sizeof(*entry));

    entry->func = func;
    entry->run_on_error = run_on_error;
    entry->next = exit_funcs;
    exit_funcs = entry;
}

// Tactile feedback function, probably used for the Logitech Cyberman

void I_Tactile(int on, int off, int total)
{
}

#ifdef _WIN32_WCE

// Windows CE-specific auto-allocation function that allocates the zone
// size based on the amount of memory reported free by the OS.

static byte *AutoAllocMemory(int *size, int default_ram, int min_ram)
{
    MEMORYSTATUS memory_status;
    byte *zonemem;
    size_t available;

    // Get available physical RAM.  We leave one megabyte extra free
    // for the OS to keep running (my PDA becomes unstable if too
    // much RAM is allocated)

    GlobalMemoryStatus(&memory_status);
    available = memory_status.dwAvailPhys - 2 * 1024 * 1024;

    // Limit to default_ram if we have more than that available:

    if (available > default_ram * 1024 * 1024)
    {
        available = default_ram * 1024 * 1024;
    }

    if (available < min_ram * 1024 * 1024)
    {
        I_Error("Unable to allocate %i MiB of RAM for zone", min_ram);
    }

    // Allocate zone:

    *size = available;
    zonemem = malloc(*size);

    if (zonemem == NULL)
    {
        I_Error("Failed when allocating %i bytes", *size);
    }

    return zonemem;
}

#else

// Zone memory auto-allocation function that allocates the zone size
// by trying progressively smaller zone sizes until one is found that
// works.

static byte *AutoAllocMemory(int *size, int default_ram, int min_ram)
{
    byte *zonemem;

    // Allocate the zone memory.  This loop tries progressively smaller
    // zone sizes until a size is found that can be allocated.
    // If we used the -mb command line parameter, only the parameter
    // provided is accepted.

    zonemem = NULL;

    while (zonemem == NULL)
    {
        // We need a reasonable minimum amount of RAM to start.

        if (default_ram < min_ram)
        {
            I_Error("Unable to allocate %i MiB of RAM for zone", default_ram);
        }

        // Try to allocate the zone memory.

        *size = default_ram * 1024 * 1024;

        zonemem = malloc(*size);

        // Failed to allocate?  Reduce zone size until we reach a size
        // that is acceptable.

        if (zonemem == NULL)
        {
            default_ram -= 1;
        }
    }

    return zonemem;
}

#endif

byte *I_ZoneBase (int *size)
{
    byte *zonemem;
    int min_ram, default_ram;
    int p;

    //!
    // @arg <mb>
    //
    // Specify the heap size, in MiB (default 16).
    //

    p = M_CheckParm("-mb");

    if (p > 0)
    {
        default_ram = atoi(myargv[p+1]);
        min_ram = default_ram;
    }
    else
    {
        default_ram = DEFAULT_RAM;
        min_ram = MIN_RAM;
    }

    zonemem = AutoAllocMemory(size, default_ram, min_ram);

    printf("zone memory: %p, %x allocated for zone\n", 
           zonemem, *size);

    return zonemem;
}

void I_PrintBanner(char *msg)
{
    int i;
    int spaces = 35 - (strlen(msg) / 2);

    for (i=0; i<spaces; ++i)
        putchar(' ');

    puts(msg);
}

void I_PrintDivider(void)
{
    int i;

    for (i=0; i<75; ++i)
    {
        putchar('=');
    }

    putchar('\n');
}

void I_PrintStartupBanner(char *gamedescription)
{
    I_PrintDivider();
    I_PrintBanner(gamedescription);
    I_PrintDivider();
    
    printf(
    " " PACKAGE_NAME " is free software, covered by the GNU General Public\n"
    " License.  There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
    " FOR A PARTICULAR PURPOSE. You are welcome to change and distribute\n"
    " copies under certain conditions. See the source for more information.\n");

    I_PrintDivider();
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
/*
void I_Init (void)
{
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
}
void I_BindVariables(void)
{
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();
}
*/


//
// I_Quit
//

void I_Quit (void)
{
    atexit_listentry_t *entry;

    // Run through all exit functions
 
    entry = exit_funcs; 

    while (entry != NULL)
    {
        entry->func();
        entry = entry->next;
    }

    exit(0);
}

//
// I_Error
//

static boolean already_quitting = false;

void I_Error (char *error, ...)
{
    va_list argptr;
    atexit_listentry_t *entry;

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
    //fprintf(stderr, "\nError: ");
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n\n");
    va_end(argptr);
    fflush(stderr);

    // Shutdown. Here might be other errors.

    entry = exit_funcs;

    while (entry != NULL)
    {
        if (entry->run_on_error)
        {
            entry->func();
        }

        entry = entry->next;
    }
  
#ifdef _WIN32
    // On Windows, pop up a dialog box with the error message.
    {
        char msgbuf[512];
        wchar_t wmsgbuf[512];

        va_start(argptr, error);
        memset(msgbuf, 0, sizeof(msgbuf));
        vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
        va_end(argptr);

        MultiByteToWideChar(CP_ACP, 0,
                            msgbuf, strlen(msgbuf) + 1,
                            wmsgbuf, sizeof(wmsgbuf));

        MessageBoxW(NULL, wmsgbuf, L"", MB_OK);
    }
#endif

    // abort();

    exit(-1);
}

