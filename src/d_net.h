// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
//	Networking stuff.
//
//-----------------------------------------------------------------------------


#ifndef __D_NET__
#define __D_NET__

#include "d_player.h"

#define MAXNETNODES 8

// Networking and tick handling related.
#define BACKUPTICS		128

extern int extratics;

// Create any new ticcmds and broadcast to other players.
void NetUpdate (void);

// Broadcasts special packets to other players
//  to notify of game exit
void D_QuitNetGame (void);

//? how many ticks to run?
void TryRunTics (void);

// Called at start of game loop to initialise timers
void D_StartGameLoop(void);

#endif

//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.6  2006/02/24 19:14:22  fraggle
// Remove redundant stuff relating to the old network code
//
// Revision 1.5  2006/02/19 13:38:59  fraggle
// Increase the size of BACKUPTICS to deal with heavy lag
//
// Revision 1.4  2006/01/22 22:29:42  fraggle
// Periodically request the time from clients to estimate their offset to
// the server time.
//
// Revision 1.3  2005/07/23 18:56:07  fraggle
// Remove unneccessary pragmas
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:50  fraggle
// Initial import
//
//
//-----------------------------------------------------------------------------

