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

// Sound control menu

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "SDL_mixer.h"

#include "textscreen.h"
#include "m_config.h"
#include "m_misc.h"

#include "execute.h"
#include "mode.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-sound"

typedef enum
{
    OPLMODE_OPL2,
    OPLMODE_OPL3,
    NUM_OPLMODES,
} oplmode_t;

static const char *opltype_strings[] =
{
    "OPL2",
    "OPL3"
};

static const char *cfg_extension[] = { "cfg", NULL };
static const char *sf_extension[] = { "sf2", "sf3", NULL };

// Config file variables:

int snd_sfxdevice = SNDDEVICE_SB;
int snd_musicdevice = SNDDEVICE_SB;
int snd_samplerate = 44100;
int opl_io_port = 0x388;
int snd_cachesize = 64 * 1024 * 1024;
int snd_maxslicetime_ms = 28;
char *snd_musiccmd = "";
int snd_pitchshift = 0;
char *snd_dmxoption = "-opl3"; // [crispy] default to OPL3 emulation

static int numChannels = 8;
static int sfxVolume = 8;
static int musicVolume = 8;
static int voiceVolume = 15;
static int show_talk = 0;
// [crispy] values 3 and higher might reproduce DOOM.EXE more accurately,
// but 1 is closer to "use_libsamplerate = 0" which is the default in Choco
// and causes only a short delay at startup
int use_libsamplerate = 1;
float libsamplerate_scale = 0.65;

char *music_pack_path = NULL;
char *timidity_cfg_path = NULL;
char *fluidsynth_sf_path = NULL;
static char *gus_patch_path = NULL;
static int gus_ram_kb = 1024;
#ifdef _WIN32
#define MAX_MIDI_DEVICES 20
static char **midi_names;
static int midi_num_devices;
static int midi_index;
char *winmm_midi_device = NULL;
int winmm_reset_type = -1;
int winmm_reset_delay = 0;
int winmm_reverb_level = -1;
int winmm_chorus_level = -1;
#endif

// DOS specific variables: these are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq = 0;
static int snd_sbdma = 0;
static int snd_mport = 0;

static int snd_oplmode;

static void UpdateSndDevices(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    switch (snd_oplmode)
    {
        default:
        case OPLMODE_OPL2:
            snd_dmxoption = "";
            break;

        case OPLMODE_OPL3:
            snd_dmxoption = "-opl3";
            break;
    }
}

static txt_dropdown_list_t *OPLTypeSelector(void)
{
    txt_dropdown_list_t *result;

    if (snd_dmxoption != NULL && strstr(snd_dmxoption, "-opl3") != NULL)
    {
        snd_oplmode = OPLMODE_OPL3;
    }
    else
    {
        snd_oplmode = OPLMODE_OPL2;
    }

    result = TXT_NewDropdownList(&snd_oplmode, opltype_strings, 2);

    TXT_SignalConnect(result, "changed", UpdateSndDevices, NULL);

    return result;
}

static void OpenMusicPackDir(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    if (!OpenFolder(music_pack_path))
    {
        TXT_MessageBox("Error", "Failed to open music pack directory.");
    }
}

#ifdef _WIN32
static void UpdateMidiDevice(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    free(winmm_midi_device);
    winmm_midi_device = M_StringDuplicate(midi_names[midi_index]);
}

