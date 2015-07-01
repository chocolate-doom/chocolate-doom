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
// DESCRIPTION:  Heads-up displays
//


#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"

#include "z_zone.h"

#include "deh_main.h"
#include "i_swap.h"
#include "i_video.h"

#include "hu_stuff.h"
#include "hu_lib.h"
#include "m_controls.h"
#include "m_misc.h"
#include "w_wad.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

#include "v_video.h" // [crispy] V_ClearDPTranslation()
#include "v_trans.h" // [crispy] colored kills/items/secret/etc. messages

//
// Locally used constants, shortcuts.
//
#define HU_TITLE	(mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2	(mapnames_commercial[gamemap-1])
#define HU_TITLEP	(mapnames_commercial[gamemap-1 + 32])
#define HU_TITLET	(mapnames_commercial[gamemap-1 + 64])
#define HU_TITLEN	(mapnames_commercial[gamemap-1 + 96])
#define HU_TITLEM	(mapnames_commercial[gamemap-1 + 105])
#define HU_TITLE_CHEX   (mapnames[gamemap - 1])
#define HU_TITLEHEIGHT	1
#define HU_TITLEX	0
#define HU_TITLEY	(167 - SHORT(hu_font[0]->height))

#define HU_INPUTTOGGLE	't'
#define HU_INPUTX	HU_MSGX
#define HU_INPUTY	(HU_MSGY + HU_MSGHEIGHT*(SHORT(hu_font[0]->height) +1))
#define HU_INPUTWIDTH	64
#define HU_INPUTHEIGHT	1

#define HU_COORDX	(320 - 8 * hu_font['A'-HU_FONTSTART]->width)


char *chat_macros[10] =
{
    HUSTR_CHATMACRO0,
    HUSTR_CHATMACRO1,
    HUSTR_CHATMACRO2,
    HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4,
    HUSTR_CHATMACRO5,
    HUSTR_CHATMACRO6,
    HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8,
    HUSTR_CHATMACRO9
};

char*	player_names[] =
{
    HUSTR_PLRGREEN,
    HUSTR_PLRINDIGO,
    HUSTR_PLRBROWN,
    HUSTR_PLRRED
};

char			chat_char; // remove later.
static player_t*	plr;
patch_t*		hu_font[HU_FONTSIZE];
static hu_textline_t	w_title;
static hu_textline_t	w_map;
static hu_textline_t	w_kills;
static hu_textline_t	w_items;
static hu_textline_t	w_scrts;
static hu_textline_t	w_ltime;
static hu_textline_t	w_coordx;
static hu_textline_t	w_coordy;
static hu_textline_t	w_coorda;
static hu_textline_t	w_fps;
boolean			chat_on;
static hu_itext_t	w_chat;
static boolean		always_off = false;
static char		chat_dest[MAXPLAYERS];
static hu_itext_t w_inputbuffer[MAXPLAYERS];

static boolean		message_on;
boolean			message_dontfuckwithme;
static boolean		message_nottobefuckedwith;
static boolean		secret_on;

static hu_stext_t	w_message;
static int		message_counter;
static hu_stext_t	w_secret;
static int		secret_counter;

static boolean		coord_on;
static int		coord_counter;

extern int		showMessages;

static boolean		headsupactive = false;

extern int		screenblocks; // [crispy]

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//

char*	mapnames[] =	// DOOM shareware/registered/retail (Ultimate) names.
{

    HUSTR_E1M1,
    HUSTR_E1M2,
    HUSTR_E1M3,
    HUSTR_E1M4,
    HUSTR_E1M5,
    HUSTR_E1M6,
    HUSTR_E1M7,
    HUSTR_E1M8,
    HUSTR_E1M9,

    HUSTR_E2M1,
    HUSTR_E2M2,
    HUSTR_E2M3,
    HUSTR_E2M4,
    HUSTR_E2M5,
    HUSTR_E2M6,
    HUSTR_E2M7,
    HUSTR_E2M8,
    HUSTR_E2M9,

    HUSTR_E3M1,
    HUSTR_E3M2,
    HUSTR_E3M3,
    HUSTR_E3M4,
    HUSTR_E3M5,
    HUSTR_E3M6,
    HUSTR_E3M7,
    HUSTR_E3M8,
    HUSTR_E3M9,

    HUSTR_E4M1,
    HUSTR_E4M2,
    HUSTR_E4M3,
    HUSTR_E4M4,
    HUSTR_E4M5,
    HUSTR_E4M6,
    HUSTR_E4M7,
    HUSTR_E4M8,
    HUSTR_E4M9,

    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL",
    "NEWLEVEL"
};

