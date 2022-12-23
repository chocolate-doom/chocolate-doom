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
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

#include "i_sound.h"
#include "i_system.h"

#include "deh_str.h"

#include "doomstat.h"
#include "doomtype.h"

#include "sounds.h"
#include "s_sound.h"
#include "s_musinfo.h" // [crispy] struct musinfo

#include "m_misc.h"
#include "m_random.h"
#include "m_argv.h"

#include "p_local.h"
#include "w_wad.h"
#include "z_zone.h"

// when to clip out sounds
// Does not fit the large outdoor areas.

#define S_CLIPPING_DIST (1200 * FRACUNIT)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// In the source code release: (160*FRACUNIT).  Changed back to the
// Vanilla value of 200 (why was this changed?)

#define S_CLOSE_DIST (200 * FRACUNIT)

// The range over which sound attenuates

#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Stereo separation

#define S_STEREO_SWING (96 * FRACUNIT)
static int stereo_swing;

#define NORM_PRIORITY 64
#define NORM_SEP 128

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t *sfxinfo;

    // origin of sound
    mobj_t *origin;

    // handle of the sound being played
    int handle;

    int pitch;

} channel_t;

// The set of channels available

static channel_t *channels;
static degenmobj_t *sobjs;

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.

int sfxVolume = 8;

// Maximum volume of music.

int musicVolume = 8;

// Internal volume level, ranging from 0-127

static int snd_SfxVolume;

// Whether songs are mus_paused

static boolean mus_paused;

// Music currently being played

static musicinfo_t *mus_playing = NULL;

// Number of channels to use

int snd_channels = 8;

// [crispy] add support for alternative music tracks for Final Doom's
// TNT and Plutonia as introduced in DoomMetalVol5.wad

typedef struct {
	const char *const from;
	const char *const to;
} altmusic_t;

static const altmusic_t altmusic_tnt[] =
{
	{"runnin", "sadist"}, // MAP01
	{"stalks", "burn"},   // MAP02
	{"countd", "messag"}, // MAP03
	{"betwee", "bells"},  // MAP04
	{"doom",   "more"},   // MAP05
	{"the_da", "agony"},  // MAP06
	{"shawn",  "chaos"},  // MAP07
	{"ddtblu", "beast"},  // MAP08
	{"in_cit", "sadist"}, // MAP09
	{"dead",   "infini"}, // MAP10
	{"stlks2", "kill"},   // MAP11
	{"theda2", "ddtbl3"}, // MAP12
	{"doom2",  "bells"},  // MAP13
	{"ddtbl2", "cold"},   // MAP14
	{"runni2", "burn2"},  // MAP15
	{"dead2",  "blood"},  // MAP16
	{"stlks3", "more"},   // MAP17
	{"romero", "infini"}, // MAP18
	{"shawn2", "countd"}, // MAP19
	{"messag", "horizo"}, // MAP20
	{"count2", "in_cit"}, // MAP21
	{"ddtbl3", "aim"},    // MAP22
//	{"ampie",  "ampie"},  // MAP23
	{"theda3", "betwee"}, // MAP24
	{"adrian", "doom"},   // MAP25
	{"messg2", "blood"},  // MAP26
	{"romer2", "beast"},  // MAP27
	{"tense",  "aim"},    // MAP28
	{"shawn3", "bells"},  // MAP29
	{"openin", "beast"},  // MAP30
//	{"evil",   "evil"},   // MAP31
	{"ultima", "in_cit"}, // MAP32
	{NULL,     NULL},
};

// Plutonia music is completely taken from Doom 1 and 2, but re-arranged.
// That is, Plutonia's D_RUNNIN (for MAP01) is the renamed D_E1M2. So,
// it makes sense to play the D_E1M2 replacement from DoomMetal in Plutonia.

