//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
// DESCRIPTION:
//     Sysop-64 PCM sound effect caching, channel control, and Chocolate Doom
//     sound backend glue.
//

#include "config.h"
#include "deh_str.h"
#include "i_sound.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "s_sound.h"
#include "sid_player_bridge.h"
#include "sysop64.h"
#include "sysop64_backend.h"
#include "w_wad.h"
#include "z_zone.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
int snd_samplerate = 44100;
int snd_cachesize = 64 * 1024 * 1024;
int snd_maxslicetime_ms = 28;
char *snd_musiccmd = "";
int snd_pitchshift = -1;
int snd_musicdevice = SNDDEVICE_SB;
int snd_sfxdevice = SNDDEVICE_SB;
char *snd_dmxoption = "";
int use_libsamplerate = 0;
float libsamplerate_scale = 0.65f;
int opl_io_port = 0x388;
char *music_pack_path = "";
char *timidity_cfg_path = "";

#define SYSOP_PCM_DEFAULT_RATE 11025
#define SYSOP_SOUND_CHANNELS 8
#define SYSOP_AUDIO_MEM_BASE 0x28000000U
#define SYSOP_AUDIO_CHANNEL_BYTES (1024U * 1024U)
#define SYSOP_AUDIO_MEM_BYTES (SYSOP_SOUND_CHANNELS * SYSOP_AUDIO_CHANNEL_BYTES)
#ifndef SYSOP_AUDIO_FORMAT_U8_MONO_VAR
#define SYSOP_AUDIO_FORMAT_U8_MONO_VAR 1
#endif

typedef struct sysop_cached_sfx_s sysop_cached_sfx_t;
struct sysop_cached_sfx_s
{
    sfxinfo_t *sfxinfo;
    int lumpnum;
    int sample_rate;
    int sample_count;
    size_t byte_length;
    uint8_t *samples;
    sysop_cached_sfx_t *next;
};

typedef struct
{
    boolean active;
    int end_time_ms;
    int left_volume;
    int right_volume;
    sysop_cached_sfx_t *sfx;
} sysop_sound_channel_t;

static boolean sysop_sound_initialized = false;
static boolean sysop_use_sfx_prefix = true;
static int sysop_sfx_volume = SYSOP_DOOM_VOLUME_MAX;
static size_t sysop_sfx_cache_bytes = 0;
static int sysop_audio_mem_fd = -1;
static uint8_t *sysop_audio_mem = NULL;
static sysop_cached_sfx_t *sysop_sfx_cache = NULL;
static sysop_sound_channel_t sysop_sound_channels[SYSOP_SOUND_CHANNELS];

