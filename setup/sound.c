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

// Disable MIDI music on OSX: there are problems with the native
// MIDI code in SDL_mixer.

#ifdef __MACOSX__
#define DEFAULT_MUSIC_DEVICE 0
#else
#define DEFAULT_MUSIC_DEVICE 3
#endif

int snd_sfxdevice = 3;
int numChannels = 8;
int sfxVolume = 15;

int snd_musicdevice = DEFAULT_MUSIC_DEVICE;
int musicVolume = 15;

static int snd_sfxenabled;
static int snd_musicenabled;

static void UpdateSndDevices(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    if (snd_sfxenabled)
    {
        snd_sfxdevice = 3;
    }
    else
    {
        snd_sfxdevice = 0;
    }

    if (snd_musicenabled)
    {
        snd_musicdevice = 3;
    }
    else
    {
        snd_musicdevice = 0;
    }
}

void ConfigSound(void)
{
    txt_window_t *window;
    txt_table_t *sfx_table;
    txt_table_t *music_table;
    txt_checkbox_t *sfx_enabled_control;
    txt_checkbox_t *music_enabled_control;

    snd_sfxenabled = snd_sfxdevice != 0;
    snd_musicenabled = snd_musicdevice != 0;

    window = TXT_NewWindow("Sound configuration");

    TXT_AddWidgets(window,
               TXT_NewSeparator("Sound effects"),
               sfx_enabled_control = TXT_NewCheckBox("Sound effects enabled", 
                                                     &snd_sfxenabled),
               sfx_table = TXT_NewTable(2),
               TXT_NewSeparator("Music"),
               music_enabled_control = TXT_NewCheckBox("Music enabled", 
                                                       &snd_musicenabled),
               music_table = TXT_NewTable(2),
               NULL);

    TXT_SetColumnWidths(sfx_table, 20, 5);

    TXT_SignalConnect(sfx_enabled_control, "changed", 
                      UpdateSndDevices, NULL);
    TXT_SignalConnect(music_enabled_control, "changed", 
                      UpdateSndDevices, NULL);

    TXT_AddWidgets(sfx_table, 
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
}

