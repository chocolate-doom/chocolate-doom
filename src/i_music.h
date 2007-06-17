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
//
// DESCRIPTION:
//	System interface, music.
//
//-----------------------------------------------------------------------------

#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomdef.h"

#include "doomstat.h"
#include "sounds.h"
#include "s_sound.h"

//
//  MUSIC I/O
//

void I_InitMusic(void);
void I_ShutdownMusic(void);

// Volume.

void I_SetMusicVolume(int volume);

// PAUSE game handling.

void I_PauseSong(void *handle);
void I_ResumeSong(void *handle);

// Registers a song handle to song data.

void *I_RegisterSong(void *data, int length);

// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.

void I_PlaySong(void *handle, int looping);

// Stops a song over 3 seconds.

void I_StopSong(void *handle);

// See above (register), then think backwards

void I_UnRegisterSong(void *handle);

boolean I_QrySongPlaying(void *handle);


#endif

