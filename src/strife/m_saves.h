// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1996 Rogue Entertainment / Velocity, Inc.
// Copyright(C) 2010 James Haley, Samuel Villareal
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
// DESCRIPTION:
//
// [STRIFE] New Module
//
// Strife Hub Saving Code
//
//-----------------------------------------------------------------------------

#ifndef M_SAVES_H__
#define M_SAVES_H__

// Strife Savegame Functions
boolean M_SaveMisObj(const char *path);

// Custom Utilities for Filepath Handling
void *M_Calloc(size_t n1, size_t n2);
void  M_NormalizeSlashes(char *str);
int   M_StringAlloc(char **str, int numstrs, size_t extra, const char *str1, ...);
char *M_SafeFilePath(const char *basepath, const char *newcomponent);
char  M_GetFilePath(const char *fn, char *dest, size_t len);

#endif

// EOF


