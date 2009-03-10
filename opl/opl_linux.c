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
//     OPL Linux interface.
//
//-----------------------------------------------------------------------------

#include "config.h"

#ifdef HAVE_IOPERM

#include <unistd.h>
#include <sys/io.h>

#include "opl.h"
#include "opl_internal.h"

static unsigned int opl_port_base;

static void OPL_Linux_Init(unsigned int port_base)
{
    // Try to get permissions:

    if (ioperm(port_base, 2, 1) < 0)
    {
        return 0;
    }

    opl_port_base = port_base;

    return 1;
}

static void OPL_Linux_Shutdown(void)
{
    // Release permissions

    ioperm(opl_port_base, 2, 0);
}

static unsigned int OPL_Linux_PortRead(opl_port_t port)
{
    return inb(opl_port_base + port);
}

static void OPL_Linux_PortWrite(opl_port_t port, unsigned int value)
{
    outb(opl_port_base + port, value);
}

opl_driver_t opl_linux_driver =
{
    "Linux",
    OPL_Linux_Init,
    OPL_Linux_Shutdown,
    OPL_Linux_PortRead,
    OPL_Linux_PortWrite
};

#endif /* #ifdef HAVE_IOPERM */

