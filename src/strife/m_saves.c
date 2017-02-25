//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2010 James Haley, Samuel Villarreal
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

// For GNU C and POSIX targets, dirent.h should be available. Otherwise, for
// Visual C++, we need to include the win_opendir module.
#if defined(_MSC_VER)
#include <win_opendir.h>
#elif defined(__GNUC__) || defined(POSIX)
#include <dirent.h>
#elif defined(__WATCOMC__)
#include <direct.h>
#else
#error Need an include for dirent.h!
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "z_zone.h"
#include "i_system.h"
#include "d_player.h"
#include "deh_str.h"
#include "doomstat.h"
#include "m_misc.h"
#include "m_saves.h"
#include "p_dialog.h"

//
// File Paths
//
// Strife maintains multiple file paths related to savegames.
//
char *savepath;     // The actual path of the selected saveslot
char *savepathtemp; // The path of the temporary saveslot (strfsav6.ssg)
char *loadpath;     // Path used while loading the game

char character_name[CHARACTER_NAME_LEN]; // Name of "character" for saveslot

//
// ClearTmp
//
// Clear the temporary save directory
//
void ClearTmp(void)
{
    DIR *sp2dir = NULL;
    struct dirent *f = NULL;

    if(savepathtemp == NULL)
        I_Error("you fucked up savedir man!");

    if(!(sp2dir = opendir(savepathtemp)))
        I_Error("ClearTmp: Couldn't open dir %s", savepathtemp);

    while((f = readdir(sp2dir)))
    {
        char *filepath = NULL;

        // haleyjd: skip "." and ".." without assuming they're the
        // first two entries like the original code did.
        if(!strcmp(f->d_name, ".") || !strcmp(f->d_name, ".."))
            continue;

        // haleyjd: use M_SafeFilePath, not sprintf
        filepath = M_SafeFilePath(savepathtemp, f->d_name);
        remove(filepath);

        Z_Free(filepath);
    }

    closedir(sp2dir);
}

//
// ClearSlot
//
// Clear a single save slot folder
//
void ClearSlot(void)
{
    DIR *spdir = NULL;
    struct dirent *f = NULL;

    if(savepath == NULL)
        I_Error("userdir is fucked up man!");

    if(!(spdir = opendir(savepath)))
        I_Error("ClearSlot: Couldn't open dir %s", savepath);

    while((f = readdir(spdir)))
    {
        char *filepath = NULL;

        if(!strcmp(f->d_name, ".") || !strcmp(f->d_name, ".."))
            continue;
        
        // haleyjd: use M_SafeFilePath, not sprintf
        filepath = M_SafeFilePath(savepath, f->d_name);
        remove(filepath);

        Z_Free(filepath);
    }

    closedir(spdir);
}

//
// FromCurr
//
// Copying files from savepathtemp to savepath
//
void FromCurr(void)
{
    DIR *sp2dir = NULL;
    struct dirent *f = NULL;

    if(!(sp2dir = opendir(savepathtemp)))
        I_Error("FromCurr: Couldn't open dir %s", savepathtemp);

    while((f = readdir(sp2dir)))
    {
        byte *filebuffer  = NULL;
        int   filelen     = 0;
        char *srcfilename = NULL;
        char *dstfilename = NULL;

        // haleyjd: skip "." and ".." without assuming they're the
        // first two entries like the original code did.
        if(!strcmp(f->d_name, ".") || !strcmp(f->d_name, ".."))
            continue;

        // haleyjd: use M_SafeFilePath, NOT sprintf.
        srcfilename = M_SafeFilePath(savepathtemp, f->d_name);
        dstfilename = M_SafeFilePath(savepath,     f->d_name);

        filelen = M_ReadFile(srcfilename, &filebuffer);
        M_WriteFile(dstfilename, filebuffer, filelen);

        Z_Free(filebuffer);
        Z_Free(srcfilename);
        Z_Free(dstfilename);
    }

    closedir(sp2dir);
}

//
// ToCurr
//
// Copying files from savepath to savepathtemp
//
void ToCurr(void)
{
    DIR *spdir = NULL;
    struct dirent *f = NULL;

    ClearTmp();

    // BUG: Rogue copypasta'd this error message, which is why we don't know
    // the real original name of this function.
    if(!(spdir = opendir(savepath)))
        I_Error("ClearSlot: Couldn't open dir %s", savepath);

    while((f = readdir(spdir)))
    {
        byte *filebuffer  = NULL;
        int   filelen     = 0;
        char *srcfilename = NULL;
        char *dstfilename = NULL;

        if(!strcmp(f->d_name, ".") || !strcmp(f->d_name, ".."))
            continue;

        // haleyjd: use M_SafeFilePath, NOT sprintf.
        srcfilename = M_SafeFilePath(savepath,     f->d_name);
        dstfilename = M_SafeFilePath(savepathtemp, f->d_name);

        filelen = M_ReadFile(srcfilename, &filebuffer);
        M_WriteFile(dstfilename, filebuffer, filelen);

        Z_Free(filebuffer);
        Z_Free(srcfilename);
        Z_Free(dstfilename);
    }

    closedir(spdir);
}

