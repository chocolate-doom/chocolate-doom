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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//



#include <stdio.h>

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_misc.h"
#include "m_random.h"
#include "w_wad.h"

#include "deh_main.h"
#include "deh_misc.h"
#include "doomdef.h"
#include "doomkeys.h"

#include "g_game.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"

#include "p_local.h"
#include "p_inter.h"

#include "am_map.h"
#include "m_cheat.h"

#include "s_sound.h"

// Needs access to LFB.
#include "v_video.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

#include "v_trans.h" // [crispy] colored cheat messages

extern int screenblocks; // [crispy] for the Crispy HUD

//
// STATUS BAR DATA
//


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS		1
#define STARTBONUSPALS		9
#define NUMREDPALS			8
#define NUMBONUSPALS		4
// Radiation suit, green shift.
#define RADIATIONPAL		13

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY		96

// For Responder
#define ST_TOGGLECHAT		KEY_ENTER

// Location of status bar
#define ST_X				0
#define ST_X2				104

#define ST_FX  			143
#define ST_FY  			169

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH		(tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES		5
#define ST_NUMSTRAIGHTFACES	3
#define ST_NUMTURNFACES		2
#define ST_NUMSPECIALFACES		3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES		2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET		(ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET		(ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET		(ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET		(ST_EVILGRINOFFSET + 1)
#define ST_GODFACE			(ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE			(ST_GODFACE+1)

#define ST_FACESX			143
#define ST_FACESY			168

#define ST_EVILGRINCOUNT		(2*TICRATE)
#define ST_STRAIGHTFACECOUNT	(TICRATE/2)
#define ST_TURNCOUNT		(1*TICRATE)
#define ST_OUCHCOUNT		(1*TICRATE)
#define ST_RAMPAGEDELAY		(2*TICRATE)

#define ST_MUCHPAIN			20


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH		3	
#define ST_AMMOX			44
#define ST_AMMOY			171

// HEALTH number pos.
#define ST_HEALTHWIDTH		3	
#define ST_HEALTHX			90
#define ST_HEALTHY			171

// Weapon pos.
#define ST_ARMSX			111
#define ST_ARMSY			172
#define ST_ARMSBGX			104
#define ST_ARMSBGY			168
#define ST_ARMSXSPACE		12
#define ST_ARMSYSPACE		10

// Frags pos.
#define ST_FRAGSX			138
#define ST_FRAGSY			171	
#define ST_FRAGSWIDTH		2

// ARMOR number pos.
#define ST_ARMORWIDTH		3
#define ST_ARMORX			221
#define ST_ARMORY			171

// Key icon positions.
#define ST_KEY0WIDTH		8
#define ST_KEY0HEIGHT		5
#define ST_KEY0X			239
#define ST_KEY0Y			171
#define ST_KEY1WIDTH		ST_KEY0WIDTH
#define ST_KEY1X			239
#define ST_KEY1Y			181
#define ST_KEY2WIDTH		ST_KEY0WIDTH
#define ST_KEY2X			239
#define ST_KEY2Y			191

// Ammunition counter.
#define ST_AMMO0WIDTH		3
#define ST_AMMO0HEIGHT		6
#define ST_AMMO0X			288
#define ST_AMMO0Y			173
#define ST_AMMO1WIDTH		ST_AMMO0WIDTH
#define ST_AMMO1X			288
#define ST_AMMO1Y			179
#define ST_AMMO2WIDTH		ST_AMMO0WIDTH
#define ST_AMMO2X			288
#define ST_AMMO2Y			191
#define ST_AMMO3WIDTH		ST_AMMO0WIDTH
#define ST_AMMO3X			288
#define ST_AMMO3Y			185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH		3
#define ST_MAXAMMO0HEIGHT		5
#define ST_MAXAMMO0X		314
#define ST_MAXAMMO0Y		173
#define ST_MAXAMMO1WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X		314
#define ST_MAXAMMO1Y		179
#define ST_MAXAMMO2WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X		314
#define ST_MAXAMMO2Y		191
#define ST_MAXAMMO3WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X		314
#define ST_MAXAMMO3Y		185

// pistol
#define ST_WEAPON0X			110 
#define ST_WEAPON0Y			172

// shotgun
#define ST_WEAPON1X			122 
#define ST_WEAPON1Y			172

// chain gun
#define ST_WEAPON2X			134 
#define ST_WEAPON2Y			172

// missile launcher
#define ST_WEAPON3X			110 
#define ST_WEAPON3Y			181

// plasma gun
#define ST_WEAPON4X			122 
#define ST_WEAPON4Y			181

 // bfg
#define ST_WEAPON5X			134
#define ST_WEAPON5Y			181

// WPNS title
#define ST_WPNSX			109 
#define ST_WPNSY			191

 // DETH title
#define ST_DETHX			109
#define ST_DETHY			191

//Incoming messages window location
//UNUSED
// #define ST_MSGTEXTX	   (viewwindowx)
// #define ST_MSGTEXTY	   (viewwindowy+viewheight-18)
#define ST_MSGTEXTX			0
#define ST_MSGTEXTY			0
// Dimensions given in characters.
#define ST_MSGWIDTH			52
// Or shall I say, in lines?
#define ST_MSGHEIGHT		1

#define ST_OUTTEXTX			0
#define ST_OUTTEXTY			6

// Width, in characters again.
#define ST_OUTWIDTH			52 
 // Height, in lines. 
#define ST_OUTHEIGHT		1

#define ST_MAPTITLEX \
    (ORIGWIDTH - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY		0
#define ST_MAPHEIGHT		1

// graphics are drawn to a backing screen and blitted to the real screen
byte                   *st_backing_screen;
	    
// main player in game
static player_t*	plyr; 

// ST_Start() has just been called
static boolean		st_firsttime;

// lump number for PLAYPAL
static int		lu_palette;

// used for timing
static unsigned int	st_clock;

// used for making messages go away
static int		st_msgcounter=0;

// used when in chat 
static st_chatstateenum_t	st_chatstate;

// whether in automap or first-person
static st_stateenum_t	st_gamestate;

// whether left-side main status bar is active
static boolean		st_statusbaron;

// whether status bar chat is active
static boolean		st_chat;

// value of st_chat before message popped up
static boolean		st_oldchat;

// whether chat window has the cursor on
static boolean		st_cursoron;

// !deathmatch
static boolean		st_notdeathmatch; 

// !deathmatch && st_statusbaron
static boolean		st_armson;

// !deathmatch
static boolean		st_fragson; 

// main bar left
static patch_t*		sbar;

// 0-9, tall numbers
static patch_t*		tallnum[10];

// tall % sign
static patch_t*		tallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t*		shortnum[10];

// 3 key-cards, 3 skulls
static patch_t*		keys[NUMCARDS]; 

// face status patches
static patch_t*		faces[ST_NUMFACES];

// face background
static patch_t*		faceback;

 // main bar right
static patch_t*		armsbg;

// weapon ownership patches
static patch_t*		arms[6][2]; 

// ready-weapon widget
static st_number_t	w_ready;

 // in deathmatch only, summary of frags stats
static st_number_t	w_frags;

// health widget
static st_percent_t	w_health;

// arms background
static st_binicon_t	w_armsbg; 


// weapon ownership widgets
static st_multicon_t	w_arms[6];

// face status widget
static st_multicon_t	w_faces; 

// keycard widgets
static st_multicon_t	w_keyboxes[3];

// armor widget
static st_percent_t	w_armor;

// ammo widgets
static st_number_t	w_ammo[4];

// max ammo widgets
static st_number_t	w_maxammo[4]; 



 // number of frags so far in deathmatch
static int	st_fragscount;

// used to use appopriately pained face
static int	st_oldhealth = -1;

// used for evil grin
static boolean	oldweaponsowned[NUMWEAPONS]; 

 // count until face changes
static int	st_facecount = 0;

// current face index, used by w_faces
static int	st_faceindex = 0;

// holds key-type for each key box on bar
static int	keyboxes[3]; 

// a random number per tick
static int	st_randomnumber;  

cheatseq_t cheat_mus = CHEAT("idmus", 2);
cheatseq_t cheat_god = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey = CHEAT("idfa", 0);
cheatseq_t cheat_noclip = CHEAT("idspispopd", 0);
cheatseq_t cheat_commercial_noclip = CHEAT("idclip", 0);

cheatseq_t	cheat_powerup[8] = // [crispy] idbehold0
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdi", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbeholdl", 0),
    CHEAT("idbehold", 0),
    CHEAT("idbehold0", 0), // [crispy] idbehold0
};

cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev = CHEAT("idclev", 2);
cheatseq_t cheat_mypos = CHEAT("idmypos", 0);

// [crispy] new cheats
cheatseq_t cheat_weapon = CHEAT("tntweap", 1);
cheatseq_t cheat_massacre = CHEAT("tntem", 0);
cheatseq_t cheat_hom = CHEAT("tnthom", 0);
cheatseq_t cheat_notarget = CHEAT("notarget", 0);
cheatseq_t cheat_spechits = CHEAT("spechits", 0);
cheatseq_t cheat_nomomentum = CHEAT("nomomentum", 0);
cheatseq_t cheat_showfps = CHEAT("showfps", 0);

//
// STATUS BAR CODE
//
void ST_Stop(void);

void ST_refreshBackground(void)
{

    if (screenblocks >= CRISPY_HUD && !automapactive)
        return;

    if (st_statusbaron)
    {
        V_UseBuffer(st_backing_screen);

	V_DrawPatch(ST_X, 0, sbar);

	if (netgame)
	    V_DrawPatch(ST_FX, 0, faceback);

        V_RestoreBuffer();

	V_CopyRect(ST_X, 0, st_backing_screen, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y);
    }

}

// [crispy] from boom202s/M_CHEAT.C:467-498
static int ST_cheat_massacre()    // jff 2/01/98 kill all monsters
{
  // jff 02/01/98 'em' cheat - kill all monsters
  // partially taken from Chi's .46 port
  //
  // killough 2/7/98: cleaned up code and changed to use dprintf;
  // fixed lost soul bug (LSs left behind when PEs are killed)

  int killcount=0;
  thinker_t *currentthinker=&thinkercap;
  extern void A_PainDie(mobj_t *);

  while ((currentthinker=currentthinker->next)!=&thinkercap)
    if (currentthinker->function.acp1 == (actionf_p1) P_MobjThinker &&
        (((mobj_t *) currentthinker)->flags & MF_COUNTKILL ||
         ((mobj_t *) currentthinker)->type == MT_SKULL))
      { // killough 3/6/98: kill even if PE is dead
        if (((mobj_t *) currentthinker)->health > 0)
          {
            killcount++;
            P_DamageMobj((mobj_t *)currentthinker, NULL, NULL, 10000);
          }
        if (((mobj_t *) currentthinker)->type == MT_PAIN)
          {
            A_PainDie((mobj_t *) currentthinker);    // killough 2/8/98
            P_SetMobjState ((mobj_t *) currentthinker, S_PAIN_DIE6);
          }
      }
  return (killcount);
}

