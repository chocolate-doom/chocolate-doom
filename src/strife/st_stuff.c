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
#include "p_dialog.h"   // villsa [STRIFE]

#include "am_map.h"
#include "m_cheat.h"
#include "m_menu.h" // villsa [STRIFE]
#include "m_misc.h"

#include "s_sound.h"

// Needs access to LFB.
#include "v_video.h"
#include "i_swap.h"

// State.
#include "doomstat.h"
#include "d_main.h"    // [STRIFE]

// Data.
#include "dstrings.h"
#include "sounds.h"
#include "m_controls.h"
#include "hu_lib.h"     // [STRIFE]
#include "hu_stuff.h"

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

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
// haleyjd 20100901: [STRIFE] Adjusted.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                311
#define ST_AMMOY                162

// HEALTH number pos.
// haleyjd 20100901: [STRIFE] Adjusted.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              79
#define ST_HEALTHY              162

// [STRIFE]
// Removed:
// * Weapon pos.
// * Frags pos.
// * ARMOR number pos.
// * Key icon positions.

// Ammunition counter.
// haleyjd 20110213 [STRIFE]: ammo counters for the popup widget
#define ST_POPUPAMMOX       206
static const int st_yforammo[NUMAMMO] = { 75, 99, 91, 139, 131, 115, 123 };
static const int st_wforammo[NUMAMMO] = { 3,  3,  2,  3,   3,   2,   3   };

// Indicate maximum ammunition.
// Only needed because backpack exists.
// haleyjd 20110213 [STRIFE]: maxammo counters for the popup widget
#define ST_POPUPMAXAMMOX        239

// [STRIFE]
// Removed:
// * Doom weapon stuff
// * DETH title (???)

// Dimensions given in characters.
#define ST_MSGWIDTH             52

// haleyjd 20100831: [STRIFE] 
// * Removed faces.
// haleyjd 20100901:
// * Removed DOOM pre-beta cruft.
// * Removed deathmatch frags/arms-related stuff.
// * Removed arms panel stuff.
// * Removed unused widgets.
// * Removed more faces, keyboxes, st_randomnumber

// graphics are drawn to a backing screen and blitted to the real screen
//byte                   *st_backing_screen;  - [STRIFE]: Unused.

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

// villsa [STRIFE]
static boolean          st_dosizedisplay = false;

// haleyjd 09/01/10: [STRIFE]
// Whether or not a popup is currently displayed
static boolean          st_displaypopup = false;

// villsa [STRIFE]
static int              st_popupdisplaytics = 0;

// villsa [STRIFE]
// Whether or not show popup objective screen
static boolean          st_showobjective = false;

// villsa [STRIFE]
static boolean          st_showinvpop = false;

// villsa [STRIFE]
static boolean          st_showkeys = false;

// villsa [STRIFE] TODO - identify variables
static int              st_keypage = -1;
// [unused] static int              dword_88490 = 0;

// haleyjd 09/19/10: [STRIFE] Cached player data
static int              st_lastcursorpos;
static int              st_lastammo;
static int              st_lastarmortype;
static int              st_lasthealth;

// haleyjd 20100901: [STRIFE] sbar -> invback
// main inventory background and other bits
static patch_t*         invback;     // main bar
static patch_t*         stback;      // multiplayer background
static patch_t*         invtop;      // top bit
static patch_t*         invpop;      // popup frame with text
static patch_t*         invpop2;     // plain popup frame
static patch_t*         invpbak;     // popup background w/details
static patch_t*         invpbak2;    // plain popup background
static patch_t*         invcursor;   // cursor

// ammo/weapon/armor patches
static patch_t*         invammo[NUMAMMO]; // ammo/weapons
static patch_t*         invsigil[5];      // sigil pieces
static patch_t*         invarmor[2];      // armor icons

// names for ammo patches
static char *invammonames[NUMAMMO] =
{
    "I_BLIT",
    "I_XQRL",
    "I_PQRL",
    "I_BRY1",
    "I_ROKT",
    "I_GRN1",
    "I_GRN2"
};

// haleyjd 20100901: [STRIFE] Replaced tallnum, shortnum w/inv fonts
// 0-9, green numbers
static patch_t*         invfontg[10];

// 0-9, yellow numbers
static patch_t*         invfonty[10];

// [unused] 3 key-cards, 3 skulls -- [STRIFE] has a lot more keys than 3 :P
//static patch_t*         keys[NUMCARDS]; 

// ready-weapon widget
static st_number_t      w_ready; // haleyjd [STRIFE]: This is still used.

// haleyjd: [STRIFE] This is still used but was changed to a st_number_t.
// health widget
static st_number_t      w_health;

// ammo widgets
static st_number_t      w_ammo[NUMAMMO];     // haleyjd [STRIFE]: Still used.

// max ammo widgets
static st_number_t      w_maxammo[NUMAMMO];  // haleyjd [STRIFE]: Still used.

// [unused] number of frags so far in deathmatch
//static int              st_fragscount;


