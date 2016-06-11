//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2010 James Haley, Samuel Villarreal
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
//
// [STRIFE] New Module
//
// Dialog Engine for Strife
//

#include <stdlib.h>

#include "z_zone.h"
#include "w_wad.h"
#include "deh_str.h"
#include "d_main.h"
#include "d_mode.h"
#include "d_player.h"
#include "doomstat.h"
#include "m_random.h"
#include "m_menu.h"
#include "m_misc.h"
#include "r_main.h"
#include "v_video.h"
#include "p_local.h"
#include "sounds.h"
#include "p_dialog.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_inter.h"

//
// Defines and Macros
//

// haleyjd: size of the original Strife mapdialog_t structure.
#define ORIG_MAPDIALOG_SIZE 0x5EC

#define DIALOG_INT(field, ptr)    \
    field = ((int)ptr[0]        | \
            ((int)ptr[1] <<  8) | \
            ((int)ptr[2] << 16) | \
            ((int)ptr[3] << 24)); \
    ptr += 4;

#define DIALOG_STR(field, ptr, len) \
    memcpy(field, ptr, len);        \
    ptr += len;

//
// Globals
//

// This can be toggled at runtime to determine if the full dialog messages
// are subtitled on screen or not. Defaults to off.
int dialogshowtext = false;

// The global mission objective buffer. This gets written to and read from file,
// and is set by dialogs and line actions.
char mission_objective[OBJECTIVE_LEN];

//
// Static Globals
//

// True if SCRIPT00 is loaded.
static boolean script0loaded;

// Number of dialogs defined in the current level's script.
static int numleveldialogs;

// The actual level dialogs. This didn't exist in Strife, but is new to account
// for structure alignment/packing concerns, given that Chocolate Doom is
// multiplatform.
static mapdialog_t *leveldialogs;

// The actual script00 dialogs. As above.
static mapdialog_t *script0dialogs;

// Number of dialogs defined in the SCRIPT00 lump.
static int numscript0dialogs;

// The player engaged in dialog. This is always player 1, though, since Rogue
// never completed the ability to use dialog outside of single-player mode.
static player_t *dialogplayer;

// The object to which the player is speaking.
static mobj_t   *dialogtalker;

// The talker's current angle
static angle_t dialogtalkerangle;

// The currently active mapdialog object.
static mapdialog_t *currentdialog;

// Text at the end of the choices
static char dialoglastmsgbuffer[48];

// Item to display to player when picked up or recieved
static char pickupstring[46];

// Health based on gameskill given by the front's medic
static const int healthamounts[] = { -100 , -75, -50, -50, -100 };

//=============================================================================
//
// Dialog State Sets
//
// These are used to animate certain actors in response to what happens in
// their dialog sequences.
//

typedef struct dialogstateset_s
{
    mobjtype_t type;  // the type of object
    statenum_t greet; // greeting state, for start of dialog
    statenum_t yes;   // "yes" state, for an affirmative response
    statenum_t no;    // "no" state, when you don't have the right items
} dialogstateset_t;

static dialogstateset_t dialogstatesets[] =
{
    { MT_PLAYER,       S_NULL,    S_NULL,    S_NULL    },
    { MT_SHOPKEEPER_W, S_MRGT_00, S_MRYS_00, S_MRNO_00 },
    { MT_SHOPKEEPER_B, S_MRGT_00, S_MRYS_00, S_MRNO_00 },
    { MT_SHOPKEEPER_A, S_MRGT_00, S_MRYS_00, S_MRNO_00 },
    { MT_SHOPKEEPER_M, S_MRGT_00, S_MRYS_00, S_MRNO_00 }
};

// Rogue stored this in a static global rather than making it a define...
static int numdialogstatesets = arrlen(dialogstatesets);

// Current dialog talker state
static dialogstateset_t *dialogtalkerstates;

//=============================================================================
//
// Random Messages
//
// Rogue hard-coded these so they wouldn't have to repeat them several times
// in the SCRIPT00 lump, apparently.
//

#define MAXRNDMESSAGES 10

typedef struct rndmessage_s
{
    const char *type_name;
    int nummessages;
    char *messages[MAXRNDMESSAGES];
} rndmessage_t;

