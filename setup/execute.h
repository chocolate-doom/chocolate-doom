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

#ifndef TESTCONFIG_H
#define TESTCONFIG_H

#include "textscreen.h"

typedef struct execute_context_s execute_context_t;

#define IWAD_DOOM2    (1 << 0)       /* doom2.wad */
#define IWAD_PLUTONIA (1 << 1)       /* plutonia.wad */
#define IWAD_TNT      (1 << 2)       /* tnt.wad */
#define IWAD_DOOM     (1 << 3)       /* doom.wad */
#define IWAD_DOOM1    (1 << 4)       /* doom1.wad */
#define IWAD_CHEX     (1 << 5)       /* chex.wad */

execute_context_t *NewExecuteContext(void);
void AddCmdLineParameter(execute_context_t *context, char *s, ...);
void PassThroughArguments(execute_context_t *context);
int ExecuteDoom(execute_context_t *context);
int FindInstalledIWADs(void);

txt_window_action_t *TestConfigAction(void);

#endif /* #ifndef TESTCONFIG_H */

