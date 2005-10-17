// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_misc.h 212 2005-10-17 22:07:26Z fraggle $
//
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
// $Log$
// Revision 1.2  2005/10/17 22:07:26  fraggle
// Fix "Monsters Infight"
//
// Revision 1.1  2005/10/17 20:27:05  fraggle
// Start of Dehacked 'Misc' section support.  Initial Health+Bullets,
// and bfg cells/shot are supported.
//
// Revision 1.2  2005/10/08 20:54:16  fraggle
// Proper dehacked error/warning framework.  Catch a load more errors.
//
// Revision 1.1  2005/10/04 22:10:32  fraggle
// Dehacked "Misc" section parser (currently a dummy)
//
//
//-----------------------------------------------------------------------------
//
// Parses "Misc" sections in dehacked files
//
//-----------------------------------------------------------------------------

#ifndef DEH_MISC_H
#define DEH_MISC_H

extern int deh_initial_health;             
extern int deh_initial_bullets;            
extern int deh_max_health;                 
extern int deh_max_armor;                  
extern int deh_green_armor_class;          
extern int deh_blue_armor_class;           
extern int deh_max_soulsphere;             
extern int deh_soulsphere_health;          
extern int deh_megasphere_health;          
extern int deh_god_mode_health;            
extern int deh_idfa_armor;                 
extern int deh_idfa_armor_class;           
extern int deh_idkfa_armor;                
extern int deh_idkfa_armor_class;          
extern int deh_bfg_cells_per_shot;         
extern int deh_species_infighting;           

#endif /* #ifndef DEH_MISC_H */