cheatseq_t cheat_mus        = CHEAT("spin", 2);         // [STRIFE]: idmus -> spin
cheatseq_t cheat_god        = CHEAT("omnipotent", 0);   // [STRIFE]: iddqd -> omnipotent
cheatseq_t cheat_ammo       = CHEAT("boomstix", 0);     // [STRIFE]: idfa -> boomstix
cheatseq_t cheat_noclip     = CHEAT("elvis", 0);        // [STRIFE]: idclip -> elvis
cheatseq_t cheat_clev       = CHEAT("rift", 2);         // [STRIFE]: idclev -> rift
cheatseq_t cheat_mypos      = CHEAT("gps", 0);          // [STRIFE]: idmypos -> gps
cheatseq_t cheat_scoot      = CHEAT("scoot", 1);        // [STRIFE]: new cheat scoot
cheatseq_t cheat_nuke       = CHEAT("stonecold", 0);    // [STRIFE]: new cheat stonecold
cheatseq_t cheat_keys       = CHEAT("jimmy", 0);        // [STRIFE]: new cheat jimmy (all keys)
cheatseq_t cheat_stealth    = CHEAT("gripper", 0);      // [STRIFE]: new cheat gripper
cheatseq_t cheat_midas      = CHEAT("donnytrump", 0);   // [STRIFE]: new cheat
cheatseq_t cheat_lego       = CHEAT("lego", 0);         // [STRIFE]: new cheat
cheatseq_t cheat_dev        = CHEAT("dots", 0);         // [STRIFE]: new cheat

// haleyjd 20110224: enumeration for access to powerup cheats
enum
{
    ST_PUMPUP_B,
    ST_PUMPUP_I,
    ST_PUMPUP_M,
    ST_PUMPUP_H,
    ST_PUMPUP_P,
    ST_PUMPUP_S,
    ST_PUMPUP_T,
    ST_PUMPUP,
    NUM_ST_PUMPUP
};

cheatseq_t cheat_powerup[NUM_ST_PUMPUP] = // [STRIFE]
{
    CHEAT("pumpupb", 0),
    CHEAT("pumpupi", 0),
    CHEAT("pumpupm", 0),
    CHEAT("pumpuph", 0),
    CHEAT("pumpupp", 0),
    CHEAT("pumpups", 0),
    CHEAT("pumpupt", 0),
    CHEAT("pumpup", 0),
};

//cheatseq_t cheat_choppers = CHEAT("idchoppers", 0); [STRIFE] no such thing

void M_SizeDisplay(int choice); // villsa [STRIFE]

//
// STATUS BAR CODE
//
void ST_Stop(void);

// [STRIFE]
static char st_msgbuf[ST_MSGWIDTH];

// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder(event_t* ev)
{
    // haleyjd 09/27/10: made static to ST_Responder
    static boolean st_keystate = false;
    int i;

    // Filter automap on/off.
    if(ev->type == ev_keyup)
    {
        if((ev->data1 & 0xffff0000) == AM_MSGHEADER)
        {
            switch(ev->data1)
            {
            case AM_MSGENTERED:
                st_gamestate = AutomapState;
                st_firsttime = true;
                break;

            case AM_MSGEXITED:
                st_gamestate = FirstPersonState;
                break;
            }

            return false;
        }

        // villsa [STRIFE]
        if(ev->data1 != key_invpop &&
           ev->data1 != key_mission &&
           ev->data1 != key_invkey)
            return false;

        // villsa [STRIFE]
        if(ev->data1 == key_invpop)
            st_showinvpop = false;
        else
        {
            if(ev->data1 == key_mission)
                st_showobjective = false;
            else
            {
                if(ev->data1 == key_invkey)
                {
                    st_showkeys = false;
                    st_keystate = false;
                }
            }
        }

        if(!st_showkeys && !st_showobjective && !st_showinvpop)
        {
             if(!st_popupdisplaytics)
             {
                 st_displaypopup = false;
                 if(st_dosizedisplay)
                     M_SizeDisplay(true);

                 st_dosizedisplay = false;
             }
        }

        return true;
    }

    // if a user keypress...
    if(ev->type != ev_keydown)
        return false;

    // haleyjd 20100927: No input allowed when the player is dead
    if(plyr->mo->health <= 0)
        return false;

    // keydown events
    if(ev->data1 == key_invquery) // inventory query
    {
        inventory_t *inv = &(plyr->inventory[plyr->inventorycursor]);
        if(inv->amount)
        {
            DEH_snprintf(st_msgbuf, sizeof(st_msgbuf), "%d %s",
                         inv->amount, 
                         DEH_String(mobjinfo[inv->type].name));
            plyr->message = st_msgbuf;
        }
    }

    // villsa [STRIFE]
    if(ev->data1 == key_invpop || ev->data1 == key_invkey || ev->data1 == key_mission)
    {
        if(ev->data1 == key_invkey)
        {
            st_showobjective = false;
            st_showinvpop = false;

            if(!st_keystate)
            {
                st_keystate = true;
                if(++st_keypage > 2)
                {
                    st_popupdisplaytics = 0;
                    st_showkeys = false;
                    st_displaypopup = false;
                    st_keypage = -1;
                    return true;
                }
            }

            if(netgame)
                st_popupdisplaytics = 20;
            else
                st_popupdisplaytics = 50;

            st_showkeys = true;
        }
        else
        {
            if(ev->data1 != key_mission || netgame)
            {
                if(ev->data1 ==  key_invpop)
                {
                    st_keypage = -1;
                    st_popupdisplaytics = false;
                    st_showkeys = false;
                    st_showobjective = false;
                    st_showinvpop = true;
                }
            }
            else
            {
                st_showkeys = netgame;
                st_showinvpop = netgame;
                st_keypage = -1;

                st_popupdisplaytics = ev->data2 ^ key_mission;

                st_showobjective = true;
            }
        }

        if(st_showkeys || st_showobjective || st_showinvpop)
        {
            st_displaypopup = true;
            if(viewheight == SCREENHEIGHT)
            {
                M_SizeDisplay(false);
                st_dosizedisplay = true;
            }
        }
    }
    
    if(ev->data1 == key_invleft) // inventory move left
    {
        if(plyr->inventorycursor > 0)
            plyr->inventorycursor--;
        return true;
    }
    else if(ev->data1 == key_invright)
    {
        if(plyr->inventorycursor < plyr->numinventory - 1)
            plyr->inventorycursor++;
        return true;
    }
    else if(ev->data1 == key_invhome)
    {
        plyr->inventorycursor = 0;
        return true;
    }
    else if(ev->data1 == key_invend)
    {
        if(plyr->numinventory)
            plyr->inventorycursor = plyr->numinventory - 1;
        else 
            plyr->inventorycursor = 0;
        return true;
    }

    //
    // [STRIFE] Cheats which are allowed in netgames/demos:
    //

    // 'spin' cheat for changing music
    if (cht_CheckCheat(&cheat_mus, ev->data2))
    {
        char        buf[3];
        int         musnum;

        plyr->message = DEH_String(STSTR_MUS);
        cht_GetParam(&cheat_mus, buf);

        musnum = (buf[0] - '0') * 10 + buf[1] - '0';

        if (((buf[0]-'0')*10 + buf[1]-'0') > 35)
            plyr->message = DEH_String(STSTR_NOMUS);
        else
            S_ChangeMusic(musnum, 1);
    }
    // [STRIFE]: "dev" cheat - "DOTS"
    else if (cht_CheckCheat(&cheat_dev, ev->data2))
    {
        devparm = !devparm;
        if (devparm)
            plyr->message = DEH_String("devparm ON");
        else
            plyr->message = DEH_String("devparm OFF");
    }

    // [STRIFE] Cheats below are not allowed in netgames or demos
    if(netgame || !usergame)
        return false;

    if (cht_CheckCheat(&cheat_god, ev->data2))
    {
        // 'omnipotent' cheat for toggleable god mode
        plyr->cheats ^= CF_GODMODE;
        if (plyr->cheats & CF_GODMODE)
        {
            if (plyr->mo)
                plyr->mo->health = 100;

            plyr->health = deh_god_mode_health;
            plyr->st_update = true; // [STRIFE]
            plyr->message = DEH_String(STSTR_DQDON);
        }
        else 
        {
            plyr->st_update = true;
            plyr->message = DEH_String(STSTR_DQDOFF);
        }
    }
    else if (cht_CheckCheat(&cheat_ammo, ev->data2))
    {
        // [STRIFE]: "BOOMSTIX" cheat for all normal weapons
        plyr->armorpoints = deh_idkfa_armor;
        plyr->armortype = deh_idkfa_armor_class;

        for (i = 0; i < NUMWEAPONS; i++)
            if(!isdemoversion || weaponinfo[i].availabledemo)
                plyr->weaponowned[i] = true;
        
        // Takes away the Sigil, even if you already had it...
        plyr->weaponowned[wp_sigil] = false;

        for (i=0;i<NUMAMMO;i++)
            plyr->ammo[i] = plyr->maxammo[i];

        plyr->message = DEH_String(STSTR_FAADDED);
    }
    else if(cht_CheckCheat(&cheat_keys, ev->data2))
    {
        // villsa [STRIFE]: "JIMMY" cheat for all keys
        #define FIRSTKEYSETAMOUNT   16

        if(plyr->cards[FIRSTKEYSETAMOUNT - 1])
        {
            if(plyr->cards[NUMCARDS - 1] || isdemoversion)
            {
                for(i = 0; i < NUMCARDS; i++)
                    plyr->cards[i] = false;

                plyr->message = DEH_String("Keys removed");
            }
            else
            {
                for(i = 0; i < NUMCARDS; i++)
                    plyr->cards[i] = true;

                plyr->message = DEH_String("Cheater Keys Added");
            }
        }
        else
        {
            for(i = 0; i < FIRSTKEYSETAMOUNT; i++)
                plyr->cards[i] = true;

            plyr->message = DEH_String("Cheater Keys Added");
        }
    }
    else if (cht_CheckCheat(&cheat_noclip, ev->data2))
    {
        // [STRIFE] Removed idspispopd, added NOCLIP flag setting/removal
        // Noclip cheat - "ELVIS" (hah-hah :P )

        plyr->cheats ^= CF_NOCLIP;

        if (plyr->cheats & CF_NOCLIP)
        {
            plyr->message = DEH_String(STSTR_NCON);
            plyr->mo->flags |= MF_NOCLIP;
        }
        else
        {
            plyr->message = DEH_String(STSTR_NCOFF);
            plyr->mo->flags &= ~MF_NOCLIP;
        }
    }
    else if(cht_CheckCheat(&cheat_stealth, ev->data2))
    {
        // villsa [STRIFE]: "GRIPPER" cheat; nothing to do with stealth...
        plyr->cheats ^= CF_NOMOMENTUM;
        if(plyr->cheats & CF_NOMOMENTUM)
            plyr->message = DEH_String("STEALTH BOOTS ON");
        else
            plyr->message = DEH_String("STEALTH BOOTS OFF");
    }
    
    for(i = 0; i < ST_PUMPUP_B + 3; ++i)
    {
        // [STRIFE]: Handle berserk, invisibility, and envirosuit
        if(cht_CheckCheat(&cheat_powerup[i], ev->data2))
        {
            if(plyr->powers[i])
                plyr->powers[i] = (i != 1);
            else
                P_GivePower(plyr, i);
            plyr->message = DEH_String(STSTR_BEHOLDX);
        }
    }
    if(cht_CheckCheat(&cheat_powerup[ST_PUMPUP_H], ev->data2))
    {
        // [STRIFE]: PUMPUPH gives medical inventory items
        P_GiveItemToPlayer(plyr, SPR_STMP, MT_INV_MED1);
        P_GiveItemToPlayer(plyr, SPR_MDKT, MT_INV_MED2);
        P_GiveItemToPlayer(plyr, SPR_FULL, MT_INV_MED3);
        plyr->message = DEH_String("you got the stuff!");
    }
    if(cht_CheckCheat(&cheat_powerup[ST_PUMPUP_P], ev->data2))
    {
        // [STRIFE]: PUMPUPP gives backpack
        if(!plyr->backpack)
        {
            for(i = 0; i < NUMAMMO; ++i)
                plyr->maxammo[i] = 2 * plyr->maxammo[i];
        }
        plyr->backpack = true;

        for(i = 0; i < NUMAMMO; ++i)
            P_GiveAmmo(plyr, i, 1);
        plyr->message = DEH_String("you got the stuff!");
    }
    if(cht_CheckCheat(&cheat_powerup[ST_PUMPUP_S], ev->data2))
    {
        // [STRIFE]: PUMPUPS gives stamina and accuracy upgrades
        P_GiveItemToPlayer(plyr, SPR_TOKN, MT_TOKEN_STAMINA);
        P_GiveItemToPlayer(plyr, SPR_TOKN, MT_TOKEN_NEW_ACCURACY);
        plyr->message = DEH_String("you got the stuff!");
    }
    if(cht_CheckCheat(&cheat_powerup[ST_PUMPUP_T], ev->data2))
    {
        // [STRIFE] PUMPUPT gives targeter
        P_GivePower(plyr, pw_targeter);
        plyr->message = DEH_String("you got the stuff!");
    }
    // [STRIFE]: PUMPUP
    if (cht_CheckCheat(&cheat_powerup[ST_PUMPUP], ev->data2))
    {
        // 'behold' power-up menu
        plyr->message = DEH_String(STSTR_BEHOLD);
        return false;
    }

    if (cht_CheckCheat(&cheat_mypos, ev->data2))
    {
        // [STRIFE] 'GPS' for player position
        static char buf[ST_MSGWIDTH];
        M_snprintf(buf, sizeof(buf),
                   "ang=0x%x;x,y=(0x%x,0x%x)",
                   players[consoleplayer].mo->angle,
                   players[consoleplayer].mo->x,
                   players[consoleplayer].mo->y);
        plyr->message = buf;
    }

    // 'rift' change-level cheat
    if (cht_CheckCheat(&cheat_clev, ev->data2))
    {
        char            buf[3];
        int             map;

        cht_GetParam(&cheat_clev, buf);

        map = (buf[0] - '0') * 10 + buf[1] - '0';

        // haleyjd 20100901: Removed Chex Quest stuff.
        // haleyjd 20100915: Removed retail/registered/shareware stuff

        // haleyjd 20130301: different bounds in v1.31
        // Ohmygod - this is not going to work.
        if(gameversion == exe_strife_1_31)
        {
            if ((isdemoversion && (map < 32 || map > 34)) ||
                (isregistered  && (map <= 0 || map > 34)))
                return false;
        }
        else
        {
            if (map <= 0 || map > 40)
                return false;
        }

        // So be it.
        plyr->message = DEH_String(STSTR_CLEV);
        G_RiftExitLevel(map, 0, plyr->mo->angle);
    }
    else if(cht_CheckCheat(&cheat_scoot, ev->data2))
    {
        char            buf[3];
        int             spot;
        
        cht_GetParam(&cheat_scoot, buf);

        spot = buf[0] - '0';

        // BUG: should be <= 9. Shouldn't do anything bad though...
        if(spot <= 10) 
        {
            plyr->message = DEH_String("Spawning to spot");
            G_RiftCheat(spot);
            return false;
        }
    }

    // villsa [STRIFE]
    if(cht_CheckCheat(&cheat_nuke, ev->data2))
    {
        stonecold ^= 1;
        plyr->message = DEH_String("Kill 'em.  Kill 'em All");
        return false;
    }

    // villsa [STRIFE]
    if(cht_CheckCheat(&cheat_midas, ev->data2))
    {
        plyr->message = DEH_String("YOU GOT THE MIDAS TOUCH, BABY");
        P_GiveItemToPlayer(plyr, SPR_HELT, MT_TOKEN_TOUGHNESS);
    }

    // villsa [STRIFE] 
    // haleyjd 20110224: No sigil in demo version
    if(!isdemoversion && cht_CheckCheat(&cheat_lego, ev->data2))
    {
        plyr->st_update = true;
        if(plyr->weaponowned[wp_sigil])
        {
            if(++plyr->sigiltype > 4)
            {
                plyr->sigiltype = -1;
                plyr->pendingweapon = wp_fist;
                plyr->weaponowned[wp_sigil] = false;
            }
        }
        else
        {
            plyr->weaponowned[wp_sigil] = true;
            plyr->sigiltype = 0;
        }
        // BUG: This brings up a bad version of the Sigil (sigiltype -1) which
        // causes some VERY interesting behavior, when you type LEGO for the
        // sixth time. This shouldn't be done when taking it away, and yet it
        // is here... verified with vanilla.
        plyr->pendingweapon = wp_sigil;
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
        dword_DC7F4 = dword_DC7F0;*/

    if(st_popupdisplaytics)
    {
        int tics = st_popupdisplaytics;

        --st_popupdisplaytics;
        if(tics == 1)
        {
            st_displaypopup = false;
            st_showkeys = false;
            st_keypage = -1;

            if(st_dosizedisplay)
                M_SizeDisplay(true);  // mondo hack?

            st_dosizedisplay = false;
        }
    }

    // haleyjd 20100901: [STRIFE] Keys are handled on a popup
    // haleyjd 20100831: [STRIFE] No face widget
    // haleyjd 20100901: [STRIFE] Armor, weapons, frags, etc. handled elsewhere

    // haleyjd: This is from the PRE-BETA! Left here because it amuses me ;)
    // get rid of chat window if up because of message
    //if (!--st_msgcounter)
    //    st_chat = st_oldchat;
}

static int st_palette = 0;

//
// ST_doPaletteStuff
//
// haleyjd 20100831: [STRIFE]
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
    // haleyjd 20100831: [STRIFE] Flash green when in nukage, not when has
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

//
// ST_drawNumFontY
//
// haleyjd 20100919: [STRIFE] New function
// Draws a small yellow number for inventory etc.
//
void ST_drawNumFontY(int x, int y, int num)
{
    if(!num)
        V_DrawPatch(x, y, invfonty[0]);
    
    while(num)
    {
        V_DrawPatch(x, y, invfonty[num % 10]);
        x -= SHORT(invfonty[0]->width) + 1;
        num /= 10;
    }
}

//
// ST_drawNumFontY2
//
// haleyjd 20100919: [STRIFE] New function
// As above, but turns negative numbers into zero.
//
void ST_drawNumFontY2(int x, int y, int num)
{
    if(!num)
        V_DrawPatch(x, y, invfonty[0]);

    if(num < 0)
        num = 0;

    while(num)
    {
        V_DrawPatchDirect(x, y, invfonty[num % 10]);
        x -= SHORT(invfonty[0]->width) + 1;
        num /= 10;
    }
}

//
// ST_drawLine
//
// haleyjd 20100920: [STRIFE] New function
// Basic horizontal line drawing routine used for the health bars.
//
void ST_drawLine(int x, int y, int len, int color)
{
    byte putcolor = (byte)(color);
    byte *drawpos = I_VideoBuffer + y * SCREENWIDTH + x;
    int i = 0;

    while(i < len)
    {
        *drawpos++ = putcolor;
        ++i;
    }
}

//
// ST_doRefresh
//
// haleyjd 20100920: Evidence more than suggests that Rogue moved all status bar
// drawing down to this function.
//
void ST_doRefresh(void)
{
    // draw status bar background to off-screen buff
    if (st_statusbaron)
    {
        int firstinventory, icon_x, num_x, i, numdrawn;

        // haleyjd 20100919: No backscreen caching in Strife.
        //V_UseBuffer(st_backing_screen);

        // TODO: only sometimes drawing?

        plyr->st_update = false;

        // cache data
        st_lastcursorpos = plyr->inventorycursor;
        st_lastammo      = weaponinfo[plyr->readyweapon].ammo;
        st_lastarmortype = plyr->armortype;
        st_lasthealth    = plyr->health;
        st_firsttime     = false;

        // draw main status bar
        V_DrawPatch(ST_X, ST_Y, invback);

        // draw multiplayer armor backdrop if netgame
        // haleyjd 20131031: BUG - vanilla is accessing a NULL pointer here when
        // playing back netdemos! It doesn't appear to draw anything, and there 
        // is no apparent ill effect on gameplay, so the best we can do is check.
        if(netgame && stback)
            V_DrawPatch(ST_X, 173, stback);

        if(plyr->inventorycursor >= 6)
            firstinventory = plyr->inventorycursor - 5;
        else
            firstinventory = 0;

        // Draw cursor.
        if(plyr->numinventory)
        {
            V_DrawPatch(35 * (plyr->inventorycursor - firstinventory) + 42,
                        180, invcursor);
        }

        // Draw inventory bar
        for(num_x = 68, icon_x = 48, i = firstinventory, numdrawn = 0; 
            num_x < 278; 
            num_x += 35, icon_x += 35, i++, numdrawn++)
        {
            int lumpnum;
            patch_t *patch;
            char iconname[8];

            if(plyr->numinventory <= numdrawn)
                break;
            
            DEH_snprintf(iconname, sizeof(iconname), "I_%s",
                         DEH_String(sprnames[plyr->inventory[i].sprite]));

            lumpnum = W_CheckNumForName(iconname);
            if(lumpnum == -1)
                patch = W_CacheLumpName(DEH_String("STCFN063"), PU_CACHE);
            else
                patch = W_CacheLumpNum(lumpnum, PU_STATIC);

            V_DrawPatch(icon_x, 182, patch);
            ST_drawNumFontY(num_x, 191, plyr->inventory[i].amount);
        }

        // haleyjd 20100919: Draw sigil icon
        if(plyr->weaponowned[wp_sigil])
            V_DrawPatch(253, 175, invsigil[plyr->sigiltype]);

        // haleyjd 20100919: Draw ammo
        if(st_lastammo < NUMAMMO)
            V_DrawPatch(290, 180, invammo[st_lastammo]);

        // haleyjd 20100919: Draw armor
        if(plyr->armortype)
        {
            V_DrawPatch(2, 177, invarmor[plyr->armortype - 1]);
            ST_drawNumFontY(20, 191, plyr->armorpoints);
        }

        // haleyjd 20100920: Draw life bars.
        {
            int barlength;
            int lifecolor1;
            int lifecolor2;

            barlength = plyr->health;
            if(barlength > 100)
                barlength = 200 - plyr->health;
            barlength *= 2;

            if(plyr->health < 11)      // Danger, Will Robinson!
                lifecolor1 = 64;
            else if(plyr->health < 21) // Caution
                lifecolor1 = 80;
            else                       // All is well.
                lifecolor1 = 96;

            if(plyr->cheats & CF_GODMODE) // Gold, probably a throwback to DOOM.
                lifecolor1 = 226;

            lifecolor2 = lifecolor1 + 3;

            // Draw the normal health bars
            ST_drawLine(49, 172, barlength, lifecolor1);
            ST_drawLine(49, 173, barlength, lifecolor2);
            ST_drawLine(49, 175, barlength, lifecolor1);
            ST_drawLine(49, 176, barlength, lifecolor2);

            // Draw the > 100 health lines
            if(plyr->health > 100)
            {
                int oldbarlength = barlength;
                lifecolor1 = 112;             // Shades of blue
                lifecolor2 = lifecolor1 + 3;

                // take up the difference not drawn by the first (<= 100) bar
                barlength = 200 - barlength;

                ST_drawLine(49 + oldbarlength, 172, barlength, lifecolor1);
                ST_drawLine(49 + oldbarlength, 173, barlength, lifecolor2);
                ST_drawLine(49 + oldbarlength, 175, barlength, lifecolor1);
                ST_drawLine(49 + oldbarlength, 176, barlength, lifecolor2);
            }
        } // end local-scope block

        // haleyjd 20100919: nope, not in Strife.
        //V_RestoreBuffer();
        //V_CopyRect(ST_X, 0, st_backing_screen, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y);
    }
}

// haleyjd [STRIFE]: Removed ST_diffDraw

void ST_Drawer (boolean fullscreen, boolean refresh)
{
    st_statusbaron = (!fullscreen) || automapactive;
    st_firsttime = st_firsttime || refresh;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // If just after ST_Start(), refresh all
    ST_doRefresh();
    // Otherwise, update as little as possible
    //ST_diffDraw(); [STRIFE]: nope
}

//
// ST_calcFrags
//
// haleyjd [STRIFE] New function.
// Calculate frags for display on the frags popup.
//
static int ST_calcFrags(int pnum)
{
    int i;
    int result = 0;

    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(i == pnum) // self-frags
            result -= players[pnum].frags[i];
        else
            result += players[pnum].frags[i];
    }

    return result;
}

