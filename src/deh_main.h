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
//
// Dehacked entrypoint and common code
//

#ifndef DEH_MAIN_H
#define DEH_MAIN_H

#include "doomtype.h"
#include "doomfeatures.h"
#include "deh_str.h"
#include "sha1.h"

// These are the limits that dehacked uses (from dheinit.h in the dehacked
// source).  If these limits are exceeded, it does not generate an error, but
// a warning is displayed.

#define DEH_VANILLA_NUMSTATES 966
#define DEH_VANILLA_NUMSFX 107

void DEH_ParseCommandLine(void);
int DEH_LoadFile(char *filename);
int DEH_LoadLump(int lumpnum, boolean allow_long, boolean allow_error);
int DEH_LoadLumpByName(char *name, boolean allow_long, boolean allow_error);

boolean DEH_ParseAssignment(char *line, char **variable_name, char **value);

void DEH_Checksum(sha1_digest_t digest);

extern boolean deh_allow_extended_strings;
extern boolean deh_allow_long_strings;
extern boolean deh_allow_long_cheats;
extern boolean deh_apply_cheats;

#endif /* #ifndef DEH_MAIN_H */