// Clamp sound mixer values before converting them to Sysop hardware ranges.
static int Sysop_SoundClamp(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

// Follow Chocolate Doom's linked-SFX indirection so aliases share one cached
// sample.
static sfxinfo_t *Sysop_BaseSfx(sfxinfo_t *sfxinfo)
{
    if (sfxinfo != NULL && sfxinfo->link != NULL) {
        return sfxinfo->link;
    }

    return sfxinfo;
}

// Build the WAD lump name for an SFX, including the Doom-style "ds" prefix
// when the current game mission uses it.
static void Sysop_GetSfxLumpName(sfxinfo_t *sfx, char *buf, size_t buf_len)
{
    sfx = Sysop_BaseSfx(sfx);

    if (sfx == NULL) {
        if (buf_len > 0) {
            buf[0] = '\0';
        }
        return;
    }

    if (sysop_use_sfx_prefix) {
        M_snprintf(buf, buf_len, "ds%s", DEH_String(sfx->name));
    } else {
        M_StringCopy(buf, DEH_String(sfx->name), buf_len);
    }
}

// Convert Doom's volume and stereo separation values into left/right Sysop
// channel volumes.
static void Sysop_StereoVolumes(int vol, int sep, int *left, int *right)
{
    vol = Sysop_SoundClamp(vol, 0, 127);
    sep = Sysop_SoundClamp(sep, 0, 254);

    *left = Sysop_SoundClamp(((254 - sep) * vol) / 127, 0, 255);
    *right = Sysop_SoundClamp((sep * vol) / 127, 0, 255);
}

// Map the fixed Sysop PCM audio memory window used by all hardware channels.
static int Sysop_PCM_MapAudioMemory(void)
{
    if (sysop_audio_mem != NULL) {
        return 1;
    }

    sysop_audio_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (sysop_audio_mem_fd < 0) {
        perror("Sysop PCM: couldn't open /dev/mem");
        return 0;
    }

    sysop_audio_mem = (uint8_t *)mmap(NULL, SYSOP_AUDIO_MEM_BYTES,
                                      PROT_READ | PROT_WRITE, MAP_SHARED,
                                      sysop_audio_mem_fd,
                                      SYSOP_AUDIO_MEM_BASE);

    if (sysop_audio_mem == MAP_FAILED) {
        perror("Sysop PCM: couldn't mmap audio memory");
        close(sysop_audio_mem_fd);
        sysop_audio_mem_fd = -1;
        sysop_audio_mem = NULL;
        return 0;
    }

    return 1;
}

// Release the mapped Sysop PCM audio memory window.
static void Sysop_PCM_UnmapAudioMemory(void)
{
    if (sysop_audio_mem != NULL) {
        munmap(sysop_audio_mem, SYSOP_AUDIO_MEM_BYTES);
        sysop_audio_mem = NULL;
    }

    if (sysop_audio_mem_fd >= 0) {
        close(sysop_audio_mem_fd);
        sysop_audio_mem_fd = -1;
    }
}

// Return the physical audio base address reserved for one hardware channel.
static uint32_t Sysop_PCM_ChannelBaseAddr(int channel)
{
    return SYSOP_AUDIO_MEM_BASE
         + ((uint32_t)channel * SYSOP_AUDIO_CHANNEL_BYTES);
}

// Return the mmap pointer for one channel's dedicated sample buffer.
static uint8_t *Sysop_PCM_ChannelMemory(int channel)
{
    if (sysop_audio_mem == NULL || channel < 0
     || channel >= SYSOP_SOUND_CHANNELS) {
        return NULL;
    }

    return sysop_audio_mem
         + ((size_t)channel * SYSOP_AUDIO_CHANNEL_BYTES);
}

// Apply Doom pitch shifting to an SFX sample rate for Sysop variable-rate
// playback.
static uint32_t Sysop_PCM_EffectiveRate(const sysop_cached_sfx_t *sfx, int pitch)
{
    uint64_t rate;

    if (sfx == NULL || sfx->sample_rate <= 0) {
        return SYSOP_PCM_DEFAULT_RATE;
    }

    if (!snd_pitchshift) {
        return (uint32_t)sfx->sample_rate;
    }

    if (pitch <= 0) {
        pitch = NORM_PITCH;
    }

    rate = ((uint64_t)sfx->sample_rate * (uint64_t)pitch
          + (NORM_PITCH / 2)) / NORM_PITCH;

    if (rate < 1) {
        rate = 1;
    } else if (rate > UINT32_MAX) {
        rate = UINT32_MAX;
    }

    return (uint32_t)rate;
}

// Ask the Sysop audio hardware whether one PCM channel is still active.
static int Sysop_PCM_IsChannelPlaying(int channel)
{
    if (channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return 0;
    }

    sysop_audio_select_status_channel((uint32_t)channel);

    return sysop_audio_is_playing() ? 1 : 0;
}

// Copy cached unsigned 8-bit mono samples into a channel buffer and start Sysop
// hardware playback.
static int Sysop_PCM_Play(int channel,
                          const sysop_cached_sfx_t *sfx,
                          int left_volume,
                          int right_volume,
                          int pitch)
{
    uint32_t effective_rate;
    uint32_t phase_step;
    uint8_t *channel_mem;
    uint32_t base_addr;

    if (channel < 0 || channel >= SYSOP_SOUND_CHANNELS || sfx == NULL
     || sfx->samples == NULL || sfx->sample_count <= 0) {
        return 0;
    }

    if ((uint32_t)sfx->sample_count > SYSOP_AUDIO_CHANNEL_BYTES) {
        fprintf(stderr,
                "Sysop PCM: SFX too large for channel buffer (%d > %u)\n",
                sfx->sample_count, SYSOP_AUDIO_CHANNEL_BYTES);
        return 0;
    }

    channel_mem = Sysop_PCM_ChannelMemory(channel);

    if (channel_mem == NULL) {
        return 0;
    }

    memcpy(channel_mem, sfx->samples, (size_t)sfx->sample_count);

    effective_rate = Sysop_PCM_EffectiveRate(sfx, pitch);
    phase_step = sysop_audio_phase_step_from_rate(effective_rate);
    base_addr = Sysop_PCM_ChannelBaseAddr(channel);

    sysop_audio_select_channel((uint32_t)channel);
    sysop_audio_stop();
    sysop_audio_set_sample_format(SYSOP_AUDIO_FORMAT_U8_MONO_VAR);
    sysop_audio_set_base_addr(base_addr);
    sysop_audio_set_length_frames((uint32_t)sfx->sample_count);
    sysop_audio_set_loop_enable(false);
    sysop_audio_set_phase_step(phase_step);
    sysop_audio_set_volume((uint32_t)left_volume, (uint32_t)right_volume);
    sysop_audio_start();

    return 1;
}

// Stop one Sysop PCM hardware channel.
static void Sysop_PCM_Stop(int channel)
{
    if (channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return;
    }

    sysop_audio_select_channel((uint32_t)channel);
    sysop_audio_stop();
}

// Update left/right volume for an already-playing Sysop PCM channel.
static void Sysop_PCM_UpdateParams(int channel, int left_volume, int right_volume)
{
    if (channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return;
    }

    sysop_audio_select_channel((uint32_t)channel);
    sysop_audio_set_volume((uint32_t)left_volume, (uint32_t)right_volume);
}

// Free all cached WAD sound samples and reset cache accounting.
static void Sysop_FreeSfxCache(void)
{
    sysop_cached_sfx_t *cached;

    cached = sysop_sfx_cache;

    while (cached != NULL) {
        sysop_cached_sfx_t *next = cached->next;

        if (cached->byte_length <= sysop_sfx_cache_bytes) {
            sysop_sfx_cache_bytes -= cached->byte_length;
        } else {
            sysop_sfx_cache_bytes = 0;
        }

        free(cached->samples);
        free(cached);

        cached = next;
    }

    sysop_sfx_cache = NULL;
}

// Count cached unique SFX entries for startup/precache diagnostics.
static int Sysop_CountSfxCache(void)
{
    int count = 0;
    sysop_cached_sfx_t *cached;

    for (cached = sysop_sfx_cache; cached != NULL; cached = cached->next) {
        ++count;
    }

    return count;
}

// Find the cached sample data for an SFX base entry.
static sysop_cached_sfx_t *Sysop_FindCachedSfx(sfxinfo_t *sfxinfo)
{
    sysop_cached_sfx_t *cached;
    sfxinfo_t *base;

    base = Sysop_BaseSfx(sfxinfo);

    for (cached = sysop_sfx_cache; cached != NULL; cached = cached->next) {
        if (cached->sfxinfo == base) {
            return cached;
        }
    }

    return NULL;
}

// Load a Doom DMX-format sound lump and cache its unsigned 8-bit mono payload.
static sysop_cached_sfx_t *Sysop_CacheSfx(sfxinfo_t *sfxinfo)
{
    char namebuf[16];
    sfxinfo_t *base;
    sysop_cached_sfx_t *cached;
    byte *lump_data;
    const byte *source;
    uint8_t *samples;
    int lumpnum;
    int lumplen;
    int sample_rate;
    unsigned int source_length;
    int sample_count;
    size_t byte_length;

    base = Sysop_BaseSfx(sfxinfo);

    if (base == NULL || base->name[0] == '\0') {
        return NULL;
    }

    cached = Sysop_FindCachedSfx(base);

    if (cached != NULL) {
        return cached;
    }

    Sysop_GetSfxLumpName(base, namebuf, sizeof(namebuf));
    lumpnum = W_CheckNumForName(namebuf);

    if (lumpnum < 0) {
        return NULL;
    }

    lump_data = W_CacheLumpNum(lumpnum, PU_STATIC);
    lumplen = W_LumpLength(lumpnum);

    if (lumplen < 8 || lump_data[0] != 0x03 || lump_data[1] != 0x00) {
        W_ReleaseLumpNum(lumpnum);
        return NULL;
    }

    sample_rate = (lump_data[3] << 8) | lump_data[2];
    source_length = ((unsigned int)lump_data[7] << 24)
                  | ((unsigned int)lump_data[6] << 16)
                  | ((unsigned int)lump_data[5] << 8)
                  | (unsigned int)lump_data[4];

    if (sample_rate <= 0 || source_length > (unsigned int)lumplen - 8
     || source_length <= 48) {
        W_ReleaseLumpNum(lumpnum);
        return NULL;
    }

    // Match Chocolate Doom's SDL backend: DMX skips the first and last
    // 16 bytes of the sound payload.
    source = lump_data + 8 + 16;
    sample_count = (int)source_length - 32;

    if (sample_count <= 0) {
        W_ReleaseLumpNum(lumpnum);
        return NULL;
    }

    byte_length = (size_t)sample_count;
    samples = malloc(byte_length);

    if (samples == NULL) {
        W_ReleaseLumpNum(lumpnum);
        return NULL;
    }

    memcpy(samples, source, byte_length);

    W_ReleaseLumpNum(lumpnum);

    cached = malloc(sizeof(*cached));

    if (cached == NULL) {
        free(samples);
        return NULL;
    }

    cached->sfxinfo = base;
    cached->lumpnum = lumpnum;
    cached->sample_rate = sample_rate;
    cached->sample_count = sample_count;
    cached->byte_length = byte_length;
    cached->samples = samples;
    cached->next = sysop_sfx_cache;
    sysop_sfx_cache = cached;
    sysop_sfx_cache_bytes += byte_length;

    base->lumpnum = lumpnum;

    return cached;
}

// Estimate playback duration for channel bookkeeping using the effective
// pitch-adjusted rate.
static int Sysop_SfxDurationMs(const sysop_cached_sfx_t *sfx, int pitch)
{
    uint32_t effective_rate;

    if (sfx == NULL || sfx->sample_count <= 0 || sfx->sample_rate <= 0) {
        return 0;
    }

    effective_rate = Sysop_PCM_EffectiveRate(sfx, pitch);

    return (int)(((uint64_t)sfx->sample_count * 1000 + effective_rate - 1)
               / effective_rate);
}

// Mark a channel inactive once hardware playback or the estimated duration has
// completed.
static void Sysop_UpdateSoundChannel(int channel)
{
    if (channel < 0 || channel >= SYSOP_SOUND_CHANNELS
     || !sysop_sound_channels[channel].active) {
        return;
    }

    if (!Sysop_PCM_IsChannelPlaying(channel)
     || I_GetTimeMS() >= sysop_sound_channels[channel].end_time_ms) {
        sysop_sound_channels[channel].active = false;
        sysop_sound_channels[channel].sfx = NULL;
    }
}

// Initialize Sysop PCM SFX playback, map channel memory, and configure all
// hardware channels.
void I_InitSound(GameMission_t mission)
{
    int i;

    ensure_sysop_backend_args_parsed(myargc, myargv);

    if (sysop_sound_initialized) {
        return;
    }

    if (M_CheckParm("-nosound") > 0 || M_CheckParm("-nosfx") > 0
     || !g_sysop_pcm_sfx_enabled) {
        sysop_sound_initialized = false;
        return;
    }

    sysop_use_sfx_prefix = (mission == doom || mission == doom2
                         || mission == pack_tnt || mission == pack_plut
                         || mission == pack_chex || mission == pack_hacx
                         || mission == doom2f || mission == strife);

    snd_samplerate = SYSOP_PCM_DEFAULT_RATE;

    if (!Sysop_AcquireLibrary("audio")) {
        sysop_sound_initialized = false;
        return;
    }

    if (!Sysop_PCM_MapAudioMemory()) {
        Sysop_ReleaseLibrary();
        sysop_sound_initialized = false;
        return;
    }

    if (snd_channels < 1) {
        snd_channels = 1;
    } else if (snd_channels > SYSOP_SOUND_CHANNELS) {
        printf("Sysop PCM: limiting snd_channels from %d to %d\n",
               snd_channels, SYSOP_SOUND_CHANNELS);
        snd_channels = SYSOP_SOUND_CHANNELS;
    }

    for (i = 0; i < SYSOP_SOUND_CHANNELS; ++i) {
        sysop_sound_channels[i].active = false;
        sysop_sound_channels[i].end_time_ms = 0;
        sysop_sound_channels[i].left_volume = 0;
        sysop_sound_channels[i].right_volume = 0;
        sysop_sound_channels[i].sfx = NULL;

        sysop_audio_select_channel((uint32_t)i);
        sysop_audio_stop();
        sysop_audio_set_sample_format(SYSOP_AUDIO_FORMAT_U8_MONO_VAR);
        sysop_audio_set_base_addr(Sysop_PCM_ChannelBaseAddr(i));
        sysop_audio_set_length_frames(0);
        sysop_audio_set_loop_enable(false);
        sysop_audio_set_phase_step(
            sysop_audio_phase_step_from_rate(SYSOP_PCM_DEFAULT_RATE));
        sysop_audio_set_volume(0, 0);
    }

    sysop_sound_initialized = true;
}

// Stop active SFX, free cached samples, unmap audio memory, and release Sysop.
void I_ShutdownSound(void)
{
    int i;

    if (!sysop_sound_initialized) {
        return;
    }

    for (i = 0; i < SYSOP_SOUND_CHANNELS; ++i) {
        if (sysop_sound_channels[i].active) {
            Sysop_PCM_Stop(i);
        }
    }

    Sysop_FreeSfxCache();
    Sysop_PCM_UnmapAudioMemory();
    Sysop_ReleaseLibrary();
    sysop_sound_initialized = false;
}

// Resolve the WAD lump number for one Doom sound effect.
int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    char namebuf[16];

    Sysop_GetSfxLumpName(sfxinfo, namebuf, sizeof(namebuf));

    return W_GetNumForName(namebuf);
}