// List of names for levels in commercial IWADs
// (doom2.wad, plutonia.wad, tnt.wad).  These are stored in a
// single large array; WADs like pl2.wad have a MAP33, and rely on
// the layout in the Vanilla executable, where it is possible to
// overflow the end of one array into the next.

char *mapnames_commercial[] =
{
    // DOOM 2 map names.

    HUSTR_1,
    HUSTR_2,
    HUSTR_3,
    HUSTR_4,
    HUSTR_5,
    HUSTR_6,
    HUSTR_7,
    HUSTR_8,
    HUSTR_9,
    HUSTR_10,
    HUSTR_11,
	
    HUSTR_12,
    HUSTR_13,
    HUSTR_14,
    HUSTR_15,
    HUSTR_16,
    HUSTR_17,
    HUSTR_18,
    HUSTR_19,
    HUSTR_20,
	
    HUSTR_21,
    HUSTR_22,
    HUSTR_23,
    HUSTR_24,
    HUSTR_25,
    HUSTR_26,
    HUSTR_27,
    HUSTR_28,
    HUSTR_29,
    HUSTR_30,
    HUSTR_31,
    HUSTR_32,

    // Plutonia WAD map names.

    PHUSTR_1,
    PHUSTR_2,
    PHUSTR_3,
    PHUSTR_4,
    PHUSTR_5,
    PHUSTR_6,
    PHUSTR_7,
    PHUSTR_8,
    PHUSTR_9,
    PHUSTR_10,
    PHUSTR_11,
	
    PHUSTR_12,
    PHUSTR_13,
    PHUSTR_14,
    PHUSTR_15,
    PHUSTR_16,
    PHUSTR_17,
    PHUSTR_18,
    PHUSTR_19,
    PHUSTR_20,
	
    PHUSTR_21,
    PHUSTR_22,
    PHUSTR_23,
    PHUSTR_24,
    PHUSTR_25,
    PHUSTR_26,
    PHUSTR_27,
    PHUSTR_28,
    PHUSTR_29,
    PHUSTR_30,
    PHUSTR_31,
    PHUSTR_32,
    
    // TNT WAD map names.

    THUSTR_1,
    THUSTR_2,
    THUSTR_3,
    THUSTR_4,
    THUSTR_5,
    THUSTR_6,
    THUSTR_7,
    THUSTR_8,
    THUSTR_9,
    THUSTR_10,
    THUSTR_11,
	
    THUSTR_12,
    THUSTR_13,
    THUSTR_14,
    THUSTR_15,
    THUSTR_16,
    THUSTR_17,
    THUSTR_18,
    THUSTR_19,
    THUSTR_20,
	
    THUSTR_21,
    THUSTR_22,
    THUSTR_23,
    THUSTR_24,
    THUSTR_25,
    THUSTR_26,
    THUSTR_27,
    THUSTR_28,
    THUSTR_29,
    THUSTR_30,
    THUSTR_31,
    THUSTR_32,

    NHUSTR_1,
    NHUSTR_2,
    NHUSTR_3,
    NHUSTR_4,
    NHUSTR_5,
    NHUSTR_6,
    NHUSTR_7,
    NHUSTR_8,
    NHUSTR_9,

    MHUSTR_1,
    MHUSTR_2,
    MHUSTR_3,
    MHUSTR_4,
    MHUSTR_5,
    MHUSTR_6,
    MHUSTR_7,
    MHUSTR_8,
    MHUSTR_9,
    MHUSTR_10,
    MHUSTR_11,
    MHUSTR_12,
    MHUSTR_13,
    MHUSTR_14,
    MHUSTR_15,
    MHUSTR_16,
    MHUSTR_17,
    MHUSTR_18,
    MHUSTR_19,
    MHUSTR_20,
    MHUSTR_21
};

