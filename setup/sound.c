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

#include "sound.h"

typedef enum
{
    SFXMODE_DISABLED,
    SFXMODE_PCSPEAKER,
    SFXMODE_DIGITAL,
    NUM_SFXMODES
} sfxmode_t;

typedef enum
{
    MUSMODE_DISABLED,
    MUSMODE_OPL,
    MUSMODE_NATIVE,
    NUM_MUSMODES
} musmode_t;

static char *sfxmode_strings[] = 
{
    "Disabled",
    "PC speaker",
    "Digital",
};

static char *musmode_strings[] =
{
    "Disabled",
    "OPL (Adlib/SB)",
    "Native MIDI"
};

int snd_sfxdevice = SNDDEVICE_SB;
int numChannels = 8;
int sfxVolume = 8;

int snd_musicdevice = SNDDEVICE_GENMIDI;
int musicVolume = 8;

int snd_samplerate = 44100;
int opl_io_port = 0x388;

int use_libsamplerate = 0;

static int snd_sfxmode;
static int snd_musmode;

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

    switch (snd_musmode)
    {
        case MUSMODE_DISABLED:
            snd_musicdevice = SNDDEVICE_NONE;
            break;
        case MUSMODE_OPL:
            snd_musicdevice = SNDDEVICE_SB;
            break;
        case MUSMODE_NATIVE:
            snd_musicdevice = SNDDEVICE_GENMIDI;
            break;
    }
}

void ConfigSound(void)
{
    txt_window_t *window;
    txt_table_t *sfx_table;
    txt_table_t *music_table;
    txt_dropdown_list_t *sfx_mode_control;
    txt_dropdown_list_t *mus_mode_control;

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

    if (snd_musicdevice == SNDDEVICE_GENMIDI)
    {
        snd_musmode = MUSMODE_NATIVE;
    }
    else if (snd_musicdevice == SNDDEVICE_SB
          || snd_musicdevice == SNDDEVICE_ADLIB
          || snd_musicdevice == SNDDEVICE_AWE32)
    {
        snd_musmode = MUSMODE_OPL;
    }
    else
    {
        snd_musmode = MUSMODE_DISABLED;
    }

    window = TXT_NewWindow("Sound configuration");

    TXT_AddWidgets(window,
               TXT_NewSeparator("Sound effects"),
               sfx_table = TXT_NewTable(2),
               TXT_NewSeparator("Music"),
               music_table = TXT_NewTable(2),
               NULL);

    TXT_SetColumnWidths(sfx_table, 20, 14);

    TXT_AddWidgets(sfx_table, 
                   TXT_NewLabel("Sound effects"),
                   sfx_mode_control = TXT_NewDropdownList(&snd_sfxmode,
                                                          sfxmode_strings,
                                                          NUM_SFXMODES),
                   TXT_NewLabel("Sound channels"),
                   TXT_NewSpinControl(&numChannels, 1, 8),
                   TXT_NewLabel("SFX volume"),
                   TXT_NewSpinControl(&sfxVolume, 0, 15),
                   NULL);

    TXT_SetColumnWidths(music_table, 20, 14);

    TXT_AddWidgets(music_table,
                   TXT_NewLabel("Music playback"),
                   mus_mode_control = TXT_NewDropdownList(&snd_musmode,
                                                          musmode_strings,
                                                          NUM_MUSMODES),
                   TXT_NewLabel("Music volume"),
                   TXT_NewSpinControl(&musicVolume, 0, 15),
                   NULL);

    TXT_SignalConnect(sfx_mode_control, "changed",
                      UpdateSndDevices, NULL);
    TXT_SignalConnect(mus_mode_control, "changed",
                      UpdateSndDevices, NULL);
}