// Poll channel completion state for Chocolate Doom's sound mixer.
void I_UpdateSound(void)
{
    int i;

    if (!sysop_sound_initialized) {
        return;
    }

    for (i = 0; i < SYSOP_SOUND_CHANNELS; ++i) {
        Sysop_UpdateSoundChannel(i);
    }
}

// Update volume and stereo separation for a playing channel.
void I_UpdateSoundParams(int channel, int vol, int sep)
{
    int left;
    int right;

    if (!sysop_sound_initialized
     || channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return;
    }

    Sysop_StereoVolumes(vol, sep, &left, &right);

    sysop_sound_channels[channel].left_volume = left;
    sysop_sound_channels[channel].right_volume = right;

    if (sysop_sound_channels[channel].active) {
        Sysop_PCM_UpdateParams(channel, left, right);
    }
}

// Start one Doom SFX on the requested Sysop PCM channel.
int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch)
{
    sysop_cached_sfx_t *cached;
    int left;
    int right;
    int duration_ms;

    if (!sysop_sound_initialized
     || channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return -1;
    }

    Sysop_UpdateSoundChannel(channel);

    if (sysop_sound_channels[channel].active) {
        Sysop_PCM_Stop(channel);
        sysop_sound_channels[channel].active = false;
        sysop_sound_channels[channel].sfx = NULL;
    }

    cached = Sysop_CacheSfx(sfxinfo);

    if (cached == NULL || cached->samples == NULL || cached->sample_count <= 0) {
        return -1;
    }

    Sysop_StereoVolumes(vol, sep, &left, &right);

    if (!Sysop_PCM_Play(channel, cached, left, right, pitch)) {
        return -1;
    }

    duration_ms = Sysop_SfxDurationMs(cached, pitch);

    sysop_sound_channels[channel].active = true;
    sysop_sound_channels[channel].end_time_ms = I_GetTimeMS() + duration_ms;
    sysop_sound_channels[channel].left_volume = left;
    sysop_sound_channels[channel].right_volume = right;
    sysop_sound_channels[channel].sfx = cached;

    return channel;
}

