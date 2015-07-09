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

#define WINDOW_HELP_URL "http://www.chocolate-doom.org/setup-sound"

typedef enum
{
    SFXMODE_DISABLED,
    SFXMODE_DIGITAL,
    SFXMODE_PCSPEAKER,
    NUM_SFXMODES
} sfxmode_t;

static char *sfxmode_strings[] =
{
    "Disabled",
    "Digital",
    "PC speaker"
};

typedef enum
{
    MUSICMODE_DISABLED,
    MUSICMODE_OPL,
    MUSICMODE_GUS,
    MUSICMODE_NATIVE,
    MUSICMODE_CD,
    NUM_MUSICMODES
} musicmode_t;

static char *musicmode_strings[] =
{
    "Disabled",
    "OPL (Adlib/SB)",
    "GUS (emulated)",
    "Native MIDI",
    "CD audio"
};

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

static int numChannels = 8;
static int sfxVolume = 8;
static int musicVolume = 8;
static int voiceVolume = 15;
static int show_talk = 0;
// [crispy] values 3 and higher might reproduce DOOM.EXE more accurately,
// but 1 is closer to "use_libsamplerate = 0" which is the default in Choco
// and causes only a short delay at startup
static int use_libsamplerate = 1;
static float libsamplerate_scale = 0.65;

static char *snd_dmxoption = "";
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

// GUI variables:

static int snd_sfxmode;
static int snd_musicmode;
static int snd_oplmode;

