// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: z_native.c 443 2006-03-25 21:47:13Z fraggle $
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Zone Memory Allocation. Neat.
//
//	This is an implementation of the zone memory API which
//	uses native calls to malloc() and free().
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: z_native.c 443 2006-03-25 21:47:13Z fraggle $";

#include <stdlib.h>

#include "z_zone.h"
#include "i_system.h"
#include "doomdef.h"

#define ZONEID	0x1d4a11

typedef struct memblock_s memblock_t;

struct memblock_s
{
    int id; // = ZONEID
    int tag;
    void **user;
    memblock_t *prev;
    memblock_t *next;
};

// Linked list of allocated blocks for each tag type
 
static memblock_t *allocated_blocks[PU_NUM_TAGS];

// Add a block into the linked list for its type.

static void Z_InsertBlock(memblock_t *block)
{
    block->prev = NULL;
    block->next = allocated_blocks[block->tag];
    allocated_blocks[block->tag] = block->next;
    
    if (block->next != NULL)
    {
        block->next->prev = block;
    }
}

// Remove a block from its linked list.

static void Z_RemoveBlock(memblock_t *block)
{
    // Unlink from list

    if (block->prev == NULL)
    {
        // Start of list

        allocated_blocks[block->tag] = block->next;
    }
    else
    {
        block->prev->next = block->next;
    }

    if (block->next != NULL)
    {
        block->next->prev = block->prev;
    }
}

//
// Z_Init
//
void Z_Init (void)
{
    memset(allocated_blocks, 0, sizeof(allocated_blocks));
    printf("zone memory: Using native C allocator.\n");
}


//
// Z_Free
//
void Z_Free (void* ptr)
{
    memblock_t*		block;

    block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
        I_Error ("Z_Free: freed a pointer without ZONEID");
		
    if (block->tag != PU_FREE && block->user != NULL)
    {
        // clear the user's mark

        *block->user = NULL;
    }

    Z_RemoveBlock(block);

    // Free back to system

    free(block);
}



//
// Z_Malloc
// You can pass a NULL user if the tag is < PU_PURGELEVEL.
//

void *Z_Malloc(int size, int tag, void *user)
{
    memblock_t *newblock;
    unsigned char *data;
    void *result;

    if (tag < 0 || tag >= PU_NUM_TAGS || tag == PU_FREE)
    {
        I_Error("Z_Malloc: attempted to allocate a block with an invalid "
                "tag: %i", tag);
    }

    if (user == NULL && tag >= PU_PURGELEVEL)
        I_Error ("Z_Malloc: an owner is required for purgable blocks");
    
    newblock = (memblock_t *) malloc(sizeof(memblock_t) + size);
    newblock->tag = tag;
    
    // Hook into the linked list for this tag type

    newblock->id = ZONEID;
    newblock->user = user;

    Z_InsertBlock(newblock);

    data = (unsigned char *) newblock;
    result = data + sizeof(memblock_t);

    if (user != NULL)
    {
        *newblock->user = result;
    }
    
    return result;
}



//
// Z_FreeTags
//

void Z_FreeTags(int lowtag, int hightag)
{
    int i;

    for (i=lowtag; i<= hightag; ++i)
    {
        memblock_t *block;
        memblock_t *next;

        // Free all in this chain

        for (block=allocated_blocks[i]; block != NULL; )
        {
            next = block->next;

            // Free this block

            if (block->user != NULL)
            {
                *block->user = NULL;
            }
            
            free(block);

            // Jump to the next in the chain

            block = next;
        }
    }
}



//
// Z_DumpHeap
//
void Z_DumpHeap(int lowtag, int	hightag)
{
    // broken

#if 0
    memblock_t*	block;
	
    printf ("zone size: %i  location: %p\n",
	    mainzone->size,mainzone);
    
    printf ("tag range: %i to %i\n",
	    lowtag, hightag);
	
    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
	if (block->tag >= lowtag && block->tag <= hightag)
	    printf ("block:%p    size:%7i    user:%p    tag:%3i\n",
		    block, block->size, block->user, block->tag);
		
	if (block->next == &mainzone->blocklist)
	{
	    // all blocks have been hit
	    break;
	}
	
	if ( (byte *)block + block->size != (byte *)block->next)
	    printf ("ERROR: block size does not touch the next block\n");

	if ( block->next->prev != block)
	    printf ("ERROR: next block doesn't have proper back link\n");

	if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	    printf ("ERROR: two consecutive free blocks\n");
    }
#endif
}


//
// Z_FileDumpHeap
//
void Z_FileDumpHeap(FILE *f)
{
    // broken
#if 0
    memblock_t*	block;
	
    fprintf (f,"zone size: %i  location: %p\n",mainzone->size,mainzone);
	
    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
	fprintf (f,"block:%p    size:%7i    user:%p    tag:%3i\n",
		 block, block->size, block->user, block->tag);
		
	if (block->next == &mainzone->blocklist)
	{
	    // all blocks have been hit
	    break;
	}
	
	if ( (byte *)block + block->size != (byte *)block->next)
	    fprintf (f,"ERROR: block size does not touch the next block\n");

	if ( block->next->prev != block)
	    fprintf (f,"ERROR: next block doesn't have proper back link\n");

	if (block->tag == PU_FREE && block->next->tag == PU_FREE)
	    fprintf (f,"ERROR: two consecutive free blocks\n");
    }
#endif
}



//
// Z_CheckHeap
//
void Z_CheckHeap (void)
{
    memblock_t *block;
    memblock_t *prev;
    int i;

    // Check all chains

    for (i=0; i<PU_NUM_TAGS; ++i)
    {
        prev = NULL;

        for (block=allocated_blocks[i]; block != NULL; block = block->next)
        {
            if (block->id != ZONEID)
            {
                I_Error("Z_CheckHeap: Block without a ZONEID!");
            }
            
            if (block->prev != prev)
            {
                I_Error("Z_CheckHeap: Doubly-linked list corrupted!");
            }
            
            prev = block;
        }
    }
}




//
// Z_ChangeTag
//

void Z_ChangeTag2(void *ptr, int tag, char *file, int line)
{
    memblock_t*	block;
	
    block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));

    if (block->id != ZONEID)
        I_Error("%s:%i: Z_ChangeTag: block without a ZONEID!",
                file, line);

    if (tag >= PU_PURGELEVEL && block->user == NULL)
        I_Error("%s:%i: Z_ChangeTag: an owner is required "
                "for purgable blocks", file, line);

    // Remove the block from its current list, and rehook it into
    // its new list.

    Z_RemoveBlock(block);
    block->tag = tag;
    Z_InsertBlock(block);
}


//
// Z_FreeMemory
//

int Z_FreeMemory(void)
{
    // Limited by the system??

    return -1;
}