// Stop one Doom SFX channel if it is active.
void I_StopSound(int channel)
{
    if (!sysop_sound_initialized
     || channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return;
    }

    if (sysop_sound_channels[channel].active) {
        Sysop_PCM_Stop(channel);
    }

    sysop_sound_channels[channel].active = false;
    sysop_sound_channels[channel].sfx = NULL;
}

// Report whether Chocolate Doom should consider an SFX channel active.
boolean I_SoundIsPlaying(int channel)
{
    if (!sysop_sound_initialized
     || channel < 0 || channel >= SYSOP_SOUND_CHANNELS) {
        return false;
    }

    Sysop_UpdateSoundChannel(channel);

    return sysop_sound_channels[channel].active;
}

// Preload all known SFX lumps into host memory so gameplay starts avoid WAD
// reads.
void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    int i;
    int before_count;
    int after_count;
    size_t before_bytes;

    if (!sysop_sound_initialized || sounds == NULL) {
        return;
    }

    before_count = Sysop_CountSfxCache();
    before_bytes = sysop_sfx_cache_bytes;

    for (i = 0; i < num_sounds; ++i) {
        Sysop_CacheSfx(&sounds[i]);
    }

    after_count = Sysop_CountSfxCache();

    printf("Sysop PCM: cached %d/%d unique SFX, %zu bytes total",
           after_count, num_sounds, sysop_sfx_cache_bytes);

    if (after_count > before_count) {
        printf(" (+%zu bytes)", sysop_sfx_cache_bytes - before_bytes);
    }

    printf("\n");
}

