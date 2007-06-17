// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
//	System interface for music.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"

#include "doomdef.h"
#include "memio.h"
#include "mus2mid.h"

#include "deh_main.h"
#include "m_misc.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXMIDLENGTH (96 * 1024)

static boolean music_initialised = false;

// If this is true, this module initialised SDL sound and has the 
// responsibility to shut it down

static boolean sdl_was_initialised = false;

static boolean musicpaused = false;
static int current_music_volume;

void I_ShutdownMusic(void)
{    
    if (music_initialised)
    {
        Mix_HaltMusic();
        music_initialised = false;

        if (sdl_was_initialised)
        {
            Mix_CloseAudio();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            sdl_was_initialised = false;
        }
    }
}

static boolean SDLIsInitialised(void)
{
    int freq, channels;
    Uint16 format;

    return Mix_QuerySpec(&freq, &format, &channels) != 0;
}

void I_InitMusic()
{ 
    // When trying to run with music enabled on OSX, display
    // a warning message.

#ifdef __MACOSX__
    printf("\n"
           "                   *** WARNING ***\n"
           "      Music playback on OSX may cause crashes and\n"
           "      is disabled by default.\n"
           "\n");
#endif
    
    // If SDL_mixer is not initialised, we have to initialise it 
    // and have the responsibility to shut it down later on.

    if (!SDLIsInitialised())
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            fprintf(stderr, "Unable to set up sound.\n");
            return;
        }

        if (Mix_OpenAudio(snd_samplerate, AUDIO_S16SYS, 2, 1024) < 0)
        {
            fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return;
        }

        SDL_PauseAudio(0);

        sdl_was_initialised = true;
    }

    music_initialised = true;
}

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//

static void UpdateMusicVolume(void)
{
    int vol;

    if (musicpaused)
    {
        vol = 0;
    }
    else
    {
        vol = (current_music_volume * MIX_MAX_VOLUME) / 127;
    }

    Mix_VolumeMusic(vol);
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;

    UpdateMusicVolume();
}

void I_PlaySong(void *handle, int looping)
{
    Mix_Music *music = (Mix_Music *) handle;
    int loops;

    if (!music_initialised)
    {
        return;
    }

    if (handle == NULL)
    {
        return;
    }

    if (looping)
    {
        loops = -1;
    }
    else
    {
        loops = 1;
    }

    Mix_PlayMusic(music, loops);
}

void I_PauseSong (void *handle)
{
    if (!music_initialised)
    {
        return;
    }

    musicpaused = true;

    UpdateMusicVolume();
}

void I_ResumeSong (void *handle)
{
    if (!music_initialised)
    {
        return;
    }

    musicpaused = false;

    UpdateMusicVolume();
}

void I_StopSong(void *handle)
{
    if (!music_initialised)
    {
        return;
    }

    Mix_HaltMusic();
}

void I_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (!music_initialised)
    {
        return;
    }

    if (handle == NULL)
    {
        return;
    }

    Mix_FreeMusic(music);
}

// Determine whether memory block is a .mid file 

static boolean IsMid(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MThd", 4);
}

static boolean ConvertMus(byte *musdata, int len, char *filename)
{
    MEMFILE *instream;
    MEMFILE *outstream;
    void *outbuf;
    size_t outbuf_len;
    int result;

    instream = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();

    result = mus2mid(instream, outstream);

    if (result == 0)
    {
        mem_get_buf(outstream, &outbuf, &outbuf_len);

        M_WriteFile(filename, outbuf, outbuf_len);
    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return result;
}

void *I_RegisterSong(void *data, int len)
{
    char *filename;
    Mix_Music *music;

    if (!music_initialised)
    {
        return NULL;
    }
    
    // MUS files begin with "MUS"
    // Reject anything which doesnt have this signature
    
    filename = M_TempFile("doom.mid");

    if (IsMid(data, len) && len < MAXMIDLENGTH)
    {
        M_WriteFile(filename, data, len);
    }
    else 
    {
	// Assume a MUS file and try to convert

        ConvertMus(data, len, filename);
    }

    // Load the MIDI

    music = Mix_LoadMUS(filename);
    
    if (music == NULL)
    {
        // Failed to load

        fprintf(stderr, "Error loading midi: %s\n", Mix_GetError());
    }

    // remove file now

    remove(filename);

    Z_Free(filename);

    return music;
}

// Is the song playing?
boolean I_QrySongPlaying(void *handle)
{
    if (!music_initialised)
    {
        return false;
    }

    return Mix_PlayingMusic();
}



