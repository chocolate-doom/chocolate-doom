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
//     OPL OpenBSD interface (also NetBSD)
//
//-----------------------------------------------------------------------------

#include "config.h"

// OpenBSD has a i386_iopl on i386 and amd64_iopl on x86_64,
// even though they do the same thing.  Take care of this
// here, and map set_iopl to point to the appropriate name.

#if defined(HAVE_LIBI386)

#include <sys/types.h>
#include <machine/sysarch.h>
#include <i386/pio.h>
#define set_iopl i386_iopl

#elif defined(HAVE_LIBAMD64)

#include <sys/types.h>
#include <machine/sysarch.h>
#include <amd64/pio.h>
#define set_iopl amd64_iopl

#else
#define NO_OBSD_DRIVER
#endif

// If the above succeeded, proceed with the rest.

#ifndef NO_OBSD_DRIVER

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "opl.h"
#include "opl_internal.h"
#include "opl_timer.h"

static unsigned int opl_port_base;

static int OPL_OpenBSD_Init(unsigned int port_base)
{
    // Try to get permissions:

    if (set_iopl(3) < 0)
    {
        fprintf(stderr, "Failed to get raise I/O privilege level: "
                        "check that you are running as root.\n");
        return 0;
    }

    opl_port_base = port_base;

    // Start callback thread

    if (!OPL_Timer_StartThread())
    {
        set_iopl(0);
        return 0;
    }

    return 1;
}

static void OPL_OpenBSD_Shutdown(void)
{
    // Stop callback thread

    OPL_Timer_StopThread();

    // Release I/O port permissions:

    set_iopl(0);
}

static unsigned int OPL_OpenBSD_PortRead(opl_port_t port)
{
    return inb(opl_port_base + port);
}

static void OPL_OpenBSD_PortWrite(opl_port_t port, unsigned int value)
{
    outb(opl_port_base + port, value);
}

opl_driver_t opl_openbsd_driver =
{
    "OpenBSD",
    OPL_OpenBSD_Init,
    OPL_OpenBSD_Shutdown,
    OPL_OpenBSD_PortRead,
    OPL_OpenBSD_PortWrite,
    OPL_Timer_SetCallback,
    OPL_Timer_ClearCallbacks,
    OPL_Timer_Lock,
    OPL_Timer_Unlock,
    OPL_Timer_SetPaused
};

#endif /* #ifndef NO_OBSD_DRIVER */

