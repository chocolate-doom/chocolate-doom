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
#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_saves.h" // STRIFE
#include "m_random.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"
#include "d_main.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

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

#include "p_dialog.h"   // villsa [STRIFE]

#include "g_game.h"


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
void	G_DoSaveGame (char *path); 
 
// Gamestate the last time G_Ticker was called.

gamestate_t     oldgamestate; 
 
gameaction_t    gameaction; 
gamestate_t     gamestate; 
skill_t         gameskill = 2; // [STRIFE] Default value set to 2.
boolean         respawnmonsters;
//int             gameepisode; 
int             gamemap;

// haleyjd 08/24/10: [STRIFE] New variables
int             destmap;   // current destination map when exiting
int             riftdest;  // destination spot for player
angle_t         riftangle; // player angle saved during exit

// If non-zero, exit the level after this number of minutes.

int             timelimit;

boolean         paused; 
boolean         sendpause;              // send a pause event next tic 
boolean         sendsave;               // send a save event next tic 
boolean         usergame;               // ok to save / end game 
 
boolean         timingdemo;             // if true, exit with report on completion 
boolean         nodrawers;              // for comparative timing purposes 
int             starttime;              // for comparative timing purposes 
 
boolean         viewactive; 
 
int             deathmatch;             // only if started as net death 
boolean         netgame;                // only true if packets are broadcast 
boolean         playeringame[MAXPLAYERS]; 
player_t        players[MAXPLAYERS]; 

boolean         turbodetected[MAXPLAYERS];
 
int             consoleplayer;          // player taking events and displaying 
int             displayplayer;          // view being displayed 
int             levelstarttic;          // gametic at level start 
int             totalkills, /*totalitems,*/ totalsecret;    // for intermission 
 
char           *demoname;
boolean         demorecording; 
boolean         longtics;               // cph's doom 1.91 longtics hack
boolean         lowres_turn;            // low resolution turning for longtics
boolean         demoplayback; 
boolean		netdemo; 
byte*		demobuffer;
byte*		demo_p;
byte*		demoend; 
boolean         singledemo;             // quit after playing a demo from cmdline 
 
boolean         precache = true;        // if true, load all graphics at start 

boolean         testcontrols = false;    // Invoked by setup to test controls
 
wbstartstruct_t wminfo;                 // parms for world map / intermission 
 
byte            consistancy[MAXPLAYERS][BACKUPTICS]; 
 
#define MAXPLMOVE		(forwardmove[1]) 
 
#define TURBOTHRESHOLD	0x32

fixed_t         forwardmove[2] = {0x19, 0x32}; 
fixed_t         sidemove[2] = {0x18, 0x28}; 
fixed_t         angleturn[3] = {640, 1280, 320};    // + slow turn 

int mouse_fire_countdown = 0;    // villsa [STRIFE]

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
// STRIFE-TODO: Check this table makes sense.

static const struct
{
    weapontype_t weapon;
    weapontype_t weapon_num;
} weapon_order_table[] = {
    { wp_fist,                  wp_fist },
    { wp_poisonbow,             wp_elecbow },
    { wp_elecbow,               wp_elecbow },
    { wp_rifle,                 wp_rifle },
    { wp_missile,               wp_missile },
    { wp_wpgrenade,             wp_hegrenade },
    { wp_hegrenade,             wp_hegrenade },
    { wp_flame,                 wp_flame },
    { wp_torpedo,               wp_mauler },
    { wp_mauler,                wp_mauler },
    { wp_sigil,                 wp_sigil },
};

#define SLOWTURNTICS	6 
 
#define NUMKEYS		256 
#define MAX_JOY_BUTTONS 20

static boolean  gamekeydown[NUMKEYS]; 
static int      turnheld;		// for accelerative turning 
 
static boolean  mousearray[MAX_MOUSE_BUTTONS + 1];
static boolean *mousebuttons = &mousearray[1];  // allow [-1]

// mouse values are used once 
int             mousex;
int             mousey;         

// [crispy] for rounding error
typedef struct carry_s
{
    double angle;
    double pitch;
    double side;
    double vert;
} carry_t;

static carry_t prevcarry;
static carry_t carry;

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
static int      joylook;
static boolean  joyarray[MAX_JOY_BUTTONS + 1]; 
static boolean *joybuttons = &joyarray[1];		// allow [-1] 
 
static int      savegameslot = 6; // [STRIFE] initialized to 6
static char     savedescription[32]; 
 
static ticcmd_t basecmd; // [crispy]

int      testcontrols_mousespeed;
 
#define	BODYQUESIZE	32

mobj_t*		bodyque[BODYQUESIZE]; 
//int       bodyqueslot; [STRIFE] unused
 
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
    player_t *player;

    player = &players[consoleplayer];

    // Can't select a weapon if we don't own it.

    if (!player->weaponowned[weapon])
    {
        return false;
    }

    // Can't use registered-only weapons in demo mode:

    if (isdemoversion && !weaponinfo[weapon].availabledemo)
    {
        return false;
    }

    // Special rules for switching to alternate versions of weapons.
    // These must match the weapon-switching rules in P_PlayerThink()

    // haleyjd 20141024: same fix here as in P_PlayerThink for torpedo.

    if (weapon == wp_torpedo
     && player->ammo[weaponinfo[wp_torpedo].ammo] < 30)
    {
        return false;
    }

    if (player->ammo[weaponinfo[weapon].ammo] == 0)
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

    // The current weapon must be present in weapon_order_table
    // otherwise something has gone terribly wrong
    if (i >= arrlen(weapon_order_table)) {
        I_Error("Internal error: weapon %d not present in weapon_order_table", weapon);
    }

    // Switch weapon.
    start_i = i;
    do
    {
        i += direction;
        i = (i + arrlen(weapon_order_table)) % arrlen(weapon_order_table);
    } while (i != start_i && !WeaponSelectable(weapon_order_table[i].weapon));

    return weapon_order_table[i].weapon_num;
}

// [crispy] holding down the "Run" key may trigger special behavior,
// e.g. quick exit, automap pan/zoom speed
boolean speedkeydown (void)
{
    return (key_speed < NUMKEYS && gamekeydown[key_speed]) ||
           (joybspeed < MAX_JOY_BUTTONS && joybuttons[joybspeed]) ||
           (mousebspeed < MAX_MOUSE_BUTTONS && mousebuttons[mousebspeed]);
}

// [crispy] for carrying rounding error
static int CarryError(double value, const double *prevcarry, double *carry)
{
    const double desired = value + *prevcarry;
    const int actual = lround(desired);
    *carry = desired - actual;

    return actual;
}

static short CarryAngle(double angle)
{
    if (lowres_turn && abs(angle + prevcarry.angle) < 128)
    {
        carry.angle = angle + prevcarry.angle;
        return 0;
    }
    else
    {
        return CarryError(angle, &prevcarry.angle, &carry.angle);
    }
}

static short CarryPitch(double pitch)
{
    return CarryError(pitch, &prevcarry.pitch, &carry.pitch);
}

static int CarryMouseVert(double vert)
{
    return CarryError(vert, &prevcarry.vert, &carry.vert);
}

