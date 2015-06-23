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

#define WINDOW_HELP_URL "http://www.chocolate-doom.org/setup-compat"

int vanilla_savegame_limit = 0;
int vanilla_demo_limit = 0;

// [crispy]
int crispy_automapstats = 0;
int crispy_centerweapon = 0;
int crispy_coloredblood = 0;
int crispy_coloredblood2 = 0;
int crispy_coloredhud = 0;
int crispy_crosshair = 0;
int crispy_flipcorpses = 0;
int crispy_freeaim = 0;
int crispy_freelook = 0;
int crispy_jump = 0;
int crispy_mouselook = 0;
int crispy_overunder = 0;
int crispy_pitch = 0;
int crispy_recoil = 0;
int crispy_secretmessage = 0;
int crispy_translucency = 0;
int crispy_uncapped = 0;

void CompatibilitySettings(void)
{
    txt_window_t *window;
    txt_scrollpane_t *scrollpane;

    // [crispy]
    if (gamemission == doom)
    {
    window = TXT_NewWindow("Crispness");

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Visual"),
                   TXT_NewCheckBox("Uncapped Framerate",
                                   &crispy_uncapped),
                   TXT_NewCheckBox("Colorize Status Bar and Texts",
                                   &crispy_coloredhud),
                   TXT_NewCheckBox("Enable Translucency",
                                   &crispy_translucency),
                   TXT_NewCheckBox("Enable Colored Blood",
                                   &crispy_coloredblood),
//                   TXT_NewCheckBox("Fix Spectre and Lost Soul Blood",
//                                   &crispy_coloredblood2),
                   TXT_NewCheckBox("Randomly Mirrored Corpses",
                                   &crispy_flipcorpses),
                   TXT_NewSeparator("Tactical"),
                   TXT_NewCheckBox("Allow Free Look [*]",
                                   &crispy_freelook),
                   TXT_NewCheckBox("Permanent Mouse Look",
                                   &crispy_mouselook),
                   TXT_NewCheckBox("Draw Crosshair",
                                   &crispy_crosshair),
//                   TXT_NewCheckBox("Center Weapon when Firing",
//                                   &crispy_centerweapon),
//                   TXT_NewCheckBox("Enable Weapon Recoil Pitch",
//                                   &crispy_pitch),
                   TXT_NewCheckBox("Show Revealed Secrets",
                                   &crispy_secretmessage),
                   TXT_NewCheckBox("Show Level Stats in Automap",
                                   &crispy_automapstats),
                   TXT_NewSeparator("Physical"),
                   TXT_NewCheckBox("Allow Jumping [*]",
                                   &crispy_jump),
                   TXT_NewCheckBox("Enable Vertical Aiming",
                                   &crispy_freeaim),
                   TXT_NewCheckBox("Walk over/under Monsters",
                                   &crispy_overunder),
                   TXT_NewCheckBox("Enable Weapon Recoil Thrust",
                                   &crispy_recoil),
                   NULL);
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
    if (gamemission == doom || gamemission == strife)
    {
        M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
        M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
        // [crispy]
        if (gamemission == doom)
        {
        M_BindIntVariable("crispy_automapstats",    &crispy_automapstats);
        M_BindIntVariable("crispy_centerweapon",    &crispy_centerweapon);
        M_BindIntVariable("crispy_coloredblood",    &crispy_coloredblood);
        M_BindIntVariable("crispy_coloredblood2",   &crispy_coloredblood2);
        M_BindIntVariable("crispy_coloredhud",      &crispy_coloredhud);
        M_BindIntVariable("crispy_crosshair",       &crispy_crosshair);
        M_BindIntVariable("crispy_flipcorpses",     &crispy_flipcorpses);
        M_BindIntVariable("crispy_freeaim",         &crispy_freeaim);
        M_BindIntVariable("crispy_freelook",        &crispy_freelook);
        M_BindIntVariable("crispy_jump",            &crispy_jump);
        M_BindIntVariable("crispy_mouselook",       &crispy_mouselook);
        M_BindIntVariable("crispy_overunder",       &crispy_overunder);
        M_BindIntVariable("crispy_pitch",           &crispy_pitch);
        M_BindIntVariable("crispy_recoil",          &crispy_recoil);
        M_BindIntVariable("crispy_secretmessage",   &crispy_secretmessage);
        M_BindIntVariable("crispy_translucency",    &crispy_translucency);
        M_BindIntVariable("crispy_uncapped",        &crispy_uncapped);
        }
    }
}

