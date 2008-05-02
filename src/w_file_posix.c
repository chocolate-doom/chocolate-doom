// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2008 Simon Howard
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

#include "config.h"

#ifdef HAVE_MMAP

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    int handle;
} posix_wad_file_t;

extern wad_file_class_t posix_wad_file;

static void MapFile(posix_wad_file_t *wad, char *filename)
{
    void *result;
    int protection;
    int flags;

    // Mapped area can be read and written to.  Ideally
    // this should be read-only, as none of the Doom code should 
    // change the WAD files after being read.  However, there may
    // be code lurking in the source that does.

    protection = PROT_READ|PROT_WRITE;

    // Writes to the mapped area result in private changes that are
    // *not* written to disk.

    flags = MAP_PRIVATE;

    result = mmap(NULL, wad->wad.length,
                  protection, flags, 
                  wad->handle, 0);

    wad->wad.mapped = result;

    if (result == NULL)
    {
        fprintf(stderr, "W_POSIX_OpenFile: Unable to mmap() %s - %s\n",
                        filename, strerror(errno));
    }
}

unsigned int GetFileLength(int handle)
{
    return lseek(handle, 0, SEEK_END);
}
   
static wad_file_t *W_POSIX_OpenFile(char *path)
{
    posix_wad_file_t *result;
    int handle;

    handle = open(path, 0);

    if (handle < 0)
    {
        return NULL;
    }

    // Create a new posix_wad_file_t to hold the file handle.

    result = Z_Malloc(sizeof(posix_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &posix_wad_file;
    result->wad.length = GetFileLength(handle);
    result->handle = handle;

    // Try to map the file into memory with mmap:

    MapFile(result, path);

    return &result->wad;
}

static void W_POSIX_CloseFile(wad_file_t *wad)
{
    posix_wad_file_t *posix_wad;

    posix_wad = (posix_wad_file_t *) wad;

    // If mapped, unmap it.

    // Close the file
  
    close(posix_wad->handle);
    Z_Free(posix_wad);
}

// Read data from the specified position in the file into the 
// provided buffer.  Returns the number of bytes read.

size_t W_POSIX_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    posix_wad_file_t *posix_wad;
    byte *byte_buffer;
    size_t bytes_read;
    int result;

    posix_wad = (posix_wad_file_t *) wad;

    // Jump to the specified position in the file.

    lseek(posix_wad->handle, offset, SEEK_SET);

    // Read into the buffer.

    bytes_read = 0;
    byte_buffer = buffer;

    while (buffer_len > 0) {
        result = read(posix_wad->handle, byte_buffer, buffer_len);

        if (result < 0) {
            perror("W_POSIX_Read");
            break;
        } else if (result == 0) {
            break;
        }

        // Successfully read some bytes

        byte_buffer += result;
        buffer_len -= result;
        bytes_read += result;
    }

    return bytes_read;
}


wad_file_class_t posix_wad_file = 
{
    W_POSIX_OpenFile,
    W_POSIX_CloseFile,
    W_POSIX_Read,
};


#endif /* #ifdef HAVE_MMAP */

