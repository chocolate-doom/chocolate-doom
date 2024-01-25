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
// DESCRIPTION:
//     Common code to parse command line, identifying WAD files to load.
//

#ifndef W_MAIN_H
#define W_MAIN_H

#include "d_mode.h"

// [crispy]
typedef struct
{
    const char *name;
    const char new_name[8];
} lump_rename_t;

boolean W_ParseCommandLine(void);
void W_CheckCorrectIWAD(GameMission_t mission);

int W_MergeDump (const char *file);
int W_LumpDump (const char *lumpname);

// Autoload all .wad files from the given directory:
void W_AutoLoadWADs(const char *path);

// [crispy] Autoload from directory with lump renaming
void W_AutoLoadWADsRename(const char *path, const lump_rename_t *renames,
                          int num_renames);

#endif /* #ifndef W_MAIN_H */