static rndmessage_t rndMessages[] = 
{
    // Peasants
    {
        "PEASANT",
        10,
        {
            "PLEASE DON'T HURT ME.",
            
            "IF YOU'RE LOOKING TO HURT ME, I'M \n"
            "NOT REALLY WORTH THE EFFORT.",
            
            "I DON'T KNOW ANYTHING.",
            
            "GO AWAY OR I'LL CALL THE GUARDS!",
            
            "I WISH SOMETIMES THAT ALL THESE \n"
            "REBELS WOULD JUST LEARN THEIR \n"
            "PLACE AND STOP THIS NONSENSE.",

            "JUST LEAVE ME ALONE, OK?",

            "I'M NOT SURE, BUT SOMETIMES I THINK \n"
            "THAT I KNOW SOME OF THE ACOLYTES.",

            "THE ORDER'S GOT EVERYTHING AROUND HERE PRETTY WELL LOCKED UP TIGHT.",

            "THERE'S NO WAY THAT THIS IS JUST A \n"
            "SECURITY FORCE.",

            "I'VE HEARD THAT THE ORDER IS REALLY \n"
            "NERVOUS ABOUT THE FRONT'S \n"
            "ACTIONS AROUND HERE."
        }
    },
    // Rebel
    {
        "REBEL",
        10,
        {
            "THERE'S NO WAY THE ORDER WILL \n"
            "STAND AGAINST US.",

            "WE'RE ALMOST READY TO STRIKE. \n"
            "MACIL'S PLANS ARE FALLING IN PLACE.",

            "WE'RE ALL BEHIND YOU, DON'T WORRY.",

            "DON'T GET TOO CLOSE TO ANY OF THOSE BIG ROBOTS. THEY'LL MELT YOU DOWN \n"
            "FOR SCRAP!",

            "THE DAY OF OUR GLORY WILL SOON \n"
            "COME, AND THOSE WHO OPPOSE US WILL \n"
            "BE CRUSHED!",

            "DON'T GET TOO COMFORTABLE. WE'VE \n"
            "STILL GOT OUR WORK CUT OUT FOR US.",

            "MACIL SAYS THAT YOU'RE THE NEW \n"
            "HOPE. BEAR THAT IN MIND.",

            "ONCE WE'VE TAKEN THESE CHARLATANS DOWN, WE'LL BE ABLE TO REBUILD THIS "
            "WORLD AS IT SHOULD BE.",

            "REMEMBER THAT YOU AREN'T FIGHTING \n"
            "JUST FOR YOURSELF, BUT FOR \n"
            "EVERYONE HERE AND OUTSIDE.",

            "AS LONG AS ONE OF US STILL STANDS, \n"
            "WE WILL WIN."
        }
    },
    // Acolyte
    {
        "AGUARD",
        10,
        {
            "MOVE ALONG,  PEASANT.",

            "FOLLOW THE TRUE FAITH, ONLY THEN \n"
            "WILL YOU BEGIN TO UNDERSTAND.",

            "ONLY THROUGH DEATH CAN ONE BE \n"
            "TRULY REBORN.",

            "I'M NOT INTERESTED IN YOUR USELESS \n"
            "DRIVEL.",

            "IF I HAD WANTED TO TALK TO YOU I \n"
            "WOULD HAVE TOLD YOU SO.",

            "GO AND ANNOY SOMEONE ELSE!",

            "KEEP MOVING!",

            "IF THE ALARM GOES OFF, JUST STAY OUT OF OUR WAY!",

            "THE ORDER WILL CLEANSE THE WORLD \n"
            "AND USHER IT INTO THE NEW ERA.",

            "PROBLEM?  NO, I THOUGHT NOT.",
        }
    },
    // Beggar
    {
        "BEGGAR",
        10,
        {
            "ALMS FOR THE POOR?",

            "WHAT ARE YOU LOOKING AT, SURFACER?",

            "YOU WOULDN'T HAVE ANY EXTRA FOOD, WOULD YOU?",

            "YOU  SURFACE PEOPLE WILL NEVER \n"
            "                                                                 "
            "                                      UNDERSTAND US.",

            "HA, THE GUARDS CAN'T FIND US.  THOSE \n"
            "IDIOTS DON'T EVEN KNOW WE EXIST.",

            "ONE DAY EVERYONE BUT THOSE WHO SERVE THE ORDER WILL BE FORCED TO "
            "  JOIN US.",

            "STARE NOW,  BUT YOU KNOW THAT THIS WILL BE YOUR OWN FACE ONE DAY.",

            // Note: "NOTHING THING" is an authentic typo
            "THERE'S NOTHING THING MORE \n"
            "ANNOYING THAN A SURFACER WITH AN ATTITUDE!",

            "THE ORDER WILL MAKE SHORT WORK OF YOUR PATHETIC FRONT.",

            "WATCH YOURSELF SURFACER. WE KNOW OUR ENEMIES!"
        }
    },
    // Templar
    {
        "PGUARD",
        10,
        {
            "WE ARE THE HANDS OF FATE. TO EARN \n"
            "OUR WRATH IS TO FIND OBLIVION!",

            "THE ORDER WILL CLEANSE THE WORLD \n"
            "OF THE WEAK AND CORRUPT!",

            "OBEY THE WILL OF THE MASTERS!",

            "LONG LIFE TO THE BROTHERS OF THE \n"
            "ORDER!",

            "FREE WILL IS AN ILLUSION THAT BINDS \n"
            "THE WEAK MINDED.",

            "POWER IS THE PATH TO GLORY. TO \n"
            "FOLLOW THE ORDER IS TO WALK THAT \n"
            "PATH!",

            "TAKE YOUR PLACE AMONG THE \n"
            "RIGHTEOUS, JOIN US!",

            "THE ORDER PROTECTS ITS OWN.",

            "ACOLYTES?  THEY HAVE YET TO SEE THE FULL GLORY OF THE ORDER.",

            "IF THERE IS ANY HONOR INSIDE THAT \n"
            "PATHETIC SHELL OF A BODY, \n"
            "YOU'LL ENTER INTO THE ARMS OF THE \n"
            "ORDER."
        }
    }
};

// And again, this could have been a define, but was a variable.
static int numrndmessages = arrlen(rndMessages);

//=============================================================================
//
// Dialog Menu Structure
//
// The Strife dialog system is actually just a serious abuse of the DOOM menu
// engine. Hence why it doesn't work in multiplayer games or during demo
// recording.
//