static void UpdateSndDevices(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    switch (snd_sfxmode)
    {
        case SFXMODE_DISABLED:
            snd_sfxdevice = SNDDEVICE_NONE;
            break;
        case SFXMODE_PCSPEAKER:
            snd_sfxdevice = SNDDEVICE_PCSPEAKER;
            break;
        case SFXMODE_DIGITAL:
            snd_sfxdevice = SNDDEVICE_SB;
            break;
    }

    switch (snd_musicmode)
    {
        case MUSICMODE_DISABLED:
            snd_musicdevice = SNDDEVICE_NONE;
            break;
        case MUSICMODE_NATIVE:
            snd_musicdevice = SNDDEVICE_GENMIDI;
            break;
        case MUSICMODE_OPL:
            snd_musicdevice = SNDDEVICE_SB;
            break;
        case MUSICMODE_GUS:
            snd_musicdevice = SNDDEVICE_GUS;
            break;
        case MUSICMODE_CD:
            snd_musicdevice = SNDDEVICE_CD;
            break;
    }

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

static void UpdateExtraTable(TXT_UNCAST_ARG(widget),
                             TXT_UNCAST_ARG(extra_table))
{
    TXT_CAST_ARG(txt_table_t, extra_table);

    TXT_ClearTable(extra_table);

    switch (snd_musicmode)
    {
        case MUSICMODE_OPL:
            TXT_SetColumnWidths(extra_table, 19, 4);
            TXT_AddWidgets(extra_table,
                           TXT_NewLabel("OPL type"),
                           OPLTypeSelector(),
                           NULL);
            break;

        case MUSICMODE_GUS:
            TXT_AddWidgets(extra_table,
                           TXT_NewLabel("GUS patch path:"),
                           TXT_NewStrut(0, 0),
                           TXT_NewFileSelector(&gus_patch_path, 30,
                                               "Select path to GUS patches",
                                               TXT_DIRECTORY),
                           NULL);
            break;

        case MUSICMODE_NATIVE:
            TXT_AddWidgets(extra_table,
                           TXT_NewLabel("Timidity configuration file:"),
                           TXT_NewStrut(0, 0),
                           TXT_NewFileSelector(&timidity_cfg_path, 30,
                                               "Select Timidity config file",
                                               cfg_extension),
                           NULL);
            break;

        default:
            break;
    }
}

void ConfigSound(void)
{
    txt_window_t *window;
    txt_table_t *sfx_table;
    txt_table_t *music_table;
    txt_table_t *extra_table;
    txt_dropdown_list_t *sfx_mode_control;
    txt_dropdown_list_t *music_mode_control;
    int num_sfx_modes, num_music_modes;

    // Work out what sfx mode we are currently using:

    if (snd_sfxdevice == SNDDEVICE_PCSPEAKER)
    {
        snd_sfxmode = SFXMODE_PCSPEAKER;
    }
    else if (snd_sfxdevice >= SNDDEVICE_SB)
    {
        snd_sfxmode = SFXMODE_DIGITAL;
    }
    else
    {
        snd_sfxmode = SFXMODE_DISABLED;
    }

    // Is music enabled?

    switch (snd_musicdevice)
    {
        case SNDDEVICE_GENMIDI:
            snd_musicmode = MUSICMODE_NATIVE;
            break;
        case SNDDEVICE_CD:
            snd_musicmode = MUSICMODE_CD;
            break;
        case SNDDEVICE_SB:
        case SNDDEVICE_ADLIB:
        case SNDDEVICE_AWE32:
            snd_musicmode = MUSICMODE_OPL;
            break;
        case SNDDEVICE_GUS:
            snd_musicmode = MUSICMODE_GUS;
            break;
        default:
            snd_musicmode = MUSICMODE_DISABLED;
            break;
    }

    // Doom has PC speaker sound effects, but others do not:

    if (gamemission == doom)
    {
        num_sfx_modes = NUM_SFXMODES;
    }
    else
    {
        num_sfx_modes = NUM_SFXMODES - 1;
    }

    // Hexen has CD audio; others do not.

    if (gamemission == hexen)
    {
        num_music_modes = NUM_MUSICMODES;
    }
    else
    {
        num_music_modes = NUM_MUSICMODES - 1;
    }

    // Build the window

    window = TXT_NewWindow("Sound configuration");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP,
                                  TXT_SCREEN_W / 2, 5);

    TXT_AddWidgets(window,
               TXT_NewSeparator("Sound effects"),
               sfx_table = TXT_NewTable(2),
               NULL);

    TXT_SetColumnWidths(sfx_table, 19, 15);

    TXT_AddWidgets(sfx_table,
                   TXT_NewLabel("Sound effects"),
                   sfx_mode_control = TXT_NewDropdownList(&snd_sfxmode,
                                                          sfxmode_strings,
                                                          num_sfx_modes),
                   TXT_NewLabel("Sound channels"),
                   TXT_NewSpinControl(&numChannels, 1, 8),
                   TXT_NewLabel("SFX volume"),
                   TXT_NewSpinControl(&sfxVolume, 0, 15),
                   NULL);

    // strife did not implement pitch shifting at all, so hide the option.

    if (gamemission != strife)
    {
        TXT_AddWidget(sfx_table,
                   TXT_NewCheckBox("Pitch-shift sounds", &snd_pitchshift));
    }

    if (gamemission == strife)
    {
        TXT_AddWidgets(sfx_table,
                       TXT_NewLabel("Voice volume"),
                       TXT_NewSpinControl(&voiceVolume, 0, 15),
                       NULL);
        TXT_AddWidget(window,
                      TXT_NewCheckBox("Show text with voices", &show_talk));
    }

    TXT_AddWidgets(window,
               TXT_NewSeparator("Music"),
               music_table = TXT_NewTable(2),
               extra_table = TXT_NewTable(2),
               NULL);

    TXT_SetColumnWidths(music_table, 19, 15);

    TXT_AddWidgets(music_table,
                   TXT_NewLabel("Music"),
                   music_mode_control = TXT_NewDropdownList(&snd_musicmode,
                                                            musicmode_strings,
                                                            num_music_modes),
                   TXT_NewLabel("Music volume"),
                   TXT_NewSpinControl(&musicVolume, 0, 15),
                   NULL);


    TXT_SignalConnect(sfx_mode_control, "changed", UpdateSndDevices, NULL);
    TXT_SignalConnect(music_mode_control, "changed", UpdateSndDevices, NULL);

    // Update extra_table when the music mode is changed, and build it now.
    TXT_SignalConnect(music_mode_control, "changed",
                      UpdateExtraTable, extra_table);
    UpdateExtraTable(music_mode_control, extra_table);
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

    // Before SDL_mixer version 1.2.11, MIDI music caused the game
    // to crash when it looped.  If this is an old SDL_mixer version,
    // disable MIDI.

#ifdef __MACOSX__
    {
        const SDL_version *v = Mix_Linked_Version();

        if (SDL_VERSIONNUM(v->major, v->minor, v->patch)
          < SDL_VERSIONNUM(1, 2, 11))
        {
            snd_musicdevice = SNDDEVICE_NONE;
        }
    }
#endif
}

