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



#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "doomdef.h" 
#include "doomkeys.h"
#include "doomstat.h"

#include "deh_main.h"
#include "deh_misc.h"
#include "deh_bexpars.h" // [crispy] bex_pars[]

#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"

#include "p_setup.h"
#include "p_saveg.h"
#include "p_extsaveg.h"
#include "p_tick.h"

#include "d_main.h"

#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "statdump.h"

// Needs access to LFB.
#include "v_video.h"

#include "w_wad.h"

#include "p_local.h" 

#include "s_sound.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "r_sky.h"



#include "g_game.h"
#include "v_trans.h" // [crispy] colored "always run" message


#define SAVEGAMESIZE	0x2c000

void	G_ReadDemoTiccmd (ticcmd_t* cmd); 
void	G_WriteDemoTiccmd (ticcmd_t* cmd); 
void	G_PlayerReborn (int player); 
 
void	G_DoReborn (int playernum); 
 
void	G_DoLoadLevel (void); 
void	G_DoNewGame (void); 
void	G_DoPlayDemo (void); 
void	G_DoCompleted (void); 
void	G_DoVictory (void); 
void	G_DoWorldDone (void); 
void	G_DoSaveGame (void); 
 
// Gamestate the last time G_Ticker was called.

gamestate_t     oldgamestate; 
 
gameaction_t    gameaction; 
gamestate_t     gamestate; 
skill_t         gameskill; 
boolean		respawnmonsters;
int             gameepisode; 
int             gamemap; 

// If non-zero, exit the level after this number of minutes.

int             timelimit;

boolean         paused; 
boolean         sendpause;             	// send a pause event next tic 
boolean         sendsave;             	// send a save event next tic 
boolean         usergame;               // ok to save / end game 
 
boolean         timingdemo;             // if true, exit with report on completion 
boolean         nodrawers;              // for comparative timing purposes 
int             starttime;          	// for comparative timing purposes  	 
 
boolean         viewactive; 
 
int             deathmatch;           	// only if started as net death 
boolean         netgame;                // only true if packets are broadcast 
boolean         playeringame[MAXPLAYERS]; 
player_t        players[MAXPLAYERS]; 

boolean         turbodetected[MAXPLAYERS];
 
int             consoleplayer;          // player taking events and displaying 
int             displayplayer;          // view being displayed 
int             levelstarttic;          // gametic at level start 
int             totalkills, totalitems, totalsecret;    // for intermission 
int             extrakills;             // [crispy] count spawned monsters
int             totalleveltimes;        // [crispy] CPhipps - total time for all completed levels
int             demostarttic;           // [crispy] fix revenant internal demo bug
 
char           *demoname;
char           *orig_demoname; // [crispy] the name originally chosen for the demo, i.e. without "-00000"
boolean         demorecording; 
boolean         longtics;               // cph's doom 1.91 longtics hack
boolean         lowres_turn;            // low resolution turning for longtics
boolean         demoplayback; 
boolean		netdemo; 
byte*		demobuffer;
byte*		demo_p;
byte*		demoend; 
boolean         singledemo;            	// quit after playing a demo from cmdline 
 
boolean         precache = true;        // if true, load all graphics at start 

boolean         testcontrols = false;    // Invoked by setup to test controls
int             testcontrols_mousespeed;
 

 
wbstartstruct_t wminfo;               	// parms for world map / intermission 
 
byte		consistancy[MAXPLAYERS][BACKUPTICS]; 
 
#define MAXPLMOVE		(forwardmove[1]) 
 
#define TURBOTHRESHOLD	0x32

fixed_t         forwardmove[2] = {0x19, 0x32}; 
fixed_t         sidemove[2] = {0x18, 0x28}; 
fixed_t         angleturn[3] = {640, 1280, 320};    // + slow turn 

static int *weapon_keys[] = {
    &key_weapon1,
    &key_weapon2,
    &key_weapon3,
    &key_weapon4,
    &key_weapon5,
    &key_weapon6,
    &key_weapon7,
    &key_weapon8
};

// Set to -1 or +1 to switch to the previous or next weapon.

static int next_weapon = 0;

// Used for prev/next weapon keys.

static const struct
{
    weapontype_t weapon;
    weapontype_t weapon_num;
} weapon_order_table[] = {
    { wp_fist,            wp_fist },
    { wp_chainsaw,        wp_fist },
    { wp_pistol,          wp_pistol },
    { wp_shotgun,         wp_shotgun },
    { wp_supershotgun,    wp_shotgun },
    { wp_chaingun,        wp_chaingun },
    { wp_missile,         wp_missile },
    { wp_plasma,          wp_plasma },
    { wp_bfg,             wp_bfg }
};

#define SLOWTURNTICS	6 
 
#define NUMKEYS		256 
#define MAX_JOY_BUTTONS 20

static boolean  gamekeydown[NUMKEYS]; 
static int      turnheld;		// for accelerative turning 
static int      lookheld;		// [crispy] for accelerative looking
 
static boolean  mousearray[MAX_MOUSE_BUTTONS + 1];
static boolean *mousebuttons = &mousearray[1];  // allow [-1]

// mouse values are used once 
int             mousex;
int             mousex2;
int             mousey;         

static int      dclicktime;
static boolean  dclickstate;
static int      dclicks; 
static int      dclicktime2;
static boolean  dclickstate2;
static int      dclicks2;

// joystick values are repeated 
static int      joyxmove;
static int      joyymove;
static int      joystrafemove;
static int      joylook; // [crispy]
static boolean  joyarray[MAX_JOY_BUTTONS + 1]; 
static boolean *joybuttons = &joyarray[1];		// allow [-1] 
 
static char     savename[256]; // [crispy] moved here, made static
static int      savegameslot; 
static char     savedescription[32]; 
 
#define	BODYQUESIZE	32

mobj_t*		bodyque[BODYQUESIZE]; 
int		bodyqueslot; 
 
int             vanilla_savegame_limit = 1;
int             vanilla_demo_limit = 1;
 
int G_CmdChecksum (ticcmd_t* cmd) 
{ 
    size_t		i;
    int		sum = 0; 
	 
    for (i=0 ; i< sizeof(*cmd)/4 - 1 ; i++) 
	sum += ((int *)cmd)[i]; 
		 
    return sum; 
} 

static boolean WeaponSelectable(weapontype_t weapon)
{
    // Can't select the super shotgun in Doom 1.

    if (weapon == wp_supershotgun && !crispy->havessg)
    {
        return false;
    }

    // These weapons aren't available in shareware.

    if ((weapon == wp_plasma || weapon == wp_bfg)
     && gamemission == doom && gamemode == shareware)
    {
        return false;
    }

    // Can't select a weapon if we don't own it.

    if (!players[consoleplayer].weaponowned[weapon])
    {
        return false;
    }

    // Can't select the fist if we have the chainsaw, unless
    // we also have the berserk pack.

    if (weapon == wp_fist
     && players[consoleplayer].weaponowned[wp_chainsaw]
     && !players[consoleplayer].powers[pw_strength])
    {
        return false;
    }

    return true;
}

static int G_NextWeapon(int direction)
{
    weapontype_t weapon;
    int start_i, i;

    // Find index in the table.

    if (players[consoleplayer].pendingweapon == wp_nochange)
    {
        weapon = players[consoleplayer].readyweapon;
    }
    else
    {
        weapon = players[consoleplayer].pendingweapon;
    }

    for (i=0; i<arrlen(weapon_order_table); ++i)
    {
        if (weapon_order_table[i].weapon == weapon)
        {
            break;
        }
    }

    // Switch weapon. Don't loop forever.
    start_i = i;
    do
    {
        i += direction;
        i = (i + arrlen(weapon_order_table)) % arrlen(weapon_order_table);
    } while (i != start_i && !WeaponSelectable(weapon_order_table[i].weapon));

    return weapon_order_table[i].weapon_num;
}