void HU_Init(void)
{

    int		i;
    int		j;
    char	buffer[9];

    // load the heads-up font
    j = HU_FONTSTART;
    for (i=0;i<HU_FONTSIZE;i++)
    {
	DEH_snprintf(buffer, 9, "STCFN%.3d", j++);
	hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

}

void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{

    int		i;
    char*	s;

    if (headsupactive)
	HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    secret_on = false;
    chat_on = false;
    coord_on = false;

    // create the message widget
    HUlib_initSText(&w_message,
		    HU_MSGX, HU_MSGY, HU_MSGHEIGHT,
		    hu_font,
		    HU_FONTSTART, &message_on);

    // [crispy] create the secret message widget
    HUlib_initSText(&w_secret,
		    88, 86, HU_MSGHEIGHT,
		    hu_font,
		    HU_FONTSTART, &secret_on);

    // create the map title widget
    HUlib_initTextLine(&w_title,
		       HU_TITLEX, HU_TITLEY,
		       hu_font,
		       HU_FONTSTART);

    // [crispy] create the generic map title, kills, items, secrets and level time widgets
    HUlib_initTextLine(&w_map,
		       HU_TITLEX, HU_TITLEY - SHORT(hu_font[0]->height + 1),
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_kills,
		       HU_TITLEX, HU_MSGY + 1 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_items,
		       HU_TITLEX, HU_MSGY + 2 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_scrts,
		       HU_TITLEX, HU_MSGY + 3 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_ltime,
		       HU_TITLEX, HU_MSGY + 5 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coordx,
		       HU_COORDX, HU_MSGY + 1 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coordy,
		       HU_COORDX, HU_MSGY + 2 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_coorda,
		       HU_COORDX, HU_MSGY + 3 * 8,
		       hu_font,
		       HU_FONTSTART);

    HUlib_initTextLine(&w_fps,
		       HU_COORDX, HU_MSGY,
		       hu_font,
		       HU_FONTSTART);

    
    switch ( logical_gamemission )
    {
      case doom:
	s = HU_TITLE;
	break;
      case doom2:
	 s = HU_TITLE2;
	 break;
      case pack_plut:
	s = HU_TITLEP;
	break;
      case pack_tnt:
	s = HU_TITLET;
	break;
      case pack_nerve:
	if (gamemap <= 9)
	  s = HU_TITLEN;
	else
	  s = HU_TITLE2;
	break;
      case pack_master:
	if (gamemap <= 21)
	  s = HU_TITLEM;
	else
	  s = HU_TITLE2;
	break;
      default:
         s = "Unknown level";
         break;
    }

    // Chex.exe always uses the episode 1 level title
    // eg. E2M1 gives the title for E1M1

    if (gameversion == exe_chex)
    {
        s = HU_TITLE_CHEX;
    }

    // [crispy] special-casing for E1M10 "Sewers" support
    if (crispy_havee1m10 && gameepisode == 1 && gamemap == 10)
    {
	s = HUSTR_E1M10;
    }

    // [crispy] explicitely display (episode and) map if the
    // map is from a PWAD or if the map title string has been dehacked
    {
	char map[6], *wad;
	extern char *iwadfile;

	if (gamemode == commercial)
	    M_snprintf(map, sizeof(map), "map%02d", gamemap);
	else
	    M_snprintf(map, sizeof(map), "e%dm%d", gameepisode, gamemap);

	wad = lumpinfo[W_GetNumForName(map)].wad_file->path;

	if (strcmp(s, DEH_String(s)) || (strcmp(wad, M_BaseName(iwadfile)) && !nervewadfile))
	{
	    char *m;

	    m = M_StringJoin(wad, ": ", crstr[CR_GRAY], map, NULL);
	    wad = m; // [crispy] free() that, else *m leaks memory

	    while (*m)
		HUlib_addCharToTextLine(&w_map, *(m++));

	    free(wad);
	}
    }

    // dehacked substitution to get modified level name

    s = DEH_String(s);
    
    while (*s)
	HUlib_addCharToTextLine(&w_title, *(s++));

    // create the chat widget
    HUlib_initIText(&w_chat,
		    HU_INPUTX, HU_INPUTY,
		    hu_font,
		    HU_FONTSTART, &chat_on);

    // create the inputbuffer widgets
    for (i=0 ; i<MAXPLAYERS ; i++)
	HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

    headsupactive = true;

}

// [crispy] print a bar indicating demo progress at the bottom of the screen
static void HU_DemoProgressBar (void)
{
    int i;
    extern char *demo_p, *demobuffer;
    extern int defdemosize;

    i = SCREENWIDTH * (demo_p - demobuffer) / defdemosize;

    V_DrawHorizLine(0, SCREENHEIGHT - 3, i, 4); // [crispy] white
    V_DrawHorizLine(0, SCREENHEIGHT - 2, i, 0); // [crispy] black
    V_DrawHorizLine(0, SCREENHEIGHT - 1, i, 4); // [crispy] white

    V_DrawHorizLine(0, SCREENHEIGHT - 2, 1, 4); // [crispy] white start
    V_DrawHorizLine(i - 1, SCREENHEIGHT - 2, 1, 4); // [crispy] white end
}

// [crispy] static, non-projected crosshair
static void HU_DrawCrosshair (void)
{
    static int		lump;
    static patch_t*	patch;

    if (plr->readyweapon == wp_fist ||
        plr->readyweapon == wp_chainsaw ||
        plr->playerstate > PST_LIVE ||
        automapactive ||
        menuactive ||
        paused ||
        secret_on)
	return;

    if (!lump)
    {
	lump = W_GetNumForName(CRISPY_CROSSHAIR);
	patch = W_CacheLumpNum(lump, PU_CACHE);
    }

    if (crispy_translucency)
	dp_translucent = true;

    V_DrawPatch(160-SHORT(patch->width/2), (screenblocks <= 10) ? 84 : 100, patch);
}

void HU_Drawer(void)
{

    static char str[32], *s;

    if (crispy_cleanscreenshot)
    {
	HU_Erase();
	return;
    }

    // [crispy] translucent messages for translucent HUD
    if (crispy_translucency && screenblocks > CRISPY_HUD && (!automapactive || (automapactive && crispy_automapoverlay)))
	dp_translucent = true;

    V_ClearDPTranslation();
    HUlib_drawSText(&w_message);
    HUlib_drawIText(&w_chat);

    if (crispy_coloredhud)
	dp_translation = cr[CR_GOLD];
    HUlib_drawSText(&w_secret);

    if (automapactive)
    {
	// [crispy] move map title to the bottom
	if (crispy_automapoverlay && screenblocks >= CRISPY_HUD - 1)
	    w_title.y = HU_TITLEY + 32; // [crispy] 32 == ST_HEIGHT
	else
	    w_title.y = HU_TITLEY;

	HUlib_drawTextLine(&w_title, false);
    }

    if (automapactive && crispy_automapstats)
    {
	int time = leveltime / TICRATE;

	// [crispy] move obtrusive line out of player view
	if (!crispy_automapoverlay || screenblocks < CRISPY_HUD - 1)
	    HUlib_drawTextLine(&w_map, false);

	// [crispy] count Lost Souls and spawned monsters
	if (extrakills)
	    M_snprintf(str, sizeof(str), "%sKills: %s%d/%d+%d", crstr[CR_RED], crstr[CR_GRAY],
	            players[consoleplayer].killcount, totalkills, extrakills);
	else
	    M_snprintf(str, sizeof(str), "%sKills: %s%d/%d", crstr[CR_RED], crstr[CR_GRAY],
	            players[consoleplayer].killcount, totalkills);
	HUlib_clearTextLine(&w_kills);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_kills, *(s++));
	HUlib_drawTextLine(&w_kills, false);

	M_snprintf(str, sizeof(str), "%sItems: %s%d/%d", crstr[CR_RED], crstr[CR_GRAY],
	        players[consoleplayer].itemcount, totalitems);
	HUlib_clearTextLine(&w_items);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_items, *(s++));
	HUlib_drawTextLine(&w_items, false);

	M_snprintf(str, sizeof(str), "%sSecret: %s%d/%d", crstr[CR_RED], crstr[CR_GRAY],
	        players[consoleplayer].secretcount, totalsecret);
	HUlib_clearTextLine(&w_scrts);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_scrts, *(s++));
	HUlib_drawTextLine(&w_scrts, false);

	M_snprintf(str, sizeof(str), "%s%02d:%02d:%02d", crstr[CR_GRAY],
	        time/3600, (time%3600)/60, time%60);
	HUlib_clearTextLine(&w_ltime);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_ltime, *(s++));
	HUlib_drawTextLine(&w_ltime, false);
    }

    // [crispy] show map coordinates in upper right corner
    // if either automap stats or IDMYPOS cheat are enabled
    if ((automapactive && crispy_automapstats) || coord_on)
    {
	M_snprintf(str, sizeof(str), "%sX: %s%-5d", crstr[CR_GREEN], crstr[CR_GRAY],
	        (players[consoleplayer].mo->x)>>FRACBITS);
	HUlib_clearTextLine(&w_coordx);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_coordx, *(s++));
	HUlib_drawTextLine(&w_coordx, false);

	M_snprintf(str, sizeof(str), "%sY: %s%-5d", crstr[CR_GREEN], crstr[CR_GRAY],
	        (players[consoleplayer].mo->y)>>FRACBITS);
	HUlib_clearTextLine(&w_coordy);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_coordy, *(s++));
	HUlib_drawTextLine(&w_coordy, false);

	M_snprintf(str, sizeof(str), "%sA: %s%-5d", crstr[CR_GREEN], crstr[CR_GRAY],
	        (players[consoleplayer].mo->angle)*(uint64_t)360/UINT32_MAX);
	HUlib_clearTextLine(&w_coorda);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_coorda, *(s++));
	HUlib_drawTextLine(&w_coorda, false);
    }

    if (crispy_showfps)
    {
	extern int crispy_fps;

	M_snprintf(str, sizeof(str), "%s%-4d %sFPS", crstr[CR_GRAY], crispy_fps, crstr[CR_GREEN]);
	HUlib_clearTextLine(&w_fps);
	s = str;
	while (*s)
	    HUlib_addCharToTextLine(&w_fps, *(s++));
	HUlib_drawTextLine(&w_fps, false);
    }

    V_ClearDPTranslation();

    if (crispy_crosshair == CROSSHAIR_STATIC)
	HU_DrawCrosshair();

    if (dp_translucent)
	dp_translucent = false;

    // [crispy] demo progress bar
    if (demoplayback)
	HU_DemoProgressBar();
}