// [crispy] trigger all special lines available on the map
static int ST_cheat_spechits()
{
    int i, speciallines = 0;
    boolean origcards[NUMCARDS];
    line_t dummy;

    // temporarily give all keys
    for (i = 0; i < NUMCARDS; i++)
    {
	origcards[i] = plyr->cards[i];
	plyr->cards[i] = true;
    }

    for (i = 0; i < numlines; i++)
    {
	if (lines[i].special)
	{
	    // do not trigger level exit switches/lines or teleporters
	    if (lines[i].special == 11 || lines[i].special == 51 ||
	        lines[i].special == 52 || lines[i].special == 124 ||
	        lines[i].special == 39 || lines[i].special == 97)
	        continue;

	    P_CrossSpecialLine (i, 0, plyr->mo);
	    P_ShootSpecialLine (plyr->mo, &lines[i]);
	    P_UseSpecialLine (plyr->mo, &lines[i], 0);

	    speciallines++;
	}
    }

    for (i = 0; i < NUMCARDS; i++)
    {
	plyr->cards[i] = origcards[i];
    }

    // [crispy] trigger tag 666/667 events
    dummy.tag = 666;
    if (gamemode == commercial)
    {
	if (gamemap == 7 ||
	// [crispy] Master Levels in PC slot 7
	(gamemission == pack_master && (gamemap == 14 || gamemap == 15 || gamemap == 16)))
	{
	    // Mancubi
	    speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);

	    // Arachnotrons
	    dummy.tag = 667;
	    speciallines += EV_DoFloor(&dummy, raiseToTexture);
	    dummy.tag = 666;
	}
    }
    else
    {
	if (gameepisode == 1)
	    // Barons of Hell
	    speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
	else
	if (gameepisode == 4)
	{
	     if (gamemap == 6)
		// Cyberdemons
		speciallines += EV_DoDoor(&dummy, vld_blazeOpen);
	    else
	    if (gamemap == 8)
		// Spider Masterminds
		speciallines += EV_DoFloor(&dummy, lowerFloorToLowest);
	}
    }
    // Keens (no matter which level they are on)
    // this call will be ignored if the tagged sector is already moving
    // so actions triggered in the condition above will have precedence
    speciallines += EV_DoDoor(&dummy, vld_open);

    return (speciallines);
}

