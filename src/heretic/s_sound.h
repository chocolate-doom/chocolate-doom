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

// soundst.h

#ifndef __SOUNDSTH__
#define __SOUNDSTH__

extern int snd_MaxVolume;
extern int snd_MusicVolume;

void S_Start(void);
void S_StartSound(void *origin, int sound_id);
void S_StartSoundAtVolume(void *origin, int sound_id, int volume);
void S_StopSound(void *origin);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_UpdateSounds(mobj_t * listener);
void S_StartSong(int song, boolean loop);
void S_Init(void);
void S_GetChannelInfo(SoundInfo_t * s);
void S_SetMaxVolume(boolean fullprocess);
void S_SetMusicVolume(void);

#endif
