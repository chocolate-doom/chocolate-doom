// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: w_merge.h 361 2006-02-03 18:40:00Z fraggle $
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
// Revision 1.1.2.1  2006/02/03 18:40:00  fraggle
// Support NWT-style WAD merging (-af and -as command line parameters).
// Restructure WAD loading so that merged WADs are always loaded before
// normal PWADs.  Remove W_InitMultipleFiles().
//
// Revision 1.1  2005/10/08 18:23:18  fraggle
// WAD merging code
//
//
// DESCRIPTION:
// Handles merging of PWADs, similar to deutex's -merge option
//
// Ideally this should work exactly the same as in deutex, but trying to
// read the deutex source code made my brain hurt.
//
//-----------------------------------------------------------------------------

#ifndef W_MERGE_H
#define W_MERGE_H

#define W_NWT_MERGE_SPRITES   0x1
#define W_NWT_MERGE_FLATS     0x2

// Add a new WAD and merge it into the main directory

void W_MergeFile(char *filename);

// NWT-style merging

void W_NWTMergeFile(char *filename, int flags);

#endif /* #ifndef W_MERGE_H */

