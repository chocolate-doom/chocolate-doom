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
//     OPL Win9x native interface.
//
//-----------------------------------------------------------------------------

#include "config.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opl.h"
#include "opl_internal.h"
#include "opl_timer.h"

static unsigned int opl_port_base;

// MingW?

#if defined(__GNUC__) && defined(__i386__)

static unsigned int OPL_Win9x_PortRead(opl_port_t port)
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

static void OPL_Win9x_PortWrite(opl_port_t port, unsigned int value)
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

// TODO: MSVC version
// #elif defined(_MSC_VER) && defined(_M_IX6) ...

#else

// Not x86, or don't know how to do port R/W on this compiler.

#define NO_PORT_RW

static unsigned int OPL_Win9x_PortRead(opl_port_t port)
{
    return 0;
}

static void OPL_Win9x_PortWrite(opl_port_t port, unsigned int value)
{
}

#endif

static int OPL_Win9x_Init(unsigned int port_base)
{
#ifndef NO_PORT_RW

    OSVERSIONINFO version_info;

    // Check that this is a Windows 9x series OS:

    memset(&version_info, 0, sizeof(version_info));
    version_info.dwOSVersionInfoSize = sizeof(version_info);

    GetVersionEx(&version_info);

    if (version_info.dwPlatformId == 1)
    {
        opl_port_base = port_base;

        // Start callback thread

        return OPL_Timer_StartThread();
    }

#endif

    return 0;
}

static void OPL_Win9x_Shutdown(void)
{
    // Stop callback thread

    OPL_Timer_StopThread();
}

opl_driver_t opl_win9x_driver =
{
    "Win9x",
    OPL_Win9x_Init,
    OPL_Win9x_Shutdown,
    OPL_Win9x_PortRead,
    OPL_Win9x_PortWrite,
    OPL_Timer_SetCallback,
    OPL_Timer_ClearCallbacks,
    OPL_Timer_Lock,
    OPL_Timer_Unlock,
    OPL_Timer_SetPaused
};

#endif /* #ifdef _WIN32 */