#define NUMDIALOGMENUITEMS 6

static void P_DialogDrawer(void);

static menuitem_t dialogmenuitems[] =
{
    { 1, "", P_DialogDoChoice, '1' }, // These items are loaded dynamically
    { 1, "", P_DialogDoChoice, '2' },
    { 1, "", P_DialogDoChoice, '3' },
    { 1, "", P_DialogDoChoice, '4' },
    { 1, "", P_DialogDoChoice, '5' },
    { 1, "", P_DialogDoChoice, '6' }  // Item 6 is always the dismissal item
};

static menu_t dialogmenu =
{
    NUMDIALOGMENUITEMS, 
    NULL, 
    dialogmenuitems, 
    P_DialogDrawer, 
    42, 
    75, 
    0
};

// Lump number of the dialog background picture, if any.
static int dialogbgpiclumpnum;

// Name of current speaking character.
static char *dialogname;

// Current dialog text.
static const char *dialogtext;

//=============================================================================
//
// Routines
//

//
// P_ParseDialogLump
//
// haleyjd 09/02/10: This is an original function added to parse out the 
// dialogs from the dialog lump rather than reading them raw from the lump 
// pointer. This avoids problems with structure packing.
//
static void P_ParseDialogLump(byte *lump, mapdialog_t **dialogs, 
                              int numdialogs, int tag)
{
    int i;
    byte *rover = lump;

    *dialogs = Z_Malloc(numdialogs * sizeof(mapdialog_t), tag, NULL);

    for(i = 0; i < numdialogs; i++)
    {
        int j;
        mapdialog_t *curdialog = &((*dialogs)[i]);

        DIALOG_INT(curdialog->speakerid,    rover);
        DIALOG_INT(curdialog->dropitem,     rover);
        DIALOG_INT(curdialog->checkitem[0], rover);
        DIALOG_INT(curdialog->checkitem[1], rover);
        DIALOG_INT(curdialog->checkitem[2], rover);
        DIALOG_INT(curdialog->jumptoconv,   rover);
        DIALOG_STR(curdialog->name,         rover, MDLG_NAMELEN);
        DIALOG_STR(curdialog->voice,        rover, MDLG_LUMPLEN);
        DIALOG_STR(curdialog->backpic,      rover, MDLG_LUMPLEN);
        DIALOG_STR(curdialog->text,         rover, MDLG_TEXTLEN);

        // copy choices
        for(j = 0; j < 5; j++)
        {
            mapdlgchoice_t *curchoice = &(curdialog->choices[j]);
            DIALOG_INT(curchoice->giveitem,         rover);
            DIALOG_INT(curchoice->needitems[0],     rover);
            DIALOG_INT(curchoice->needitems[1],     rover);
            DIALOG_INT(curchoice->needitems[2],     rover);
            DIALOG_INT(curchoice->needamounts[0],   rover);
            DIALOG_INT(curchoice->needamounts[1],   rover);
            DIALOG_INT(curchoice->needamounts[2],   rover);
            DIALOG_STR(curchoice->text,             rover, MDLG_CHOICELEN);
            DIALOG_STR(curchoice->textok,           rover, MDLG_MSGLEN);
            DIALOG_INT(curchoice->next,             rover);
            DIALOG_INT(curchoice->objective,        rover);
            DIALOG_STR(curchoice->textno,           rover, MDLG_MSGLEN);
        }
    }
}

//
// P_DialogLoad
//
// [STRIFE] New function
// haleyjd 09/02/10: Loads the dialog script for the current map. Also loads 
// SCRIPT00 if it has not yet been loaded.
//
void P_DialogLoad(void)
{
    char lumpname[9];
    int  lumpnum;

    // load the SCRIPTxy lump corresponding to MAPxy, if it exists.
    DEH_snprintf(lumpname, sizeof(lumpname), "script%02d", gamemap);
    if((lumpnum = W_CheckNumForName(lumpname)) == -1)
        numleveldialogs = 0;
    else
    {
        byte *leveldialogptr = W_CacheLumpNum(lumpnum, PU_STATIC);
        numleveldialogs = W_LumpLength(lumpnum) / ORIG_MAPDIALOG_SIZE;
        P_ParseDialogLump(leveldialogptr, &leveldialogs, numleveldialogs, 
                          PU_LEVEL);
        Z_Free(leveldialogptr); // haleyjd: free the original lump
    }

    // also load SCRIPT00 if it has not been loaded yet
    if(!script0loaded)
    {
        byte *script0ptr;

        script0loaded = true; 
        // BUG: Rogue should have used W_GetNumForName here...
        lumpnum = W_CheckNumForName(DEH_String("script00")); 
        script0ptr = W_CacheLumpNum(lumpnum, PU_STATIC);
        numscript0dialogs = W_LumpLength(lumpnum) / ORIG_MAPDIALOG_SIZE;
        P_ParseDialogLump(script0ptr, &script0dialogs, numscript0dialogs,
                          PU_STATIC);
        Z_Free(script0ptr); // haleyjd: free the original lump
    }
}

