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
//  Internally used data structures for virtually everything,
//   lots of other stuff.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMDEF__
#define __DOOMDEF__

#include <stdio.h>
#include <string.h>

#include "doomtype.h"
#include "i_timer.h"
#include "d_mode.h"

//
// Global parameters/defines.
//
// DOOM version
#define DOOM_VERSION 109

// Version code for cph's longtics hack ("v1.91")
#define DOOM_191_VERSION 111


// If rangecheck is undefined,
// most parameter validation debugging code will not be compiled
#define RANGECHECK

// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or a demo. 
typedef enum
{
    GS_LEVEL,
    GS_UNKNOWN,
    GS_FINALE,
    GS_DEMOSCREEN,
} gamestate_t;

typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_playdemo,
    ga_completed,
    ga_victory,
    ga_worlddone,
    ga_screenshot
} gameaction_t;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define	MTF_EASY                1
#define	MTF_NORMAL              2
#define	MTF_HARD                4
// villsa [STRIFE] standing monsters
#define MTF_STAND               8
// villsa [STRIFE] don't spawn in single player
#define MTF_NOTSINGLE           16
// Deaf monsters/do not react to sound.
#define MTF_AMBUSH              32
// villsa [STRIFE] friendly to players
#define MTF_FRIEND              64
// villsa [STRIFE] TODO - identify
#define MTF_UNKNOWN1            128
// villsa [STRIFE] thing is translucent - STRIFE-TODO: But how much?
#define MTF_TRANSLUCENT         256
// villsa [STRIFE] thing is more - or less? - translucent - STRIFE-TODO
#define MTF_MVIS                512
// villsa [STRIFE] TODO - identify
#define MTF_UNKNOWN2            1024



//
// Key cards.
//
// villsa [STRIFE]
typedef enum
{
    key_BaseKey,        // 0
    key_GovsKey,        // 1
    key_Passcard,       // 2
    key_IDCard,         // 3
    key_PrisonKey,      // 4
    key_SeveredHand,    // 5
    key_Power1Key,      // 6
    key_Power2Key,      // 7
    key_Power3Key,      // 8
    key_GoldKey,        // 9
    key_IDBadge,        // 10
    key_SilverKey,      // 11
    key_OracleKey,      // 12
    key_MilitaryID,     // 13
    key_OrderKey,       // 14
    key_WarehouseKey,   // 15
    key_BrassKey,       // 16
    key_RedCrystalKey,  // 17
    key_BlueCrystalKey, // 18
    key_ChapelKey,      // 19
    key_CatacombKey,    // 20
    key_SecurityKey,    // 21
    key_CoreKey,        // 22
    key_MaulerKey,      // 23
    key_FactoryKey,     // 24
    key_MineKey,        // 25
    key_NewKey5,        // 26

    NUMCARDS            // 27
} card_t;



// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
// villsa [STRIFE]
typedef enum
{
    wp_fist,
    wp_elecbow,
    wp_rifle,
    wp_missile,
    wp_hegrenade,
    wp_flame,
    wp_mauler,
    wp_sigil,
    wp_poisonbow,
    wp_wpgrenade,
    wp_torpedo,

    NUMWEAPONS,
    
    // No pending weapon change.
    wp_nochange

} weapontype_t;


// Ammunition types defined.
typedef enum
{
    am_bullets,
    am_elecbolts,
    am_poisonbolts,
    am_cell,
    am_missiles,
    am_hegrenades,
    am_wpgrenades,

    NUMAMMO,

    am_noammo   // unlimited ammo

} ammotype_t;


// Power up artifacts.
// villsa [STRIFE]
typedef enum
{
    pw_strength,
    pw_invisibility,
    pw_ironfeet,
    pw_allmap,
    pw_communicator,
    pw_targeter,
    NUMPOWERS
    
} powertype_t;

// villsa [STRIFE]
// quest numbers
typedef enum
{
    tk_quest1,
    tk_quest2,
    tk_quest3,
    tk_quest4,
    tk_quest5,
    tk_quest6,
    tk_quest7,
    tk_quest8,
    tk_quest9,
    tk_quest10,
    tk_quest11,
    tk_quest12,
    tk_quest13,
    tk_quest14,
    tk_quest15,
    tk_quest16,
    tk_quest17,
    tk_quest18,
    tk_quest19,
    tk_quest20,
    tk_quest21,
    tk_quest22,
    tk_quest23,
    tk_quest24,
    tk_quest25,
    tk_quest26,
    tk_quest27,
    tk_quest28,
    tk_quest29,
    tk_quest30,
    tk_quest31,
    tk_quest32,   // most likely unused
    tk_numquests
} questtype_t;

// haleyjd 09/12/10: [STRIFE]
// flag values for each quest.
enum
{ //  Name       Flag from bitnum      Purpose, if known
    QF_QUEST1  = (1 << tk_quest1),
    QF_QUEST2  = (1 << tk_quest2),
    QF_QUEST3  = (1 << tk_quest3), 
    QF_QUEST4  = (1 << tk_quest4),
    QF_QUEST5  = (1 << tk_quest5),
    QF_QUEST6  = (1 << tk_quest6),  // Destroyed the Power Coupling
    QF_QUEST7  = (1 << tk_quest7),
    QF_QUEST8  = (1 << tk_quest8),
    QF_QUEST9  = (1 << tk_quest9),
    QF_QUEST10 = (1 << tk_quest10),
    QF_QUEST11 = (1 << tk_quest11),
    QF_QUEST12 = (1 << tk_quest12),
    QF_QUEST13 = (1 << tk_quest13),
    QF_QUEST14 = (1 << tk_quest14),
    QF_QUEST15 = (1 << tk_quest15),
    QF_QUEST16 = (1 << tk_quest16),
    QF_QUEST17 = (1 << tk_quest17),
    QF_QUEST18 = (1 << tk_quest18), // Obtained Oracle Pass
    QF_QUEST19 = (1 << tk_quest19),
    QF_QUEST20 = (1 << tk_quest20),
    QF_QUEST21 = (1 << tk_quest21),
    QF_QUEST22 = (1 << tk_quest22),
    QF_QUEST23 = (1 << tk_quest23),
    QF_QUEST24 = (1 << tk_quest24),
    QF_QUEST25 = (1 << tk_quest25), // Destroyed the Converter
    QF_QUEST26 = (1 << tk_quest26),
    QF_QUEST27 = (1 << tk_quest27),
    QF_QUEST28 = (1 << tk_quest28),
    QF_QUEST29 = (1 << tk_quest29), // Destroyed the Mines Transmitter
    QF_QUEST30 = (1 << tk_quest30),
    QF_QUEST31 = (1 << tk_quest31),
    QF_QUEST32 = (1 << tk_quest32), // Note: seems to be unused
    
    QF_ALLQUESTS  = (QF_QUEST31 + (QF_QUEST31 - 1)) // does not include bit 32!
};

//
// Power up durations,
//  how many seconds till expiration,
//  assuming TICRATE is 35 ticks/second.
//
typedef enum
{
    INVISTICS	= (55*TICRATE), // villsa [STRIFE] changed from 60 to 55
    IRONTICS	= (80*TICRATE), // villsa [STRIFE] changed from 60 to 80
    PMUPTICS    = (80*TICRATE), // villsa [STRIFE]
    TARGTICS    = (160*TICRATE),// villsa [STRIFE]
    
} powerduration_t;

#endif          // __DOOMDEF__

