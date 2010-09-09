// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// DESCRIPTION:
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------



#include <stdio.h>

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
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

//
// STATUS BAR DATA
//


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// Location of status bar
#define ST_X                    0

#define ST_FX                   143

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
// haleyjd 09/01/10: [STRIFE] Adjusted.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                311
#define ST_AMMOY                162

// HEALTH number pos.
// haleyjd 09/01/10: [STRIFE] Adjusted.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              79
#define ST_HEALTHY              162

// Weapon pos.
#define ST_ARMSX		111
#define ST_ARMSY		172
#define ST_ARMSBGX		104
#define ST_ARMSBGY		168
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

// Dimensions given in characters.
#define ST_MSGWIDTH			52

// haleyjd 08/31/10: [STRIFE] 
// * Removed faces.
// haleyjd 09/01/10:
// * Removed DOOM pre-beta cruft.
// * Removed deathmatch frags/arms-related stuff.
// * Removed arms panel stuff.
// * Removed unused widgets.
// * Removed more faces, keyboxes, st_randomnumber

// graphics are drawn to a backing screen and blitted to the real screen
byte                   *st_backing_screen;
	    
// main player in game
static player_t*        plyr; 

// ST_Start() has just been called
static boolean          st_firsttime;

// lump number for PLAYPAL
static int              lu_palette;

// whether in automap or first-person
static st_stateenum_t   st_gamestate;

// whether left-side main status bar is active
static boolean          st_statusbaron;

// haleyjd 09/01/10: [STRIFE]
// Whether or not a popup is currently displayed
static boolean          st_displaypopup;

// haleyjd 09/01/10: [STRIFE] sbar -> invback
// main inventory background and other bits
static patch_t*         invback;  // main bar
static patch_t*         invtop;   // top bit
static patch_t*         invpop;   // popup frame with text
static patch_t*         invpop2;  // plain popup frame
static patch_t*         invpbak;  // popup background w/details
static patch_t*         invpbak2; // plain popup background

// haleyjd 09/01/10: [STRIFE] Replaced tallnum, shortnum w/inv fonts
// 0-9, green numbers
static patch_t*         invfontg[10];

// 0-9, yellow numbers
static patch_t*         invfonty[10];

// 3 key-cards, 3 skulls -- STRIFE-TODO: This is handled differently
static patch_t*         keys[NUMCARDS]; 

// ready-weapon widget
static st_number_t      w_ready; // haleyjd [STRIFE]: This is still used.

// haleyjd: [STRIFE] This is still used but was changed to a st_number_t.
// health widget
static st_number_t      w_health;

// ammo widgets
static st_number_t      w_ammo[NUMAMMO];     // haleyjd [STRIFE]: Still used.

// max ammo widgets
static st_number_t      w_maxammo[NUMAMMO];  // haleyjd [STRIFE]: Still used.

// number of frags so far in deathmatch
static int              st_fragscount;


cheatseq_t cheat_mus = CHEAT("idmus", 2);
cheatseq_t cheat_god = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey = CHEAT("idfa", 0);
cheatseq_t cheat_noclip = CHEAT("idspispopd", 0);
cheatseq_t cheat_commercial_noclip = CHEAT("idclip", 0);

cheatseq_t	cheat_powerup[7] =
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdi", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbeholdl", 0),
    CHEAT("idbehold", 0),
};

cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev = CHEAT("idclev", 2);
cheatseq_t cheat_mypos = CHEAT("idmypos", 0);


//
// STATUS BAR CODE
//
void ST_Stop(void);

