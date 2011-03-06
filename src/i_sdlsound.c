// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
// Copyright(C) 2008 David Flater
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "SDL.h"
#include "SDL_mixer.h"

#ifdef HAVE_LIBSAMPLERATE
#include <samplerate.h>
#endif

#include "deh_main.h"
#include "i_system.h"
#include "i_swap.h"
#include "s_sound.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomdef.h"

#define LOW_PASS_FILTER
//#define DEBUG_DUMP_WAVS
#define MAX_SOUND_SLICE_TIME 70 /* ms */
#define NUM_CHANNELS 16

static boolean setpanning_workaround = false;

static boolean sound_initialized = false;

static Mix_Chunk sound_chunks[NUMSFX];
static int channels_playing[NUM_CHANNELS];

static int mixer_freq;
static Uint16 mixer_format;
static int mixer_channels;

int use_libsamplerate = 0;

// When a sound stops, check if it is still playing.  If it is not, 
// we can mark the sound data as CACHE to be freed back for other
// means.

static void ReleaseSoundOnChannel(int channel)
{
    int i;
    int id = channels_playing[channel];

    if (!id)
    {
        return;
    }

    channels_playing[channel] = sfx_None;
    
#ifdef HAVE_LIBSAMPLERATE
    // Don't allow precached sounds to be swapped out.
    if (use_libsamplerate)
        return;
#endif

    for (i=0; i<NUM_CHANNELS; ++i)
    {
        // Playing on this channel? if so, don't release.

        if (channels_playing[i] == id)
            return;
    }

    // Not used on any channel, and can be safely released
    
    Z_ChangeTag(sound_chunks[id].abuf, PU_CACHE);
}


#ifdef HAVE_LIBSAMPLERATE

// Returns the conversion mode for libsamplerate to use.

static int SRC_ConversionMode(void)
{
    switch (use_libsamplerate)
    {
        // 0 = disabled

        default:
        case 0:
            return -1;

        // Ascending numbers give higher quality

        case 1:
            return SRC_LINEAR;
        case 2:
            return SRC_ZERO_ORDER_HOLD;
        case 3:
            return SRC_SINC_FASTEST;
        case 4:
            return SRC_SINC_MEDIUM_QUALITY;
        case 5:
            return SRC_SINC_BEST_QUALITY;
    }
}

#endif

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

#ifdef DEBUG_DUMP_WAVS

// Debug code to dump resampled sound effects to WAV files for analysis.

static void WriteWAV(char *filename, byte *data,
                     uint32_t length, int samplerate)
{
    FILE *wav;
    unsigned int i;
    unsigned short s;

    wav = fopen(filename, "wb");

    // Header

    fwrite("RIFF", 1, 4, wav);
    i = LONG(36 + samplerate);
    fwrite(&i, 4, 1, wav);
    fwrite("WAVE", 1, 4, wav);

    // Subchunk 1

    fwrite("fmt ", 1, 4, wav);
    i = LONG(16);
    fwrite(&i, 4, 1, wav);           // Length
    s = SHORT(1);
    fwrite(&s, 2, 1, wav);           // Format (PCM)
    s = SHORT(2);
    fwrite(&s, 2, 1, wav);           // Channels (2=stereo)
    i = LONG(samplerate);
    fwrite(&i, 4, 1, wav);           // Sample rate
    i = LONG(samplerate * 2 * 2);
    fwrite(&i, 4, 1, wav);           // Byte rate (samplerate * stereo * 16 bit)
    s = SHORT(2 * 2);
    fwrite(&s, 2, 1, wav);           // Block align (stereo * 16 bit)
    s = SHORT(16);
    fwrite(&s, 2, 1, wav);           // Bits per sample (16 bit)

    // Data subchunk

    fwrite("data", 1, 4, wav);
    i = LONG(length);
    fwrite(&i, 4, 1, wav);           // Data length
    fwrite(data, 1, length, wav);    // Data

    fclose(wav);
}

#endif

// Generic sound expansion function for any sample rate.

