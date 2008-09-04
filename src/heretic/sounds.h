
// sounds.h

#ifndef __SOUNDSH__
#define __SOUNDSH__

#define MAX_SND_DIST 	1600
#define MAX_CHANNELS	16

// Music identifiers

typedef enum
{
	mus_e1m1,
	mus_e1m2,
	mus_e1m3,
	mus_e1m4,
	mus_e1m5,
	mus_e1m6,
	mus_e1m7,
	mus_e1m8,
	mus_e1m9,

	mus_e2m1,
	mus_e2m2,
	mus_e2m3,
	mus_e2m4,
	mus_e2m5,
	mus_e2m6,
	mus_e2m7,
	mus_e2m8,
	mus_e2m9,

	mus_e3m1,
	mus_e3m2,
	mus_e3m3,
	mus_e3m4,
	mus_e3m5,
	mus_e3m6,
	mus_e3m7,
	mus_e3m8,
	mus_e3m9,

	mus_e4m1,
	mus_e4m2,
	mus_e4m3,
	mus_e4m4,
	mus_e4m5,
	mus_e4m6,
	mus_e4m7,
	mus_e4m8,
	mus_e4m9,

	mus_e5m1,
	mus_e5m2,
	mus_e5m3,
	mus_e5m4,
	mus_e5m5,
	mus_e5m6,
	mus_e5m7,
	mus_e5m8,
	mus_e5m9,

	mus_e6m1,
	mus_e6m2,
	mus_e6m3,

	mus_titl,
	mus_intr,
	mus_cptd,
	NUMMUSIC
} musicenum_t;

typedef struct
{
	char name[8];
	int p1;
} musicinfo_t;

typedef struct sfxinfo_s
{
	char name[8];
	struct sfxinfo_s *link; // Make alias for another sound
	unsigned short priority; // Higher priority takes precendence
	int usefulness; // Determines when a sound should be cached out
	void *snd_ptr;
	int lumpnum;
	int numchannels; // total number of channels a sound type may occupy
} sfxinfo_t;

typedef struct
{
	mobj_t *mo;
	long sound_id;
	long handle;
	long pitch;
	int priority;
} channel_t;

typedef struct
{
	long id;
	unsigned short priority;
	char *name;
	mobj_t *mo;
	int distance;
} ChanInfo_t;

typedef	struct
{
	int channelCount;
	int musicVolume;
	int soundVolume;
	ChanInfo_t chan[8];
} SoundInfo_t;

// Sound identifiers

typedef enum
{
	sfx_None,
	sfx_gldhit,
	sfx_gntful,
	sfx_gnthit,
	sfx_gntpow,
	sfx_gntact,
	sfx_gntuse,
	sfx_phosht,
	sfx_phohit,
	sfx_phopow,
	sfx_lobsht,
	sfx_lobhit,
	sfx_lobpow,
	sfx_hrnsht,
	sfx_hrnhit,
	sfx_hrnpow,
	sfx_ramphit,
	sfx_ramrain,
	sfx_bowsht,
	sfx_stfhit,
	sfx_stfpow,
	sfx_stfcrk,
	sfx_impsit,
	sfx_impat1,
	sfx_impat2,
	sfx_impdth,
	sfx_impact,
	sfx_imppai,
	sfx_mumsit,
	sfx_mumat1,
	sfx_mumat2,
	sfx_mumdth,
	sfx_mumact,
	sfx_mumpai,
	sfx_mumhed,
	sfx_bstsit,
	sfx_bstatk,
	sfx_bstdth,
	sfx_bstact,
	sfx_bstpai,
	sfx_clksit,
	sfx_clkatk,
	sfx_clkdth,
	sfx_clkact,
	sfx_clkpai,
	sfx_snksit,
	sfx_snkatk,
	sfx_snkdth,
	sfx_snkact,
	sfx_snkpai,
	sfx_kgtsit,
	sfx_kgtatk,
	sfx_kgtat2,
	sfx_kgtdth,
	sfx_kgtact,
	sfx_kgtpai,
	sfx_wizsit,
	sfx_wizatk,
	sfx_wizdth,
	sfx_wizact,
	sfx_wizpai,
	sfx_minsit,
	sfx_minat1,
	sfx_minat2,
	sfx_minat3,
	sfx_mindth,
	sfx_minact,
	sfx_minpai,
	sfx_hedsit,
	sfx_hedat1,
	sfx_hedat2,
	sfx_hedat3,
	sfx_heddth,
	sfx_hedact,
	sfx_hedpai,
	sfx_sorzap,
	sfx_sorrise,
	sfx_sorsit,
	sfx_soratk,
	sfx_soract,
	sfx_sorpai,
	sfx_sordsph,
	sfx_sordexp,
	sfx_sordbon,
	sfx_sbtsit,
	sfx_sbtatk,
	sfx_sbtdth,
	sfx_sbtact,
	sfx_sbtpai,
	sfx_plroof,
	sfx_plrpai,
	sfx_plrdth,		// Normal
	sfx_gibdth,		// Extreme
	sfx_plrwdth,	// Wimpy
	sfx_plrcdth,	// Crazy
	sfx_itemup,
	sfx_wpnup,
	sfx_telept,
	sfx_doropn,
	sfx_dorcls,
	sfx_dormov,
	sfx_artiup,
	sfx_switch,
	sfx_pstart,
	sfx_pstop,
	sfx_stnmov,
	sfx_chicpai,
	sfx_chicatk,
	sfx_chicdth,
	sfx_chicact,
	sfx_chicpk1,
	sfx_chicpk2,
	sfx_chicpk3,
	sfx_keyup,
	sfx_ripslop,
	sfx_newpod,
	sfx_podexp,
	sfx_bounce,
	sfx_volsht,
	sfx_volhit,
	sfx_burn,
	sfx_splash,
	sfx_gloop,
	sfx_respawn,
	sfx_blssht,
	sfx_blshit,
	sfx_chat,
	sfx_artiuse,
	sfx_gfrag,
	sfx_waterfl,

	// Monophonic sounds

	sfx_wind,
	sfx_amb1,
	sfx_amb2,
	sfx_amb3,
	sfx_amb4,
	sfx_amb5,
	sfx_amb6,
	sfx_amb7,
	sfx_amb8,
	sfx_amb9,
	sfx_amb10,
	sfx_amb11,
	NUMSFX
} sfxenum_t;

#endif
