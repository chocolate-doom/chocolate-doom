// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_saveg.h 202 2005-10-16 01:18:10Z fraggle $
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
//	Savegame I/O, archiving, persistence.
//
//-----------------------------------------------------------------------------


#ifndef __P_SAVEG__
#define __P_SAVEG__

// filename to use for a savegame slot

char *P_SaveGameFile(int slot);


// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayers (void);
void P_UnArchivePlayers (void);
void P_ArchiveWorld (void);
void P_UnArchiveWorld (void);
void P_ArchiveThinkers (void);
void P_UnArchiveThinkers (void);
void P_ArchiveSpecials (void);
void P_UnArchiveSpecials (void);

extern byte*		save_p; 


#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.4  2005/10/16 01:18:10  fraggle
// Global "configdir" variable with directory to store config files in.
// Create a function to find the filename for a savegame slot.  Store
// savegames in the config dir.
//
// Revision 1.3  2005/07/23 18:56:07  fraggle
// Remove unneccessary pragmas
//
// Revision 1.2  2005/07/23 16:44:56  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:39  fraggle
// Initial import
//
//
//-----------------------------------------------------------------------------