// Music initialization is handled by the Sysop SID bridge in the video backend.
void I_InitMusic(void) {}
// Music shutdown is handled with the SID bridge during backend teardown.
void I_ShutdownMusic(void) {}
// Route Chocolate Doom's music-volume slider to the Sysop SID mixer.
void I_SetMusicVolume(int volume)
{
    g_sysop_sid_music_volume = Sysop_ClampDoomVolume(volume);
    Sysop_ApplySidMusicVolume();
}

// Apply Chocolate Doom's SFX volume slider to currently playing PCM channels.
void I_SetSfxVolume(int volume)
{
    int old_volume;
    int new_volume;
    int i;

    old_volume = sysop_sfx_volume;
    new_volume = Sysop_ClampDoomVolume(volume);
    sysop_sfx_volume = new_volume;

    if (!sysop_sound_initialized || old_volume == new_volume) {
        return;
    }

    for (i = 0; i < SYSOP_SOUND_CHANNELS; ++i) {
        int left;
        int right;

        if (!sysop_sound_channels[i].active) {
            continue;
        }

        if (old_volume <= 0) {
            left = new_volume > 0 ? sysop_sound_channels[i].left_volume : 0;
            right = new_volume > 0 ? sysop_sound_channels[i].right_volume : 0;
        } else {
            left = (sysop_sound_channels[i].left_volume * new_volume
                  + (old_volume / 2)) / old_volume;
            right = (sysop_sound_channels[i].right_volume * new_volume
                   + (old_volume / 2)) / old_volume;
        }

        left = Sysop_SoundClamp(left, 0, SYSOP_AUDIO_VOLUME_MAX);
        right = Sysop_SoundClamp(right, 0, SYSOP_AUDIO_VOLUME_MAX);

        sysop_sound_channels[i].left_volume = left;
        sysop_sound_channels[i].right_volume = right;
        Sysop_PCM_UpdateParams(i, left, right);
    }
}

