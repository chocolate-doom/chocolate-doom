// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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

#ifndef SETUP_JOYSTICK_H
#define SETUP_JOYSTICK_H

extern int usejoystick;
extern int joybfire;
extern int joybstrafe;
extern int joybuse;
extern int joybspeed;
extern int joybstrafeleft;
extern int joybstraferight;
extern int joybprevweapon;
extern int joybnextweapon;

extern int joystick_index;
extern int joystick_x_axis;
extern int joystick_x_invert;
extern int joystick_y_axis;
extern int joystick_y_invert;

void ConfigJoystick(void);

#endif /* #ifndef SETUP_JOYSTICK_H */

