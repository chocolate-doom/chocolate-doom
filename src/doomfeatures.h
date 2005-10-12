// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomfeatures.h 190 2005-10-12 21:52:01Z fraggle $
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
// Revision 1.1  2005/10/12 21:52:01  fraggle
// doomfeatures.h to allow certain features to be disabled in the build
//
//
// DESCRIPTION:
//     List of features which can be enabled/disabled to slim down the
//     program.
//
//-----------------------------------------------------------------------------

#ifndef DOOM_FEATURES_H
#define DOOM_FEATURES_H

// Enables wad merging (the '-merge' command line parameter)

#define FEATURE_WAD_MERGE 1

// Enables dehacked support ('-deh')

#define FEATURE_DEHACKED 1

#endif /* #ifndef DOOM_FEATURES_H */


