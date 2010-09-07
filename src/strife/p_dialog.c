// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1996 Rogue Entertainment / Velocity, Inc.
// Copyright(C) 2010 James Haley, Samuel Villareal
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
//
// [STRIFE] New Module
//
// Dialog Engine for Strife
//
//-----------------------------------------------------------------------------

#include "z_zone.h"
#include "w_wad.h"
#include "deh_str.h"
#include "d_player.h"
#include "doomstat.h"
#include "m_random.h"
#include "m_menu.h"
#include "r_main.h"
#include "v_video.h"
#include "p_local.h"
#include "sounds.h"
#include "p_dialog.h"

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
boolean dialogshowtext = false;

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
player_t *dialogplayer;

// The object to which the player is speaking.
mobj_t   *dialogtalker;

// The talker's current angle
angle_t dialogtalkerangle;

// The currently active mapdialog object.
static mapdialog_t *currentdialog;

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
    { MT_PLAYER,       S_NULL,     S_NULL,   S_NULL    },
    { MT_SHOPKEEPER_W, S_MRGT_00, S_MRYS_00, S_MRNO_00 },
    { MT_SHOPKEEPER_B, S_MRGT_00, S_MRYS_00, S_MRNO_00 },
    { MT_SHOPKEEPER_A, S_MRGT_00, S_MRYS_00, S_MRNO_00 },
    { MT_SHOPKEEPER_M, S_MRGT_00, S_MRYS_00, S_MRNO_00 }
};

// Rogue stored this in a static global rather than making it a define...
static int numdialogstatesets = arrlen(dialogstatesets);

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
    const char *messages[MAXRNDMESSAGES];
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
            "MACIL'S PLANES ARE FALLING IN PLACE.",

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

            "IF I HAD WANTED TO TALK YOU I \n"
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

            "STARE NOW, BUT YOU KNOW THAT THIS WILL BE YOUR OWN FACE ONE DAY.",

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
static void P_DialogDoChoice(int choice);

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
static char *dialogtext;

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

        DIALOG_INT(curdialog->speakerid,  rover);
        DIALOG_INT(curdialog->dropitem,   rover);
        DIALOG_INT(curdialog->checkitem1, rover);
        DIALOG_INT(curdialog->checkitem2, rover);
        DIALOG_INT(curdialog->checkitem3, rover);
        DIALOG_INT(curdialog->jumptoconv, rover);
        DIALOG_STR(curdialog->name,       rover, MDLG_NAMELEN);
        DIALOG_STR(curdialog->voice,      rover, MDLG_LUMPLEN);
        DIALOG_STR(curdialog->backpic,    rover, MDLG_LUMPLEN);
        DIALOG_STR(curdialog->text,       rover, MDLG_TEXTLEN);

        // copy choices
        for(j = 0; j < 5; j++)
        {
            mapdlgchoice_t *curchoice = &(curdialog->choices[j]);
            DIALOG_INT(curchoice->giveitem,    rover);
            DIALOG_INT(curchoice->needitem1,   rover);
            DIALOG_INT(curchoice->needitem2,   rover);
            DIALOG_INT(curchoice->needitem3,   rover);
            DIALOG_INT(curchoice->needamount1, rover);
            DIALOG_INT(curchoice->needamount2, rover);
            DIALOG_INT(curchoice->needamount3, rover);
            DIALOG_STR(curchoice->text,        rover, MDLG_CHOICELEN);
            DIALOG_STR(curchoice->textok,      rover, MDLG_MSGLEN);
            DIALOG_INT(curchoice->next,        rover);
            DIALOG_INT(curchoice->objective,   rover);
            DIALOG_STR(curchoice->textno,      rover, MDLG_MSGLEN);
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
    sprintf(lumpname, DEH_String("script%02d"), gamemap);
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
mapdialog_t *P_DialogFind(int type)
{
    int i;

    // check the map-specific dialogs first
    for(i = 0; i < numleveldialogs; i++)
    {
        if(type == leveldialogs[i].speakerid)
            return &leveldialogs[i];
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
// STRIFE-TODO: Or is it a conversation ID?
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
    if(!strncasecmp(message, "RANDOM", 6))
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
                return rndMessages[i].messages[rnd % nummessages];
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
//
boolean P_GiveInventoryItem(player_t *player, int a2, int a3)
{
    int v3  = 0;
    int v15 = a2;
    int v4  = a3;

    // repaint the status bar due to inventory changing
    player->st_update = true;

    // STRIFE-TODO: do an insertion sort on the inventory...
    // Too bad the code is nearly impossible to understand!!!

    return true;
}

//
// P_GiveItemToPlayer
//
// [STRIFE] New function
// haleyjd 09/03/10: Sorts out how to give something to the player.
// Not strictly just for inventory items.
//
boolean P_GiveItemToPlayer(player_t *player, int sprnum, mobjtype_t type)
{
    // haleyjd: STRIFE-TODO
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
    int height;
    int finaly;

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
    if(angle > 0x20000000 && angle < 0xE0000000 || dialogtalker->flags & MF_INCOMBAT)
        P_DialogDoChoice(dialogmenu.numitems - 1);

    dialogtalker->reactiontime = 2;

    if(dialogbgpiclumpnum != -1)
    {
        patch_t *patch = W_CacheLumpNum(dialogbgpiclumpnum, PU_CACHE);
        V_DrawPatchDirect(0, 0, patch);
    }

    if(menupausetime <= gametic)
    {
        if(menuindialog)
        {
            if(menupausetime + 3 < gametic)
                menupause = true;
        }
        M_WriteText(12, 18, dialogname);
        y = 28;

        if(dialogshowtext || currentdialog->voice[0] == '\0')
            y = M_WriteText(20, 28, dialogtext);

        height = 20 * dialogmenu.numitems;

        finaly = 175 - height;     // preferred height
        if(y > finaly)
            finaly = 199 - height; // height it will bump down to if necessary.

        M_WriteText(42, finaly - 6, "______________________________");

        /*
        dialogmenu
        */
    }
}

//
// P_DialogDoChoice
//
// [STRIFE] New function
// haleyjd 09/05/10: Handles making a choice in a dialog. Installed as the
// callback for all items in the dialogmenu structure.
//
static void P_DialogDoChoice(int choice)
{
    // STRIFE-TODO
}

//
// P_DialogStart
//
// villsa [STRIFE] New function
//
void P_DialogStart(player_t *player)
{
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

    if(linetarget)
    {
        // already in combat, can't talk to it
        if(linetarget->flags & MF_INCOMBAT)
            return;

        dialogtalker = linetarget;

        // play a sound
        if(player = &players[consoleplayer])
            S_StartSound(0, sfx_radio);

        linetarget->target = player->mo;
        dialogtalker->reactiontime = 2;
        dialogtalkerangle = dialogtalker->angle;

        // face towards player
        A_FaceTarget(linetarget);
        // face towards NPC's direction
        player->mo->angle = R_PointToAngle2(
                            player->mo->x,
                            player->mo->y,
                            dialogtalker->x,
                            dialogtalker->y);

        dialogplayer = player;
    }

    //**[STRIFE] TODO**
}