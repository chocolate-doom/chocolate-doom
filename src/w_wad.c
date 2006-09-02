// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: w_wad.c 596 2006-09-02 19:10:07Z fraggle $
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
// $Log$
// Revision 1.11  2006/02/03 18:41:26  fraggle
// Support NWT-style WAD merging (-af and -as command line parameters).
// Restructure WAD loading so that merged WADs are always loaded before
// normal PWADs.  Remove W_InitMultipleFiles().
//
// Revision 1.10  2006/01/24 01:46:08  fraggle
// More endianness fixes
//
// Revision 1.9  2006/01/10 22:14:13  fraggle
// Shut up compiler warnings
//
// Revision 1.8  2005/10/08 18:22:46  fraggle
// Store the cache as part of the lumpinfo_t struct.  Add W_AddFile prototype
// to header.
//
// Revision 1.7  2005/08/30 22:15:11  fraggle
// More Windows fixes
//
// Revision 1.6  2005/08/30 22:11:10  fraggle
// Windows fixes
//
// Revision 1.5  2005/08/04 18:42:15  fraggle
// Silence compiler warnings
//
// Revision 1.4  2005/08/04 01:13:46  fraggle
// Loading disk
//
// Revision 1.3  2005/07/23 18:50:34  fraggle
// Use standard C file functions for WAD code
//
// Revision 1.2  2005/07/23 16:44:57  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:39  fraggle
// Initial import
//
//
// DESCRIPTION:
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: w_wad.c 596 2006-09-02 19:10:07Z fraggle $";


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "m_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"

#include "w_wad.h"

//
// GLOBALS
//

// Location of each lump on disk.

lumpinfo_t *lumpinfo;		
int numlumps = 0;

// Hash table for fast lookups

static lumpinfo_t **lumphash;

static int FileLength (FILE *handle)
{ 
    long savedpos;
    long length;
    // save the current position in the file
    savedpos = ftell(handle);
    
    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}


static void ExtractFileBase(char *path, char *dest)
{
    char*	src;
    int		length;

    src = path + strlen(path) - 1;
    
    // back up until a \ or the start
    while (src != path
	   && *(src-1) != '\\'
	   && *(src-1) != '/')
    {
	src--;
    }
    
    // copy up to eight characters
    memset (dest,0,8);
    length = 0;
    
    while (*src && *src != '.')
    {
	if (++length == 9)
	    I_Error ("Filename base of %s >8 chars",path);

	*dest++ = toupper((int)*src++);
    }
}

// Hash function used for lump names.
// Must be mod'ed with table size.
// Can be used for any 8-character names.
// by Lee Killough

