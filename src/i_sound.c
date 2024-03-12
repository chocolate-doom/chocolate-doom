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

#include "config.h"
#include "doomtype.h"

#include "gusconf.h"
#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"

#ifndef DISABLE_SDL2MIXER

#include "SDL_mixer.h"

#endif  // DISABLE_SDL2MIXER


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

int snd_musicdevice = SNDDEVICE_SB;
int snd_sfxdevice = SNDDEVICE_SB;

// Low-level sound and music modules we are using
static const sound_module_t *sound_module;
static const music_module_t *music_module;

// If true, the music pack module was successfully initialized.
static boolean music_packs_active = false;

// This is either equal to music_module or &music_pack_module,
// depending on whether the current track is substituted.
static const music_module_t *active_music_module;


// DOS-specific options: These are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq = 0;
static int snd_sbdma = 0;
static int snd_mport = 0;

// Compiled-in sound modules:

static const sound_module_t *sound_modules[] =
{
#ifndef DISABLE_SDL2MIXER
    &sound_sdl_module,
#endif // DISABLE_SDL2MIXER
    &sound_pcsound_module,
    NULL,
};

// Compiled-in music modules:

static const music_module_t *music_modules[] =
{
#ifdef _WIN32
    &music_win_module,
#endif
#ifdef HAVE_FLUIDSYNTH
    &music_fl_module,
#endif // HAVE_FLUIDSYNTH
#ifndef DISABLE_SDL2MIXER
    &music_sdl_module,
#endif // DISABLE_SDL2MIXER
    &music_opl_module,
    NULL,
};

// Check if a sound device is in the given list of devices

static boolean SndDeviceInList(snddevice_t device, const snddevice_t *list,
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

static void InitSfxModule(GameMission_t mission)
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

            if (sound_modules[i]->Init(mission))
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
        #ifdef _WIN32
            // Skip the native Windows MIDI module if using Timidity.

            if (strcmp(timidity_cfg_path, "") &&
                music_modules[i] == &music_win_module)
            {
                continue;
            }
        #endif

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

void I_InitSound(GameMission_t mission)
{
    boolean nosound, nosfx, nomusic, nomusicpacks;

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

    //!
    //
    // Disable substitution music packs.
    //

    nomusicpacks = M_ParmExists("-nomusicpacks");

    // Auto configure the music pack directory.
    M_SetMusicPackDir();

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
            InitSfxModule(mission);
        }

        if (!nomusic)
        {
            InitMusicModule();
            active_music_module = music_module;
        }

        // We may also have substitute MIDIs we can load.
        if (!nomusicpacks && music_module != NULL)
        {
            music_packs_active = music_pack_module.Init();
        }
    }
}

void I_ShutdownSound(void)
{
    if (sound_module != NULL)
    {
        sound_module->Shutdown();
    }

    if (music_packs_active)
    {
        music_pack_module.Shutdown();
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

    if (active_music_module != NULL && active_music_module->Poll != NULL)
    {
        active_music_module->Poll();
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

        if (music_packs_active && music_module != &music_pack_module)
        {
            music_pack_module.SetMusicVolume(volume);
        }
    }
}

void I_PauseSong(void)
{
    if (active_music_module != NULL)
    {
        active_music_module->PauseMusic();
    }
}

void I_ResumeSong(void)
{
    if (active_music_module != NULL)
    {
        active_music_module->ResumeMusic();
    }
}

void *I_RegisterSong(void *data, int len)
{
    // If the music pack module is active, check to see if there is a
    // valid substitution for this track. If there is, we set the
    // active_music_module pointer to the music pack module for the
    // duration of this particular track.
    if (music_packs_active)
    {
        void *handle;

        handle = music_pack_module.RegisterSong(data, len);
        if (handle != NULL)
        {
            active_music_module = &music_pack_module;
            return handle;
        }
    }

    // No substitution for this track, so use the main module.
    active_music_module = music_module;
    if (active_music_module != NULL)
    {
        return active_music_module->RegisterSong(data, len);
    }
    else
    {
        return NULL;
    }
}

void I_UnRegisterSong(void *handle)
{
    if (active_music_module != NULL)
    {
        active_music_module->UnRegisterSong(handle);
    }
}

void I_PlaySong(void *handle, boolean looping)
{
    if (active_music_module != NULL)
    {
        active_music_module->PlaySong(handle, looping);
    }
}

void I_StopSong(void)
{
    if (active_music_module != NULL)
    {
        active_music_module->StopSong();
    }
}

boolean I_MusicIsPlaying(void)
{
    if (active_music_module != NULL)
    {
        return active_music_module->MusicIsPlaying();
    }
    else
    {
        return false;
    }
}

void I_BindSoundVariables(void)
{
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
#ifdef _WIN32
    M_BindStringVariable("winmm_midi_device",    &winmm_midi_device);
    M_BindIntVariable("winmm_complevel",         &winmm_complevel);
    M_BindIntVariable("winmm_reset_type",        &winmm_reset_type);
    M_BindIntVariable("winmm_reset_delay",       &winmm_reset_delay);
#endif

#ifdef HAVE_FLUIDSYNTH
    M_BindIntVariable("fsynth_chorus_active",       &fsynth_chorus_active);
    M_BindFloatVariable("fsynth_chorus_depth",      &fsynth_chorus_depth);
    M_BindFloatVariable("fsynth_chorus_level",      &fsynth_chorus_level);
    M_BindIntVariable("fsynth_chorus_nr",           &fsynth_chorus_nr);
    M_BindFloatVariable("fsynth_chorus_speed",      &fsynth_chorus_speed);
    M_BindStringVariable("fsynth_midibankselect",   &fsynth_midibankselect);
    M_BindIntVariable("fsynth_polyphony",           &fsynth_polyphony);
    M_BindIntVariable("fsynth_reverb_active",       &fsynth_reverb_active);
    M_BindFloatVariable("fsynth_reverb_damp",       &fsynth_reverb_damp);
    M_BindFloatVariable("fsynth_reverb_level",      &fsynth_reverb_level);
    M_BindFloatVariable("fsynth_reverb_roomsize",   &fsynth_reverb_roomsize);
    M_BindFloatVariable("fsynth_reverb_width",      &fsynth_reverb_width);
    M_BindFloatVariable("fsynth_gain",              &fsynth_gain);
    M_BindStringVariable("fsynth_sf_path",          &fsynth_sf_path);
#endif // HAVE_FLUIDSYNTH

    M_BindIntVariable("use_libsamplerate",       &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale",   &libsamplerate_scale);
}