void HU_Erase(void)
{

    HUlib_eraseSText(&w_message);
    HUlib_eraseSText(&w_secret);
    HUlib_eraseIText(&w_chat);
    HUlib_eraseTextLine(&w_title);
    HUlib_eraseTextLine(&w_coordx);
    HUlib_eraseTextLine(&w_coordy);
    HUlib_eraseTextLine(&w_coorda);
    HUlib_eraseTextLine(&w_fps);

}

void HU_Ticker(void)
{

    int i, rc;
    char c;

    // tick down message counter if message is up
    if (message_counter && !--message_counter)
    {
	message_on = false;
	message_nottobefuckedwith = false;
    }

    if (secret_counter && !--secret_counter)
    {
	secret_on = false;
    }

    if (coord_counter && !--coord_counter)
    {
	coord_on = false;
    }

    if (showMessages || message_dontfuckwithme)
    {

	// [crispy] display centered message
	if (plr->centermessage)
	{
	    HUlib_addMessageToSText(&w_secret, 0, plr->centermessage);
	    plr->centermessage = 0;
	    secret_on = true;
	    secret_counter = HU_MSGTIMEOUT >> 1;
	}

	// [crispy] display map coordinates
	if (plr->mapcoords)
	{
	    plr->mapcoords = 0;
	    coord_on = true;
	    coord_counter = HU_MSGTIMEOUT << 1;
	}

	// display message if necessary
	if ((plr->message && !message_nottobefuckedwith)
	    || (plr->message && message_dontfuckwithme))
	{
	    HUlib_addMessageToSText(&w_message, 0, plr->message);
	    plr->message = 0;
	    message_on = true;
	    message_counter = HU_MSGTIMEOUT;
	    message_nottobefuckedwith = message_dontfuckwithme;
	    message_dontfuckwithme = 0;
	}

    } // else message_on = false;

    // check for incoming chat characters
    if (netgame)
    {
	for (i=0 ; i<MAXPLAYERS; i++)
	{
	    if (!playeringame[i])
		continue;
	    if (i != consoleplayer
		&& (c = players[i].cmd.chatchar))
	    {
		if (c <= HU_BROADCAST)
		    chat_dest[i] = c;
		else
		{
		    rc = HUlib_keyInIText(&w_inputbuffer[i], c);
		    if (rc && c == KEY_ENTER)
		    {
			if (w_inputbuffer[i].l.len
			    && (chat_dest[i] == consoleplayer+1
				|| chat_dest[i] == HU_BROADCAST))
			{
			    HUlib_addMessageToSText(&w_message,
						    DEH_String(player_names[i]),
						    w_inputbuffer[i].l.l);
			    
			    message_nottobefuckedwith = true;
			    message_on = true;
			    message_counter = HU_MSGTIMEOUT;
			    if ( gamemode == commercial )
			      S_StartSound(0, sfx_radio);
			    else
			      S_StartSound(0, sfx_tink);
			}
			HUlib_resetIText(&w_inputbuffer[i]);
		    }
		}
		players[i].cmd.chatchar = 0;
	    }
	}
    }

}

