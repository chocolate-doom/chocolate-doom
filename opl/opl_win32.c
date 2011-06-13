// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
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
//     OPL Win32 native interface.
//
//-----------------------------------------------------------------------------

#include "config.h"

#ifdef _WIN32

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opl.h"
#include "opl_internal.h"
#include "opl_timer.h"

#include "ioperm_sys.h"

static unsigned int opl_port_base;

// MingW?

#if defined(__GNUC__) && defined(__i386__)

static unsigned int OPL_Win32_PortRead(opl_port_t port)
{
    unsigned char result;

    __asm__ volatile (
       "movl %1, %%edx\n"
       "inb  %%dx, %%al\n"
       "movb %%al, %0"
       :   "=m" (result)
       :   "r" (opl_port_base + port)
       :   "edx", "al", "memory"
    );

    return result;
}

static void OPL_Win32_PortWrite(opl_port_t port, unsigned int value)
{
    __asm__ volatile (
       "movl %0, %%edx\n"
       "movb %1, %%al\n"
       "outb %%al, %%dx"
       :
       :   "r" (opl_port_base + port), "r" ((unsigned char) value)
       :   "edx", "al"
    );
}

// haleyjd 20110417: MSVC version
#elif defined(_MSC_VER) && defined(_M_IX86)

static unsigned int OPL_Win32_PortRead(opl_port_t port)
{
    unsigned char result;
    opl_port_t dst_port = opl_port_base + port;
    
    __asm    
    {
        mov edx, dword ptr [dst_port]
        in al, dx
        mov byte ptr [result], al
    }
    
    return result;
}

static void OPL_Win32_PortWrite(opl_port_t port, unsigned int value)
{
    opl_port_t dst_port = opl_port_base + port;
    
    __asm    
    {
        mov edx, dword ptr [dst_port]
        mov al, byte ptr [value]
        out dx, al
    }
}

#else

// Not x86, or don't know how to do port R/W on this compiler.

#define NO_PORT_RW

static unsigned int OPL_Win32_PortRead(opl_port_t port)
{
    return 0;
}

static void OPL_Win32_PortWrite(opl_port_t port, unsigned int value)
{
}

#endif

static int OPL_Win32_Init(unsigned int port_base)
{
#ifndef NO_PORT_RW

    OSVERSIONINFO version_info;

    opl_port_base = port_base;

    // Check the OS version.

    memset(&version_info, 0, sizeof(version_info));
    version_info.dwOSVersionInfoSize = sizeof(version_info);

    GetVersionEx(&version_info);

    // On NT-based systems, we must acquire I/O port permissions
    // using the ioperm.sys driver.

    if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        // Install driver.

        if (!IOperm_InstallDriver())
        {
            return 0;
        }

        // Open port range.

        if (!IOperm_EnablePortRange(opl_port_base, 2, 1))
        {
            IOperm_UninstallDriver();
            return 0;
        }
    }

    // Start callback thread

    if (!OPL_Timer_StartThread())
    {
        IOperm_UninstallDriver();
        return 0;
    }

    return 1;

#endif

    return 0;
}

static void OPL_Win32_Shutdown(void)
{
    // Stop callback thread

    OPL_Timer_StopThread();

    // Unload IOperm library.

    IOperm_UninstallDriver();
}

opl_driver_t opl_win32_driver =
{
    "Win32",
    OPL_Win32_Init,
    OPL_Win32_Shutdown,
    OPL_Win32_PortRead,
    OPL_Win32_PortWrite,
    OPL_Timer_SetCallback,
    OPL_Timer_ClearCallbacks,
    OPL_Timer_Lock,
    OPL_Timer_Unlock,
    OPL_Timer_SetPaused
};

#endif /* #ifdef _WIN32 */

