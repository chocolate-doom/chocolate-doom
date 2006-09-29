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

int snd_sfxenabled;
int snd_channels = 8;
int sfx_volume = 15;

int snd_musicenabled;
int music_volume = 15;

void ConfigSound(void)
{
        txt_window_t *window;
        txt_table_t *sfx_table;
        txt_table_t *music_table;

        window = TXT_NewWindow("Sound configuration");

        TXT_AddWidgets(window,
                   TXT_NewSeparator("Sound effects"),
                   TXT_NewCheckBox("Sound effects enabled", &snd_sfxenabled),
                   sfx_table = TXT_NewTable(2),
                   TXT_NewSeparator("Music"),
                   TXT_NewCheckBox("Music enabled", &snd_musicenabled),
                   music_table = TXT_NewTable(2),
                   NULL);

        TXT_SetColumnWidths(sfx_table, 20, 5);

        TXT_AddWidgets(sfx_table, 
                       TXT_NewLabel("Sound channels"),
                       TXT_NewSpinControl(&snd_channels, 1, 8),
                       TXT_NewLabel("SFX volume"),
                       TXT_NewSpinControl(&sfx_volume, 0, 15),
                       NULL);

        TXT_SetColumnWidths(music_table, 20, 5);

        TXT_AddWidgets(music_table,
                       TXT_NewLabel("Music volume"),
                       TXT_NewSpinControl(&music_volume, 0, 15),
                       NULL);
}

