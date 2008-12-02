// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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

// Sound control menu

#include <stdlib.h>

#include "textscreen.h"
#include "m_config.h"

#include "i_sound.h"
#include "mode.h"
#include "sound.h"

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
    MUSICMODE_MIDI,
    MUSICMODE_CD,
    NUM_MUSICMODES
} musicmode_t;

static char *musicmode_strings[] =
{
    "Disabled",
    "MIDI",
    "CD audio"
};

// Disable MIDI music on OSX: there are problems with the native
// MIDI code in SDL_mixer.

#ifdef __MACOSX__
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_NONE
#else
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_SB
#endif

// Config file variables:

int snd_sfxdevice = SNDDEVICE_SB;
int snd_musicdevice = DEFAULT_MUSIC_DEVICE;
int snd_samplerate = 22050;

static int numChannels = 8;
static int sfxVolume = 15;
static int musicVolume = 15;
static int use_libsamplerate = 0;

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
        case MUSICMODE_MIDI:
            snd_musicdevice = SNDDEVICE_SB;
            break;
        case MUSICMODE_CD:
            break;
    }
}

void ConfigSound(void)
{
    txt_window_t *window;
    txt_table_t *sfx_table;
    txt_table_t *music_table;
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

    if (snd_musicdevice == SNDDEVICE_NONE)
    {
        snd_musicmode = MUSICMODE_DISABLED;
    }
    else if (snd_musicmode == SNDDEVICE_CD)
    {
        snd_musicmode = MUSICMODE_CD;
    }
    else
    {
        snd_musicmode = MUSICMODE_MIDI;
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

    TXT_AddWidgets(window,
               TXT_NewSeparator("Sound effects"),
               sfx_table = TXT_NewTable(2),
               TXT_NewSeparator("Music"),
               music_table = TXT_NewTable(2),
               NULL);

    TXT_SetColumnWidths(sfx_table, 20, 5);

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

    TXT_SetColumnWidths(music_table, 20, 5);

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

}

void BindSoundVariables(void)
{
    M_BindVariable("snd_sfxdevice",       &snd_sfxdevice);
    M_BindVariable("snd_musicdevice",     &snd_musicdevice);
    M_BindVariable("snd_channels",        &numChannels);
    M_BindVariable("sfx_volume",          &sfxVolume);
    M_BindVariable("music_volume",        &musicVolume);
    M_BindVariable("snd_samplerate",      &snd_samplerate);
    M_BindVariable("use_libsamplerate",   &use_libsamplerate);

    M_BindVariable("snd_sbport",          &snd_sbport);
    M_BindVariable("snd_sbirq",          &snd_sbirq);
    M_BindVariable("snd_sbdma",          &snd_sbdma);
    M_BindVariable("snd_mport",          &snd_mport);
}

