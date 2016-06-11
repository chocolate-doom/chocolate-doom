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
// DESCRIPTION:
//
// [STRIFE] New Module
//
// Dialog Engine for Strife
//

#ifndef P_DIALOG_H__
#define P_DIALOG_H__

#define OBJECTIVE_LEN       300

#define MAXINVENTORYSLOTS   30

#define MDLG_CHOICELEN      32
#define MDLG_MSGLEN         80
#define MDLG_NAMELEN        16
#define MDLG_LUMPLEN        8
#define MDLG_TEXTLEN        320
#define MDLG_MAXCHOICES     5
#define MDLG_MAXITEMS       3

extern char mission_objective[OBJECTIVE_LEN];

extern int dialogshowtext;

// villsa - convenient macro for giving objective logs to player
#define GiveObjective(x, minlumpnum) \
do { \
  int obj_ln  = W_CheckNumForName(DEH_String(x)); \
  if(obj_ln > minlumpnum) \
    M_StringCopy(mission_objective, W_CacheLumpNum(obj_ln, PU_CACHE), \
                 OBJECTIVE_LEN);\
} while(0)

// haleyjd - voice and objective in one
#define GiveVoiceObjective(voice, log, minlumpnum) \
do { \
  int obj_ln = W_CheckNumForName(DEH_String(log)); \
  I_StartVoice(DEH_String(voice)); \
  if(obj_ln > minlumpnum) \
    M_StringCopy(mission_objective, W_CacheLumpNum(obj_ln, PU_CACHE), \
                 OBJECTIVE_LEN);\
} while(0)

typedef struct mapdlgchoice_s
{
    int  giveitem;                      // item given when successful
    int  needitems[MDLG_MAXITEMS];      // item needed for success
    int  needamounts[MDLG_MAXITEMS];    // amount of items needed
    char text[MDLG_CHOICELEN];          // normal text
    char textok[MDLG_MSGLEN];           // message given on success
    int next;                           // next dialog?
    int objective;                      // ???
    char textno[MDLG_MSGLEN];           // message given on failure
} mapdlgchoice_t;

typedef struct mapdialog_s
{
    int speakerid;                      // script ID# for mobjtype that will use this dialog
    int dropitem;                       // item to drop if that thingtype is killed
    int checkitem[MDLG_MAXITEMS];       // item(s) needed to see this dialog
    int jumptoconv;                     // conversation to jump to when... ?
    char name[MDLG_NAMELEN];            // name of speaker
    char voice[MDLG_LUMPLEN];           // voice file to play
    char backpic[MDLG_LUMPLEN];         // backdrop pic for character, if any
    char text[MDLG_TEXTLEN];            // main message text
    
    // options that this dialog gives the player
    mapdlgchoice_t choices[MDLG_MAXCHOICES];
} mapdialog_t;

void         P_DialogLoad(void);
void         P_DialogStart(player_t *player);
void         P_DialogDoChoice(int choice);
boolean      P_GiveItemToPlayer(player_t *player, int sprnum, mobjtype_t type);
boolean      P_GiveInventoryItem(player_t *player, int sprnum, mobjtype_t type);
boolean      P_UseInventoryItem(player_t* player, int item);
void         P_DialogStartP1(void);
mapdialog_t* P_DialogFind(mobjtype_t type, int jumptoconv);
int          P_PlayerHasItem(player_t *player, mobjtype_t type);

#endif

// EOF


