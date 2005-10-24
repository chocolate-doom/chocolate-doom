// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomstat.c 223 2005-10-24 18:50:39Z fraggle $
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
// $Log$
// Revision 1.6  2005/10/24 18:50:39  fraggle
// Allow the game version to emulate to be specified from the command line
// and set compatibility options accordingly.
//
// Revision 1.5  2005/09/04 14:51:19  fraggle
// Display the correct quit messages according to which game is being played.
// Remove "language" variable (do this through gettext, if ever)
//
// Revision 1.4  2005/08/31 21:21:18  fraggle
// Better IWAD detection and identification. Support '-iwad' to specify
// the IWAD to use.
//
// Revision 1.3  2005/07/23 18:56:07  fraggle
// Remove unneccessary pragmas
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:07  fraggle
// Initial import
//
//
// DESCRIPTION:
//	Put all global tate variables here.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: doomstat.c 223 2005-10-24 18:50:39Z fraggle $";


#include "doomstat.h"


// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t gamemode = indetermined;
GameMission_t	gamemission = doom;
GameVersion_t   gameversion = exe_final;
char *gamedescription;

// Set if homebrew PWAD stuff has been added.
boolean	modifiedgame;




