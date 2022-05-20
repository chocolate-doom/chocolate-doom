//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//
// [STRIFE] New Module
//
// Strife Hub Saving Code
//

#ifndef M_SAVES_H__
#define M_SAVES_H__

#define CHARACTER_NAME_LEN 32

extern char *savepath;
extern char *savepathtemp;
extern char *loadpath;
extern char character_name[CHARACTER_NAME_LEN];

// Strife Savegame Functions
void ClearTmp(void);
void ClearSlot(void);
void FromCurr(void);
void ToCurr(void);
void M_SaveMoveMapToHere(void);
void M_SaveMoveHereToMap(void);

boolean M_SaveMisObj(const char *path);
void    M_ReadMisObj(void);

// Custom Utilities for Filepath Handling
void *M_Calloc(size_t n1, size_t n2);
void  M_NormalizeSlashes(char *str);
int   M_StringAlloc(char **str, int numstrs, size_t extra, const char *str1, ...);
char *M_SafeFilePath(const char *basepath, const char *newcomponent);
char  M_GetFilePath(const char *fn, char *dest, size_t len);
char *M_MakeStrifeSaveDir(int slotnum, const char *extra);
void  M_CreateSaveDirs(const char *savedir);

#endif

// EOF