static int CarryMouseSide(double side)
{
    const double desired = side + prevcarry.side;
    const int actual = lround(side * 0.5) * 2; // Even values only.
    carry.side = desired - actual;
    return actual;
}

static double CalcMouseAngle(int mousex)
{
    if (!mouseSensitivity)
        return 0.0;

    return (I_AccelerateMouse(mousex) * (mouseSensitivity + 5) * 8 / 10);
}

static double CalcMouseSide(int mousex)
{
    if (!mouseSensitivity_x2)
        return 0.0;

    return (I_AccelerateMouse(mousex) * (mouseSensitivity_x2 + 5) * 2 / 10);
}

static double CalcMouseVert(int mousey)
{
    if (!mouseSensitivity_y)
        return 0.0;

    return (I_AccelerateMouseY(mousey) * (mouseSensitivity_y + 5) / 10);
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
    int		angle = 0; // [crispy]
    int		mousex_angleturn; // [crispy]
    int		forward;
    int		side;
    player_t *const player = &players[consoleplayer]; // [crispy]
    static char playermessage[48]; // [crispy]
    static unsigned int mbmlookctrl = 0; // [crispy]
    static unsigned int kbdlookctrl = 0; // [crispy]

    // [crispy] For fast polling.
    G_PrepTiccmd();
    memcpy(cmd, &basecmd, sizeof(*cmd));
    memset(&basecmd, 0, sizeof(ticcmd_t));

    cmd->consistancy = 
        consistancy[consoleplayer][maketic%BACKUPTICS]; 

    // villsa [STRIFE] look up/down keys
    // [crispy] center view key and lookspring support
    if (crispy->freelook_hh == FREELOOK_HH_SPRING)
    {
        if (gamekeydown[key_lookup] || (joylook < 0 && joystick_look_sensitivity))
        {
            cmd->buttons2 |= BT2_LOOKUP;
            kbdlookctrl += ticdup;
        }
        else if (gamekeydown[key_lookdown] || (joylook > 0 && joystick_look_sensitivity))
        {
            cmd->buttons2 |= BT2_LOOKDOWN;
            kbdlookctrl += ticdup;
        }
        else if (gamekeydown[key_lookcenter] || kbdlookctrl)
        {
            cmd->buttons2 |= BT2_CENTERVIEW;
            if (!player->mo->reactiontime) // teleport delay
                kbdlookctrl = 0;
        }
    }
    else
    {
        if (gamekeydown[key_lookup] || (joylook < 0 && joystick_look_sensitivity))
            cmd->buttons2 |= BT2_LOOKUP;

        if (gamekeydown[key_lookdown] || (joylook > 0 && joystick_look_sensitivity))
            cmd->buttons2 |= BT2_LOOKDOWN;

        if (gamekeydown[key_lookcenter])
            cmd->buttons2 |= BT2_CENTERVIEW;
    }

    // villsa [STRIFE] inventory use key
    // [crispy] mouse inventory use
    if(gamekeydown[key_invuse] || mousebuttons[mousebinvuse])
    {
        if(player->numinventory > 0)
        {
            cmd->buttons2 |= BT2_INVUSE;
            cmd->inventory = player->inventory[player->inventorycursor].sprite;

            // [crispy] Crispy HUD: keep inventory visible when using an item
            if (st_invtics)
                st_invtics = 5 * 35;
        }
    }

    // villsa [STRIFE] inventory drop key
    if(gamekeydown[key_invdrop])
    {
        if(player->numinventory > 0)
        {
            cmd->buttons2 |= BT2_INVDROP;
            cmd->inventory = player->inventory[player->inventorycursor].sprite;

            // [crispy] Crispy HUD: keep inventory visible when dropping an item
            if (st_invtics)
                st_invtics = 5 * 35;
        }
    }

    // villsa [STRIFE] use medkit
    if(gamekeydown[key_usehealth])
        cmd->buttons2 |= BT2_HEALTH;


 
    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe] 
        || joybuttons[joybstrafe]; 

    // fraggle: support the old "joyb_speed = 31" hack which
    // allowed an autorun effect

    // [crispy] when "always run" is active,
    // pressing the "run" key will result in walking
    speed = (key_speed >= NUMKEYS
         || joybspeed >= MAX_JOY_BUTTONS);
    speed ^= speedkeydown();
 
    forward = side = 0;

    // villsa [STRIFE] running causes centerview to occur
    // [crispy] decouple run from centerview
    if(speed && runcentering)
        cmd->buttons2 |= BT2_CENTERVIEW;

    // villsa [STRIFE] disable running if low on health
    if (player->health <= 15)
        speed = 0;
    
    // use two stage accelerative turning
    // on the keyboard and joystick
    if (joyxmove < 0
        || joyxmove > 0  
        || gamekeydown[key_right]
        || gamekeydown[key_left]
        || mousebuttons[mousebturnright]
        || mousebuttons[mousebturnleft])
        turnheld += ticdup; 
    else 
        turnheld = 0; 

    if (turnheld < SLOWTURNTICS) 
        tspeed = 2;             // slow turn 
    else 
        tspeed = speed;

    // [crispy] add quick 180° reverse
    if (gamekeydown[key_reverse] || mousebuttons[mousebreverse])
    {
        angle += ANG180 >> FRACBITS;
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

        M_snprintf(playermessage, sizeof(playermessage),
                   "Always Run %s",
                   (joybspeed >= MAX_JOY_BUTTONS) ? "On" : "Off");
        player->message = playermessage;
        S_StartSound(NULL, sfx_swtchn);

        gamekeydown[key_toggleautorun] = false;
    }

    // [crispy] toggle vertical mouse movement
    if (gamekeydown[key_togglenovert])
    {
        novert = !novert;

        M_snprintf(playermessage, sizeof(playermessage),
                   "Vertical Mouse Movement %s",
                   !novert ? "On" : "Off");
        player->message = playermessage;
        S_StartSound(NULL, sfx_swtchn);

        gamekeydown[key_togglenovert] = false;
    }

    // let movement keys cancel each other out
    if (strafe) 
    { 
        if (!cmd->angleturn)
        {
            if (gamekeydown[key_right] || mousebuttons[mousebturnright])
            {
                // fprintf(stderr, "strafe right\n");
                side += sidemove[speed];
            }
            if (gamekeydown[key_left] || mousebuttons[mousebturnleft])
            {
                //	fprintf(stderr, "strafe left\n");
                side -= sidemove[speed];
            }
            if (use_analog && joyxmove)
            {
                joyxmove = joyxmove * joystick_move_sensitivity / 10;
                joyxmove = BETWEEN(-FRACUNIT, FRACUNIT, joyxmove);
                side += FixedMul(sidemove[speed], joyxmove);
            }
            else if (joystick_move_sensitivity)
            {
                if (joyxmove > 0)
                    side += sidemove[speed];
                if (joyxmove < 0)
                    side -= sidemove[speed];
            }
        }
    } 
    else 
    { 
        if (gamekeydown[key_right] || mousebuttons[mousebturnright])
            angle -= angleturn[tspeed];
        if (gamekeydown[key_left] || mousebuttons[mousebturnleft])
            angle += angleturn[tspeed];
        if (use_analog && joyxmove)
        {
            // Cubic response curve allows for finer control when stick
            // deflection is small.
            joyxmove = FixedMul(FixedMul(joyxmove, joyxmove), joyxmove);
            joyxmove = joyxmove * joystick_turn_sensitivity / 10;
            angle -= FixedMul(angleturn[1], joyxmove);
        }
        else if (joystick_turn_sensitivity)
        {
            if (joyxmove > 0)
                angle -= angleturn[tspeed];
            if (joyxmove < 0)
                angle += angleturn[tspeed];
        }
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

    if (use_analog && joyymove)
    {
        joyymove = joyymove * joystick_move_sensitivity / 10;
        joyymove = BETWEEN(-FRACUNIT, FRACUNIT, joyymove);
        forward -= FixedMul(forwardmove[speed], joyymove);
    }
    else if (joystick_move_sensitivity)
    {
        if (joyymove < 0)
            forward += forwardmove[speed];
        if (joyymove > 0)
            forward -= forwardmove[speed];
    }

    if (gamekeydown[key_strafeleft] || gamekeydown[key_alt_strafeleft] // [crispy] add key_alt_*
     || joybuttons[joybstrafeleft]
     || mousebuttons[mousebstrafeleft])
    {
        side -= sidemove[speed];
    }

    if (gamekeydown[key_straferight] || gamekeydown[key_alt_straferight] // [crispy] add key_alt_*
     || joybuttons[joybstraferight]
     || mousebuttons[mousebstraferight])
    {
        side += sidemove[speed]; 
    }

    if (use_analog && joystrafemove)
    {
        joystrafemove = joystrafemove * joystick_move_sensitivity / 10;
        joystrafemove = BETWEEN(-FRACUNIT, FRACUNIT, joystrafemove);
        side += FixedMul(sidemove[speed], joystrafemove);
    }
    else if (joystick_move_sensitivity)
    {
        if (joystrafemove < 0)
            side -= sidemove[speed];
        if (joystrafemove > 0)
            side += sidemove[speed];
    }
    // buttons
    cmd->chatchar = HU_dequeueChatChar(); 

    // villsa [STRIFE] - add mouse button support for jump
    if (gamekeydown[key_jump] || mousebuttons[mousebjump]
     || joybuttons[joybjump])
        cmd->buttons2 |= BT2_JUMP;
 
    // villsa [STRIFE]: Moved mousebuttons[mousebfire] to below
    if (gamekeydown[key_fire] || joybuttons[joybfire]) 
        cmd->buttons |= BT_ATTACK;

    // villsa [STRIFE]
    if(mousebuttons[mousebfire])
    {
         if(mouse_fire_countdown <= 0)
             cmd->buttons |= BT_ATTACK;
         else
             --mouse_fire_countdown;
    }
 
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
    if (cmd->lookdir && crispy->singleplayer)
    {
        static fixed_t carry_lookdir = 0;
        fixed_t desired_lookdir;

        // [crispy] repurpose lookdir and carry error like low-res turning
        desired_lookdir = FixedDiv(cmd->lookdir << FRACBITS, MLOOKUNIT << FRACBITS) +
                          carry_lookdir;
        cmd->lookdir = desired_lookdir >> FRACBITS;
        carry_lookdir = desired_lookdir - (cmd->lookdir << FRACBITS);
    }
    else if (!novert)
    {
        forward += CarryMouseVert(CalcMouseVert(mousey));
    }

    // [crispy] single click on mouse look button centers view
    if (mousebuttons[mousebmouselook]) // [crispy] clicked
    {
        mbmlookctrl += ticdup;
    }
    else if (mbmlookctrl) // [crispy] released
    {
        if (crispy->freelook_hh == FREELOOK_HH_SPRING ||
            mbmlookctrl < SLOWTURNTICS) // [crispy] short click
        {
            cmd->buttons2 |= BT2_CENTERVIEW;
        }

        if (!player->mo->reactiontime) // [crispy] teleport delay
            mbmlookctrl = 0;
    }

    if (strafe && !cmd->angleturn)
        side += CarryMouseSide(CalcMouseSide(mousex));

    mousex_angleturn = cmd->angleturn;

    if (mousex_angleturn == 0)
    {
        // No movement in the previous frame

        testcontrols_mousespeed = 0;
    }
    
    if (angle)
    {
        cmd->angleturn = CarryAngle(cmd->angleturn + angle);
        localview.ticangleturn = cmd->angleturn - mousex_angleturn;
    }

    mousex = mousey = 0;

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
    
    // [crispy]
    localview.angle = 0;
    localview.rawangle = 0.0;
    prevcarry = carry;

    // special buttons
    if (sendpause) 
    { 
        sendpause = false; 
        cmd->buttons = BT_SPECIAL | BTS_PAUSE; 
    } 
 
    if (sendsave) 
    { 
        sendsave = false; 
        cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT); 
    } 

    // low-res turning

    if (lowres_turn)
    {
        signed short desired_angleturn;

        desired_angleturn = cmd->angleturn;

        // round angleturn to the nearest 256 unit boundary
        // for recording demos with single byte values for turn

        cmd->angleturn = (desired_angleturn + 128) & 0xff00;

        if (angle)
        {
            localview.ticangleturn = cmd->angleturn - mousex_angleturn;
        }

        // Carry forward the error from the reduced resolution to the
        // next tic, so that successive small movements can accumulate.

        prevcarry.angle += desired_angleturn - cmd->angleturn;
    }
} 
 