//
// ST_drawTime
//
// villsa [STRIFE] New function.
// Draws game time on pop up screen
//
static void ST_drawTime(int x, int y, int time)
{
    int hours;
    int minutes;
    int seconds;
    char string[16];

    // haleyjd 20110213: fixed minutes
    hours   = time / 3600;
    minutes = (time / 60) % 60;
    seconds = time % 60;

    DEH_snprintf(string, 16, "%02d:%02d:%02d", hours, minutes, seconds);
    HUlib_drawYellowText(x, y, string);
}

#define ST_KEYSPERPAGE   10
#define ST_KEYS_X        20
#define ST_KEYS_Y        63
#define ST_KEYNAME_X     17
#define ST_KEYNAME_Y      4
#define ST_KEYS_YSTEP    17
#define ST_KEYS_NUMROWS   4
#define ST_KEYS_COL2X   160

//
// ST_drawKeysPopup
//
// haleyjd 20110213: [STRIFE] New function
// This has taken the longest out of almost everything to get working properly.
//
static boolean ST_drawKeysPopup(void)
{
    int x, y, yt, key, keycount;
    mobjinfo_t *info;

    V_DrawXlaPatch(0, 56, invpbak2);
    V_DrawPatchDirect(0, 56, invpop2);

    if(deathmatch)
    {
        int pnum;
        patch_t *colpatch;
        char buffer[128];
        int frags;

        // In deathmatch, the keys popup is replaced by a chart of frag counts
        
        // first column
        y  = 64;
        yt = 66;
        for(pnum = 0; pnum < MAXPLAYERS/2; pnum++)
        {
            DEH_snprintf(buffer, sizeof(buffer), "stcolor%d", pnum+1);
            colpatch = W_CacheLumpName(buffer, PU_CACHE);
            V_DrawPatchDirect(28, y, colpatch);
            frags = ST_calcFrags(pnum);
            DEH_snprintf(buffer, sizeof(buffer), "%s%d", player_names[pnum], frags);
            HUlib_drawYellowText(38, yt, buffer);
            if(!playeringame[pnum])
                HUlib_drawYellowText(28, pnum*17 + 65, "X");
            y  += 17;
            yt += 17;
        }

        // second column
        y  = 64;
        yt = 66;
        for(pnum = MAXPLAYERS/2; pnum < MAXPLAYERS; pnum++)
        {
            DEH_snprintf(buffer, sizeof(buffer), "stcolor%d", pnum+1);
            colpatch = W_CacheLumpName(buffer, PU_CACHE);
            V_DrawPatchDirect(158, y, colpatch);
            frags = ST_calcFrags(pnum);
            DEH_snprintf(buffer, sizeof(buffer), "%s%d", player_names[pnum], frags);
            HUlib_drawYellowText(168, yt, buffer);
            if(!playeringame[pnum])
                HUlib_drawYellowText(158, pnum*17 - 3, "X");
            y  += 17;
            yt += 17;
        }
    }
    else
    {
        // Bounds-check page number
        if(st_keypage < 0 || st_keypage > 2)
        {
            st_keypage = -1;
            st_popupdisplaytics = 0;
            st_displaypopup = false;

            return false;
        }

        // Are there any keys to display on this page?
        if(st_keypage > 0)
        {
            boolean haskeyinrange = false;

            for(key = ST_KEYSPERPAGE * st_keypage, keycount = 0;
                keycount < ST_KEYSPERPAGE && key < NUMCARDS;
                ++key, ++keycount)
            {
                if(plyr->cards[key])
                    haskeyinrange = true;
            }

            if(!haskeyinrange)
            {
                st_displaypopup = false;
                st_showkeys = false;
                st_keypage = -1;
                
                return false;
            }
        }

        // Draw the keys for the current page
        key = ST_KEYSPERPAGE * st_keypage;
        keycount = 0;
        x = ST_KEYS_X;
        y = ST_KEYS_Y;
        info = &mobjinfo[MT_KEY_BASE + key];

        for(; keycount < ST_KEYSPERPAGE && key < NUMCARDS; ++key, ++keycount, ++info)
        {
            char sprname[8];
            patch_t *patch;
            memset(sprname, 0, sizeof(sprname));

            if(plyr->cards[key])
            {
                // Get spawnstate sprite name and load corresponding icon
                DEH_snprintf(sprname, sizeof(sprname), "I_%s",
                    sprnames[states[info->spawnstate].sprite]);
                patch = W_CacheLumpName(sprname, PU_CACHE);
                V_DrawPatchDirect(x, y, patch);
                HUlib_drawYellowText(x + ST_KEYNAME_X, y + ST_KEYNAME_Y, info->name);
            }

            if(keycount != ST_KEYS_NUMROWS)
                y += ST_KEYS_YSTEP;
            else
            {
                x = ST_KEYS_COL2X;
                y = ST_KEYS_Y;
            }
        }
    }

    return true;
}