//
// M_SaveMoveMapToHere
//
// Moves a map to the "HERE" save.
//
void M_SaveMoveMapToHere(void)
{
    char *mapsave  = NULL;
    char *heresave = NULL;
    char tmpnum[33];

    // haleyjd: no itoa available...
    M_snprintf(tmpnum, sizeof(tmpnum), "%d", gamemap);

    // haleyjd: use M_SafeFilePath, not sprintf
    mapsave  = M_SafeFilePath(savepath, tmpnum);
    heresave = M_SafeFilePath(savepath, "here");

    // haleyjd: use M_FileExists, not access
    if(M_FileExists(mapsave))
    {
        remove(heresave);
        rename(mapsave, heresave);
    }

    Z_Free(mapsave);
    Z_Free(heresave);
}

//
// M_SaveMoveHereToMap
//
// Moves the "HERE" save to a map.
//
void M_SaveMoveHereToMap(void)
{
    char *mapsave  = NULL;
    char *heresave = NULL;
    char tmpnum[33];

    // haleyjd: no itoa available...
    M_snprintf(tmpnum, sizeof(tmpnum), "%d", gamemap);

    mapsave  = M_SafeFilePath(savepathtemp, tmpnum);
    heresave = M_SafeFilePath(savepathtemp, "here");

    if(M_FileExists(heresave))
    {
        remove(mapsave);
        rename(heresave, mapsave);
    }

    Z_Free(mapsave);
    Z_Free(heresave);
}

//
// M_SaveMisObj
//
// Writes the mission objective into the MIS_OBJ file.
//
boolean M_SaveMisObj(const char *path)
{
    boolean result;
    char *destpath = NULL;

    // haleyjd 20110210: use M_SafeFilePath, not sprintf
    destpath = M_SafeFilePath(path, "mis_obj");
    result   = M_WriteFile(destpath, mission_objective, OBJECTIVE_LEN);

    Z_Free(destpath);
    return result;
}

//
// M_ReadMisObj
//
// Reads the mission objective from the MIS_OBJ file.
//
void M_ReadMisObj(void)
{
    FILE *f = NULL;
    char *srcpath = NULL;

    // haleyjd: use M_SafeFilePath, not sprintf
    srcpath = M_SafeFilePath(savepathtemp, "mis_obj");

    if((f = fopen(srcpath, "rb")))
    {
        fread(mission_objective, 1, OBJECTIVE_LEN, f);
        fclose(f);
    }

    Z_Free(srcpath);
}

//=============================================================================
//
// Original Routines
//
// haleyjd - None of the below code is derived from Strife itself, but has been
// adapted or created in order to provide secure, portable filepath handling
// for the purposes of savegame support. This is partially needed to allow for
// differences in Choco due to it being multiplatform. The rest exists because
// I cannot stand programming in an impoverished ANSI C environment that
// calls sprintf on fixed-size buffers. :P
//

//
// M_Calloc
//
// haleyjd 20110210 - original routine
// Because Choco doesn't have Z_Calloc O_o
//
void *M_Calloc(size_t n1, size_t n2)
{
    return (n1 *= n2) ? memset(Z_Malloc(n1, PU_STATIC, NULL), 0, n1) : NULL;
}

//
// M_StringAlloc
//
// haleyjd: This routine takes any number of strings and a number of extra
// characters, calculates their combined length, and calls Z_Alloca to create
// a temporary buffer of that size. This is extremely useful for allocation of
// file paths, and is used extensively in d_main.c.  The pointer returned is
// to a temporary Z_Alloca buffer, which lives until the next main loop
// iteration, so don't cache it. Note that this idiom is not possible with the
// normal non-standard alloca function, which allocates stack space.
//
// [STRIFE] - haleyjd 20110210
// This routine is taken from the Eternity Engine and adapted to do without
// Z_Alloca. I need secure string concatenation for filepath handling. The
// only difference from use in EE is that the pointer returned in *str must
// be manually freed.
//
int M_StringAlloc(char **str, int numstrs, size_t extra, const char *str1, ...)
{
    va_list args;
    size_t len = extra;

    if(numstrs < 1)
        I_Error("M_StringAlloc: invalid input\n");

    len += strlen(str1);

    --numstrs;

    if(numstrs != 0)
    {   
        va_start(args, str1);

        while(numstrs != 0)
        {
            const char *argstr = va_arg(args, const char *);

            len += strlen(argstr);

            --numstrs;
        }

        va_end(args);
    }

    ++len;

    *str = (char *)(M_Calloc(1, len));

    return len;
}