//
// G_DoLoadLevel 
//
void G_DoLoadLevel (void) 
{ 
    int             i; 

    // haleyjd 10/03/10: [STRIFE] This is not done here.
    //skyflatnum = R_FlatNumForName(DEH_String(SKYFLATNAME));

    levelstarttic = gametic;        // for time calculation

    if (wipegamestate == GS_LEVEL) 
        wipegamestate = -1;             // force a wipe 

    gamestate = GS_LEVEL; 

    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
        turbodetected[i] = false;

        // haleyjd 20110204 [STRIFE]: PST_REBORN if players[i].health <= 0
        if (playeringame[i] && (players[i].playerstate == PST_DEAD || players[i].health <= 0))
            players[i].playerstate = PST_REBORN; 
        memset (players[i].frags,0,sizeof(players[i].frags)); 
    } 

    // [crispy] update the "singleplayer" variable
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);

    // [crispy] double ammo
    if (crispy->moreammo && !crispy->singleplayer)
    {
        const char message[] = "The -doubleammo option is not supported"
                               " for demos and\n"
                               " network play.";
        if (!demo_p) demorecording = false;
        I_Error(message);
    }

    P_SetupLevel (gamemap, 0, gameskill);    
    displayplayer = consoleplayer;      // view the guy you are playing    
    starttime = I_GetTime(); // haleyjd 20110204 [STRIFE]
    gameaction = ga_nothing; 
    Z_CheckHeap ();

    // clear cmd building stuff

    memset (gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joystrafemove = joylook = 0;
    mousex = mousey = 0;
    memset(&localview, 0, sizeof(localview)); // [crispy]
    memset(&carry, 0, sizeof(carry)); // [crispy]
    memset(&prevcarry, 0, sizeof(prevcarry)); // [crispy]
    memset(&basecmd, 0, sizeof(basecmd)); // [crispy]
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));
    memset(joyarray, 0, sizeof(joyarray));

    if (testcontrols)
    {
        players[consoleplayer].message = "Press escape to quit.";
    }

    P_DialogLoad(); // villsa [STRIFE]
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
    player_t *const player = &players[consoleplayer]; // [crispy]

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
            else if (i == mousebinvleft) // [crispy] mouse inventory left
            {
                if (player->inventorycursor > 0)
                    player->inventorycursor--;
                st_invtics = 5 * 35; // [crispy] Crispy HUD
            }
            else if (i == mousebinvright) // [crispy] mouse inventory right
            {
                if (player->inventorycursor < player->numinventory - 1)
                    player->inventorycursor++;
                st_invtics = 5 * 35; // [crispy] Crispy HUD
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
        && ev->data1 == key_spy && (singledemo || !gameskill) ) // [STRIFE]: o_O
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
            if(devparm && ev->data1 == 'g')
                D_PageTicker(); // [STRIFE]: wat? o_O
            else
            {
                // [crispy] play a sound if the menu is activated with a different key than ESC
                if (!menuactive && crispy->soundfix)
                    S_StartSound(NULL, sfx_swtchn);

                M_StartControlPanel (); 
            }
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
        mousex += ev->data2;
        mousey += ev->data3;
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

