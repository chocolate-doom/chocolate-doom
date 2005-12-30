// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
// Revision 1.1  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//
//
// Reading and writing various structures into packets
//

#include "net_packet.h"

void NET_WriteSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    NET_WriteInt8(packet, settings->ticdup);
    NET_WriteInt8(packet, settings->extratics);
    NET_WriteInt8(packet, settings->deathmatch);
    NET_WriteInt8(packet, settings->episode);
    NET_WriteInt8(packet, settings->map);
    NET_WriteInt8(packet, settings->skill);
}

boolean NET_ReadSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    return NET_ReadInt8(packet, (unsigned int *) &settings->ticdup)
        && NET_ReadInt8(packet, (unsigned int *) &settings->extratics)
        && NET_ReadInt8(packet, (unsigned int *) &settings->deathmatch)
        && NET_ReadInt8(packet, (unsigned int *) &settings->episode)
        && NET_ReadInt8(packet, (unsigned int *) &settings->map)
        && NET_ReadInt8(packet, (unsigned int *) &settings->skill);
}

