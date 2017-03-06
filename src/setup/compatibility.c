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

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"

#include "compatibility.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-compat"

int vanilla_savegame_limit = 0;
int vanilla_demo_limit = 0;

// [crispy]
int crispy_automapstats = 0;
int crispy_centerweapon = 0;
int crispy_coloredblood = 0;
int crispy_coloredhud = 0;
int crispy_crosshair = 0;
int crispy_extsaveg = 1;
int crispy_flipcorpses = 0;
int crispy_freeaim = 0;
int crispy_freelook = 0;
int crispy_fullsounds = 0;
int crispy_jump = 0;
int crispy_mouselook = 0;
int crispy_neghealth = 0;
int crispy_overunder = 0;
int crispy_pitch = 0;
int crispy_recoil = 0;
int crispy_secretmessage = 0;
int crispy_translucency = 0;
int crispy_uncapped = 0;

void CompatibilitySettings(void)
{
    txt_window_t *window;

    // [crispy]
    if (gamemission == doom)
    {
        TXT_MessageBox(NULL, "Please refer to the in-game Crispness menu.");
    }
    else
    {
    window = TXT_NewWindow("Compatibility");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

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
    // [crispy]
    if (gamemission == doom)
    {
        M_BindIntVariable("crispy_automapstats",    &crispy_automapstats);
        M_BindIntVariable("crispy_centerweapon",    &crispy_centerweapon);
        M_BindIntVariable("crispy_coloredblood",    &crispy_coloredblood);
        M_BindIntVariable("crispy_coloredhud",      &crispy_coloredhud);
        M_BindIntVariable("crispy_crosshair",       &crispy_crosshair);
        M_BindIntVariable("crispy_extsaveg",        &crispy_extsaveg);
        M_BindIntVariable("crispy_flipcorpses",     &crispy_flipcorpses);
        M_BindIntVariable("crispy_freeaim",         &crispy_freeaim);
        M_BindIntVariable("crispy_freelook",        &crispy_freelook);
        M_BindIntVariable("crispy_fullsounds",      &crispy_fullsounds);
        M_BindIntVariable("crispy_jump",            &crispy_jump);
        M_BindIntVariable("crispy_mouselook",       &crispy_mouselook);
        M_BindIntVariable("crispy_neghealth",       &crispy_neghealth);
        M_BindIntVariable("crispy_overunder",       &crispy_overunder);
        M_BindIntVariable("crispy_pitch",           &crispy_pitch);
        M_BindIntVariable("crispy_recoil",          &crispy_recoil);
        M_BindIntVariable("crispy_secretmessage",   &crispy_secretmessage);
        M_BindIntVariable("crispy_translucency",    &crispy_translucency);
        M_BindIntVariable("crispy_uncapped",        &crispy_uncapped);
    }
    else
    {
    M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
    M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    }
}