// [crispy] For fast polling.
void G_FastResponder (void)
{
    if (newfastmouse)
    {
        mousex += fastmouse.data2;
        mousey += fastmouse.data3;

        newfastmouse = false;
    }
}

// [crispy]
void G_PrepTiccmd (void)
{
    const boolean strafe = gamekeydown[key_strafe] ||
        mousebuttons[mousebstrafe] || joybuttons[joybstrafe];

    if (mousex && !strafe)
    {
        localview.rawangle -= CalcMouseAngle(mousex);
        basecmd.angleturn = CarryAngle(localview.rawangle);
        localview.angle = basecmd.angleturn << 16;
        mousex = 0;
    }

    if (mousey && (crispy->mouselook || mousebuttons[mousebmouselook]))
    {
        const double vert = CalcMouseVert(mousey);
        basecmd.lookdir += mouse_y_invert ?
                            CarryPitch(-vert): CarryPitch(vert);
        mousey = 0;
    }
}

// [crispy] take a screenshot after rendering the next frame
static void G_CrispyScreenShot()
{
    // [crispy] increase screenshot filename limit
    V_ScreenShot("STRIFE%04i.%s"); // [STRIFE] file name, message
    players[consoleplayer].message = DEH_String("STRIFE  by Rogue entertainment");
    crispy->cleanscreenshot = 0;
    crispy->screenshotmsg = 2;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void) 
{ 
    int         i;
    int         buf; 
    ticcmd_t*   cmd;

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
            G_DoNewGame (); 
            break; 
        case ga_loadgame: 
            G_DoLoadGame(true); 
            M_SaveMoveHereToMap(); // [STRIFE]
            M_ReadMisObj();
            break; 
        case ga_savegame: 
            M_SaveMoveMapToHere(); // [STRIFE]
            M_SaveMisObj(savepath);
            G_DoSaveGame(savepath); 
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

    // STRIFE-TODO: pnameprefixes bullcrap

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i]) 
        { 
            cmd = &players[i].cmd; 

            memcpy (cmd, &netcmds[i], sizeof(ticcmd_t)); 

            if (demoplayback)
                G_ReadDemoTiccmd (cmd); 
            if (demorecording)
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
                        S_ResumeSound (); 
                    break; 

                case BTS_SAVEGAME: 
                    if (!character_name[0]) // [STRIFE]
                    {
                        M_StringCopy(character_name, "NET GAME",
                                     sizeof(character_name));
                    }
                    savegameslot =  
                        (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
                    gameaction = ga_savegame; 
                    break; 
                } 
            } 
        }
    }

    // Have we just finished displaying an intermission screen?
    // haleyjd 08/23/10: [STRIFE] No intermission.
    /*
    if (oldgamestate == GS_INTERMISSION && gamestate != GS_INTERMISSION)
    {
        WI_End();
    }
    */

    oldgamestate = gamestate;
    oldleveltime = leveltime; // [crispy] Track if game is running

    // do main actions
    switch (gamestate) 
    { 
    case GS_LEVEL: 
        P_Ticker (); 
        ST_Ticker (); 
        AM_Ticker (); 
        HU_Ticker ();
        break; 

        // haleyjd 08/23/10: [STRIFE] No intermission.
        /*
    case GS_INTERMISSION: 
        WI_Ticker (); 
        break; 
        */
    case GS_UNKNOWN: // STRIFE-TODO: What is this? is it ever used??
        F_WaitTicker();
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
// [STRIFE] No such function.
/*
void G_InitPlayer (int player) 
{ 
    player_t*	p; 
 
    // set up the saved info         
    p = &players[player]; 
	 
    // clear everything else to defaults 
    G_PlayerReborn (player); 
	 
} 
*/
 

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
// [STRIFE] No such function. The equivalent to this logic was moved into
// G_DoCompleted.
/*
void G_PlayerFinishLevel (int player) 
{ 
    player_t*	p; 
	 
    p = &players[player]; 
	 
    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    p->mo->flags &= ~MF_SHADOW;		// cancel invisibility 
    p->extralight = 0;			// cancel gun flashes 
    p->fixedcolormap = 0;		// cancel ir gogles 
    p->damagecount = 0;			// no palette changes 
    p->bonuscount = 0; 
} 
*/

//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
//
// [STRIFE] Small changes for allegiance, inventory, health auto-use, and
// mission objective.
//
void G_PlayerReborn (int player) 
{ 
    player_t*   p; 
    int         i; 
    int         frags[MAXPLAYERS]; 
    int         killcount;
    int         allegiance;

    killcount = players[player].killcount;
    allegiance = players[player].allegiance; // [STRIFE]

    memcpy(frags,players[player].frags,sizeof(frags));

    p = &players[player]; 
    memset (p, 0, sizeof(*p)); 

    memcpy(p->frags, frags, sizeof(p->frags));

    p->usedown               = true;                 // don't do anything immediately
    p->attackdown            = true;
    p->inventorydown         = true;                 // villsa [STRIFE]
    p->playerstate           = PST_LIVE;       
    p->health                = deh_initial_health;   // Use dehacked value
    p->readyweapon           = wp_fist;              // villsa [STRIFE] default to fists
    p->pendingweapon         = wp_fist;              // villsa [STRIFE] default to fists
    p->weaponowned[wp_fist]  = true;                 // villsa [STRIFE] default to fists
    p->cheats               |= CF_AUTOHEALTH;        // villsa [STRIFE]
    p->killcount             = killcount;
    p->allegiance            = allegiance;           // villsa [STRIFE]
    p->centerview            = true;                 // villsa [STRIFE]

    for(i = 0; i < NUMAMMO; i++) 
        p->maxammo[i] = maxammo[i]; 

    // [STRIFE] clear inventory
    for(i = 0; i < 32; i++)
        p->inventory[i].type = NUMMOBJTYPES;

    // villsa [STRIFE]: Default objective
    M_StringCopy(mission_objective, DEH_String("Find help"),
                 OBJECTIVE_LEN);
}

//
// G_CheckSpot  
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
//
// [STRIFE] Changed to eliminate body queue and an odd error message was added.
//
void P_SpawnPlayer (mapthing_t* mthing); 
 
boolean
G_CheckSpot
( int		playernum,
  mapthing_t*	mthing ) 
{ 
    fixed_t             x;
    fixed_t             y; 
    subsector_t*        ss; 
    unsigned            an; 
    mobj_t*             mo; 
    int                 i;

    if (!players[playernum].mo)
    {
        // [STRIFE] weird error message added here:
        if(leveltime > 0)
            players[playernum].message = DEH_String("you didn't have a body!");

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
    // [STRIFE] player corpses remove themselves after a short time, so
    // evidently this wasn't needed.
    /*
    if (bodyqueslot >= BODYQUESIZE) 
        P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]); 
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
    bodyqueslot++; 
    */

    // spawn a teleport fog 
    ss = R_PointInSubsector (x,y); 
    an = ( ANG45 * (((unsigned int) mthing->angle)/45) ) >> ANGLETOFINESHIFT; 

    mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an] 
                      , ss->sector->floorheight 
                      , MT_TFOG); 

    if (players[consoleplayer].viewz != 1) 
        S_StartSound (mo, sfx_telept);	// don't start sound on first frame 

    return true; 
} 