unsigned int W_LumpNameHash(const char *s)
{
  unsigned int hash;

  ((hash =        toupper(s[0]), s[1]) &&
   (hash = hash*3+toupper(s[1]), s[2]) &&
   (hash = hash*2+toupper(s[2]), s[3]) &&
   (hash = hash*2+toupper(s[3]), s[4]) &&
   (hash = hash*2+toupper(s[4]), s[5]) &&
   (hash = hash*2+toupper(s[5]), s[6]) &&
   (hash = hash*2+toupper(s[6]),
    hash = hash*2+toupper(s[7]))
  );

  return hash;
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

int			reloadlump;
char*			reloadname;


FILE *W_AddFile (char *filename)
{
    wadinfo_t		header;
    lumpinfo_t*		lump_p;
    unsigned		i;
    FILE               *handle;
    int			length;
    int			startlump;
    filelump_t*		fileinfo;
    filelump_t*         filerover;
    FILE               *storehandle;
    
    // open the file and add to directory

    // handle reload indicator.
    if (filename[0] == '~')
    {
	filename++;
	reloadname = filename;
	reloadlump = numlumps;
    }
		
    if ( (handle = fopen(filename,"rb")) == NULL)
    {
	printf (" couldn't open %s\n",filename);
	return NULL;
    }

    startlump = numlumps;
	
    if (strcasecmp(filename+strlen(filename)-3 , "wad" ) )
    {
	// single lump file
	fileinfo = Z_Malloc(sizeof(filelump_t), PU_STATIC, 0);
	fileinfo->filepos = 0;
	fileinfo->size = FileLength(handle);
	ExtractFileBase (filename, fileinfo->name);
	numlumps++;
    }
    else 
    {
	// WAD file
	fread (&header, sizeof(header), 1, handle);
	if (strncmp(header.identification,"IWAD",4))
	{
	    // Homebrew levels?
	    if (strncmp(header.identification,"PWAD",4))
	    {
		I_Error ("Wad file %s doesn't have IWAD "
			 "or PWAD id\n", filename);
	    }
	    
	    // ???modifiedgame = true;		
	}
	header.numlumps = LONG(header.numlumps);
	header.infotableofs = LONG(header.infotableofs);
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = Z_Malloc(length, PU_STATIC, 0);
	fseek(handle, header.infotableofs, SEEK_SET);
	fread(fileinfo, length, 1, handle);
	numlumps += header.numlumps;
    }

    
    // Fill in lumpinfo
    lumpinfo = realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));

    if (!lumpinfo)
	I_Error ("Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];
	
    storehandle = reloadname ? NULL : handle;
	
    for (i=startlump,filerover=fileinfo ; i<numlumps ; i++,lump_p++, filerover++)
    {
	lump_p->handle = storehandle;
	lump_p->position = LONG(filerover->filepos);
	lump_p->size = LONG(filerover->size);
        lump_p->cache = NULL;
	strncpy (lump_p->name, filerover->name, 8);
    }
	
    if (reloadname)
	fclose (handle);

    Z_Free(fileinfo);

    if (lumphash != NULL)
    {
        Z_Free(lumphash);
        lumphash = NULL;
    }

    return handle;
}




//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload (void)
{
    wadinfo_t		header;
    int			lumpcount;
    lumpinfo_t*		lump_p;
    unsigned		i;
    FILE               *handle;
    int			length;
    filelump_t*		fileinfo;
	
    if (!reloadname)
	return;
		
    if ( (handle = fopen(reloadname,"rb")) == NULL)
	I_Error ("W_Reload: couldn't open %s",reloadname);

    fread(&header, sizeof(header), 1, handle);
    lumpcount = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = lumpcount*sizeof(filelump_t);
    fileinfo = Z_Malloc(length, PU_STATIC, 0);
    fseek(handle, header.infotableofs, SEEK_SET);
    fread(fileinfo, length, 1, handle);
    
    // Fill in lumpinfo
    lump_p = &lumpinfo[reloadlump];
	
    for (i=reloadlump ;
	 i<reloadlump+lumpcount ;
	 i++,lump_p++, fileinfo++)
    {
	if (lumpinfo[i].cache)
	    Z_Free (lumpinfo[i].cache);

	lump_p->position = LONG(fileinfo->filepos);
	lump_p->size = LONG(fileinfo->size);
    }
	
    fclose(handle);

    Z_Free(fileinfo);
}



//
// W_NumLumps
//
int W_NumLumps (void)
{
    return numlumps;
}



//
// W_CheckNumForName
// Returns -1 if name not found.
//

int W_CheckNumForName (char* name)
{
    lumpinfo_t *lump_p;
    int i;

    // Do we have a hash table yet?

    if (lumphash != NULL)
    {
        int hash;
        
        // We do! Excellent.

        hash = W_LumpNameHash(name) % numlumps;
        
        for (lump_p = lumphash[hash]; lump_p != NULL; lump_p = lump_p->next)
        {
            if (!strncasecmp(lump_p->name, name, 8))
            {
                return lump_p - lumpinfo;
            }
        }
    } 
    else
    {
        // We don't have a hash table generate yet. Linear search :-(
        // 
        // scan backwards so patch lump files take precedence

        for (i=numlumps-1; i >= 0; --i)
        {
            if (!strncasecmp(lumpinfo[i].name, name, 8))
            {
                return i;
            }
        }
    }

    // TFB. Not found.

    return -1;
}