static txt_dropdown_list_t *MidiDeviceSelector(void)
{
    txt_dropdown_list_t *result;
    int all_devices;
    int device_ids[MAX_MIDI_DEVICES];
    MMRESULT mmr;
    MIDIOUTCAPS mcaps;
    int i;

    if (midi_num_devices > 0)
    {
        for (i = 0; i < midi_num_devices; ++i)
        {
            free(midi_names[i]);
            midi_names[i] = NULL;
        }
        free(midi_names);
        midi_names = NULL;
    }
    midi_num_devices = 0;

    // get the number of midi devices on this system
    all_devices = midiOutGetNumDevs() + 1; // include MIDI_MAPPER
    if (all_devices > MAX_MIDI_DEVICES)
    {
        all_devices = MAX_MIDI_DEVICES;
    }

    // get the valid device ids only, starting from -1 (MIDI_MAPPER)
    for (i = 0; i < all_devices; ++i)
    {
        mmr = midiOutGetDevCaps(i - 1, &mcaps, sizeof(mcaps));
        if (mmr == MMSYSERR_NOERROR)
        {
            device_ids[midi_num_devices] = i - 1;
            midi_num_devices++;
        }
    }

    // get the device names
    midi_names = malloc(midi_num_devices * sizeof(char *));
    for (i = 0; i < midi_num_devices; ++i)
    {
        mmr = midiOutGetDevCaps(device_ids[i], &mcaps, sizeof(mcaps));
        if (mmr == MMSYSERR_NOERROR)
        {
            midi_names[i] = M_StringDuplicate(mcaps.szPname);
        }
    }

    // set the dropdown list index to the previously selected device
    for (i = 0; i < midi_num_devices; ++i)
    {
        if (winmm_midi_device != NULL &&
            strstr(winmm_midi_device, midi_names[i]))
        {
            midi_index = i;
            break;
        }
        else if (winmm_midi_device == NULL || i == midi_num_devices - 1)
        {
            // give up and use MIDI_MAPPER
            midi_index = 0;
            free(winmm_midi_device);
            winmm_midi_device = M_StringDuplicate(midi_names[0]);
            break;
        }
    }

    result = TXT_NewDropdownList(&midi_index, (const char **)midi_names,
                                 midi_num_devices);
    TXT_SignalConnect(result, "changed", UpdateMidiDevice, NULL);

    return result;
}
#endif

void ConfigSound(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_window_action_t *music_action;
#ifdef _WIN32
    int window_ypos = 2;
#else
    int window_ypos = 3;
#endif

    // Build the window

    window = TXT_NewWindow("Sound configuration");
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 40);
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, window_ypos);

    music_action = TXT_NewWindowAction('m', "Music Packs");
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, music_action);
    TXT_SignalConnect(music_action, "pressed", OpenMusicPackDir, NULL);

    TXT_AddWidgets(window,
        TXT_NewSeparator("Sound effects"),
        TXT_NewRadioButton("Disabled", &snd_sfxdevice, SNDDEVICE_NONE),
        TXT_If(gamemission == doom,
            TXT_NewRadioButton("PC speaker effects", &snd_sfxdevice,
                               SNDDEVICE_PCSPEAKER)),
        TXT_NewRadioButton("Digital sound effects",
                           &snd_sfxdevice,
                           SNDDEVICE_SB),
        TXT_If(gamemission == doom || gamemission == heretic
            || gamemission == hexen,
            TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
                TXT_NewHorizBox(
                    TXT_NewStrut(4, 0),
                    TXT_NewCheckBox("Pitch-shifted sounds", &snd_pitchshift),
                    NULL))),
        TXT_If(gamemission == strife,
            TXT_NewConditional(&snd_sfxdevice, SNDDEVICE_SB,
                TXT_NewHorizBox(
                    TXT_NewStrut(4, 0),
                    TXT_NewCheckBox("Show text with voices", &show_talk),
                    NULL))),

        TXT_NewSeparator("Music"),
        TXT_NewRadioButton("Disabled", &snd_musicdevice, SNDDEVICE_NONE),

        TXT_NewRadioButton("OPL (Adlib/Soundblaster)", &snd_musicdevice,
                           SNDDEVICE_SB),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_SB,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewLabel("Chip type: "),
                OPLTypeSelector(),
                NULL)),

        TXT_NewRadioButton("GUS (emulated)", &snd_musicdevice, SNDDEVICE_GUS),
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GUS,
            TXT_MakeTable(2,
                TXT_NewStrut(4, 0),
                TXT_NewLabel("Path to patch files: "),
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&gus_patch_path, 34,
                                    "Select directory containing GUS patches",
                                    TXT_DIRECTORY),
                NULL)),

        TXT_NewRadioButton("Native MIDI", &snd_musicdevice, SNDDEVICE_GENMIDI),