//
// G_DeathMatchSpawnPlayer 
// Spawns a player at one of the random death match spots 
// called at level load and each death 
//
// [STRIFE]: Modified exit message to match binary.
//
void G_DeathMatchSpawnPlayer (int playernum) 
{ 
    int             i,j; 
    int             selections; 

    selections = deathmatch_p - deathmatchstarts; 
    if (selections < 4) 
        I_Error ("Only %i deathmatch spots, at least 4 required!", selections); 

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

//
// G_LoadPath
//
// haleyjd 20101003: [STRIFE] New function
// Sets loadpath based on the map and "savepathtemp"
//
void G_LoadPath(int map)
{
    char mapbuf[33];

    memset(mapbuf, 0, sizeof(mapbuf));
    M_snprintf(mapbuf, sizeof(mapbuf), "%d", map);

    // haleyjd: free if already set, and use M_SafeFilePath
    if(loadpath)
        Z_Free(loadpath);
    loadpath = M_SafeFilePath(savepathtemp, mapbuf);
}

//
// G_DoReborn
// 
void G_DoReborn (int playernum) 
{ 
    int                             i; 

    if (!netgame)
    {
        // reload the level from scratch
        // [STRIFE] Reborn level load
        G_LoadPath(gamemap);
        gameaction = ga_loadgame;
    }
    else 
    {
        // respawn at the start

        // first dissasociate the corpse 
        // [STRIFE] Checks for NULL first
        if(players[playernum].mo)
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
                playerstarts[i].type = playernum+1;     // fake as other player 
                P_SpawnPlayer (&playerstarts[i]);
                playerstarts[i].type = i+1;             // restore 
                return; 
            }
            // he's going to be inside something.  Too bad.
        }
        P_SpawnPlayer (&playerstarts[playernum]); 
    } 
} 
 
//
// G_ScreenShot
//
// [STRIFE] Verified unmodified
//
void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
} 

// haleyjd 20100823: [STRIFE] Removed par times.

//
// G_DoCompleted 
//

//
// G_RiftExitLevel
//
// haleyjd 20100824: [STRIFE] New function
// * Called from some exit linedefs to exit to a specific riftspot in the 
//   given destination map.
//
void G_RiftExitLevel(int map, int spot, angle_t angle)
{
    gameaction = ga_completed;
    
    // special handling for post-Sigil map changes
    if(players[0].weaponowned[wp_sigil])
    {
        if(map == 3) // Front Base -> Abandoned Front Base
            map = 30;
        if(map == 7) // Castle -> New Front Base
            map = 10;
    }

    // no rifting in deathmatch games
    if(deathmatch)
        spot = 0;

    riftangle = angle;
    riftdest  = spot;
    destmap   = map;
}

//
// G_Exit2
//
// haleyjd 20101003: [STRIFE] New function.
// No xrefs to this, doesn't seem to be used. Could have gotten inlined
// somewhere but I haven't seen it.
//
void G_Exit2(int dest, angle_t angle)
{
    riftdest = dest;
    gameaction = ga_completed;
    riftangle = angle;
    destmap = gamemap;
}

//
// G_ExitLevel
//
// haleyjd 20100824: [STRIFE]:
// * Default to next map in numeric order; init destmap and riftdest.
//
void G_ExitLevel (int dest) 
{
    if(dest == 0)
        dest = gamemap + 1;
    destmap = dest;
    riftdest = 0;
    gameaction = ga_completed; 
} 

/*
// haleyjd 20100823: [STRIFE] No secret exits in Strife.
// Here's for the german edition.
void G_SecretExitLevel (void) 
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if ( (gamemode == commercial)
        && (W_CheckNumForName("map31")<0))
        secretexit = false;
    else
        secretexit = true; 
    gameaction = ga_completed; 
}
*/

//
// G_StartFinale
//
// haleyjd 20100921: [STRIFE] New function.
// This replaced G_SecretExitLevel in Strife. I don't know that it's actually
// used anywhere in the game, but it *is* usable in mods via linetype 124,
// W1 Start Finale.
//
void G_StartFinale(void)
{
    gameaction = ga_victory;
}

//
// G_DoCompleted
//
// haleyjd 20100823: [STRIFE]:
// * Removed G_PlayerFinishLevel and just sets some powerup states.
// * Removed Chex, as not relevant to Strife.
// * Removed DOOM level transfer logic 
// * Removed intermission code.
// * Added setting gameaction to ga_worlddone.
//
void G_DoCompleted (void) 
{
    int i;

    // deal with powerup states
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i])
        {
            // [STRIFE] restore pw_allmap power from mapstate cache
            if(destmap < 40)
                players[i].powers[pw_allmap] = players[i].mapstate[destmap];

            // Shadowarmor doesn't persist between maps in netgames
            if(netgame)
                players[i].powers[pw_invisibility] = 0;
        }
    }

    stonecold = false;  // villsa [STRIFE]

    if (automapactive) 
        AM_Stop (); 

    // [STRIFE] HUB SAVE
    if(!deathmatch)
        G_DoSaveGame(savepathtemp);
    
    gameaction = ga_worlddone;
} 


// haleyjd 20100824: [STRIFE] No secret exits.
/*
//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
    gameaction = ga_worlddone; 

    if (secretexit) 
        players[consoleplayer].didsecret = true; 

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
} 
*/

//
// G_RiftPlayer
//
// haleyjd 20100824: [STRIFE] New function
// Teleports the player to the appropriate rift spot.
//
void G_RiftPlayer(void)
{
    if(riftdest)
    {
        P_TeleportMove(players[0].mo, 
                       riftSpots[riftdest - 1].x << FRACBITS, 
                       riftSpots[riftdest - 1].y << FRACBITS);
        players[0].mo->angle  = riftangle;
        players[0].mo->health = players[0].health;
    }
}

