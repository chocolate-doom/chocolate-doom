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

#ifdef _WIN32_WCE
#include "libc_wince.h"
#endif

#include "SDL.h"

#include "opl.h"
#include "opl_internal.h"

//#define OPL_DEBUG_TRACE

#ifdef HAVE_IOPERM
extern opl_driver_t opl_linux_driver;
#endif
#if defined(HAVE_LIBI386) || defined(HAVE_LIBAMD64)
extern opl_driver_t opl_openbsd_driver;
#endif
#ifdef _WIN32
extern opl_driver_t opl_win32_driver;
#endif
extern opl_driver_t opl_sdl_driver;

static opl_driver_t *drivers[] =
{
#ifdef HAVE_IOPERM
    &opl_linux_driver,
#endif
#if defined(HAVE_LIBI386) || defined(HAVE_LIBAMD64)
    &opl_openbsd_driver,
#endif
#ifdef _WIN32
    &opl_win32_driver,
#endif
    &opl_sdl_driver,
    NULL
};

static opl_driver_t *driver = NULL;
static int init_stage_reg_writes = 1;

unsigned int opl_sample_rate = 22050;

//
// Init/shutdown code.
//

// Initialize the specified driver and detect an OPL chip.  Returns
// true if an OPL is detected.

static int InitDriver(opl_driver_t *_driver, unsigned int port_base)
{
    // Initialize the driver.

    if (!_driver->init_func(port_base))
    {
        return 0;
    }

    // The driver was initialized okay, so we now have somewhere
    // to write to.  It doesn't mean there's an OPL chip there,
    // though.  Perform the detection sequence to make sure.
    // (it's done twice, like how Doom does it).

    driver = _driver;
    init_stage_reg_writes = 1;

    if (!OPL_Detect() || !OPL_Detect())
    {
        printf("OPL_Init: No OPL detected using '%s' driver.\n", _driver->name);
        _driver->shutdown_func();
        driver = NULL;
        return 0;
    }

    // Initialize all registers.

    OPL_InitRegisters();

    init_stage_reg_writes = 0;

    printf("OPL_Init: Using driver '%s'.\n", driver->name);

    return 1;
}

// Find a driver automatically by trying each in the list.

static int AutoSelectDriver(unsigned int port_base)
{
    int i;

    for (i=0; drivers[i] != NULL; ++i)
    {
        if (InitDriver(drivers[i], port_base))
        {
            return 1;
        }
    }

    printf("OPL_Init: Failed to find a working driver.\n");

    return 0;
}

// Initialize the OPL library.  Returns true if initialized
// successfully.

int OPL_Init(unsigned int port_base)
{
    char *driver_name;
    int i;

    driver_name = getenv("OPL_DRIVER");

    if (driver_name != NULL)
    {
        // Search the list until we find the driver with this name.

        for (i=0; drivers[i] != NULL; ++i)
        {
            if (!strcmp(driver_name, drivers[i]->name))
            {
                if (InitDriver(drivers[i], port_base))
                {
                    return 1;
                }
                else
                {
                    printf("OPL_Init: Failed to initialize "
                           "driver: '%s'.\n", driver_name);
                    return 0;
                }
            }
        }

        printf("OPL_Init: unknown driver: '%s'.\n", driver_name);

        return 0;
    }
    else
    {
        return AutoSelectDriver(port_base);
    }
}

// Shut down the OPL library.

void OPL_Shutdown(void)
{
    if (driver != NULL)
    {
        driver->shutdown_func();
        driver = NULL;
    }
}

// Set the sample rate used for software OPL emulation.

void OPL_SetSampleRate(unsigned int rate)
{
    opl_sample_rate = rate;
}

void OPL_WritePort(opl_port_t port, unsigned int value)
{
    if (driver != NULL)
    {
#ifdef OPL_DEBUG_TRACE
        printf("OPL_write: %i, %x\n", port, value);
        fflush(stdout);
#endif
        driver->write_port_func(port, value);
    }
}

unsigned int OPL_ReadPort(opl_port_t port)
{
    if (driver != NULL)
    {
        unsigned int result;

#ifdef OPL_DEBUG_TRACE
        printf("OPL_read: %i...\n", port);
        fflush(stdout);
#endif

        result = driver->read_port_func(port);

#ifdef OPL_DEBUG_TRACE
        printf("OPL_read: %i -> %x\n", port, result);
        fflush(stdout);
#endif

        return result;
    }
    else
    {
        return 0;
    }
}

//
// Higher-level functions, based on the lower-level functions above
// (register write, etc).
//

unsigned int OPL_ReadStatus(void)
{
    return OPL_ReadPort(OPL_REGISTER_PORT);
}

// Write an OPL register value

void OPL_WriteRegister(int reg, int value)
{
    int i;

    OPL_WritePort(OPL_REGISTER_PORT, reg);

    // For timing, read the register port six times after writing the
    // register number to cause the appropriate delay

    for (i=0; i<6; ++i)
    {
        // An oddity of the Doom OPL code: at startup initialization,
        // the spacing here is performed by reading from the register
        // port; after initialization, the data port is read, instead.

        if (init_stage_reg_writes)
        {
            OPL_ReadPort(OPL_REGISTER_PORT);
        }
        else
        {
            OPL_ReadPort(OPL_DATA_PORT);
        }
    }

    OPL_WritePort(OPL_DATA_PORT, value);

    // Read the register port 24 times after writing the value to
    // cause the appropriate delay

    for (i=0; i<24; ++i)
    {
        OPL_ReadStatus();
    }
}

// Detect the presence of an OPL chip

int OPL_Detect(void)
{
    int result1, result2;
    int i;

    // Reset both timers:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x60);

    // Enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x80);

    // Read status
    result1 = OPL_ReadStatus();

    // Set timer:
    OPL_WriteRegister(OPL_REG_TIMER1, 0xff);

    // Start timer 1:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x21);

    // Wait for 80 microseconds
    // This is how Doom does it:

    for (i=0; i<200; ++i)
    {
        OPL_ReadStatus();
    }

    OPL_Delay(1);

    // Read status
    result2 = OPL_ReadStatus();

    // Reset both timers:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x60);

    // Enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL, 0x80);

    return (result1 & 0xe0) == 0x00
        && (result2 & 0xe0) == 0xc0;
}

// Initialize registers on startup

void OPL_InitRegisters(void)
{
    int r;

    // Initialize level registers

    for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x3f);
    }

    // Initialize other registers
    // These two loops write to registers that actually don't exist,
    // but this is what Doom does ...
    // Similarly, the <= is also intenational.

    for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // More registers ...

    for (r=1; r < OPL_REGS_LEVEL; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // Re-initialize the low registers:

    // Reset both timers and enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL,      0x60);
    OPL_WriteRegister(OPL_REG_TIMER_CTRL,      0x80);

    // "Allow FM chips to control the waveform of each operator":
    OPL_WriteRegister(OPL_REG_WAVEFORM_ENABLE, 0x20);

    // Keyboard split point on (?)
    OPL_WriteRegister(OPL_REG_FM_MODE,         0x40);
}

//
// Timer functions.
//

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

    SDL_CondSignal(delay_data->cond);

    SDL_UnlockMutex(delay_data->mutex);
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