// [crispy] holding down the "Run" key may trigger special behavior,
// e.g. quick exit, clean screenshots, resurrection from savegames
boolean speedkeydown (void)
{
    return (key_speed < NUMKEYS && gamekeydown[key_speed]) ||
           (joybspeed < MAX_JOY_BUTTONS && joybuttons[joybspeed]);
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer. 
// If recording a demo, write it out 
// 
void G_BuildTiccmd (ticcmd_t* cmd, int maketic) 
{ 
    int		i; 
    boolean	strafe;
    boolean	bstrafe; 
    int		speed;
    int		tspeed; 
    int		lspeed;
    int		forward;
    int		side;
    int		look;
    player_t *const player = &players[consoleplayer];
    static char playermessage[48];

    memset(cmd, 0, sizeof(ticcmd_t));

    cmd->consistancy = 
	consistancy[consoleplayer][maketic%BACKUPTICS]; 
 
    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe] 
	|| joybuttons[joybstrafe]; 

    // fraggle: support the old "joyb_speed = 31" hack which
    // allowed an autorun effect

    // [crispy] when "always run" is active,
    // pressing the "run" key will result in walking
    speed = key_speed >= NUMKEYS
         || joybspeed >= MAX_JOY_BUTTONS;
    speed ^= speedkeydown();
 
    forward = side = look = 0;
    
    // use two stage accelerative turning
    // on the keyboard and joystick
    if (joyxmove < 0
	|| joyxmove > 0  
	|| gamekeydown[key_right]
	|| gamekeydown[key_left]) 
	turnheld += ticdup; 
    else 
	turnheld = 0; 

    if (turnheld < SLOWTURNTICS) 
	tspeed = 2;             // slow turn 
    else 
	tspeed = speed;
    
    // [crispy] use two stage accelerative looking
    if (gamekeydown[key_lookdown] || gamekeydown[key_lookup])
    {
        lookheld += ticdup;
    }
    else
    {
        lookheld = 0;
    }
    if (lookheld < SLOWTURNTICS)
    {
        lspeed = 1;
    }
    else
    {
        lspeed = 2;
    }

    // [crispy] add quick 180° reverse
    if (gamekeydown[key_reverse] || mousebuttons[mousebreverse])
    {
        cmd->angleturn += ANG180 >> FRACBITS;
        gamekeydown[key_reverse] = false;
        mousebuttons[mousebreverse] = false;
    }

    // [crispy] toggle "always run"
    if (gamekeydown[key_toggleautorun])
    {
        static int joybspeed_old = 2;

        if (joybspeed >= MAX_JOY_BUTTONS)
        {
            joybspeed = joybspeed_old;
        }
        else
        {
            joybspeed_old = joybspeed;
            joybspeed = 29;
        }

        M_snprintf(playermessage, sizeof(playermessage), "ALWAYS RUN %s%s",
            crstr[CR_GREEN],
            (joybspeed >= MAX_JOY_BUTTONS) ? "ON" : "OFF");
        player->message = playermessage;
        S_StartSound(NULL, sfx_swtchn);

        gamekeydown[key_toggleautorun] = false;
    }

    // [crispy] Toggle vertical mouse movement
    if (gamekeydown[key_togglenovert])
    {
        novert = !novert;

        M_snprintf(playermessage, sizeof(playermessage),
            "vertical mouse movement %s%s",
            crstr[CR_GREEN],
            !novert ? "ON" : "OFF");
        player->message = playermessage;
        S_StartSound(NULL, sfx_swtchn);

        gamekeydown[key_togglenovert] = false;
    }

    // [crispy] extra high precision IDMYPOS variant, updates for 10 seconds
    if (player->powers[pw_mapcoords])
    {
        M_snprintf(playermessage, sizeof(playermessage),
            "X=%.10f Y=%.10f A=%d",
            (double)player->mo->x/FRACUNIT,
            (double)player->mo->y/FRACUNIT,
            player->mo->angle >> 24);
        player->message = playermessage;

        player->powers[pw_mapcoords]--;

        // [crispy] discard instead of going static
        if (!player->powers[pw_mapcoords])
        {
            player->message = "";
        }
    }

    // let movement keys cancel each other out
    if (strafe) 
    { 
	if (gamekeydown[key_right]) 
	{
	    // fprintf(stderr, "strafe right\n");
	    side += sidemove[speed]; 
	}
	if (gamekeydown[key_left]) 
	{
	    //	fprintf(stderr, "strafe left\n");
	    side -= sidemove[speed]; 
	}
	if (joyxmove > 0) 
	    side += sidemove[speed]; 
	if (joyxmove < 0) 
	    side -= sidemove[speed]; 
 
    } 
    else 
    { 
	if (gamekeydown[key_right]) 
	    cmd->angleturn -= angleturn[tspeed]; 
	if (gamekeydown[key_left]) 
	    cmd->angleturn += angleturn[tspeed]; 
	if (joyxmove > 0) 
	    cmd->angleturn -= angleturn[tspeed]; 
	if (joyxmove < 0) 
	    cmd->angleturn += angleturn[tspeed]; 
    } 
 
    if (gamekeydown[key_up] || gamekeydown[key_alt_up]) // [crispy] add key_alt_*
    {
	// fprintf(stderr, "up\n");
	forward += forwardmove[speed]; 
    }
    if (gamekeydown[key_down] || gamekeydown[key_alt_down]) // [crispy] add key_alt_*
    {
	// fprintf(stderr, "down\n");
	forward -= forwardmove[speed]; 
    }

    if (joyymove < 0) 
        forward += forwardmove[speed]; 
    if (joyymove > 0) 
        forward -= forwardmove[speed]; 

    if (gamekeydown[key_strafeleft] || gamekeydown[key_alt_strafeleft] // [crispy] add key_alt_*
     || joybuttons[joybstrafeleft]
     || mousebuttons[mousebstrafeleft]
     || joystrafemove < 0)
    {
        side -= sidemove[speed];
    }

    if (gamekeydown[key_straferight] || gamekeydown[key_alt_straferight] // [crispy] add key_alt_*
     || joybuttons[joybstraferight]
     || mousebuttons[mousebstraferight]
     || joystrafemove > 0)
    {
        side += sidemove[speed]; 
    }

    // [crispy] look up/down/center keys
    if (crispy->freelook)
    {
        static unsigned int kbdlookctrl = 0;

        if (gamekeydown[key_lookup] || joylook < 0)
        {
            look = lspeed;
            kbdlookctrl += ticdup;
        }
        else
        if (gamekeydown[key_lookdown] || joylook > 0)
        {
            look = -lspeed;
            kbdlookctrl += ticdup;
        }
        else
        // [crispy] keyboard lookspring
        if (gamekeydown[key_lookcenter] || (crispy->freelook == FREELOOK_SPRING && kbdlookctrl))
        {
            look = TOCENTER;
            kbdlookctrl = 0;
        }
    }

    // [crispy] jump keys
    if (critical->jump)
    {
        if (gamekeydown[key_jump] || mousebuttons[mousebjump]
            || joybuttons[joybjump])
        {
            cmd->arti |= AFLAG_JUMP;
        }
    }

    // buttons
    cmd->chatchar = HU_dequeueChatChar(); 
 
    if (gamekeydown[key_fire] || mousebuttons[mousebfire] 
	|| joybuttons[joybfire]) 
	cmd->buttons |= BT_ATTACK; 
 
    if (gamekeydown[key_use]
     || joybuttons[joybuse]
     || mousebuttons[mousebuse])
    { 
	cmd->buttons |= BT_USE;
	// clear double clicks if hit use button 
	dclicks = 0;                   
    } 

    // If the previous or next weapon button is pressed, the
    // next_weapon variable is set to change weapons when
    // we generate a ticcmd.  Choose a new weapon.

    if (gamestate == GS_LEVEL && next_weapon != 0)
    {
        i = G_NextWeapon(next_weapon);
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= i << BT_WEAPONSHIFT;
    }
    else
    {
        // Check weapon keys.

        for (i=0; i<arrlen(weapon_keys); ++i)
        {
            int key = *weapon_keys[i];

            if (gamekeydown[key])
            {
                cmd->buttons |= BT_CHANGE;
                cmd->buttons |= i<<BT_WEAPONSHIFT;
                break;
            }
        }
    }

    next_weapon = 0;

    // mouse
    if (mousebuttons[mousebforward]) 
    {
	forward += forwardmove[speed];
    }
    if (mousebuttons[mousebbackward])
    {
        forward -= forwardmove[speed];
    }

    if (dclick_use)
    {
        // forward double click
        if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1 ) 
        { 
            dclickstate = mousebuttons[mousebforward]; 
            if (dclickstate) 
                dclicks++; 
            if (dclicks == 2) 
            { 
                cmd->buttons |= BT_USE; 
                dclicks = 0; 
            } 
            else 
                dclicktime = 0; 
        } 
        else 
        { 
            dclicktime += ticdup; 
            if (dclicktime > 20) 
            { 
                dclicks = 0; 
                dclickstate = 0; 
            } 
        }
        
        // strafe double click
        bstrafe =
            mousebuttons[mousebstrafe] 
            || joybuttons[joybstrafe]; 
        if (bstrafe != dclickstate2 && dclicktime2 > 1 ) 
        { 
            dclickstate2 = bstrafe; 
            if (dclickstate2) 
                dclicks2++; 
            if (dclicks2 == 2) 
            { 
                cmd->buttons |= BT_USE; 
                dclicks2 = 0; 
            } 
            else 
                dclicktime2 = 0; 
        } 
        else 
        { 
            dclicktime2 += ticdup; 
            if (dclicktime2 > 20) 
            { 
                dclicks2 = 0; 
                dclickstate2 = 0; 
            } 
        } 
    }

    // [crispy] mouse look
    if ((crispy->freelook && mousebuttons[mousebmouselook]) ||
         crispy->mouselook)
    {
        cmd->lookdir = mouse_y_invert ? -mousey : mousey;
    }
    else
    if (!novert)
    {
    forward += mousey;
    }

    // [crispy] single click on mouse look button centers view
    if (crispy->freelook)
    {
        static unsigned int mbmlookctrl = 0;

        // [crispy] single click view centering
        if (mousebuttons[mousebmouselook]) // [crispy] clicked
        {
            mbmlookctrl += ticdup;
        }
        else
        // [crispy] released
        if (mbmlookctrl)
        {
            if (crispy->freelook == FREELOOK_SPRING || mbmlookctrl < SLOWTURNTICS) // [crispy] short click
            {
                look = TOCENTER;
            }
            mbmlookctrl = 0;
        }
    }

    if (strafe) 
	side += mousex2*2;
    else 
	cmd->angleturn -= mousex*0x8; 

    if (mousex == 0)
    {
        // No movement in the previous frame

        testcontrols_mousespeed = 0;
    }
    
    mousex = mousex2 = mousey = 0;
	 
    if (forward > MAXPLMOVE) 
	forward = MAXPLMOVE; 
    else if (forward < -MAXPLMOVE) 
	forward = -MAXPLMOVE; 
    if (side > MAXPLMOVE) 
	side = MAXPLMOVE; 
    else if (side < -MAXPLMOVE) 
	side = -MAXPLMOVE; 
 
    cmd->forwardmove += forward; 
    cmd->sidemove += side;

    // [crispy] lookdir delta is stored in the lower 4 bits of the lookfly variable
    if (player->playerstate == PST_LIVE)
    {
        if (look < 0)
        {
            look += 16;
        }
        cmd->lookfly = look;
    }
    
    // special buttons
    if (sendpause) 
    { 
	sendpause = false; 
	// [crispy] ignore un-pausing in menus during demo recording
	if (!(menuactive && demorecording && paused) && gameaction != ga_loadgame)
	{
	cmd->buttons = BT_SPECIAL | BTS_PAUSE; 
	}
    } 
 
    if (sendsave) 
    { 
	sendsave = false; 
	cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT); 
    } 

    if (crispy->fliplevels)
    {
	cmd->angleturn = -cmd->angleturn;
	cmd->sidemove = -cmd->sidemove;
    }

    // low-res turning

    if (lowres_turn)
    {
        static signed short carry = 0;
        signed short desired_angleturn;

        desired_angleturn = cmd->angleturn + carry;

        // round angleturn to the nearest 256 unit boundary
        // for recording demos with single byte values for turn

        cmd->angleturn = (desired_angleturn + 128) & 0xff00;

        // Carry forward the error from the reduced resolution to the
        // next tic, so that successive small movements can accumulate.

        carry = desired_angleturn - cmd->angleturn;
    }
} 
 