//
// G_RiftCheat
//
// haleyjd 20100824: [STRIFE] New function
// Called from the cheat code to jump to a rift spot.
//
boolean G_RiftCheat(int riftSpotNum)
{
    return P_TeleportMove(players[0].mo,
                          riftSpots[riftSpotNum - 1].x << FRACBITS,
                          riftSpots[riftSpotNum - 1].y << FRACBITS);
}

//
// G_DoWorldDone
//
// haleyjd 20100824: [STRIFE] Added destmap -> gamemap set.
//
void G_DoWorldDone (void) 
{        
    int temp_leveltime = leveltime;
    boolean temp_shadow = false;
    boolean temp_mvis   = false;

    gamestate = GS_LEVEL; 
    gamemap = destmap;

    // [STRIFE] HUB LOAD
    G_LoadPath(destmap);
    if (!deathmatch)
    {
        // Remember Shadowarmor across hub loads
        if(players[0].mo->flags & MF_SHADOW)
            temp_shadow = true;
        if(players[0].mo->flags & MF_MVIS)
            temp_mvis = true;
    }
    G_DoLoadGame(false);

    // [STRIFE] leveltime carries over between maps
    leveltime = temp_leveltime;

    if(!deathmatch)
    {
        // [STRIFE]: transfer saved powerups
        players[0].mo->flags &= ~(MF_SHADOW|MF_MVIS);
        if(temp_shadow)
            players[0].mo->flags |= MF_SHADOW;
        if(temp_mvis)
            players[0].mo->flags |= MF_MVIS;

        // [STRIFE] HUB SAVE
        G_RiftPlayer();
        G_DoSaveGame(savepathtemp);
        M_SaveMisObj(savepathtemp);
    }

    gameaction = ga_nothing; 
    viewactive = true; 
} 

//
// G_DoWorldDone2
//
// haleyjd 20101003: [STRIFE] New function. No xrefs; unused.
//
void G_DoWorldDone2(void)
{
    gamestate = GS_LEVEL;
    gameaction = ga_nothing;
    viewactive = true;
}

