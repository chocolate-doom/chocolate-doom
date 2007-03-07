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
//	System interface for PC speaker sound.
//
//-----------------------------------------------------------------------------

#include "SDL.h"

#include "doomdef.h"
#include "doomtype.h"

#include "i_pcsound.h"
#include "i_sound.h"
#include "sounds.h"

#include "w_wad.h"
#include "z_zone.h"

#include "pcsound/pcsound.h"

static boolean pcs_initialised = false;

static SDL_mutex *sound_lock;

static uint8_t *current_sound_lump = NULL;
static uint8_t *current_sound_pos = NULL;
static unsigned int current_sound_remaining = 0;
static int current_sound_handle = 0;

static float frequencies[] = {
    0, 175.00, 180.02, 185.01, 190.02, 196.02, 202.02, 208.01, 214.02, 220.02,
    226.02, 233.04, 240.02, 247.03, 254.03, 262.00, 269.03, 277.03, 285.04,
    294.03, 302.07, 311.04, 320.05, 330.06, 339.06, 349.08, 359.06, 370.09,
    381.08, 392.10, 403.10, 415.01, 427.05, 440.12, 453.16, 466.08, 480.15,
    494.07, 508.16, 523.09, 539.16, 554.19, 571.17, 587.19, 604.14, 622.09,
    640.11, 659.21, 679.10, 698.17, 719.21, 740.18, 762.41, 784.47, 807.29,
    831.48, 855.32, 880.57, 906.67, 932.17, 960.69, 988.55, 1017.20, 1046.64,
    1077.85, 1109.93, 1141.79, 1175.54, 1210.12, 1244.19, 1281.61, 1318.43,
    1357.42, 1397.16, 1439.30, 1480.37, 1523.85, 1569.97, 1614.58, 1661.81,
    1711.87, 1762.45, 1813.34, 1864.34, 1921.38, 1975.46, 2036.14, 2093.29,
    2157.64, 2217.80, 2285.78, 2353.41, 2420.24, 2490.98, 2565.97, 2639.77,
};

#define NUM_FREQUENCIES (sizeof(frequencies) / sizeof(*frequencies))

void PCSCallbackFunc(int *duration, int *freq)
{
    int tone;

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

        if (tone < NUM_FREQUENCIES)
        {
            *freq = (int) frequencies[tone];
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

static boolean CachePCSLump(int sound_id)
{
    int lumplen;
    int headerlen;

    // Free the current sound lump back to the cache
 
    if (current_sound_lump != NULL)
    {
        Z_ChangeTag(current_sound_lump, PU_CACHE);
        current_sound_lump = NULL;
    }

    // Load from WAD

    current_sound_lump = W_CacheLumpNum(S_sfx[sound_id].lumpnum, PU_STATIC);
    lumplen = W_LumpLength(S_sfx[sound_id].lumpnum);

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

    return true;
}

int I_PCS_StartSound(int id,
                     int channel,
                     int vol,
                     int sep,
                     int pitch,
                     int priority)
{
    int result;

    if (!pcs_initialised)
    {
        return -1;
    }

    // These PC speaker sounds are not played - this can be seen in the 
    // Heretic source code, where there are remnants of this left over
    // from Doom.

    if (id == sfx_posact || id == sfx_bgact || id == sfx_dmact
     || id == sfx_dmpain || id == sfx_popain || id == sfx_sawidl)
    {
        return -1;
    }

    if (SDL_LockMutex(sound_lock) < 0)
    {
        return -1;
    }

    result = CachePCSLump(id);

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

void I_PCS_StopSound(int handle)
{
    if (!pcs_initialised)
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

int I_PCS_SoundIsPlaying(int handle)
{
    if (!pcs_initialised)
    {
        return false;
    }

    if (handle != current_sound_handle)
    {
        return false;
    }

    return current_sound_lump != NULL && current_sound_remaining > 0;
}

void I_PCS_InitSound(void)
{
    pcs_initialised = PCSound_Init(PCSCallbackFunc);

    sound_lock = SDL_CreateMutex();
}

