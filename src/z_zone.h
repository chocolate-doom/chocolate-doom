// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//	Remark: this was the only stuff that, according
//	 to John Carmack, might have been useful for
//	 Quake.
//
//---------------------------------------------------------------------



#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stdio.h>

//
// ZONE MEMORY
// PU - purge tags.

enum
{
    PU_STATIC = 1,                  // static entire execution time
    PU_SOUND,                       // static while playing
    PU_MUSIC,                       // static while playing
    PU_FREE,                        // a free block
    PU_LEVEL,                       // static until level exited
    PU_LEVSPEC,                     // a special thinker in a level
    
    // Tags >= PU_PURGELEVEL are purgable whenever needed.

    PU_PURGELEVEL,
    PU_CACHE,

    // Total number of different tag types

    PU_NUM_TAGS
};
        

void	Z_Init (void);
void*	Z_Malloc (int size, int tag, void *ptr);
void    Z_Free (void *ptr);
void    Z_FreeTags (int lowtag, int hightag);
void    Z_DumpHeap (int lowtag, int hightag);
void    Z_FileDumpHeap (FILE *f);
void    Z_CheckHeap (void);
void    Z_ChangeTag2 (void *ptr, int tag, char *file, int line);
int     Z_FreeMemory (void);


//
// This is used to get the local FILE:LINE info from CPP
// prior to really call the function in question.
//
#define Z_ChangeTag(p,t)                                       \
    Z_ChangeTag2((p), (t), __FILE__, __LINE__)


#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.3  2005/09/22 12:58:46  fraggle
// Use a new PU_FREE tag to mark free blocks, rather than the 'user' field
// (avoids using magic numbers to mark allocated blocks with no user)
//
// Revision 1.2  2005/07/23 16:44:57  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:19:55  fraggle
// Initial import
//
//
//-----------------------------------------------------------------------------
