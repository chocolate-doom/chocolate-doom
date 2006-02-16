// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_structrw.h 369 2006-02-16 01:12:28Z fraggle $
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
// Revision 1.4  2006/02/16 01:12:28  fraggle
// Define a new type net_full_ticcmd_t, a structure containing all ticcmds
// for a given tic.  Store received game data in a receive window.  Add
// send queues for clients and add data from the receive window to
// generate complete sets of ticcmds.
//
// Revision 1.3  2006/01/13 02:22:47  fraggle
// Update prototypes to match header.  Make sure we include the header in the
// source file.
//
// Revision 1.2  2006/01/11 01:37:53  fraggle
// ticcmd diffs: allow compare and patching ticcmds, and reading/writing
// ticdiffs to packets.
//
// Revision 1.1  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//

#ifndef NET_STRUCTRW_H
#define NET_STRUCTRW_H

#include "net_defs.h"
#include "net_packet.h"

extern void NET_WriteSettings(net_packet_t *packet, net_gamesettings_t *settings);
extern boolean NET_ReadSettings(net_packet_t *packet, net_gamesettings_t *settings);

extern void NET_WriteTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff, boolean lowres_turn);
extern boolean NET_ReadTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff, boolean lowres_turn);
extern void NET_TiccmdDiff(ticcmd_t *tic1, ticcmd_t *tic2, net_ticdiff_t *diff);
extern void NET_TiccmdPatch(ticcmd_t *src, net_ticdiff_t *diff, ticcmd_t *dest);

boolean NET_ReadFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd);
void NET_WriteFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd);


#endif /* #ifndef NET_STRUCTRW_H */

