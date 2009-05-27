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

#include "SDL.h"
#include "SDL_mixer.h"

#include "fmopl.h"

#include "opl.h"
#include "opl_internal.h"

// TODO:
#define opl_sample_rate 22050

static FM_OPL *opl_emulator = NULL;
static int sdl_was_initialised = 0;
static int mixing_freq, mixing_channels;
static Uint16 mixing_format;

static int SDLIsInitialised(void)
{
    int freq, channels;
    Uint16 format;

    return Mix_QuerySpec(&freq, &format, &channels);
}

// Callback function to fill a new sound buffer:

static void OPL_Mix_Callback(void *udata, Uint8 *stream, int len)
{
}

static void OPL_SDL_Shutdown(void)
{
    if (sdl_was_initialised)
    {
        Mix_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        sdl_was_initialised = 0;
    }

    if (opl_emulator != NULL)
    {
        OPLDestroy(opl_emulator);
        opl_emulator = NULL;
    }
}

static int OPL_SDL_Init(unsigned int port_base)
{
    // Check if SDL_mixer has been opened already
    // If not, we must initialise it now

    if (!SDLIsInitialised())
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            fprintf(stderr, "Unable to set up sound.\n");
            return 0;
        }

        if (Mix_OpenAudio(opl_sample_rate, AUDIO_S16SYS, 2, 1024) < 0)
        {
            fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());

            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return 0;
        }

        SDL_PauseAudio(0);

        // When this module shuts down, it has the responsibility to 
        // shut down SDL.

        sdl_was_initialised = 1;
    }
    else
    {
        sdl_was_initialised = 0;
    }

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

    // Create the emulator structure:

    opl_emulator = makeAdlibOPL(mixing_freq);

    if (opl_emulator == NULL)
    {
        fprintf(stderr, "Failed to initialise software OPL emulator!\n");
        OPL_SDL_Shutdown();
        return 0;
    }

    // TODO: This should be music callback? or-?
    Mix_SetPostMix(OPL_Mix_Callback, NULL);

    return 1;
}

static unsigned int OPL_SDL_PortRead(opl_port_t port)
{
    if (opl_emulator != NULL)
    {
        return OPLRead(opl_emulator, port);
    }
    else
    {
        return 0;
    }
}

static void OPL_SDL_PortWrite(opl_port_t port, unsigned int value)
{
    if (opl_emulator != NULL)
    {
        OPLWrite(opl_emulator, port, value);
    }
}

opl_driver_t opl_sdl_driver =
{
    "SDL",
    OPL_SDL_Init,
    OPL_SDL_Shutdown,
    OPL_SDL_PortRead,
    OPL_SDL_PortWrite
};

