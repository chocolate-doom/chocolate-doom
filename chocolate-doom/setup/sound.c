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

#include "SDL.h"
#include "textscreen.h"

#include "sound.h"

enum
{
    SNDDEVICE_NONE = 0,
    SNDDEVICE_PCSPEAKER = 1,
    SNDDEVICE_ADLIB = 2,
    SNDDEVICE_SB = 3,
    SNDDEVICE_PAS = 4,
    SNDDEVICE_GUS = 5,
    SNDDEVICE_WAVEBLASTER = 6,
    SNDDEVICE_SOUNDCANVAS = 7,
    SNDDEVICE_GENMIDI = 8,
    SNDDEVICE_AWE32 = 9,
};

typedef enum
{
    SFXMODE_DISABLED,
    SFXMODE_PCSPEAKER,
    SFXMODE_DIGITAL,
    NUM_SFXMODES
} sfxmode_t;

static char *sfxmode_strings[] = 
{
    "Disabled",
    "PC speaker",
    "Digital",
};

// Disable MIDI music on OSX: there are problems with the native
// MIDI code in SDL_mixer.

#ifdef __MACOSX__
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_NONE
#else
#define DEFAULT_MUSIC_DEVICE SNDDEVICE_SB
#endif

int snd_sfxdevice = SNDDEVICE_SB;
int numChannels = 8;
int sfxVolume = 15;

int snd_musicdevice = DEFAULT_MUSIC_DEVICE;
int musicVolume = 15;

static int snd_sfxmode;
static int snd_musicenabled;

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
    
    if (snd_musicenabled)
    {
        snd_musicdevice = SNDDEVICE_SB;
    }
    else
    {
        snd_musicdevice = SNDDEVICE_NONE;
    }
}

void ConfigSound(void)
{
    txt_window_t *window;
    txt_table_t *sfx_table;
    txt_table_t *music_table;
    txt_dropdown_list_t *sfx_mode_control;
    txt_checkbox_t *music_enabled_control;

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
    
    snd_musicenabled = snd_musicdevice != SNDDEVICE_NONE;

    window = TXT_NewWindow("Sound configuration");

    TXT_AddWidgets(window,
               TXT_NewSeparator("Sound effects"),
               sfx_table = TXT_NewTable(2),
               TXT_NewSeparator("Music"),
               music_enabled_control = TXT_NewCheckBox("Music enabled", 
                                                       &snd_musicenabled),
               music_table = TXT_NewTable(2),
               NULL);

    TXT_SetColumnWidths(sfx_table, 20, 5);

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

    TXT_SetColumnWidths(music_table, 20, 5);

    TXT_AddWidgets(music_table,
                   TXT_NewLabel("Music volume"),
                   TXT_NewSpinControl(&musicVolume, 0, 15),
                   NULL);

    TXT_SignalConnect(sfx_mode_control, "changed", 
                      UpdateSndDevices, NULL);
    TXT_SignalConnect(music_enabled_control, "changed", 
                      UpdateSndDevices, NULL);

}

