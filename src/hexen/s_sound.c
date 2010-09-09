// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//-----------------------------------------------------------------------------

#include "h2def.h"
#include "m_random.h"
#include "i_cdmus.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "r_local.h"
#include "p_local.h"            // for P_AproxDistance
#include "sounds.h"
#include "s_sound.h"

#define PRIORITY_MAX_ADJUST 10
#define DIST_ADJUST (MAX_SND_DIST/PRIORITY_MAX_ADJUST)

#define DEFAULT_ARCHIVEPATH     "o:\\sound\\archive\\"

void S_ShutDown(void);

boolean i_CDMusic;
int i_CDTrack;
int i_CDCurrentTrack;
int i_CDMusicLength;
int oldTic;

/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

//static channel_t channel[MAX_CHANNELS];

//static int rs; //the current registered song.
//int mus_song = -1;
//int mus_lumpnum;
//void *mus_sndptr;
//byte *soundCurve;

extern sfxinfo_t S_sfx[];
extern musicinfo_t S_music[];

static channel_t Channel[MAX_CHANNELS];
static void *RegisteredSong;      //the current registered song.
static boolean MusicPaused;
static int Mus_Song = -1;
static byte *Mus_SndPtr;
static byte *SoundCurve;

int snd_MaxVolume = 10;                // maximum volume for sound
int snd_MusicVolume = 10;              // maximum volume for music
int snd_Channels = 16;

extern int startepisode;
extern int startmap;

// int AmbChan;

//==========================================================================
//
// S_Start
//
//==========================================================================

void S_Start(void)
{
    S_StopAllSound();
    S_StartSong(gamemap, true);
}

//==========================================================================
//
// S_StartSong
//
//==========================================================================

void S_StartSong(int song, boolean loop)
{
    char *songLump;
    int lumpnum;
    int length;
    int track;

    if (i_CDMusic)
    {                           // Play a CD track, instead
        if (i_CDTrack)
        {                       // Default to the player-chosen track
            track = i_CDTrack;
        }
        else
        {
            track = P_GetMapCDTrack(gamemap);
        }
        if (track == i_CDCurrentTrack && i_CDMusicLength > 0)
        {
            return;
        }
        if (!I_CDMusPlay(track))
        {
            if (loop)
            {
                i_CDMusicLength = 35 * I_CDMusTrackLength(track);
                oldTic = gametic;
            }
            else
            {
                i_CDMusicLength = -1;
            }
            i_CDCurrentTrack = track;
        }
    }
    else
    {
        if (song == Mus_Song)
        {                       // don't replay an old song
            return;
        }
        if (RegisteredSong)
        {
            I_StopSong();
            I_UnRegisterSong(RegisteredSong);
            RegisteredSong = 0;
        }
        songLump = P_GetMapSongLump(song);
        if (!songLump)
        {
            return;
        }

        lumpnum = W_GetNumForName(songLump);
        Mus_SndPtr = W_CacheLumpNum(lumpnum, PU_STATIC);
        length = W_LumpLength(lumpnum);

        RegisteredSong = I_RegisterSong(Mus_SndPtr, length);
        I_PlaySong(RegisteredSong, loop);
        Mus_Song = song;

        W_ReleaseLumpNum(lumpnum);
    }
}

//==========================================================================
//
// S_StartSongName
//
//==========================================================================

