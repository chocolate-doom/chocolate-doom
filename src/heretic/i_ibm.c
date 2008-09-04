
// I_IBM.C

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <graph.h>
#include "DoomDef.h"
#include "R_local.h"
#include "sounds.h"
#include "i_sound.h"
#include "dmx.h"

// Macros

#define DPMI_INT 0x31
//#define NOKBD
//#define NOTIMER

// Public Data

int DisplayTicker = 0;

// Code

void main(int argc, char **argv)
{
	myargc = argc;
	myargv = argv;
	D_DoomMain();
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
void I_InitDiskFlash (void);

extern  int     usemouse, usejoystick;

extern void **lumpcache;

/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

static channel_t channel[MAX_CHANNELS];

static int rs; //the current registered song.
int mus_song = -1;
int mus_lumpnum;
void *mus_sndptr;
byte *soundCurve;

extern sfxinfo_t S_sfx[];
extern musicinfo_t S_music[];

extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;
extern int snd_MaxVolume;
extern int snd_MusicVolume;
extern int snd_Channels;

extern int startepisode;
extern int startmap;

int AmbChan;

void S_Start(void)
{
	int i;

	S_StartSong((gameepisode-1)*9 + gamemap-1, true);

	//stop all sounds
	for(i=0; i < snd_Channels; i++)
	{
		if(channel[i].handle)
		{
			S_StopSound(channel[i].mo);
		}
	}
	memset(channel, 0, 8*sizeof(channel_t));
}

void S_StartSong(int song, boolean loop)
{
	if(song == mus_song)
	{ // don't replay an old song
		return;
	}
	if(rs)
	{
		I_StopSong(rs);
		I_UnRegisterSong(rs);
		Z_ChangeTag(lumpcache[mus_lumpnum], PU_CACHE);
		#ifdef __WATCOMC__
			_dpmi_unlockregion(mus_sndptr, lumpinfo[mus_lumpnum].size);
		#endif
	}
	if(song < mus_e1m1 || song > NUMMUSIC)
	{
		return;
	}
	mus_lumpnum = W_GetNumForName(S_music[song].name);
	mus_sndptr = W_CacheLumpNum(mus_lumpnum, PU_MUSIC);
	#ifdef __WATCOMC__
		_dpmi_lockregion(mus_sndptr, lumpinfo[mus_lumpnum].size);
	#endif
	rs = I_RegisterSong(mus_sndptr);
	I_PlaySong(rs, loop); //'true' denotes endless looping.
	mus_song = song;
}

void S_StartSound(mobj_t *origin, int sound_id)
{
	int dist, vol;
	int i;
	int sound;
	int priority;
	int sep;
	int angle;
	int absx;
	int absy;

	static int sndcount = 0;
	int chan;

	if(sound_id==0 || snd_MaxVolume == 0)
		return;
	if(origin == NULL)
	{
		origin = players[consoleplayer].mo;
	}

// calculate the distance before other stuff so that we can throw out
// sounds that are beyond the hearing range.
	absx = abs(origin->x-players[consoleplayer].mo->x);
	absy = abs(origin->y-players[consoleplayer].mo->y);
	dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
	dist >>= FRACBITS;
//  dist = P_AproxDistance(origin->x-viewx, origin->y-viewy)>>FRACBITS;

	if(dist >= MAX_SND_DIST)
	{
//      dist = MAX_SND_DIST - 1;
	  return; //sound is beyond the hearing range...
	}
	if(dist < 0)
	{
		dist = 0;
	}
	priority = S_sfx[sound_id].priority;
	priority *= (10 - (dist/160));
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
		if(origin == channel[i].mo)
		{ // only allow other mobjs one sound
			S_StopSound(channel[i].mo);
			break;
		}
	}
	if(i >= snd_Channels)
	{
		if(sound_id >= sfx_wind)
		{
			if(AmbChan != -1 && S_sfx[sound_id].priority <=
				S_sfx[channel[AmbChan].sound_id].priority)
			{
				return; //ambient channel already in use
			}
			else
			{
				AmbChan = -1;
			}
		}
		for(i=0; i<snd_Channels; i++)
		{
			if(channel[i].mo == NULL)
			{
				break;
			}
		}
		if(i >= snd_Channels)
		{
			//look for a lower priority sound to replace.
			sndcount++;
			if(sndcount >= snd_Channels)
			{
				sndcount = 0;
			}
			for(chan=0; chan < snd_Channels; chan++)
			{
				i = (sndcount+chan)%snd_Channels;
				if(priority >= channel[i].priority)
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
				if(channel[i].handle)
				{
					if(I_SoundIsPlaying(channel[i].handle))
					{
						I_StopSound(channel[i].handle);
					}
					if(S_sfx[channel[i].sound_id].usefulness > 0)
					{
						S_sfx[channel[i].sound_id].usefulness--;
					}

					if(AmbChan == i)
					{
						AmbChan = -1;
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
		S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
			PU_SOUND);
		#ifdef __WATCOMC__
		_dpmi_lockregion(S_sfx[sound_id].snd_ptr,
			lumpinfo[S_sfx[sound_id].lumpnum].size);
		#endif
	}

	// calculate the volume based upon the distance from the sound origin.
//      vol = (snd_MaxVolume*16 + dist*(-snd_MaxVolume*16)/MAX_SND_DIST)>>9;
	vol = soundCurve[dist];

	if(origin == players[consoleplayer].mo)
	{
		sep = 128;
	}
	else
	{
		angle = R_PointToAngle2(players[consoleplayer].mo->x,
			players[consoleplayer].mo->y, channel[i].mo->x, channel[i].mo->y);
		angle = (angle-viewangle)>>24;
		sep = angle*2-128;
		if(sep < 64)
			sep = -sep;
		if(sep > 192)
			sep = 512-sep;
	}

	channel[i].pitch = (byte)(127+(M_Random()&7)-(M_Random()&7));
	channel[i].handle = I_StartSound(sound_id, S_sfx[sound_id].snd_ptr, vol, sep, channel[i].pitch, 0);
	channel[i].mo = origin;
	channel[i].sound_id = sound_id;
	channel[i].priority = priority;
	if(sound_id >= sfx_wind)
	{
		AmbChan = i;
	}
	if(S_sfx[sound_id].usefulness == -1)
	{
		S_sfx[sound_id].usefulness = 1;
	}
	else
	{
		S_sfx[sound_id].usefulness++;
	}
}

void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume)
{
	int dist;
	int i;
	int sep;

	static int sndcount;
	int chan;

	if(sound_id == 0 || snd_MaxVolume == 0)
		return;
	if(origin == NULL)
	{
		origin = players[consoleplayer].mo;
	}

	if(volume == 0)
	{
		return;
	}
	volume = (volume*(snd_MaxVolume+1)*8)>>7;

// no priority checking, as ambient sounds would be the LOWEST.
	for(i=0; i<snd_Channels; i++)
	{
		if(channel[i].mo == NULL)
		{
			break;
		}
	}
	if(i >= snd_Channels)
	{
		return;
	}
	if(S_sfx[sound_id].lumpnum == 0)
	{
		S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
	}
	if(S_sfx[sound_id].snd_ptr == NULL)
	{
		S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
			PU_SOUND);
		#ifdef __WATCOMC__
		_dpmi_lockregion(S_sfx[sound_id].snd_ptr,
			lumpinfo[S_sfx[sound_id].lumpnum].size);
		#endif
	}
	channel[i].pitch = (byte)(127-(M_Random()&3)+(M_Random()&3));
	channel[i].handle = I_StartSound(sound_id, S_sfx[sound_id].snd_ptr, volume, 128, channel[i].pitch, 0);
	channel[i].mo = origin;
	channel[i].sound_id = sound_id;
	channel[i].priority = 1; //super low priority.
	if(S_sfx[sound_id].usefulness == -1)
	{
		S_sfx[sound_id].usefulness = 1;
	}
	else
	{
		S_sfx[sound_id].usefulness++;
	}
}

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
		if(channel[i].sound_id == sound_id && channel[i].mo)
		{
			found++; //found one.  Now, should we replace it??
			if(priority >= channel[i].priority)
			{ // if we're gonna kill one, then this'll be it
				lp = i;
				priority = channel[i].priority;
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
	if(channel[lp].handle)
	{
		if(I_SoundIsPlaying(channel[lp].handle))
		{
			I_StopSound(channel[lp].handle);
		}
		if(S_sfx[channel[i].sound_id].usefulness > 0)
		{
			S_sfx[channel[i].sound_id].usefulness--;
		}
		channel[lp].mo = NULL;
	}
	return(true);
}

void S_StopSound(mobj_t *origin)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(channel[i].mo == origin)
		{
			I_StopSound(channel[i].handle);
			if(S_sfx[channel[i].sound_id].usefulness > 0)
			{
				S_sfx[channel[i].sound_id].usefulness--;
			}
			channel[i].handle = 0;
			channel[i].mo = NULL;
			if(AmbChan == i)
			{
				AmbChan = -1;
			}
		}
	}
}

