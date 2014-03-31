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

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

int vanilla_savegame_limit = 0;
int vanilla_demo_limit = 0;

int crispy_automapstats = 0;
int crispy_secretmessage = 0;
int crispy_crosshair = 0;
int crispy_crosshair_highlight = 0;
int crispy_jump = 0;
int crispy_freelook = 0;
int crispy_mouselook = 0;

void CompatibilitySettings(void)
{
    txt_window_t *window;

    if (gamemission == doom)
    {
    window = TXT_NewWindow("Crispness");

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Show level stats in automap",
                                   &crispy_automapstats),
                   TXT_NewCheckBox("Show secrets revealed message",
                                   &crispy_secretmessage),
                   TXT_NewCheckBox("Show laser pointer",
                                   &crispy_crosshair),
                   TXT_NewCheckBox("Change laser pointer color on target",
                                   &crispy_crosshair_highlight),
                   TXT_NewCheckBox("Enable jumping [*]",
                                   &crispy_jump),
                   TXT_NewCheckBox("Enable free look [*]",
                                   &crispy_freelook),
                   TXT_NewCheckBox("Enable permanent mouse look",
                                   &crispy_mouselook),
                   NULL);
    }
    else
    {
    window = TXT_NewWindow("Compatibility");

    TXT_AddWidgets(window, 
                   TXT_NewCheckBox("Vanilla savegame limit",
                                   &vanilla_savegame_limit),
                   TXT_NewCheckBox("Vanilla demo limit",
                                   &vanilla_demo_limit),
                   NULL);
    }
}

void BindCompatibilityVariables(void)
{
    if (gamemission == doom || gamemission == strife)
    {
        M_BindVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
        M_BindVariable("vanilla_demo_limit",     &vanilla_demo_limit);
        if (gamemission == doom)
        {
        M_BindVariable("crispy_automapstats",    &crispy_automapstats);
        M_BindVariable("crispy_secretmessage",   &crispy_secretmessage);
        M_BindVariable("crispy_crosshair",       &crispy_crosshair);
        M_BindVariable("crispy_crosshair_highlight", &crispy_crosshair_highlight);
        M_BindVariable("crispy_jump",            &crispy_jump);
        M_BindVariable("crispy_freelook",        &crispy_freelook);
        M_BindVariable("crispy_mouselook",       &crispy_mouselook);
        }
    }
}

