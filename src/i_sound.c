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
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "doomfeatures.h"
#include "doomtype.h"

#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"

// Disable music on OSX by default; there are problems with SDL_mixer.

#ifndef __APPLE__
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_SB
#else
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_NONE
#endif

// Low-level sound and music modules we are using

static sound_module_t *sound_module;
static music_module_t *music_module;

int snd_musicdevice = DEFAULT_MUSIC_DEVICE;
int snd_sfxdevice = SNDDEVICE_SB;

// Sound modules

extern sound_module_t sound_sdl_module;
extern sound_module_t sound_pcsound_module;
extern music_module_t music_sdl_module;

// Compiled-in sound modules:

static sound_module_t *sound_modules[] = 
{
#ifdef FEATURE_SOUND
    &sound_sdl_module,
    &sound_pcsound_module,
#endif
    NULL,
};

// Compiled-in music modules:

static music_module_t *music_modules[] =
{
#ifdef FEATURE_SOUND
    &music_sdl_module,
#endif
    NULL,
};

// Check if a sound device is in the given list of devices

static boolean SndDeviceInList(snddevice_t device, snddevice_t *list,
                               int len)
{
    int i;

    for (i=0; i<len; ++i)
    {
        if (device == list[i])
        {
            return true;
        }
    }

    return false;
}

// Find and initialise a sound_module_t appropriate for the setting
// in snd_sfxdevice.

static void InitSfxModule(void)
{
    int i;

    sound_module = NULL;

    for (i=0; sound_modules[i] != NULL; ++i)
    {
        // Is the sfx device in the list of devices supported by
        // this module?

        if (SndDeviceInList(snd_sfxdevice, 
                            sound_modules[i]->sound_devices,
                            sound_modules[i]->num_sound_devices))
        {
            // Initialise the module

            if (sound_modules[i]->Init())
            {
                sound_module = sound_modules[i];
                return;
            }
        }
    }
}

// Initialise music according to snd_musicdevice.

static void InitMusicModule(void)
{
    int i;

    music_module = NULL;

    for (i=0; music_modules[i] != NULL; ++i)
    {
        // Is the music device in the list of devices supported
        // by this module?

        if (SndDeviceInList(snd_musicdevice, 
                            music_modules[i]->sound_devices,
                            music_modules[i]->num_sound_devices))
        {
            // Initialise the module

            if (music_modules[i]->Init())
            {
                music_module = music_modules[i];
                return;
            }
        }
    }
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void I_InitSound(void)
{  
    boolean nosound, nosfx, nomusic;

    //!
    // @vanilla
    //
    // Disable all sound output.
    //

    nosound = M_CheckParm("-nosound") > 0;

    //!
    // @vanilla
    //
    // Disable sound effects. 
    //

    nosfx = M_CheckParm("-nosfx") > 0;

    //!
    // @vanilla
    //
    // Disable music.
    //

    nomusic = M_CheckParm("-nomusic") > 0;

    // Initialise the sound and music subsystems.

    if (!nosound && !screensaver_mode)
    {
        if (!nosfx)
        {
            InitSfxModule();
        }

        if (!nomusic)
        {
            InitMusicModule();
        }
    }
}

void I_ShutdownSound(void)
{
    if (sound_module != NULL)
    {
        sound_module->Shutdown();
    }

    if (music_module != NULL)
    {
        music_module->Shutdown();
    }
}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    if (sound_module != NULL) 
    {
        return sound_module->GetSfxLumpNum(sfxinfo);
    }
    else
    {
        return 0;
    }
}

void I_UpdateSound(void)
{
    if (sound_module != NULL)
    {
        sound_module->Update();
    }
}

void I_UpdateSoundParams(int channel, int vol, int sep)
{
    if (sound_module != NULL)
    {
        sound_module->UpdateSoundParams(channel, vol, sep);
    }
}

int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep)
{
    if (sound_module != NULL)
    {
        return sound_module->StartSound(sfxinfo, channel, vol, sep);
    }
    else
    {
        return 0;
    }
}

void I_StopSound(int channel)
{
    if (sound_module != NULL)
    {
        sound_module->StopSound(channel);
    }
}

boolean I_SoundIsPlaying(int channel)
{
    if (sound_module != NULL)
    {
        return sound_module->SoundIsPlaying(channel);
    }
    else
    {
        return false;
    }
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{

}

void I_SetMusicVolume(int volume)
{
    if (music_module != NULL)
    {
        music_module->SetMusicVolume(volume);
    }
}

void I_PauseSong(void)
{
    if (music_module != NULL)
    {
        music_module->PauseMusic();
    }
}

void I_ResumeSong(void)
{
    if (music_module != NULL)
    {
        music_module->ResumeMusic();
    }
}

void *I_RegisterSong(void *data, int len)
{
    if (music_module != NULL)
    {
        return music_module->RegisterSong(data, len);
    }
    else
    {
        return NULL;
    }
}

void I_UnRegisterSong(void *handle)
{
    if (music_module != NULL)
    {
        music_module->UnRegisterSong(handle);
    }
}

void I_PlaySong(void *handle, boolean looping)
{
    if (music_module != NULL)
    {
        music_module->PlaySong(handle, looping);
    }
}

void I_StopSong(void)
{
    if (music_module != NULL)
    {
        music_module->StopSong();
    }
}

boolean I_MusicIsPlaying(void)
{
    if (music_module != NULL)
    {
        return music_module->MusicIsPlaying();
    }
    else
    {
        return false;
    }
}

