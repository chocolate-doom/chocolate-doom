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
//	System interface for sound.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include "memio.h"
#include "mus2mid.h"
#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "deh_main.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_swap.h"
#include "w_wad.h"

#include "doomdef.h"

#define NUM_CHANNELS		16

#define MAXMIDLENGTH        (96 * 1024)

enum 
{
    SNDDEVICE_NONE = 0,
    SNDDEVICE_PCSPEAKER = 1,
    SNDDEVICE_ADLIB = 2,
    SNDDEVICE_SB = 3,
    SNDDEVICE_PAS = 4,
    SNDDEVICE_GUS = 5,
    SNDDEVICE_WAVEBLASTER = 6,
    SNDDEVICE_SOUNDCANVAS = 7,
    SNDDEVICE_GENMIDI = 8,
    SNDDEVICE_AWE32 = 9,
};

extern int snd_sfxdevice;
extern int snd_musicdevice;

static boolean nosfxparm;
static boolean nomusicparm;

static boolean sound_initialised = false;
static boolean music_initialised = false;

static Mix_Chunk sound_chunks[NUMSFX];
static int channels_playing[NUM_CHANNELS];

// Disable music on OSX by default; there are problems with SDL_mixer.

#ifndef __MACOSX__
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_SB
#else
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_NONE
#endif

int snd_musicdevice = DEFAULT_MUSIC_DEVICE;
int snd_sfxdevice = SNDDEVICE_SB;

// When a sound stops, check if it is still playing.  If it is not, 
// we can mark the sound data as CACHE to be freed back for other
// means.

void ReleaseSoundOnChannel(int channel)
{
    int i;
    int id = channels_playing[channel];

    if (!id)
        return;

    channels_playing[channel] = sfx_None;
    
    for (i=0; i<NUM_CHANNELS; ++i)
    {
        // Playing on this channel? if so, don't release.

        if (channels_playing[i] == id)
            return;
    }

    // Not used on any channel, and can be safely released
    
    Z_ChangeTag(sound_chunks[id].abuf, PU_CACHE);
}

// Expands the 11025Hz, 8bit, mono sound effects in Doom to
// 22050Hz, 16bit stereo

static void ExpandSoundData(byte *data, int samplerate, int length,
                            Mix_Chunk *destination)
{
    byte *expanded = (byte *) destination->abuf;
    int expanded_length;
    int expand_ratio;
    int i;

    if (samplerate == 11025)
    {
        // Most of Doom's sound effects are 11025Hz

        // need to expand to 2 channels, 11025->22050 and 8->16 bit

        for (i=0; i<length; ++i)
        {
            Uint16 sample;

            sample = data[i] | (data[i] << 8);
            sample -= 32768;

            expanded[i * 8] = expanded[i * 8 + 2]
              = expanded[i * 8 + 4] = expanded[i * 8 + 6] = sample & 0xff;
            expanded[i * 8 + 1] = expanded[i * 8 + 3]
              = expanded[i * 8 + 5] = expanded[i * 8 + 7] = (sample >> 8) & 0xff;
        }
    }
    else if (samplerate == 22050)
    {
        for (i=0; i<length; ++i)
        {
            Uint16 sample;

            sample = data[i] | (data[i] << 8);
            sample -= 32768;

            expanded[i * 4] = expanded[i * 4 + 2] = sample & 0xff;
            expanded[i * 4 + 1] = expanded[i * 4 + 3] = (sample >> 8) & 0xff;
        }
    }
    else
    {
        // Generic expansion function for all other sample rates

        // number of samples in the converted sound

        expanded_length = (length * 22050) / samplerate;
        expand_ratio = (length << 8) / expanded_length;

        for (i=0; i<expanded_length; ++i)
        {
            Uint16 sample;
            int src;

            src = (i * expand_ratio) >> 8;

            sample = data[src] | (data[src] << 8);
            sample -= 32768;

            // expand 8->16 bits, mono->stereo

            expanded[i * 4] = expanded[i * 4 + 2] = sample & 0xff;
            expanded[i * 4 + 1] = expanded[i * 4 + 3] = (sample >> 8) & 0xff;
        }
    }
}

// Load and convert a sound effect
// Returns true if successful

static boolean CacheSFX(int sound)
{
    int lumpnum;
    int lumplen;
    int samplerate;
    int length;
    int expanded_length;
    byte *data;

    // need to load the sound

    lumpnum = I_GetSfxLumpNum(&S_sfx[sound]);
    data = W_CacheLumpNum(lumpnum, PU_STATIC);
    lumplen = W_LumpLength(lumpnum);

    // Check the header, and ensure this is a valid sound

    if (lumplen < 8
     || data[0] != 0x03 || data[1] != 0x00
     || data[6] != 0x00 || data[7] != 0x00)
    {
        // Invalid sound

        return false;
    }
    
    samplerate = (data[3] << 8) | data[2];
    length = (data[5] << 8) | data[4];

    // If the header specifies that the length of the sound is greater than
    // the length of the lump itself, this is an invalid sound lump

    if (length - 8 > lumplen)
    {
        return false;
    }

    expanded_length = (length * 22050) / (samplerate / 4);

    sound_chunks[sound].allocated = 1;
    sound_chunks[sound].alen = expanded_length;
    sound_chunks[sound].abuf 
        = Z_Malloc(expanded_length, PU_STATIC, &sound_chunks[sound].abuf);
    sound_chunks[sound].volume = MIX_MAX_VOLUME;

    ExpandSoundData(data + 8, samplerate, length, &sound_chunks[sound]);

    // don't need the original lump any more
  
    Z_ChangeTag(data, PU_CACHE);

    return true;
}