#ifdef _WIN32
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GENMIDI,
            TXT_NewHorizBox(
                TXT_NewStrut(4, 0),
                TXT_NewLabel("Device: "),
                MidiDeviceSelector(),
                NULL)),
#endif
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GENMIDI,
            TXT_MakeTable(2,
                TXT_NewStrut(4, 0),
                TXT_NewLabel("Timidity configuration file: "),
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&timidity_cfg_path, 34,
                                    "Select Timidity config file",
                                    cfg_extension),
                TXT_NewStrut(4, 0),
                TXT_NewLabel("FluidSynth soundfont file: "),
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&fluidsynth_sf_path, 34,
                                    "Select FluidSynth soundfont file",
                                    sf_extension),
                NULL)),
        NULL);
}

void BindSoundVariables(void)
{
    M_BindIntVariable("snd_sfxdevice",            &snd_sfxdevice);
    M_BindIntVariable("snd_musicdevice",          &snd_musicdevice);
    M_BindIntVariable("snd_channels",             &numChannels);
    M_BindIntVariable("snd_samplerate",           &snd_samplerate);
    M_BindIntVariable("sfx_volume",               &sfxVolume);
    M_BindIntVariable("music_volume",             &musicVolume);

    M_BindIntVariable("use_libsamplerate",        &use_libsamplerate);
    M_BindFloatVariable("libsamplerate_scale",    &libsamplerate_scale);

    M_BindIntVariable("gus_ram_kb",               &gus_ram_kb);
    M_BindStringVariable("gus_patch_path",        &gus_patch_path);
    M_BindStringVariable("music_pack_path",     &music_pack_path);
    M_BindStringVariable("timidity_cfg_path",     &timidity_cfg_path);
    M_BindStringVariable("fluidsynth_sf_path",    &fluidsynth_sf_path);
#ifdef _WIN32
    M_BindStringVariable("winmm_midi_device",     &winmm_midi_device);
    M_BindIntVariable("winmm_reset_type",         &winmm_reset_type);
    M_BindIntVariable("winmm_reset_delay",        &winmm_reset_delay);
    M_BindIntVariable("winmm_reverb_level",       &winmm_reverb_level);
    M_BindIntVariable("winmm_chorus_level",       &winmm_chorus_level);
#endif

    M_BindIntVariable("snd_sbport",               &snd_sbport);
    M_BindIntVariable("snd_sbirq",                &snd_sbirq);
    M_BindIntVariable("snd_sbdma",                &snd_sbdma);
    M_BindIntVariable("snd_mport",                &snd_mport);
    M_BindIntVariable("snd_maxslicetime_ms",      &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd",          &snd_musiccmd);
    M_BindStringVariable("snd_dmxoption",         &snd_dmxoption);

    M_BindIntVariable("snd_cachesize",            &snd_cachesize);
    M_BindIntVariable("opl_io_port",              &opl_io_port);

    M_BindIntVariable("snd_pitchshift",           &snd_pitchshift);

    if (gamemission == strife)
    {
        M_BindIntVariable("voice_volume",         &voiceVolume);
        M_BindIntVariable("show_talk",            &show_talk);
    }

    music_pack_path = M_StringDuplicate("");
    timidity_cfg_path = M_StringDuplicate("");
    gus_patch_path = M_StringDuplicate("");
    fluidsynth_sf_path = M_StringDuplicate("");

    // All versions of Heretic and Hexen did pitch-shifting.
    // Most versions of Doom did not and Strife never did.
    snd_pitchshift = gamemission == heretic || gamemission == hexen;

    // Default sound volumes - different games use different values.

    switch (gamemission)
    {
        case doom:
        default:
            sfxVolume = 8;  musicVolume = 8;
            break;
        case heretic:
        case hexen:
            sfxVolume = 10; musicVolume = 10;
            break;
        case strife:
            sfxVolume = 8;  musicVolume = 13;
            break;
    }
}