static const altmusic_t altmusic_plut[] =
{
	{"runnin", "e1m2"},   // MAP01
	{"stalks", "e1m3"},   // MAP02
	{"countd", "e1m6"},   // MAP03
	{"betwee", "e1m4"},   // MAP04
	{"doom",   "e1m9"},   // MAP05
	{"the_da", "e1m8"},   // MAP06
	{"shawn",  "e2m1"},   // MAP07
	{"ddtblu", "e2m2"},   // MAP08
	{"in_cit", "e3m3"},   // MAP09
	{"dead",   "e1m7"},   // MAP10
	{"stlks2", "bunny"},  // MAP11
	{"theda2", "e3m8"},   // MAP12
	{"doom2",  "e3m2"},   // MAP13
	{"ddtbl2", "e2m8"},   // MAP14
	{"runni2", "e2m7"},   // MAP15
	{"dead2",  "e3m1"},   // MAP16
	{"stlks3", "e1m1"},   // MAP17
	{"romero", "e2m5"},   // MAP18
	{"shawn2", "e1m5"},   // MAP19
//	{"messag", "messag"}, // MAP20
//	{"count2", "count2"}, // MAP21 (d_read_m has no instumental cover in Doom Metal)
//	{"ddtbl3", "ddtbl3"}, // MAP22
//	{"ampie",  "ampie"},  // MAP23
//	{"theda3", "theda3"}, // MAP24
//	{"adrian", "adrian"}, // MAP25
//	{"messg2", "messg2"}, // MAP26
	{"romer2", "e2m1"},   // MAP27
	{"tense",  "e2m2"},   // MAP28
	{"shawn3", "e1m1"},   // MAP29
//	{"openin", "openin"}, // MAP30 (d_victor has no instumental cover in Doom Metal)
	{"evil",   "e3m4"},   // MAP31
	{"ultima", "e2m8"},   // MAP32
	{NULL,     NULL},
};

static void S_RegisterAltMusic()
{
	const altmusic_t *altmusic_fromto, *altmusic;

	if (gamemission == pack_tnt)
	{
		altmusic_fromto = altmusic_tnt;
	}
	else
	if (gamemission == pack_plut)
	{
		altmusic_fromto = altmusic_plut;
	}
	else
	{
		return;
	}

	// [crispy] chicken-out if only one lump is missing, something must be wrong
	for (altmusic = altmusic_fromto; altmusic->from; altmusic++)
	{
		char name[9];

		M_snprintf(name, sizeof(name), "d_%s", altmusic->to);

		if (W_CheckNumForName(name) == -1)
		{
			return;
		}
	}

	for (altmusic = altmusic_fromto; altmusic->from; altmusic++)
	{
		DEH_AddStringReplacement(altmusic->from, altmusic->to);
	}
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
    int i;

    if (gameversion == exe_doom_1_666)
    {
        if (logical_gamemission == doom)
        {
            I_SetOPLDriverVer(opl_doom1_1_666);
        }
        else
        {
            I_SetOPLDriverVer(opl_doom2_1_666);
        }
    }
    else
    {
        I_SetOPLDriverVer(opl_doom_1_9);
    }

    I_PrecacheSounds(S_sfx, NUMSFX);

    S_SetSfxVolume(sfxVolume);
    S_SetMusicVolume(musicVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    // [crispy] variable number of sound channels
    channels = I_Realloc(NULL, snd_channels*sizeof(channel_t));
    sobjs = I_Realloc(NULL, snd_channels*sizeof(degenmobj_t));

    // Free all channels for use
    for (i=0 ; i<snd_channels ; i++)
    {
        channels[i].sfxinfo = 0;
    }

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;

    // Note that sounds have not been cached (yet).
    for (i=1 ; i<NUMSFX ; i++)
    {
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
    }

    // Doom defaults to pitch-shifting off.
    if (snd_pitchshift == -1)
    {
        snd_pitchshift = 0;
    }

    I_AtExit(S_Shutdown, true);

    // [crispy] initialize dedicated music tracks for the 4th episode
    for (i = mus_e4m1; i <= mus_e5m9; i++)
    {
        musicinfo_t *const music = &S_music[i];
        char namebuf[9];

        M_snprintf(namebuf, sizeof(namebuf), "d_%s", DEH_String(music->name));
        music->lumpnum = W_CheckNumForName(namebuf);
    }

    // [crispy] add support for alternative music tracks for Final Doom's
    // TNT and Plutonia as introduced in DoomMetalVol5.wad
    S_RegisterAltMusic();

    // [crispy] handle stereo separation for mono-sfx and flipped levels
    S_UpdateStereoSeparation();
}

void S_Shutdown(void)
{
    I_ShutdownSound();
    I_ShutdownMusic();
}

static void S_StopChannel(int cnum)
{
    int i;
    channel_t *c;

    c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing

        if (I_SoundIsPlaying(c->handle))
        {
            I_StopSound(c->handle);
        }

        // check to see if other channels are playing the sound

        for (i=0; i<snd_channels; i++)
        {
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }

        // degrade usefulness of sound data

        c->sfxinfo->usefulness--;
        c->sfxinfo = NULL;
        c->origin = NULL;
    }
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
static short prevmap = -1;

void S_Start(void)
{
    int cnum;
    int mnum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo)
        {
            S_StopChannel(cnum);
        }
    }

    // start new music for the level
    if (musicVolume) // [crispy] do not reset pause state at zero music volume
    mus_paused = 0;

    if (gamemode == commercial)
    {
        const int nmus[] =
        {
            mus_messag,
            mus_ddtblu,
            mus_doom,
            mus_shawn,
            mus_in_cit,
            mus_the_da,
            mus_in_cit,
            mus_shawn2,
            mus_ddtbl2,
        };

        if ((gameepisode == 2 || gamemission == pack_nerve) &&
            gamemap <= arrlen(nmus))
        {
            mnum = nmus[gamemap - 1];
        }
        else
        mnum = mus_runnin + gamemap - 1;
    }
    else
    {
        int spmus[]=
        {
            // Song - Who? - Where?

            mus_e3m4,        // American     e4m1
            mus_e3m2,        // Romero       e4m2
            mus_e3m3,        // Shawn        e4m3
            mus_e1m5,        // American     e4m4
            mus_e2m7,        // Tim          e4m5
            mus_e2m4,        // Romero       e4m6
            mus_e2m6,        // J.Anderson   e4m7 CHIRON.WAD
            mus_e2m5,        // Shawn        e4m8
            mus_e1m9,        // Tim          e4m9
        };

        if (gameepisode < 4 || gameepisode == 5) // [crispy] Sigil
        {
            mnum = mus_e1m1 + (gameepisode-1)*9 + gamemap-1;
        }
        else
        {
            mnum = spmus[gamemap-1];

            // [crispy] support dedicated music tracks for the 4th episode
            {
                const int sp_mnum = mus_e1m1 + 3 * 9 + gamemap - 1;

                if (S_music[sp_mnum].lumpnum > 0)
                {
                    mnum = sp_mnum;
                }
            }
        }
    }

    // [crispy] do not change music if not changing map (preserves IDMUS choice)
    {
	const short curmap = (gameepisode << 8) + gamemap;

	if (prevmap == curmap || (nodrawers && singletics))
	    return;

	prevmap = curmap;
    }

    // [crispy] reset musinfo data at the start of a new map
    memset(&musinfo, 0, sizeof(musinfo));

    S_ChangeMusic(mnum, true);
}