void S_StartSongName(char *songLump, boolean loop)
{
    int lumpnum;
    int cdTrack;
    int length;

    if (!songLump)
    {
        return;
    }
    if (i_CDMusic)
    {
        cdTrack = 0;

        if (!strcmp(songLump, "hexen"))
        {
            cdTrack = P_GetCDTitleTrack();
        }
        else if (!strcmp(songLump, "hub"))
        {
            cdTrack = P_GetCDIntermissionTrack();
        }
        else if (!strcmp(songLump, "hall"))
        {
            cdTrack = P_GetCDEnd1Track();
        }
        else if (!strcmp(songLump, "orb"))
        {
            cdTrack = P_GetCDEnd2Track();
        }
        else if (!strcmp(songLump, "chess") && !i_CDTrack)
        {
            cdTrack = P_GetCDEnd3Track();
        }
/*	Uncomment this, if Kevin writes a specific song for startup
		else if(!strcmp(songLump, "start"))
		{
			cdTrack = P_GetCDStartTrack();
		}
*/
        if (!cdTrack || (cdTrack == i_CDCurrentTrack && i_CDMusicLength > 0))
        {
            return;
        }
        if (!I_CDMusPlay(cdTrack))
        {
            if (loop)
            {
                i_CDMusicLength = 35 * I_CDMusTrackLength(cdTrack);
                oldTic = gametic;
            }
            else
            {
                i_CDMusicLength = -1;
            }
            i_CDCurrentTrack = cdTrack;
            i_CDTrack = false;
        }
    }
    else
    {
        if (RegisteredSong)
        {
            I_StopSong();
            I_UnRegisterSong(RegisteredSong);
            RegisteredSong = NULL;
        }

        lumpnum = W_GetNumForName(songLump);
        Mus_SndPtr = W_CacheLumpNum(lumpnum, PU_MUSIC);
        length = W_LumpLength(lumpnum);

        RegisteredSong = I_RegisterSong(Mus_SndPtr, length);
        I_PlaySong(RegisteredSong, loop);
        W_ReleaseLumpNum(lumpnum);
        Mus_Song = -1;
    }
}

//==========================================================================
//
// S_GetSoundID
//
//==========================================================================

int S_GetSoundID(char *name)
{
    int i;

    for (i = 0; i < NUMSFX; i++)
    {
        if (!strcmp(S_sfx[i].tagname, name))
        {
            return i;
        }
    }
    return 0;
}

//==========================================================================
//
// S_StartSound
//
//==========================================================================

void S_StartSound(mobj_t * origin, int sound_id)
{
    S_StartSoundAtVolume(origin, sound_id, 127);
}

static mobj_t *GetSoundListener(void)
{
    static degenmobj_t dummy_listener;

    // If we are at the title screen, the console player doesn't have an
    // object yet, so return a pointer to a static dummy listener instead.

    if (players[displayplayer].mo != NULL)
    {
        return players[displayplayer].mo;
    }
    else
    {
        dummy_listener.x = 0;
        dummy_listener.y = 0;
        dummy_listener.z = 0;

        return (mobj_t *) &dummy_listener;
    }
}

//==========================================================================
//
// S_StartSoundAtVolume
//
//==========================================================================