void ST_refreshBackground(void)
{
    if (st_statusbaron)
    {
        V_UseBuffer(st_backing_screen);

        V_DrawPatch(ST_X, 0, invback);

        // haleyjd 09/01/10: STRIFE-TODO: status bar stuff
        /*
        if (netgame)
            V_DrawPatch(ST_FX, 0, faceback);
        */
        V_RestoreBuffer();

        V_CopyRect(ST_X, 0, st_backing_screen, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y);
    }
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
      }
      // 'fa' cheat for killer fucking arsenal
      else if (cht_CheckCheat(&cheat_ammonokey, ev->data2))
      {
	plyr->armorpoints = deh_idfa_armor;
	plyr->armortype = deh_idfa_armor_class;
	
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
	
	for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	for (i=0;i<NUMCARDS;i++)
	  plyr->cards[i] = true;
	
	plyr->message = DEH_String(STSTR_KFAADDED);
      }
      // 'mus' cheat for changing music
      else if (cht_CheckCheat(&cheat_mus, ev->data2))
      {
	
	char	buf[3];
	int		musnum;
	
	plyr->message = DEH_String(STSTR_MUS);
	cht_GetParam(&cheat_mus, buf);
	
	if (gamemode == commercial)
	{
	  musnum = mus_logo + (buf[0]-'0')*10 + buf[1]-'0' - 1; // villsa [STRIFE] TODO - fix music
	  
	  if (((buf[0]-'0')*10 + buf[1]-'0') > 35)
	    plyr->message = DEH_String(STSTR_NOMUS);
	  else
	    S_ChangeMusic(musnum, 1);
	}
	else
	{
	  musnum = mus_logo + (buf[0]-'1')*9 + (buf[1]-'1');    // villsa [STRIFE] TODO - fix music
	  
	  if (((buf[0]-'1')*9 + buf[1]-'1') > 31)
	    plyr->message = DEH_String(STSTR_NOMUS);
	  else
	    S_ChangeMusic(musnum, 1);
	}
      }
      else if ( (gamemission == doom 
                 && cht_CheckCheat(&cheat_noclip, ev->data2))
             || (gamemission != doom 
                 && cht_CheckCheat(&cheat_commercial_noclip,ev->data2)))
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
      
      // 'behold' power-up menu
      if (cht_CheckCheat(&cheat_powerup[6], ev->data2))
      {
	plyr->message = DEH_String(STSTR_BEHOLD);
      }
      // 'choppers' invulnerability & chainsaw
      else if (cht_CheckCheat(&cheat_choppers, ev->data2))
      {
          // villsa [STRIFE] unused
	//plyr->weaponowned[wp_chainsaw] = true;
	plyr->powers[pw_invulnerability] = true;
	plyr->message = DEH_String(STSTR_CHOPPERS);
      }
      // 'mypos' for player position
      else if (cht_CheckCheat(&cheat_mypos, ev->data2))
      {
	static char	buf[ST_MSGWIDTH];
	sprintf(buf, "ang=0x%x;x,y=(0x%x,0x%x)",
		players[consoleplayer].mo->angle,
		players[consoleplayer].mo->x,
		players[consoleplayer].mo->y);
	plyr->message = buf;
      }
    }
    
    // 'clev' change-level cheat
    if (!netgame && cht_CheckCheat(&cheat_clev, ev->data2))
    {
      char		buf[3];
      int		epsd;
      int		map;
      
      cht_GetParam(&cheat_clev, buf);
      
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

      // haleyjd 09/01/10: Removed Chex Quest stuff.

      // Catch invalid maps.
      if (epsd < 1)
	return false;

      if (map < 1)
	return false;

      // Ohmygod - this is not going to work.
      if ((gamemode == retail)
	  && ((epsd > 4) || (map > 9)))
	return false;

      if ((gamemode == registered)
	  && ((epsd > 3) || (map > 9)))
	return false;

      if ((gamemode == shareware)
	  && ((epsd > 1) || (map > 9)))
	return false;

      if ((gamemode == commercial)
	&& (( epsd > 1) || (map > 34)))
	return false;

      // So be it.
      plyr->message = DEH_String(STSTR_CLEV);
      G_DeferedInitNew(gameskill, epsd, map);
    }    
  }
  return false;
}


/*
int ST_calcPainOffset(void)
{
    // haleyjd 08/31/10: [STRIFE] Removed.
}
*/

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
/*
void ST_updateFaceWidget(void)
{
    // haleyjd 08/31/10: [STRIFE] Removed.
}
*/

/*
void ST_updateWidgets(void)
{
    // haleyjd 09/01/10: [STRIFE] Rogue merged this into ST_Ticker below.
}
*/

//
// ST_Ticker
//
// haleyjd 09/01/10: [STRIFE]
// * Removed st_clock and st_randomnumber.
// * Merged ST_updateWidgets here. Wasn't inlined, as doesn't exist separately 
//   in the binary as inlined functions normally do.
//
void ST_Ticker (void)
{
    static int  largeammo = 1994; // means "n/a"
    int         i;

    // must redirect the pointer if the ready weapon has changed.
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];

    w_ready.data = plyr->readyweapon;

    // STRIFE-TODO: Gobbledeegunk.
    /*
    v2 = dword_88490-- == 1; // no clue yet...
    if(v2)
        dword_DC7F4 = dword_DC7F0;
    v1 = st_popupdisplaytics;
    if(st_popupdisplaytics)
    {
        --st_popupdisplaytics;
        if(v1 == 1)
        {
            st_displaypopup = false;
            st_showkeys = false;
            dword_88484 = -1;     // unknown var
            if(st_dosizedisplay)
                M_SizeDisplay();  // mondo hack?
            st_dosizedisplay = false;
        }
    }
    */

    // haleyjd 09/01/10: [STRIFE] Keys are handled on a popup
    // haleyjd 08/31/10: [STRIFE] No face widget
    // haleyjd 09/01/10: [STRIFE] Armor, weapons, frags, etc. handled elsewhere

    // haleyjd: This is from the PRE-BETA! Left here because it amuses me ;)
    // get rid of chat window if up because of message
    //if (!--st_msgcounter)
    //    st_chat = st_oldchat;
}

static int st_palette = 0;

