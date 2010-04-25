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

#ifdef _WIN32

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "i_system.h"
#include "w_file.h"
#include "z_zone.h"

// This constant doesn't exist in VC6:

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER 0xffffffff
#endif

typedef struct
{
    wad_file_t wad;
    HANDLE handle;
    HANDLE handle_map;
} win32_wad_file_t;

extern wad_file_class_t win32_wad_file;

static void MapFile(win32_wad_file_t *wad, char *filename)
{
    wad->handle_map = CreateFileMapping(wad->handle,
                                        NULL,
                                        PAGE_WRITECOPY,
                                        0,
                                        0,
                                        NULL);

    if (wad->handle_map == NULL)
    {
        fprintf(stderr, "W_Win32_OpenFile: Unable to CreateFileMapping() "
                        "for %s\n", filename);
        return;
    }

    wad->wad.mapped = MapViewOfFile(wad->handle_map,
                                    FILE_MAP_COPY,
                                    0, 0, 0);

    if (wad->wad.mapped == NULL)
    {
        fprintf(stderr, "W_Win32_OpenFile: Unable to MapViewOfFile() for %s\n",
                        filename);
    }
}

unsigned int GetFileLength(HANDLE handle)
{
    DWORD result;

    result = SetFilePointer(handle, 0, NULL, FILE_END);

    if (result == INVALID_SET_FILE_POINTER)
    {
        I_Error("W_Win32_OpenFile: Failed to read file length");
    }

    return result;
}
   
static wad_file_t *W_Win32_OpenFile(char *path)
{
    win32_wad_file_t *result;
    wchar_t wpath[MAX_PATH + 1];
    HANDLE handle;

    // Open the file:

    MultiByteToWideChar(CP_OEMCP, 0,
                        path, strlen(path) + 1,
                        wpath, sizeof(wpath));

    handle = CreateFileW(wpath,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    // Create a new win32_wad_file_t to hold the file handle.

    result = Z_Malloc(sizeof(win32_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &win32_wad_file;
    result->wad.length = GetFileLength(handle);
    result->handle = handle;

    // Try to map the file into memory with mmap:

    MapFile(result, path);

    return &result->wad;
}

static void W_Win32_CloseFile(wad_file_t *wad)
{
    win32_wad_file_t *win32_wad;

    win32_wad = (win32_wad_file_t *) wad;

    // If mapped, unmap it.

    if (win32_wad->wad.mapped != NULL)
    {
        UnmapViewOfFile(win32_wad->wad.mapped);
    }

    if (win32_wad->handle_map != NULL)
    {
        CloseHandle(win32_wad->handle_map);
    }

    // Close the file
  
    if (win32_wad->handle != NULL)
    {
        CloseHandle(win32_wad->handle);
    }

    Z_Free(win32_wad);
}

// Read data from the specified position in the file into the 
// provided buffer.  Returns the number of bytes read.

size_t W_Win32_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    win32_wad_file_t *win32_wad;
    DWORD bytes_read;
    DWORD result;

    win32_wad = (win32_wad_file_t *) wad;

    // Jump to the specified position in the file.

    result = SetFilePointer(win32_wad->handle, offset, NULL, FILE_BEGIN);

    if (result == INVALID_SET_FILE_POINTER)
    {
        I_Error("W_Win32_Read: Failed to set file pointer to %i",
                offset);
    }

    // Read into the buffer.

    if (!ReadFile(win32_wad->handle, buffer, buffer_len, &bytes_read, NULL))
    {
        I_Error("W_Win32_Read: Error reading from file");
    }

    return bytes_read;
}


wad_file_class_t win32_wad_file = 
{
    W_Win32_OpenFile,
    W_Win32_CloseFile,
    W_Win32_Read,
};


#endif /* #ifdef _WIN32 */