void S_StartSoundAtVolume(mobj_t * origin, int sound_id, int volume)
{
    mobj_t *listener;
    int dist, vol;
    int i;
    int priority;
    int sep;
    int angle;
    int absx;
    int absy;

    static int sndcount = 0;
    int chan;

    if (sound_id == 0 || snd_MaxVolume == 0)
        return;

    listener = GetSoundListener();

    if (origin == NULL)
    {
        origin = listener;
    }
    if (volume == 0)
    {
        return;
    }

    // calculate the distance before other stuff so that we can throw out
    // sounds that are beyond the hearing range.
    absx = abs(origin->x - listener->x);
    absy = abs(origin->y - listener->y);
    dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
    dist >>= FRACBITS;
    if (dist >= MAX_SND_DIST)
    {
        return;                 // sound is beyond the hearing range...
    }
    if (dist < 0)
    {
        dist = 0;
    }
    priority = S_sfx[sound_id].priority;
    priority *= (PRIORITY_MAX_ADJUST - (dist / DIST_ADJUST));
    #if 0
    // TODO
    if (!S_StopSoundID(sound_id, priority))
    {
        return;                 // other sounds have greater priority
    }
    #endif
    for (i = 0; i < snd_Channels; i++)
    {
        if (origin->player)
        {
            i = snd_Channels;
            break;              // let the player have more than one sound.
        }
        if (origin == Channel[i].mo)
        {                       // only allow other mobjs one sound
            S_StopSound(Channel[i].mo);
            break;
        }
    }
    if (i >= snd_Channels)
    {
        for (i = 0; i < snd_Channels; i++)
        {
            if (Channel[i].mo == NULL)
            {
                break;
            }
        }
        if (i >= snd_Channels)
        {
            // look for a lower priority sound to replace.
            sndcount++;
            if (sndcount >= snd_Channels)
            {
                sndcount = 0;
            }
            for (chan = 0; chan < snd_Channels; chan++)
            {
                i = (sndcount + chan) % snd_Channels;
                if (priority >= Channel[i].priority)
                {
                    chan = -1;  //denote that sound should be replaced.
                    break;
                }
            }
            if (chan != -1)
            {
                return;         //no free channels.
            }
            else                //replace the lower priority sound.
            {
                if (Channel[i].handle)
                {
                    if (I_SoundIsPlaying(Channel[i].handle))
                    {
                        I_StopSound(Channel[i].handle);
                    }
                    if (S_sfx[Channel[i].sound_id].usefulness > 0)
                    {
                        S_sfx[Channel[i].sound_id].usefulness--;
                    }
                }
            }
        }
    }

    Channel[i].mo = origin;

    vol = (SoundCurve[dist] * (snd_MaxVolume * 8) * volume) >> 14;
    if (origin == listener)
    {
        sep = 128;
//              vol = (volume*(snd_MaxVolume+1)*8)>>7;
    }
    else
    {
        angle = R_PointToAngle2(listener->x,
                                listener->y,
                                Channel[i].mo->x, Channel[i].mo->y);
        angle = (angle - viewangle) >> 24;
        sep = angle * 2 - 128;
        if (sep < 64)
            sep = -sep;
        if (sep > 192)
            sep = 512 - sep;
//              vol = SoundCurve[dist];
    }

#if 0
// TODO
    if (S_sfx[sound_id].changePitch)
    {
        Channel[i].pitch = (byte) (127 + (M_Random() & 7) - (M_Random() & 7));
    }
    else
    {
        Channel[i].pitch = 127;
    }
#endif
    if (S_sfx[sound_id].lumpnum == 0)
    {
        S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
    }

    Channel[i].handle = I_StartSound(&S_sfx[sound_id],
                                     i,
                                     vol,
                                     sep /* , Channel[i].pitch] */);
    Channel[i].sound_id = sound_id;
    Channel[i].priority = priority;
    Channel[i].volume = volume;
    if (S_sfx[sound_id].usefulness < 0)
    {
        S_sfx[sound_id].usefulness = 1;
    }
    else
    {
        S_sfx[sound_id].usefulness++;
    }
}

//==========================================================================
//
// S_StopSoundID
//
//==========================================================================

boolean S_StopSoundID(int sound_id, int priority)
{
    int i;
    int lp;                     //least priority
    int found;

    if (S_sfx[sound_id].numchannels == -1)
    {
        return (true);
    }
    lp = -1;                    //denote the argument sound_id
    found = 0;
    for (i = 0; i < snd_Channels; i++)
    {
        if (Channel[i].sound_id == sound_id && Channel[i].mo)
        {
            found++;            //found one.  Now, should we replace it??
            if (priority >= Channel[i].priority)
            {                   // if we're gonna kill one, then this'll be it
                lp = i;
                priority = Channel[i].priority;
            }
        }
    }
    if (found < S_sfx[sound_id].numchannels)
    {
        return (true);
    }
    else if (lp == -1)
    {
        return (false);         // don't replace any sounds
    }
    if (Channel[lp].handle)
    {
        if (I_SoundIsPlaying(Channel[lp].handle))
        {
            I_StopSound(Channel[lp].handle);
        }
        if (S_sfx[Channel[lp].sound_id].usefulness > 0)
        {
            S_sfx[Channel[lp].sound_id].usefulness--;
        }
        Channel[lp].mo = NULL;
    }
    return (true);
}

//==========================================================================
//
// S_StopSound
//
//==========================================================================

void S_StopSound(mobj_t * origin)
{
    int i;

    for (i = 0; i < snd_Channels; i++)
    {
        if (Channel[i].mo == origin)
        {
            I_StopSound(Channel[i].handle);
            if (S_sfx[Channel[i].sound_id].usefulness > 0)
            {
                S_sfx[Channel[i].sound_id].usefulness--;
            }
            Channel[i].handle = 0;
            Channel[i].mo = NULL;
        }
    }
}

