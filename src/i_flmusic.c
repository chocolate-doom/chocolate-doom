//
// Copyright(C) 2022 Roman Fomin
// Copyright(C) 2023 Michael Day
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
//      FluidSynth backend

#include "config.h"

#ifdef HAVE_FLUIDSYNTH

#include "fluidsynth.h"

#if (FLUIDSYNTH_VERSION_MAJOR < 2 ||                                           \
     (FLUIDSYNTH_VERSION_MAJOR == 2 && FLUIDSYNTH_VERSION_MINOR < 2))

typedef int fluid_int_t;
typedef long fluid_long_long_t;

#else

typedef fluid_long_long_t fluid_int_t;

#endif

#include "SDL_mixer.h"

#include "doomtype.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_misc.h"
#include "memio.h"
#include "mus2mid.h"

#include <string.h>

char *fsynth_sf_path = "";
int fsynth_chorus_active = 1;
float fsynth_chorus_depth = 5.0f;
float fsynth_chorus_level = 0.35f;
int fsynth_chorus_nr = 3;
float fsynth_chorus_speed = 0.3f;
char *fsynth_midibankselect = "gs";
int fsynth_polyphony = 256;
int fsynth_reverb_active = 1;
float fsynth_reverb_damp = 0.4f;
float fsynth_reverb_level = 0.15f;
float fsynth_reverb_roomsize = 0.6f;
float fsynth_reverb_width = 4.0f;
float fsynth_gain = 1.0f;

static fluid_synth_t *synth = NULL;
static fluid_settings_t *settings = NULL;
static fluid_player_t *player = NULL;

static void FL_Mix_Callback(void *udata, Uint8 *stream, int len)
{
    int result;

    result = fluid_synth_write_s16(synth, len / 4, stream, 0, 2, stream, 1, 2);

    if (result != FLUID_OK)
    {
        fprintf(stderr,
                "FL_Mix_Callback: Error generating FluidSynth audio.\n");
    }
}

static boolean I_FL_InitMusic(void)
{
    int sf_id;

    if (strlen(fsynth_sf_path) == 0)
    {
        fprintf(stderr,
                "I_FL_InitMusic: No FluidSynth soundfont file specified.\n");
        return false;
    }

    settings = new_fluid_settings();

    fluid_settings_setnum(settings, "synth.sample-rate", snd_samplerate);
    fluid_settings_setstr(settings, "synth.midi-bank-select",
                          fsynth_midibankselect);
    fluid_settings_setint(settings, "synth.polyphony", fsynth_polyphony);

    fluid_settings_setint(settings, "synth.chorus.active",
                          fsynth_chorus_active);
    fluid_settings_setint(settings, "synth.reverb.active",
                          fsynth_reverb_active);

    if (fsynth_reverb_active)
    {
        fluid_settings_setnum(settings, "synth.reverb.room-size",
                              fsynth_reverb_roomsize);
        fluid_settings_setnum(settings, "synth.reverb.damp",
                              fsynth_reverb_damp);
        fluid_settings_setnum(settings, "synth.reverb.width",
                              fsynth_reverb_width);
        fluid_settings_setnum(settings, "synth.reverb.level",
                              fsynth_reverb_level);
    }

    if (fsynth_chorus_active)
    {
        fluid_settings_setnum(settings, "synth.chorus.level",
                              fsynth_chorus_level);
        fluid_settings_setnum(settings, "synth.chorus.depth",
                              fsynth_chorus_depth);
        fluid_settings_setint(settings, "synth.chorus.nr", fsynth_chorus_nr);
        fluid_settings_setnum(settings, "synth.chorus.speed",
                              fsynth_chorus_speed);
    }

    if (fsynth_gain < 0.0f)
    {
        fsynth_gain = 0.0f;
    }
    if (fsynth_gain > 10.0f)
    {
        fsynth_gain = 10.0f;
    }

    synth = new_fluid_synth(settings);

    if (synth == NULL)
    {
        fprintf(stderr,
                "I_FL_InitMusic: FluidSynth failed to initialize synth.\n");
        return false;
    }

    sf_id = fluid_synth_sfload(synth, fsynth_sf_path, true);
    if (sf_id == FLUID_FAILED)
    {
        delete_fluid_synth(synth);
        synth = NULL;
        delete_fluid_settings(settings);
        settings = NULL;
        fprintf(stderr,
                "I_FL_InitMusic: Error loading FluidSynth soundfont: '%s'.\n",
                fsynth_sf_path);
        return false;
    }

    printf("I_FL_InitMusic: Using '%s'.\n", fsynth_sf_path);

    return true;
}