void S_StopSound(mobj_t *origin)
{
    int cnum;

    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}

// [crispy] removed map objects may finish their sounds
// When map objects are removed from the map by P_RemoveMobj(), instead of
// stopping their sounds, their coordinates are transfered to "sound objects"
// so stereo positioning and distance calculations continue to work even after
// the corresponding map object has already disappeared.
// Thanks to jeff-d and kb1 for discussing this feature and the former for the
// original implementation idea: https://www.doomworld.com/vb/post/1585325
void S_UnlinkSound(mobj_t *origin)
{
    int cnum;

    if (origin)
    {
        for (cnum=0 ; cnum<snd_channels ; cnum++)
        {
            if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
            {
                degenmobj_t *const sobj = &sobjs[cnum];
                sobj->x = origin->x;
                sobj->y = origin->y;
                sobj->z = origin->z;
                channels[cnum].origin = (mobj_t *) sobj;
                break;
            }
        }
    }
}

//
// S_GetChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int S_GetChannel(mobj_t *origin, sfxinfo_t *sfxinfo)
{
    // channel number to use
    int                cnum;

    channel_t*        c;

    // Find an open channel
    for (cnum=0 ; cnum<snd_channels ; cnum++)
    {
        if (!channels[cnum].sfxinfo)
        {
            break;
        }
        else if (origin && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // None available
    if (cnum == snd_channels)
    {
        // Look for lower priority
        for (cnum=0 ; cnum<snd_channels ; cnum++)
        {
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
            {
                break;
            }
        }

        if (cnum == snd_channels)
        {
            // FUCK!  No lower priority.  Sorry, Charlie.
            return -1;
        }
        else
        {
            // Otherwise, kick out lower priority.
            S_StopChannel(cnum);
        }
    }

    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}

//
// Changes volume and stereo-separation variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

static int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                               int *vol, int *sep)
{
    fixed_t        approx_dist;
    fixed_t        adx;
    fixed_t        ady;
    angle_t        angle;

    // [crispy] proper sound clipping in Doom 2 MAP08 and The Ultimate Doom E4M8 / Sigil E5M8
    const boolean doom1map8 = (gamemap == 8 && ((gamemode != commercial && gameepisode < 4) || !crispy->soundfix));

    // calculate the distance to sound origin
    //  and clip it if necessary
    adx = abs(listener->x - source->x);
    ady = abs(listener->y - source->y);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

    if (!doom1map8 && approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle = R_PointToAngle2(listener->x,
                            listener->y,
                            source->x,
                            source->y);

    if (angle > listener->angle)
    {
        angle = angle - listener->angle;
    }
    else
    {
        angle = angle + (0xffffffff - listener->angle);
    }

    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - (FixedMul(stereo_swing, finesine[angle]) >> FRACBITS);

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
        *vol = snd_SfxVolume;
    }
    else if (doom1map8)
    {
        if (approx_dist > S_CLIPPING_DIST)
        {
            approx_dist = S_CLIPPING_DIST;
        }

        *vol = 15+ ((snd_SfxVolume-15)
                    *((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
            / S_ATTENUATOR;
    }
    else
    {
        // distance effect
        *vol = (snd_SfxVolume
                * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
            / S_ATTENUATOR;
    }

    return (*vol > 0);
}

// clamp supplied integer to the range 0 <= x <= 255.

static int Clamp(int x)
{
    if (x < 0)
    {
        return 0;
    }
    else if (x > 255)
    {
        return 255;
    }
    return x;
}

void S_StartSound(void *origin_p, int sfx_id)
{
    sfxinfo_t *sfx;
    mobj_t *origin;
    int rc;
    int sep;
    int pitch;
    int cnum;
    int volume;

    origin = (mobj_t *) origin_p;
    volume = snd_SfxVolume;

    // [crispy] make non-fatal, consider zero volume
    if (sfx_id == sfx_None || !snd_SfxVolume || (nodrawers && singletics))
    {
        return;
    }
    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > NUMSFX)
    {
        I_Error("Bad sfx #: %d", sfx_id);
    }

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    pitch = NORM_PITCH;
    if (sfx->link)
    {
        volume += sfx->volume;
        pitch = sfx->pitch;

        if (volume < 1)
        {
            return;
        }

        if (volume > snd_SfxVolume)
        {
            volume = snd_SfxVolume;
        }
    }


    // Check to see if it is audible,
    //  and if not, modify the params
    if (origin && origin != players[displayplayer].mo && origin != players[displayplayer].so) // [crispy] weapon sound source
    {
        rc = S_AdjustSoundParams(players[displayplayer].mo,
                                 origin,
                                 &volume,
                                 &sep);

        if (origin->x == players[displayplayer].mo->x
         && origin->y == players[displayplayer].mo->y)
        {
            sep = NORM_SEP;
        }

        if (!rc)
        {
            return;
        }
    }
    else
    {
        sep = NORM_SEP;
    }

    // hacks to vary the sfx pitches
    if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
    {
        pitch += 8 - (M_Random()&15);
    }
    else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
    {
        pitch += 16 - (M_Random()&31);
    }
    pitch = Clamp(pitch);

    // kill old sound
    if (!crispy->soundfull || origin || gamestate != GS_LEVEL)
    {
    S_StopSound(origin);
    }

    // try to find a channel
    cnum = S_GetChannel(origin, sfx);

    if (cnum < 0)
    {
        return;
    }

    // increase the usefulness
    if (sfx->usefulness++ < 0)
    {
        sfx->usefulness = 1;
    }

    if (sfx->lumpnum < 0)
    {
        sfx->lumpnum = I_GetSfxLumpNum(sfx);
    }

    channels[cnum].pitch = pitch;
    channels[cnum].handle = I_StartSound(sfx, cnum, volume, sep, channels[cnum].pitch);
}

void S_StartSoundOnce (void *origin_p, int sfx_id)
{
    int cnum;
    const sfxinfo_t *const sfx = &S_sfx[sfx_id];

    for (cnum = 0; cnum < snd_channels; cnum++)
    {
        if (channels[cnum].sfxinfo == sfx &&
            channels[cnum].origin == origin_p)
        {
            return;
        }
    }

    S_StartSound(origin_p, sfx_id);
}

// [NS] Try to play an optional sound.
void S_StartSoundOptional(void *origin_p, int sfx_id, int old_sfx_id)
{
    if (I_GetSfxLumpNum(&S_sfx[sfx_id]) != -1)
    {
        S_StartSound(origin_p, sfx_id);
    }
    else if (old_sfx_id != -1) // Play a fallback?
    {
        S_StartSound(origin_p, old_sfx_id);
    }
}

//
// Stop and resume music, during game PAUSE.
//

void S_PauseSound(void)
{
    if (mus_playing && !mus_paused)
    {
        I_PauseSong();
        mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    if (mus_playing && mus_paused)
    {
        I_ResumeSong();
        mus_paused = false;
    }
}

//
// Updates music & sounds
//

void S_UpdateSounds(mobj_t *listener)
{
    int                audible;
    int                cnum;
    int                volume;
    int                sep;
    sfxinfo_t*        sfx;
    channel_t*        c;

    I_UpdateSound();

    for (cnum=0; cnum<snd_channels; cnum++)
    {
        c = &channels[cnum];
        sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                volume = snd_SfxVolume;
                sep = NORM_SEP;

                if (sfx->link)
                {
                    volume += sfx->volume;
                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                    else if (volume > snd_SfxVolume)
                    {
                        volume = snd_SfxVolume;
                    }
                }

                // check non-local sounds for distance clipping
                //  or modify their params
                if (c->origin && listener != c->origin && c->origin != players[displayplayer].so) // [crispy] weapon sound source
                {
                    audible = S_AdjustSoundParams(listener,
                                                  c->origin,
                                                  &volume,
                                                  &sep);

                    if (!audible)
                    {
                        S_StopChannel(cnum);
                    }
                    else
                    {
                        I_UpdateSoundParams(c->handle, volume, sep);
                    }
                }
            }
            else
            {
                // if channel is allocated but sound has stopped,
                //  free it
                S_StopChannel(cnum);
            }
        }
    }
}

void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set music volume at %d",
                volume);
    }

    // [crispy] [JN] Fixed bug when music was hearable with zero volume
    if (!musicVolume)
    {
        S_PauseSound();
    }
    else
    if (!paused)
    {
        S_ResumeSound();
    }

    I_SetMusicVolume(volume);
}