//
// G_DoLoadLevel 
//
void G_DoLoadLevel (void) 
{ 
    int             i; 

    // Set the sky map.
    // First thing, we have a dummy sky texture name,
    //  a flat. The data is in the WAD only because
    //  we look for an actual index, instead of simply
    //  setting one.

    skyflatnum = R_FlatNumForName(DEH_String(SKYFLATNAME));

    // The "Sky never changes in Doom II" bug was fixed in
    // the id Anthology version of doom2.exe for Final Doom.
    // [crispy] correct "Sky never changes in Doom II" bug
    if ((gamemode == commercial)
     && (gameversion == exe_final2 || gameversion == exe_chex || true))
    {
        const char *skytexturename;

        if (gamemap < 12)
        {
            if ((gameepisode == 2 || gamemission == pack_nerve) && gamemap >= 4 && gamemap <= 8)
                skytexturename = "SKY3";
            else
            skytexturename = "SKY1";
        }
        else if (gamemap < 21)
        {
            skytexturename = "SKY2";
        }
        else
        {
            skytexturename = "SKY3";
        }

        skytexturename = DEH_String(skytexturename);

        skytexture = R_TextureNumForName(skytexturename);
    }
    // [crispy] sky texture scales
    R_InitSkyMap();

    levelstarttic = gametic;        // for time calculation
    
    if (wipegamestate == GS_LEVEL) 
	wipegamestate = -1;             // force a wipe 

    gamestate = GS_LEVEL; 

    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
	turbodetected[i] = false;
	if (playeringame[i] && players[i].playerstate == PST_DEAD) 
	    players[i].playerstate = PST_REBORN; 
	memset (players[i].frags,0,sizeof(players[i].frags)); 
    } 
		 
    // [crispy] update the "singleplayer" variable
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
    P_SetupLevel (gameepisode, gamemap, 0, gameskill);    
    displayplayer = consoleplayer;		// view the guy you are playing    
    gameaction = ga_nothing; 
    Z_CheckHeap ();
    
    // clear cmd building stuff

    memset (gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joystrafemove = joylook = 0;
    mousex = mousex2 = mousey = 0;
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));
    memset(joyarray, 0, sizeof(joyarray));

    if (testcontrols)
    {
        players[consoleplayer].message = "Press escape to quit.";
    }
} 

static void SetJoyButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_JOY_BUTTONS; ++i)
    {
        int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!joybuttons[i] && button_on)
        {
            // Weapon cycling:

            if (i == joybprevweapon)
            {
                next_weapon = -1;
            }
            else if (i == joybnextweapon)
            {
                next_weapon = 1;
            }
        }

        joybuttons[i] = button_on;
    }
}

static void SetMouseButtons(unsigned int buttons_mask)
{
    int i;

    for (i=0; i<MAX_MOUSE_BUTTONS; ++i)
    {
        unsigned int button_on = (buttons_mask & (1 << i)) != 0;

        // Detect button press:

        if (!mousebuttons[i] && button_on)
        {
            if (i == mousebprevweapon)
            {
                next_weapon = -1;
            }
            else if (i == mousebnextweapon)
            {
                next_weapon = 1;
            }
        }

	mousebuttons[i] = button_on;
    }
}

//
// G_Responder  
// Get info needed to make ticcmd_ts for the players.
// 
boolean G_Responder (event_t* ev) 
{ 
    // allow spy mode changes even during the demo
    if (gamestate == GS_LEVEL && ev->type == ev_keydown 
     && ev->data1 == key_spy && (singledemo || !deathmatch) )
    {
	// spy mode 
	do 
	{ 
	    displayplayer++; 
	    if (displayplayer == MAXPLAYERS) 
		displayplayer = 0; 
	} while (!playeringame[displayplayer] && displayplayer != consoleplayer); 
	return true; 
    }
    
    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo && 
	(demoplayback || gamestate == GS_DEMOSCREEN) 
	) 
    { 
	if (ev->type == ev_keydown ||  
	    (ev->type == ev_mouse && ev->data1) || 
	    (ev->type == ev_joystick && ev->data1) ) 
	{ 
	    M_StartControlPanel (); 
	    // [crispy] play a sound if the menu is activated with a different key than ESC
	    if (crispy->soundfix)
		S_StartSound(NULL,sfx_swtchn);
	    return true; 
	} 
	return false; 
    } 

    if (gamestate == GS_LEVEL) 
    { 
#if 0 
	if (devparm && ev->type == ev_keydown && ev->data1 == ';') 
	{ 
	    G_DeathMatchSpawnPlayer (0); 
	    return true; 
	} 
#endif 
	if (HU_Responder (ev)) 
	    return true;	// chat ate the event 
	if (ST_Responder (ev)) 
	    return true;	// status window ate it 
	if (AM_Responder (ev)) 
	    return true;	// automap ate it 
    } 
	 
    if (gamestate == GS_FINALE) 
    { 
	if (F_Responder (ev)) 
	    return true;	// finale ate the event 
    } 

    if (testcontrols && ev->type == ev_mouse)
    {
        // If we are invoked by setup to test the controls, save the 
        // mouse speed so that we can display it on-screen.
        // Perform a low pass filter on this so that the thermometer 
        // appears to move smoothly.

        testcontrols_mousespeed = abs(ev->data2);
    }

    // If the next/previous weapon keys are pressed, set the next_weapon
    // variable to change weapons when the next ticcmd is generated.

    if (ev->type == ev_keydown && ev->data1 == key_prevweapon)
    {
        next_weapon = -1;
    }
    else if (ev->type == ev_keydown && ev->data1 == key_nextweapon)
    {
        next_weapon = 1;
    }

    switch (ev->type) 
    { 
      case ev_keydown: 
	if (ev->data1 == key_pause) 
	{ 
	    sendpause = true; 
	}
        else if (ev->data1 <NUMKEYS) 
        {
	    gamekeydown[ev->data1] = true; 
        }

	return true;    // eat key down events 
 
      case ev_keyup: 
	if (ev->data1 <NUMKEYS) 
	    gamekeydown[ev->data1] = false; 
	return false;   // always let key up events filter down 
		 
      case ev_mouse: 
        SetMouseButtons(ev->data1);
	if (mouseSensitivity)
	mousex = ev->data2*(mouseSensitivity+5)/10; 
	else
	    mousex = 0; // [crispy] disable entirely
	if (mouseSensitivity_x2)
	mousex2 = ev->data2*(mouseSensitivity_x2+5)/10; // [crispy] separate sensitivity for strafe
	else
	    mousex2 = 0; // [crispy] disable entirely
	if (mouseSensitivity_y)
	mousey = ev->data3*(mouseSensitivity_y+5)/10; // [crispy] separate sensitivity for y-axis
	else
	    mousey = 0; // [crispy] disable entirely
	return true;    // eat events 
 
      case ev_joystick: 
        SetJoyButtons(ev->data1);
	joyxmove = ev->data2; 
	joyymove = ev->data3; 
        joystrafemove = ev->data4;
        joylook = ev->data5;
	return true;    // eat events 
 
      default: 
	break; 
    } 
 
    return false; 
} 
 
 
// [crispy] re-read game parameters from command line
static void G_ReadGameParms (void)
{
    respawnparm = M_CheckParm ("-respawn");
    fastparm = M_CheckParm ("-fast");
    nomonsters = M_CheckParm ("-nomonsters");
}
 