static void I_FL_SetMusicVolume(int volume)
{
    if (synth == NULL)
    {
        return;
    }
    // FluidSynth's default is 0.2. Make 1.0 the maximum.
    // 0 -- 0.2 -- 10.0
    fluid_synth_set_gain(synth, ((float) volume / 127) * fsynth_gain);
}

static void I_FL_PauseSong(void)
{
    if (player)
    {
        Mix_HookMusic(NULL, NULL);
    }
}

static void I_FL_ResumeSong(void)
{
    if (player)
    {
        Mix_HookMusic(FL_Mix_Callback, NULL);
    }
}

static void I_FL_PlaySong(void *handle, boolean looping)
{
    if (player)
    {
        fluid_player_set_loop(player, looping ? -1 : 1);
        fluid_player_play(player);
    }
}

static void I_FL_StopSong(void)
{
    if (player)
    {
        fluid_player_stop(player);
    }
}

static void *I_FL_RegisterSong(void *data, int len)
{
    int result = FLUID_FAILED;

    player = new_fluid_player(synth);

    if (player == NULL)
    {
        fprintf(stderr,
                "I_FL_RegisterSong: FluidSynth failed to initialize player.\n");
        return NULL;
    }

    if (IsMid(data, len))
    {
        result = fluid_player_add_mem(player, data, len);

        if (result == FLUID_FAILED)
        {
            fprintf(stderr,
                    "I_FL_RegisterSong: FluidSynth failed to load MIDI.\n");
            return NULL;
        }
    }
    else
    {
        // Assume a MUS file and try to convert
        MEMFILE *instream;
        MEMFILE *outstream;
        void *outbuf;
        size_t outbuf_len;

        instream = mem_fopen_read(data, len);
        outstream = mem_fopen_write();

        if (mus2mid(instream, outstream) == 0)
        {
            mem_get_buf(outstream, &outbuf, &outbuf_len);
            result = fluid_player_add_mem(player, outbuf, outbuf_len);
        }

        mem_fclose(instream);
        mem_fclose(outstream);

        if (result == FLUID_FAILED)
        {
            fprintf(stderr,
                    "I_FL_RegisterSong: FluidSynth failed to load MUS.\n");
            return NULL;
        }
    }

    Mix_HookMusic(FL_Mix_Callback, NULL);
    return player;
}

static void I_FL_UnRegisterSong(void *handle)
{
    if (player)
    {
        fluid_synth_program_reset(synth);
        fluid_synth_system_reset(synth);

        Mix_HookMusic(NULL, NULL);

        delete_fluid_player(player);
        player = NULL;
    }
}

static void I_FL_ShutdownMusic(void)
{
    I_FL_StopSong();
    I_FL_UnRegisterSong(NULL);

    if (synth)
    {
        delete_fluid_synth(synth);
        synth = NULL;
    }

    if (settings)
    {
        delete_fluid_settings(settings);
        settings = NULL;
    }
}

static boolean I_FL_MusicIsPlaying(void)
{
    if (player == NULL)
    {
        return false;
    }

    return (fluid_player_get_status(player) == FLUID_PLAYER_PLAYING);
}

static const snddevice_t music_fl_devices[] =
{
    SNDDEVICE_FSYNTH,
};

const music_module_t music_fl_module =
{
    music_fl_devices,
    arrlen(music_fl_devices),
    I_FL_InitMusic,
    I_FL_ShutdownMusic,
    I_FL_SetMusicVolume,
    I_FL_PauseSong,
    I_FL_ResumeSong,
    I_FL_RegisterSong,
    I_FL_UnRegisterSong,
    I_FL_PlaySong,
    I_FL_StopSong,
    I_FL_MusicIsPlaying,
    NULL // Poll
};

#endif
