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

#include "p_dialog.h"

//
// Defines and Macros
//

// haleyjd: size of the original Strife mapdialog_t structure.
#define ORIG_MAPDIALOG_SIZE 0x5EC

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

//
// Routines
//

#define DIALOG_INT(field, ptr)  \
    field = ((int)ptr[0]        | \
            ((int)ptr[1] <<  8) | \
            ((int)ptr[2] << 16) | \
            ((int)ptr[3] << 24)); \
    ptr += 4;

#define DIALOG_STR(field, ptr, len) \
    memcpy(field, ptr, len); \
    ptr += len;

//
// P_ParseDialogLump
//
// haleyjd 09/02/10: This is a new function added to parse out the dialogs
// from the dialog lump rather than reading them raw from the lump pointer.
// This avoids problems with structure packing.
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
            DIALOG_INT(curchoice->giveitem, rover);
            DIALOG_INT(curchoice->needitem1, rover);
            DIALOG_INT(curchoice->needitem2, rover);
            DIALOG_INT(curchoice->needitem3, rover);
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
void P_DialogLoad(int eax0, int a2, int a3, int a4)
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