//
// ST_doPaletteStuff
//
// haleyjd 08/31/10: [STRIFE]
// * Changed radsuit palette handling for Strife nukagecount.
// * All other logic verified to be unmodified.
//
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

        palette += STARTREDPALS;
    }

    else if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;

        if (palette >= NUMBONUSPALS)
            palette = NUMBONUSPALS-1;

        palette += STARTBONUSPALS;
    }
    // haleyjd 08/31/10: [STRIFE] Flash green when in nukage, not when has
    // an environment suit (a breathing sound is played to indicate that
    // instead).
    else if ( plyr->nukagecount > 16*TICRATE || 
              (plyr->nukagecount & 8))
        palette = RADIATIONPAL;
    else
        palette = 0;

    // haleyjd 08/31/10: Removed Chex Quest

    if (palette != st_palette)
    {
        st_palette = palette;
        pal = (byte *) W_CacheLumpNum (lu_palette, PU_CACHE)+palette*768;
        I_SetPalette (pal);
    }

}

/* 
void ST_drawWidgets(boolean refresh)
{
    haleyjd 09/01/10: [STRIFE] Removed
}
*/

void ST_doRefresh(void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();
}

void ST_diffDraw(void)
{
    // haleyjd: STRIFE-TODO: Needed?
}

void ST_Drawer (boolean fullscreen, boolean refresh)
{
    st_statusbaron = (!fullscreen) || automapactive;
    st_firsttime = st_firsttime || refresh;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // haleyjd 09/01/10: STRIFE-TODO: work out statbar details

    // If just after ST_Start(), refresh all
    if (st_firsttime) ST_doRefresh();
    // Otherwise, update as little as possible
    else ST_diffDraw();
}

//
// ST_DrawExternal
//
// haleyjd 09/01/10: [STRIFE] New function.
// * Draws external portions of the status bar such the top bar and popups.
//
boolean ST_DrawExternal(void)
{
    if (st_statusbaron)
    {
        V_DrawPatchDirect(0, 160, invtop);
        STlib_drawNumPositive(&w_health);
        STlib_drawNumPositive(&w_ready);
    }
    else
    {
        // STRIFE-TODO:
        // ST_drawNumFontY2(15, 194, plyr->health);
        // v5 = weaponinfo[plyr->readyweapon].ammo;
        // if (v5 != am_noammo)
        //     ST_drawNumFontY2(310, 194, plyr->ammo[v5]);
    }

    if(!st_displaypopup)
        return false;

    // STRIFE-TODO: Shitloads more stuff:
    // * showobjective shit
    // * keys/frags popup
    // * weapons/ammo/stats popup
    // * etc etc etc

    return true;
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

    // haleyjd 09/01/10: [STRIFE]
    // Load the numbers, green and yellow
    for (i=0;i<10;i++)
    {
        DEH_snprintf(namebuf, 9, "INVFONG%d", i);
        callback(namebuf, &invfontg[i]);

        DEH_snprintf(namebuf, 9, "INVFONY%d", i);
        callback(namebuf, &invfonty[i]);
    }

    // haleyjd 08/31/10: [STRIFE] 
    // * No face - STRIFE-TODO: but there are similar color patches, which appear behind the armor in deathmatch...
    // 09/01/10:
    // * Removed all unused DOOM stuff (arms, numbers, %, etc).

    // haleyjd 09/01/10: [STRIFE]: stbar -> invback, added new patches
    // status bar background bits
    callback(DEH_String("INVBACK"),  &invback);
    callback(DEH_String("INVTOP"),   &invtop);
    callback(DEH_String("INVPOP"),   &invpop);
    callback(DEH_String("INVPOP2"),  &invpop2);
    callback(DEH_String("INVPBAK"),  &invpbak);
    callback(DEH_String("INVPBAK2"), &invpbak2);
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

//
// ST_initData
//
// haleyjd 09/01/10: [STRIFE]
// * Removed prebeta cruft, face stuff, keyboxes, and oldwe
//
void ST_initData(void)
{
    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_gamestate = FirstPersonState;

    st_statusbaron = true;

    st_palette = -1;

    STlib_init();
}



void ST_createWidgets(void)
{
    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready,
                  ST_AMMOX,
                  ST_AMMOY,
                  invfontg,
                  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                  ST_AMMOWIDTH);

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // health percentage
    STlib_initNum(&w_health,
                  ST_HEALTHX,
                  ST_HEALTHY,
                  invfontg,
                  &plyr->health,
                  ST_HEALTHWIDTH);

    // haleyjd 08/31/10: [STRIFE] 
    // * No face.
    // 09/01/10:
    // * No arms, weaponsowned, frags, armor, keyboxes

    // haleyjd 09/01/10: STRIFE-TODO: Ammo Widgets!!!
    /*
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
  */
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
    ST_loadData();
    st_backing_screen = (byte *) Z_Malloc(ST_WIDTH * ST_HEIGHT, PU_STATIC, 0);
}

