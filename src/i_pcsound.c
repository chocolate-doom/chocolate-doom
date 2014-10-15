//
// Copyright(C) 2005-2014 Simon Howard
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
//	System interface for PC speaker sound.
//

#include "SDL.h"
#include <string.h>

#include "doomtype.h"

#include "deh_str.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "pcsound.h"

#define TIMER_FREQ 1193181 /* hz */

static boolean pcs_initialized = false;

static SDL_mutex *sound_lock;
static boolean use_sfx_prefix;

static uint8_t *current_sound_lump = NULL;
static uint8_t *current_sound_pos = NULL;
static unsigned int current_sound_remaining = 0;
static int current_sound_handle = 0;
static int current_sound_lump_num = -1;

static const uint16_t divisors[] = {
    0,
    6818, 6628, 6449, 6279, 6087, 5906, 5736, 5575,
    5423, 5279, 5120, 4971, 4830, 4697, 4554, 4435,
    4307, 4186, 4058, 3950, 3836, 3728, 3615, 3519,
    3418, 3323, 3224, 3131, 3043, 2960, 2875, 2794,
    2711, 2633, 2560, 2485, 2415, 2348, 2281, 2213,
    2153, 2089, 2032, 1975, 1918, 1864, 1810, 1757,
    1709, 1659, 1612, 1565, 1521, 1478, 1435, 1395,
    1355, 1316, 1280, 1242, 1207, 1173, 1140, 1107,
    1075, 1045, 1015,  986,  959,  931,  905,  879,
     854,  829,  806,  783,  760,  739,  718,  697,
     677,  658,  640,  621,  604,  586,  570,  553,
     538,  522,  507,  493,  479,  465,  452,  439,
     427,  415,  403,  391,  380,  369,  359,  348,
     339,  329,  319,  310,  302,  293,  285,  276,
     269,  261,  253,  246,  239,  232,  226,  219,
     213,  207,  201,  195,  190,  184,  179,
};

static void PCSCallbackFunc(int *duration, int *freq)
{
    unsigned int tone;

    *duration = 1000 / 140;

    if (SDL_LockMutex(sound_lock) < 0)
    {
        *freq = 0;
        return;
    }

    if (current_sound_lump != NULL && current_sound_remaining > 0)
    {
        // Read the next tone

        tone = *current_sound_pos;

        // Use the tone -> frequency lookup table.  See pcspkr10.zip
        // for a full discussion of this.
        // Check we don't overflow the frequency table.

        if (tone < arrlen(divisors) && divisors[tone] != 0)
        {
            *freq = (int) (TIMER_FREQ / divisors[tone]);
        }
        else
        {
            *freq = 0;
        }

        ++current_sound_pos;
        --current_sound_remaining;
    }
    else
    {
        *freq = 0;
    }

    SDL_UnlockMutex(sound_lock);
}

static boolean CachePCSLump(sfxinfo_t *sfxinfo)
{
    int lumplen;
    int headerlen;

    // Free the current sound lump back to the cache
 
    if (current_sound_lump != NULL)
    {
        W_ReleaseLumpNum(current_sound_lump_num);
        current_sound_lump = NULL;
    }

    // Load from WAD

    current_sound_lump = W_CacheLumpNum(sfxinfo->lumpnum, PU_STATIC);
    lumplen = W_LumpLength(sfxinfo->lumpnum);

    // Read header
  
    if (current_sound_lump[0] != 0x00 || current_sound_lump[1] != 0x00)
    {
        return false;
    }

    headerlen = (current_sound_lump[3] << 8) | current_sound_lump[2];

    if (headerlen > lumplen - 4)
    {
        return false;
    }

    // Header checks out ok

    current_sound_remaining = headerlen;
    current_sound_pos = current_sound_lump + 4;
    current_sound_lump_num = sfxinfo->lumpnum;

    return true;
}

// These Doom PC speaker sounds are not played - this can be seen in the 
// Heretic source code, where there are remnants of this left over
// from Doom.

static boolean IsDisabledSound(sfxinfo_t *sfxinfo)
{
    int i;
    const char *disabled_sounds[] = {
        "posact",
        "bgact",
        "dmact",
        "dmpain",
        "popain",
        "sawidl",
    };

    for (i=0; i<arrlen(disabled_sounds); ++i)
    {
        if (!strcmp(sfxinfo->name, disabled_sounds[i]))
        {
            return true;
        }
    }

    return false;
}

static int I_PCS_StartSound(sfxinfo_t *sfxinfo,
                            int channel,
                            int vol,
                            int sep)
{
    int result;

    if (!pcs_initialized)
    {
        return -1;
    }

    if (IsDisabledSound(sfxinfo))
    {
        return -1;
    }

    if (SDL_LockMutex(sound_lock) < 0)
    {
        return -1;
    }

    result = CachePCSLump(sfxinfo);

    if (result)
    {
        current_sound_handle = channel;
    }

    SDL_UnlockMutex(sound_lock);

    if (result)
    {
        return channel;
    }
    else
    {
        return -1;
    }
}

static void I_PCS_StopSound(int handle)
{
    if (!pcs_initialized)
    {
        return;
    }

    if (SDL_LockMutex(sound_lock) < 0)
    {
        return;
    }

    // If this is the channel currently playing, immediately end it.

    if (current_sound_handle == handle)
    {
        current_sound_remaining = 0;
    }
    
    SDL_UnlockMutex(sound_lock);
}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//

static int I_PCS_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];

    if (use_sfx_prefix)
    {
        M_snprintf(namebuf, sizeof(namebuf), "dp%s", DEH_String(sfx->name));
    }
    else
    {
        M_StringCopy(namebuf, DEH_String(sfx->name), sizeof(namebuf));
    }

    return W_GetNumForName(namebuf);
}


static boolean I_PCS_SoundIsPlaying(int handle)
{
    if (!pcs_initialized)
    {
        return false;
    }

    if (handle != current_sound_handle)
    {
        return false;
    }

    return current_sound_lump != NULL && current_sound_remaining > 0;
}

static boolean I_PCS_InitSound(boolean _use_sfx_prefix)
{
    use_sfx_prefix = _use_sfx_prefix;

    // Use the sample rate from the configuration file

    PCSound_SetSampleRate(snd_samplerate);

    // Initialize the PC speaker subsystem.

    pcs_initialized = PCSound_Init(PCSCallbackFunc);

    if (pcs_initialized)
    {
        sound_lock = SDL_CreateMutex();
    }

    return pcs_initialized;
}

static void I_PCS_ShutdownSound(void)
{
    if (pcs_initialized)
    {
        PCSound_Shutdown();
    }
}

static void I_PCS_UpdateSound(void)
{
    // no-op.
}

void I_PCS_UpdateSoundParams(int channel, int vol, int sep)
{
    // no-op.
}

static snddevice_t sound_pcsound_devices[] = 
{
    SNDDEVICE_PCSPEAKER,
};

sound_module_t sound_pcsound_module = 
{
    sound_pcsound_devices,
    arrlen(sound_pcsound_devices),
    I_PCS_InitSound,
    I_PCS_ShutdownSound,
    I_PCS_GetSfxLumpNum,
    I_PCS_UpdateSound,
    I_PCS_UpdateSoundParams,
    I_PCS_StartSound,
    I_PCS_StopSound,
    I_PCS_SoundIsPlaying,
};

