
//**************************************************************************
//**
//** I_IBM.C
//**
//**************************************************************************

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <graph.h>
#include "h2def.h"
#include "r_local.h"
#include "p_local.h"    // for P_AproxDistance
#include "sounds.h"
#include "i_sound.h"
#include "soundst.h"
#include "st_start.h"
#include "dmx.h"
#include "dpmiapi.h"

// Macros

#define DEFAULT_ARCHIVEPATH     "o:\\sound\\archive\\"
#define PRIORITY_MAX_ADJUST 10
#define DIST_ADJUST (MAX_SND_DIST/PRIORITY_MAX_ADJUST)

#define DPMI_INT 0x31

#define SEQ_ADDR 0x3C4
#define SEQ_DATA 0x3C5
#define REG_MAPMASK 0x02

#define MASK_PLANE0 0x01
#define MASK_PLANE1 0x02
#define MASK_PLANE2 0x04
#define MASK_PLANE3 0x08

#define P0OFFSET 38400*0
#define P1OFFSET 38400*1
#define P2OFFSET 38400*2
#define P3OFFSET 38400*3

#define VID_INT 0x10
#define VB_SYNC {while(!(inp(0x3da)&8)); while(inp(0x3da)&8);}
#define BITPLANE(p) (outp(SEQ_ADDR,REG_MAPMASK),outp(SEQ_DATA,(p)))

//#define NOKBD
//#define NOTIMER

// Public Data

int DisplayTicker = 0;

// Code

void main(int argc, char **argv)
{
	myargc = argc;
	myargv = argv;
	H2_Main();
}

void I_StartupNet (void);
void I_ShutdownNet (void);
void I_ReadExternDriver(void);

typedef struct
{
	unsigned        edi, esi, ebp, reserved, ebx, edx, ecx, eax;
	unsigned short  flags, es, ds, fs, gs, ip, cs, sp, ss;
} dpmiregs_t;

extern  dpmiregs_t      dpmiregs;

void I_ReadMouse (void);

extern  int     usemouse, usejoystick;

extern void **lumpcache;

int i_Vector;
externdata_t *i_ExternData;
boolean useexterndriver;

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
static int RegisteredSong; //the current registered song.
static int NextCleanup;
static boolean MusicPaused;
static int Mus_Song = -1;
static int Mus_LumpNum;
static void *Mus_SndPtr;
static byte *SoundCurve;

static boolean UseSndScript;
static char ArchivePath[128];

