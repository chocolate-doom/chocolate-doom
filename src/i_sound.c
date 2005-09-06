// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_sound.c 77 2005-09-06 21:11:23Z fraggle $
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
// $Log$
// Revision 1.14  2005/09/06 21:11:23  fraggle
// Working music!
//
// Revision 1.13  2005/09/05 22:50:56  fraggle
// Add mmus2mid code from prboom.  Use 'void *' for music handles.  Pass
// length of data when registering music.
//
// Revision 1.12  2005/09/05 21:03:43  fraggle
// 16-bit sound
//
// Revision 1.11  2005/09/05 20:32:18  fraggle
// Use the system-nonspecific sound code to assign the channel number used
// by SDL.  Remove handle tagging stuff.
//
// Revision 1.10  2005/08/19 21:55:51  fraggle
// Make sounds louder.  Use the correct maximum of 15 when doing sound
// calculations.
//
// Revision 1.9  2005/08/07 19:21:01  fraggle
// Cycle round sound channels to stop reuse and conflicts of channel
// numbers.  Add debug to detect when incorrect sound handles are used.
//
// Revision 1.8  2005/08/06 17:05:51  fraggle
// Remove debug messages, send error messages to stderr
// Fix overflow when playing large sound files
//
// Revision 1.7  2005/08/05 17:53:07  fraggle
// More sensible defaults
//
// Revision 1.6  2005/08/04 21:48:32  fraggle
// Turn on compiler optimisation and warning options
// Add SDL_mixer sound code
//
// Revision 1.5  2005/07/23 21:32:47  fraggle
// Add missing errno.h, fix crash on startup when no IWAD present
//
// Revision 1.4  2005/07/23 19:17:11  fraggle
// Use ANSI-standard limit constants.  Remove LINUX define.
//
// Revision 1.3  2005/07/23 17:21:35  fraggle
// Remove step table (unused, adds dependency on pow function)
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:46  fraggle
// Initial import
//
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_sound.c 77 2005-09-06 21:11:23Z fraggle $";

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "mmus2mid.h"
#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_swap.h"
#include "w_wad.h"

#include "doomdef.h"

#define NUM_CHANNELS		16

static int sound_initialised = 0;
static Mix_Chunk sound_chunks[NUMSFX];

static byte *expand_sound_data(byte *data, int samplerate, int length)
{
    byte *result = data;
    int i;

    if (samplerate == 11025)
    {
        // need to expand to 2 channels, 11025->22050 and 8->16 bit

        result = Z_Malloc(length * 8, PU_STATIC, NULL);

        for (i=0; i<length; ++i)
        {
            Uint16 sample;

            sample = data[i] | (data[i] << 8);
            sample -= 32768;

            result[i * 8] = result[i * 8 + 2]
              = result[i * 8 + 4] = result[i * 8 + 6] = sample & 0xff;
            result[i * 8 + 1] = result[i * 8 + 3]
              = result[i * 8 + 5] = result[i * 8 + 7] = (sample >> 8) & 0xff;
        }
    }
    else if (samplerate == 22050)
    {
        // need to expand to 2 channels (sample rate is already correct)

        result = Z_Malloc(length * 4, PU_STATIC, NULL);

        for (i=0; i<length; ++i)
        {
            Uint16 sample;

            sample = data[i] | (data[i] << 8);
            sample -= 32768;

            result[i * 4] = result[i * 4 + 2] = sample & 0xff;
            result[i * 4 + 1] = result[i * 4 + 3] = (sample >> 8) & 0xff;
        }
    }
    else
    {
        I_Error("Unsupported sample rate %i", samplerate);
    }

    return result;
}

