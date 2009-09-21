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

#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "opl.h"
#include "opl_internal.h"

#define OPL_DEBUG_TRACE

#ifdef HAVE_IOPERM
extern opl_driver_t opl_linux_driver;
#endif
extern opl_driver_t opl_sdl_driver;

static opl_driver_t *drivers[] =
{
#ifdef HAVE_IOPERM
    &opl_linux_driver,
#endif
    &opl_sdl_driver,
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

void OPL_SetCallback(unsigned int ms, opl_callback_t callback, void *data)
{
    if (driver != NULL)
    {
        driver->set_callback_func(ms, callback, data);
    }
}

void OPL_ClearCallbacks(void)
{
    if (driver != NULL)
    {
        driver->clear_callbacks_func();
    }
}

void OPL_Lock(void)
{
    if (driver != NULL)
    {
        driver->lock_func();
    }
}

void OPL_Unlock(void)
{
    if (driver != NULL)
    {
        driver->unlock_func();
    }
}

typedef struct
{
    int finished;

    SDL_mutex *mutex;
    SDL_cond *cond;
} delay_data_t;

static void DelayCallback(void *_delay_data)
{
    delay_data_t *delay_data = _delay_data;

    SDL_LockMutex(delay_data->mutex);
    delay_data->finished = 1;
    SDL_UnlockMutex(delay_data->mutex);

    SDL_CondSignal(delay_data->cond);
}

void OPL_Delay(unsigned int ms)
{
    delay_data_t delay_data;

    if (driver == NULL)
    {
        return;
    }

    // Create a callback that will signal this thread after the
    // specified time.

    delay_data.finished = 0;
    delay_data.mutex = SDL_CreateMutex();
    delay_data.cond = SDL_CreateCond();

    OPL_SetCallback(ms, DelayCallback, &delay_data);

    // Wait until the callback is invoked.

    SDL_LockMutex(delay_data.mutex);

    while (!delay_data.finished)
    {
        SDL_CondWait(delay_data.cond, delay_data.mutex);
    }

    SDL_UnlockMutex(delay_data.mutex);

    // Clean up.

    SDL_DestroyMutex(delay_data.mutex);
    SDL_DestroyCond(delay_data.cond);
}

void OPL_SetPaused(int paused)
{
    if (driver != NULL)
    {
        driver->set_paused_func(paused);
    }
}