static void ExpandSoundData_SDL(byte *data,
                                int samplerate,
                                uint32_t length,
                                Mix_Chunk *destination)
{
    SDL_AudioCVT convertor;
    uint32_t expanded_length;
 
    // Calculate the length of the expanded version of the sample.    

    expanded_length = (uint32_t) ((((uint64_t) length) * mixer_freq) / samplerate);

    // Double up twice: 8 -> 16 bit and mono -> stereo

    expanded_length *= 4;
    destination->alen = expanded_length;
    destination->abuf 
        = Z_Malloc(expanded_length, PU_STATIC, &destination->abuf);

    // If we can, use the standard / optimized SDL conversion routines.

    if (samplerate <= mixer_freq
     && ConvertibleRatio(samplerate, mixer_freq)
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

        expanded_length = ((uint64_t) length * mixer_freq) / samplerate;
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

#ifdef LOW_PASS_FILTER
        // Perform a low-pass filter on the upscaled sound to filter
        // out high-frequency noise from the conversion process.

        {
            float rc, dt, alpha;

            // Low-pass filter for cutoff frequency f:
            //
            // For sampling rate r, dt = 1 / r
            // rc = 1 / 2*pi*f
            // alpha = dt / (rc + dt)

            // Filter to the half sample rate of the original sound effect
            // (maximum frequency, by nyquist)

            dt = 1.0f / mixer_freq;
            rc = 1.0f / (3.14f * samplerate);
            alpha = dt / (rc + dt);

            // Both channels are processed in parallel, hence [i-2]:

            for (i=2; i<expanded_length * 2; ++i)
            {
                expanded[i] = (Sint16) (alpha * expanded[i]
                                      + (1 - alpha) * expanded[i-2]);
            }
        }
#endif /* #ifdef LOW_PASS_FILTER */
    }
}


// Load and validate a sound effect lump.
// Preconditions:
//     S_sfx[sound].lumpnum has been set
// Postconditions if sound is valid:
//     returns true
//     starred parameters are set, with data_ref pointing to start of sound
//     caller is responsible for releasing the identified lump
// Postconditions if sound is invalid:
//     returns false
//     starred parameters are garbage
//     lump already released

static boolean LoadSoundLump(int sound,
			     int *lumpnum,
			     int *samplerate,
			     uint32_t *length,
			     byte **data_ref)
{
    int lumplen;
    byte *data;

    // Load the sound

    *lumpnum    = S_sfx[sound].lumpnum;
    *data_ref   = W_CacheLumpNum(*lumpnum, PU_STATIC);
    lumplen = W_LumpLength(*lumpnum);
    data  = *data_ref;

    // Ensure this is a valid sound

    if (lumplen < 8 || data[0] != 0x03 || data[1] != 0x00)
    {
	// Invalid sound
	W_ReleaseLumpNum(*lumpnum);
	return false;
    }

    // 16 bit sample rate field, 32 bit length field

    *samplerate = (data[3] << 8) | data[2];
    *length = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];

    // If the header specifies that the length of the sound is
    // greater than the length of the lump itself, this is an invalid
    // sound lump.

    // We also discard sound lumps that are less than 49 samples long,
    // as this is how DMX behaves - although the actual cut-off length
    // seems to vary slightly depending on the sample rate.  This needs
    // further investigation to better understand the correct
    // behavior.

    if (*length > lumplen - 8 || *length <= 48)
    {
	W_ReleaseLumpNum(*lumpnum);
	return false;
    }

    // Prune header
    *data_ref += 8;

    // The DMX sound library seems to skip the first 16 and last 16
    // bytes of the lump - reason unknown.

    *data_ref += 16;
    *length -= 32;

    return true;
}


// Load and convert a sound effect
// Returns true if successful

