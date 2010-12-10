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
//     OPL SDL interface.
//
//-----------------------------------------------------------------------------

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "dbopl.h"

#include "opl.h"
#include "opl_internal.h"

#include "opl_queue.h"

#define MAX_SOUND_SLICE_TIME 100 /* ms */

typedef struct
{
    unsigned int rate;        // Number of times the timer is advanced per sec.
    unsigned int enabled;     // Non-zero if timer is enabled.
    unsigned int value;       // Last value that was set.
    unsigned int expire_time; // Calculated time that timer will expire.
} opl_timer_t;

// When the callback mutex is locked using OPL_Lock, callback functions
// are not invoked.

static SDL_mutex *callback_mutex = NULL;

// Queue of callbacks waiting to be invoked.

static opl_callback_queue_t *callback_queue;

// Mutex used to control access to the callback queue.

static SDL_mutex *callback_queue_mutex = NULL;

// Current time, in number of samples since startup:

static int current_time;

// If non-zero, playback is currently paused.

static int opl_sdl_paused;

// Time offset (in samples) due to the fact that callbacks
// were previously paused.

static unsigned int pause_offset;

// OPL software emulator structure.

static Chip opl_chip;

// Temporary mixing buffer used by the mixing callback.

static int32_t *mix_buffer = NULL;

// Register number that was written.

static int register_num = 0;

// Timers; DBOPL does not do timer stuff itself.

static opl_timer_t timer1 = { 12500, 0, 0, 0 };
static opl_timer_t timer2 = { 3125, 0, 0, 0 };

// SDL parameters.

static int sdl_was_initialized = 0;
static int mixing_freq, mixing_channels;
static Uint16 mixing_format;

static int SDLIsInitialized(void)
{
    int freq, channels;
    Uint16 format;

    return Mix_QuerySpec(&freq, &format, &channels);
}

// Advance time by the specified number of samples, invoking any
// callback functions as appropriate.

static void AdvanceTime(unsigned int nsamples)
{
    opl_callback_t callback;
    void *callback_data;

    SDL_LockMutex(callback_queue_mutex);

    // Advance time.

    current_time += nsamples;

    if (opl_sdl_paused)
    {
        pause_offset += nsamples;
    }

    // Are there callbacks to invoke now?  Keep invoking them
    // until there are none more left.

    while (!OPL_Queue_IsEmpty(callback_queue)
        && current_time >= OPL_Queue_Peek(callback_queue) + pause_offset)
    {
        // Pop the callback from the queue to invoke it.

        if (!OPL_Queue_Pop(callback_queue, &callback, &callback_data))
        {
            break;
        }

        // The mutex stuff here is a bit complicated.  We must
        // hold callback_mutex when we invoke the callback (so that
        // the control thread can use OPL_Lock() to prevent callbacks
        // from being invoked), but we must not be holding
        // callback_queue_mutex, as the callback must be able to
        // call OPL_SetCallback to schedule new callbacks.

        SDL_UnlockMutex(callback_queue_mutex);

        SDL_LockMutex(callback_mutex);
        callback(callback_data);
        SDL_UnlockMutex(callback_mutex);

        SDL_LockMutex(callback_queue_mutex);
    }

    SDL_UnlockMutex(callback_queue_mutex);
}

// Call the OPL emulator code to fill the specified buffer.

static void FillBuffer(int16_t *buffer, unsigned int nsamples)
{
    unsigned int i;

    // This seems like a reasonable assumption.  mix_buffer is
    // 1 second long, which should always be much longer than the
    // SDL mix buffer.

    assert(nsamples < mixing_freq);

    Chip__GenerateBlock2(&opl_chip, nsamples, mix_buffer);

    // Mix into the destination buffer, doubling up into stereo.

    for (i=0; i<nsamples; ++i)
    {
        buffer[i * 2] = (int16_t) mix_buffer[i];
        buffer[i * 2 + 1] = (int16_t) mix_buffer[i];
    }
}

