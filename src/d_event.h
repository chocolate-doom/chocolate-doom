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
//
//    


#ifndef __D_EVENT__
#define __D_EVENT__


#include "doomtype.h"


//
// Event handling.
//

// Input event types.
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_joystick,
    ev_quit
} evtype_t;

// Event structure.
typedef struct
{
    evtype_t type;

    // Event-related data that depends on the type of event:
    //
    // ev_keydown/ev_keyup:
    //    data1: Key code (from doomkeys.h) of the key that was
    //           pressed or released.
    //    data2: Ascii text of the character that was pressed,
    //           shifted appropriately (eg. '$' if 4 was pressed
    //           while shift was held).
    //
    // ev_mouse:
    //    data1: Bitfield of buttons currently held down.
    //           (bit 0 = left; bit 1 = right; bit 2 = middle).
    //    data2: X axis mouse movement (turn).
    //    data3: Y axis mouse movement (forward/backward).
    //
    // ev_joystick:
    //    data1: Bitfield of buttons currently pressed.
    //    data2: X axis mouse movement (turn).
    //    data3: Y axis mouse movement (forward/backward).
    //    data4: Third axis mouse movement (strafe).

    int data1, data2, data3, data4;
} event_t;

 
//
// Button/action code definitions.
//
typedef enum
{
    // Press "Fire".
    BT_ATTACK		= 1,
    // Use button, to open doors, activate switches.
    BT_USE		= 2,

    // Flag: game events, not really buttons.
    BT_SPECIAL		= 128,
    BT_SPECIALMASK	= 3,
    
    // Flag, weapon change pending.
    // If true, the next 3 bits hold weapon num.
    BT_CHANGE		= 4,
    // The 3bit weapon mask and shift, convenience.
    BT_WEAPONMASK	= (8+16+32),
    BT_WEAPONSHIFT	= 3,

    // Pause the game.
    BTS_PAUSE		= 1,
    // Save the game at each console.
    BTS_SAVEGAME	= 2,

    // Savegame slot numbers
    //  occupy the second byte of buttons.    
    BTS_SAVEMASK	= (4+8+16),
    BTS_SAVESHIFT 	= 2,
  
} buttoncode_t;

// villsa [STRIFE] Strife specific buttons
// TODO - not finished
typedef enum
{
    // Player view look up
    BT2_LOOKUP          = 1,
    // Player view look down
    BT2_LOOKDOWN        = 2,
    // Center player's view
    BT2_CENTERVIEW      = 4,
    // Use inventory item
    BT2_INVUSE          = 8,
    // Drop inventory item
    BT2_INVDROP         = 16,
    // Jump up and down
    BT2_JUMP            = 32,
    // Use medkit
    BT2_HEALTH          = 128,
  
} buttoncode2_t;




// Called by IO functions when input is detected.
void D_PostEvent (event_t *ev);

// Read an event from the event queue

event_t *D_PopEvent(void);


#endif