//==========================================================================
//
// S_StopAllSound
//
//==========================================================================

void S_StopAllSound(void)
{
    int i;

    //stop all sounds
    for (i = 0; i < snd_Channels; i++)
    {
        if (Channel[i].handle)
        {
            S_StopSound(Channel[i].mo);
        }
    }
    memset(Channel, 0, 8 * sizeof(channel_t));
}

//==========================================================================
//
// S_SoundLink
//
//==========================================================================

void S_SoundLink(mobj_t * oldactor, mobj_t * newactor)
{
    int i;

    for (i = 0; i < snd_Channels; i++)
    {
        if (Channel[i].mo == oldactor)
            Channel[i].mo = newactor;
    }
}

//==========================================================================
//
// S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
    if (i_CDMusic)
    {
        I_CDMusStop();
    }
    else
    {
        I_PauseSong();
    }
}

//==========================================================================
//
// S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
    if (i_CDMusic)
    {
        I_CDMusResume();
    }
    else
    {
        I_ResumeSong();
    }
}

//==========================================================================
//
// S_UpdateSounds
//
//==========================================================================

void S_UpdateSounds(mobj_t * listener)
{
    int i, dist, vol;
    int angle;
    int sep;
    int priority;
    int absx;
    int absy;

#ifdef CDMUSIC
    if (i_CDMusic)
    {
        I_UpdateCDMusic();
    }
#endif
    if (snd_MaxVolume == 0)
    {
        return;
    }

    // Update any Sequences
    SN_UpdateActiveSequences();

    for (i = 0; i < snd_Channels; i++)
    {
        if (!Channel[i].handle || S_sfx[Channel[i].sound_id].usefulness == -1)
        {
            continue;
        }
        if (!I_SoundIsPlaying(Channel[i].handle))
        {
            if (S_sfx[Channel[i].sound_id].usefulness > 0)
            {
                S_sfx[Channel[i].sound_id].usefulness--;
            }
            Channel[i].handle = 0;
            Channel[i].mo = NULL;
            Channel[i].sound_id = 0;
        }
        if (Channel[i].mo == NULL || Channel[i].sound_id == 0
            || Channel[i].mo == listener)
        {
            continue;
        }
        else
        {
            absx = abs(Channel[i].mo->x - listener->x);
            absy = abs(Channel[i].mo->y - listener->y);
            dist = absx + absy - (absx > absy ? absy >> 1 : absx >> 1);
            dist >>= FRACBITS;

            if (dist >= MAX_SND_DIST)
            {
                S_StopSound(Channel[i].mo);
                continue;
            }
            if (dist < 0)
            {
                dist = 0;
            }
            //vol = SoundCurve[dist];
            vol =
                (SoundCurve[dist] * (snd_MaxVolume * 8) *
                 Channel[i].volume) >> 14;
            if (Channel[i].mo == listener)
            {
                sep = 128;
            }
            else
            {
                angle = R_PointToAngle2(listener->x, listener->y,
                                        Channel[i].mo->x, Channel[i].mo->y);
                angle = (angle - viewangle) >> 24;
                sep = angle * 2 - 128;
                if (sep < 64)
                    sep = -sep;
                if (sep > 192)
                    sep = 512 - sep;
            }
            I_UpdateSoundParams(i, vol, sep /*, Channel[i].pitch */);
            priority = S_sfx[Channel[i].sound_id].priority;
            priority *= PRIORITY_MAX_ADJUST - (dist / DIST_ADJUST);
            Channel[i].priority = priority;
        }
    }
}

//==========================================================================
//
// S_Init
//
//==========================================================================

