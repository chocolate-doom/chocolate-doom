// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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

#ifndef NET_STRUCTRW_H
#define NET_STRUCTRW_H

#include "md5.h"
#include "net_defs.h"
#include "net_packet.h"

extern void NET_WriteSettings(net_packet_t *packet, net_gamesettings_t *settings);
extern boolean NET_ReadSettings(net_packet_t *packet, net_gamesettings_t *settings);

extern void NET_WriteQueryData(net_packet_t *packet, net_querydata_t *querydata);
extern boolean NET_ReadQueryData(net_packet_t *packet, net_querydata_t *querydata);

extern void NET_WriteTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff, boolean lowres_turn);
extern boolean NET_ReadTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff, boolean lowres_turn);
extern void NET_TiccmdDiff(ticcmd_t *tic1, ticcmd_t *tic2, net_ticdiff_t *diff);
extern void NET_TiccmdPatch(ticcmd_t *src, net_ticdiff_t *diff, ticcmd_t *dest);

boolean NET_ReadFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, boolean lowres_turn);
void NET_WriteFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, boolean lowres_turn);

boolean NET_ReadMD5Sum(net_packet_t *packet, md5_digest_t digest);
void NET_WriteMD5Sum(net_packet_t *packet, md5_digest_t digest);

#endif /* #ifndef NET_STRUCTRW_H */

