
// soundst.h

#ifndef __SOUNDSTH__
#define __SOUNDSTH__

extern int snd_MaxVolume;
extern int snd_MusicVolume;

void S_Start(void);
void S_StartSound(mobj_t *origin, int sound_id);
void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume);
void S_StopSound(mobj_t *origin);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_UpdateSounds(mobj_t *listener);
void S_StartSong(int song, boolean loop);
void S_Init(void);
void S_GetChannelInfo(SoundInfo_t *s);
void S_SetMaxVolume(boolean fullprocess);
void S_SetMusicVolume(void);

#endif