void S_Init(void)
{
    SoundCurve = W_CacheLumpName("SNDCURVE", PU_STATIC);
//      SoundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);

    I_InitSound(false);

    if (snd_Channels > 8)
    {
        snd_Channels = 8;
    }
    I_SetMusicVolume(snd_MusicVolume * 8);

    I_AtExit(S_ShutDown, true);

#ifdef CDMUSIC
//TODO
    // Attempt to setup CD music
    if (snd_MusicDevice == snd_CDMUSIC)
    {
        ST_Message("    Attempting to initialize CD Music: ");
        if (!cdrom)
        {
            i_CDMusic = (I_CDMusInit() != -1);
        }
        else
        {                       // The user is trying to use the cdrom for both game and music
            i_CDMusic = false;
        }
        if (i_CDMusic)
        {
            ST_Message("initialized.\n");
        }
        else
        {
            ST_Message("failed.\n");
        }
    }
#endif
}

//==========================================================================
//
// S_GetChannelInfo
//
//==========================================================================

void S_GetChannelInfo(SoundInfo_t * s)
{
    int i;
    ChanInfo_t *c;

    s->channelCount = snd_Channels;
    s->musicVolume = snd_MusicVolume;
    s->soundVolume = snd_MaxVolume;
    for (i = 0; i < snd_Channels; i++)
    {
        c = &s->chan[i];
        c->id = Channel[i].sound_id;
        c->priority = Channel[i].priority;
        c->name = S_sfx[c->id].name;
        c->mo = Channel[i].mo;

        if (c->mo != NULL)
        {
            c->distance = P_AproxDistance(c->mo->x - viewx, c->mo->y - viewy)
                >> FRACBITS;
        }
        else
        {
            c->distance = 0;
        }
    }
}

//==========================================================================
//
// S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(mobj_t * mobj, int sound_id)
{
    int i;

    for (i = 0; i < snd_Channels; i++)
    {
        if (Channel[i].sound_id == sound_id && Channel[i].mo == mobj)
        {
            if (I_SoundIsPlaying(Channel[i].handle))
            {
                return true;
            }
        }
    }
    return false;
}

//==========================================================================
//
// S_SetMusicVolume
//
//==========================================================================

void S_SetMusicVolume(void)
{
    if (i_CDMusic)
    {
        I_CDMusSetVolume(snd_MusicVolume * 16); // 0-255
    }
    else
    {
        I_SetMusicVolume(snd_MusicVolume * 8);
    }
    if (snd_MusicVolume == 0)
    {
        if (!i_CDMusic)
        {
            I_PauseSong();
        }
        MusicPaused = true;
    }
    else if (MusicPaused)
    {
        if (!i_CDMusic)
        {
            I_ResumeSong();
        }
        MusicPaused = false;
    }
}

//==========================================================================
//
// S_ShutDown
//
//==========================================================================

void S_ShutDown(void)
{
    I_StopSong();
    I_UnRegisterSong(RegisteredSong);
    I_ShutdownSound();
    if (i_CDMusic)
    {
        I_CDMusStop();
    }
}

//==========================================================================
//
// S_InitScript
//
//==========================================================================

void S_InitScript(void)
{
    int i;

    SC_OpenLump("sndinfo");

    while (SC_GetString())
    {
        if (*sc_String == '$')
        {
            if (!strcasecmp(sc_String, "$ARCHIVEPATH"))
            {
                SC_MustGetString();
            }
            else if (!strcasecmp(sc_String, "$MAP"))
            {
                SC_MustGetNumber();
                SC_MustGetString();
                if (sc_Number)
                {
                    P_PutMapSongLump(sc_Number, sc_String);
                }
            }
            continue;
        }
        else
        {
            for (i = 0; i < NUMSFX; i++)
            {
                if (!strcmp(S_sfx[i].tagname, sc_String))
                {
                    SC_MustGetString();
                    if (*sc_String != '?')
                    {
                        strcpy(S_sfx[i].name, sc_String);
                    }
                    else
                    {
                        strcpy(S_sfx[i].name, "default");
                    }
                    break;
                }
            }
            if (i == NUMSFX)
            {
                SC_MustGetString();
            }
        }
    }
    SC_Close();

    for (i = 0; i < NUMSFX; i++)
    {
        if (!strcmp(S_sfx[i].name, ""))
        {
            strcpy(S_sfx[i].name, "default");
        }
    }
}