extern int snd_MusicDevice;
extern int snd_SfxDevice;
extern int snd_MaxVolume;
extern int snd_MusicVolume;
extern int snd_Channels;

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
	int track;

	if(i_CDMusic)
	{ // Play a CD track, instead
		if(i_CDTrack)
		{ // Default to the player-chosen track
			track = i_CDTrack;
		}
		else
		{
			track = P_GetMapCDTrack(gamemap);
		}
		if(track == i_CDCurrentTrack && i_CDMusicLength > 0)
		{
			return;
		}
		if(!I_CDMusPlay(track))
		{
			if(loop)
			{
				i_CDMusicLength = 35*I_CDMusTrackLength(track);
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
		if(song == Mus_Song)
		{ // don't replay an old song
			return;
		}
		if(RegisteredSong)
		{
			I_StopSong(RegisteredSong);
			I_UnRegisterSong(RegisteredSong);
			if(UseSndScript)
			{
				Z_Free(Mus_SndPtr);
			}
			else
			{
				Z_ChangeTag(lumpcache[Mus_LumpNum], PU_CACHE);
			}
			#ifdef __WATCOMC__
				_dpmi_unlockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
			#endif
			RegisteredSong = 0;
		}
		songLump = P_GetMapSongLump(song);
		if(!songLump)
		{
			return;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			M_ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = W_GetNumForName(songLump);
			Mus_SndPtr = W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		#ifdef __WATCOMC__
			_dpmi_lockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
		#endif
		RegisteredSong = I_RegisterSong(Mus_SndPtr);
		I_PlaySong(RegisteredSong, loop); // 'true' denotes endless looping.
		Mus_Song = song;
	}
}

//==========================================================================
//
// S_StartSongName
//
//==========================================================================

void S_StartSongName(char *songLump, boolean loop)
{
	int cdTrack;

	if(!songLump)
	{
		return;
	}
	if(i_CDMusic)
	{
		cdTrack = 0;

		if(!strcmp(songLump, "hexen"))
		{
			cdTrack = P_GetCDTitleTrack();
		}
		else if(!strcmp(songLump, "hub"))
		{
			cdTrack = P_GetCDIntermissionTrack();
		}
		else if(!strcmp(songLump, "hall"))
		{
			cdTrack = P_GetCDEnd1Track();
		}
		else if(!strcmp(songLump, "orb"))
		{
			cdTrack = P_GetCDEnd2Track();
		}
		else if(!strcmp(songLump, "chess") && !i_CDTrack)
		{
			cdTrack = P_GetCDEnd3Track();
		}
/*	Uncomment this, if Kevin writes a specific song for startup
		else if(!strcmp(songLump, "start"))
		{
			cdTrack = P_GetCDStartTrack();
		}
*/
		if(!cdTrack || (cdTrack == i_CDCurrentTrack && i_CDMusicLength > 0))
		{
			return;
		}
		if(!I_CDMusPlay(cdTrack))
		{
			if(loop)
			{
				i_CDMusicLength = 35*I_CDMusTrackLength(cdTrack);
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
		if(RegisteredSong)
		{
			I_StopSong(RegisteredSong);
			I_UnRegisterSong(RegisteredSong);
			if(UseSndScript)
			{
				Z_Free(Mus_SndPtr);
			}
			else
			{
				Z_ChangeTag(lumpcache[Mus_LumpNum], PU_CACHE);
			}
			#ifdef __WATCOMC__
				_dpmi_unlockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
			#endif
			RegisteredSong = 0;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			M_ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = W_GetNumForName(songLump);
			Mus_SndPtr = W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		#ifdef __WATCOMC__
			_dpmi_lockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
		#endif
		RegisteredSong = I_RegisterSong(Mus_SndPtr);
		I_PlaySong(RegisteredSong, loop); // 'true' denotes endless looping.
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

	for(i = 0; i < NUMSFX; i++)
	{
		if(!strcmp(S_sfx[i].tagName, name))
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

void S_StartSound(mobj_t *origin, int sound_id)
{
	S_StartSoundAtVolume(origin, sound_id, 127);
}

//==========================================================================
//
// S_StartSoundAtVolume
//
//==========================================================================

void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume)
{
	int dist, vol;
	int i;
	int priority;
	int sep;
	int angle;
	int absx;
	int absy;

	static int sndcount = 0;
	int chan;

	if(sound_id == 0 || snd_MaxVolume == 0)
		return;
	if(origin == NULL)
	{
		origin = players[displayplayer].mo;
	}
	if(volume == 0)
	{
		return;
	}

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	absx = abs(origin->x-players[displayplayer].mo->x);
	absy = abs(origin->y-players[displayplayer].mo->y);
	dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
	dist >>= FRACBITS;
	if(dist >= MAX_SND_DIST)
	{
	  return; // sound is beyond the hearing range...
	}
	if(dist < 0)
	{
		dist = 0;
	}
	priority = S_sfx[sound_id].priority;
	priority *= (PRIORITY_MAX_ADJUST-(dist/DIST_ADJUST));
	if(!S_StopSoundID(sound_id, priority))
	{
		return; // other sounds have greater priority
	}
	for(i=0; i<snd_Channels; i++)
	{
		if(origin->player)
		{
			i = snd_Channels;
			break; // let the player have more than one sound.
		}
		if(origin == Channel[i].mo)
		{ // only allow other mobjs one sound
			S_StopSound(Channel[i].mo);
			break;
		}
	}
	if(i >= snd_Channels)
	{
		for(i = 0; i < snd_Channels; i++)
		{
			if(Channel[i].mo == NULL)
			{
				break;
			}
		}
		if(i >= snd_Channels)
		{
			// look for a lower priority sound to replace.
			sndcount++;
			if(sndcount >= snd_Channels)
			{
				sndcount = 0;
			}
			for(chan = 0; chan < snd_Channels; chan++)
			{
				i = (sndcount+chan)%snd_Channels;
				if(priority >= Channel[i].priority)
				{
					chan = -1; //denote that sound should be replaced.
					break;
				}
			}
			if(chan != -1)
			{
				return; //no free channels.
			}
			else //replace the lower priority sound.
			{
				if(Channel[i].handle)
				{
					if(I_SoundIsPlaying(Channel[i].handle))
					{
						I_StopSound(Channel[i].handle);
					}
					if(S_sfx[Channel[i].sound_id].usefulness > 0)
					{
						S_sfx[Channel[i].sound_id].usefulness--;
					}
				}
			}
		}
	}
	if(S_sfx[sound_id].lumpnum == 0)
	{
		S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
	}
	if(S_sfx[sound_id].snd_ptr == NULL)
	{
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, S_sfx[sound_id].lumpname);
			M_ReadFile(name, (byte **)&S_sfx[sound_id].snd_ptr);
		}
		else
		{
			S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
				PU_SOUND);
		}
		#ifdef __WATCOMC__
		_dpmi_lockregion(S_sfx[sound_id].snd_ptr,
			lumpinfo[S_sfx[sound_id].lumpnum].size);
		#endif
	}

	vol = (SoundCurve[dist]*(snd_MaxVolume*8)*volume)>>14;
	if(origin == players[displayplayer].mo)
	{
		sep = 128;
//              vol = (volume*(snd_MaxVolume+1)*8)>>7;
	}
	else
	{
		angle = R_PointToAngle2(players[displayplayer].mo->x,
			players[displayplayer].mo->y, Channel[i].mo->x, Channel[i].mo->y);
		angle = (angle-viewangle)>>24;
		sep = angle*2-128;
		if(sep < 64)
			sep = -sep;
		if(sep > 192)
			sep = 512-sep;
//              vol = SoundCurve[dist];
	}

	if(S_sfx[sound_id].changePitch)
	{
		Channel[i].pitch = (byte)(127+(M_Random()&7)-(M_Random()&7));
	}
	else
	{
		Channel[i].pitch = 127;
	}
	Channel[i].handle = I_StartSound(sound_id, S_sfx[sound_id].snd_ptr, vol,
		sep, Channel[i].pitch, 0);
	Channel[i].mo = origin;
	Channel[i].sound_id = sound_id;
	Channel[i].priority = priority;
	Channel[i].volume = volume;
	if(S_sfx[sound_id].usefulness < 0)
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
	int lp; //least priority
	int found;

	if(S_sfx[sound_id].numchannels == -1)
	{
		return(true);
	}
	lp = -1; //denote the argument sound_id
	found = 0;
	for(i=0; i<snd_Channels; i++)
	{
		if(Channel[i].sound_id == sound_id && Channel[i].mo)
		{
			found++; //found one.  Now, should we replace it??
			if(priority >= Channel[i].priority)
			{ // if we're gonna kill one, then this'll be it
				lp = i;
				priority = Channel[i].priority;
			}
		}
	}
	if(found < S_sfx[sound_id].numchannels)
	{
		return(true);
	}
	else if(lp == -1)
	{
		return(false); // don't replace any sounds
	}
	if(Channel[lp].handle)
	{
		if(I_SoundIsPlaying(Channel[lp].handle))
		{
			I_StopSound(Channel[lp].handle);
		}
		if(S_sfx[Channel[lp].sound_id].usefulness > 0)
		{
			S_sfx[Channel[lp].sound_id].usefulness--;
		}
		Channel[lp].mo = NULL;
	}
	return(true);
}

//==========================================================================
//
// S_StopSound
//
//==========================================================================

void S_StopSound(mobj_t *origin)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(Channel[i].mo == origin)
		{
			I_StopSound(Channel[i].handle);
			if(S_sfx[Channel[i].sound_id].usefulness > 0)
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
	for(i=0; i < snd_Channels; i++)
	{
		if(Channel[i].handle)
		{
			S_StopSound(Channel[i].mo);
		}
	}
	memset(Channel, 0, 8*sizeof(channel_t));
}

//==========================================================================
//
// S_SoundLink
//
//==========================================================================

void S_SoundLink(mobj_t *oldactor, mobj_t *newactor)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(Channel[i].mo == oldactor)
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
	if(i_CDMusic)
	{
		I_CDMusStop();
	}
	else
	{
		I_PauseSong(RegisteredSong);
	}
}

//==========================================================================
//
// S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	if(i_CDMusic)
	{
		I_CDMusResume();
	}
	else
	{
		I_ResumeSong(RegisteredSong);
	}
}

//==========================================================================
//
// S_UpdateSounds
//
//==========================================================================

void S_UpdateSounds(mobj_t *listener)
{
	int i, dist, vol;
	int angle;
	int sep;
	int priority;
	int absx;
	int absy;

	if(i_CDMusic)
	{
		I_UpdateCDMusic();
	}
	if(snd_MaxVolume == 0)
	{
		return;
	}

	// Update any Sequences
	SN_UpdateActiveSequences();

	if(NextCleanup < gametic)
	{
		if(UseSndScript)
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
				{
					S_sfx[i].usefulness = -1;
				}
			}
		}
		else
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
				{
					if(lumpcache[S_sfx[i].lumpnum])
					{
						if(((memblock_t *)((byte*)
							(lumpcache[S_sfx[i].lumpnum])-
							sizeof(memblock_t)))->id == 0x1d4a11)
						{ // taken directly from the Z_ChangeTag macro
							Z_ChangeTag2(lumpcache[S_sfx[i].lumpnum],
								PU_CACHE);
#ifdef __WATCOMC__
								_dpmi_unlockregion(S_sfx[i].snd_ptr,
									lumpinfo[S_sfx[i].lumpnum].size);
#endif
						}
					}
					S_sfx[i].usefulness = -1;
					S_sfx[i].snd_ptr = NULL;
				}
			}
		}
		NextCleanup = gametic+35*30; // every 30 seconds
	}
	for(i=0;i<snd_Channels;i++)
	{
		if(!Channel[i].handle || S_sfx[Channel[i].sound_id].usefulness == -1)
		{
			continue;
		}
		if(!I_SoundIsPlaying(Channel[i].handle))
		{
			if(S_sfx[Channel[i].sound_id].usefulness > 0)
			{
				S_sfx[Channel[i].sound_id].usefulness--;
			}
			Channel[i].handle = 0;
			Channel[i].mo = NULL;
			Channel[i].sound_id = 0;
		}
		if(Channel[i].mo == NULL || Channel[i].sound_id == 0
			|| Channel[i].mo == listener)
		{
			continue;
		}
		else
		{
			absx = abs(Channel[i].mo->x-listener->x);
			absy = abs(Channel[i].mo->y-listener->y);
			dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
			dist >>= FRACBITS;

			if(dist >= MAX_SND_DIST)
			{
				S_StopSound(Channel[i].mo);
				continue;
			}
			if(dist < 0)
			{
				dist = 0;
			}
			//vol = SoundCurve[dist];
			vol = (SoundCurve[dist]*(snd_MaxVolume*8)*Channel[i].volume)>>14;
			if(Channel[i].mo == listener)
			{
				sep = 128;
			}
			else
			{
				angle = R_PointToAngle2(listener->x, listener->y,
								Channel[i].mo->x, Channel[i].mo->y);
				angle = (angle-viewangle)>>24;
				sep = angle*2-128;
				if(sep < 64)
					sep = -sep;
				if(sep > 192)
					sep = 512-sep;
			}
			I_UpdateSoundParams(Channel[i].handle, vol, sep,
				Channel[i].pitch);
			priority = S_sfx[Channel[i].sound_id].priority;
			priority *= PRIORITY_MAX_ADJUST-(dist/DIST_ADJUST);
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
	I_StartupSound();
	if(snd_Channels > 8)
	{
		snd_Channels = 8;
	}
	I_SetChannels(snd_Channels);
	I_SetMusicVolume(snd_MusicVolume);

	// Attempt to setup CD music
	if(snd_MusicDevice == snd_CDMUSIC)
	{
	   	ST_Message("    Attempting to initialize CD Music: ");
		if(!cdrom)
		{
			i_CDMusic = (I_CDMusInit() != -1);
		}
		else
		{ // The user is trying to use the cdrom for both game and music
			i_CDMusic = false;
		}
	   	if(i_CDMusic)
	   	{
	   		ST_Message("initialized.\n");
	   	}
	   	else
	   	{
	   		ST_Message("failed.\n");
	   	}
	}
}

//==========================================================================
//
// S_GetChannelInfo
//
//==========================================================================

void S_GetChannelInfo(SoundInfo_t *s)
{
	int i;
	ChanInfo_t *c;

	s->channelCount = snd_Channels;
	s->musicVolume = snd_MusicVolume;
	s->soundVolume = snd_MaxVolume;
	for(i = 0; i < snd_Channels; i++)
	{
		c = &s->chan[i];
		c->id = Channel[i].sound_id;
		c->priority = Channel[i].priority;
		c->name = S_sfx[c->id].lumpname;
		c->mo = Channel[i].mo;
		c->distance = P_AproxDistance(c->mo->x-viewx, c->mo->y-viewy)
			>>FRACBITS;
	}
}

//==========================================================================
//
// S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(mobj_t *mobj, int sound_id)
{
	int i;

	for(i = 0; i < snd_Channels; i++)
	{
		if(Channel[i].sound_id == sound_id && Channel[i].mo == mobj)
		{
			if(I_SoundIsPlaying(Channel[i].handle))
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
	if(i_CDMusic)
	{
		I_CDMusSetVolume(snd_MusicVolume*16); // 0-255
	}
	else
	{
		I_SetMusicVolume(snd_MusicVolume);
	}
	if(snd_MusicVolume == 0)
	{
		if(!i_CDMusic)
		{
			I_PauseSong(RegisteredSong);
		}
		MusicPaused = true;
	}
	else if(MusicPaused)
	{
		if(!i_CDMusic)
		{
			I_ResumeSong(RegisteredSong);
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
	extern int tsm_ID;
	if(tsm_ID != -1)
	{
		I_StopSong(RegisteredSong);
		I_UnRegisterSong(RegisteredSong);
		I_ShutdownSound();
	}
	if(i_CDMusic)
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
	int p;
	int i;

	strcpy(ArchivePath, DEFAULT_ARCHIVEPATH);
	if(!(p = M_CheckParm("-devsnd")))
	{
		UseSndScript = false;
		SC_OpenLump("sndinfo");
	}
	else
	{
		UseSndScript = true;
		SC_OpenFile(myargv[p+1]);
	}
	while(SC_GetString())
	{
		if(*sc_String == '$')
		{
			if(!stricmp(sc_String, "$ARCHIVEPATH"))
			{
				SC_MustGetString();
				strcpy(ArchivePath, sc_String);
			}
			else if(!stricmp(sc_String, "$MAP"))
			{
				SC_MustGetNumber();
				SC_MustGetString();
				if(sc_Number)
				{
					P_PutMapSongLump(sc_Number, sc_String);
				}
			}
			continue;
		}
		else
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(!strcmp(S_sfx[i].tagName, sc_String))
				{
					SC_MustGetString();
					if(*sc_String != '?')
					{
						strcpy(S_sfx[i].lumpname, sc_String);
					}
					else
					{
						strcpy(S_sfx[i].lumpname, "default");
					}
					break;
				}
			}
			if(i == NUMSFX)
			{
				SC_MustGetString();
			}
		}
	}
	SC_Close();

	for(i = 0; i < NUMSFX; i++)
	{
		if(!strcmp(S_sfx[i].lumpname, ""))
		{
			strcpy(S_sfx[i].lumpname, "default");
		}
	}
}

//==========================================================================
//
// I_UpdateCDMusic
//
// Updates playing time for current track, and restarts the track, if
// needed
//
//==========================================================================

void I_UpdateCDMusic(void)
{
	extern boolean MenuActive;

	if(MusicPaused || i_CDMusicLength < 0
	|| (paused && !MenuActive))
	{ // Non-looping song/song paused
		return;
	}
	i_CDMusicLength -= gametic-oldTic;
	oldTic = gametic;
	if(i_CDMusicLength <= 0)
	{
		S_StartSong(gamemap, true);
	}
}

/*
============================================================================

							CONSTANTS

============================================================================
*/

#define SC_INDEX                0x3C4
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_H_TOTAL    0
#define CRTC_H_DISPEND  1
#define CRTC_H_BLANK    2
#define CRTC_H_ENDBLANK 3
#define CRTC_H_RETRACE  4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL    6
#define CRTC_OVERFLOW   7
#define CRTC_ROWSCAN    8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND  11
#define CRTC_STARTHIGH  12
#define CRTC_STARTLOW   13
#define CRTC_CURSORHIGH 14
#define CRTC_CURSORLOW  15
#define CRTC_V_RETRACE  16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND  18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE  20
#define CRTC_V_BLANK    21
#define CRTC_V_ENDBLANK 22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE 24


#define GC_INDEX                0x3CE
#define GC_SETRESET             0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE 2
#define GC_DATAROTATE   3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN    17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN              19
#define ATR_COLORSELECT 20

#define STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA                0x3c9
#define PEL_MASK                0x3c6

// boolean grmode;

//==================================================
//
// joystick vars
//
//==================================================

boolean         joystickpresent;
extern  unsigned        joystickx, joysticky;
boolean I_ReadJoystick (void);          // returns false if not connected


//==================================================

#define VBLCOUNTER              34000           // hardware tics to a frame


#define TIMERINT 8
#define KEYBOARDINT 9

#define CRTCOFF (_inbyte(STATUS_REGISTER_1)&1)
#define CLI     _disable()
#define STI     _enable()

#define _outbyte(x,y) (outp(x,y))
#define _outhword(x,y) (outpw(x,y))

#define _inbyte(x) (inp(x))
#define _inhword(x) (inpw(x))

#define MOUSEB1 1
#define MOUSEB2 2
#define MOUSEB3 4

boolean mousepresent;
//static  int tsm_ID = -1; // tsm init flag

//===============================

int             ticcount;

// REGS stuff used for int calls
union REGS regs;
struct SREGS segregs;

boolean novideo; // if true, stay in text mode for debugging

#define KBDQUESIZE 32
byte keyboardque[KBDQUESIZE];
int kbdtail, kbdhead;

#define KEY_LSHIFT      0xfe

#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)

#define SC_RSHIFT       0x36
#define SC_LSHIFT       0x2a

byte        scantokey[128] =
					{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    KEY_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13 ,    KEY_RCTRL,'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	39 ,    '`',    KEY_LSHIFT,92,  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '/',    KEY_RSHIFT,'*',
	KEY_RALT,' ',   0  ,    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,   // 3
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,0  ,    0  , KEY_HOME,
	KEY_UPARROW,KEY_PGUP,'-',KEY_LEFTARROW,'5',KEY_RIGHTARROW,'+',KEY_END, //4
	KEY_DOWNARROW,KEY_PGDN,KEY_INS,KEY_DEL,0,0,             0,              KEY_F11,
	KEY_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7
					};

//==========================================================================

//--------------------------------------------------------------------------
//
// FUNC I_GetTime
//
// Returns time in 1/35th second tics.
//
//--------------------------------------------------------------------------

int I_GetTime (void)
{
#ifdef NOTIMER
	ticcount++;
#endif
	return(ticcount);
}

//--------------------------------------------------------------------------
//
// PROC I_ColorBorder
//
//--------------------------------------------------------------------------

/*
void I_ColorBorder(void)
{
	int i;

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 3; i++)
	{
		_outbyte(PEL_DATA, 63);
	}
}
*/

//--------------------------------------------------------------------------
//
// PROC I_UnColorBorder
//
//--------------------------------------------------------------------------

/*
void I_UnColorBorder(void)
{
	int i;

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 3; i++)
	{
		_outbyte(PEL_DATA, 0);
	}
}
*/

/*
============================================================================

								USER INPUT

============================================================================
*/

//--------------------------------------------------------------------------
//
// PROC I_WaitVBL
//
//--------------------------------------------------------------------------

void I_WaitVBL(int vbls)
{
	int stat;

	if(novideo)
	{
		return;
	}
	while(vbls--)
	{
		do
		{
			stat = inp(STATUS_REGISTER_1);
			if(stat&8)
			{
				break;
			}
		} while(1);
		do
		{
			stat = inp(STATUS_REGISTER_1);
			if((stat&8) == 0)
			{
				break;
			}
		} while(1);
	}
}

//--------------------------------------------------------------------------
//
// PROC I_SetPalette
//
// Palette source must use 8 bit RGB elements.
//
//--------------------------------------------------------------------------

void I_SetPalette(byte *palette)
{
	int i;

	if(novideo)
	{
		return;
	}
	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 768; i++)
	{
		_outbyte(PEL_DATA, (gammatable[usegamma][*palette++])>>2);
	}
}

/*
============================================================================

							GRAPHICS MODE

============================================================================
*/

byte *pcscreen, *destscreen, *destview;


/*
==============
=
= I_Update
=
==============
*/

int UpdateState;
extern int screenblocks;

void I_Update (void)
{
	int i;
	byte *dest;
	int tics;
	static int lasttic;

//
// blit screen to video
//
	if(DisplayTicker)
	{
		if(screenblocks > 9 || UpdateState&(I_FULLSCRN|I_MESSAGES))
		{
			dest = (byte *)screen;
		}
		else
		{
			dest = (byte *)pcscreen;
		}
		tics = ticcount-lasttic;
		lasttic = ticcount;
		if(tics > 20)
		{
			tics = 20;
		}
		for(i = 0; i < tics; i++)
		{
			*dest = 0xff;
			dest += 2;
		}
		for(i = tics; i < 20; i++)
		{
			*dest = 0x00;
			dest += 2;
		}
	}

	//memset(pcscreen, 255, SCREENHEIGHT*SCREENWIDTH);

	if(UpdateState == I_NOUPDATE)
	{
		return;
	}
	if(UpdateState&I_FULLSCRN)
	{
		memcpy(pcscreen, screen, SCREENWIDTH*SCREENHEIGHT);
		UpdateState = I_NOUPDATE; // clear out all draw types
	}
	if(UpdateState&I_FULLVIEW)
	{
		if(UpdateState&I_MESSAGES && screenblocks > 7)
		{
			for(i = 0; i <
				(viewwindowy+viewheight)*SCREENWIDTH; i += SCREENWIDTH)
			{
				memcpy(pcscreen+i, screen+i, SCREENWIDTH);
			}
			UpdateState &= ~(I_FULLVIEW|I_MESSAGES);
		}
		else
		{
			for(i = viewwindowy*SCREENWIDTH+viewwindowx; i <
				(viewwindowy+viewheight)*SCREENWIDTH; i += SCREENWIDTH)
			{
				memcpy(pcscreen+i, screen+i, viewwidth);
			}
			UpdateState &= ~I_FULLVIEW;
		}
	}
	if(UpdateState&I_STATBAR)
	{
		memcpy(pcscreen+SCREENWIDTH*(SCREENHEIGHT-SBARHEIGHT),
			screen+SCREENWIDTH*(SCREENHEIGHT-SBARHEIGHT),
			SCREENWIDTH*SBARHEIGHT);
		UpdateState &= ~I_STATBAR;
	}
	if(UpdateState&I_MESSAGES)
	{
		memcpy(pcscreen, screen, SCREENWIDTH*28);
		UpdateState &= ~I_MESSAGES;
	}


//  memcpy(pcscreen, screen, SCREENHEIGHT*SCREENWIDTH);
}

//--------------------------------------------------------------------------
//
// PROC I_InitGraphics
//
//--------------------------------------------------------------------------

void I_InitGraphics(void)
{
	if(novideo)
	{
		return;
	}
	//grmode = true;
	regs.w.ax = 0x13;
	int386(0x10, (const union REGS *)&regs, &regs);
	pcscreen = destscreen = (byte *)0xa0000;
	I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}

//--------------------------------------------------------------------------
//
// PROC I_ShutdownGraphics
//
//--------------------------------------------------------------------------

void I_ShutdownGraphics(void)
{
	byte mode;

	// don't reset mode if it didn't get set
	mode = *(byte *)0x449;
	if(mode == 0x12 || mode == 0x13)
	{
		regs.w.ax = 3;
		int386(0x10, &regs, &regs); // back to text mode
	}
}

//--------------------------------------------------------------------------
//
// PROC I_ReadScreen
//
// Reads the screen currently displayed into a linear buffer.
//
//--------------------------------------------------------------------------

/*
void I_ReadScreen(byte *scr)
{
	memcpy(scr, screen, SCREENWIDTH*SCREENHEIGHT);
}
*/

//===========================================================================

/*
===================
=
= I_StartTic
=
// called by D_DoomLoop
// called before processing each tic in a frame
// can call D_PostEvent
// asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events
===================
*/

/*
 OLD STARTTIC STUFF

void   I_StartTic (void)
{
	int             k;
	event_t ev;


	I_ReadMouse ();

//
// keyboard events
//
	while (kbdtail < kbdhead)
	{
		k = keyboardque[kbdtail&(KBDQUESIZE-1)];

//      if (k==14)
//              I_Error ("exited");

		kbdtail++;

		// extended keyboard shift key bullshit
		if ( (k&0x7f)==KEY_RSHIFT )
		{
			if ( keyboardque[(kbdtail-2)&(KBDQUESIZE-1)]==0xe0 )
				continue;
			k &= 0x80;
			k |= KEY_RSHIFT;
		}

		if (k==0xe0)
			continue;               // special / pause keys
		if (keyboardque[(kbdtail-2)&(KBDQUESIZE-1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k==0xc5 && keyboardque[(kbdtail-2)&(KBDQUESIZE-1)] == 0x9d)
		{
			ev.type = ev_keydown;
			ev.data1 = KEY_PAUSE;
			D_PostEvent (&ev);
			continue;
		}

		if (k&0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;
		k &= 0x7f;

		ev.data1 = k;
		//ev.data1 = scantokey[k];

		D_PostEvent (&ev);
	}
}
*/

#define SC_UPARROW              0x48
#define SC_DOWNARROW    0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW   0x4d

void   I_StartTic (void)
{
	int             k;
	event_t ev;


	I_ReadMouse ();

//
// keyboard events
//
	while (kbdtail < kbdhead)
	{
		k = keyboardque[kbdtail&(KBDQUESIZE-1)];
		kbdtail++;

		// extended keyboard shift key bullshit
		if ( (k&0x7f)==SC_LSHIFT || (k&0x7f)==SC_RSHIFT )
		{
			if ( keyboardque[(kbdtail-2)&(KBDQUESIZE-1)]==0xe0 )
				continue;
			k &= 0x80;
			k |= SC_RSHIFT;
		}

		if (k==0xe0)
			continue;               // special / pause keys
		if (keyboardque[(kbdtail-2)&(KBDQUESIZE-1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k==0xc5 && keyboardque[(kbdtail-2)&(KBDQUESIZE-1)] == 0x9d)
		{
			ev.type = ev_keydown;
			ev.data1 = KEY_PAUSE;
			H2_PostEvent (&ev);
			continue;
		}

		if (k&0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;
		k &= 0x7f;
		switch (k)
		{
		case SC_UPARROW:
			ev.data1 = KEY_UPARROW;
			break;
		case SC_DOWNARROW:
			ev.data1 = KEY_DOWNARROW;
			break;
		case SC_LEFTARROW:
			ev.data1 = KEY_LEFTARROW;
			break;
		case SC_RIGHTARROW:
			ev.data1 = KEY_RIGHTARROW;
			break;
		default:
			ev.data1 = scantokey[k];
			break;
		}
		H2_PostEvent (&ev);
	}

}


/*
void   I_ReadKeys (void)
{
	int             k;


	while (1)
	{
	   while (kbdtail < kbdhead)
	   {
		   k = keyboardque[kbdtail&(KBDQUESIZE-1)];
		   kbdtail++;
		   printf ("0x%x\n",k);
		   if (k == 1)
			   I_Quit ();
	   }
	}
}
*/

/*
===============
=
= I_StartFrame
=
===============
*/

void I_StartFrame (void)
{
	I_JoystickEvents ();
	if(useexterndriver)
	{
		DPMIInt(i_Vector);
	}
}

/*
============================================================================

					TIMER INTERRUPT

============================================================================
*/

/*
void I_ColorBlack (int r, int g, int b)
{
_outbyte (PEL_WRITE_ADR,0);
_outbyte(PEL_DATA,r);
_outbyte(PEL_DATA,g);
_outbyte(PEL_DATA,b);
}
*/


/*
================
=
= I_TimerISR
=
================
*/

int I_TimerISR (void)
{
	ticcount++;
	return 0;
}

/*
============================================================================

						KEYBOARD

============================================================================
*/

void (__interrupt __far *oldkeyboardisr) () = NULL;

int lastpress;

/*
================
=
= I_KeyboardISR
=
================
*/

void __interrupt I_KeyboardISR (void)
{
// Get the scan code

	keyboardque[kbdhead&(KBDQUESIZE-1)] = lastpress = _inbyte(0x60);
	kbdhead++;

// acknowledge the interrupt

	_outbyte(0x20,0x20);
}



/*
===============
=
= I_StartupKeyboard
=
===============
*/

void I_StartupKeyboard (void)
{
#ifndef NOKBD
	oldkeyboardisr = _dos_getvect(KEYBOARDINT);
	_dos_setvect (0x8000 | KEYBOARDINT, I_KeyboardISR);
#endif

//I_ReadKeys ();
}


void I_ShutdownKeyboard (void)
{
	if (oldkeyboardisr)
		_dos_setvect (KEYBOARDINT, oldkeyboardisr);
	*(short *)0x41c = *(short *)0x41a;      // clear bios key buffer
}



/*
============================================================================

							MOUSE

============================================================================
*/


int I_ResetMouse (void)
{
	regs.w.ax = 0;                  // reset
	int386 (0x33, &regs, &regs);
	return regs.w.ax;
}



/*
================
=
= StartupMouse
=
================
*/

void I_StartupCyberMan(void);

void I_StartupMouse (void)
{
   //
   // General mouse detection
   //
	mousepresent = 0;
	if ( M_CheckParm ("-nomouse") || !usemouse )
		return;

	if (I_ResetMouse () != 0xffff)
	{
		ST_Message("Mouse: not present\n");
		return;
	}
	ST_Message("Mouse: detected\n");

	mousepresent = 1;

	I_StartupCyberMan();
}


/*
================
=
= ShutdownMouse
=
================
*/

void I_ShutdownMouse (void)
{
	if (!mousepresent)
	  return;

	I_ResetMouse ();
}


/*
================
=
= I_ReadMouse
=
================
*/

void I_ReadMouse (void)
{
	event_t ev;

//
// mouse events
//
	if (!mousepresent)
		return;

	ev.type = ev_mouse;

	memset (&dpmiregs,0,sizeof(dpmiregs));
	dpmiregs.eax = 3;                               // read buttons / position
	DPMIInt (0x33);
	ev.data1 = dpmiregs.ebx;

	dpmiregs.eax = 11;                              // read counters
	DPMIInt (0x33);
	ev.data2 = (short)dpmiregs.ecx;
	ev.data3 = -(short)dpmiregs.edx;

	H2_PostEvent (&ev);
}

/*
============================================================================

					JOYSTICK

============================================================================
*/

int     joyxl, joyxh, joyyl, joyyh;

boolean WaitJoyButton (void)
{
	int             oldbuttons, buttons;

	oldbuttons = 0;
	do
	{
		I_WaitVBL (1);
		buttons =  ((inp(0x201) >> 4)&1)^1;
		if (buttons != oldbuttons)
		{
			oldbuttons = buttons;
			continue;
		}

		if ( (lastpress& 0x7f) == 1 )
		{
			joystickpresent = false;
			return false;
		}
	} while ( !buttons);

	do
	{
		I_WaitVBL (1);
		buttons =  ((inp(0x201) >> 4)&1)^1;
		if (buttons != oldbuttons)
		{
			oldbuttons = buttons;
			continue;
		}

		if ( (lastpress& 0x7f) == 1 )
		{
			joystickpresent = false;
			return false;
		}
	} while ( buttons);

	return true;
}



/*
===============
=
= I_StartupJoystick
=
===============
*/

int             basejoyx, basejoyy;

void I_StartupJoystick (void)
{
	int     centerx, centery;

	joystickpresent = 0;
	if ( M_CheckParm ("-nojoy") || !usejoystick )
		return;

	if (!I_ReadJoystick ())
	{
		joystickpresent = false;
		ST_Message("joystick not found\n ");
		return;
	}
	ST_Message("joystick found\n");
	joystickpresent = true;

	ST_RealMessage("CENTER the joystick and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	centerx = joystickx;
	centery = joysticky;

	ST_RealMessage("\nPush the joystick to the UPPER LEFT corner and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	joyxl = (centerx + joystickx)/2;
	joyyl = (centerx + joysticky)/2;

	ST_RealMessage("\nPush the joystick to the LOWER RIGHT corner and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	joyxh = (centerx + joystickx)/2;
	joyyh = (centery + joysticky)/2;
	ST_RealMessage("\n");
}

/*
===============
=
= I_JoystickEvents
=
===============
*/

void I_JoystickEvents (void)
{
	event_t ev;

//
// joystick events
//
	if (!joystickpresent)
		return;

	I_ReadJoystick ();
	ev.type = ev_joystick;
	ev.data1 =  ((inp(0x201) >> 4)&15)^15;

	if (joystickx < joyxl)
		ev.data2 = -1;
	else if (joystickx > joyxh)
		ev.data2 = 1;
	else
		ev.data2 = 0;
	if (joysticky < joyyl)
		ev.data3 = -1;
	else if (joysticky > joyyh)
		ev.data3 = 1;
	else
		ev.data3 = 0;

	H2_PostEvent (&ev);
}



/*
============================================================================

					DPMI STUFF

============================================================================
*/

#define REALSTACKSIZE   1024

dpmiregs_t      dpmiregs;

unsigned                realstackseg;

void I_DivException (void);
int I_SetDivException (void);

/*
void DPMIFarCall (void)
{
	segread (&segregs);
	regs.w.ax = 0x301;
	regs.w.bx = 0;
	regs.w.cx = 0;
	regs.x.edi = (unsigned)&dpmiregs;
	segregs.es = segregs.ds;
	int386x( DPMI_INT, &regs, &regs, &segregs );
}
*/

void DPMIInt (int i)
{
	dpmiregs.ss = realstackseg;
	dpmiregs.sp = REALSTACKSIZE-4;

	segread (&segregs);
	regs.w.ax = 0x300;
	regs.w.bx = i;
	regs.w.cx = 0;
	regs.x.edi = (unsigned)&dpmiregs;
	segregs.es = segregs.ds;
	int386x( DPMI_INT, &regs, &regs, &segregs );
}


/*
==============
=
= I_StartupDPMI
=
==============
*/

void I_StartupDPMI (void)
{
//	extern char __begtext;
//	extern char ___argc;
//	int     n,d;

//
// allocate a decent stack for real mode ISRs
//
	realstackseg = (int)I_AllocLow (1024) >> 4;

//
// lock the entire program down
//

//      _dpmi_lockregion (&__begtext, &___argc - &__begtext);


//
// catch divide by 0 exception
//
#if 0
	segread(&segregs);
	regs.w.ax = 0x0203;             // DPMI set processor exception handler vector
	regs.w.bx = 0;                  // int 0
	regs.w.cx = segregs.cs;
	regs.x.edx = (int)&I_DivException;
 printf ("%x : %x\n",regs.w.cx, regs.x.edx);
	int386( DPMI_INT, &regs, &regs);
#endif

#if 0
	n = I_SetDivException ();
	printf ("return: %i\n",n);
	n = 100;
	d = 0;
   printf ("100 / 0 = %i\n",n/d);

exit (1);
#endif
}



/*
============================================================================

					TIMER INTERRUPT

============================================================================
*/

void (__interrupt __far *oldtimerisr) ();


/*
void IO_ColorBlack (int r, int g, int b)
{
_outbyte (PEL_WRITE_ADR,0);
_outbyte(PEL_DATA,r);
_outbyte(PEL_DATA,g);
_outbyte(PEL_DATA,b);
}
*/


/*
================
=
= IO_TimerISR
=
================
*/

//void __interrupt IO_TimerISR (void)

void __interrupt __far IO_TimerISR (void)
{
	ticcount++;
	_outbyte(0x20,0x20);                            // Ack the interrupt
}

/*
=====================
=
= IO_SetTimer0
=
= Sets system timer 0 to the specified speed
=
=====================
*/

void IO_SetTimer0(int speed)
{
	if (speed > 0 && speed < 150)
		I_Error ("INT_SetTimer0: %i is a bad value",speed);

	_outbyte(0x43,0x36);                            // Change timer 0
	_outbyte(0x40,speed);
	_outbyte(0x40,speed >> 8);
}



/*
===============
=
= IO_StartupTimer
=
===============
*/

/*
void IO_StartupTimer (void)
{
	oldtimerisr = _dos_getvect(TIMERINT);

	_dos_setvect (0x8000 | TIMERINT, IO_TimerISR);
	IO_SetTimer0 (VBLCOUNTER);
}
*/

void IO_ShutdownTimer (void)
{
	if (oldtimerisr)
	{
		IO_SetTimer0 (0);              // back to 18.4 ips
		_dos_setvect (TIMERINT, oldtimerisr);
	}
}

//===========================================================================


/*
===============
=
= I_Init
=
= hook interrupts and set graphics mode
=
===============
*/

void I_Init (void)
{
	extern void I_StartupTimer(void);

	novideo = M_CheckParm("novideo");
	ST_Message("  I_StartupDPMI\n");
	I_StartupDPMI();
	ST_Message("  I_StartupMouse ");
	I_StartupMouse();
//	tprintf("I_StartupJoystick ",1);
//	I_StartupJoystick();
//	tprintf("I_StartupKeyboard ",1);
//	I_StartupKeyboard();
	ST_Message("  S_Init... ");
	S_Init();
	//IO_StartupTimer();
	S_Start();
}


/*
===============
=
= I_Shutdown
=
= return to default system state
=
===============
*/

void I_Shutdown (void)
{
	I_ShutdownGraphics ();
	IO_ShutdownTimer ();
	S_ShutDown ();
	I_ShutdownMouse ();
	I_ShutdownKeyboard ();

	IO_SetTimer0 (0);
}


/*
================
=
= I_Error
=
================
*/

void I_Error (char *error, ...)
{
	union REGS regs;

	va_list argptr;

	D_QuitNetGame ();
	I_Shutdown ();
	va_start (argptr,error);
	regs.x.eax = 0x3;
	int386(0x10, &regs, &regs);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	exit (1);
}

//--------------------------------------------------------------------------
//
// I_Quit
//
// Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//--------------------------------------------------------------------------

void I_Quit(void)
{
	D_QuitNetGame();
	M_SaveDefaults();
	I_Shutdown();

//	scr = (byte *)W_CacheLumpName("ENDTEXT", PU_CACHE);
/*
	memcpy((void *)0xb8000, scr, 80*25*2);
	regs.w.ax = 0x0200;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int386(0x10, (const union REGS *)&regs, &regs); // Set text pos
	_settextposition(24, 1);
*/
	printf("\nHexen: Beyond Heretic\n");
	exit(0);
}

/*
===============
=
= I_ZoneBase
=
===============
*/

byte *I_ZoneBase (int *size)
{
	int meminfo[32];
	int heap;
	byte *ptr;

	memset (meminfo,0,sizeof(meminfo));
	segread(&segregs);
	segregs.es = segregs.ds;
	regs.w.ax = 0x500;      // get memory info
	regs.x.edi = (int)&meminfo;
	int386x( 0x31, &regs, &regs, &segregs );

	heap = meminfo[0];
	ST_Message ("  DPMI memory: 0x%x, ",heap);
	ST_Message ("Maxzone: 0x%x\n", maxzone);

	do
	{
		heap -= 0x10000;                // leave 64k alone
		if (heap > maxzone)
			heap = maxzone;
		ptr = malloc (heap);
	} while (!ptr);

	ST_Message ("  0x%x allocated for zone, ", heap);
	ST_Message ("ZoneBase: 0x%X\n", (int)ptr);

	if (heap < 0x180000)
		I_Error ("  Insufficient DPMI memory!");
#if 0
	regs.w.ax = 0x501;      // allocate linear block
	regs.w.bx = heap>>16;
	regs.w.cx = heap&0xffff;
	int386( 0x31, &regs, &regs);
	if (regs.w.cflag)
		I_Error ("  Couldn't allocate DPMI memory!");

	block = (regs.w.si << 16) + regs.w.di;
#endif

	*size = heap;
	return ptr;
}

/*
=============
=
= I_AllocLow
=
=============
*/

byte *I_AllocLow (int length)
{
	byte    *mem;

	// DPMI call 100h allocates DOS memory
	segread(&segregs);
	regs.w.ax = 0x0100;          // DPMI allocate DOS memory
	regs.w.bx = (length+15) / 16;
	int386( DPMI_INT, &regs, &regs);
//      segment = regs.w.ax;
//   selector = regs.w.dx;
	if (regs.w.cflag != 0)
		I_Error ("I_AllocLow: DOS alloc of %i failed, %i free",
			length, regs.w.bx*16);


	mem = (void *) ((regs.x.eax & 0xFFFF) << 4);

	memset (mem,0,length);
	return mem;
}

/*
============================================================================

						NETWORKING

============================================================================
*/

/* // FUCKED LINES
typedef struct
{
	char    priv[508];
} doomdata_t;
*/ // FUCKED LINES

#define DOOMCOM_ID              0x12345678l

/* // FUCKED LINES
typedef struct
{
	long    id;
	short   intnum;                 // DOOM executes an int to execute commands

// communication between DOOM and the driver
	short   command;                // CMD_SEND or CMD_GET
	short   remotenode;             // dest for send, set by get (-1 = no packet)
	short   datalength;             // bytes in doomdata to be sent

// info common to all nodes
	short   numnodes;               // console is allways node 0
	short   ticdup;                 // 1 = no duplication, 2-5 = dup for slow nets
	short   extratics;              // 1 = send a backup tic in every packet
	short   deathmatch;             // 1 = deathmatch
	short   savegame;               // -1 = new game, 0-5 = load savegame
	short   episode;                // 1-3
	short   map;                    // 1-9
	short   skill;                  // 1-5

// info specific to this node
	short   consoleplayer;
	short   numplayers;
	short   angleoffset;    // 1 = left, 0 = center, -1 = right
	short   drone;                  // 1 = drone

// packet data to be sent
	doomdata_t      data;
} doomcom_t;
*/ // FUCKED LINES

extern  doomcom_t               *doomcom;

/*
====================
=
= I_InitNetwork
=
====================
*/

void I_InitNetwork (void)
{
	int             i;

	i = M_CheckParm ("-net");
	if (!i)
	{
	//
	// single player game
	//
		doomcom = malloc (sizeof (*doomcom) );
		memset (doomcom, 0, sizeof(*doomcom) );
		netgame = false;
		doomcom->id = DOOMCOM_ID;
		doomcom->numplayers = doomcom->numnodes = 1;
		doomcom->deathmatch = false;
		doomcom->consoleplayer = 0;
		doomcom->ticdup = 1;
		doomcom->extratics = 0;
		return;
	}

	netgame = true;
	doomcom = (doomcom_t *)atoi(myargv[i+1]);
//DEBUG
doomcom->skill = startskill;
doomcom->episode = startepisode;
doomcom->map = startmap;
doomcom->deathmatch = deathmatch;

}

void I_NetCmd (void)
{
	if (!netgame)
		I_Error ("I_NetCmd when not in netgame");
	DPMIInt (doomcom->intnum);
}

//=========================================================================
//
// I_CheckExternDriver
//
//		Checks to see if a vector, and an address for an external driver
//			have been passed.
//=========================================================================

void I_CheckExternDriver(void)
{
	int i;

	if(!(i = M_CheckParm("-externdriver")))
	{
		return;
	}
	i_ExternData = (externdata_t *)atoi(myargv[i+1]);
	i_Vector = i_ExternData->vector;

	useexterndriver = true;
}

//=========================================================================
//=========================================================================
// Hi-Res (mode 12) stuff
//=========================================================================
//=========================================================================


//==========================================================================
//
// SetVideoModeHR - Set video mode to 640x480x16
//
//==========================================================================


void SetVideoModeHR(void)
{
	union REGS regs;
	regs.x.eax = 0x12;
	int386(VID_INT, &regs, &regs);
}


//==========================================================================
//
// ClearScreenHR - Clear the screen to color 0
//
//==========================================================================

void ClearScreenHR(void)
{
	BITPLANE(MASK_PLANE0|MASK_PLANE1|MASK_PLANE2|MASK_PLANE3);
	memset((char *)0xa0000,0,38400);
}


//==========================================================================
//
// SlamHR - copy 4-plane buffer to screen
//
//==========================================================================

void SlamHR(char *buffer)
{
	BITPLANE(MASK_PLANE0);
	memcpy((char *)0xA0000, buffer+P0OFFSET, 38400);
	BITPLANE(MASK_PLANE1);
	memcpy((char *)0xA0000, buffer+P1OFFSET, 38400);
	BITPLANE(MASK_PLANE2);
	memcpy((char *)0xA0000, buffer+P2OFFSET, 38400);
	BITPLANE(MASK_PLANE3);
	memcpy((char *)0xA0000, buffer+P3OFFSET, 38400);
}


//==========================================================================
//
// SlamHR - copy 4-plane buffer to screen
//
// X and Width should be a multiple of 8
// src should be 4 planes of block size, back to back
//==========================================================================

void SlamBlockHR(int x, int y, int w, int h, char *src)
{
	int srcwid = w>>3;
	char *dest = ((char *)0xA0000) + (y*(640/8)) + (x>>3);
	char *dst;
	int i;

	VB_SYNC;

	BITPLANE(MASK_PLANE0);
	dst = dest;
	for ( i=0; i<h; i++ )
	{
		memcpy(dst, src, srcwid);
		dst += 640/8;
		src += srcwid;
	}
	BITPLANE(MASK_PLANE1);
	dst = dest;
	for ( i=0; i<h; i++ )
	{
		memcpy(dst, src, srcwid);
		dst += 640/8;
		src += srcwid;
	}
	BITPLANE(MASK_PLANE2);
	dst = dest;
	for ( i=0; i<h; i++ )
	{
		memcpy(dst, src, srcwid);
		dst += 640/8;
		src += srcwid;
	}
	BITPLANE(MASK_PLANE3);
	dst = dest;
	for ( i=0; i<h; i++ )
	{
		memcpy(dst, src, srcwid);
		dst += 640/8;
		src += srcwid;
	}
}

//==========================================================================
//
// InitPaletteHR
//
//==========================================================================

void InitPaletteHR(void)
{
	int i;
	union REGS regs;

	// Set palette registers to point into color registers
	for ( i=0; i<16; i++ )
	{
		regs.x.eax = (0x10<<8)|0x00;
		regs.x.ebx = (i<<8)|i;
		int386(VID_INT, &regs, &regs);
	}

}


//==========================================================================
//
// SetPaletteHR - Set the HR palette
//
//==========================================================================

void SetPaletteHR(byte *palette)
{
	int i;
	VB_SYNC;
	outp(PEL_WRITE_ADR, 0);

	for(i = 0; i < 16*3; i++)
	{
		outp(PEL_DATA, (*palette++));
	}
}


//==========================================================================
//
// GetPaletteHR - Get the HR palette
//
//==========================================================================

void GetPaletteHR(byte *palette)
{
	int i;
	outp(PEL_READ_ADR, 0);
	for (i=0; i<16*3; i++)
	{
		*palette++ = inp(PEL_DATA);
	}
}


//==========================================================================
//
// FadeToPaletteHR
//
//==========================================================================

void FadeToPaletteHR(byte *palette)
{
	int i,j;
	int steps=140;          // two-seconds
	byte basep[16*3];
	byte work[16*3];
	int delta;

	GetPaletteHR(basep);
	for(i = 0; i < steps; i++)
	{
		for(j = 0; j < 16*3; j++)
		{
			delta = palette[j]-basep[j];
			work[j] = basep[j]+delta*i/steps;
		}
		SetPaletteHR(work);
	}
	SetPaletteHR(palette);
}


//==========================================================================
//
// FadeToBlackHR - Fades the palette out to black
//
//==========================================================================

/*
void FadeToBlackHR(void)
{
	char work[16*3];
	char base[16*3];
	int i,j,steps=70;

	GetPaletteHR(base);
	for (i=0; i<steps; i++)
	{
		for (j=0; j<16*3; j++)
		{
			work[j] = base[j]-(base[j]*i/steps);
		}
		VB_SYNC;
		SetPaletteHR(work);
	}
	memset(work,0,16*3);
	SetPaletteHR(work);
}
*/

//==========================================================================
//
// BlackPaletteHR - Instantly blacks out the palette
//
//==========================================================================

void BlackPaletteHR(void)
{
	char blackpal[16*3];

	memset(blackpal,0,16*3);
	SetPaletteHR(blackpal);
}

//==========================================================================
//
//
// I_StartupReadKeys
//
//
//==========================================================================

void I_StartupReadKeys(void)
{
	int k;

   while (kbdtail < kbdhead)
   {
	   k = keyboardque[kbdtail&(KBDQUESIZE-1)];
	   kbdtail++;
	   if (k == 1)
		   I_Quit ();
   }
}
