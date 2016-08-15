//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

#include <stdio.h>

#include "doomtype.h"
#include "doomkeys.h"

#include "m_config.h"
#include "m_misc.h"

//
// Keyboard controls
//

int key_right = KEY_RIGHTARROW;
int key_left = KEY_LEFTARROW;

int key_up = KEY_UPARROW;
int key_down = KEY_DOWNARROW; 
int key_strafeleft = ',';
int key_straferight = '.';
int key_fire = KEY_RCTRL;
int key_use = ' ';
int key_strafe = KEY_RALT;
int key_speed = KEY_RSHIFT; 

// 
// Heretic keyboard controls
//
 
int key_flyup = KEY_PGUP;
int key_flydown = KEY_INS;
int key_flycenter = KEY_HOME;

int key_lookup = KEY_PGDN;
int key_lookdown = KEY_DEL;
int key_lookcenter = KEY_END;

int key_invleft = '[';
int key_invright = ']';
int key_useartifact = KEY_ENTER;

//
// Hexen key controls
//

int key_jump = '/';

int key_arti_all             = KEY_BACKSPACE;
int key_arti_health          = '\\';
int key_arti_poisonbag       = '0';
int key_arti_blastradius     = '9';
int key_arti_teleport        = '8';
int key_arti_teleportother   = '7';
int key_arti_egg             = '6';
int key_arti_invulnerability = '5';

//
// Strife key controls
//
// haleyjd 09/01/10
//

// Note: Strife also uses key_invleft, key_invright, key_jump, key_lookup, and
// key_lookdown, but with different default values.

int key_usehealth = 'h';
int key_invquery  = 'q';
int key_mission   = 'w';
int key_invpop    = 'z';
int key_invkey    = 'k';
int key_invhome   = KEY_HOME;
int key_invend    = KEY_END;
int key_invuse    = KEY_ENTER;
int key_invdrop   = KEY_BACKSPACE;


//
// Mouse controls
//

int mousebfire = 0;
int mousebstrafe = 1;
int mousebforward = 2;

int mousebjump = -1;

int mousebstrafeleft = -1;
int mousebstraferight = -1;
int mousebbackward = -1;
int mousebuse = -1;

int mousebprevweapon = -1;
int mousebnextweapon = -1;


int key_message_refresh = KEY_ENTER;
int key_pause = KEY_PAUSE;
int key_demo_quit = 'q';
int key_spy = KEY_F12;

// Multiplayer chat keys:

int key_multi_msg = 't';
int key_multi_msgplayer[8];

// Weapon selection keys:

int key_weapon1 = '1';
int key_weapon2 = '2';
int key_weapon3 = '3';
int key_weapon4 = '4';
int key_weapon5 = '5';
int key_weapon6 = '6';
int key_weapon7 = '7';
int key_weapon8 = '8';
int key_prevweapon = 0;
int key_nextweapon = 0;

// Map control keys:

int key_map_north     = KEY_UPARROW;
int key_map_south     = KEY_DOWNARROW;
int key_map_east      = KEY_RIGHTARROW;
int key_map_west      = KEY_LEFTARROW;
int key_map_zoomin    = '=';
int key_map_zoomout   = '-';
int key_map_toggle    = KEY_TAB;
int key_map_maxzoom   = '0';
int key_map_follow    = 'f';
int key_map_grid      = 'g';
int key_map_mark      = 'm';
int key_map_clearmark = 'c';

// menu keys:

int key_menu_activate  = KEY_ESCAPE;
int key_menu_up        = KEY_UPARROW;
int key_menu_down      = KEY_DOWNARROW;
int key_menu_left      = KEY_LEFTARROW;
int key_menu_right     = KEY_RIGHTARROW;
int key_menu_back      = KEY_BACKSPACE;
int key_menu_forward   = KEY_ENTER;
int key_menu_confirm   = 'y';
int key_menu_abort     = 'n';

int key_menu_help      = KEY_F1;
int key_menu_save      = KEY_F2;
int key_menu_load      = KEY_F3;
int key_menu_volume    = KEY_F4;
int key_menu_detail    = KEY_F5;
int key_menu_qsave     = KEY_F6;
int key_menu_endgame   = KEY_F7;
int key_menu_messages  = KEY_F8;
int key_menu_qload     = KEY_F9;
int key_menu_quit      = KEY_F10;
int key_menu_gamma     = KEY_F11;

int key_menu_incscreen = KEY_EQUALS;
int key_menu_decscreen = KEY_MINUS;
int key_menu_screenshot = 0;

//
// Joystick controls
//

int joybfire = 0;
int joybstrafe = 1;
int joybuse = 3;
int joybspeed = 2;

int joybstrafeleft = -1;
int joybstraferight = -1;

int joybjump = -1;

int joybprevweapon = -1;
int joybnextweapon = -1;

int joybmenu = -1;
int joybautomap = -1;

// Control whether if a mouse button is double clicked, it acts like 
// "use" has been pressed