// [crispy] take a screenshot after rendering the next frame
static void G_CrispyScreenShot()
{
	// [crispy] increase screenshot filename limit
	V_ScreenShot("DOOM%04i.%s");
	players[consoleplayer].message = DEH_String("screen shot");
	crispy->cleanscreenshot = 0;
	crispy->screenshotmsg = 2;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void) 
{ 
    int		i;
    int		buf; 
    ticcmd_t*	cmd;
    
    // do player reborns if needed
    for (i=0 ; i<MAXPLAYERS ; i++) 
	if (playeringame[i] && players[i].playerstate == PST_REBORN) 
	    G_DoReborn (i);
    
    // do things to change the game state
    while (gameaction != ga_nothing) 
    { 
	switch (gameaction) 
	{ 
	  case ga_loadlevel: 
	    G_DoLoadLevel (); 
	    break; 
	  case ga_newgame: 
	    // [crispy] re-read game parameters from command line
	    G_ReadGameParms();
	    G_DoNewGame (); 
	    break; 
	  case ga_loadgame: 
	    // [crispy] re-read game parameters from command line
	    G_ReadGameParms();
	    G_DoLoadGame (); 
	    break; 
	  case ga_savegame: 
	    G_DoSaveGame (); 
	    break; 
	  case ga_playdemo: 
	    G_DoPlayDemo (); 
	    break; 
	  case ga_completed: 
	    G_DoCompleted (); 
	    break; 
	  case ga_victory: 
	    F_StartFinale (); 
	    break; 
	  case ga_worlddone: 
	    G_DoWorldDone (); 
	    break; 
	  case ga_screenshot: 
	    // [crispy] redraw view without weapons and HUD
	    if (gamestate == GS_LEVEL && (crispy->cleanscreenshot || crispy->screenshotmsg == 1))
	    {
		crispy->screenshotmsg = 4;
		crispy->post_rendering_hook = G_CrispyScreenShot;
	    }
	    else
	    {
		G_CrispyScreenShot();
	    }
	    gameaction = ga_nothing; 
	    break; 
	  case ga_nothing: 
	    break; 
	} 
    }
    
    // get commands, check consistancy,
    // and build new consistancy check
    buf = (gametic/ticdup)%BACKUPTICS; 
 
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]) 
	{ 
	    cmd = &players[i].cmd; 

	    memcpy(cmd, &netcmds[i], sizeof(ticcmd_t));

	    if (demoplayback) 
		G_ReadDemoTiccmd (cmd); 
	    // [crispy] do not record tics while still playing back in demo continue mode
	    if (demorecording && !demoplayback)
		G_WriteDemoTiccmd (cmd);
	    
	    // check for turbo cheats

            // check ~ 4 seconds whether to display the turbo message. 
            // store if the turbo threshold was exceeded in any tics
            // over the past 4 seconds.  offset the checking period
            // for each player so messages are not displayed at the
            // same time.

            if (cmd->forwardmove > TURBOTHRESHOLD)
            {
                turbodetected[i] = true;
            }

            if ((gametic & 31) == 0 
             && ((gametic >> 5) % MAXPLAYERS) == i
             && turbodetected[i])
            {
                static char turbomessage[80];
                extern char *player_names[4];
                M_snprintf(turbomessage, sizeof(turbomessage),
                           "%s is turbo!", player_names[i]);
                players[consoleplayer].message = turbomessage;
                turbodetected[i] = false;
            }

	    if (netgame && !netdemo && !(gametic%ticdup) ) 
	    { 
		if (gametic > BACKUPTICS 
		    && consistancy[i][buf] != cmd->consistancy) 
		{ 
		    I_Error ("consistency failure (%i should be %i)",
			     cmd->consistancy, consistancy[i][buf]); 
		} 
		if (players[i].mo) 
		    consistancy[i][buf] = players[i].mo->x; 
		else 
		    consistancy[i][buf] = rndindex; 
	    } 
	}
    }
    
    // check for special buttons
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]) 
	{ 
	    if (players[i].cmd.buttons & BT_SPECIAL) 
	    { 
		switch (players[i].cmd.buttons & BT_SPECIALMASK) 
		{ 
		  case BTS_PAUSE: 
		    paused ^= 1; 
		    if (paused) 
			S_PauseSound (); 
		    else 
		    // [crispy] Fixed bug when music was hearable with zero volume
		    if (musicVolume)
			S_ResumeSound (); 
		    break; 
					 
		  case BTS_SAVEGAME: 
		    // [crispy] never override savegames by demo playback
		    if (demoplayback)
			break;
		    if (!savedescription[0]) 
                    {
                        M_StringCopy(savedescription, "NET GAME",
                                     sizeof(savedescription));
                    }

		    savegameslot =  
			(players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
		    gameaction = ga_savegame; 
		    // [crispy] un-pause immediately after saving
		    // (impossible to send save and pause specials within the same tic)
		    if (demorecording && paused)
			sendpause = true;
		    break; 
		} 
	    } 
	}
    }

    // Have we just finished displaying an intermission screen?

    if (oldgamestate == GS_INTERMISSION && gamestate != GS_INTERMISSION)
    {
        WI_End();
    }

    oldgamestate = gamestate;
    oldleveltime = leveltime;
    
    // do main actions
    switch (gamestate) 
    { 
      case GS_LEVEL: 
	P_Ticker (); 
	ST_Ticker (); 
	AM_Ticker (); 
	HU_Ticker ();            
	break; 
	 
      case GS_INTERMISSION: 
	WI_Ticker (); 
	break; 
			 
      case GS_FINALE: 
	F_Ticker (); 
	break; 
 
      case GS_DEMOSCREEN: 
	D_PageTicker (); 
	break;
    }        
} 
 
 
//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer 
// Called at the start.
// Called by the game initialization functions.
//
void G_InitPlayer (int player) 
{
    // clear everything else to defaults
    G_PlayerReborn (player); 
}
 
 

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel (int player) 
{ 
    player_t*	p; 
	 
    p = &players[player]; 
	 
    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    memset (p->tryopen, 0, sizeof (p->tryopen)); // [crispy] blinking key or skull in the status bar
    p->mo->flags &= ~MF_SHADOW;		// cancel invisibility 
    p->extralight = 0;			// cancel gun flashes 
    p->fixedcolormap = 0;		// cancel ir gogles 
    p->damagecount = 0;			// no palette changes 
    p->bonuscount = 0; 
    // [crispy] reset additional player properties
    p->lookdir = p->oldlookdir =
    p->centering =
    p->jumpTics =
    p->recoilpitch = p->oldrecoilpitch =
    p->psp_dy_max = 0;
} 
 

//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
//
void G_PlayerReborn (int player) 
{ 
    player_t*	p; 
    int		i; 
    int		frags[MAXPLAYERS]; 
    int		killcount;
    int		itemcount;
    int		secretcount; 
	 
    memcpy (frags,players[player].frags,sizeof(frags)); 
    killcount = players[player].killcount; 
    itemcount = players[player].itemcount; 
    secretcount = players[player].secretcount; 
	 
    p = &players[player]; 
    memset (p, 0, sizeof(*p)); 
 
    memcpy (players[player].frags, frags, sizeof(players[player].frags)); 
    players[player].killcount = killcount; 
    players[player].itemcount = itemcount; 
    players[player].secretcount = secretcount; 
 
    p->usedown = p->attackdown = true;	// don't do anything immediately 
    p->playerstate = PST_LIVE;       
    p->health = deh_initial_health;     // Use dehacked value
    // [crispy] negative player health
    p->neghealth = p->health;
    p->readyweapon = p->pendingweapon = wp_pistol; 
    p->weaponowned[wp_fist] = true; 
    p->weaponowned[wp_pistol] = true; 
    p->ammo[am_clip] = deh_initial_bullets; 
	 
    for (i=0 ; i<NUMAMMO ; i++) 
	p->maxammo[i] = maxammo[i]; 
		 
}

//
// G_CheckSpot  
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
//
void P_SpawnPlayer (mapthing_t* mthing); 
 
boolean
G_CheckSpot
( int		playernum,
  mapthing_t*	mthing ) 
{ 
    fixed_t		x;
    fixed_t		y; 
    subsector_t*	ss; 
    mobj_t*		mo; 
    int			i;
	
    if (!players[playernum].mo)
    {
	// first spawn of level, before corpses
	for (i=0 ; i<playernum ; i++)
	    if (players[i].mo->x == mthing->x << FRACBITS
		&& players[i].mo->y == mthing->y << FRACBITS)
		return false;	
	return true;
    }
		
    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 
	 
    if (!P_CheckPosition (players[playernum].mo, x, y) ) 
	return false; 
 
    // flush an old corpse if needed 
    if (bodyqueslot >= BODYQUESIZE) 
	P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]); 
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
    bodyqueslot++; 

    // spawn a teleport fog
    ss = R_PointInSubsector (x,y);


    // The code in the released source looks like this:
    //
    //    an = ( ANG45 * (((unsigned int) mthing->angle)/45) )
    //         >> ANGLETOFINESHIFT;
    //    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an]
    //                     , ss->sector->floorheight
    //                     , MT_TFOG);
    //
    // But 'an' can be a signed value in the DOS version. This means that
    // we get a negative index and the lookups into finecosine/finesine
    // end up dereferencing values in finetangent[].
    // A player spawning on a deathmatch start facing directly west spawns
    // "silently" with no spawn fog. Emulate this.
    //
    // This code is imported from PrBoom+.

    {
        fixed_t xa, ya;
        signed int an;

        // This calculation overflows in Vanilla Doom, but here we deliberately
        // avoid integer overflow as it is undefined behavior, so the value of
        // 'an' will always be positive.
        an = (ANG45 >> ANGLETOFINESHIFT) * ((signed int) mthing->angle / 45);

        switch (an)
        {
            case 4096:  // -4096:
                xa = finetangent[2048];    // finecosine[-4096]
                ya = finetangent[0];       // finesine[-4096]
                break;
            case 5120:  // -3072:
                xa = finetangent[3072];    // finecosine[-3072]
                ya = finetangent[1024];    // finesine[-3072]
                break;
            case 6144:  // -2048:
                xa = finesine[0];          // finecosine[-2048]
                ya = finetangent[2048];    // finesine[-2048]
                break;
            case 7168:  // -1024:
                xa = finesine[1024];       // finecosine[-1024]
                ya = finetangent[3072];    // finesine[-1024]
                break;
            case 0:
            case 1024:
            case 2048:
            case 3072:
                xa = finecosine[an];
                ya = finesine[an];
                break;
            default:
                I_Error("G_CheckSpot: unexpected angle %d\n", an);
                xa = ya = 0;
                break;
        }
        mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya,
                         ss->sector->floorheight, MT_TFOG);
    }

    if (players[consoleplayer].viewz != 1) 
	S_StartSound (mo, sfx_telept);	// don't start sound on first frame 
 
    return true; 
} 


