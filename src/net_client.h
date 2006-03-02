// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_client.h 405 2006-03-02 00:57:25Z fraggle $
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
// Revision 1.10  2006/01/12 02:11:52  fraggle
// Game start packets
//
// Revision 1.9  2006/01/09 02:03:39  fraggle
// Send clients their player number, and indicate on the waiting screen
// which client we are.
//
// Revision 1.8  2006/01/09 01:50:51  fraggle
// Deduce a sane player name by examining environment variables.  Add
// a "player_name" setting to chocolate-doom.cfg.  Transmit the name
// to the server and use the names players send in the waiting data list.
//
// Revision 1.7  2006/01/07 20:08:11  fraggle
// Send player name and address in the waiting data packets.  Display these
// on the waiting screen, and improve the waiting screen appearance.
//
// Revision 1.6  2006/01/02 21:50:26  fraggle
// Restructure the waiting screen code.  Establish our own separate event
// loop while waiting for the game to start, to avoid affecting the original
// code too much.  Move some _gui variables to net_client.c.
//
// Revision 1.5  2006/01/02 00:00:08  fraggle
// Neater prefixes: NET_Client -> NET_CL_.  NET_Server -> NET_SV_.
//
// Revision 1.4  2006/01/01 23:54:31  fraggle
// Client disconnect code
//
// Revision 1.3  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//
// Revision 1.2  2005/12/29 21:29:55  fraggle
// Working client connect code
//
// Revision 1.1  2005/12/29 17:48:25  fraggle
// Add initial client/server connect code.  Reorganise sources list in
// Makefile.am.
//
//
// Network client code
//

#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "doomdef.h"
#include "doomtype.h"
#include "d_ticcmd.h"
#include "net_defs.h"

#define MAXPLAYERNAME 30

boolean NET_CL_Connect(net_addr_t *addr);
void NET_CL_Disconnect(void);
void NET_CL_Run(void);
void NET_CL_Init(void);
void NET_CL_StartGame();
void NET_CL_SendTiccmd(ticcmd_t *ticcmd, int maketic);
void NET_Init(void);

extern boolean net_client_connected;
extern boolean net_client_controller;
extern int net_clients_in_game;
extern boolean net_waiting_for_start;
extern char net_player_names[MAXPLAYERS][MAXPLAYERNAME];
extern char net_player_addresses[MAXPLAYERS][MAXPLAYERNAME];
extern int net_player_number;
extern char *net_player_name;

#endif /* #ifndef NET_CLIENT_H */