// SID playback currently continues through pause requests.
void I_PauseSong(void) {}
// SID playback currently continues through resume requests.
void I_ResumeSong(void) {}
// Return a dummy song handle because external music packs are not used here.
void *I_RegisterSong(void *data, int len) { (void)data; (void)len; return (void *)1; }
// Dummy song handles do not own resources.
void I_UnRegisterSong(void *handle) { (void)handle; }
// Song playback is controlled by the SID bridge rather than registered song
// data.
void I_PlaySong(void *handle, boolean looping) { (void)handle; (void)looping; }
// Stopping the registered song is a no-op for SID-backed music.
void I_StopSong(void) {}
// Report SID load state as Chocolate Doom's music-playing state.
boolean I_MusicIsPlaying(void) { return doom_sid_is_loaded() ? true : false; }
// OPL driver selection is unused by the Sysop SID/PCM backend.
void I_SetOPLDriverVer(opl_driver_ver_t ver) { (void)ver; }
// Return an empty OPL diagnostic string because OPL is not active on Sysop.
void I_OPL_DevMessages(char *result, size_t result_len) { if (result_len > 0) result[0] = '\0'; }
// Timidity config is unused because MIDI music is replaced by SID playback.
void I_InitTimidityConfig(void) {}
// ENDOOM display is unused on the C64/Sysop video path.
void I_Endoom(byte *data) { (void)data; }

// Bind the standard Chocolate Doom sound config variables so existing config
// files continue to load.
void I_BindSoundVariables(void)
{
    M_BindIntVariable("snd_musicdevice", &snd_musicdevice);
    M_BindIntVariable("snd_sfxdevice", &snd_sfxdevice);
    M_BindIntVariable("snd_samplerate", &snd_samplerate);
    M_BindIntVariable("snd_cachesize", &snd_cachesize);
    M_BindIntVariable("snd_maxslicetime_ms", &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd", &snd_musiccmd);
    M_BindStringVariable("snd_dmxoption", &snd_dmxoption);
    M_BindIntVariable("opl_io_port", &opl_io_port);
    M_BindIntVariable("snd_pitchshift", &snd_pitchshift);
    M_BindStringVariable("music_pack_path", &music_pack_path);
    M_BindStringVariable("timidity_cfg_path", &timidity_cfg_path);
    M_BindIntVariable("use_libsamplerate", &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale", &libsamplerate_scale);
}