int dclick_use = 1;
 
// 
// Bind all of the common controls used by Doom and all other games.
//

void M_BindBaseControls(void)
{
    M_BindIntVariable("key_right",          &key_right);
    M_BindIntVariable("key_left",           &key_left);
    M_BindIntVariable("key_up",             &key_up);
    M_BindIntVariable("key_down",           &key_down);
    M_BindIntVariable("key_strafeleft",     &key_strafeleft);
    M_BindIntVariable("key_straferight",    &key_straferight);
    M_BindIntVariable("key_fire",           &key_fire);
    M_BindIntVariable("key_use",            &key_use);
    M_BindIntVariable("key_strafe",         &key_strafe);
    M_BindIntVariable("key_speed",          &key_speed);

    M_BindIntVariable("mouseb_fire",        &mousebfire);
    M_BindIntVariable("mouseb_strafe",      &mousebstrafe);
    M_BindIntVariable("mouseb_forward",     &mousebforward);

    M_BindIntVariable("joyb_fire",          &joybfire);
    M_BindIntVariable("joyb_strafe",        &joybstrafe);
    M_BindIntVariable("joyb_use",           &joybuse);
    M_BindIntVariable("joyb_speed",         &joybspeed);

    M_BindIntVariable("joyb_menu_activate", &joybmenu);
    M_BindIntVariable("joyb_toggle_automap", &joybautomap);

    // Extra controls that are not in the Vanilla versions:

    M_BindIntVariable("joyb_strafeleft",     &joybstrafeleft);
    M_BindIntVariable("joyb_straferight",    &joybstraferight);
    M_BindIntVariable("mouseb_strafeleft",   &mousebstrafeleft);
    M_BindIntVariable("mouseb_straferight",  &mousebstraferight);
    M_BindIntVariable("mouseb_use",          &mousebuse);
    M_BindIntVariable("mouseb_backward",     &mousebbackward);
    M_BindIntVariable("dclick_use",          &dclick_use);
    M_BindIntVariable("key_pause",           &key_pause);
    M_BindIntVariable("key_message_refresh", &key_message_refresh);
}

void M_BindHereticControls(void)
{
    M_BindIntVariable("key_flyup",          &key_flyup);
    M_BindIntVariable("key_flydown",        &key_flydown);
    M_BindIntVariable("key_flycenter",      &key_flycenter);

    M_BindIntVariable("key_lookup",         &key_lookup);
    M_BindIntVariable("key_lookdown",       &key_lookdown);
    M_BindIntVariable("key_lookcenter",     &key_lookcenter);

    M_BindIntVariable("key_invleft",        &key_invleft);
    M_BindIntVariable("key_invright",       &key_invright);
    M_BindIntVariable("key_useartifact",    &key_useartifact);
}

void M_BindHexenControls(void)
{
    M_BindIntVariable("key_jump",           &key_jump);
    M_BindIntVariable("mouseb_jump",        &mousebjump);
    M_BindIntVariable("joyb_jump",          &joybjump);

    M_BindIntVariable("key_arti_all",             &key_arti_all);
    M_BindIntVariable("key_arti_health",          &key_arti_health);
    M_BindIntVariable("key_arti_poisonbag",       &key_arti_poisonbag);
    M_BindIntVariable("key_arti_blastradius",     &key_arti_blastradius);
    M_BindIntVariable("key_arti_teleport",        &key_arti_teleport);
    M_BindIntVariable("key_arti_teleportother",   &key_arti_teleportother);
    M_BindIntVariable("key_arti_egg",             &key_arti_egg);
    M_BindIntVariable("key_arti_invulnerability", &key_arti_invulnerability);
}

void M_BindStrifeControls(void)
{
    // These are shared with all games, but have different defaults:
    key_message_refresh = '/';

    // These keys are shared with Heretic/Hexen but have different defaults:
    key_jump     = 'a';
    key_lookup   = KEY_PGUP;
    key_lookdown = KEY_PGDN;
    key_invleft  = KEY_INS;
    key_invright = KEY_DEL;

    M_BindIntVariable("key_jump",           &key_jump);
    M_BindIntVariable("key_lookUp",         &key_lookup);
    M_BindIntVariable("key_lookDown",       &key_lookdown);
    M_BindIntVariable("key_invLeft",        &key_invleft);
    M_BindIntVariable("key_invRight",       &key_invright);

    // Custom Strife-only Keys:
    M_BindIntVariable("key_useHealth",      &key_usehealth);
    M_BindIntVariable("key_invquery",       &key_invquery);
    M_BindIntVariable("key_mission",        &key_mission);
    M_BindIntVariable("key_invPop",         &key_invpop);
    M_BindIntVariable("key_invKey",         &key_invkey);
    M_BindIntVariable("key_invHome",        &key_invhome);
    M_BindIntVariable("key_invEnd",         &key_invend);
    M_BindIntVariable("key_invUse",         &key_invuse);
    M_BindIntVariable("key_invDrop",        &key_invdrop);

    // Strife also supports jump on mouse and joystick, and in the exact same
    // manner as Hexen!
    M_BindIntVariable("mouseb_jump",        &mousebjump);
    M_BindIntVariable("joyb_jump",          &joybjump);
}

