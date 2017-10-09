//
// Copyright(C) 2005-2014 Simon Howard
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

#ifndef NET_STRUCTRW_H
#define NET_STRUCTRW_H

#include "aes_prng.h"
#include "sha1.h"
#include "net_defs.h"
#include "net_packet.h"

void NET_WriteConnectData(net_packet_t *packet, net_connect_data_t *data);
boolean NET_ReadConnectData(net_packet_t *packet, net_connect_data_t *data);

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

boolean NET_ReadSHA1Sum(net_packet_t *packet, sha1_digest_t digest);
void NET_WriteSHA1Sum(net_packet_t *packet, sha1_digest_t digest);

void NET_WriteWaitData(net_packet_t *packet, net_waitdata_t *data);
boolean NET_ReadWaitData(net_packet_t *packet, net_waitdata_t *data);

boolean NET_ReadPRNGSeed(net_packet_t *packet, prng_seed_t seed);
void NET_WritePRNGSeed(net_packet_t *packet, prng_seed_t seed);

// Protocol list exchange.
net_protocol_t NET_ReadProtocol(net_packet_t *packet);
void NET_WriteProtocol(net_packet_t *packet, net_protocol_t protocol);
net_protocol_t NET_ReadProtocolList(net_packet_t *packet);
void NET_WriteProtocolList(net_packet_t *packet);

#endif /* #ifndef NET_STRUCTRW_H */
