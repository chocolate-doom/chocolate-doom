//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2021 Fabian Greffrath
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

#include "accessibility.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-a11y"

int a11y_sector_lighting = 1;
int a11y_extra_lighting = 0;
int a11y_weapon_flash = 1;
int a11y_weapon_pspr = 1;
int a11y_weapon_palette = 1;
int a11y_palette_changes = 1;
int a11y_invul_colormap = 1;

void AccessibilitySettings(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    window = TXT_NewWindow("Accessibility");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidget(window,
                    TXT_NewCheckBox("Flickering Sector Lighting",
                                    &a11y_sector_lighting));

    if (gamemission == doom)
    {
        TXT_AddWidget(window,
                        TXT_NewCheckBox("Weapon Flash Lighting",
                                        &a11y_weapon_flash));
    } 
    else if (gamemission == heretic)
    {
        TXT_AddWidget(window,
                        TXT_NewCheckBox("Gauntlet & Torch Flash",
                                        &a11y_weapon_flash));
    }
    else if (gamemission == hexen)
    {
        TXT_AddWidget(window,
                        TXT_NewCheckBox("Torch Flash",
                                        &a11y_weapon_flash));
    }
  
    if (gamemission != strife)
    {
        TXT_AddWidgets(window,
                        TXT_NewCheckBox("Weapon Flash Sprite",
                                        &a11y_weapon_pspr),
                        TXT_NewCheckBox("Palette Changes",
                                        &a11y_palette_changes),                                      
                        NULL);
    }

    if (gamemission == doom || gamemission == heretic)
    {
        TXT_AddWidget(window,
                        TXT_NewCheckBox("Invulnerability Colormap",
                                        &a11y_invul_colormap));
    } 

    if (gamemission == hexen)
    {
        TXT_AddWidget(window,
                        TXT_NewCheckBox("Ultimate Weapon Palette",
                                        &a11y_weapon_palette));
    }

    if (gamemission != strife)
    {
        TXT_SetTableColumns(window, 2);

        TXT_AddWidgets(window,
                        TXT_NewLabel("Extra Lighting"),
                        TXT_NewSpinControl(&a11y_extra_lighting, 0, 8),
                        NULL);
    }

}

void BindAccessibilityVariables(void)
{
    M_BindIntVariable("a11y_sector_lighting", &a11y_sector_lighting);
    M_BindIntVariable("a11y_extra_lighting",  &a11y_extra_lighting);
    M_BindIntVariable("a11y_weapon_flash",    &a11y_weapon_flash);
    M_BindIntVariable("a11y_weapon_pspr",     &a11y_weapon_pspr);
    M_BindIntVariable("a11y_weapon_palette",  &a11y_weapon_palette);
    M_BindIntVariable("a11y_palette_changes", &a11y_palette_changes);
    M_BindIntVariable("a11y_invul_colormap",  &a11y_invul_colormap);
}