void S_SoundLink(mobj_t *oldactor, mobj_t *newactor)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(channel[i].mo == oldactor)
			channel[i].mo = newactor;
	}
}

void S_PauseSound(void)
{
	I_PauseSong(rs);
}

void S_ResumeSound(void)
{
	I_ResumeSong(rs);
}

static int nextcleanup;

void S_UpdateSounds(mobj_t *listener)
{
	int i, dist, vol;
	int angle;
	int sep;
	int priority;
	int absx;
	int absy;

	listener = players[consoleplayer].mo;
	if(snd_MaxVolume == 0)
	{
		return;
	}
	if(nextcleanup < gametic)
	{
		for(i=0; i < NUMSFX; i++)
		{
			if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
			{
				if(lumpcache[S_sfx[i].lumpnum])
				{
					if(((memblock_t *)((byte *)(lumpcache[S_sfx[i].lumpnum])-
						sizeof(memblock_t)))->id == 0x1d4a11)
					{ // taken directly from the Z_ChangeTag macro
						Z_ChangeTag2(lumpcache[S_sfx[i].lumpnum], PU_CACHE);
						#ifdef __WATCOMC__
							_dpmi_unlockregion(S_sfx[i].snd_ptr, lumpinfo[S_sfx[i].lumpnum].size);
						#endif
					}
				}
				S_sfx[i].usefulness = -1;
				S_sfx[i].snd_ptr = NULL;
			}
		}
		nextcleanup = gametic+35; //CLEANUP DEBUG cleans every second
	}
	for(i=0;i<snd_Channels;i++)
	{
		if(!channel[i].handle || S_sfx[channel[i].sound_id].usefulness == -1)
		{
			continue;
		}
		if(!I_SoundIsPlaying(channel[i].handle))
		{
			if(S_sfx[channel[i].sound_id].usefulness > 0)
			{
				S_sfx[channel[i].sound_id].usefulness--;
			}
			channel[i].handle = 0;
			channel[i].mo = NULL;
			channel[i].sound_id = 0;
			if(AmbChan == i)
			{
				AmbChan = -1;
			}
		}
		if(channel[i].mo == NULL || channel[i].sound_id == 0
			|| channel[i].mo == players[consoleplayer].mo)
		{
			continue;
		}
		else
		{
			absx = abs(channel[i].mo->x-players[consoleplayer].mo->x);
			absy = abs(channel[i].mo->y-players[consoleplayer].mo->y);
			dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
			dist >>= FRACBITS;
//          dist = P_AproxDistance(channel[i].mo->x-listener->x, channel[i].mo->y-listener->y)>>FRACBITS;

			if(dist >= MAX_SND_DIST)
			{
				S_StopSound(channel[i].mo);
				continue;
			}
			if(dist < 0)
				dist = 0;

// calculate the volume based upon the distance from the sound origin.
//          vol = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE)+dist)*(snd_MaxVolume*8))>>7;
			vol = soundCurve[dist];

			angle = R_PointToAngle2(players[consoleplayer].mo->x,
				players[consoleplayer].mo->y, channel[i].mo->x, channel[i].mo->y);
			angle = (angle-viewangle)>>24;
			sep = angle*2-128;
			if(sep < 64)
				sep = -sep;
			if(sep > 192)
				sep = 512-sep;
			I_UpdateSoundParams(channel[i].handle, vol, sep, channel[i].pitch);
			priority = S_sfx[channel[i].sound_id].priority;
			priority *= (10 - (dist>>8));
			channel[i].priority = priority;
		}
	}
}

