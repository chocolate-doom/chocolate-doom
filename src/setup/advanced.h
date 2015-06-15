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

#ifndef SETUP_ADVANCED_H
#define SETUP_ADVANCED_H

void AdvancedSettings(void);
void BindAdvancedVariables(void);

extern int startup_delay;
extern int vanilla_savegame_limit;
extern int vanilla_demo_limit;
extern int cn_timer_enabled;

#endif /* #ifndef SETUP_ADVANCED_H */