// Callback function to fill a new sound buffer:

static void OPL_Mix_Callback(void *udata,
                             Uint8 *byte_buffer,
                             int buffer_bytes)
{
    int16_t *buffer;
    unsigned int buffer_len;
    unsigned int filled = 0;

    // Buffer length in samples (quadrupled, because of 16-bit and stereo)

    buffer = (int16_t *) byte_buffer;
    buffer_len = buffer_bytes / 4;

    // Repeatedly call the OPL emulator update function until the buffer is
    // full.

    while (filled < buffer_len)
    {
        unsigned int next_callback_time;
        unsigned int nsamples;

        SDL_LockMutex(callback_queue_mutex);

        // Work out the time until the next callback waiting in
        // the callback queue must be invoked.  We can then fill the
        // buffer with this many samples.

        if (opl_sdl_paused || OPL_Queue_IsEmpty(callback_queue))
        {
            nsamples = buffer_len - filled;
        }
        else
        {
            next_callback_time = OPL_Queue_Peek(callback_queue) + pause_offset;

            nsamples = next_callback_time - current_time;

            if (nsamples > buffer_len - filled)
            {
                nsamples = buffer_len - filled;
            }
        }

        SDL_UnlockMutex(callback_queue_mutex);

        // Add emulator output to buffer.

        FillBuffer(buffer + filled * 2, nsamples);
        filled += nsamples;

        // Invoke callbacks for this point in time.

        AdvanceTime(nsamples);
    }
}

static void OPL_SDL_Shutdown(void)
{
    Mix_HookMusic(NULL, NULL);

    if (sdl_was_initialized)
    {
        Mix_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        OPL_Queue_Destroy(callback_queue);
        free(mix_buffer);
        sdl_was_initialized = 0;
    }

/*
    if (opl_chip != NULL)
    {
        OPLDestroy(opl_chip);
        opl_chip = NULL;
    }
    */

    if (callback_mutex != NULL)
    {
        SDL_DestroyMutex(callback_mutex);
        callback_mutex = NULL;
    }

    if (callback_queue_mutex != NULL)
    {
        SDL_DestroyMutex(callback_queue_mutex);
        callback_queue_mutex = NULL;
    }
}

static unsigned int GetSliceSize(void)
{
    int limit;
    int n;

    limit = (opl_sample_rate * MAX_SOUND_SLICE_TIME) / 1000;

    // Try all powers of two, not exceeding the limit.

    for (n=0;; ++n)
    {
        // 2^n <= limit < 2^n+1 ?

        if ((1 << (n + 1)) > limit)
        {
            return (1 << n);
        }
    }

    // Should never happen?

    return 1024;
}

static int OPL_SDL_Init(unsigned int port_base)
{
    // Check if SDL_mixer has been opened already
    // If not, we must initialize it now

    if (!SDLIsInitialized())
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            fprintf(stderr, "Unable to set up sound.\n");
            return 0;
        }

        if (Mix_OpenAudio(opl_sample_rate, AUDIO_S16SYS, 2, GetSliceSize()) < 0)
        {
            fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());

            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return 0;
        }

        SDL_PauseAudio(0);

        // When this module shuts down, it has the responsibility to 
        // shut down SDL.

        sdl_was_initialized = 1;
    }
    else
    {
        sdl_was_initialized = 0;
    }

    opl_sdl_paused = 0;
    pause_offset = 0;

    // Queue structure of callbacks to invoke.

    callback_queue = OPL_Queue_Create();
    current_time = 0;

    // Get the mixer frequency, format and number of channels.

    Mix_QuerySpec(&mixing_freq, &mixing_format, &mixing_channels);

    // Only supports AUDIO_S16SYS

    if (mixing_format != AUDIO_S16SYS || mixing_channels != 2)
    {
        fprintf(stderr, 
                "OPL_SDL only supports native signed 16-bit LSB, "
                "stereo format!\n");

        OPL_SDL_Shutdown();
        return 0;
    }

    // Mix buffer:

    mix_buffer = malloc(mixing_freq * sizeof(uint32_t));

    // Create the emulator structure:

    DBOPL_InitTables();
    Chip__Chip(&opl_chip);
    Chip__Setup(&opl_chip, mixing_freq);

    callback_mutex = SDL_CreateMutex();
    callback_queue_mutex = SDL_CreateMutex();

    // TODO: This should be music callback? or-?
    Mix_HookMusic(OPL_Mix_Callback, NULL);

    return 1;
}