// Respond to keyboard input events,
//  intercept cheats.
boolean
ST_Responder (event_t* ev)
{
  int		i;
    
  // Filter automap on/off.
  if (ev->type == ev_keyup
      && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
  {
    switch(ev->data1)
    {
      case AM_MSGENTERED:
	st_gamestate = AutomapState;
	st_firsttime = true;
	break;
	
      case AM_MSGEXITED:
	//	fprintf(stderr, "AM exited\n");
	st_gamestate = FirstPersonState;
	break;
    }
  }

  // if a user keypress...
  else if (ev->type == ev_keydown)
  {
    if (!netgame && gameskill != sk_nightmare)
    {
      // 'dqd' cheat for toggleable god mode
      if (cht_CheckCheat(&cheat_god, ev->data2))
      {
	// [crispy] dead players are first respawned at the current position
	mapthing_t mt = {0};
	if (plyr->playerstate == PST_DEAD)
	{
	    signed int an;
	    extern void P_SpawnPlayer (mapthing_t* mthing);

	    mt.x = plyr->mo->x >> FRACBITS;
	    mt.y = plyr->mo->y >> FRACBITS;
	    mt.angle = (plyr->mo->angle + ANG45/2)*(uint64_t)45/ANG45;
	    mt.type = consoleplayer + 1;
	    P_SpawnPlayer(&mt);

	    // [crispy] spawn a teleport fog
	    an = plyr->mo->angle >> ANGLETOFINESHIFT;
	    P_SpawnMobj(plyr->mo->x+20*finecosine[an], plyr->mo->y+20*finesine[an], plyr->mo->z, MT_TFOG);
	    S_StartSound(plyr, sfx_slop);
	}

	plyr->cheats ^= CF_GODMODE;
	if (plyr->cheats & CF_GODMODE)
	{
	  if (plyr->mo)
	    plyr->mo->health = 100;
	  
	  plyr->health = deh_god_mode_health;
	  plyr->message = DEH_String(STSTR_DQDON);
	}
	else 
	  plyr->message = DEH_String(STSTR_DQDOFF);

	// [crispy] eat key press when respawning
	if (mt.type)
	    return true;
      }
      // 'fa' cheat for killer fucking arsenal
      else if (cht_CheckCheat(&cheat_ammonokey, ev->data2))
      {
	plyr->armorpoints = deh_idfa_armor;
	plyr->armortype = deh_idfa_armor_class;
	
	// [crispy] give backpack
	if (!plyr->backpack)
	{
	    for (i=0 ; i<NUMAMMO ; i++)
		plyr->maxammo[i] *= 2;
	    plyr->backpack = true;
	}

	for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	plyr->message = DEH_String(STSTR_FAADDED);
      }
      // 'kfa' cheat for key full ammo
      else if (cht_CheckCheat(&cheat_ammo, ev->data2))
      {
	plyr->armorpoints = deh_idkfa_armor;
	plyr->armortype = deh_idkfa_armor_class;
	
	// [crispy] give backpack
	if (!plyr->backpack)
	{
	    for (i=0 ; i<NUMAMMO ; i++)
		plyr->maxammo[i] *= 2;
	    plyr->backpack = true;
	}

	for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	for (i=0;i<NUMCARDS;i++)
	  plyr->cards[i] = true;
	
	plyr->message = DEH_String(STSTR_KFAADDED);
      }
      // [crispy] implement Boom's "tntem" cheat
      else if (cht_CheckCheat(&cheat_massacre, ev->data2))
      {
	static char msg[32];
	int killcount = ST_cheat_massacre();

	M_snprintf(msg, sizeof(msg), "%s%d %sMonster%s Killed",
	           crstr[CR_GOLD],
	           killcount, crstr[CR_NONE], (killcount == 1) ? "" : "s");
	plyr->message = msg;
      }
      // [crispy] implement Crispy Doom's "spechits" cheat
      else if (cht_CheckCheat(&cheat_spechits, ev->data2))
      {
	static char msg[32];
	int triggeredlines = ST_cheat_spechits();

	M_snprintf(msg, sizeof(msg), "%s%d %sSpecial Line%s Triggered",
	           crstr[CR_GOLD],
	           triggeredlines, crstr[CR_NONE], (triggeredlines == 1) ? "" : "s");
	plyr->message = msg;
      }
      // [crispy] implement Boom's "tnthom" cheat
      else if (cht_CheckCheat(&cheat_hom, ev->data2))
      {
	static char msg[32];

	crispy_flashinghom = !crispy_flashinghom;

	M_snprintf(msg, sizeof(msg), "HOM Detection %s%s",
	           crstr[CR_GREEN],
	           (crispy_flashinghom) ? "ON" : "OFF");
	plyr->message = msg;
      }
      // 'mus' cheat for changing music
      else if (cht_CheckCheat(&cheat_mus, ev->data2))
      {
	
	char	buf[3];
	int		musnum;
	
	plyr->message = DEH_String(STSTR_MUS);
	cht_GetParam(&cheat_mus, buf);

        // Note: The original v1.9 had a bug that tried to play back
        // the Doom II music regardless of gamemode.  This was fixed
        // in the Ultimate Doom executable so that it would work for
        // the Doom 1 music as well.

	if (gamemode == commercial || gameversion < exe_ultimate)
	{
	  musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;
	  
	  // [crispy] prevent crash with IDMUS00
	  if (((buf[0]-'0')*10 + buf[1]-'0') > 35 || musnum < mus_runnin)
	    plyr->message = DEH_String(STSTR_NOMUS);
	  else
	    S_ChangeMusic(musnum, 1);
	}
	else
	{
	  musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');
	  
	  // [crispy] prevent crash with IDMUS0x or IDMUSx0
	  if (((buf[0]-'1')*9 + buf[1]-'1') > 31 || buf[0] < '1' || buf[1] < '1')
	    plyr->message = DEH_String(STSTR_NOMUS);
	  else
	    S_ChangeMusic(musnum, 1);
	}
      }
      // [crispy] allow both idspispopd and idclip cheats in all gamemissions
      else if ( ( /* logical_gamemission == doom
                 && */ cht_CheckCheat(&cheat_noclip, ev->data2))
             || ( /* logical_gamemission != doom
                 && */ cht_CheckCheat(&cheat_commercial_noclip,ev->data2)))
      {	
        // Noclip cheat.
        // For Doom 1, use the idspipsopd cheat; for all others, use
        // idclip

	plyr->cheats ^= CF_NOCLIP;
	
	if (plyr->cheats & CF_NOCLIP)
	  plyr->message = DEH_String(STSTR_NCON);
	else
	  plyr->message = DEH_String(STSTR_NCOFF);
      }
      // [crispy] implement PrBoom+'s "notarget" cheat
      else if (cht_CheckCheat(&cheat_notarget, ev->data2))
      {
	static char msg[32];

	plyr->cheats ^= CF_NOTARGET;

	M_snprintf(msg, sizeof(msg), "Notarget Mode %s%s",
	           crstr[CR_GREEN],
	           (plyr->cheats & CF_NOTARGET) ? "ON" : "OFF");
	plyr->message = msg;
      }
      // [crispy] implement "nomomentum" cheat, ne debug aid -- pretty useless, though
      else if (cht_CheckCheat(&cheat_nomomentum, ev->data2))
      {
	static char msg[32];

	plyr->cheats ^= CF_NOMOMENTUM;

	M_snprintf(msg, sizeof(msg), "Nomomentum Mode %s%s",
	           crstr[CR_GREEN],
	           (plyr->cheats & CF_NOMOMENTUM) ? "ON" : "OFF");
	plyr->message = msg;
      }
      // [crispy] implement Crispy Doom's "showfps" cheat, ne debug aid
      else if (cht_CheckCheat(&cheat_showfps, ev->data2))
      {
	crispy_showfps ^= 1;
      }
      // 'behold?' power-up cheats
      for (i=0;i<6;i++)
      {
	if (cht_CheckCheat(&cheat_powerup[i], ev->data2))
	{
	  if (!plyr->powers[i])
	    P_GivePower( plyr, i);
	  else if (i!=pw_strength)
	    plyr->powers[i] = 1;
	  else
	    plyr->powers[i] = 0;
	  
	  plyr->message = DEH_String(STSTR_BEHOLDX);
	}
      }
      // [crispy] idbehold0
      if (cht_CheckCheat(&cheat_powerup[7], ev->data2))
      {
	memset(plyr->powers, 0, sizeof(plyr->powers));
	plyr->message = DEH_String(STSTR_BEHOLDX);
      }
      
      // 'behold' power-up menu
      if (cht_CheckCheat(&cheat_powerup[6], ev->data2))
      {
	plyr->message = DEH_String(STSTR_BEHOLD);
      }
      // [crispy] implement Boom's "tntweap?" weapon cheats
      else if (cht_CheckCheat(&cheat_weapon, ev->data2))
      {
	char		buf[2];
	int		w;
	static char	msg[32];

	cht_GetParam(&cheat_weapon, buf);
	w = *buf - '1';

	if (w < 0 || w >= NUMWEAPONS)
	    return false;

	if (w == wp_supershotgun && !crispy_havessg)
	    return false;

	if ((w == wp_bfg || w == wp_plasma) && gamemode == shareware)
	    return false;

	// make '1' apply beserker strength toggle
	if (w == wp_fist)
	{
	    if (!plyr->powers[pw_strength])
		P_GivePower(plyr, pw_strength);
	    else
		plyr->powers[pw_strength] = 0;
	    M_snprintf(msg, sizeof(msg), DEH_String(STSTR_BEHOLDX));
	}
	else
	{
	    if ((plyr->weaponowned[w] = !plyr->weaponowned[w]))
		M_snprintf(msg, sizeof(msg), "Weapon %s%d%s Added",
		           crstr[CR_GOLD],
		           w + 1, crstr[CR_NONE]);
	    else
	    {
		M_snprintf(msg, sizeof(msg), "Weapon %s%d%s Removed",
		           crstr[CR_GOLD],
		           w + 1, crstr[CR_NONE]);

		// [crispy] removed current weapon, select another one
		if (w == plyr->readyweapon)
		{
		    extern boolean P_CheckAmmo (player_t* player);

		    P_CheckAmmo(plyr);
		}
	    }
	}
	plyr->message = msg;
      }
      // 'choppers' invulnerability & chainsaw
      else if (cht_CheckCheat(&cheat_choppers, ev->data2))
      {
	plyr->weaponowned[wp_chainsaw] = true;
	plyr->powers[pw_invulnerability] = true;
	plyr->message = DEH_String(STSTR_CHOPPERS);
      }
      // 'mypos' for player position
      else if (cht_CheckCheat(&cheat_mypos, ev->data2))
      {
        // [crispy] show (human readable) map coordinates
        // in the upper right corner (like automap stats)
/*
        static char buf[ST_MSGWIDTH];
        M_snprintf(buf, sizeof(buf), "ang=0x%x;x,y=(0x%x,0x%x)",
                   players[consoleplayer].mo->angle,
                   players[consoleplayer].mo->x,
                   players[consoleplayer].mo->y);
        plyr->message = buf;
*/
        plyr->mapcoords ^= 1;
      }
    }
    
    // 'clev' change-level cheat
    if (!netgame && cht_CheckCheat(&cheat_clev, ev->data2))
    {
      char		buf[3];
      int		epsd;
      int		map;
      
      cht_GetParam(&cheat_clev, buf);
      
      if (gamemission == pack_nerve)
      {
	epsd = 2;
	map = (buf[0] - '0')*10 + buf[1] - '0';
      }
      else
      if (gamemode == commercial)
      {
	epsd = 1;
	map = (buf[0] - '0')*10 + buf[1] - '0';
      }
      else
      {
	epsd = buf[0] - '0';
	map = buf[1] - '0';
      }

      // Chex.exe always warps to episode 1.

      if (gameversion == exe_chex)
      {
        epsd = 1;
      }

      // Catch invalid maps.
      if (epsd == 0) // [crispy] allow IDCLEV0x to work in Doom 1
	epsd = gameepisode;
      else
      if (epsd < 1)
	return false;

      if ((map == 0) && (buf[0] - '0' == 0)) // [crispy] IDCLEV00 restarts current map
	map = gamemap;
      else
      if ((map == 0) && crispy_havee1m10 && epsd == 1) // [crispy] support E1M10 "Sewers"
	map = 10;
      else
      if (map < 1)
	return false;

      // Ohmygod - this is not going to work.
      if ((gamemode == retail)
	  && ((epsd > 4) || (map > 9)))
      {
	if (!(crispy_havee1m10 && epsd == 1 && map == 10)) // [crispy] support E1M10 "Sewers"
	return false;
      }

      if ((gamemode == registered)
	  && ((epsd > 3) || (map > 9)))
	return false;

      if ((gamemode == shareware)
	  && ((epsd > 1) || (map > 9)))
	return false;

      // The source release has this check as map > 34. However, Vanilla
      // Doom allows IDCLEV up to MAP40 even though it normally crashes.
      if ((gamemode == commercial && gamemission != pack_nerve && gamemission != pack_master)
	&& (( epsd > 1) || (map > 40)))
	return false;

      if ((gamemission == pack_nerve)
	&& (( epsd > 2) || (map > 9)))
	return false;

      if ((gamemission == pack_master)
	&& (( epsd > 1) || (map > 21)))
	return false;

      // So be it.
      plyr->message = DEH_String(STSTR_CLEV);
      G_DeferedInitNew(gameskill, epsd, map);

      // [crispy] eat key press, i.e. don't change weapon upon level change
      return true;
    }
  }
  return false;
}



int ST_calcPainOffset(void)
{
    int		health;
    static int	lastcalc;
    static int	oldhealth = -1;
    
    health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {
	lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
	oldhealth = health;
    }
    return lastcalc;
}


//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget(void)
{
    int		i;
    angle_t	badguyangle;
    angle_t	diffang;
    static int	lastattackdown = -1;
    static int	priority = 0;
    boolean	doevilgrin;

    if (priority < 10)
    {
	// dead
	if (!plyr->health)
	{
	    priority = 9;
	    st_faceindex = ST_DEADFACE;
	    st_facecount = 1;
	}
    }

    if (priority < 9)
    {
	if (plyr->bonuscount)
	{
	    // picking up bonus
	    doevilgrin = false;

	    for (i=0;i<NUMWEAPONS;i++)
	    {
		if (oldweaponsowned[i] != plyr->weaponowned[i])
		{
		    doevilgrin = true;
		    oldweaponsowned[i] = plyr->weaponowned[i];
		}
	    }
	    if (doevilgrin) 
	    {
		// evil grin if just picked up weapon
		priority = 8;
		st_facecount = ST_EVILGRINCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
	    }
	}

    }
  
    if (priority < 8)
    {
	if (plyr->damagecount
	    && plyr->attacker
	    && plyr->attacker != plyr->mo)
	{
	    // being attacked
	    priority = 7;
	    
	    // [crispy] show "Ouch Face" as intended
	    if (st_oldhealth - plyr->health > ST_MUCHPAIN)
	    {
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
	    }
	    else
	    {
		badguyangle = R_PointToAngle2(plyr->mo->x,
					      plyr->mo->y,
					      plyr->attacker->x,
					      plyr->attacker->y);
		
		if (badguyangle > plyr->mo->angle)
		{
		    // whether right or left
		    diffang = badguyangle - plyr->mo->angle;
		    i = diffang > ANG180; 
		}
		else
		{
		    // whether left or right
		    diffang = plyr->mo->angle - badguyangle;
		    i = diffang <= ANG180; 
		} // confusing, aint it?

		
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset();
		
		if (diffang < ANG45)
		{
		    // head-on    
		    st_faceindex += ST_RAMPAGEOFFSET;
		}
		else if (i)
		{
		    // turn face right
		    st_faceindex += ST_TURNOFFSET;
		}
		else
		{
		    // turn face left
		    st_faceindex += ST_TURNOFFSET+1;
		}
	    }
	}
    }
  
    if (priority < 7)
    {
	// getting hurt because of your own damn stupidity
	if (plyr->damagecount)
	{
	    // [crispy] show "Ouch Face" as intended
	    if (st_oldhealth - plyr->health > ST_MUCHPAIN)
	    {
		priority = 7;
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
	    }
	    else
	    {
		priority = 6;
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
	    }

	}

    }
  
    if (priority < 6)
    {
	// rapid firing
	if (plyr->attackdown)
	{
	    if (lastattackdown==-1)
		lastattackdown = ST_RAMPAGEDELAY;
	    else if (!--lastattackdown)
	    {
		priority = 5;
		st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
		st_facecount = 1;
		lastattackdown = 1;
	    }
	}
	else
	    lastattackdown = -1;

    }
  
    if (priority < 5)
    {
	// invulnerability
	if ((plyr->cheats & CF_GODMODE)
	    || plyr->powers[pw_invulnerability])
	{
	    priority = 4;

	    st_faceindex = ST_GODFACE;
	    st_facecount = 1;

	}

    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
	st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
	st_facecount = ST_STRAIGHTFACECOUNT;
	priority = 0;
    }

    st_facecount--;

}

void ST_updateWidgets(void)
{
    static int	largeammo = 1994; // means "n/a"
    int		i;

    // must redirect the pointer if the ready weapon has changed.
    //  if (w_ready.data != plyr->readyweapon)
    //  {
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
	w_ready.num = &largeammo;
    else
	w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
    //{
    // static int tic=0;
    // static int dir=-1;
    // if (!(tic&15))
    //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
    // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
    //   dir = 1;
    // tic++;
    // }
    w_ready.data = plyr->readyweapon;

    // if (*w_ready.on)
    //  STlib_updateNum(&w_ready, true);
    // refresh weapon change
    //  }

    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
	keyboxes[i] = plyr->cards[i] ? i : -1;

	if (plyr->cards[i+3])
	    keyboxes[i] = i+3;
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !deathmatch;
    
    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch; 

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 
    st_fragscount = 0;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (i != consoleplayer)
	    st_fragscount += plyr->frags[i];
	else
	    st_fragscount -= plyr->frags[i];
    }

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
	st_chat = st_oldchat;

}