//
// P_PlayerHasItem
//
// [STRIFE] New function
// haleyjd 09/02/10: Checks for inventory items, quest flags, etc. for dialogs.
// Returns the amount possessed, or 0 if none.
//
int P_PlayerHasItem(player_t *player, mobjtype_t type)
{
    int i;

    if(type > 0)
    {
        // check keys
        if(type >= MT_KEY_BASE && type < MT_INV_SHADOWARMOR)
            return (player->cards[type - MT_KEY_BASE]);

        // check sigil pieces
        if(type >= MT_SIGIL_A && type <= MT_SIGIL_E)
            return (type - MT_SIGIL_A <= player->sigiltype);

        // check quest tokens
        if(type >= MT_TOKEN_QUEST1 && type <= MT_TOKEN_QUEST31)
            return (player->questflags & (1 << (type - MT_TOKEN_QUEST1)));

        // check inventory
        for(i = 0; i < 32; i++)
        {
            if(type == player->inventory[i].type)
                return player->inventory[i].amount;
        }
    }
    return 0;
}

//
// P_DialogFind
//
// [STRIFE] New function
// haleyjd 09/03/10: Looks for a dialog definition matching the given 
// Script ID # for an mobj.
//
mapdialog_t *P_DialogFind(mobjtype_t type, int jumptoconv)
{
    int i;

    // check the map-specific dialogs first
    for(i = 0; i < numleveldialogs; i++)
    {
        if(type == leveldialogs[i].speakerid)
        {
            if(jumptoconv <= 1)
                return &leveldialogs[i];
            else
                --jumptoconv;
        }
    }

    // check SCRIPT00 dialogs next
    for(i = 0; i < numscript0dialogs; i++)
    {
        if(type == script0dialogs[i].speakerid)
            return &script0dialogs[i];
    }

    // the default dialog is script 0 in the SCRIPT00 lump.
    return &script0dialogs[0];
}

//
// P_DialogGetStates
//
// [STRIFE] New function
// haleyjd 09/03/10: Find the set of special dialog states (greetings, yes, no)
// for a particular thing type.
//
static dialogstateset_t *P_DialogGetStates(mobjtype_t type)
{
    int i;

    // look for a match by type
    for(i = 0; i < numdialogstatesets; i++)
    {
        if(type == dialogstatesets[i].type)
            return &dialogstatesets[i];
    }

    // return the default 0 record if no match.
    return &dialogstatesets[0];
}

//
// P_DialogGetMsg
//
// [STRIFE] New function
// haleyjd 09/03/10: Redirects dialog messages when the script indicates that
// the actor should use a random message stored in the executable instead.
//
static const char *P_DialogGetMsg(const char *message)
{
    // if the message starts with "RANDOM"...
    if(!strncasecmp(message, DEH_String("RANDOM"), 6))
    {
        int i;
        const char *nameloc = message + 7;

        // look for a match in rndMessages for the string starting 
        // 7 chars after "RANDOM_"
        for(i = 0; i < numrndmessages; i++)
        {
            if(!strncasecmp(nameloc, rndMessages[i].type_name, 4))
            {
                // found a match, so return a random message
                int rnd = M_Random();
                int nummessages = rndMessages[i].nummessages;
                return DEH_String(rndMessages[i].messages[rnd % nummessages]);
            }
        }
    }

    // otherwise, just return the message passed in.
    return message;
}

//
// P_GiveInventoryItem
//
// [STRIFE] New function
// haleyjd 09/03/10: Give an inventory item to the player, if possible.
// villsa 09/09/10: Fleshed out routine
//
boolean P_GiveInventoryItem(player_t *player, int sprnum, mobjtype_t type)
{
    int curinv = 0;
    int i;
    boolean ok = false;
    mobjtype_t item = 0;
    inventory_t* invtail;

    // repaint the status bar due to inventory changing
    player->st_update = true;

    while(1)
    {
        // inventory is full
        if(curinv > player->numinventory)
            return true;

        item = player->inventory[curinv].type;
        if(type < item)
        {
            if(curinv != MAXINVENTORYSLOTS)
            {
                // villsa - sort inventory item if needed
                invtail = &player->inventory[player->numinventory - 1];
                if(player->numinventory >= (curinv + 1))
                {
                    for(i = player->numinventory; i >= (curinv + 1); --i)    
                    {
                        invtail[1].sprite   = invtail[0].sprite;
                        invtail[1].type     = invtail[0].type;
                        invtail[1].amount   = invtail[0].amount;

                        invtail--;
                    }
                }

                // villsa - add inventory item
                player->inventory[curinv].amount = 1;
                player->inventory[curinv].sprite = sprnum;
                player->inventory[curinv].type = type;

                // sort cursor if needed
                if(player->numinventory)
                {
                    if(curinv <= player->inventorycursor)
                        player->inventorycursor++;
                }

                player->numinventory++;

                return true;
            }

            return false;
        }

        if(type == item)
            break;

        curinv++;
    }

    // check amount of inventory item by using the mass from mobjinfo
    if(player->inventory[curinv].amount < mobjinfo[item].mass)
    {
        player->inventory[curinv].amount++;
        ok = true;
    }
    else
        ok = false;

    return ok;
}

