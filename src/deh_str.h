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
// Dehacked string replacements
//
//-----------------------------------------------------------------------------

#ifndef DEH_STR_H
#define DEH_STR_H

#include <stdio.h>

#include "doomfeatures.h"

// Used to do dehacked text substitutions throughout the program

#ifdef FEATURE_DEHACKED

char *DEH_String(char *s);
void DEH_printf(char *fmt, ...);
void DEH_fprintf(FILE *fstream, char *fmt, ...);
void DEH_snprintf(char *buffer, size_t len, char *fmt, ...);
void DEH_AddStringReplacement(char *from_text, char *to_text);


#else

#define DEH_String(x) (x)
#define DEH_printf printf
#define DEH_fprintf fprintf
#define DEH_snprintf snprintf

#endif

#endif /* #ifndef DEH_STR_H */

