// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005 Simon Howard
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
//-----------------------------------------------------------------------------
//
// Dehacked entrypoint and common code
//
//-----------------------------------------------------------------------------

#ifndef DEH_MAIN_H
#define DEH_MAIN_H

#include "doomtype.h"
#include "doomfeatures.h"
#include "md5.h"

// These are the limits that dehacked uses (from dheinit.h in the dehacked
// source).  If these limits are exceeded, it does not generate an error, but
// a warning is displayed.

#define DEH_VANILLA_NUMSTATES 966
#define DEH_VANILLA_NUMSFX 107

void DEH_Init(void);

boolean DEH_ParseAssignment(char *line, char **variable_name, char **value);

void DEH_Checksum(md5_digest_t digest);

// deh_text.c:
//
// Used to do dehacked text substitutions throughout the program

#ifdef FEATURE_DEHACKED

char *DEH_String(char *s);

#else

#define DEH_String(x) (x)

#endif

#endif /* #ifndef DEH_MAIN_H */

