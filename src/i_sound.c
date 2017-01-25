//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

#include "SDL_mixer.h"

#include "config.h"
#include "doomtype.h"

#include "gusconf.h"
#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"

// Sound sample rate to use for digital output (Hz)

int snd_samplerate = 44100;

// Maximum number of bytes to dedicate to allocated sound effects.
// (Default: 64MB)

int snd_cachesize = 64 * 1024 * 1024;

// Config variable that controls the sound buffer size.
// We default to 28ms (1000 / 35fps = 1 buffer per tic).

int snd_maxslicetime_ms = 28;

// External command to invoke to play back music.

char *snd_musiccmd = "";

// Whether to vary the pitch of sound effects
// Each game will set the default differently

int snd_pitchshift = -1;

// Low-level sound and music modules we are using

static sound_module_t *sound_module;
static music_module_t *music_module;

int snd_musicdevice = SNDDEVICE_SB;
int snd_sfxdevice = SNDDEVICE_SB;

// Sound modules

extern void I_InitTimidityConfig(void);
extern sound_module_t sound_sdl_module;
extern sound_module_t sound_pcsound_module;
extern music_module_t music_sdl_module;
extern music_module_t music_opl_module;

// For OPL module:

extern opl_driver_ver_t opl_drv_ver;
extern int opl_io_port;

// For native music module:

extern char *music_pack_path;
extern char *timidity_cfg_path;

// DOS-specific options: These are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq = 0;
static int snd_sbdma = 0;
static int snd_mport = 0;

// Compiled-in sound modules:

static sound_module_t *sound_modules[] = 
{
    &sound_sdl_module,
    &sound_pcsound_module,
    NULL,
};

// Compiled-in music modules:

static music_module_t *music_modules[] =
{
    &music_sdl_module,
    &music_opl_module,
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

// Find and initialize a sound_module_t appropriate for the setting
// in snd_sfxdevice.

static void InitSfxModule(boolean use_sfx_prefix)
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
            // Initialize the module

            if (sound_modules[i]->Init(use_sfx_prefix))
            {
                sound_module = sound_modules[i];
                return;
            }
        }
    }
}

// Initialize music according to snd_musicdevice.

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
            // Initialize the module

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

void I_InitSound(boolean use_sfx_prefix)
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

    // Initialize the sound and music subsystems.

    if (!nosound && !screensaver_mode)
    {
        // This is kind of a hack. If native MIDI is enabled, set up
        // the TIMIDITY_CFG environment variable here before SDL_mixer
        // is opened.

        if (!nomusic
         && (snd_musicdevice == SNDDEVICE_GENMIDI
          || snd_musicdevice == SNDDEVICE_GUS))
        {
            I_InitTimidityConfig();
        }

        if (!nosfx)
        {
            InitSfxModule(use_sfx_prefix);
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

    if (music_module != NULL && music_module->Poll != NULL)
    {
        music_module->Poll();
    }
}

static void CheckVolumeSeparation(int *vol, int *sep)
{
    if (*sep < 0)
    {
        *sep = 0;
    }
    else if (*sep > 254)
    {
        *sep = 254;
    }

    if (*vol < 0)
    {
        *vol = 0;
    }
    else if (*vol > 127)
    {
        *vol = 127;
    }
}

void I_UpdateSoundParams(int channel, int vol, int sep)
{
    if (sound_module != NULL)
    {
        CheckVolumeSeparation(&vol, &sep);
        sound_module->UpdateSoundParams(channel, vol, sep);
    }
}

int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch)
{
    if (sound_module != NULL)
    {
        CheckVolumeSeparation(&vol, &sep);
        return sound_module->StartSound(sfxinfo, channel, vol, sep, pitch);
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

void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    if (sound_module != NULL && sound_module->CacheSounds != NULL)
    {
        sound_module->CacheSounds(sounds, num_sounds);
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

void I_BindSoundVariables(void)
{
    extern char *snd_dmxoption;
    extern int use_libsamplerate;
    extern float libsamplerate_scale;

    M_BindIntVariable("snd_musicdevice",         &snd_musicdevice);
    M_BindIntVariable("snd_sfxdevice",           &snd_sfxdevice);
    M_BindIntVariable("snd_sbport",              &snd_sbport);
    M_BindIntVariable("snd_sbirq",               &snd_sbirq);
    M_BindIntVariable("snd_sbdma",               &snd_sbdma);
    M_BindIntVariable("snd_mport",               &snd_mport);
    M_BindIntVariable("snd_maxslicetime_ms",     &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd",         &snd_musiccmd);
    M_BindStringVariable("snd_dmxoption",        &snd_dmxoption);
    M_BindIntVariable("snd_samplerate",          &snd_samplerate);
    M_BindIntVariable("snd_cachesize",           &snd_cachesize);
    M_BindIntVariable("opl_io_port",             &opl_io_port);
    M_BindIntVariable("snd_pitchshift",          &snd_pitchshift);

    M_BindStringVariable("music_pack_path",      &music_pack_path);
    M_BindStringVariable("timidity_cfg_path",    &timidity_cfg_path);
    M_BindStringVariable("gus_patch_path",       &gus_patch_path);
    M_BindIntVariable("gus_ram_kb",              &gus_ram_kb);

    M_BindIntVariable("use_libsamplerate",       &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale",   &libsamplerate_scale);
}