void ST_Ticker (void)
{

    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;

}

static int st_palette = 0;

void ST_doPaletteStuff(void)
{

    int		palette;
    byte*	pal;
    int		cnt;
    int		bzc;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
	// slowly fade the berzerk out
  	bzc = 12 - (plyr->powers[pw_strength]>>6);

	if (bzc > cnt)
	    cnt = bzc;
    }
	
    if (cnt)
    {
	palette = (cnt+7)>>3;
	
	if (palette >= NUMREDPALS)
	    palette = NUMREDPALS-1;

	// [crispy] tune down a bit so the menu remains legible
	if (menuactive || paused)
	    palette >>= 1;

	palette += STARTREDPALS;
    }

    else if (plyr->bonuscount)
    {
	palette = (plyr->bonuscount+7)>>3;

	if (palette >= NUMBONUSPALS)
	    palette = NUMBONUSPALS-1;

	palette += STARTBONUSPALS;
    }

    else if ( plyr->powers[pw_ironfeet] > 4*32
	      || plyr->powers[pw_ironfeet]&8)
	palette = RADIATIONPAL;
    else
	palette = 0;

    // In Chex Quest, the player never sees red.  Instead, the
    // radiation suit palette is used to tint the screen green,
    // as though the player is being covered in goo by an
    // attacking flemoid.

    if (gameversion == exe_chex
     && palette >= STARTREDPALS && palette < STARTREDPALS + NUMREDPALS)
    {
        palette = RADIATIONPAL;
    }

    if (palette != st_palette)
    {
	st_palette = palette;
	pal = (byte *) W_CacheLumpNum (lu_palette, PU_CACHE)+palette*768;
	I_SetPalette (pal);
    }

}

