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

#ifdef __MACOSX__
#include <CoreFoundation/CFUserNotification.h>
#endif

#define DEFAULT_RAM 16 /* MiB */
#define MIN_RAM     4  /* MiB */

int show_endoom = 1;

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

    p = M_CheckParmWithArgs("-mb", 1);

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

#define ENDOOM_W 80
#define ENDOOM_H 25

// 
// Displays the text mode ending screen after the game quits
//

void I_Endoom(void)
{
    unsigned char *endoom_data;
    unsigned char *screendata;
    int y;
    int indent;

    endoom_data = W_CacheLumpName(DEH_String("ENDOOM"), PU_STATIC);

    // Set up text mode screen

    TXT_Init();

    // Make sure the new window has the right title and icon
 
    I_SetWindowCaption();
    I_SetWindowIcon();
    
    // Write the data to the screen memory
  
    screendata = TXT_GetScreenData();

    indent = (ENDOOM_W - TXT_SCREEN_W) / 2;

    for (y=0; y<TXT_SCREEN_H; ++y)
    {
        memcpy(screendata + (y * TXT_SCREEN_W * 2),
               endoom_data + (y * ENDOOM_W + indent) * 2,
               TXT_SCREEN_W * 2);
    }

    // Wait for a keypress

    while (true)
    {
        TXT_UpdateScreen();

        if (TXT_GetChar() > 0)
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
    //fprintf(stderr, "\nError: ");
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n\n");
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

#ifdef __MACOSX__
    {
        CFStringRef message;
        char msgbuf[512];
	int i;

        va_start(argptr, error);
        memset(msgbuf, 0, sizeof(msgbuf));
        vsnprintf(msgbuf, sizeof(msgbuf) - 1, error, argptr);
        va_end(argptr);

	// The CoreFoundation message box wraps text lines, so replace
	// newline characters with spaces so that multiline messages
	// are continuous.

	for (i = 0; msgbuf[i] != '\0'; ++i)
        {
            if (msgbuf[i] == '\n')
            {
                msgbuf[i] = ' ';
            }
        }

        message = CFStringCreateWithCString(NULL, msgbuf,
                                            kCFStringEncodingUTF8);

        CFUserNotificationDisplayNotice(0,
                                        kCFUserNotificationCautionAlertLevel,
                                        NULL,
                                        NULL,
                                        NULL,
                                        CFSTR(PACKAGE_STRING),
                                        message,
                                        NULL);
    }
#endif

    // abort();

    exit(-1);
}

//
// Read Access Violation emulation.
//
// From PrBoom+, by entryway.
//

// C:\>debug
// -d 0:0
//
// DOS 6.22:
// 0000:0000  (57 92 19 00) F4 06 70 00-(16 00)
// DOS 7.1:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// Win98:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// DOSBox under XP:
// 0000:0000  (00 00 00 F1) ?? ?? ?? 00-(07 00)

#define DOS_MEM_DUMP_SIZE 10

static const unsigned char mem_dump_dos622[DOS_MEM_DUMP_SIZE] = {
  0x57, 0x92, 0x19, 0x00, 0xF4, 0x06, 0x70, 0x00, 0x16, 0x00};
static const unsigned char mem_dump_win98[DOS_MEM_DUMP_SIZE] = {
  0x9E, 0x0F, 0xC9, 0x00, 0x65, 0x04, 0x70, 0x00, 0x16, 0x00};
static const unsigned char mem_dump_dosbox[DOS_MEM_DUMP_SIZE] = {
  0x00, 0x00, 0x00, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00};
static unsigned char mem_dump_custom[DOS_MEM_DUMP_SIZE];

static const unsigned char *dos_mem_dump = mem_dump_dos622;

boolean I_GetMemoryValue(unsigned int offset, void *value, int size)
{
    static boolean firsttime = true;

    if (firsttime)
    {
        int p, i, val;

        firsttime = false;
        i = 0;

        //!
        // @category compat
        // @arg <version>
        //
        // Specify DOS version to emulate for NULL pointer dereference
        // emulation.  Supported versions are: dos622, dos71, dosbox.
        // The default is to emulate DOS 7.1 (Windows 98).
        //

        p = M_CheckParmWithArgs("-setmem", 1);

        if (p > 0)
        {
            if (!strcasecmp(myargv[p + 1], "dos622"))
            {
                dos_mem_dump = mem_dump_dos622;
            }
            if (!strcasecmp(myargv[p + 1], "dos71"))
            {
                dos_mem_dump = mem_dump_win98;
            }
            else if (!strcasecmp(myargv[p + 1], "dosbox"))
            {
                dos_mem_dump = mem_dump_dosbox;
            }
            else
            {
                for (i = 0; i < DOS_MEM_DUMP_SIZE; ++i)
                {
                    ++p;

                    if (p >= myargc || myargv[p][0] == '-')
                    {
                        break;
                    }

                    M_StrToInt(myargv[p], &val);
                    mem_dump_custom[i++] = (unsigned char) val;
                }

                dos_mem_dump = mem_dump_custom;
            }
        }
    }

    switch (size)
    {
    case 1:
        *((unsigned char *) value) = dos_mem_dump[offset];
        return true;
    case 2:
        *((unsigned short *) value) = dos_mem_dump[offset]
                                    | (dos_mem_dump[offset + 1] << 8);
        return true;
    case 4:
        *((unsigned int *) value) = dos_mem_dump[offset]
                                  | (dos_mem_dump[offset + 1] << 8)
                                  | (dos_mem_dump[offset + 2] << 16)
                                  | (dos_mem_dump[offset + 3] << 24);
        return true;
    }

    return false;
}