//
// P_GiveItemToPlayer
//
// [STRIFE] New function
// haleyjd 09/03/10: Sorts out how to give something to the player.
// Not strictly just for inventory items.
// villsa 09/09/10: Fleshed out function
//
boolean P_GiveItemToPlayer(player_t *player, int sprnum, mobjtype_t type)
{
    int i = 0;
    line_t junk;
    int sound = sfx_itemup; // haleyjd 09/21/10: different sounds for items

    // set quest if mf_givequest flag is set
    if(mobjinfo[type].flags & MF_GIVEQUEST)
        player->questflags |= 1 << (mobjinfo[type].speed - 1);

    // check for keys
    if(type >= MT_KEY_BASE && type <= MT_NEWKEY5)
    {
        P_GiveCard(player, type - MT_KEY_BASE);
        return true;
    }

    // check for quest tokens
    if(type >= MT_TOKEN_QUEST1 && type <= MT_TOKEN_QUEST31)
    {
        if(mobjinfo[type].name)
        {
            M_StringCopy(pickupstring, DEH_String(mobjinfo[type].name), 39);
            player->message = pickupstring;
        }
        player->questflags |= 1 << (type - MT_TOKEN_QUEST1);

        if(player == &players[consoleplayer])
            S_StartSound(NULL, sound);
        return true;
    }

    // haleyjd 09/22/10: Refactored to give sprites higher priority than
    // mobjtypes and to implement missing logic.
    switch(sprnum)
    {
    case SPR_HELT: // This is given only by the "DONNYTRUMP" cheat (aka Midas)
        P_GiveInventoryItem(player, SPR_HELT, MT_TOKEN_TOUGHNESS);
        P_GiveInventoryItem(player, SPR_GUNT, MT_TOKEN_ACCURACY);

        // [STRIFE] Bizarre...
        for(i = 0; i < 5 * player->accuracy + 300; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_ARM1: // Armor 1
        if(!P_GiveArmor(player, -2))
            P_GiveInventoryItem(player, sprnum, type);
        break;

    case SPR_ARM2: // Armor 2
        if(!P_GiveArmor(player, -1))
            P_GiveInventoryItem(player, sprnum, type);
        break;

    case SPR_COIN: // 1 Gold
        P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_CRED: // 10 Gold
        for(i = 0; i < 10; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_SACK: // 25 gold
        for(i = 0; i < 25; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_CHST: // 50 gold
        for(i = 0; i < 50; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break; // haleyjd 20141215: missing break, caused Rowan to not take ring from you.

    case SPR_BBOX: // Box of Bullets
        if(!P_GiveAmmo(player, am_bullets, 5))
            return false;
        break;

    case SPR_BLIT: // Bullet Clip
        if(!P_GiveAmmo(player, am_bullets, 1))
            return false;
        break;

    case SPR_PMAP: // Map powerup
        if(!P_GivePower(player, pw_allmap))
            return false;
        sound = sfx_yeah; // bluh-doop!
        break;

    case SPR_COMM: // Communicator
        if(!P_GivePower(player, pw_communicator))
            return false;
        sound = sfx_yeah; // bluh-doop!
        break;

    case SPR_MSSL: // Mini-missile
        if(!P_GiveAmmo(player, am_missiles, 1))
            return false;
        break;

    case SPR_ROKT: // Crate of missiles
        if(!P_GiveAmmo(player, am_missiles, 5))
            return false;
        break;

    case SPR_BRY1: // Battery cell
        if(!P_GiveAmmo(player, am_cell, 1))
            return false;
        break;

    case SPR_CPAC: // Cell pack
        if(!P_GiveAmmo(player, am_cell, 5))
            return false;
        break;

    case SPR_PQRL: // Poison bolts
        if(!P_GiveAmmo(player, am_poisonbolts, 5))
            return false;
        break;

    case SPR_XQRL: // Electric bolts
        if(!P_GiveAmmo(player, am_elecbolts, 5))
            return false;
        break;

    case SPR_GRN1: // HE Grenades
        if(!P_GiveAmmo(player, am_hegrenades, 1))
            return false;
        break;

    case SPR_GRN2: // WP Grenades
        if(!P_GiveAmmo(player, am_wpgrenades, 1))
            return false;
        break;

    case SPR_BKPK: // Backpack (aka Ammo Satchel)
        if(!player->backpack)
        {
            for(i = 0; i < NUMAMMO; i++)
                player->maxammo[i] *= 2;

            player->backpack = true;
        }
        for(i = 0; i < NUMAMMO; i++)
            P_GiveAmmo(player, i, 1);
        break;

    case SPR_RIFL: // Assault Rifle
        if(player->weaponowned[wp_rifle])
            return false;

        if(!P_GiveWeapon(player, wp_rifle, false))
            return false;
        
        sound = sfx_wpnup; // SHK-CHK!
        break;

    case SPR_FLAM: // Flamethrower
        if(player->weaponowned[wp_flame])
            return false;

        if(!P_GiveWeapon(player, wp_flame, false))
            return false;

        sound = sfx_wpnup; // SHK-CHK!
        break;

    case SPR_MMSL: // Mini-missile Launcher
        if(player->weaponowned[wp_missile])
            return false;

        if(!P_GiveWeapon(player, wp_missile, false))
            return false;

        sound = sfx_wpnup; // SHK-CHK!
        break;

    case SPR_TRPD: // Mauler
        if(player->weaponowned[wp_mauler])
            return false;

        if(!P_GiveWeapon(player, wp_mauler, false))
            return false;

        sound = sfx_wpnup; // SHK-CHK!
        break;

    case SPR_CBOW: // Here's a crossbow. Just aim straight, and *SPLAT!*
        if(player->weaponowned[wp_elecbow])
            return false;

        if(!P_GiveWeapon(player, wp_elecbow, false))
            return false;

        sound = sfx_wpnup; // SHK-CHK!
        break;

    case SPR_TOKN: // Miscellaneous items - These are determined by thingtype.
        switch(type)
        {
        case MT_KEY_HAND: // Severed hand
            P_GiveCard(player, key_SeveredHand);
            break;

        case MT_MONY_300: // 300 Gold (this is the only way to get it, in fact)
            for(i = 0; i < 300; i++)
                P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
            break;

        case MT_TOKEN_AMMO: // Ammo token - you get this from the Weapons Trainer
            if(player->ammo[am_bullets] >= 50)
                return false;

            player->ammo[am_bullets] = 50;
            break;

        case MT_TOKEN_HEALTH: // Health token - from the Front's doctor
            if(!P_GiveBody(player, healthamounts[gameskill]))
                return false;
            break;

        case MT_TOKEN_ALARM: // Alarm token - particularly from the Oracle.
            P_NoiseAlert(player->mo, player->mo);
            A_AlertSpectreC(dialogtalker); // BUG: assumes in a dialog o_O
            break;

        case MT_TOKEN_DOOR1: // Door special 1
            junk.tag = 222;
            EV_DoDoor(&junk, vld_open);
            break;

        case MT_TOKEN_PRISON_PASS: // Door special 1 - Prison pass
            junk.tag = 223;
            EV_DoDoor(&junk, vld_open);
            if(gamemap == 2) // If on Tarnhill, give Prison pass object
                P_GiveInventoryItem(player, sprnum, type);
            break;

        case MT_TOKEN_SHOPCLOSE: // Door special 3 - "Shop close" - unused?
            junk.tag = 222;
            EV_DoDoor(&junk, vld_close);
            break;

        case MT_TOKEN_DOOR3: // Door special 4 (or 3? :P ) 
            junk.tag = 224;
            EV_DoDoor(&junk, vld_close);
            break;

        case MT_TOKEN_STAMINA: // Stamina upgrade
            if(player->stamina >= 100)
                return false;

            player->stamina += 10;
            P_GiveBody(player, 200); // full healing
            break;

        case MT_TOKEN_NEW_ACCURACY: // Accuracy upgrade
            if(player->accuracy >= 100)
                return false;

            player->accuracy += 10;
            break;

        case MT_SLIDESHOW: // Slideshow (start a finale)
            gameaction = ga_victory;
            if(gamemap == 10)
                P_GiveItemToPlayer(player, SPR_TOKN, MT_TOKEN_QUEST17);
            break;
        
        default: // The default is to just give it as an inventory item.
            P_GiveInventoryItem(player, sprnum, type);
            break;
        }
        break;

    default: // The ultimate default: Give it as an inventory item.
        if(!P_GiveInventoryItem(player, sprnum, type))
            return false;
        break;
    }

    // Play sound.
    if(player == &players[consoleplayer])
        S_StartSound(NULL, sound);

    return true;
}

//
// P_TakeDialogItem
//
// [STRIFE] New function
// haleyjd 09/03/10: Removes needed items from the player's inventory.
//
static void P_TakeDialogItem(player_t *player, int type, int amount)
{
    int i;

    if(amount <= 0)
        return;

    for(i = 0; i < player->numinventory; i++)
    {
        // find a matching item
        if(type != player->inventory[i].type)
            continue;

        // if there is none left...
        if((player->inventory[i].amount -= amount) < 1)
        {
            // ...shift everything above it down
            int j;

            // BUG: They should have stopped at j < numinventory. This
            // seems to implicitly assume that numinventory is always at
            // least one less than the max # of slots, otherwise it 
            // pulls in data from the following player_t fields:
            // st_update, numinventory, inventorycursor, accuracy, stamina
            for(j = i + 1; j <= player->numinventory; j++)
            {
                inventory_t *item1 = &(player->inventory[j - 1]);
                inventory_t *item2 = &(player->inventory[j]);

                *item1 = *item2;
            }

            // blank the topmost slot
            // BUG: This will overwrite the aforementioned fields if
            // numinventory is equal to the number of slots!
            // STRIFE-TODO: Overflow emulation?
            player->inventory[player->numinventory].type = NUMMOBJTYPES;
            player->inventory[player->numinventory].sprite = -1;
            player->numinventory--;

            // update cursor position
            if(player->inventorycursor >= player->numinventory)
            {
                if(player->inventorycursor)
                    player->inventorycursor--;
            }
        } // end if
        
        return; // done!

    } // end for
}

//
// P_DialogDrawer
//
// This function is set as the drawer callback for the dialog menu.
//
static void P_DialogDrawer(void)
{
    angle_t angle;
    int y;
    int i;
    int height;
    int finaly;
    char choicetext[64];
    char choicetext2[64];

    // Run down bonuscount faster than usual so that flashes from being given
    // items are less obvious.
    if(dialogplayer->bonuscount)
    {
        dialogplayer->bonuscount -= 3;
        if(dialogplayer->bonuscount < 0)
            dialogplayer->bonuscount = 0;
    }

    angle = R_PointToAngle2(dialogplayer->mo->x,
                            dialogplayer->mo->y,
                            dialogtalker->x,
                            dialogtalker->y);
    angle -= dialogplayer->mo->angle;

    // Dismiss the dialog if the player is out of alignment, or the thing he was
    // talking to is now engaged in battle.
    if ((angle > ANG45 && angle < (ANG270+ANG45))
     || (dialogtalker->flags & MF_NODIALOG) != 0)
    {
        P_DialogDoChoice(dialogmenu.numitems - 1);
    }

    dialogtalker->reactiontime = 2;

    // draw background
    if(dialogbgpiclumpnum != -1)
    {
        patch_t *patch = W_CacheLumpNum(dialogbgpiclumpnum, PU_CACHE);
        V_DrawPatchDirect(0, 0, patch);
    }

    // if there's a valid background pic, delay drawing the rest of the menu 
    // for a while; otherwise, it will appear immediately
    if(dialogbgpiclumpnum == -1 || menupausetime <= gametic)
    {
        if(menuindialog)
        {
            // time to pause the game?
            if(menupausetime + 3 < gametic)
                menupause = true;
        }

        // draw character name
        M_WriteText(12, 18, dialogname);
        y = 28;

        // show text (optional for dialogs with voices)
        if(dialogshowtext || currentdialog->voice[0] == '\0')
            y = M_WriteText(20, 28, dialogtext);

        height = 20 * dialogmenu.numitems;

        finaly = 175 - height;     // preferred height
        if(y > finaly)
            finaly = 199 - height; // height it will bump down to if necessary.

        // draw divider
        M_WriteText(42, finaly - 6, DEH_String("______________________________"));

        dialogmenu.y = finaly + 6;
        y = 0;

        // draw the menu items
        for(i = 0; i < dialogmenu.numitems - 1; i++)
        {
            DEH_snprintf(choicetext, sizeof(choicetext),
                         "%d) %s", i + 1, currentdialog->choices[i].text);
            
            // alternate text for items that need money
            if(currentdialog->choices[i].needamounts[0] > 0)
            {
                // haleyjd 20120401: necessary to avoid undefined behavior:
                M_StringCopy(choicetext2, choicetext, sizeof(choicetext2));
                DEH_snprintf(choicetext, sizeof(choicetext),
                             "%s for %d", choicetext2,
                             currentdialog->choices[i].needamounts[0]);
            }

            M_WriteText(dialogmenu.x, dialogmenu.y + 3 + y, choicetext);
            y += 19;
        }

        // draw the final item for dismissing the dialog
        M_WriteText(dialogmenu.x, 19 * i + dialogmenu.y + 3, dialoglastmsgbuffer);
    }
}

//
// P_DialogDoChoice
//
// [STRIFE] New function
// haleyjd 09/05/10: Handles making a choice in a dialog. Installed as the
// callback for all items in the dialogmenu structure.
//
void P_DialogDoChoice(int choice)
{
    int i = 0, nextdialog = 0;
    boolean candochoice = true;
    char *message = NULL;
    mapdlgchoice_t *currentchoice;

    if(choice == -1)
        choice = dialogmenu.numitems - 1;

    currentchoice = &(currentdialog->choices[choice]);

    I_StartVoice(NULL); // STRIFE-TODO: verify (should stop previous voice I believe)

    // villsa 09/08/10: converted into for loop
    for(i = 0; i < MDLG_MAXITEMS; i++)
    {
        if(P_PlayerHasItem(dialogplayer, currentchoice->needitems[i]) <
                                         currentchoice->needamounts[i])
        {
            candochoice = false; // nope, missing something
        }
    }

    if(choice != dialogmenu.numitems - 1 && candochoice)
    {
        int item;

        message = currentchoice->textok;
        if(dialogtalkerstates->yes)
            P_SetMobjState(dialogtalker, dialogtalkerstates->yes);

        item = currentchoice->giveitem;
        if(item < 0 || 
           P_GiveItemToPlayer(dialogplayer, 
                              states[mobjinfo[item].spawnstate].sprite, 
                              item))
        {
            // if successful, take needed items
            int count = 0;
            // villsa 09/08/10: converted into for loop
            for(count = 0; count < MDLG_MAXITEMS; count++)
            {
                P_TakeDialogItem(dialogplayer, 
                                 currentchoice->needitems[count],
                                 currentchoice->needamounts[count]);
            }
        }
        else
            message = DEH_String("You seem to have enough!");

        // store next dialog into the talking actor
        nextdialog = currentchoice->next;
        if(nextdialog != 0)
            dialogtalker->miscdata = (byte)(abs(nextdialog));
    }
    else
    {
        // not successful
        message = currentchoice->textno;
        if(dialogtalkerstates->no)
            P_SetMobjState(dialogtalker, dialogtalkerstates->no);
    }
    
    if(choice != dialogmenu.numitems - 1)
    {
        int objective;
        char *objlump;

        if((objective = currentchoice->objective))
        {
            DEH_snprintf(mission_objective, OBJECTIVE_LEN, "log%i", objective);
            objlump = W_CacheLumpName(mission_objective, PU_CACHE);
            M_StringCopy(mission_objective, objlump, OBJECTIVE_LEN);
        }
        // haleyjd 20130301: v1.31 hack: if first char of message is a period,
        // clear the player's message. Is this actually used anywhere?
        if(gameversion == exe_strife_1_31 && message[0] == '.')
            message = NULL;
        dialogplayer->message = message;
    }

    dialogtalker->angle = dialogtalkerangle;
    dialogplayer->st_update = true;
    M_ClearMenus(0);

    if(nextdialog >= 0 || gameaction == ga_victory) // Macil hack
        menuindialog = false;
    else
        P_DialogStart(dialogplayer);
}

//
// P_DialogStartP1
//
// [STRIFE] New function
// haleyjd 09/13/10: This is a hack used by the finale system.
//
void P_DialogStartP1(void)
{
    P_DialogStart(&players[0]);
}

//
// P_DialogStart
//
// villsa [STRIFE] New function
//
void P_DialogStart(player_t *player)
{
    int i = 0;
    int pic;
    int rnd = 0;
    char* byetext;
    int jumptoconv;

    if(menuactive || netgame)
        return;

    // are we facing towards our NPC?
    P_AimLineAttack(player->mo, player->mo->angle, (128*FRACUNIT));
    if(!linetarget)
    {
        P_AimLineAttack(player->mo, player->mo->angle + (ANG90/16), (128*FRACUNIT));
        if(!linetarget)
            P_AimLineAttack(player->mo, player->mo->angle - (ANG90/16), (128*FRACUNIT));
    }

    if(!linetarget)
       return;

    // already in combat, can't talk to it
    if(linetarget->flags & MF_NODIALOG)
       return;

    // set pointer to the character talking
    dialogtalker = linetarget;

    // play a sound
    if(player == &players[consoleplayer])
       S_StartSound(0, sfx_radio);

    linetarget->target = player->mo;         // target the player
    dialogtalker->reactiontime = 2;          // set reactiontime
    dialogtalkerangle = dialogtalker->angle; // remember original angle

    // face talker towards player
    A_FaceTarget(dialogtalker);

    // face towards NPC's direction
    player->mo->angle = R_PointToAngle2(player->mo->x,
                                        player->mo->y,
                                        dialogtalker->x,
                                        dialogtalker->y);
    // set pointer to player talking
    dialogplayer = player;

    // haleyjd 09/08/10: get any stored dialog state from this object
    jumptoconv = linetarget->miscdata;

    // check item requirements
    while(1)
    {
        int i = 0;
        currentdialog = P_DialogFind(linetarget->type, jumptoconv);

        // dialog's jumptoconv equal to 0? There's nothing to jump to.
        if(currentdialog->jumptoconv == 0)
            break;

        // villsa 09/08/10: converted into for loop
        for(i = 0; i < MDLG_MAXITEMS; i++)
        {
            // if the item is non-zero, the player must have at least one in his
            // or her inventory
            if(currentdialog->checkitem[i] != 0 &&
                P_PlayerHasItem(dialogplayer, currentdialog->checkitem[i]) < 1)
                break;
        }

        if(i < MDLG_MAXITEMS) // didn't find them all? this is our dialog!
            break;

        jumptoconv = currentdialog->jumptoconv;
    }

    M_DialogDimMsg(20, 28, currentdialog->text, false);
    dialogtext = P_DialogGetMsg(currentdialog->text);

    // get states
    dialogtalkerstates = P_DialogGetStates(linetarget->type);

    // have talker greet the player
    if(dialogtalkerstates->greet)
        P_SetMobjState(dialogtalker, dialogtalkerstates->greet);

    // get talker's name
    if(currentdialog->name[0])
        dialogname = currentdialog->name;
    else
    {
        // use a fallback:
        if(mobjinfo[linetarget->type].name)
            dialogname = DEH_String(mobjinfo[linetarget->type].name); // mobjtype name
        else
            dialogname = DEH_String("Person"); // default name - like Joe in Doom 3 :P
    }

    // setup number of choices to choose from
    for(i = 0; i < MDLG_MAXCHOICES; i++)
    {
        if(!currentdialog->choices[i].giveitem)
            break;
    }

    // set number of choices to menu
    dialogmenu.numitems = i + 1;

    rnd = M_Random() % 3;

    // setup dialog menu
    M_StartControlPanel();
    menupause = false;
    menuindialog = true;
    menupausetime = gametic + 17;
    currentMenu = &dialogmenu;

    if(i >= dialogmenu.lastOn)
        itemOn = dialogmenu.lastOn;
    else
        itemOn = 0;

    // get backdrop
    pic = W_CheckNumForName(currentdialog->backpic);
    dialogbgpiclumpnum = pic;
    if(pic != -1)
        V_DrawPatchDirect(0, 0, W_CacheLumpNum(pic, PU_CACHE));

    // get voice
    I_StartVoice(currentdialog->voice);

    // get bye text
    switch(rnd)
    {
    case 2:
        byetext = DEH_String("BYE!");
        break;
    case 1:
        byetext = DEH_String("Thanks, Bye!");
        break;
    default:
    case 0:
        byetext = DEH_String("See you later!");
        break;
    }

    DEH_snprintf(dialoglastmsgbuffer, sizeof(dialoglastmsgbuffer),
                 "%d) %s", i + 1, byetext);
}

// EOF