static Mix_Chunk *getsfx(int sound)
{
    if (sound_chunks[sound].abuf == NULL)
    {
        if (!CacheSFX(sound))
            return NULL;
    }
    else
    {
        // don't free the sound while it is playing!
   
        Z_ChangeTag(sound_chunks[sound].abuf, PU_STATIC);
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
    // Unused
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", DEH_String(sfx->name));
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
    Mix_Chunk *chunk;

    if (!sound_initialised)
        return 0;

    // Release a sound effect if there is already one playing
    // on this channel

    ReleaseSoundOnChannel(channel);

    // Get the sound data

    chunk = getsfx(id);

    if (chunk == NULL)
    {
        return -1;
    }

    // play sound

    Mix_PlayChannelTimed(channel, chunk, 0, -1);

    channels_playing[channel] = id;

    // set separation, etc.
 
    I_UpdateSoundParams(channel, vol, sep, pitch);

    return channel;
}

void I_StopSound (int handle)
{
    if (!sound_initialised)
        return;

    Mix_HaltChannel(handle);

    // Sound data is no longer needed; release the
    // sound data being used for this channel

    ReleaseSoundOnChannel(handle);
}


int I_SoundIsPlaying(int handle)
{
    if (!sound_initialised) 
        return false;

    if (handle < 0)
        return false;

    return Mix_Playing(handle);
}




// 
// Periodically called to update the sound system
//

void I_UpdateSound( void )
{
    int i;

    if (!sound_initialised)
        return;

    // Check all channels to see if a sound has finished

    for (i=0; i<NUM_CHANNELS; ++i)
    {
        if (channels_playing[i] && !I_SoundIsPlaying(i))
        {
            // Sound has finished playing on this channel,
            // but sound data has not been released to cache
            
            ReleaseSoundOnChannel(i);
        }
    }
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

    if (!sound_initialised)
        return;

    left = ((254 - sep) * vol) / 127;
    right = ((sep) * vol) / 127;

    Mix_SetPanning(handle, left, right);
}




void I_ShutdownSound(void)
{    
    if (!sound_initialised && !music_initialised)
        return;

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}



void
I_InitSound()
{ 
    int i;
    
    // No sounds yet

    for (i=0; i<NUMSFX; ++i)
    {
        sound_chunks[i].abuf = NULL;
    }

    for (i=0; i<NUM_CHANNELS; ++i)
    {
        channels_playing[i] = sfx_None;
    }

    //! 
    // Disable music playback.
    //

    nomusicparm = M_CheckParm("-nomusic") > 0;

    if (snd_musicdevice < SNDDEVICE_ADLIB)
    {
        nomusicparm = true;
    }

    //!
    // Disable sound effects.
    //

    nosfxparm = M_CheckParm("-nosfx") > 0;

    if (snd_sfxdevice < SNDDEVICE_SB)
    {
        nosfxparm = true;
    }

    // When trying to run with music enabled on OSX, display
    // a warning message.

#ifdef __MACOSX__
    if (!nomusicparm)
    {
        printf("\n"
               "                   *** WARNING ***\n"
               "      Music playback on OSX may cause crashes and\n"
               "      is disabled by default.\n"
               "\n");
    }
#endif

    //!
    // Disable sound effects and music.
    //

    if (M_CheckParm("-nosound") > 0)
    {
        nosfxparm = true;
        nomusicparm = true;
    }

    // If music or sound is going to play, we need to at least
    // initialise SDL
    // No sound in screensaver mode.

    if (screensaver_mode || (nomusicparm && nosfxparm))
        return;

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Unable to set up sound.\n");
        return;
    }

    if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0)
    {
        fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());
        return;
    }

    Mix_AllocateChannels(NUM_CHANNELS);
    
    SDL_PauseAudio(0);

    if (!nomusicparm)
        music_initialised = true;

    if (!nosfxparm)
        sound_initialised = true;
}




//
// MUSIC API.
//

void I_InitMusic(void)		
{ 
}

void I_ShutdownMusic(void)	
{ 
    music_initialised = false;
}

static boolean  musicpaused = false;
static int currentMusicVolume;

//
// SDL_mixer's native MIDI music playing does not pause properly.
// As a workaround, set the volume to 0 when paused.
//

static void UpdateMusicVolume(void)
{
    int vol;

    if (musicpaused)
        vol = 0;
    else
        vol = (currentMusicVolume * MIX_MAX_VOLUME) / 127;

    Mix_VolumeMusic(vol);
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    currentMusicVolume = volume;

    UpdateMusicVolume();
}

void I_PlaySong(void *handle, int looping)
{
    Mix_Music *music = (Mix_Music *) handle;
    int loops;

    if (!music_initialised)
        return;

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
    if (!music_initialised)
        return;

    musicpaused = true;

    UpdateMusicVolume();
}

void I_ResumeSong (void *handle)
{
    if (!music_initialised)
        return;

    musicpaused = false;

    UpdateMusicVolume();
}

void I_StopSong(void *handle)
{
    if (!music_initialised)
        return;

    Mix_HaltMusic();
}

void I_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (!music_initialised)
        return;

    if (handle == NULL)
        return;

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
        return NULL;
    
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
        return false;

    return Mix_PlayingMusic();
}



