// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomkeys.h 488 2006-05-20 16:16:35Z fraggle $
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
//       Key definitions
//
//-----------------------------------------------------------------------------

#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW	0xae
#define KEY_LEFTARROW	0xac
#define KEY_UPARROW	0xad
#define KEY_DOWNARROW	0xaf
#define KEY_ESCAPE	27
#define KEY_ENTER	13
#define KEY_TAB		9
#define KEY_F1		(0x80+0x3b)
#define KEY_F2		(0x80+0x3c)
#define KEY_F3		(0x80+0x3d)
#define KEY_F4		(0x80+0x3e)
#define KEY_F5		(0x80+0x3f)
#define KEY_F6		(0x80+0x40)
#define KEY_F7		(0x80+0x41)
#define KEY_F8		(0x80+0x42)
#define KEY_F9		(0x80+0x43)
#define KEY_F10		(0x80+0x44)
#define KEY_F11		(0x80+0x57)
#define KEY_F12		(0x80+0x58)

#define KEY_BACKSPACE	'\b'
#define KEY_PAUSE	0xff

#define KEY_EQUALS	0x3d
#define KEY_MINUS	0x2d

#define KEY_RSHIFT	(0x80+0x36)
#define KEY_RCTRL	(0x80+0x1d)
#define KEY_RALT	(0x80+0x38)

#define KEY_LALT	KEY_RALT

// new keys:

#define KEY_CAPSLOCK    (0x80+0x3a)
#define KEY_SCRLCK      (0x80+0x46)

#define KEYP_0          (0x80+0x52)
#define KEYP_1          (0x80+0x4F)
#define KEYP_2          (0x80+0x50)
#define KEYP_3          (0x80+0x41)
#define KEYP_4          (0x80+0x4B)
#define KEYP_5          (0x80+0x4C)
#define KEYP_6          (0x80+0x4D)
#define KEYP_7          (0x80+0x47)
#define KEYP_8          (0x80+0x48)
#define KEYP_9          (0x80+0x49)

#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)
#define KEYP_UPARROW      KEY_UPARROW
#define KEYP_DOWNARROW    KEY_DOWNARROW
#define KEYP_LEFTARROW    KEY_LEFTARROW
#define KEYP_RIGHTARROW   KEY_RIGHTARROW
#define KEYP_MULTIPLY     '*'
#define KEYP_PLUS         '+'
#define KEYP_MINUS        '-'
#define KEYP_DIVIDE       '/'




#endif          // __DOOMKEYS__