//
// G_ReadCurrent
//
// haleyjd 20101003: [STRIFE] New function.
// Reads the "CURRENT" file from the given path and then sets it to
// gamemap.
//
void G_ReadCurrent(const char *path)
{
    char *temppath = NULL;
    byte *buffer = NULL;

    temppath = M_SafeFilePath(path, "\\current");

    if(M_ReadFile(temppath, &buffer) <= 0)
        gameaction = ga_newgame;
    else
    {
        // haleyjd 20110211: do endian-correct read
        gamemap = (((int)buffer[0])       |
                   ((int)buffer[1] <<  8) |
                   ((int)buffer[2] << 16) |
                   ((int)buffer[3] << 24));
        gameaction = ga_loadgame;
        Z_Free(buffer);
    }

    Z_Free(temppath);
    
    G_LoadPath(gamemap);
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//

char	savename[256];

// [STRIFE]: No such function.
/*
void G_LoadGame (char* name)
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame;
}
*/

// haleyjd 20100928: [STRIFE] VERSIONSIZE == 8
#define VERSIONSIZE             8

void G_DoLoadGame (boolean userload) 
{
    int savedleveltime;
    skill_t currentskill; // [crispy]
    skill_t skill; // [crispy]

    gameaction = ga_nothing;

    save_stream = M_fopen(loadpath, "rb");

    // [STRIFE] If the file does not exist, G_DoLoadLevel is called.
    if (save_stream == NULL)
    {
        G_DoLoadLevel();
        return;
    }

    savegame_error = false;

    // [crispy] save current gameskill before calling P_ReadSaveGameHeader()
    currentskill = gameskill;

    if (!P_ReadSaveGameHeader())
    {
        fclose(save_stream);
        return;
    }

    // [crispy] fix skill and gameskill checks in G_InitNew() (haleyjd)
    skill = gameskill;
    gameskill = currentskill;

    // haleyjd: A comment would be good here, fraggle...
    // Evidently this is a Choco-ism, necessitated by reading the savegame
    // header *before* calling G_DoLoadLevel.
    savedleveltime = leveltime;
    
    // load a base level

    // STRIFE-TODO: ????
    if(userload)
        G_InitNew(skill, gamemap); // [crispy] use skill
    else
        G_DoLoadLevel();
 
    leveltime = savedleveltime;

    // dearchive all the modifications
    // [STRIFE] some portions of player_t are not overwritten when loading
    //   between hub levels
    P_UnArchivePlayers (userload); 
    P_UnArchiveWorld (); 
    P_UnArchiveThinkers (); 
    P_UnArchiveSpecials (); 
    P_RestoreTargets (); // [crispy] restore mobj->target and mobj->tracer pointers
 
    if (!P_ReadSaveGameEOF())
        I_Error ("Bad savegame");

    fclose(save_stream);
    
    if (setsizeneeded)
        R_ExecuteSetViewSize ();
    
    // draw the pattern into the back screen
    R_FillBackScreen ();
} 

//
// G_WriteSaveName
//
// haleyjd 20101003: [STRIFE] New function
//
// Writes the character name to the NAME file.
//
boolean G_WriteSaveName(int slot, const char *charname)
{
    //char savedir[16];
    char *tmpname;
    boolean retval;

    savegameslot = slot;

    // haleyjd: removed special -cdrom treatment, as I believe it is taken
    // care of automatically via using Choco's savegamedir setting.

    // haleyjd: free previous path, if any, and allocate new one using
    // M_SafeFilePath routine, which isn't limited to 128 characters.
    if(savepathtemp)
        Z_Free(savepathtemp);
    savepathtemp = M_SafeFilePath(savegamedir, "strfsav6.ssg");

    // haleyjd: as above.
    if(savepath)
        Z_Free(savepath);
    savepath = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(savegameslot, ""));

    // haleyjd: memset full character_name for safety
    memset(character_name, 0, CHARACTER_NAME_LEN);
    M_StringCopy(character_name, charname, sizeof(character_name));

    // haleyjd: use M_SafeFilePath
    tmpname = M_SafeFilePath(savepathtemp, "name");

    // Write the "name" file under the directory
    retval = M_WriteFile(tmpname, character_name, 32);

    Z_Free(tmpname);

    return retval;
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//
// [STRIFE] No such function, at least in v1.2
// STRIFE-TODO: Does this make a comeback in v1.31?
/*
void
G_SaveGame
( int	slot,
  char*	description )
{
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
}
*/

void G_DoSaveGame (char *path)
{ 
    char *current_path;
    char *savegame_file;
    char *temp_savegame_file;
    byte gamemapbytes[4];
    char gamemapstr[33];

    temp_savegame_file = P_TempSaveGameFile();
    
    // [STRIFE] custom save file path logic
    memset(gamemapstr, 0, sizeof(gamemapstr));
    M_snprintf(gamemapstr, sizeof(gamemapstr), "%d", gamemap);
    savegame_file = M_SafeFilePath(path, gamemapstr);

    // [STRIFE] write the "current" file, which tells which hub map
    //   the save slot is currently on.
    current_path = M_SafeFilePath(path, "current");
    // haleyjd: endian-agnostic IO
    gamemapbytes[0] = (byte)( gamemap        & 0xff);
    gamemapbytes[1] = (byte)((gamemap >>  8) & 0xff);
    gamemapbytes[2] = (byte)((gamemap >> 16) & 0xff);
    gamemapbytes[3] = (byte)((gamemap >> 24) & 0xff);
    M_WriteFile(current_path, gamemapbytes, 4);
    Z_Free(current_path);

    // Open the savegame file for writing.  We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by 
    // a corrupted one, or if a savegame buffer overrun occurs.

    save_stream = M_fopen(temp_savegame_file, "wb");

    if (save_stream == NULL)
    {
        return;
    }

    savegame_error = false;

    P_WriteSaveGameHeader(savedescription);
 
    P_ArchivePlayers (); 
    P_ArchiveWorld (); 
    P_ArchiveThinkers (); 
    P_ArchiveSpecials (); 

    P_WriteSaveGameEOF();

    // [crispy] unconditionally disable savegame and demo limits
    /*
    // Enforce the same savegame size limit as in Vanilla Doom, 
    // except if the vanilla_savegame_limit setting is turned off.
    // [STRIFE]: Verified subject to same limit.

    if (vanilla_savegame_limit && ftell(save_stream) > SAVEGAMESIZE)
    {
        I_Error ("Savegame buffer overrun");
    }
    */
    
    // Finish up, close the savegame file.

    fclose(save_stream);

    // Now rename the temporary savegame file to the actual savegame
    // file, overwriting the old savegame if there was one there.

    M_remove(savegame_file);
    M_rename(temp_savegame_file, savegame_file);
    
    // haleyjd: free the savegame_file path
    Z_Free(savegame_file);

    gameaction = ga_nothing; 
    //M_StringCopy(savedescription, "", sizeof(savedescription));

    // [STRIFE]: custom message logic
    if(!strcmp(path, savepath))
    {
        M_snprintf(savename, sizeof(savename), "%s saved.", character_name);
        players[consoleplayer].message = savename;
    }

    // draw the pattern into the back screen
    R_FillBackScreen ();
} 
 

//
skill_t d_skill; 
//int     d_episode; [STRIFE] No such thing as episodes in Strife
int     d_map; 

//
// G_DeferedInitNew
//
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
//
// haleyjd 20100922: [STRIFE] Removed episode parameter
//
void G_DeferedInitNew(skill_t skill, int map)
{ 
    d_skill = skill; 
    d_map = map; 
    gameaction = ga_newgame; 
} 

//
// G_DoNewGame
//
// [STRIFE] Code added to turn off the stonecold effect.
//   Someone also removed the nomonsters reset...
//
void G_DoNewGame (void) 
{
    demoplayback = false; 
    netdemo = false;
    netgame = false;
    deathmatch = false;
    playeringame[1] = playeringame[2] = playeringame[3] = 0;
    respawnparm = false;
    fastparm = false;
    stonecold = false;      // villsa [STRIFE]
    //nomonsters = false;   [STRIFE] not set here!?!
    consoleplayer = 0;
    G_InitNew (d_skill, d_map);
    gameaction = ga_nothing; 
} 

//
// G_InitNew
//
// haleyjd 20100824: [STRIFE]:
// * Added riftdest initialization
// * Removed episode parameter
//
void
G_InitNew
( skill_t       skill,
  int           map ) 
{ 
    const char *skytexturename;
    int             i; 

    if (paused) 
    { 
        paused = false; 
        S_ResumeSound (); 
    } 

    if (skill > sk_nightmare) 
        skill = sk_nightmare;

    // [STRIFE] Removed episode nonsense and gamemap clipping

    M_ClearRandom (); 

    if (skill == sk_nightmare || respawnparm )
        respawnmonsters = true;
    else
        respawnmonsters = false;

    // [STRIFE] Strife skill level mobjinfo/states tweaking
    // BUG: None of this code runs properly when loading save games, so
    // basically it's impossible to play any skill level properly unless
    // you never quit and reload from the command line.
    // [crispy] fixed in G_DoLoadGame()
    if(!skill && gameskill)
    {
        // Setting to Baby skill... make things easier.

        // Acolytes walk, attack, and feel pain slower
        for(i = S_AGRD_13; i <= S_AGRD_23; i++)
            states[i].tics *= 2; 

        // Reavers attack slower
        for(i = S_ROB1_10; i <= S_ROB1_15; i++)
            states[i].tics *= 2;

        // Turrets attack slower
        for(i = S_TURT_02; i <= S_TURT_03; i++)
            states[i].tics *= 2;

        // Crusaders attack and feel pain slower
        for(i = S_ROB2_09; i <= S_ROB2_19; i++)
            states[i].tics *= 2;

        // Stalkers think, walk, and attack slower
        for(i = S_SPID_03; i <= S_SPID_10; i++)
            states[i].tics *= 2;

        // The Bishop's homing missiles are faster (what?? BUG?)
        mobjinfo[MT_SEEKMISSILE].speed *= 2;
    }
    if(skill && !gameskill)
    {
        // Setting a higher skill when previously on baby... make things normal

        // Acolytes
        for(i = S_AGRD_13; i <= S_AGRD_23; i++)
            states[i].tics >>= 1; 

        // Reavers
        for(i = S_ROB1_10; i <= S_ROB1_15; i++)
            states[i].tics >>= 1;

        // Turrets
        for(i = S_TURT_02; i <= S_TURT_03; i++)
            states[i].tics >>= 1;

        // Crusaders
        for(i = S_ROB2_09; i <= S_ROB2_19; i++)
            states[i].tics >>= 1;

        // Stalkers
        for(i = S_SPID_03; i <= S_SPID_10; i++)
            states[i].tics >>= 1;

        // The Bishop's homing missiles - again, seemingly backward.
        mobjinfo[MT_SEEKMISSILE].speed >>= 1;
    }
    if(fastparm || (skill == sk_nightmare && skill != gameskill))
    {
        // BLOODBATH! Make some things super-aggressive.
        
        // Acolytes walk, attack, and feel pain twice as fast
        // (This makes just getting out of the first room almost impossible)
        for(i = S_AGRD_13; i <= S_AGRD_23; i++)
            states[i].tics >>= 1;

        // Bishop's homing missiles again get SLOWER and not faster o_O
        mobjinfo[MT_SEEKMISSILE].speed >>= 1;
    }
    else if(skill != sk_nightmare && gameskill == sk_nightmare)
    {
        // Setting back to an ordinary skill after being on Bloodbath?
        // Put stuff back to normal.

        // Acolytes
        for(i = S_AGRD_13; i <= S_AGRD_23; i++)
            states[i].tics *= 2;

        // Bishop's homing missiles
        mobjinfo[MT_SEEKMISSILE].speed *= 2;
    }

    // force players to be initialized upon first level load
    for (i=0 ; i<MAXPLAYERS ; i++) 
        players[i].playerstate = PST_REBORN; 

    usergame = true;                // will be set false if a demo 
    paused = false; 
    demoplayback = false; 
    automapactive = false; 
    viewactive = true; 
    //gameepisode = episode; [STRIFE] no episodes
    gamemap = map; 
    gameskill = skill; 
    riftdest = 0; // haleyjd 08/24/10: [STRIFE] init riftdest to zero on new game

    // Set the sky to use.
    //
    // Note: This IS broken, but it is how Vanilla Doom behaves.
    // See http://doomwiki.org/wiki/Sky_never_changes_in_Doom_II.
    //
    // Because we set the sky here at the start of a game, not at the
    // start of a level, the sky texture never changes unless we
    // restore from a saved game.  This was fixed before the Doom
    // source release, but this IS the way Vanilla DOS Doom behaves.

    // [STRIFE] Strife skies (of which there are but two)
    if(gamemap >= 9 && gamemap < 32)
        skytexturename = "skymnt01";
    else
        skytexturename = "skymnt02";

    skytexturename = DEH_String(skytexturename);

    skytexture = R_TextureNumForName(skytexturename);

    // [STRIFE] HUBS
    G_LoadPath(gamemap);
    G_DoLoadLevel();
} 
 

//
// DEMO RECORDING 
// 
#define DEMOMARKER		0x80

//
// G_ReadDemoTiccmd
//
// [STRIFE] Modified for Strife ticcmd_t
//
void G_ReadDemoTiccmd (ticcmd_t* cmd) 
{ 
    if (*demo_p == DEMOMARKER) 
    {
        // end of demo data stream 
        G_CheckDemoStatus (); 
        return; 
    }
    cmd->forwardmove = ((signed char)*demo_p++); 
    cmd->sidemove = ((signed char)*demo_p++); 
    cmd->angleturn = ((unsigned char) *demo_p++)<<8; 
    cmd->buttons = (unsigned char)*demo_p++; 
    cmd->buttons2 = (unsigned char)*demo_p++; // [STRIFE]
    cmd->inventory = (int)*demo_p++;          // [STRIFE]
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

//
// G_WriteDemoTiccmd
//
// [STRIFE] Modified for Strife ticcmd_t.
//
void G_WriteDemoTiccmd (ticcmd_t* cmd) 
{ 
    byte *demo_start;

    if (gamekeydown[key_demo_quit])           // press q to end demo recording 
        G_CheckDemoStatus (); 

    demo_start = demo_p;

    *demo_p++ = cmd->forwardmove; 
    *demo_p++ = cmd->sidemove; 
    *demo_p++ = cmd->angleturn >> 8; 
    *demo_p++ = cmd->buttons; 
    *demo_p++ = cmd->buttons2;                 // [STRIFE]
    *demo_p++ = (byte)(cmd->inventory & 0xff); // [STRIFE]

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
// [STRIFE] Verified unmodified
//
void G_RecordDemo (const char* name)
{
    size_t demoname_size;
    int             i;
    int             maxsize;

    usergame = false;
    demoname_size = strlen(name) + 5;
    demoname = Z_Malloc(demoname_size, PU_STATIC, NULL);
    M_snprintf(demoname, demoname_size, "%s.lmp", name);
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
 
 
void G_BeginRecording (void) 
{ 
    int             i; 

    //
    // @category demo
    //
    // Record a high resolution "Doom 1.91" demo.
    //
    
    // STRIFE-TODO: if somebody makes a "Strife Plus", we could add this.
    /*
    longtics = M_CheckParm("-longtics") != 0;
    */
    longtics = false;

    // If not recording a longtics demo, record in low res
    lowres_turn = !longtics;

    demo_p = demobuffer;

    // Save the right version code for this demo
    *demo_p++ = STRIFE_VERSION;

    *demo_p++ = gameskill; 
    //*demo_p++ = gameepisode; [STRIFE] Doesn't have episodes.
    *demo_p++ = gamemap; 
    *demo_p++ = deathmatch; 
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
        *demo_p++ = playeringame[i]; 
} 
 

//
// G_PlayDemo 
//

const char	*defdemoname;
 
//
// G_DeferedPlayDemo
//
// [STRIFE] Verified unmodified
//
void G_DeferedPlayDemo(const char *name)
{ 
    defdemoname = name; 
    gameaction = ga_playdemo; 
} 

// Generate a string describing a demo version
// [STRIFE] Modified to handle the one and only Strife demo version.
static const char *DemoVersionDescription(int version)
{
    static char resultbuf[16];
 
    // [STRIFE] All versions of Strife 1.1 and later use 101 as their 
    // internal version number. Brilliant, huh? So we can't discern much
    // here.

    switch (version)
    {
    case 100: 
        return "v1.0"; // v1.0 would be the ancient demo version
    default:
        break;
    }

    // Unknown version. Who knows?
    M_snprintf(resultbuf, sizeof(resultbuf),
               "%i.%i (unknown)", version / 100, version % 100);

    return resultbuf;
}

//
// G_DoPlayDemo
//
// [STRIFE] Modified for Strife demo format.
//
void G_DoPlayDemo (void) 
{ 
    skill_t skill; 
    int     i, map; 
    int     demoversion;

    gameaction = ga_nothing; 
    demobuffer = demo_p = W_CacheLumpName (defdemoname, PU_STATIC); 

    demoversion = *demo_p++;

    if (demoversion == STRIFE_VERSION)
    {
        longtics = false;
    }
    /* STRIFE-TODO: Not until/unless somebody makes a Strife-Plus :P
    else if (demoversion == DOOM_191_VERSION)
    {
        // demo recorded with cph's modified "v1.91" doom exe
        longtics = true;
    }
    */
    else
    {
        const char *message = "Demo is from a different game version!\n"
                              "(read %i, should be %i)\n"
                              "\n"
                              "*** You may need to upgrade your version "
                                  "of Strife to v1.1 or later. ***\n"
                              "    See: https://www.doomworld.com/classicdoom"
                                        "/info/patches.php\n"
                              "    This appears to be %s.";

        I_Error(message, demoversion, STRIFE_VERSION,
                         DemoVersionDescription(demoversion));
    }
    
    skill = *demo_p++; 
    //episode = *demo_p++; [STRIFE] No episodes
    map = *demo_p++; 
    deathmatch = *demo_p++;
    respawnparm = *demo_p++;
    fastparm = *demo_p++;
    nomonsters = *demo_p++;
    consoleplayer = *demo_p++;

    for (i=0 ; i<MAXPLAYERS ; i++) 
        playeringame[i] = *demo_p++; 

    //!
    // @category demo
    // 
    // Play back a demo recorded in a netgame with a single player.
    //

    if (playeringame[1] || M_ParmExists("-solo-net"))
    {
	netgame = true;
	netdemo = true;
    }

    // don't spend a lot of time in loadlevel 
    precache = false;
    G_InitNew(skill, map); 
    precache = true; 
    
    // [STRIFE] not here...
    //starttime = I_GetTime (); 

    usergame = false; 
    demoplayback = true; 

    // [crispy] update the "singleplayer" variable
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
} 

//
// G_TimeDemo 
//
// [STRIFE] Verified unmodified
//
void G_TimeDemo (char* name) 
{
    //!
    // @category video
    // @vanilla
    //
    // Disable rendering the screen entirely.
    //

    nodrawers = M_ParmExists("-nodraw");

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
//
// [STRIFE] Verified unmodified
//
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
        respawnparm = false;
        fastparm = false;
        nomonsters = false;
        consoleplayer = 0;
        
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
        I_Error ("Demo %s recorded", demoname); 
    } 

    return false; 
} 
