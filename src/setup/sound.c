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

#include <stdlib.h>

#include "SDL_mixer.h"

#include "textscreen.h"
#include "m_config.h"
#include "m_misc.h"

#include "mode.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-sound"

typedef enum
{
    OPLMODE_OPL2,
    OPLMODE_OPL3,
    NUM_OPLMODES,
} oplmode_t;

static char *opltype_strings[] =
{
    "OPL2",
    "OPL3"
};

static char *cfg_extension[] = { "cfg", NULL };

// Config file variables:

int snd_sfxdevice = SNDDEVICE_SB;
int snd_musicdevice = SNDDEVICE_SB;
int snd_samplerate = 44100;
int opl_io_port = 0x388;
int snd_cachesize = 64 * 1024 * 1024;
int snd_maxslicetime_ms = 28;
char *snd_musiccmd = "";
int snd_pitchshift = 0;
char *snd_dmxoption = "";

static int numChannels = 8;
static int sfxVolume = 8;
static int musicVolume = 8;
static int voiceVolume = 15;
static int show_talk = 0;
static int use_libsamplerate = 0;
static float libsamplerate_scale = 0.65;

static char *music_pack_path = NULL;
static char *timidity_cfg_path = NULL;
static char *gus_patch_path = NULL;
static int gus_ram_kb = 1024;

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

void ConfigSound(void)
{
    txt_window_t *window;

    // Build the window

    window = TXT_NewWindow("Sound configuration");
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetColumnWidths(window, 40);
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, 3);

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
        TXT_NewConditional(&snd_musicdevice, SNDDEVICE_GENMIDI,
            TXT_MakeTable(2,
                TXT_NewStrut(4, 0),
                TXT_NewLabel("Timidity configuration file: "),
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&timidity_cfg_path, 34,
                                    "Select Timidity config file",
                                    cfg_extension),
                TXT_NewStrut(4, 0),
                TXT_NewLabel("Digital music pack directory: "),
                TXT_NewStrut(4, 0),
                TXT_NewFileSelector(&music_pack_path, 34,
                                    "Select directory containing music pack "
                                    "config files",
                                    TXT_DIRECTORY),
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