static unsigned int OPL_SDL_PortRead(opl_port_t port)
{
    unsigned int result = 0;

    if (timer1.enabled && current_time > timer1.expire_time)
    {
        result |= 0x80;   // Either have expired
        result |= 0x40;   // Timer 1 has expired
    }

    if (timer2.enabled && current_time > timer2.expire_time)
    {
        result |= 0x80;   // Either have expired
        result |= 0x20;   // Timer 2 has expired
    }

    return result;
}

static void OPLTimer_CalculateEndTime(opl_timer_t *timer)
{
    int tics;

    // If the timer is enabled, calculate the time when the timer
    // will expire.

    if (timer->enabled)
    {
        tics = 0x100 - timer->value;
        timer->expire_time = current_time
                           + (tics * opl_sample_rate) / timer->rate;
    }
}

static void WriteRegister(unsigned int reg_num, unsigned int value)
{
    switch (reg_num)
    {
        case OPL_REG_TIMER1:
            timer1.value = value;
            OPLTimer_CalculateEndTime(&timer1);
            break;

        case OPL_REG_TIMER2:
            timer2.value = value;
            OPLTimer_CalculateEndTime(&timer2);
            break;

        case OPL_REG_TIMER_CTRL:
            if (value & 0x80)
            {
                timer1.enabled = 0;
                timer2.enabled = 0;
            }
            else
            {
                if ((value & 0x40) == 0)
                {
                    timer1.enabled = (value & 0x01) != 0;
                    OPLTimer_CalculateEndTime(&timer1);
                }

                if ((value & 0x20) == 0)
                {
                    timer1.enabled = (value & 0x02) != 0;
                    OPLTimer_CalculateEndTime(&timer2);
                }
            }

            break;

        default:
            Chip__WriteReg(&opl_chip, reg_num, value);
            break;
    }
}

static void OPL_SDL_PortWrite(opl_port_t port, unsigned int value)
{
    if (port == OPL_REGISTER_PORT)
    {
        register_num = value;
    }
    else if (port == OPL_DATA_PORT)
    {
        WriteRegister(register_num, value);
    }
}

static void OPL_SDL_SetCallback(unsigned int ms,
                                opl_callback_t callback,
                                void *data)
{
    SDL_LockMutex(callback_queue_mutex);
    OPL_Queue_Push(callback_queue, callback, data,
                   current_time - pause_offset + (ms * mixing_freq) / 1000);
    SDL_UnlockMutex(callback_queue_mutex);
}

static void OPL_SDL_ClearCallbacks(void)
{
    SDL_LockMutex(callback_queue_mutex);
    OPL_Queue_Clear(callback_queue);
    SDL_UnlockMutex(callback_queue_mutex);
}

static void OPL_SDL_Lock(void)
{
    SDL_LockMutex(callback_mutex);
}

static void OPL_SDL_Unlock(void)
{
    SDL_UnlockMutex(callback_mutex);
}

static void OPL_SDL_SetPaused(int paused)
{
    opl_sdl_paused = paused;
}

opl_driver_t opl_sdl_driver =
{
    "SDL",
    OPL_SDL_Init,
    OPL_SDL_Shutdown,
    OPL_SDL_PortRead,
    OPL_SDL_PortWrite,
    OPL_SDL_SetCallback,
    OPL_SDL_ClearCallbacks,
    OPL_SDL_Lock,
    OPL_SDL_Unlock,
    OPL_SDL_SetPaused
};