void S_Init(void)
{
	soundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);
	I_StartupSound();
	if(snd_Channels > 8)
	{
		snd_Channels = 8;
	}
	I_SetChannels(snd_Channels);
	I_SetMusicVolume(snd_MusicVolume);
	S_SetMaxVolume(true);
}

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
		c->id = channel[i].sound_id;
		c->priority = channel[i].priority;
		c->name = S_sfx[c->id].name;
		c->mo = channel[i].mo;
		c->distance = P_AproxDistance(c->mo->x-viewx, c->mo->y-viewy)
			>>FRACBITS;
	}
}

void S_SetMaxVolume(boolean fullprocess)
{
	int i;

	if(!fullprocess)
	{
		soundCurve[0] = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE))*(snd_MaxVolume*8))>>7;
	}
	else
	{
		for(i = 0; i < MAX_SND_DIST; i++)
		{
			soundCurve[i] = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE)+i)*(snd_MaxVolume*8))>>7;
		}
	}
}

static boolean musicPaused;
void S_SetMusicVolume(void)
{
	I_SetMusicVolume(snd_MusicVolume);
	if(snd_MusicVolume == 0)
	{
		I_PauseSong(rs);
		musicPaused = true;
	}
	else if(musicPaused)
	{
		musicPaused = false;
		I_ResumeSong(rs);
	}
}

