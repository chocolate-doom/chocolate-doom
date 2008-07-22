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
//       Generate a checksum of the WAD directory.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "w_checksum.h"
#include "w_wad.h"

static wad_file_t **open_wadfiles = NULL;
static int num_open_wadfiles = 0;

static int GetFileNumber(wad_file_t *handle)
{
    int i;
    int result;

    for (i=0; i<num_open_wadfiles; ++i)
    {
        if (open_wadfiles[i] == handle)
        {
            return i;
        }
    }

    // Not found in list.  This is a new file we haven't seen yet.
    // Allocate another slot for this file.

    open_wadfiles = realloc(open_wadfiles,
                            sizeof(wad_file_t *) * (num_open_wadfiles + 1));
    open_wadfiles[num_open_wadfiles] = handle;

    result = num_open_wadfiles;
    ++num_open_wadfiles;

    return result;
}

static void ChecksumAddLump(md5_context_t *md5_context, lumpinfo_t *lump)
{
    char buf[9];

    strncpy(buf, lump->name, 8);
    buf[8] = '\0';
    MD5_UpdateString(md5_context, buf);
    MD5_UpdateInt32(md5_context, GetFileNumber(lump->wad_file));
    MD5_UpdateInt32(md5_context, lump->position);
    MD5_UpdateInt32(md5_context, lump->size);
}

void W_Checksum(md5_digest_t digest)
{
    md5_context_t md5_context;
    unsigned int i;

    MD5_Init(&md5_context);

    num_open_wadfiles = 0;

    // Go through each entry in the WAD directory, adding information
    // about each entry to the MD5 hash.

    for (i=0; i<numlumps; ++i)
    {
        ChecksumAddLump(&md5_context, &lumpinfo[i]);
    }
    
    MD5_Final(digest, &md5_context);
}