void S_SetSfxVolume(int volume)
{
    if (volume < 0 || volume > 127)
    {
        I_Error("Attempt to set sfx volume at %d", volume);
    }

    snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//

void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false);
}

void S_ChangeMusic(int musicnum, int looping)
{
    musicinfo_t *music = NULL;
    char namebuf[9];
    void *handle;

    if (gamestate != GS_LEVEL)
    {
	prevmap = -1;
    }
    musinfo.current_item = -1;

    // [crispy] play no music if this is not the right map
    if (nodrawers && singletics)
	return;

    // [crispy] restart current music if IDMUS00 is entered
    if (looping == 2)
    {
	music = mus_playing;
    }

    // The Doom IWAD file has two versions of the intro music: d_intro
    // and d_introa.  The latter is used for OPL playback.

    if (musicnum == mus_intro && (snd_musicdevice == SNDDEVICE_ADLIB
                               || snd_musicdevice == SNDDEVICE_SB)
        && W_CheckNumForName("D_INTROA") >= 0)
    {
        const int intro = W_GetNumForName("D_INTRO"),
                  introa = W_GetNumForName("D_INTROA");
        // [crispy] if D_INTRO is from a PWAD, and D_INTROA is from a different WAD file, play the former
        if (W_IsIWADLump(lumpinfo[intro]) || (lumpinfo[intro]->wad_file == lumpinfo[introa]->wad_file))
        {
        musicnum = mus_introa;
        }
    }

    // [crispy] prevent music number under- and overflows
    if (musicnum <= mus_None || (gamemode == commercial && musicnum < mus_runnin) ||
        musicnum >= NUMMUSIC || (gamemode != commercial && musicnum >= mus_runnin) ||
        S_music[musicnum].lumpnum == -1)
    {
        const unsigned int umusicnum = (unsigned int) musicnum;

        if (gamemode == commercial)
        {
            musicnum = mus_runnin + (umusicnum % (NUMMUSIC - mus_runnin));
        }
        else
        {
            musicnum = mus_e1m1 + (umusicnum % (mus_e4m1 - mus_e1m1));
        }
    }

    if (musicnum <= mus_None || musicnum >= NUMMUSIC)
    {
        I_Error("Bad music number %d", musicnum);
    }
    else
    {
      if (!music) // [crispy] restart current music if IDMUS00 is entered
        music = &S_music[musicnum];
    }

    if (mus_playing == music)
    {
      if (looping != 2) // [crispy] restart current music if IDMUS00 is entered
        return;
    }

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
        M_snprintf(namebuf, sizeof(namebuf), "d_%s", DEH_String(music->name));
        music->lumpnum = W_GetNumForName(namebuf);
    }

    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);

    handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
    music->handle = handle;
    I_PlaySong(handle, looping);
    // [crispy] log played music
    {
        char name[9];
        M_snprintf(name, sizeof(name), "%s", lumpinfo[music->lumpnum]->name);
        fprintf(stderr, "S_ChangeMusic: %s (%s)\n", name,
                W_WadNameForLump(lumpinfo[music->lumpnum]));
    }

    mus_playing = music;

    // [crispy] musinfo.items[0] is reserved for the map's default music
    if (!musinfo.items[0])
    {
	musinfo.items[0] = music->lumpnum;
	S_music[mus_musinfo].lumpnum = -1;
    }
}

