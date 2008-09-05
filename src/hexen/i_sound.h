// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//-----------------------------------------------------------------------------
#ifndef __SOUND__
#define __SOUND__

#define SND_TICRATE             140     // tic rate for updating sound
#define SND_MAXSONGS    40      // max number of songs in game
#define SND_SAMPLERATE  11025   // sample rate of sound effects

typedef enum
{
    snd_none,
    snd_PC,
    snd_Adlib,
    snd_SB,
    snd_PAS,
    snd_GUS,
    snd_MPU,
    snd_MPU2,
    snd_MPU3,
    snd_AWE,
    snd_CDMUSIC,
    NUM_SCARDS
} cardenum_t;

void I_PauseSong(int handle);
void I_ResumeSong(int handle);
void I_SetMusicVolume(int volume);
void I_SetSfxVolume(int volume);
int I_RegisterSong(void *data);
void I_UnRegisterSong(int handle);
int I_QrySongPlaying(int handle);
void I_StopSong(int handle);
void I_PlaySong(int handle, int looping);
int I_GetSfxLumpNum(sfxinfo_t * sound);
int I_StartSound(int id, void *data, int vol, int sep, int pitch,
                 int priority);
void I_StopSound(int handle);
int I_SoundIsPlaying(int handle);
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch);
void I_sndArbitrateCards(void);
void I_StartupSound(void);
void I_ShutdownSound(void);
void I_SetChannels(int channels);

#endif

#ifndef __ICDMUS__
#define __ICDMUS__

#define CDERR_NOTINSTALLED   10 // MSCDEX not installed
#define CDERR_NOAUDIOSUPPORT 11 // CD-ROM Doesn't support audio
#define CDERR_NOAUDIOTRACKS  12 // Current CD has no audio tracks
#define CDERR_BADDRIVE       20 // Bad drive number
#define CDERR_BADTRACK       21 // Bad track number
#define CDERR_IOCTLBUFFMEM   22 // Not enough low memory for IOCTL
#define CDERR_DEVREQBASE     100        // DevReq errors

extern int cd_Error;

int I_CDMusInit(void);
int I_CDMusPlay(int track);
int I_CDMusStop(void);
int I_CDMusResume(void);
int I_CDMusSetVolume(int volume);
int I_CDMusFirstTrack(void);
int I_CDMusLastTrack(void);
int I_CDMusTrackLength(int track);

#endif
