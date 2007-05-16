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
#include "i_pcsound.h"
#include "i_sound.h"
#include "i_swap.h"
#include "deh_main.h"
#include "s_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"

#define NUM_CHANNELS		16

#define MAXMIDLENGTH        (96 * 1024)

static boolean nosfxparm;
static boolean nomusicparm;

static boolean sound_initialised = false;
static boolean music_initialised = false;

static Mix_Chunk sound_chunks[NUMSFX];
static int channels_playing[NUM_CHANNELS];

static int mixer_freq;
static Uint16 mixer_format;
static int mixer_channels;

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

static boolean ConvertibleRatio(int freq1, int freq2)
{
    int ratio;

    if (freq1 > freq2)
    {
        return ConvertibleRatio(freq2, freq1);
    }
    else if ((freq2 % freq1) != 0)
    {
        // Not in a direct ratio

        return false;
    }
    else
    {
        // Check the ratio is a power of 2

        ratio = freq2 / freq1;

        while ((ratio & 1) == 0)
        {
            ratio = ratio >> 1;
        }

        return ratio == 1;
    }
}

// Generic sound expansion function for any sample rate

static void ExpandSoundData(byte *data,
                            int samplerate,
                            int length,
                            Mix_Chunk *destination)
{
    SDL_AudioCVT convertor;
    
    if (ConvertibleRatio(samplerate, mixer_freq)
     && SDL_BuildAudioCVT(&convertor,
                          AUDIO_U8, 1, samplerate,
                          mixer_format, mixer_channels, mixer_freq))
    {
        convertor.buf = destination->abuf;
        convertor.len = length;
        memcpy(convertor.buf, data, length);

        SDL_ConvertAudio(&convertor);
    }
    else
    {
        Sint16 *expanded = (Sint16 *) destination->abuf;
        int expanded_length;
        int expand_ratio;
        int i;

        // Generic expansion if conversion does not work:
        //
        // SDL's audio conversion only works for rate conversions that are
        // powers of 2; if the two formats are not in a direct power of 2
        // ratio, do this naive conversion instead.

        // number of samples in the converted sound

        expanded_length = (length * mixer_freq) / samplerate;
        expand_ratio = (length << 8) / expanded_length;

        for (i=0; i<expanded_length; ++i)
        {
            Sint16 sample;
            int src;

            src = (i * expand_ratio) >> 8;

            sample = data[src] | (data[src] << 8);
            sample -= 32768;

            // expand 8->16 bits, mono->stereo

            expanded[i * 2] = expanded[i * 2 + 1] = sample;
        }
    }
}

// Load and convert a sound effect
// Returns true if successful

static boolean CacheSFX(int sound)
{
    int lumpnum;
    unsigned int lumplen;
    int samplerate;
    unsigned int length;
    unsigned int expanded_length;
    byte *data;

    // need to load the sound

    lumpnum = S_sfx[sound].lumpnum;
    data = W_CacheLumpNum(lumpnum, PU_STATIC);
    lumplen = W_LumpLength(lumpnum);

    // Check the header, and ensure this is a valid sound

    if (lumplen < 8
     || data[0] != 0x03 || data[1] != 0x00)
    {
        // Invalid sound

        return false;
    }

    // 16 bit sample rate field, 32 bit length field

    samplerate = (data[3] << 8) | data[2];
    length = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];

    // If the header specifies that the length of the sound is greater than
    // the length of the lump itself, this is an invalid sound lump

    if (length - 8 > lumplen)
    {
        return false;
    }

    expanded_length = (uint32_t) ((((uint64_t) length) * 4 * mixer_freq) / samplerate);

    sound_chunks[sound].allocated = 1;
    sound_chunks[sound].alen = expanded_length;
    sound_chunks[sound].abuf 
        = Z_Malloc(expanded_length, PU_STATIC, &sound_chunks[sound].abuf);
    sound_chunks[sound].volume = MIX_MAX_VOLUME;

    ExpandSoundData(data + 8, 
                    samplerate, 
                    length - 8, 
                    &sound_chunks[sound]);

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
    char *prefix;

    // Different prefix for PC speaker sound effects.

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        prefix = "dp";
    }
    else
    {
        prefix = "ds";
    }

    sprintf(namebuf, "%s%s", prefix, DEH_String(sfx->name));
    
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

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        return I_PCS_StartSound(id, channel, vol, sep, pitch, priority);
    }

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

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        I_PCS_StopSound(handle);
        return;
    }
    
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

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        return I_PCS_SoundIsPlaying(handle);
    }
    else
    {
        return Mix_Playing(handle);
    }
}




// 
// Periodically called to update the sound system
//

void I_UpdateSound( void )
{
    int i;

    if (!sound_initialised)
        return;

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        return;
    }

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

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        return;
    }

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

    sound_initialised = false;
    music_initialised = false;
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

    // If the SFX device is 0 (none), then disable sound effects,
    // just like if we specified -nosfx.  However, we still continue
    // with initialising digital sound output even if we are using
    // the PC speaker, because we might be using the SDL PC speaker
    // emulation.

    if (snd_sfxdevice == SNDDEVICE_NONE)
    {
        nosfxparm = true;
    }

    //!
    // Disable sound effects and music.
    //

    if (M_CheckParm("-nosound") > 0)
    {
        nosfxparm = true;
        nomusicparm = true;
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

    if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 1024) < 0)
    {
        fprintf(stderr, "Error initialising SDL_mixer: %s\n", Mix_GetError());
        return;
    }

    Mix_QuerySpec(&mixer_freq, &mixer_format, &mixer_channels);

    Mix_AllocateChannels(NUM_CHANNELS);
    
    SDL_PauseAudio(0);

    // If we are using the PC speaker, we now need to initialise it.

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        I_PCS_InitSound();
    }

    if (!nomusicparm)
        music_initialised = true;

    if (!nosfxparm)
        sound_initialised = true;
}




//
// MUSIC API.
//

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