#define QUEUESIZE		128

static char	chatchars[QUEUESIZE];
static int	head = 0;
static int	tail = 0;


void HU_queueChatChar(char c)
{
    if (((head + 1) & (QUEUESIZE-1)) == tail)
    {
	plr->message = DEH_String(HUSTR_MSGU);
    }
    else
    {
	chatchars[head] = c;
	head = (head + 1) & (QUEUESIZE-1);
    }
}

char HU_dequeueChatChar(void)
{
    char c;

    if (head != tail)
    {
	c = chatchars[tail];
	tail = (tail + 1) & (QUEUESIZE-1);
    }
    else
    {
	c = 0;
    }

    return c;
}

boolean HU_Responder(event_t *ev)
{

    static char		lastmessage[HU_MAXLINELENGTH+1];
    char*		macromessage;
    boolean		eatkey = false;
    static boolean	altdown = false;
    unsigned char 	c;
    int			i;
    int			numplayers;
    
    static int		num_nobrainers = 0;

    numplayers = 0;
    for (i=0 ; i<MAXPLAYERS ; i++)
	numplayers += playeringame[i];

    if (ev->data1 == KEY_RSHIFT)
    {
	return false;
    }
    else if (ev->data1 == KEY_RALT || ev->data1 == KEY_LALT)
    {
	altdown = ev->type == ev_keydown;
	return false;
    }

    if (ev->type != ev_keydown)
	return false;

    if (!chat_on)
    {
	if (ev->data1 == key_message_refresh)
	{
	    message_on = true;
	    message_counter = HU_MSGTIMEOUT;
	    eatkey = true;
	}
	else if (netgame && ev->data2 == key_multi_msg)
	{
	    eatkey = chat_on = true;
	    HUlib_resetIText(&w_chat);
	    HU_queueChatChar(HU_BROADCAST);
	}
	else if (netgame && numplayers > 2)
	{
	    for (i=0; i<MAXPLAYERS ; i++)
	    {
		if (ev->data2 == key_multi_msgplayer[i])
		{
		    if (playeringame[i] && i!=consoleplayer)
		    {
			eatkey = chat_on = true;
			HUlib_resetIText(&w_chat);
			HU_queueChatChar(i+1);
			break;
		    }
		    else if (i == consoleplayer)
		    {
			num_nobrainers++;
			if (num_nobrainers < 3)
			    plr->message = DEH_String(HUSTR_TALKTOSELF1);
			else if (num_nobrainers < 6)
			    plr->message = DEH_String(HUSTR_TALKTOSELF2);
			else if (num_nobrainers < 9)
			    plr->message = DEH_String(HUSTR_TALKTOSELF3);
			else if (num_nobrainers < 32)
			    plr->message = DEH_String(HUSTR_TALKTOSELF4);
			else
			    plr->message = DEH_String(HUSTR_TALKTOSELF5);
		    }
		}
	    }
	}
    }
    else
    {
	// send a macro
	if (altdown)
	{
	    c = ev->data1 - '0';
	    if (c > 9)
		return false;
	    // fprintf(stderr, "got here\n");
	    macromessage = chat_macros[c];
	    
	    // kill last message with a '\n'
	    HU_queueChatChar(KEY_ENTER); // DEBUG!!!
	    
	    // send the macro message
	    while (*macromessage)
		HU_queueChatChar(*macromessage++);
	    HU_queueChatChar(KEY_ENTER);
	    
            // leave chat mode and notify that it was sent
            chat_on = false;
            M_StringCopy(lastmessage, chat_macros[c], sizeof(lastmessage));
            plr->message = lastmessage;
            eatkey = true;
	}
	else
	{
            c = ev->data2;

	    eatkey = HUlib_keyInIText(&w_chat, c);
	    if (eatkey)
	    {
		// static unsigned char buf[20]; // DEBUG
		HU_queueChatChar(c);
		
		// M_snprintf(buf, sizeof(buf), "KEY: %d => %d", ev->data1, c);
		//        plr->message = buf;
	    }
	    if (c == KEY_ENTER)
	    {
		chat_on = false;
                if (w_chat.l.len)
                {
                    M_StringCopy(lastmessage, w_chat.l.l, sizeof(lastmessage));
                    plr->message = lastmessage;
                }
	    }
	    else if (c == KEY_ESCAPE)
		chat_on = false;
	}
    }

    return eatkey;

}