// [crispy] adapted from prboom-plus/src/s_sound.c:552-590

void S_ChangeMusInfoMusic (int lumpnum, int looping)
{
    musicinfo_t *music;

    // [crispy] restarting the map plays the original music
    prevmap = -1;

    // [crispy] play no music if this is not the right map
    if (nodrawers && singletics)
    {
	musinfo.current_item = lumpnum;
	return;
    }

    if (mus_playing && mus_playing->lumpnum == lumpnum)
    {
	return;
    }

    music = &S_music[mus_musinfo];

    if (music->lumpnum == lumpnum)
    {
	return;
    }

    S_StopMusic();

    music->lumpnum = lumpnum;

    music->data = W_CacheLumpNum(music->lumpnum, PU_STATIC);
    music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

    I_PlaySong(music->handle, looping);
    // [crispy] log played music
    {
        char name[9];
        M_snprintf(name, sizeof(name), "%s", lumpinfo[music->lumpnum]->name);
        fprintf(stderr, "S_ChangeMusInfoMusic: %s (%s)\n", name,
                W_WadNameForLump(lumpinfo[music->lumpnum]));
    }

    mus_playing = music;

    musinfo.current_item = lumpnum;
}

boolean S_MusicPlaying(void)
{
    return I_MusicIsPlaying();
}

