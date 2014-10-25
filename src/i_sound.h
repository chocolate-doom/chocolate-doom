//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	The not so system specific sound interface.
//


#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomtype.h"


//
// SoundFX struct.
//
typedef struct sfxinfo_struct	sfxinfo_t;

struct sfxinfo_struct
{
    // tag name, used for hexen.
    char *tagname;
    
    // lump name.  If we are running with use_sfx_prefix=true, a
    // 'DS' (or 'DP' for PC speaker sounds) is prepended to this.

    char name[9];

    // Sfx priority
    int priority;

    // referenced sound if a link
    sfxinfo_t *link;

    // pitch if a link
    int pitch;

    // volume if a link
    int volume;

    // this is checked every second to see if sound
    // can be thrown out (if 0, then decrement, if -1,
    // then throw out, if > 0, then it is in use)
    int usefulness;

    // lump number of sfx
    int lumpnum;		

    // Maximum number of channels that the sound can be played on 
    // (Heretic)
    int numchannels;

    // data used by the low level code
    void *driver_data;
};

//
// MusicInfo struct.
//
typedef struct
{
    // up to 6-character name
    char *name;

    // lump number of music
    int lumpnum;
    
    // music data
    void *data;

    // music handle once registered
    void *handle;
    
} musicinfo_t;

typedef enum 
{
    SNDDEVICE_NONE = 0,
    SNDDEVICE_PCSPEAKER = 1,
    SNDDEVICE_ADLIB = 2,
    SNDDEVICE_SB = 3,
    SNDDEVICE_PAS = 4,
    SNDDEVICE_GUS = 5,
    SNDDEVICE_WAVEBLASTER = 6,
    SNDDEVICE_SOUNDCANVAS = 7,
    SNDDEVICE_GENMIDI = 8,
    SNDDEVICE_AWE32 = 9,
    SNDDEVICE_CD = 10,
} snddevice_t;

// Interface for sound modules

typedef struct
{
    // List of sound devices that this sound module is used for.

    snddevice_t *sound_devices;
    int num_sound_devices;

    // Initialise sound module
    // Returns true if successfully initialised

    boolean (*Init)(boolean use_sfx_prefix);

    // Shutdown sound module

    void (*Shutdown)(void);

    // Returns the lump index of the given sound.

    int (*GetSfxLumpNum)(sfxinfo_t *sfxinfo);

    // Called periodically to update the subsystem.

    void (*Update)(void);

    // Update the sound settings on the given channel.

    void (*UpdateSoundParams)(int channel, int vol, int sep);

    // Start a sound on a given channel.  Returns the channel id
    // or -1 on failure.

    int (*StartSound)(sfxinfo_t *sfxinfo, int channel, int vol, int sep);

    // Stop the sound playing on the given channel.

    void (*StopSound)(int channel);

    // Query if a sound is playing on the given channel

    boolean (*SoundIsPlaying)(int channel);

    // Called on startup to precache sound effects (if necessary)

    void (*CacheSounds)(sfxinfo_t *sounds, int num_sounds);

} sound_module_t;

void I_InitSound(boolean use_sfx_prefix);
void I_ShutdownSound(void);
int I_GetSfxLumpNum(sfxinfo_t *sfxinfo);
void I_UpdateSound(void);
void I_UpdateSoundParams(int channel, int vol, int sep);
int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep);
void I_StopSound(int channel);
boolean I_SoundIsPlaying(int channel);
void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds);

// Interface for music modules

typedef struct
{
    // List of sound devices that this music module is used for.

    snddevice_t *sound_devices;
    int num_sound_devices;

    // Initialise the music subsystem

    boolean (*Init)(void);

    // Shutdown the music subsystem

    void (*Shutdown)(void);

    // Set music volume - range 0-127

    void (*SetMusicVolume)(int volume);

    // Pause music

    void (*PauseMusic)(void);

    // Un-pause music

    void (*ResumeMusic)(void);

    // Register a song handle from data
    // Returns a handle that can be used to play the song

    void *(*RegisterSong)(void *data, int len);

    // Un-register (free) song data

    void (*UnRegisterSong)(void *handle);

    // Play the song

    void (*PlaySong)(void *handle, boolean looping);

    // Stop playing the current song.

    void (*StopSong)(void);

    // Query if music is playing.

    boolean (*MusicIsPlaying)(void);

    // Invoked periodically to poll.

    void (*Poll)(void);
} music_module_t;

void I_InitMusic(void);
void I_ShutdownMusic(void);
void I_SetMusicVolume(int volume);
void I_PauseSong(void);
void I_ResumeSong(void);
void *I_RegisterSong(void *data, int len);
void I_UnRegisterSong(void *handle);
void I_PlaySong(void *handle, boolean looping);
void I_StopSong(void);
boolean I_MusicIsPlaying(void);

extern int snd_sfxdevice;
extern int snd_musicdevice;
extern int snd_samplerate;
extern int snd_cachesize;
extern int snd_maxslicetime_ms;
extern char *snd_musiccmd;

void I_BindSoundVariables(void);

#endif