void S_ShutDown(void)
{
	extern int tsm_ID;
	if(tsm_ID != -1)
	{
  		I_StopSong(rs);
  		I_UnRegisterSong(rs);
  		I_ShutdownSound();
	}
}

/*
=============================================================================

							CONSTANTS

=============================================================================
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

boolean grmode;

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

//--------------------------------------------------------------------------
//
// PROC I_UnColorBorder
//
//--------------------------------------------------------------------------

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
	int i;
	int old;
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
	grmode = true;
	regs.w.ax = 0x13;
	int386(0x10, (const union REGS *)&regs, &regs);
	pcscreen = destscreen = (byte *)0xa0000;
	I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
	I_InitDiskFlash();
}

//--------------------------------------------------------------------------
//
// PROC I_ShutdownGraphics
//
//--------------------------------------------------------------------------

void I_ShutdownGraphics(void)
{

	if(*(byte *)0x449 == 0x13) // don't reset mode if it didn't get set
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

void I_ReadScreen(byte *scr)
{
	memcpy(scr, screen, SCREENWIDTH*SCREENHEIGHT);
}


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
			D_PostEvent (&ev);
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
		D_PostEvent (&ev);
	}

}


void   I_ReadKeys (void)
{
	int             k;
	event_t ev;


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
	I_ReadExternDriver();
}

/*
============================================================================

					TIMER INTERRUPT

============================================================================
*/