static boolean CacheSFX_SDL(int sound)
{
    int lumpnum;
    int samplerate;
    uint32_t length;
    byte *data;

#ifdef HAVE_LIBSAMPLERATE
    assert(!use_libsamplerate); // Should be using I_PrecacheSounds_SRC instead
#endif

    if (!LoadSoundLump(sound, &lumpnum, &samplerate, &length, &data))
        return false;

    // Sample rate conversion
    // sound_chunks[sound].alen and abuf are determined by ExpandSoundData.

    sound_chunks[sound].allocated = 1;
    sound_chunks[sound].volume = MIX_MAX_VOLUME;

    ExpandSoundData_SDL(data,
			samplerate, 
			length, 
			&sound_chunks[sound]);

#ifdef DEBUG_DUMP_WAVS
    {
        char filename[16];

        sprintf(filename, "%s.wav", DEH_String(S_sfx[sound].name));
        WriteWAV(filename, sound_chunks[sound].abuf,
                 sound_chunks[sound].alen, mixer_freq);
    }
#endif

    // don't need the original lump any more
  
    W_ReleaseLumpNum(lumpnum);

    return true;
}


#ifdef HAVE_LIBSAMPLERATE

// Preload and resample all sound effects with libsamplerate.

static void I_PrecacheSounds_SRC(void)
{
    char namebuf[9];
    uint32_t sound_i, sample_i;
    boolean good_sound[NUMSFX];
    float *resampled_sound[NUMSFX];
    uint32_t resampled_sound_length[NUMSFX];
    float norm_factor;
    float max_amp = 0;
    unsigned int zone_size;

    assert(use_libsamplerate);

    zone_size = Z_ZoneSize();

    if (zone_size < 32 * 1024 * 1024)
    {
        fprintf(stderr,
                "WARNING: low memory.  Heap size is only %d MiB.\n"
                "WARNING: use_libsamplerate needs more heap!\n"
                "WARNING: put -mb 64 on the command line to avoid "
                "\"Error: Z_Malloc: failed on allocation of X bytes\" !\n",
                zone_size / (1024 * 1024));
    }

    printf("I_PrecacheSounds_SRC: Precaching all sound effects..");

    // Pass 1:  resample all sounds and determine maximum amplitude.

    for (sound_i=sfx_pistol; sound_i<NUMSFX; ++sound_i)
    {
        good_sound[sound_i] = false;

        if ((sound_i % 6) == 0)
        {
            printf(".");
            fflush(stdout);
        }

        sprintf(namebuf, "ds%s", DEH_String(S_sfx[sound_i].name));
        S_sfx[sound_i].lumpnum = W_CheckNumForName(namebuf);
        if (S_sfx[sound_i].lumpnum != -1)
        {
	    int lumpnum;
	    int samplerate;
	    uint32_t length;
	    byte *data;
            double of_temp;
            int retn;
            float *rsound;
            uint32_t rlen;
	    SRC_DATA src_data;

	    if (!LoadSoundLump(sound_i, &lumpnum, &samplerate, &length, &data))
		continue;

            assert(length <= LONG_MAX);
	    src_data.input_frames = length;
	    src_data.data_in = malloc(length * sizeof(float));
	    src_data.src_ratio = (double)mixer_freq / samplerate;

	    // mixer_freq / 4 adds a quarter-second safety margin.

            of_temp = src_data.src_ratio * length + (mixer_freq / 4);
            assert(of_temp <= LONG_MAX);
	    src_data.output_frames = of_temp;
	    src_data.data_out = malloc(src_data.output_frames * sizeof(float));
	    assert(src_data.data_in != NULL && src_data.data_out != NULL);

	    // Convert input data to floats

	    for (sample_i=0; sample_i<length; ++sample_i)
	    {
		// Unclear whether 128 should be interpreted as "zero" or
		// whether a symmetrical range should be assumed.  The
		// following assumes a symmetrical range.

		src_data.data_in[sample_i] = data[sample_i] / 127.5 - 1;
	    }

            // don't need the original lump any more

            W_ReleaseLumpNum(lumpnum);

	    // Resample

	    retn = src_simple(&src_data, SRC_ConversionMode(), 1);
	    assert(retn == 0);
            assert(src_data.output_frames_gen > 0);
            resampled_sound[sound_i] = src_data.data_out;
            resampled_sound_length[sound_i] = src_data.output_frames_gen;
            free(src_data.data_in);
            good_sound[sound_i] = true;

            // Track maximum amplitude for later normalization

            rsound = resampled_sound[sound_i];
            rlen = resampled_sound_length[sound_i];
	    for (sample_i=0; sample_i<rlen; ++sample_i)
	    {
	        float fabs_amp = fabsf(rsound[sample_i]);
                if (fabs_amp > max_amp)
  		    max_amp = fabs_amp;
            }
        }
    }

    // Pass 2:  normalize and convert to signed 16-bit stereo.

    if (max_amp <= 0)
        max_amp = 1;
    norm_factor = INT16_MAX / max_amp;

    for (sound_i=sfx_pistol; sound_i<NUMSFX; ++sound_i)
    {
        if (good_sound[sound_i])
        {
            uint32_t rlen = resampled_sound_length[sound_i];
            int16_t *expanded;
            uint32_t abuf_index;
            float *rsound;

            sound_chunks[sound_i].allocated = 1;
            sound_chunks[sound_i].volume = MIX_MAX_VOLUME;
            sound_chunks[sound_i].alen = rlen * 4;
            sound_chunks[sound_i].abuf = Z_Malloc(sound_chunks[sound_i].alen,
                                                  PU_STATIC,
                                                  &sound_chunks[sound_i].abuf);
            expanded = (int16_t *) sound_chunks[sound_i].abuf;
            abuf_index=0;

            rsound = resampled_sound[sound_i];
            for (sample_i=0; sample_i<rlen; ++sample_i)
            {
                float   cvtval_f = norm_factor * rsound[sample_i];
                int16_t cvtval_i = cvtval_f + (cvtval_f < 0 ? -0.5 : 0.5);

                // Left and right channels

                expanded[abuf_index++] = cvtval_i;
                expanded[abuf_index++] = cvtval_i;
            }
            free(rsound);
        }
    }

    printf(" norm factor = %f\n", norm_factor);
}