//
// G_DeathMatchSpawnPlayer 
// Spawns a player at one of the random death match spots 
// called at level load and each death 
//
void G_DeathMatchSpawnPlayer (int playernum) 
{ 
    int             i,j; 
    int				selections; 
	 
    selections = deathmatch_p - deathmatchstarts; 
    if (selections < 4) 
	I_Error ("Only %i deathmatch spots, 4 required", selections); 
 
    for (j=0 ; j<20 ; j++) 
    { 
	i = P_Random() % selections; 
	if (G_CheckSpot (playernum, &deathmatchstarts[i]) ) 
	{ 
	    deathmatchstarts[i].type = playernum+1; 
	    P_SpawnPlayer (&deathmatchstarts[i]); 
	    return; 
	} 
    } 
 
    // no good spot, so the player will probably get stuck 
    P_SpawnPlayer (&playerstarts[playernum]); 
} 

// [crispy] clear the "savename" variable,
// i.e. restart level from scratch upon resurrection
static inline void G_ClearSavename ()
{
    M_StringCopy(savename, "", sizeof(savename));
}

//
// G_DoReborn 
// 
void G_DoReborn (int playernum) 
{ 
    int                             i; 
	 
    if (!netgame)
    {
	// [crispy] if the player dies and the game has been loaded or saved
	// in the mean time, reload that savegame instead of restarting the level
	// when "Run" is pressed upon resurrection
	if (crispy->singleplayer && *savename && speedkeydown())
	gameaction = ga_loadgame;
	else
	{
	// reload the level from scratch
	gameaction = ga_loadlevel;  
	G_ClearSavename();
	}
    }
    else 
    {
	// respawn at the start

	// first dissasociate the corpse 
	players[playernum].mo->player = NULL;   
		 
	// spawn at random spot if in death match 
	if (deathmatch) 
	{ 
	    G_DeathMatchSpawnPlayer (playernum); 
	    return; 
	} 
		 
	if (G_CheckSpot (playernum, &playerstarts[playernum]) ) 
	{ 
	    P_SpawnPlayer (&playerstarts[playernum]); 
	    return; 
	}
	
	// try to spawn at one of the other players spots 
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (G_CheckSpot (playernum, &playerstarts[i]) ) 
	    { 
		playerstarts[i].type = playernum+1;	// fake as other player 
		P_SpawnPlayer (&playerstarts[i]); 
		playerstarts[i].type = i+1;		// restore 
		return; 
	    }	    
	    // he's going to be inside something.  Too bad.
	}
	P_SpawnPlayer (&playerstarts[playernum]); 
    } 
} 
 
 
void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
} 
 


// DOOM Par Times
int pars[6][10] =
{ 
    {0}, 
    {0,30,75,120,90,165,180,180,30,165}, 
    {0,90,90,90,120,90,360,240,30,170}, 
    {0,90,45,90,150,90,90,165,30,135} 
    // [crispy] Episode 4 par times from the BFG Edition
   ,{0,165,255,135,150,180,390,135,360,180}
    // [crispy] Episode 5 par times from Sigil v1.21
   ,{0,90,150,360,420,780,420,780,300,660}
}; 

// DOOM II Par Times
int cpars[32] =
{
    30,90,120,120,90,150,120,120,270,90,	//  1-10
    210,150,150,150,210,150,420,150,210,150,	// 11-20
    240,150,180,150,150,300,330,420,300,180,	// 21-30
    120,30					// 31-32
};
 
// [crispy] No Rest For The Living par times from the BFG Edition
static int npars[9] =
{
    75,105,120,105,210,105,165,105,135
};

//
// G_DoCompleted 
//
boolean		secretexit; 
extern char*	pagename; 
 
void G_ExitLevel (void) 
{ 
    secretexit = false; 
    G_ClearSavename();
    gameaction = ga_completed; 
} 

// Here's for the german edition.
void G_SecretExitLevel (void) 
{ 
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if ( (gamemode == commercial)
      && (W_CheckNumForName("map31")<0))
	secretexit = false;
    else
	secretexit = true; 
    G_ClearSavename();
    gameaction = ga_completed; 
} 
 
void G_DoCompleted (void) 
{ 
    int             i; 
	 
    gameaction = ga_nothing; 
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
	if (playeringame[i]) 
	    G_PlayerFinishLevel (i);        // take away cards and stuff 
	 
    if (automapactive) 
	AM_Stop (); 
	
    if (gamemode != commercial)
    {
        // Chex Quest ends after 5 levels, rather than 8.

        if (gameversion == exe_chex)
        {
            if (gamemap == 5)
            {
                gameaction = ga_victory;
                return;
            }
        }
        else
        {
            switch(gamemap)
            {
            // [crispy] display tally screen after ExM8
            /*
              case 8:
                gameaction = ga_victory;
                return;
            */
              case 9: 
                for (i=0 ; i<MAXPLAYERS ; i++) 
                    players[i].didsecret = true; 
                break;
            }
        }
    }

// [crispy] disable redundant code
/*
//#if 0  Hmmm - why?
    if ( (gamemap == 8)
	 && (gamemode != commercial) ) 
    {
	// victory 
	gameaction = ga_victory; 
	return; 
    } 
	 
    if ( (gamemap == 9)
	 && (gamemode != commercial) ) 
    {
	// exit secret level 
	for (i=0 ; i<MAXPLAYERS ; i++) 
	    players[i].didsecret = true; 
    } 
//#endif
*/
    
	 
    wminfo.didsecret = players[consoleplayer].didsecret; 
    wminfo.epsd = gameepisode -1; 
    wminfo.last = gamemap -1;
    
    // wminfo.next is 0 biased, unlike gamemap
    if ( gamemission == pack_nerve && gamemap <= 9 && crispy->singleplayer )
    {
	if (secretexit)
	    switch(gamemap)
	    {
	      case  4: wminfo.next = 8; break;
	    }
	else
	    switch(gamemap)
	    {
	      case  9: wminfo.next = 4; break;
	      default: wminfo.next = gamemap;
	    }
    }
    else
    if ( gamemission == pack_master && gamemap <= 21 && crispy->singleplayer )
    {
	wminfo.next = gamemap;
    }
    else
    if ( gamemode == commercial)
    {
	if (secretexit)
	    if (gamemap == 2 && critical->havemap33)
	      wminfo.next = 32;
	    else
	    switch(gamemap)
	    {
	      case 15: wminfo.next = 30; break;
	      case 31: wminfo.next = 31; break;
	    }
	else
	    if (gamemap == 33 && critical->havemap33)
	      wminfo.next = 2;
	    else
	    switch(gamemap)
	    {
	      case 31:
	      case 32: wminfo.next = 15; break;
	      default: wminfo.next = gamemap;
	    }
    }
    else
    {
	if (secretexit) 
	{
	    if (critical->havee1m10 && gameepisode == 1 && gamemap == 1)
	    wminfo.next = 9; // [crispy] go to secret level E1M10 "Sewers"
	    else
	    wminfo.next = 8; 	// go to secret level 
	}
	else if (gamemap == 9) 
	{
	    // returning from secret level 
	    switch (gameepisode) 
	    { 
	      case 1: 
		wminfo.next = 3; 
		break; 
	      case 2: 
		wminfo.next = 5; 
		break; 
	      case 3: 
	      case 5: // [crispy] Sigil
		wminfo.next = 6; 
		break; 
	      case 4:
		wminfo.next = 2;
		break;
	    }                
	} 
	else
	if (critical->havee1m10 && gameepisode == 1 && gamemap == 10)
	    wminfo.next = 1; // [crispy] returning from secret level E1M10 "Sewers"
	else 
	    wminfo.next = gamemap;          // go to next level 
    }
		 
    wminfo.maxkills = totalkills; 
    wminfo.maxitems = totalitems; 
    wminfo.maxsecret = totalsecret; 
    wminfo.maxfrags = 0; 

    // Set par time. Exceptions are added for purposes of
    // statcheck regression testing.
    if (gamemode == commercial)
    {
        // map33 reads its par time from beyond the cpars[] array
        if (gamemap == 33)
        {
            int cpars32;

            memcpy(&cpars32, DEH_String(GAMMALVL0), sizeof(int));
            cpars32 = LONG(cpars32);

            wminfo.partime = TICRATE*cpars32;
        }
        // [crispy] support [PARS] sections in BEX files
        else if (bex_cpars[gamemap-1])
        {
            wminfo.partime = TICRATE*bex_cpars[gamemap-1];
        }
        // [crispy] single player par times for NRFTL
        else if (gamemission == pack_nerve && crispy->singleplayer)
        {
            wminfo.partime = TICRATE*npars[gamemap-1];
        }
        else
        {
            wminfo.partime = TICRATE*cpars[gamemap-1];
        }
    }
    // Doom episode 4 doesn't have a par time, so this
    // overflows into the cpars array.
    else if (gameepisode < 4 ||
        // [crispy] single player par times for episode 4
        (gameepisode == 4 && crispy->singleplayer) ||
        // [crispy] par times for Sigil
        gameepisode == 5)
    {
        // [crispy] support [PARS] sections in BEX files
        if (bex_pars[gameepisode][gamemap])
        {
            wminfo.partime = TICRATE*bex_pars[gameepisode][gamemap];
        }
        else
        wminfo.partime = TICRATE*pars[gameepisode][gamemap];
    }
    else
    {
        wminfo.partime = TICRATE*cpars[gamemap];
    }

    wminfo.pnum = consoleplayer; 
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
	wminfo.plyr[i].in = playeringame[i]; 
	wminfo.plyr[i].skills = players[i].killcount; 
	wminfo.plyr[i].sitems = players[i].itemcount; 
	wminfo.plyr[i].ssecret = players[i].secretcount; 
	wminfo.plyr[i].stime = leveltime; 
	memcpy (wminfo.plyr[i].frags, players[i].frags 
		, sizeof(wminfo.plyr[i].frags)); 
    } 
 
    // [crispy] CPhipps - total time for all completed levels
    // cph - modified so that only whole seconds are added to the totalleveltimes
    // value; so our total is compatible with the "naive" total of just adding
    // the times in seconds shown for each level. Also means our total time
    // will agree with Compet-n.
    wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime % TICRATE));

    gamestate = GS_INTERMISSION; 
    viewactive = false; 
    automapactive = false; 

    // [crispy] no statdump output for ExM8
    if (gamemode == commercial || gamemap != 8)
    {
    StatCopy(&wminfo);
    }
 
    WI_Start (&wminfo); 
} 