void I_ColorBlack (int r, int g, int b)
{
_outbyte (PEL_WRITE_ADR,0);
_outbyte(PEL_DATA,r);
_outbyte(PEL_DATA,g);
_outbyte(PEL_DATA,b);
}


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
   int  (far *function)();

   //
   // General mouse detection
   //
	mousepresent = 0;
	if ( M_CheckParm ("-nomouse") || !usemouse )
		return;

	if (I_ResetMouse () != 0xffff)
	{
		tprintf ("Mouse: not present ",0);
		return;
	}
	tprintf ("Mouse: detected ",0);

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

	D_PostEvent (&ev);
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
	int     buttons;
	int     count;
	int     centerx, centery;

	joystickpresent = 0;
	if ( M_CheckParm ("-nojoy") || !usejoystick )
		return;

	if (!I_ReadJoystick ())
	{
		joystickpresent = false;
		tprintf ("joystick not found ",0);
		return;
	}
	printf("joystick found\n");
	joystickpresent = true;

	printf("CENTER the joystick and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	centerx = joystickx;
	centery = joysticky;

	printf("\nPush the joystick to the UPPER LEFT corner and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	joyxl = (centerx + joystickx)/2;
	joyyl = (centerx + joysticky)/2;

	printf("\nPush the joystick to the LOWER RIGHT corner and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	joyxh = (centerx + joystickx)/2;
	joyyh = (centery + joysticky)/2;
	printf("\n");
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

	D_PostEvent (&ev);
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
	extern char __begtext;
	extern char ___argc;
	int     n,d;

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


void IO_ColorBlack (int r, int g, int b)
{
_outbyte (PEL_WRITE_ADR,0);
_outbyte(PEL_DATA,r);
_outbyte(PEL_DATA,g);
_outbyte(PEL_DATA,b);
}


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

void IO_StartupTimer (void)
{
	oldtimerisr = _dos_getvect(TIMERINT);

	_dos_setvect (0x8000 | TIMERINT, IO_TimerISR);
	IO_SetTimer0 (VBLCOUNTER);
}

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
	tprintf("I_StartupDPMI",1);
	I_StartupDPMI();
	tprintf("I_StartupMouse ",1);
	I_StartupMouse();
//	tprintf("I_StartupJoystick ",1);
//	I_StartupJoystick();
//	tprintf("I_StartupKeyboard ",1);
//	I_StartupKeyboard();
	tprintf("S_Init... ",1);
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
	byte *scr;
	char *lumpName;
	int r;

	D_QuitNetGame();
	M_SaveDefaults();
	scr = (byte *)W_CacheLumpName("ENDTEXT", PU_CACHE);
	I_Shutdown();
	memcpy((void *)0xb8000, scr, 80*25*2);
	regs.w.ax = 0x0200;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int386(0x10, (const union REGS *)&regs, &regs); // Set text pos
	_settextposition(24, 1);
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
	int             meminfo[32];
	int             heap;
	int             i;
	int                             block;
	byte                    *ptr;

	memset (meminfo,0,sizeof(meminfo));
	segread(&segregs);
	segregs.es = segregs.ds;
	regs.w.ax = 0x500;      // get memory info
	regs.x.edi = (int)&meminfo;
	int386x( 0x31, &regs, &regs, &segregs );

	heap = meminfo[0];
	printf ("DPMI memory: 0x%x, ",heap);

	do
	{
		heap -= 0x10000;                // leave 64k alone
		if (heap > 0x800000)
			heap = 0x800000;
		ptr = malloc (heap);
	} while (!ptr);

	printf ("0x%x allocated for zone\n", heap);
	if (heap < 0x180000)
		I_Error ("Insufficient DPMI memory!");
#if 0
	regs.w.ax = 0x501;      // allocate linear block
	regs.w.bx = heap>>16;
	regs.w.cx = heap&0xffff;
	int386( 0x31, &regs, &regs);
	if (regs.w.cflag)
		I_Error ("Couldn't allocate DPMI memory!");

	block = (regs.w.si << 16) + regs.w.di;
#endif

	*size = heap;
	return ptr;
}

/*
=============================================================================

					DISK ICON FLASHING

=============================================================================
*/

void I_InitDiskFlash (void)
{
#if 0
	void    *pic;
	byte    *temp;

	pic = W_CacheLumpName ("STDISK",PU_CACHE);
	temp = destscreen;
	destscreen = (byte *)0xac000;
	V_DrawPatchDirect (SCREENWIDTH-16,SCREENHEIGHT-16,0,pic);
	destscreen = temp;
#endif
}

// draw disk icon
void I_BeginRead (void)
{
#if 0
	byte    *src,*dest;
	int             y;

	if (!grmode)
		return;

// write through all planes
	outp (SC_INDEX,SC_MAPMASK);
	outp (SC_INDEX+1,15);
// set write mode 1
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)|1);

// copy to backup
	src = currentscreen + 184*80 + 304/4;
	dest = (byte *)0xac000 + 184*80 + 288/4;
	for (y=0 ; y<16 ; y++)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		src += 80;
		dest += 80;
	}

// copy disk over
	dest = currentscreen + 184*80 + 304/4;
	src = (byte *)0xac000 + 184*80 + 304/4;
	for (y=0 ; y<16 ; y++)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		src += 80;
		dest += 80;
	}


// set write mode 0
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)&~1);
#endif
}

// erase disk icon
void I_EndRead (void)
{
#if 0
	byte    *src,*dest;
	int             y;

	if (!grmode)
		return;

// write through all planes
	outp (SC_INDEX,SC_MAPMASK);
	outp (SC_INDEX+1,15);
// set write mode 1
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)|1);


// copy disk over
	dest = currentscreen + 184*80 + 304/4;
	src = (byte *)0xac000 + 184*80 + 288/4;
	for (y=0 ; y<16 ; y++)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		src += 80;
		dest += 80;
	}

// set write mode 0
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)&~1);
#endif
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

int i_Vector;
externdata_t *i_ExternData;
boolean useexterndriver;

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
//
// I_ReadExternDriver
//
//		calls the external interrupt, which should then update i_ExternDriver
//=========================================================================

void I_ReadExternDriver(void)
{
	event_t ev;

	if(useexterndriver)
	{
		DPMIInt(i_Vector);
	}
}
