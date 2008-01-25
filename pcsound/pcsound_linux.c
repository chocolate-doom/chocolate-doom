// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007 Simon Howard
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
//    PC speaker driver for Linux.
//
//-----------------------------------------------------------------------------

#include "config.h"

#ifdef HAVE_LINUX_KD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "pcsound.h"
#include "pcsound_internal.h"

#define CONSOLE_DEVICE "/dev/console"

static int console_handle;
static pcsound_callback_func callback;
static int sound_thread_running = 0;
static SDL_Thread *sound_thread_handle;
static int sleep_adjust = 0;

static void AdjustedSleep(unsigned int ms)
{
    unsigned int start_time;
    unsigned int end_time;
    unsigned int actual_time;

    // Adjust based on previous error to keep the tempo right

    if (sleep_adjust > ms)
    {
        sleep_adjust -= ms;
        return;
    }
    else
    {
        ms -= sleep_adjust;
    }

    // Do the sleep and record how long it takes

    start_time = SDL_GetTicks();

    SDL_Delay(ms);
    
    end_time = SDL_GetTicks();

    if (end_time > start_time)
    {
        actual_time = end_time - start_time;
    }
    else
    {
        actual_time = ms;
    }

    if (actual_time < ms)
    {
        actual_time = ms;
    }

    // Save sleep_adjust for next time

    sleep_adjust = actual_time - ms;
}

static int SoundThread(void *unused)
{
    int frequency;
    int duration;
    int cycles;
    
    while (sound_thread_running)
    {
        callback(&duration, &frequency);

        if (frequency != 0) 
        {
            cycles = PCSOUND_8253_FREQUENCY / frequency;
        }
        else
        {
            cycles = 0;
        }

        ioctl(console_handle, KIOCSOUND, cycles);

        AdjustedSleep(duration);
    }

    return 0;
}

static int PCSound_Linux_Init(pcsound_callback_func callback_func)
{
    // Try to open the console

    console_handle = open(CONSOLE_DEVICE, O_WRONLY);

    if (console_handle == -1)
    {
        // Don't have permissions for the console device?

	fprintf(stderr, "PCSound_Linux_Init: Failed to open '%s': %s\n",
			CONSOLE_DEVICE, strerror(errno));
        return 0;
    }

    if (ioctl(console_handle, KIOCSOUND, 0) < 0)
    {
        // KIOCSOUND not supported: non-PC linux?

        close(console_handle);
        return 0;
    }

    // Start a thread up to generate PC speaker output
    
    callback = callback_func;
    sound_thread_running = 1;

    sound_thread_handle = SDL_CreateThread(SoundThread, NULL);
    
    return 1;
}

static void PCSound_Linux_Shutdown(void)
{
    sound_thread_running = 0;
    SDL_WaitThread(sound_thread_handle, NULL);
    close(console_handle);
}

pcsound_driver_t pcsound_linux_driver =
{
    "Linux",
    PCSound_Linux_Init,
    PCSound_Linux_Shutdown,
};

#endif /* #ifdef HAVE_LINUX_KD_H */