//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
    gameaction = ga_worlddone; 

    if (secretexit) 
      // [crispy] special-casing for E1M10 "Sewers" support
      // i.e. avoid drawing the splat for E1M9 already
      if (!crispy->havee1m10 || gameepisode != 1 || gamemap != 1)
	players[consoleplayer].didsecret = true; 

    if ( gamemission == pack_nerve && crispy->singleplayer )
    {
	switch (gamemap)
	{
	  case 8:
	    F_StartFinale ();
	    break;
	}
    }
    else
    if ( gamemission == pack_master && crispy->singleplayer )
    {
	switch (gamemap)
	{
	  case 20:
	    if (secretexit)
		break;
	  case 21:
	    F_StartFinale ();
	    break;
	}
    }
    else
    if ( gamemode == commercial )
    {
	switch (gamemap)
	{
	  case 15:
	  case 31:
	    if (!secretexit)
		break;
	  case 6:
	  case 11:
	  case 20:
	  case 30:
	    F_StartFinale ();
	    break;
	}
    }
    // [crispy] display tally screen after ExM8
    else
    if ( gamemap == 8 )
    {
	gameaction = ga_victory;
    }
} 
 
void G_DoWorldDone (void) 
{        
    gamestate = GS_LEVEL; 
    gamemap = wminfo.next+1; 
    G_DoLoadLevel (); 
    gameaction = ga_nothing; 
    viewactive = true; 
} 
 


//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//
extern boolean setsizeneeded;
void R_ExecuteSetViewSize (void);


void G_LoadGame (char* name) 
{ 
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame; 
} 

int savedleveltime = 0; // [crispy] moved here for level time logging
void G_DoLoadGame (void) 
{ 
	 
    // [crispy] loaded game must always be single player.
    // Needed for ability to use a further game loading, as well as
    // cheat codes and other single player only specifics.
    if (startloadgame == -1)
    {
	netdemo = false;
	netgame = false;
	deathmatch = false;
    }
    gameaction = ga_nothing; 
	 
    save_stream = fopen(savename, "rb");

    if (save_stream == NULL)
    {
        I_Error("Could not load savegame %s", savename);
    }

    // [crispy] read extended savegame data
    if (crispy->extsaveg)
    {
        // [crispy] first pass, read "savewadfilename"
        P_ReadExtendedSaveGameData(0);
    }
    // [crispy] check if WAD file is valid to restore saved map
    if (savewadfilename)
    {
        // [crispy] strings are not equal
        if (!savemaplumpinfo ||
            // [crispy] case-insensitive, so "doom.wad" matches "DOOM.WAD"
            strcasecmp(savewadfilename, W_WadNameForLump(savemaplumpinfo)))
        {
            M_ForceLoadGame();
            fclose(save_stream);
            return;
        }
        else
        // [crispy] strings are equal, but not identical
        if (savewadfilename != W_WadNameForLump(savemaplumpinfo))
        {
            free(savewadfilename);
        }
    }
    savewadfilename = NULL;

    savegame_error = false;

    if (!P_ReadSaveGameHeader())
    {
        // [crispy] indicate game version mismatch
        extern void M_LoadGameVerMismatch ();
        M_LoadGameVerMismatch();
        fclose(save_stream);
        return;
    }

    savedleveltime = leveltime;
    
    // load a base level 
    G_InitNew (gameskill, gameepisode, gamemap); 
 
    leveltime = savedleveltime;
    savedleveltime = 0;

    // dearchive all the modifications
    P_UnArchivePlayers (); 
    P_UnArchiveWorld (); 
    P_UnArchiveThinkers (); 
    P_UnArchiveSpecials (); 
    P_RestoreTargets (); // [crispy] restore mobj->target and mobj->tracer pointers
 
    if (!P_ReadSaveGameEOF())
	I_Error ("Bad savegame");

    // [crispy] read more extended savegame data
    if (crispy->extsaveg)
    {
        P_ReadExtendedSaveGameData(1);
    }

    fclose(save_stream);
    
    if (setsizeneeded)
	R_ExecuteSetViewSize ();
    
    // draw the pattern into the back screen
    R_FillBackScreen ();   

    // [crispy] if the player is dead in this savegame,
    // do not consider it for reload
    if (players[consoleplayer].health <= 0)
	G_ClearSavename();

    // [crisy] once loaded from the command line,
    // the next savegame will be loaded from the menu
    startloadgame = -1;
} 
 

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string 
//
void
G_SaveGame
( int	slot,
  char*	description )
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}

void G_DoSaveGame (void) 
{ 
    char *savegame_file;
    char *temp_savegame_file;
    char *recovery_savegame_file;

    recovery_savegame_file = NULL;
    temp_savegame_file = P_TempSaveGameFile();
    savegame_file = P_SaveGameFile(savegameslot);

    // Open the savegame file for writing.  We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by
    // a corrupted one, or if a savegame buffer overrun occurs.
    save_stream = fopen(temp_savegame_file, "wb");

    if (save_stream == NULL)
    {
        // Failed to save the game, so we're going to have to abort. But
        // to be nice, save to somewhere else before we call I_Error().
        recovery_savegame_file = M_TempFile("recovery.dsg");
        save_stream = fopen(recovery_savegame_file, "wb");
        if (save_stream == NULL)
        {
            I_Error("Failed to open either '%s' or '%s' to write savegame.",
                    temp_savegame_file, recovery_savegame_file);
        }
    }

    savegame_error = false;

    P_WriteSaveGameHeader(savedescription);

    // [crispy] some logging when saving
    {
	const int ltime = leveltime / TICRATE,
	          ttime = (totalleveltimes + leveltime) / TICRATE;
	extern const char *skilltable[];

	fprintf(stderr, "G_DoSaveGame: Episode %d, Map %d, %s, Time %d:%02d:%02d, Total %d:%02d:%02d.\n",
	        gameepisode, gamemap, skilltable[BETWEEN(0,5,(int) gameskill+1)],
	        ltime/3600, (ltime%3600)/60, ltime%60,
	        ttime/3600, (ttime%3600)/60, ttime%60);
    }

    P_ArchivePlayers ();
    P_ArchiveWorld ();
    P_ArchiveThinkers ();
    P_ArchiveSpecials ();

    P_WriteSaveGameEOF();
    // [crispy] write extended savegame data
    if (crispy->extsaveg)
    {
        P_WriteExtendedSaveGameData();
    }

    // [crispy] unconditionally disable savegame and demo limits
    /*
    // Enforce the same savegame size limit as in Vanilla Doom,
    // except if the vanilla_savegame_limit setting is turned off.

    if (vanilla_savegame_limit && ftell(save_stream) > SAVEGAMESIZE)
    {
        I_Error("Savegame buffer overrun");
    }
    */

    // Finish up, close the savegame file.

    fclose(save_stream);

    if (recovery_savegame_file != NULL)
    {
        // We failed to save to the normal location, but we wrote a
        // recovery file to the temp directory. Now we can bomb out
        // with an error.
        I_Error("Failed to open savegame file '%s' for writing.\n"
                "But your game has been saved to '%s' for recovery.",
                temp_savegame_file, recovery_savegame_file);
    }

    // Now rename the temporary savegame file to the actual savegame
    // file, overwriting the old savegame if there was one there.

    remove(savegame_file);
    rename(temp_savegame_file, savegame_file);

    gameaction = ga_nothing;
    M_StringCopy(savedescription, "", sizeof(savedescription));
    M_StringCopy(savename, savegame_file, sizeof(savename));

    players[consoleplayer].message = DEH_String(GGSAVED);

    // draw the pattern into the back screen
    R_FillBackScreen ();
}
 

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
//
skill_t	d_skill; 
int     d_episode; 
int     d_map; 
 
void
G_DeferedInitNew
( skill_t	skill,
  int		episode,
  int		map) 
{ 
    d_skill = skill; 
    d_episode = episode; 
    d_map = map; 
    G_ClearSavename();
    gameaction = ga_newgame; 

    // [crispy] if a new game is started during demo recording, start a new demo
    if (demorecording)
    {
	G_CheckDemoStatus();
	Z_Free(demoname);

	G_RecordDemo(orig_demoname);
	G_BeginRecording();
    }
} 


void G_DoNewGame (void) 
{
    demoplayback = false; 
    netdemo = false;
    netgame = false;
    deathmatch = false;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    // [crispy] do not reset -respawn, -fast and -nomonsters parameters
    /*
    respawnparm = false;
    fastparm = false;
    nomonsters = false;
    */
    consoleplayer = 0;
    G_InitNew (d_skill, d_episode, d_map); 
    gameaction = ga_nothing; 
} 


