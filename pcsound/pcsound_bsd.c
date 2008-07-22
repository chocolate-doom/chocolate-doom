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
//    PC speaker driver for [Open]BSD 
//    (Should be NetBSD as well, but untested).
//
//-----------------------------------------------------------------------------

#include "config.h"

// OpenBSD/NetBSD:

#ifdef HAVE_DEV_ISA_SPKRIO_H
#define HAVE_BSD_SPEAKER
#include <dev/isa/spkrio.h>
#endif

// FreeBSD

#ifdef HAVE_DEV_SPEAKER_SPEAKER_H
#define HAVE_BSD_SPEAKER
#include <dev/speaker/speaker.h>
#endif

#ifdef HAVE_BSD_SPEAKER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "pcsound.h"
#include "pcsound_internal.h"

#define SPEAKER_DEVICE "/dev/speaker"

//
// This driver is far more complicated than it should be, because 
// OpenBSD has sucky support for threads.  Because multithreading
// is done in userspace, invoking the ioctl to make the speaker
// beep will lock all threads until the beep has completed.  
// 
// Thus, to get the beeping to occur in real-time, we must invoke
// the ioctl in a separate process.  To do this, a separate 
// sound server is forked that listens on a socket for tones to
// play.  When a tone is received, a reply is sent back to the
// main process and the tone played.
//
// Meanwhile, back in the main process, there is a sound thread
// that runs, invoking the pcsound callback function to get 
// more tones.  This blocks on the sound server socket, waiting
// for replies.  In this way, when the sound server finishes 
// playing a tone, the next one is sent.
//
// This driver is a bit less accurate than the others, because
// we can only specify sound durations in 1/100ths of a second,
// as opposed to the normal millisecond durations.

static pcsound_callback_func callback;
static int sound_server_pid;
static int sleep_adjust = 0;
static int sound_thread_running;
static SDL_Thread *sound_thread_handle;
static int sound_server_pipe[2];

// Play a sound, checking how long the system call takes to complete
// and autoadjusting for drift.

static void AdjustedBeep(int speaker_handle, int ms, int freq)
{
    unsigned int start_time;
    unsigned int end_time;
    unsigned int actual_time;
    tone_t tone;

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

    // Invoke the system call and time how long it takes

    start_time = SDL_GetTicks();

    tone.duration = ms / 10;        // in 100ths of a second
    tone.frequency = freq;

    // Always a positive duration

    if (tone.duration < 1)
    {
        tone.duration = 1;
    }

    if (ioctl(speaker_handle, SPKRTONE, &tone) != 0)
    {
        perror("ioctl");
        return;
    }
    
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

static void SoundServer(int speaker_handle)
{
    tone_t tone;
    int result;

    // Run in a loop, invoking the callback

    for (;;)
    {
        result = read(sound_server_pipe[1], &tone, sizeof(tone_t));

        if (result < 0)
        {
            perror("read");
            return;
        }

        // Send back a response, so the main process knows to send another

        write(sound_server_pipe[1], &tone, sizeof(tone_t));

        // Beep! (blocks until complete)

        AdjustedBeep(speaker_handle, tone.duration, tone.frequency);
    }
}

// Start up the sound server.  Returns non-zero if successful.

static int StartSoundServer(void)
{
    int result;
    int speaker_handle;

    // Try to open the speaker device

    speaker_handle = open(SPEAKER_DEVICE, O_WRONLY);

    if (speaker_handle == -1)
    {
        // Don't have permissions for the console device?

	fprintf(stderr, "StartSoundServer: Failed to open '%s': %s\n",
                        SPEAKER_DEVICE, strerror(errno));
        return 0;
    }

    // Create a pipe for communications

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sound_server_pipe) < 0)
    {
        perror("socketpair");
        close(speaker_handle);
        return 0;
    }

    // Start a separate process to generate PC speaker output
    // We can't use the SDL threading functions because OpenBSD's
    // threading sucks :-(
    
    result = fork();

    if (result < 0)
    {
        fprintf(stderr, "Failed to fork sound server!\n");
        close(speaker_handle);
        return 0;
    }
    else if (result == 0)
    {
        // This is the child (sound server)

        SoundServer(speaker_handle);
        close(speaker_handle);

        exit(0);
    }
    else
    {
        // This is the parent

        sound_server_pid = result;
    }

    return 1;
}

static void StopSoundServer(void)
{
    int status;

    kill(sound_server_pid, SIGINT);
    waitpid(sound_server_pid, &status, 0);
}

static int SoundThread(void *unused)
{
    tone_t tone;
    int duration;
    int frequency;

    while (sound_thread_running)
    {
        // Get the next frequency to play

        callback(&duration, &frequency);

//printf("dur: %i, freq: %i\n", duration, frequency);

        // Build up a tone structure and send to the sound server

        tone.frequency = frequency;
        tone.duration = duration;

        if (write(sound_server_pipe[0], &tone, sizeof(tone_t)) < 0) 
        {
            perror("write");
            break;
        }

        // Wait until the sound server responds before sending another

        if (read(sound_server_pipe[0], &tone, sizeof(tone_t)) < 0)
        {
            perror("read");
            break;
        }
    }

    return 0;
}

static int PCSound_BSD_Init(pcsound_callback_func callback_func)
{
    callback = callback_func;

    if (!StartSoundServer())
    {
        fprintf(stderr, "PCSound_BSD_Init: Failed to start sound server.\n");
        return 0;
    }

    sound_thread_running = 1;
    sound_thread_handle = SDL_CreateThread(SoundThread, NULL);

    return 1;
}

static void PCSound_BSD_Shutdown(void)
{
    // Stop the sound thread

    sound_thread_running = 0;

    SDL_WaitThread(sound_thread_handle, NULL);

    // Stop the sound server

    StopSoundServer();
}

pcsound_driver_t pcsound_bsd_driver =
{
    "BSD",
    PCSound_BSD_Init,
    PCSound_BSD_Shutdown,
};

#endif /* #ifdef HAVE_BSD_SPEAKER */