//
// M_NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// killough 11/98: rewritten
//
// [STRIFE] - haleyjd 20110210: Borrowed from Eternity and adapted to respect 
// the DIR_SEPARATOR define used by Choco Doom. This routine originated in
// BOOM.
//
void M_NormalizeSlashes(char *str)
{
    char *p;
   
    // Convert all slashes/backslashes to DIR_SEPARATOR
    for(p = str; *p; p++)
    {
        if((*p == '/' || *p == '\\') && *p != DIR_SEPARATOR)
            *p = DIR_SEPARATOR;
    }

    // Remove trailing slashes
    while(p > str && *--p == DIR_SEPARATOR)
        *p = 0;

    // Collapse multiple slashes
    for(p = str; (*str++ = *p); )
        if(*p++ == DIR_SEPARATOR)
            while(*p == DIR_SEPARATOR)
                p++;
}

//
// M_SafeFilePath
//
// haleyjd 20110210 - original routine.
// This routine performs safe, portable concatenation of a base file path
// with another path component or file name. The returned string is Z_Malloc'd
// and should be freed when it has exhausted its usefulness.
//
char *M_SafeFilePath(const char *basepath, const char *newcomponent)
{
    int   newstrlen = 0;
    char *newstr = NULL;

    if (!strcmp(basepath, ""))
    {
        basepath = ".";
    }

    // Always throw in a slash. M_NormalizeSlashes will remove it in the case
    // that either basepath or newcomponent includes a redundant slash at the
    // end or beginning respectively.
    newstrlen = M_StringAlloc(&newstr, 3, 1, basepath, "/", newcomponent);
    M_snprintf(newstr, newstrlen, "%s/%s", basepath, newcomponent);
    M_NormalizeSlashes(newstr);

    return newstr;
}

//
// M_CreateSaveDirs
//
// haleyjd 20110210: Vanilla Strife went tits-up if it didn't have the full set
// of save folders which were created externally by the installer. fraggle says
// that's no good for Choco purposes, and I agree, so this routine will create
// the full set of folders under the configured savegamedir.
//
void M_CreateSaveDirs(const char *savedir)
{
    int i;

    for(i = 0; i < 7; i++)
    {
        char *compositedir;

        // compose the full path by concatenating with savedir
        compositedir = M_SafeFilePath(savedir, M_MakeStrifeSaveDir(i, ""));

        M_MakeDirectory(compositedir);

        Z_Free(compositedir);
    }
}

//
// M_MakeStrifeSaveDir
//
// haleyjd 20110211: Convenience routine
//
char *M_MakeStrifeSaveDir(int slotnum, const char *extra)
{
    static char tmpbuffer[32];

    M_snprintf(tmpbuffer, sizeof(tmpbuffer),
               "strfsav%d.ssg%s", slotnum, extra);

    return tmpbuffer;
}

// 
// M_GetFilePath
//
// haleyjd: STRIFE-FIXME: Temporary?
// Code borrowed from Eternity, and modified to return separator char
//
char M_GetFilePath(const char *fn, char *dest, size_t len)
{
    boolean found_slash = false;
    char *p;
    char sepchar = '\0';

    memset(dest, 0, len);

    p = dest + len - 1;

    M_StringCopy(dest, fn, len);

    while(p >= dest)
    {
        if(*p == '/' || *p == '\\')
        {
            sepchar = *p;
            found_slash = true; // mark that the path ended with a slash
            *p = '\0';
            break;
        }
        *p = '\0';
        p--;
    }

    // haleyjd: in the case that no slash was ever found, yet the
    // path string is empty, we are dealing with a file local to the
    // working directory. The proper path to return for such a string is
    // not "", but ".", since the format strings add a slash now. When
    // the string is empty but a slash WAS found, we really do want to
    // return the empty string, since the path is relative to the root.
    if(!found_slash && *dest == '\0')
        *dest = '.';

    // if a separator is not found, default to forward, because Windows 
    // supports that too.
    if(sepchar == '\0') 
        sepchar = '/';

    return sepchar;
}

// EOF


