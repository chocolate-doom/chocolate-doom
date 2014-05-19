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
//  Internally used data structures for virtually everything,
//   lots of other stuff.
//

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
//
// haleyjd 09/28/10: Replaced with Strife version
#define STRIFE_VERSION 101

// Version code for cph's longtics hack ("v1.91")
#define DOOM_191_VERSION 111


// Maximum players for Strife:
#define MAXPLAYERS 8

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
{               // Hex          Watcom Name               player_t offset
    tk_quest1,  // 0x00000001   questflags & 1            0x4D
    tk_quest2,  // 0x00000002   questflags & 2
    tk_quest3,  // 0x00000004   questflags & 4
    tk_quest4,  // 0x00000008   questflags & 8
    tk_quest5,  // 0x00000010   questflags & 10h
    tk_quest6,  // 0x00000020   questflags & 20h
    tk_quest7,  // 0x00000040   questflags & 40h
    tk_quest8,  // 0x00000080   questflags & 80h
    tk_quest9,  // 0x00000100   BYTE1(questflags) & 1     0x4E
    tk_quest10, // 0x00000200   BYTE1(questflags) & 2
    tk_quest11, // 0x00000400   BYTE1(questflags) & 4
    tk_quest12, // 0x00000800   BYTE1(questflags) & 8
    tk_quest13, // 0x00001000   BYTE1(questflags) & 10h
    tk_quest14, // 0x00002000   BYTE1(questflags) & 20h
    tk_quest15, // 0x00004000   BYTE1(questflags) & 40h
    tk_quest16, // 0x00008000   BYTE1(questflags) & 80h
    tk_quest17, // 0x00010000   BYTE2(questflags) & 1     0x4F
    tk_quest18, // 0x00020000   BYTE2(questflags) & 2
    tk_quest19, // 0x00040000   BYTE2(questflags) & 4
    tk_quest20, // 0x00080000   BYTE2(questflags) & 8
    tk_quest21, // 0x00100000   BYTE2(questflags) & 10h
    tk_quest22, // 0x00200000   BYTE2(questflags) & 20h
    tk_quest23, // 0x00400000   BYTE2(questflags) & 40h
    tk_quest24, // 0x00800000   BYTE2(questflags) & 80h
    tk_quest25, // 0x01000000   BYTE3(questflags) & 1     0x50
    tk_quest26, // 0x02000000   BYTE3(questflags) & 2
    tk_quest27, // 0x04000000   BYTE3(questflags) & 4
    tk_quest28, // 0x08000000   BYTE3(questflags) & 8
    tk_quest29, // 0x10000000   BYTE3(questflags) & 10h
    tk_quest30, // 0x20000000   BYTE3(questflags) & 20h
    tk_quest31, // 0x40000000   BYTE3(questflags) & 40h
    tk_quest32, // most likely unused
    tk_numquests
} questtype_t;

// haleyjd 09/12/10: [STRIFE]
// flag values for each quest.
enum
{ //  Name       Flag from bitnum      Purpose, if known
    QF_QUEST1  = (1 << tk_quest1),  // Obtained Beldin's ring
    QF_QUEST2  = (1 << tk_quest2),  // Stole the Chalice
    QF_QUEST3  = (1 << tk_quest3),  // Permission to visit Irale (visited Macil)
    QF_QUEST4  = (1 << tk_quest4),  // Accepted Gov. Mourel's "messy" chore
    QF_QUEST5  = (1 << tk_quest5),  // Accepted Gov. Mourel's "bloody" chore
    QF_QUEST6  = (1 << tk_quest6),  // Destroyed the Power Coupling
    QF_QUEST7  = (1 << tk_quest7),  // Killed Blue Acolytes ("Scanning Team")
    QF_QUEST8  = (1 << tk_quest8),  // Unused; formerly, picked up Broken Coupling
    QF_QUEST9  = (1 << tk_quest9),  // Obtained Derwin's ear
    QF_QUEST10 = (1 << tk_quest10), // Obtained Prison Pass
    QF_QUEST11 = (1 << tk_quest11), // Obtained Prison Key
    QF_QUEST12 = (1 << tk_quest12), // Obtained Judge Wolenick's hand
    QF_QUEST13 = (1 << tk_quest13), // Freed the Prisoners
    QF_QUEST14 = (1 << tk_quest14), // Destroyed the Power Crystal
    QF_QUEST15 = (1 << tk_quest15), // Obtained Guard Uniform
    QF_QUEST16 = (1 << tk_quest16), // Destroyed the Gate Mechanism
    QF_QUEST17 = (1 << tk_quest17), // Heard Macil's story about the Sigil (MAP10)
    QF_QUEST18 = (1 << tk_quest18), // Obtained Oracle Pass
    QF_QUEST19 = (1 << tk_quest19),
    QF_QUEST20 = (1 << tk_quest20),
    QF_QUEST21 = (1 << tk_quest21), // Killed Bishop
    QF_QUEST22 = (1 << tk_quest22), // Killed Oracle with QUEST21 set
    QF_QUEST23 = (1 << tk_quest23), // Killed Oracle (always given)
    QF_QUEST24 = (1 << tk_quest24), // Killed Macil
    QF_QUEST25 = (1 << tk_quest25), // Destroyed the Converter
    QF_QUEST26 = (1 << tk_quest26), // Killed Loremaster
    QF_QUEST27 = (1 << tk_quest27), // Destroyed the Computer (checked for good ending)
    QF_QUEST28 = (1 << tk_quest28), // Obtained Catacomb Key (checked by line type 228)
    QF_QUEST29 = (1 << tk_quest29), // Destroyed the Mines Transmitter
    QF_QUEST30 = (1 << tk_quest30),
    QF_QUEST31 = (1 << tk_quest31),
    QF_QUEST32 = (1 << tk_quest32), // Unused; BUG: Broken Coupling accidentally sets it.
    
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
