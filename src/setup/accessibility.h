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

#ifndef SETUP_ACCESSIBILITY_H
#define SETUP_ACCESSIBILITY_H

void AccessibilitySettings(void *widget, void *user_data);
void BindAccessibilityVariables(void);

extern int a11y_sector_lighting;
extern int a11y_extra_lighting;
extern int a11y_weapon_flash;
extern int a11y_weapon_pspr;
extern int a11y_weapon_palette;
extern int a11y_palette_changes;
extern int a11y_invul_colormap;

#endif /* #ifndef SETUP_ACCESSIBILITY_H */