void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
        {
            I_ResumeSong();
        }

        I_StopSong();
        I_UnRegisterSong(mus_playing->handle);
        W_ReleaseLumpNum(mus_playing->lumpnum);
        mus_playing->data = NULL;
        mus_playing = NULL;
    }
}

// [crispy] variable number of sound channels
void S_UpdateSndChannels (int choice)
{
	int i;

	for (i = 0; i < snd_channels; i++)
	{
		if (channels[i].sfxinfo)
		{
			S_StopChannel(i);
		}
	}

	if (choice)
	{
		snd_channels <<= 1;
	}
	else
	{
		snd_channels >>= 1;
	}

	if (snd_channels > 32)
	{
		snd_channels = 8;
	}
	else if (snd_channels < 8)
	{
		snd_channels = 32;
	}

	channels = I_Realloc(channels, snd_channels * sizeof(channel_t));
	sobjs = I_Realloc(sobjs, snd_channels * sizeof(degenmobj_t));

	for (i = 0; i < snd_channels; i++)
	{
		channels[i].sfxinfo = 0;
	}
}

void S_UpdateStereoSeparation (void)
{
	// [crispy] play all sound effects in mono
	if (crispy->soundmono)
	{
		stereo_swing = 0;
	}
	else
	if (crispy->fliplevels)
	{
		stereo_swing = -S_STEREO_SWING;
	}
	else
	{
		stereo_swing = S_STEREO_SWING;
	}
}