#endif


static Mix_Chunk *GetSFXChunk(int sound_id)
{
    if (sound_chunks[sound_id].abuf == NULL)
    {
#ifdef HAVE_LIBSAMPLERATE
        if (use_libsamplerate != 0)
            return NULL;   /* If valid, it should have been precached */
#endif
        if (!CacheSFX_SDL(sound_id))
            return NULL;
    }
    else
    {
        // don't free the sound while it is playing!
   
        Z_ChangeTag(sound_chunks[sound_id].abuf, PU_STATIC);
    }

    return &sound_chunks[sound_id];
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//

static int I_SDL_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];

    sprintf(namebuf, "ds%s", DEH_String(sfx->name));
    
    return W_GetNumForName(namebuf);
}

static void I_SDL_UpdateSoundParams(int handle, int vol, int sep)
{
    int left, right;

    if (!sound_initialized)
    {
        return;
    }

    left = ((254 - sep) * vol) / 127;
    right = ((sep) * vol) / 127;

    // SDL_mixer version 1.2.8 and earlier has a bug in the Mix_SetPanning
    // function.  A workaround is to call Mix_UnregisterAllEffects for
    // the channel before calling it.  This is undesirable as it may lead
    // to the channel volumes resetting briefly.

    if (setpanning_workaround)
    {
        Mix_UnregisterAllEffects(handle);
    }

    Mix_SetPanning(handle, left, right);
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

static int I_SDL_StartSound(int id, int channel, int vol, int sep)
{
    Mix_Chunk *chunk;

    if (!sound_initialized)
    {
        return -1;
    }

    // Release a sound effect if there is already one playing
    // on this channel

    ReleaseSoundOnChannel(channel);

    // Get the sound data

    chunk = GetSFXChunk(id);

    if (chunk == NULL)
    {
        return -1;
    }

    // play sound

    Mix_PlayChannelTimed(channel, chunk, 0, -1);

    channels_playing[channel] = id;

    // set separation, etc.
 
    I_SDL_UpdateSoundParams(channel, vol, sep);

    return channel;
}

static void I_SDL_StopSound (int handle)
{
    if (!sound_initialized)
    {
        return;
    }

    Mix_HaltChannel(handle);

    // Sound data is no longer needed; release the
    // sound data being used for this channel

    ReleaseSoundOnChannel(handle);
}


static boolean I_SDL_SoundIsPlaying(int handle)
{
    if (handle < 0)
    {
        return false;
    }

    return Mix_Playing(handle);
}

// 
// Periodically called to update the sound system
//

static void I_SDL_UpdateSound(void)
{
    int i;

    // Check all channels to see if a sound has finished

    for (i=0; i<NUM_CHANNELS; ++i)
    {
        if (channels_playing[i] && !I_SDL_SoundIsPlaying(i))
        {
            // Sound has finished playing on this channel,
            // but sound data has not been released to cache
            
            ReleaseSoundOnChannel(i);
        }
    }
}

static void I_SDL_ShutdownSound(void)
{    
    if (!sound_initialized)
    {
        return;
    }

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    sound_initialized = false;
}

// Calculate slice size, based on MAX_SOUND_SLICE_TIME.
// The result must be a power of two.

static int GetSliceSize(void)
{
    int limit;
    int n;

    limit = (snd_samplerate * MAX_SOUND_SLICE_TIME) / 1000;

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

static boolean I_SDL_InitSound(void)
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

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Unable to set up sound.\n");
        return false;
    }

    if (Mix_OpenAudio(snd_samplerate, AUDIO_S16SYS, 2, GetSliceSize()) < 0)
    {
        fprintf(stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
        return false;
    }

    Mix_QuerySpec(&mixer_freq, &mixer_format, &mixer_channels);

#ifdef HAVE_LIBSAMPLERATE
    if (use_libsamplerate != 0)
    {
        if (SRC_ConversionMode() < 0)
        {
            I_Error("I_SDL_InitSound: Invalid value for use_libsamplerate: %i",
                    use_libsamplerate);
        }

        I_PrecacheSounds_SRC();
    }
#else
    if (use_libsamplerate != 0)
    {
        fprintf(stderr, "I_SDL_InitSound: use_libsamplerate=%i, but "
                        "libsamplerate support not compiled in.\n",
                        use_libsamplerate);
    }
#endif

    // SDL_mixer version 1.2.8 and earlier has a bug in the Mix_SetPanning
    // function that can cause the game to lock up.  If we're using an old
    // version, we need to apply a workaround.  But the workaround has its
    // own drawbacks ...

    {
        const SDL_version *mixer_version;
        int v;

        mixer_version = Mix_Linked_Version();
        v = SDL_VERSIONNUM(mixer_version->major,
                           mixer_version->minor,
                           mixer_version->patch);

        if (v <= SDL_VERSIONNUM(1, 2, 8))
        {
            setpanning_workaround = true;
            fprintf(stderr, "\n"
              "ATTENTION: You are using an old version of SDL_mixer!\n"
              "           This version has a bug that may cause "
                          "your sound to stutter.\n"
              "           Please upgrade to a newer version!\n"
              "\n");
        }
    }

    Mix_AllocateChannels(NUM_CHANNELS);

    SDL_PauseAudio(0);

    sound_initialized = true;

    return true;
}

static snddevice_t sound_sdl_devices[] = 
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_AWE32,
};

sound_module_t sound_sdl_module = 
{
    sound_sdl_devices,
    arrlen(sound_sdl_devices),
    I_SDL_InitSound,
    I_SDL_ShutdownSound,
    I_SDL_GetSfxLumpNum,
    I_SDL_UpdateSound,
    I_SDL_UpdateSoundParams,
    I_SDL_StartSound,
    I_SDL_StopSound,
    I_SDL_SoundIsPlaying,
};