void
G_InitNew
( skill_t	skill,
  int		episode,
  int		map )
{
    const char *skytexturename;
    int             i;
    // [crispy] make sure "fast" parameters are really only applied once
    static boolean fast_applied;

    if (paused)
    {
	paused = false;
	S_ResumeSound ();
    }

    /*
    // Note: This commented-out block of code was added at some point
    // between the DOS version(s) and the Doom source release. It isn't
    // found in disassemblies of the DOS version and causes IDCLEV and
    // the -warp command line parameter to behave differently.
    // This is left here for posterity.

    // This was quite messy with SPECIAL and commented parts.
    // Supposedly hacks to make the latest edition work.
    // It might not work properly.
    if (episode < 1)
      episode = 1;

    if ( gamemode == retail )
    {
      if (episode > 4)
	episode = 4;
    }
    else if ( gamemode == shareware )
    {
      if (episode > 1)
	   episode = 1;	// only start episode 1 on shareware
    }
    else
    {
      if (episode > 3)
	episode = 3;
    }
    */

    if (skill > sk_nightmare)
	skill = sk_nightmare;

  // [crispy] if NRFTL is not found, episode 2 means The Master Levels
  if (gamemode == commercial && episode == 2 && !nervewadfile)
  {
    episode = masterlevelsfile ? 3 : 0;
  }

  // [crispy] only fix episode/map if it doesn't exist
  if (P_GetNumForMap(episode, map, false) < 0)
  {
    if (gameversion >= exe_ultimate)
    {
        if (episode == 0)
        {
            episode = 4;
        }
    }
    else
    {
        if (episode < 1)
        {
            episode = 1;
        }
        if (episode > 3)
        {
            episode = 3;
        }
    }

    if (episode > 1 && gamemode == shareware)
    {
        episode = 1;
    }

    if (map < 1)
	map = 1;

    if ( (map > 9)
	 && ( gamemode != commercial) )
    {
      // [crispy] support E1M10 "Sewers"
      if (!crispy->havee1m10 || episode != 1)
      map = 9;
      else
      map = 10;
    }
  }

    M_ClearRandom ();

    if (skill == sk_nightmare || respawnparm )
	respawnmonsters = true;
    else
	respawnmonsters = false;

    // [crispy] make sure "fast" parameters are really only applied once
    if ((fastparm || skill == sk_nightmare) && !fast_applied)
    {
	for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
	    // [crispy] Fix infinite loop caused by Demon speed bug
	    if (states[i].tics > 1)
	    {
	    states[i].tics >>= 1;
	    }
	mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
	mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
	mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
	fast_applied = true;
    }
    else if (!fastparm && skill != sk_nightmare && fast_applied)
    {
	for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
	    states[i].tics <<= 1;
	mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
	mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
	mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
	fast_applied = false;
    }

    // force players to be initialized upon first level load
    for (i=0 ; i<MAXPLAYERS ; i++)
	players[i].playerstate = PST_REBORN;

    usergame = true;                // will be set false if a demo
    paused = false;
    demoplayback = false;
    automapactive = false;
    viewactive = true;
    gameepisode = episode;
    gamemap = map;
    gameskill = skill;

    // [crispy] CPhipps - total time for all completed levels
    totalleveltimes = 0;
    defdemotics = 0;
    demostarttic = gametic; // [crispy] fix revenant internal demo bug

    viewactive = true;

    // Set the sky to use.
    //
    // Note: This IS broken, but it is how Vanilla Doom behaves.
    // See http://doomwiki.org/wiki/Sky_never_changes_in_Doom_II.
    //
    // Because we set the sky here at the start of a game, not at the
    // start of a level, the sky texture never changes unless we
    // restore from a saved game.  This was fixed before the Doom
    // source release, but this IS the way Vanilla DOS Doom behaves.

    if (gamemode == commercial)
    {
        skytexturename = DEH_String("SKY3");
        skytexture = R_TextureNumForName(skytexturename);
        if (gamemap < 21)
        {
            skytexturename = DEH_String(gamemap < 12 ? "SKY1" : "SKY2");
            skytexture = R_TextureNumForName(skytexturename);
        }
    }
    else
    {
        switch (gameepisode)
        {
          default:
          case 1:
            skytexturename = "SKY1";
            break;
          case 2:
            skytexturename = "SKY2";
            break;
          case 3:
            skytexturename = "SKY3";
            break;
          case 4:        // Special Edition sky
            skytexturename = "SKY4";
            break;
          case 5:        // [crispy] Sigil
            skytexturename = "SKY5_ZD";
            if (R_CheckTextureNumForName(DEH_String(skytexturename)) == -1)
            {
                skytexturename = "SKY3";
            }
            break;
        }
        skytexturename = DEH_String(skytexturename);
        skytexture = R_TextureNumForName(skytexturename);
    }

    G_DoLoadLevel ();
}


//
// DEMO RECORDING 
// 
#define DEMOMARKER		0x80

// [crispy] demo progress bar and timer widget
int defdemotics = 0, deftotaldemotics;
// [crispy] moved here
static const char *defdemoname;

void G_ReadDemoTiccmd (ticcmd_t* cmd) 
{ 
    if (*demo_p == DEMOMARKER) 
    {
	// end of demo data stream 
	G_CheckDemoStatus (); 
	return; 
    } 

    // [crispy] if demo playback is quit by pressing 'q',
    // stay in the game, hand over controls to the player and
    // continue recording the demo under a different name
    if (gamekeydown[key_demo_quit] && singledemo && !netgame)
    {
	byte *actualbuffer = demobuffer;
	char *actualname = M_StringDuplicate(defdemoname);

	gamekeydown[key_demo_quit] = false;

	// [crispy] find a new name for the continued demo
	G_RecordDemo(actualname);
	free(actualname);

	// [crispy] discard the newly allocated demo buffer
	Z_Free(demobuffer);
	demobuffer = actualbuffer;

	// [crispy] continue recording
	G_CheckDemoStatus();
	return;
    }

    cmd->forwardmove = ((signed char)*demo_p++); 
    cmd->sidemove = ((signed char)*demo_p++); 

    // If this is a longtics demo, read back in higher resolution

    if (longtics)
    {
        cmd->angleturn = *demo_p++;
        cmd->angleturn |= (*demo_p++) << 8;
    }
    else
    {
        cmd->angleturn = ((unsigned char) *demo_p++)<<8; 
    }

    cmd->buttons = (unsigned char)*demo_p++; 

    // [crispy] increase demo tics counter
    // applies to both recording and playback,
    // because G_WriteDemoTiccmd() calls G_ReadDemoTiccmd() once
    defdemotics++;
} 

// Increase the size of the demo buffer to allow unlimited demos

static void IncreaseDemoBuffer(void)
{
    int current_length;
    byte *new_demobuffer;
    byte *new_demop;
    int new_length;

    // Find the current size

    current_length = demoend - demobuffer;
    
    // Generate a new buffer twice the size
    new_length = current_length * 2;
    
    new_demobuffer = Z_Malloc(new_length, PU_STATIC, 0);
    new_demop = new_demobuffer + (demo_p - demobuffer);

    // Copy over the old data

    memcpy(new_demobuffer, demobuffer, current_length);

    // Free the old buffer and point the demo pointers at the new buffer.

    Z_Free(demobuffer);

    demobuffer = new_demobuffer;
    demo_p = new_demop;
    demoend = demobuffer + new_length;
}

void G_WriteDemoTiccmd (ticcmd_t* cmd) 
{ 
    byte *demo_start;

    if (gamekeydown[key_demo_quit])           // press q to end demo recording 
	G_CheckDemoStatus (); 

    demo_start = demo_p;

    *demo_p++ = cmd->forwardmove; 
    *demo_p++ = cmd->sidemove; 

    // If this is a longtics demo, record in higher resolution
 
    if (longtics)
    {
        *demo_p++ = (cmd->angleturn & 0xff);
        *demo_p++ = (cmd->angleturn >> 8) & 0xff;
    }
    else
    {
        *demo_p++ = cmd->angleturn >> 8; 
    }

    *demo_p++ = cmd->buttons; 

    // reset demo pointer back
    demo_p = demo_start;

    if (demo_p > demoend - 16)
    {
        // [crispy] unconditionally disable savegame and demo limits
        /*
        if (vanilla_demo_limit)
        {
            // no more space 
            G_CheckDemoStatus (); 
            return; 
        }
        else
        */
        {
            // Vanilla demo limit disabled: unlimited
            // demo lengths!

            IncreaseDemoBuffer();
        }
    } 
	
    G_ReadDemoTiccmd (cmd);         // make SURE it is exactly the same 
} 
 
 
 
//
// G_RecordDemo
//
void G_RecordDemo (char *name)
{
    size_t demoname_size;
    int i;
    int maxsize;

    // [crispy] demo file name suffix counter
    static unsigned int j = 0;
    FILE *fp = NULL;

    // [crispy] the name originally chosen for the demo, i.e. without "-00000"
    if (!orig_demoname)
    {
	orig_demoname = name;
    }

    usergame = false;
    demoname_size = strlen(name) + 5 + 6; // [crispy] + 6 for "-00000"
    demoname = Z_Malloc(demoname_size, PU_STATIC, NULL);
    M_snprintf(demoname, demoname_size, "%s.lmp", name);

    // [crispy] prevent overriding demos by adding a file name suffix
    for ( ; j <= 99999 && (fp = fopen(demoname, "rb")) != NULL; j++)
    {
	M_snprintf(demoname, demoname_size, "%s-%05d.lmp", name, j);
	fclose (fp);
    }

    maxsize = 0x20000;

    //!
    // @arg <size>
    // @category demo
    // @vanilla
    //
    // Specify the demo buffer size (KiB)
    //

    i = M_CheckParmWithArgs("-maxdemo", 1);
    if (i)
	maxsize = atoi(myargv[i+1])*1024;
    demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL); 
    demoend = demobuffer + maxsize;
	
    demorecording = true; 
} 