//
// ST_DrawExternal
//
// haleyjd 20100901: [STRIFE] New function.
// * Draws external portions of the status bar such the top bar and popups.
//
boolean ST_DrawExternal(void)
{
    int i;

    if(st_statusbaron)
    {
        V_DrawPatchDirect(0, 160, invtop);
        STlib_drawNumPositive(&w_health);
        STlib_drawNumPositive(&w_ready);
    }
    else
    {
        ammotype_t ammo;

        ST_drawNumFontY2(15, 194, plyr->health);
        ammo = weaponinfo[plyr->readyweapon].ammo;
        if (ammo != am_noammo)
            ST_drawNumFontY2(310, 194, plyr->ammo[ammo]);
    }

    if(!st_displaypopup)
        return false;

    // villsa [STRIFE] added 20100926
    if(st_showobjective)
    {
        V_DrawXlaPatch(0, 56, invpbak2);
        V_DrawPatchDirect(0, 56, invpop2);
        M_DialogDimMsg(24, 74, mission_objective, true);
        HUlib_drawYellowText(24, 74, mission_objective);
        ST_drawTime(210, 64, leveltime / TICRATE);
    }
    else
    {
        int keys = 0;

        // villsa [STRIFE] keys popup
        if(st_showkeys || st_popupdisplaytics)
            return ST_drawKeysPopup();

        V_DrawXlaPatch(0, 56, invpbak);
        V_DrawPatchDirect(0, 56, invpop);

        for(i = 0; i < NUMCARDS; i++)
        {
            if(plyr->cards[i])
                keys++;
        }

        ST_drawNumFontY2(261, 132, keys);

         if(plyr->weaponowned[wp_elecbow])
         {
             V_DrawPatchDirect(38, 86, 
                 W_CacheLumpName(DEH_String("CBOWA0"), PU_CACHE));
         }
         if(plyr->weaponowned[wp_rifle])
         {
             V_DrawPatchDirect(40, 107, 
                 W_CacheLumpName(DEH_String("RIFLA0"), PU_CACHE));
         }
         if(plyr->weaponowned[wp_missile])
         {
             V_DrawPatchDirect(39, 131, 
                 W_CacheLumpName(DEH_String("MMSLA0"), PU_CACHE));
         }
         if(plyr->weaponowned[wp_hegrenade])
         {
             V_DrawPatchDirect(78, 87, 
                 W_CacheLumpName(DEH_String("GRNDA0"), PU_CACHE));
         }
         if(plyr->weaponowned[wp_flame])
         {
             V_DrawPatchDirect(80, 117, 
                 W_CacheLumpName(DEH_String("FLAMA0"), PU_CACHE));
         }
         if(plyr->weaponowned[wp_mauler])
         {
             V_DrawPatchDirect(75, 142, 
                 W_CacheLumpName(DEH_String("TRPDA0"), PU_CACHE));
         }
         
         // haleyjd 20110213: draw ammo
         for(i = 0; i < NUMAMMO; i++)
         {
             STlib_drawNumPositive(&w_ammo[i]);
             STlib_drawNumPositive(&w_maxammo[i]);
         }

         ST_drawNumFontY2(261, 84,  plyr->accuracy);
         ST_drawNumFontY2(261, 108, plyr->stamina);

         if(plyr->powers[pw_communicator])
         {
             V_DrawPatchDirect(280, 130, 
                 W_CacheLumpName(DEH_String("I_COMM"), PU_CACHE));
         }
    }

    return true;
}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable); 