static Mix_Chunk *getsfx(int sound)
{
    if (sound_chunks[sound].abuf == NULL)
    {
        int lumpnum;
        int samplerate;
        int length;
        byte *data;

        // need to load the sound

        lumpnum = I_GetSfxLumpNum(&S_sfx[sound]);
        data = W_CacheLumpNum(lumpnum, PU_STATIC);

        samplerate = (data[3] << 8) | data[2];
        length = (data[5] << 8) | data[4];

        sound_chunks[sound].allocated = 1;
        sound_chunks[sound].abuf = expand_sound_data(data + 8, samplerate, length);
        sound_chunks[sound].alen = (length * 4)  * (22050 / samplerate);
        sound_chunks[sound].volume = 64;
    }

    return &sound_chunks[sound];
}

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels()
{
}	

 
void I_SetSfxVolume(int volume)
{
  // Identical to DOS.
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
  snd_SfxVolume = volume;
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
  // Internal state variable.
  snd_MusicVolume = volume;
  // Now set volume on output device.
  // Whatever( snd_MusciVolume );
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int
I_StartSound
( int		id,
  int           channel,
  int		vol,
  int		sep,
  int		pitch,
  int		priority )
{
    Mix_Chunk *chunk = getsfx(id);

    // play sound

    Mix_PlayChannelTimed(channel, chunk, 0, -1);

    // set separation, etc.
 
    I_UpdateSoundParams(channel, vol, sep, pitch);

    return channel;
}



void I_StopSound (int handle)
{
    Mix_HaltChannel(handle);
}


int I_SoundIsPlaying(int handle)
{
    return Mix_Playing(handle);
}




//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
// This function currently supports only 16bit.
//
void I_UpdateSound( void )
{
}


// 
// This would be used to write out the mixbuffer
//  during each game loop update.
// Updates sound buffer and audio device at runtime. 
// It is called during Timer interrupt with SNDINTR.
// Mixing now done synchronous, and
//  only output be done asynchronous?
//
void
I_SubmitSound(void)
{
}



void
I_UpdateSoundParams
( int	handle,
  int	vol,
  int	sep,
  int	pitch)
{
    int left, right;

    left = ((254 - sep) * vol) / 15;
    right = ((sep) * vol) / 15;

    Mix_SetPanning(handle, left, right);
}




void I_ShutdownSound(void)
{    
    if (!sound_initialised)
        return;

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}






void
I_InitSound()
{ 
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Unable to set up sound.\n");
        return;
    }

    if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0)
    {
        fprintf(stderr, "Error initialising SDL_mixer: %s\n", SDL_GetError());
    }

    Mix_AllocateChannels(NUM_CHANNELS);
    
    sound_initialised = 1;

    SDL_PauseAudio(0);
}




//
// MUSIC API.
// Still no music done.
// Remains. Dummies.
//
void I_InitMusic(void)		{ }
void I_ShutdownMusic(void)	{ }

static int	looping=0;
static int	musicdies=-1;

void I_PlaySong(void *handle, int looping)
{
    Mix_Music *music = (Mix_Music *) handle;
    int loops;

    if (handle == NULL)
        return;

    if (looping)
        loops = -1;
    else
        loops = 1;

    Mix_PlayMusic(music, loops);
}

void I_PauseSong (void *handle)
{
    Mix_PauseMusic();
}

void I_ResumeSong (void *handle)
{
    Mix_ResumeMusic();
}

void I_StopSong(void *handle)
{
    Mix_HaltMusic();
}

void I_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (handle == NULL)
        return;

    Mix_FreeMusic(music);
}

void *I_RegisterSong(void *data, int len)
{
    char filename[64];
    Mix_Music *music;
    MIDI *mididata;
    UBYTE *mid;
    int midlen;
    
#ifdef _WIN32
    sprintf(filename, "doom.mid");
#else
    sprintf(filename, "/tmp/doom-%i.mid", getpid());
#endif

    // Convert from mus to midi
    // Bits here came from PrBoom
  
    mididata = Z_Malloc(sizeof(MIDI), PU_STATIC, 0);
    mmus2mid(data, mididata, 89, 0);

    if (MIDIToMidi(mididata, &mid, &midlen))
    {
        // Error occurred

        fprintf(stderr, "Error converting MUS lump.\n");

        music = NULL;
    }
    else
    {
        // Write midi data to disk
       
        M_WriteFile(filename, mid, midlen);

        // Clean up
       
        free(mid);
        free_mididata(mididata);
        music = Mix_LoadMUS(filename);
        
        if (music == NULL)
        {
            // Failed to load

            fprintf(stderr, "Error loading midi\n");
        }
    }

    Z_Free(mididata);
    
    return music;
}

// Is the song playing?
int I_QrySongPlaying(void *handle)
{
    return Mix_PlayingMusic();
}



