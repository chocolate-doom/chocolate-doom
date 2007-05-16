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
// DESCRIPTION:  Dummy sound interface for running with FEATURE_SOUND
//               disabled.
//
//-----------------------------------------------------------------------------

#include "doomtype.h"
#include "s_sound.h"
#include "p_mobj.h"
#include "sounds.h"

int snd_musicdevice = SNDDEVICE_NONE;
int snd_sfxdevice = SNDDEVICE_NONE;

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int sfxVolume = 8;

// Maximum volume of music. 
int musicVolume = 8;

// number of channels available

int			numChannels = 8;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init
( int		sfxVolume,
  int		musicVolume )
{  
}

void S_Shutdown(void)
{
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
}	

void
S_StartSoundAtVolume
( void*		origin_p,
  int		sfx_id,
  int		volume )
{
}	

void
S_StartSound
( void*		origin,
  int		sfx_id )
{
}

void S_StopSound(void *origin)
{
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
}

void S_ResumeSound(void)
{
}


//
// Updates music & sounds
//
void S_UpdateSounds(void* listener_p)
{
}

void S_SetMusicVolume(int volume)
{
}

void S_SetSfxVolume(int volume)
{
}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
}

void
S_ChangeMusic
( int			musicnum,
  int			looping )
{
}

boolean S_MusicPlaying(void)
{
    return false;
}


void S_StopMusic(void)
{
}

void S_StopChannel(int cnum)
{
}



//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
int
S_AdjustSoundParams
( mobj_t*	listener,
  mobj_t*	source,
  int*		vol,
  int*		sep,
  int*		pitch )
{
    return 0;
}




//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int
S_getChannel
( void*		origin,
  sfxinfo_t*	sfxinfo )
{
    return -1;
}