// Get the demo version code appropriate for the version set in gameversion.
int G_VanillaVersionCode(void)
{
    switch (gameversion)
    {
        case exe_doom_1_666:
            return 106;
        case exe_doom_1_7:
            return 107;
        case exe_doom_1_8:
            return 108;
        case exe_doom_1_9:
        default:  // All other versions are variants on v1.9:
            return 109;
    }
}

void G_BeginRecording (void) 
{ 
    int             i; 

    demo_p = demobuffer;

    //!
    // @category demo
    //
    // Record a high resolution "Doom 1.91" demo.
    //

    longtics = D_NonVanillaRecord(M_ParmExists("-longtics"),
                                  "Doom 1.91 demo format");

    // If not recording a longtics demo, record in low res
    lowres_turn = !longtics;

    if (longtics)
    {
        *demo_p++ = DOOM_191_VERSION;
    }
    else if (gameversion > exe_doom_1_2)
    {
        *demo_p++ = G_VanillaVersionCode();
    }

    *demo_p++ = gameskill; 
    *demo_p++ = gameepisode; 
    *demo_p++ = gamemap; 
    if (longtics || gameversion > exe_doom_1_2)
    {
        *demo_p++ = deathmatch; 
        *demo_p++ = respawnparm;
        *demo_p++ = fastparm;
        *demo_p++ = nomonsters;
        *demo_p++ = consoleplayer;
    }
	 
    for (i=0 ; i<MAXPLAYERS ; i++) 
	*demo_p++ = playeringame[i]; 		 
} 
 

//
// G_PlayDemo 
//

 
void G_DeferedPlayDemo(const char *name)
{ 
    defdemoname = name; 
    gameaction = ga_playdemo; 

    // [crispy] fast-forward demo up to the desired map
    // in demo warp mode or to the end of the demo in continue mode
    if (crispy->demowarp || demorecording)
    {
	nodrawers = true;
	singletics = true;
    }
} 

// Generate a string describing a demo version

static const char *DemoVersionDescription(int version)
{
    static char resultbuf[16];

    switch (version)
    {
        case 104:
            return "v1.4";
        case 105:
            return "v1.5";
        case 106:
            return "v1.6/v1.666";
        case 107:
            return "v1.7/v1.7a";
        case 108:
            return "v1.8";
        case 109:
            return "v1.9";
        case 111:
            return "v1.91 hack demo?";
        default:
            break;
    }

    // Unknown version.  Perhaps this is a pre-v1.4 IWAD?  If the version
    // byte is in the range 0-4 then it can be a v1.0-v1.2 demo.

    if (version >= 0 && version <= 4)
    {
        return "v1.0/v1.1/v1.2";
    }
    else
    {
        M_snprintf(resultbuf, sizeof(resultbuf),
                   "%i.%i (unknown)", version / 100, version % 100);
        return resultbuf;
    }
}

void G_DoPlayDemo (void)
{
    skill_t skill;
    int i, lumpnum, episode, map;
    int demoversion;
    boolean olddemo = false;
    int lumplength; // [crispy]

    // [crispy] in demo continue mode free the obsolete demo buffer
    // of size 'maxsize' previously allocated in G_RecordDemo()
    if (demorecording)
    {
        Z_Free(demobuffer);
    }

    lumpnum = W_GetNumForName(defdemoname);
    gameaction = ga_nothing;
    demobuffer = W_CacheLumpNum(lumpnum, PU_STATIC);
    demo_p = demobuffer;

    // [crispy] ignore empty demo lumps
    lumplength = W_LumpLength(lumpnum);
    if (lumplength < 0xd)
    {
	demoplayback = true;
	G_CheckDemoStatus();
	return;
    }

    demoversion = *demo_p++;

    if (demoversion >= 0 && demoversion <= 4)
    {
        olddemo = true;
        demo_p--;
    }

    longtics = false;

    // Longtics demos use the modified format that is generated by cph's
    // hacked "v1.91" doom exe. This is a non-vanilla extension.
    if (D_NonVanillaPlayback(demoversion == DOOM_191_VERSION, lumpnum,
                             "Doom 1.91 demo format"))
    {
        longtics = true;
    }
    else if (demoversion != G_VanillaVersionCode() &&
             !(gameversion <= exe_doom_1_2 && olddemo))
    {
        const char *message = "Demo is from a different game version!\n"
                              "(read %i, should be %i)\n"
                              "\n"
                              "*** You may need to upgrade your version "
                                  "of Doom to v1.9. ***\n"
                              "    See: https://www.doomworld.com/classicdoom"
                                        "/info/patches.php\n"
                              "    This appears to be %s.";

        if (singledemo)
        I_Error(message, demoversion, G_VanillaVersionCode(),
                         DemoVersionDescription(demoversion));
        // [crispy] make non-fatal
        else
        {
        fprintf(stderr, message, demoversion, G_VanillaVersionCode(),
                         DemoVersionDescription(demoversion));
	fprintf(stderr, "\n");
	demoplayback = true;
	G_CheckDemoStatus();
	return;
        }
    }

    skill = *demo_p++; 
    episode = *demo_p++; 
    map = *demo_p++; 
    if (!olddemo)
    {
        deathmatch = *demo_p++;
        respawnparm = *demo_p++;
        fastparm = *demo_p++;
        nomonsters = *demo_p++;
        consoleplayer = *demo_p++;
    }
    else
    {
        deathmatch = 0;
        respawnparm = 0;
        fastparm = 0;
        nomonsters = 0;
        consoleplayer = 0;
    }
    
        
    for (i=0 ; i<MAXPLAYERS ; i++) 
	playeringame[i] = *demo_p++; 

    if (playeringame[1] || M_CheckParm("-solo-net") > 0
                        || M_CheckParm("-netdemo") > 0)
    {
	netgame = true;
	netdemo = true;
	// [crispy] impossible to continue a multiplayer demo
	demorecording = false;
    }

    // don't spend a lot of time in loadlevel 
    precache = false;
    // [crispy] support playing demos from savegames
    if (startloadgame >= 0)
    {
	M_StringCopy(savename, P_SaveGameFile(startloadgame), sizeof(savename));
	G_DoLoadGame();
    }
    else
    {
    G_InitNew (skill, episode, map); 
    }
    precache = true; 
    starttime = I_GetTime (); 
    demostarttic = gametic; // [crispy] fix revenant internal demo bug

    usergame = false; 
    demoplayback = true; 
    // [crispy] update the "singleplayer" variable
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);

    // [crispy] demo progress bar
    {
	int i, numplayersingame = 0;
	byte *demo_ptr = demo_p;

	for (i = 0; i < MAXPLAYERS; i++)
	{
	    if (playeringame[i])
	    {
		numplayersingame++;
	    }
	}

	deftotaldemotics = defdemotics = 0;

	while (*demo_ptr != DEMOMARKER && (demo_ptr - demobuffer) < lumplength)
	{
	    demo_ptr += numplayersingame * (longtics ? 5 : 4);
	    deftotaldemotics++;
	}
    }
} 

//
// G_TimeDemo 
//
void G_TimeDemo (char* name) 
{
    //!
    // @category video
    // @vanilla
    //
    // Disable rendering the screen entirely.
    //

    nodrawers = M_CheckParm ("-nodraw");

    timingdemo = true; 
    singletics = true; 

    defdemoname = name; 
    gameaction = ga_playdemo; 
} 
 
 
/* 
=================== 
= 
= G_CheckDemoStatus 
= 
= Called after a death or level completion to allow demos to be cleaned up 
= Returns true if a new demo loop action will take place 
=================== 
*/ 
 
boolean G_CheckDemoStatus (void) 
{ 
    int             endtime; 
	 
    if (timingdemo) 
    { 
        float fps;
        int realtics;

	endtime = I_GetTime (); 
        realtics = endtime - starttime;
        fps = ((float) gametic * TICRATE) / realtics;

        // Prevent recursive calls
        timingdemo = false;
        demoplayback = false;

	I_Error ("timed %i gametics in %i realtics (%f fps)",
                 gametic, realtics, fps);
    } 
	 
    if (demoplayback) 
    { 
        W_ReleaseLumpName(defdemoname);
	demoplayback = false; 
	netdemo = false;
	netgame = false;
	deathmatch = false;
	playeringame[1] = playeringame[2] = playeringame[3] = 0;
	// [crispy] leave game parameters intact when continuing a demo
	if (!demorecording)
	{
	respawnparm = false;
	fastparm = false;
	nomonsters = false;
	}
	consoleplayer = 0;
        
        // [crispy] in demo continue mode increase the demo buffer and
        // continue recording once we are done with playback
        if (demorecording)
        {
            demoend = demo_p;
            IncreaseDemoBuffer();

            nodrawers = false;
            singletics = false;

            // [crispy] start music for the current level
            if (gamestate == GS_LEVEL)
            {
                S_Start();
            }

            return true;
        }

        if (singledemo) 
            I_Quit (); 
        else 
            D_AdvanceDemo (); 

	return true; 
    } 
 
    if (demorecording) 
    { 
	*demo_p++ = DEMOMARKER; 
	M_WriteFile (demoname, demobuffer, demo_p - demobuffer); 
	Z_Free (demobuffer); 
	demorecording = false; 
	// [crispy] if a new game is started during demo recording, start a new demo
	if (gameaction != ga_newgame)
	{
	I_Error ("Demo %s recorded",demoname); 
	}
	else
	{
	    fprintf(stderr, "Demo %s recorded\n",demoname);
	}
    } 
	 
    return false; 
} 
 
 
 