void M_BindWeaponControls(void)
{
    M_BindIntVariable("key_weapon1",        &key_weapon1);
    M_BindIntVariable("key_weapon2",        &key_weapon2);
    M_BindIntVariable("key_weapon3",        &key_weapon3);
    M_BindIntVariable("key_weapon4",        &key_weapon4);
    M_BindIntVariable("key_weapon5",        &key_weapon5);
    M_BindIntVariable("key_weapon6",        &key_weapon6);
    M_BindIntVariable("key_weapon7",        &key_weapon7);
    M_BindIntVariable("key_weapon8",        &key_weapon8);

    M_BindIntVariable("key_prevweapon",     &key_prevweapon);
    M_BindIntVariable("key_nextweapon",     &key_nextweapon);

    M_BindIntVariable("joyb_prevweapon",    &joybprevweapon);
    M_BindIntVariable("joyb_nextweapon",    &joybnextweapon);

    M_BindIntVariable("mouseb_prevweapon",  &mousebprevweapon);
    M_BindIntVariable("mouseb_nextweapon",  &mousebnextweapon);
}

void M_BindMapControls(void)
{
    M_BindIntVariable("key_map_north",      &key_map_north);
    M_BindIntVariable("key_map_south",      &key_map_south);
    M_BindIntVariable("key_map_east",       &key_map_east);
    M_BindIntVariable("key_map_west",       &key_map_west);
    M_BindIntVariable("key_map_zoomin",     &key_map_zoomin);
    M_BindIntVariable("key_map_zoomout",    &key_map_zoomout);
    M_BindIntVariable("key_map_toggle",     &key_map_toggle);
    M_BindIntVariable("key_map_maxzoom",    &key_map_maxzoom);
    M_BindIntVariable("key_map_follow",     &key_map_follow);
    M_BindIntVariable("key_map_grid",       &key_map_grid);
    M_BindIntVariable("key_map_mark",       &key_map_mark);
    M_BindIntVariable("key_map_clearmark",  &key_map_clearmark);
}

void M_BindMenuControls(void)
{
    M_BindIntVariable("key_menu_activate",  &key_menu_activate);
    M_BindIntVariable("key_menu_up",        &key_menu_up);
    M_BindIntVariable("key_menu_down",      &key_menu_down);
    M_BindIntVariable("key_menu_left",      &key_menu_left);
    M_BindIntVariable("key_menu_right",     &key_menu_right);
    M_BindIntVariable("key_menu_back",      &key_menu_back);
    M_BindIntVariable("key_menu_forward",   &key_menu_forward);
    M_BindIntVariable("key_menu_confirm",   &key_menu_confirm);
    M_BindIntVariable("key_menu_abort",     &key_menu_abort);

    M_BindIntVariable("key_menu_help",      &key_menu_help);
    M_BindIntVariable("key_menu_save",      &key_menu_save);
    M_BindIntVariable("key_menu_load",      &key_menu_load);
    M_BindIntVariable("key_menu_volume",    &key_menu_volume);
    M_BindIntVariable("key_menu_detail",    &key_menu_detail);
    M_BindIntVariable("key_menu_qsave",     &key_menu_qsave);
    M_BindIntVariable("key_menu_endgame",   &key_menu_endgame);
    M_BindIntVariable("key_menu_messages",  &key_menu_messages);
    M_BindIntVariable("key_menu_qload",     &key_menu_qload);
    M_BindIntVariable("key_menu_quit",      &key_menu_quit);
    M_BindIntVariable("key_menu_gamma",     &key_menu_gamma);

    M_BindIntVariable("key_menu_incscreen", &key_menu_incscreen);
    M_BindIntVariable("key_menu_decscreen", &key_menu_decscreen);
    M_BindIntVariable("key_menu_screenshot",&key_menu_screenshot);
    M_BindIntVariable("key_demo_quit",      &key_demo_quit);
    M_BindIntVariable("key_spy",            &key_spy);
}

void M_BindChatControls(unsigned int num_players)
{
    char name[32];  // haleyjd: 20 not large enough - Thank you, come again!
    unsigned int i; // haleyjd: signedness conflict

    M_BindIntVariable("key_multi_msg",     &key_multi_msg);

    for (i=0; i<num_players; ++i)
    {
        M_snprintf(name, sizeof(name), "key_multi_msgplayer%i", i + 1);
        M_BindIntVariable(name, &key_multi_msgplayer[i]);
    }
}

//
// Apply custom patches to the default values depending on the
// platform we are running on.
//

void M_ApplyPlatformDefaults(void)
{
    // no-op. Add your platform-specific patches here.
}

