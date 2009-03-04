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
//	System interface for music.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>

#include "doomdef.h"
#include "memio.h"
#include "mus2mid.h"

#include "deh_main.h"
#include "m_misc.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAXMIDLENGTH (96 * 1024)

static boolean music_initialised = false;

//static boolean musicpaused = false;
static int current_music_volume;

// Shutdown music

static void I_OPL_ShutdownMusic(void)
{
}

// Initialise music subsystem

static boolean I_OPL_InitMusic(void)
{ 
    music_initialised = true;

    return true;
}

// Set music volume (0 - 127)

static void I_OPL_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;
}

// Start playing a mid

static void I_OPL_PlaySong(void *handle, int looping)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_PauseSong(void)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_ResumeSong(void)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_StopSong(void)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_UnRegisterSong(void *handle)
{
    if (!music_initialised)
    {
        return;
    }
}

// Determine whether memory block is a .mid file 

static boolean IsMid(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MThd", 4);
}

static boolean ConvertMus(byte *musdata, int len, char *filename)
{
    MEMFILE *instream;
    MEMFILE *outstream;
    void *outbuf;
    size_t outbuf_len;
    int result;

    instream = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();

    result = mus2mid(instream, outstream);

    if (result == 0)
    {
        mem_get_buf(outstream, &outbuf, &outbuf_len);

        M_WriteFile(filename, outbuf, outbuf_len);
    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return result;
}

static void *I_OPL_RegisterSong(void *data, int len)
{
    char *filename;

    if (!music_initialised)
    {
        return NULL;
    }

    // MUS files begin with "MUS"
    // Reject anything which doesnt have this signature

    filename = M_TempFile("doom.mid");

    if (IsMid(data, len) && len < MAXMIDLENGTH)
    {
        M_WriteFile(filename, data, len);
    }
    else 
    {
	// Assume a MUS file and try to convert

        ConvertMus(data, len, filename);
    }

    // ....

    // remove file now

    remove(filename);

    Z_Free(filename);

    return NULL;
}

// Is the song playing?
static boolean I_OPL_MusicIsPlaying(void)
{
    if (!music_initialised)
    {
        return false;
    }

    return false;
}

static snddevice_t music_opl_devices[] =
{
    SNDDEVICE_ADLIB,
    SNDDEVICE_SB,
};

music_module_t music_opl_module =
{
    music_opl_devices,
    arrlen(music_opl_devices),
    I_OPL_InitMusic,
    I_OPL_ShutdownMusic,
    I_OPL_SetMusicVolume,
    I_OPL_PauseSong,
    I_OPL_ResumeSong,
    I_OPL_RegisterSong,
    I_OPL_UnRegisterSong,
    I_OPL_PlaySong,
    I_OPL_StopSong,
    I_OPL_MusicIsPlaying,
};

