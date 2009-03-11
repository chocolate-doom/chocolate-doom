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
//     OPL interface.
//
//-----------------------------------------------------------------------------

#include "config.h"

#include <stdlib.h>

#include "opl.h"
#include "opl_internal.h"

#define OPL_DEBUG_TRACE

#ifdef HAVE_IOPERM
extern opl_driver_t opl_linux_driver;
#endif

static opl_driver_t *drivers[] =
{
#ifdef HAVE_IOPERM
    &opl_linux_driver,
#endif
    NULL
};

static opl_driver_t *driver = NULL;

int OPL_Init(unsigned int port_base)
{
    int i;

    // Try drivers until we find a working one:

    for (i=0; drivers[i] != NULL; ++i)
    {
        if (drivers[i]->init_func(port_base))
        {
            driver = drivers[i];
            return 1;
        }
    }

    return 0;
}

void OPL_Shutdown(void)
{
    if (driver != NULL)
    {
        driver->shutdown_func();
        driver = NULL;
    }
}

void OPL_WritePort(opl_port_t port, unsigned int value)
{
    if (driver != NULL)
    {
#ifdef OPL_DEBUG_TRACE
        printf("OPL_write: %i, %x\n", port, value);
#endif
        driver->write_port_func(port, value);
    }
}

unsigned int OPL_ReadPort(opl_port_t port)
{
    if (driver != NULL)
    {
        unsigned int result;

        result = driver->read_port_func(port);

#ifdef OPL_DEBUG_TRACE
        printf("OPL_read: %i -> %x\n", port, result);
#endif

        return result;
    }
    else
    {
        return 0;
    }
}

