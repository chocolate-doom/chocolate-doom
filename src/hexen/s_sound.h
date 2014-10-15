//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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


#ifndef __S_SOUND__
#define __S_SOUND__

/*
typedef struct
{
    char name[8];
    int p1;
} musicinfo_t;
*/

/*
typedef struct sfxinfo_s
{
    char tagName[32];
    char lumpname[12];          // Only need 9 bytes, but padded out to be dword aligned
    //struct sfxinfo_s *link; // Make alias for another sound
    int priority;               // Higher priority takes precendence
    int usefulness;             // Determines when a sound should be cached out
    void *snd_ptr;
    int lumpnum;
    int numchannels;            // total number of channels a sound type may occupy
    boolean changePitch;
} sfxinfo_t;
*/

typedef struct
{
    mobj_t *mo;
    int sound_id;
    int handle;
    int volume;
    int pitch;
    int priority;
} channel_t;

typedef struct
{
    int id;
    unsigned short priority;
    char *name;
    mobj_t *mo;
    int distance;
} ChanInfo_t;

typedef struct
{
    int channelCount;
    int musicVolume;
    int soundVolume;
    ChanInfo_t chan[8];
} SoundInfo_t;

extern int snd_MaxVolume;
extern int snd_MusicVolume;
extern int snd_Channels;
extern boolean cdmusic;

void S_Start(void);
void S_StartSound(mobj_t * origin, int sound_id);
int S_GetSoundID(char *name);
void S_StartSoundAtVolume(mobj_t * origin, int sound_id, int volume);
void S_StopSound(mobj_t * origin);
void S_StopAllSound(void);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_UpdateSounds(mobj_t * listener);
void S_StartSong(int song, boolean loop);
void S_StartSongName(char *songLump, boolean loop);
void S_Init(void);
void S_GetChannelInfo(SoundInfo_t * s);
void S_SetMusicVolume(void);
boolean S_GetSoundPlayingInfo(mobj_t * mobj, int sound_id);
boolean S_StartCustomCDTrack(int tracknum);
int S_GetCurrentCDTrack(void);

#endif
