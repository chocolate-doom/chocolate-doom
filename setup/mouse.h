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

#ifndef SETUP_MOUSE_H
#define SETUP_MOUSE_H

extern int usemouse;

extern int novert;
extern int mouseSensitivity;
extern float mouse_acceleration;
extern int mouse_threshold;
extern int grabmouse;
extern int mousebfire;
extern int mousebforward;
extern int mousebstrafe;
extern int mousebstrafeleft;
extern int mousebstraferight;
extern int mousebbackward;
extern int mousebuse;
extern int dclick_use;
extern int mousebprevweapon;
extern int mousebnextweapon;

void ConfigMouse(void);


#endif /* #ifndef SETUP_MOUSE_H */

