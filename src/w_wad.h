// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	WAD I/O functions.
//
//-----------------------------------------------------------------------------


#ifndef __W_WAD__
#define __W_WAD__

#include <stdio.h>

#include "doomtype.h"


//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];		
    int			numlumps;
    int			infotableofs;
} wadinfo_t;


typedef struct
{
    int			filepos;
    int			size;
    char		name[8];
} filelump_t;

//
// WADFILE I/O related stuff.
//

typedef struct lumpinfo_s lumpinfo_t;

struct lumpinfo_s
{
    char	name[8];
    FILE       *handle;
    int		position;
    int		size;
    void       *cache;

    // Used for hash table lookups

    lumpinfo_t *next;
};


extern	void**		lumpcache;
extern	lumpinfo_t*	lumpinfo;
extern	unsigned int	numlumps;

FILE   *W_AddFile (char *filename);
void    W_Reload (void);

int	W_CheckNumForName (char* name);
int	W_GetNumForName (char* name);

int	W_LumpLength (unsigned int lump);
void    W_ReadLump (unsigned int lump, void *dest);

void*	W_CacheLumpNum (int lump, int tag);
void*	W_CacheLumpName (char* name, int tag);

void    W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(const char *s);


#endif