enum
{
    hudcolor_ammo,
    hudcolor_health,
    hudcolor_frags,
    hudcolor_armor
} hudcolor_t;

// [crispy] return ammo/health/armor widget color
static byte* ST_WidgetColor(int i)
{
    if (!crispy_coloredhud)
        return NULL;

    switch (i)
    {
        case hudcolor_ammo:
        {
            if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
            {
                return NULL;
            }
            else
            {
                int ammo =  plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
                int fullammo = maxammo[weaponinfo[plyr->readyweapon].ammo];
                int ammopct = 100 * ammo / fullammo;

                if (ammopct < 25)
                    return cr[CR_RED];
                else if (ammopct < 50)
                    return cr[CR_GOLD];
                else if (ammopct <= 100)
                    return cr[CR_GREEN];
                else
                    return cr[CR_BLUE];
            }
            break;
        }
        case hudcolor_health:
        {
            int health = plyr->health;

            // [crispy] Invulnerability powerup and God Mode cheat turn Health values gray
            if (plyr->cheats & CF_GODMODE ||
                plyr->powers[pw_invulnerability])
                return cr[CR_GRAY];
            else if (health < 25)
                return cr[CR_RED];
            else if (health < 50)
                return cr[CR_GOLD];
            else if (health <= 100)
                return cr[CR_GREEN];
            else
                return cr[CR_BLUE];

            break;
        }
        case hudcolor_frags:
        {
            int frags = st_fragscount;

            if (frags < 0)
                return cr[CR_RED];
            else if (frags == 0)
                return cr[CR_GOLD];
            else
                return cr[CR_GREEN];

            break;
        }
        case hudcolor_armor:
        {
	    // [crispy] Invulnerability powerup and God Mode cheat turn Armor values gray
	    if (plyr->cheats & CF_GODMODE ||
                plyr->powers[pw_invulnerability])
                return cr[CR_GRAY];
	    // [crispy] color by armor type
	    else if (plyr->armortype == 0)
                return cr[CR_RED];
	    else if (plyr->armortype == 1)
                return cr[CR_GREEN];
	    else if (plyr->armortype == 2)
                return cr[CR_BLUE];
/*
            // [crispy] alternatively, color by armor points
            int armor = plyr->armorpoints;

            if (armor < 25)
                return cr[CR_RED];
            else if (armor < 50)
                return cr[CR_GOLD];
            else if (armor <= 100)
                return cr[CR_GREEN];
            else
                return cr[CR_BLUE];
*/
            break;
        }
    }

    return NULL;
}