//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (char* name)
{
    int	i;

    i = W_CheckNumForName (name);
    
    if (i == -1)
      I_Error ("W_GetNumForName: %s not found!", name);
      
    return i;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
    if (lump >= numlumps)
	I_Error ("W_LumpLength: %i >= numlumps",lump);

    return lumpinfo[lump].size;
}



//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void
W_ReadLump
( int		lump,
  void*		dest )
{
    int		c;
    lumpinfo_t*	l;
    FILE       *handle;
	
    if (lump >= numlumps)
	I_Error ("W_ReadLump: %i >= numlumps",lump);

    l = lumpinfo+lump;
	
    I_BeginRead ();
	
    if (l->handle == NULL)
    {
	// reloadable file, so use open / read / close
	if ( (handle = fopen(reloadname,"rb")) == NULL)
	    I_Error ("W_ReadLump: couldn't open %s",reloadname);
    }
    else
	handle = l->handle;
		
    fseek(handle, l->position, SEEK_SET);
    c = fread (dest, 1, l->size, handle);

    if (c < l->size)
	I_Error ("W_ReadLump: only read %i of %i on lump %i",
		 c,l->size,lump);	

    if (l->handle == NULL)
	fclose (handle);
		
    I_EndRead ();
}




//
// W_CacheLumpNum
//
void*
W_CacheLumpNum
( int		lump,
  int		tag )
{
    byte*	ptr;

    if ((unsigned)lump >= numlumps)
	I_Error ("W_CacheLumpNum: %i >= numlumps",lump);
		
    if (!lumpinfo[lump].cache)
    {
	// read the lump in
	
	//printf ("cache miss on lump %i\n",lump);
	ptr = Z_Malloc (W_LumpLength (lump), tag, &lumpinfo[lump].cache);
	W_ReadLump (lump, lumpinfo[lump].cache);
    }
    else
    {
	//printf ("cache hit on lump %i\n",lump);
	Z_ChangeTag (lumpinfo[lump].cache,tag);
    }
	
    return lumpinfo[lump].cache;
}



//
// W_CacheLumpName
//
void*
W_CacheLumpName
( char*		name,
  int		tag )
{
    return W_CacheLumpNum (W_GetNumForName(name), tag);
}

#if 0

//
// W_Profile
//
int		info[2500][10];
int		profilecount;

void W_Profile (void)
{
    int		i;
    memblock_t*	block;
    void*	ptr;
    char	ch;
    FILE*	f;
    int		j;
    char	name[9];
	
	
    for (i=0 ; i<numlumps ; i++)
    {	
	ptr = lumpinfo[i].cache;
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;
	
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
	memcpy (name,lumpinfo[i].name,8);

	for (j=0 ; j<8 ; j++)
	    if (!name[j])
		break;

	for ( ; j<8 ; j++)
	    name[j] = ' ';

	fprintf (f,"%s ",name);

	for (j=0 ; j<profilecount ; j++)
	    fprintf (f,"    %c",info[i][j]);

	fprintf (f,"\n");
    }
    fclose (f);
}


#endif

// Generate a hash table for fast lookups

void W_GenerateHashTable(void)
{
    int i;

    // Free the old hash table, if there is one

    if (lumphash != NULL)
    {
        Z_Free(lumphash);
    }

    // Generate hash table

    lumphash = Z_Malloc(sizeof(lumpinfo_t *) * numlumps, PU_STATIC, NULL);
    memset(lumphash, 0, sizeof(lumpinfo_t *) * numlumps);

    for (i=0; i<numlumps; ++i)
    {
        unsigned int hash;

        hash = W_LumpNameHash(lumpinfo[i].name) % numlumps;

        // Hook into the hash table

        lumpinfo[i].next = lumphash[hash];
        lumphash[hash] = &lumpinfo[i];
    }

    // All done!
}