//
// ST_loadUnloadGraphics
//
// Iterates through all graphics to be loaded or unloaded, along with
// the variable they use, invoking the specified callback function.
//
// [STRIFE] Altered to load all Strife status bar resources.
//
static void ST_loadUnloadGraphics(load_callback_t callback)
{
    int         i;
    char        namebuf[9];

    // haleyjd 20100901: [STRIFE]
    // Load the numbers, green and yellow
    for (i=0;i<10;i++)
    {
        DEH_snprintf(namebuf, 9, "INVFONG%d", i);
        callback(namebuf, &invfontg[i]);

        DEH_snprintf(namebuf, 9, "INVFONY%d", i);
        callback(namebuf, &invfonty[i]);
    }

    // haleyjd 20100919: load Sigil patches
    if(!isdemoversion)
    {
        for(i = 0; i < 5; i++)
        {
            DEH_snprintf(namebuf, 9, "I_SGL%d", i+1);
            callback(namebuf, &invsigil[i]);
        }
    }

    // load ammo patches
    for(i = 0; i < NUMAMMO; i++)
        callback(DEH_String(invammonames[i]), &invammo[i]);

    // load armor patches
    callback(DEH_String("I_ARM2"), &invarmor[0]);
    callback(DEH_String("I_ARM1"), &invarmor[1]);

    // haleyjd 20100919: [STRIFE] 
    // * No face, but there is this patch, which appears behind the armor
    DEH_snprintf(namebuf, 9, "STBACK0%d", consoleplayer + 1);
    if(netgame)
        callback(namebuf, &stback);
    
    // 20100901:
    // * Removed all unused DOOM stuff (arms, numbers, %, etc).

    // haleyjd 20100901: [STRIFE]: stbar -> invback, added new patches
    // status bar background bits
    callback(DEH_String("INVBACK"),  &invback);
    callback(DEH_String("INVTOP"),   &invtop);
    callback(DEH_String("INVPOP"),   &invpop);
    callback(DEH_String("INVPOP2"),  &invpop2);
    callback(DEH_String("INVPBAK"),  &invpbak);
    callback(DEH_String("INVPBAK2"), &invpbak2);
    callback(DEH_String("INVCURS"),  &invcursor);
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
//    static int dword_8848C = 1; // STRIFE-TODO: what is the purpose of this?
//    dword_8848C = 0;

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
// haleyjd 20100901: [STRIFE]
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

    // haleyjd 20100831: [STRIFE] 
    // * No face.
    // 20100901:
    // * No arms, weaponsowned, frags, armor, keyboxes

    // haleyjd 20110213: Ammo Widgets!!!
    for(i = 0; i < NUMAMMO; i++)
    {
        STlib_initNum(&w_ammo[i], ST_POPUPAMMOX, st_yforammo[i], 
                      invfonty, &plyr->ammo[i], st_wforammo[i]);

        STlib_initNum(&w_maxammo[i], ST_POPUPMAXAMMOX, st_yforammo[i],
                      invfonty, &plyr->maxammo[i], st_wforammo[i]);
    }
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

    // haleyjd 20100919: This is not used by Strife. More memory for voices!
    //st_backing_screen = (byte *) Z_Malloc(ST_WIDTH * ST_HEIGHT, PU_STATIC, 0);
}