void ST_drawWidgets(boolean refresh)
{
    int		i;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch;

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 

    dp_translation = ST_WidgetColor(hudcolor_ammo);
    STlib_updateNum(&w_ready, refresh);
    V_ClearDPTranslation();

    // [crispy] draw berserk pack instead of no ammo if appropriate
    if (screenblocks >= CRISPY_HUD && !automapactive &&
        plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
    {
        V_DrawPatch(ST_AMMOX-23, ST_AMMOY+13, W_CacheLumpName("PSTRA0", PU_CACHE));
    }

    for (i=0;i<4;i++)
    {
	STlib_updateNum(&w_ammo[i], refresh);
	STlib_updateNum(&w_maxammo[i], refresh);
    }

    dp_translation = ST_WidgetColor(hudcolor_health);
    // [crispy] in the Crispy HUD, health blinks if below 10%
    if (screenblocks < CRISPY_HUD || automapactive || plyr->health > 9 || (gametic & TICRATE/2) > TICRATE/4)
    {
    STlib_updatePercent(&w_health, refresh || screenblocks >= CRISPY_HUD);
    }
    dp_translation = ST_WidgetColor(hudcolor_armor);
    STlib_updatePercent(&w_armor, refresh || screenblocks >= CRISPY_HUD);
    V_ClearDPTranslation();

    if (screenblocks < CRISPY_HUD || (automapactive && !crispy_automapoverlay))
    {
    STlib_updateBinIcon(&w_armsbg, refresh);
    }

    for (i=0;i<6;i++)
	STlib_updateMultIcon(&w_arms[i], refresh || screenblocks >= CRISPY_HUD);

    if (screenblocks < CRISPY_HUD || (automapactive && !crispy_automapoverlay))
    {
    STlib_updateMultIcon(&w_faces, refresh);
    }

    for (i=0;i<3;i++)
	STlib_updateMultIcon(&w_keyboxes[i], refresh || screenblocks >= CRISPY_HUD);

    dp_translation = ST_WidgetColor(hudcolor_frags);
    STlib_updateNum(&w_frags, refresh || screenblocks >= CRISPY_HUD);
    V_ClearDPTranslation();
}

void ST_doRefresh(void)
{

    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);

}

void ST_diffDraw(void)
{
    // update all widgets
    ST_drawWidgets(false);
}

void ST_Drawer (boolean fullscreen, boolean refresh)
{
  
    st_statusbaron = (!fullscreen) || (automapactive && !crispy_automapoverlay) || screenblocks >= CRISPY_HUD;
    st_firsttime = st_firsttime || refresh;

    if (crispy_cleanscreenshot && screenblocks >= CRISPY_HUD)
        return;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // [crispy] translucent HUD
    if (crispy_translucency && screenblocks > CRISPY_HUD && !(automapactive && !crispy_automapoverlay))
	dp_translucent = true;

    // If just after ST_Start(), refresh all
    if (st_firsttime) ST_doRefresh();
    // Otherwise, update as little as possible
    else ST_diffDraw();

    if (dp_translucent)
	dp_translucent = false;
}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable); 

// Iterates through all graphics to be loaded or unloaded, along with
// the variable they use, invoking the specified callback function.

static void ST_loadUnloadGraphics(load_callback_t callback)
{

    int		i;
    int		j;
    int		facenum;
    
    char	namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
	DEH_snprintf(namebuf, 9, "STTNUM%d", i);
        callback(namebuf, &tallnum[i]);

	DEH_snprintf(namebuf, 9, "STYSNUM%d", i);
        callback(namebuf, &shortnum[i]);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?

    callback(DEH_String("STTPRCNT"), &tallpercent);

    // key cards
    for (i=0;i<NUMCARDS;i++)
    {
	DEH_snprintf(namebuf, 9, "STKEYS%d", i);
        callback(namebuf, &keys[i]);
    }

    // arms background
    callback(DEH_String("STARMS"), &armsbg);

    // arms ownership widgets
    for (i=0; i<6; i++)
    {
	DEH_snprintf(namebuf, 9, "STGNUM%d", i+2);

	// gray #
        callback(namebuf, &arms[i][0]);

	// yellow #
	arms[i][1] = shortnum[i+2]; 
    }

    // face backgrounds for different color players
    DEH_snprintf(namebuf, 9, "STFB%d", consoleplayer);
    callback(namebuf, &faceback);

    // status bar background bits
    callback(DEH_String("STBAR"), &sbar);

    // face states
    facenum = 0;
    for (i=0; i<ST_NUMPAINFACES; i++)
    {
	for (j=0; j<ST_NUMSTRAIGHTFACES; j++)
	{
	    DEH_snprintf(namebuf, 9, "STFST%d%d", i, j);
            callback(namebuf, &faces[facenum]);
            ++facenum;
	}
	DEH_snprintf(namebuf, 9, "STFTR%d0", i);	// turn right
        callback(namebuf, &faces[facenum]);
        ++facenum;
	DEH_snprintf(namebuf, 9, "STFTL%d0", i);	// turn left
        callback(namebuf, &faces[facenum]);
        ++facenum;
	DEH_snprintf(namebuf, 9, "STFOUCH%d", i);	// ouch!
        callback(namebuf, &faces[facenum]);
        ++facenum;
	DEH_snprintf(namebuf, 9, "STFEVL%d", i);	// evil grin ;)
        callback(namebuf, &faces[facenum]);
        ++facenum;
	DEH_snprintf(namebuf, 9, "STFKILL%d", i);	// pissed off
        callback(namebuf, &faces[facenum]);
        ++facenum;
    }

    callback(DEH_String("STFGOD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFDEAD0"), &faces[facenum]);
    ++facenum;
}

static void ST_loadCallback(char *lumpname, patch_t **variable)
{
    *variable = W_CacheLumpName(lumpname, PU_STATIC);
}

void ST_loadGraphics(void)
{
    ST_loadUnloadGraphics(ST_loadCallback);
}

void ST_loadData(void)
{
    lu_palette = W_GetNumForName (DEH_String("PLAYPAL"));
    ST_loadGraphics();
}

static void ST_unloadCallback(char *lumpname, patch_t **variable)
{
    W_ReleaseLumpName(lumpname);
    *variable = NULL;
}

void ST_unloadGraphics(void)
{
    ST_loadUnloadGraphics(ST_unloadCallback);
}

void ST_unloadData(void)
{
    ST_unloadGraphics();
}

void ST_initData(void)
{

    int		i;

    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_clock = 0;
    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
	oldweaponsowned[i] = plyr->weaponowned[i];

    for (i=0;i<3;i++)
	keyboxes[i] = -1;

    STlib_init();

}



void ST_createWidgets(void)
{

    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready,
		  ST_AMMOX,
		  ST_AMMOY,
		  tallnum,
		  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
		  &st_statusbaron,
		  ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // health percentage
    STlib_initPercent(&w_health,
		      ST_HEALTHX,
		      ST_HEALTHY,
		      tallnum,
		      &plyr->health,
		      &st_statusbaron,
		      tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg,
		      ST_ARMSBGX,
		      ST_ARMSBGY,
		      armsbg,
		      &st_notdeathmatch,
		      &st_statusbaron);

    // weapons owned
    for(i=0;i<6;i++)
    {
        STlib_initMultIcon(&w_arms[i],
                           ST_ARMSX+(i%3)*ST_ARMSXSPACE,
                           ST_ARMSY+(i/3)*ST_ARMSYSPACE,
                           arms[i],
                           &plyr->weaponowned[i+1],
                           &st_armson);
    }

    // frags sum
    STlib_initNum(&w_frags,
		  ST_FRAGSX,
		  ST_FRAGSY,
		  tallnum,
		  &st_fragscount,
		  &st_fragson,
		  ST_FRAGSWIDTH);

    // faces
    STlib_initMultIcon(&w_faces,
		       ST_FACESX,
		       ST_FACESY,
		       faces,
		       &st_faceindex,
		       &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
		      ST_ARMORX,
		      ST_ARMORY,
		      tallnum,
		      &plyr->armorpoints,
		      &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0],
		       ST_KEY0X,
		       ST_KEY0Y,
		       keys,
		       &keyboxes[0],
		       &st_statusbaron);
    
    STlib_initMultIcon(&w_keyboxes[1],
		       ST_KEY1X,
		       ST_KEY1Y,
		       keys,
		       &keyboxes[1],
		       &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[2],
		       ST_KEY2X,
		       ST_KEY2Y,
		       keys,
		       &keyboxes[2],
		       &st_statusbaron);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0],
		  ST_AMMO0X,
		  ST_AMMO0Y,
		  shortnum,
		  &plyr->ammo[0],
		  &st_statusbaron,
		  ST_AMMO0WIDTH);

    STlib_initNum(&w_ammo[1],
		  ST_AMMO1X,
		  ST_AMMO1Y,
		  shortnum,
		  &plyr->ammo[1],
		  &st_statusbaron,
		  ST_AMMO1WIDTH);

    STlib_initNum(&w_ammo[2],
		  ST_AMMO2X,
		  ST_AMMO2Y,
		  shortnum,
		  &plyr->ammo[2],
		  &st_statusbaron,
		  ST_AMMO2WIDTH);
    
    STlib_initNum(&w_ammo[3],
		  ST_AMMO3X,
		  ST_AMMO3Y,
		  shortnum,
		  &plyr->ammo[3],
		  &st_statusbaron,
		  ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0],
		  ST_MAXAMMO0X,
		  ST_MAXAMMO0Y,
		  shortnum,
		  &plyr->maxammo[0],
		  &st_statusbaron,
		  ST_MAXAMMO0WIDTH);

    STlib_initNum(&w_maxammo[1],
		  ST_MAXAMMO1X,
		  ST_MAXAMMO1Y,
		  shortnum,
		  &plyr->maxammo[1],
		  &st_statusbaron,
		  ST_MAXAMMO1WIDTH);

    STlib_initNum(&w_maxammo[2],
		  ST_MAXAMMO2X,
		  ST_MAXAMMO2Y,
		  shortnum,
		  &plyr->maxammo[2],
		  &st_statusbaron,
		  ST_MAXAMMO2WIDTH);
    
    STlib_initNum(&w_maxammo[3],
		  ST_MAXAMMO3X,
		  ST_MAXAMMO3Y,
		  shortnum,
		  &plyr->maxammo[3],
		  &st_statusbaron,
		  ST_MAXAMMO3WIDTH);

}

static boolean	st_stopped = true;


void ST_Start (void)
{

    if (!st_stopped)
	ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;

}

void ST_Stop (void)
{
    if (st_stopped)
	return;

    I_SetPalette (W_CacheLumpNum (lu_palette, PU_CACHE));

    st_stopped = true;
}

void ST_Init (void)
{
    // [crispy] colorize the confusing 'behold' power-up menu
    if (!strcmp(STSTR_BEHOLD, DEH_String(STSTR_BEHOLD)))
    {
	char str_behold[80];
	M_snprintf(str_behold, sizeof(str_behold),
	           "in%sV%suln, %sS%str, %sI%snviso, %sR%sad, %sA%sllmap, or %sL%site-amp",
	           crstr[CR_GOLD], crstr[CR_NONE],
	           crstr[CR_GOLD], crstr[CR_NONE],
	           crstr[CR_GOLD], crstr[CR_NONE],
	           crstr[CR_GOLD], crstr[CR_NONE],
	           crstr[CR_GOLD], crstr[CR_NONE],
	           crstr[CR_GOLD], crstr[CR_NONE]);
	DEH_AddStringReplacement(STSTR_BEHOLD, str_behold);
    }

    ST_loadData();
    st_backing_screen = (byte *) Z_Malloc((ST_WIDTH << hires) * (ST_HEIGHT << hires), PU_STATIC, 0);
}

