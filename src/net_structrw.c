// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_structrw.c 378 2006-02-23 19:12:02Z fraggle $
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
// Revision 1.8  2006/02/23 19:12:02  fraggle
// Add lowres_turn to indicate whether we generate angleturns which are
// 8-bit as opposed to 16-bit.  This is used when recording demos without
// -longtics enabled.  Sync this option between clients in a netgame, so
// that if one player is recording a Vanilla demo, all clients record
// in lowres.
//
// Revision 1.7  2006/02/23 18:19:05  fraggle
// Add lowres_turn parameter to net_full_ticcmd_t structure r/w functions
//
// Revision 1.6  2006/02/16 01:12:28  fraggle
// Define a new type net_full_ticcmd_t, a structure containing all ticcmds
// for a given tic.  Store received game data in a receive window.  Add
// send queues for clients and add data from the receive window to
// generate complete sets of ticcmds.
//
// Revision 1.5  2006/01/14 02:06:48  fraggle
// Include the game version in the settings structure.
//
// Revision 1.4  2006/01/13 02:22:47  fraggle
// Update prototypes to match header.  Make sure we include the header in the
// source file.
//
// Revision 1.3  2006/01/13 02:20:12  fraggle
// Signed integer read functions.  Use these when reading ticcmd diffs.
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
//
// Reading and writing various structures into packets
//

#include <stdlib.h>
#include <string.h>

#include "doomdef.h"

#include "net_packet.h"
#include "net_structrw.h"

void NET_WriteSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    NET_WriteInt8(packet, settings->ticdup);
    NET_WriteInt8(packet, settings->extratics);
    NET_WriteInt8(packet, settings->deathmatch);
    NET_WriteInt8(packet, settings->episode);
    NET_WriteInt8(packet, settings->map);
    NET_WriteInt8(packet, settings->skill);
    NET_WriteInt8(packet, settings->gameversion);
    NET_WriteInt8(packet, settings->lowres_turn);
}

boolean NET_ReadSettings(net_packet_t *packet, net_gamesettings_t *settings)
{
    return NET_ReadInt8(packet, (unsigned int *) &settings->ticdup)
        && NET_ReadInt8(packet, (unsigned int *) &settings->extratics)
        && NET_ReadInt8(packet, (unsigned int *) &settings->deathmatch)
        && NET_ReadInt8(packet, (unsigned int *) &settings->episode)
        && NET_ReadInt8(packet, (unsigned int *) &settings->map)
        && NET_ReadInt8(packet, (unsigned int *) &settings->skill)
        && NET_ReadInt8(packet, (unsigned int *) &settings->gameversion)
        && NET_ReadInt8(packet, (unsigned int *) &settings->lowres_turn);
}

void NET_WriteTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff, 
                         boolean lowres_turn)
{
    // Header

    NET_WriteInt8(packet, diff->diff);

    // Write the fields which are enabled:

    if (diff->diff & NET_TICDIFF_FORWARD)
        NET_WriteInt8(packet, diff->cmd.forwardmove);
    if (diff->diff & NET_TICDIFF_SIDE)
        NET_WriteInt8(packet, diff->cmd.sidemove);
    if (diff->diff & NET_TICDIFF_TURN)
    {
        if (lowres_turn)
        {
            NET_WriteInt8(packet, diff->cmd.angleturn / 256);
        }
        else
        {
            NET_WriteInt16(packet, diff->cmd.angleturn);
        }
    }
    if (diff->diff & NET_TICDIFF_BUTTONS)
        NET_WriteInt8(packet, diff->cmd.buttons);
    if (diff->diff & NET_TICDIFF_CONSISTANCY)
        NET_WriteInt8(packet, diff->cmd.consistancy);
    if (diff->diff & NET_TICDIFF_CHATCHAR)
        NET_WriteInt8(packet, diff->cmd.chatchar);
}

boolean NET_ReadTiccmdDiff(net_packet_t *packet, net_ticdiff_t *diff,
                           boolean lowres_turn)
{
    unsigned int val;
    signed int sval;

    // Read header

    if (!NET_ReadInt8(packet, &diff->diff))
        return false;
    
    // Read fields

    if (diff->diff & NET_TICDIFF_FORWARD)
    {
        if (!NET_ReadSInt8(packet, &sval))
            return false;
        diff->cmd.forwardmove = sval;
    }

    if (diff->diff & NET_TICDIFF_SIDE)
    {
        if (!NET_ReadSInt8(packet, &sval))
            return false;
        diff->cmd.sidemove = sval;
    }

    if (diff->diff & NET_TICDIFF_TURN)
    {
        if (lowres_turn)
        {
            if (!NET_ReadSInt8(packet, &sval))
                return false;
            diff->cmd.angleturn = sval * 256;
        }
        else
        {
            if (!NET_ReadSInt16(packet, &sval))
                return false;
            diff->cmd.angleturn = sval;
        }
    }

    if (diff->diff & NET_TICDIFF_BUTTONS)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.buttons = val;
    }

    if (diff->diff & NET_TICDIFF_CONSISTANCY)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.consistancy = val;
    }

    if (diff->diff & NET_TICDIFF_CHATCHAR)
    {
        if (!NET_ReadInt8(packet, &val))
            return false;
        diff->cmd.chatchar = val;
    }

    return true;
}

void NET_TiccmdDiff(ticcmd_t *tic1, ticcmd_t *tic2, net_ticdiff_t *diff)
{
    diff->diff = 0;
    diff->cmd = *tic2;

    if (tic1->forwardmove != tic2->forwardmove)
        diff->diff |= NET_TICDIFF_FORWARD;
    if (tic1->sidemove != tic2->sidemove)
        diff->diff |= NET_TICDIFF_SIDE;
    if (tic1->angleturn != tic2->angleturn)
        diff->diff |= NET_TICDIFF_TURN;
    if (tic1->buttons != tic2->buttons)
        diff->diff |= NET_TICDIFF_BUTTONS;
    if (tic1->consistancy != tic2->consistancy)
        diff->diff |= NET_TICDIFF_CONSISTANCY;
    if (tic2->chatchar != 0)
        diff->diff |= NET_TICDIFF_CHATCHAR;
}

void NET_TiccmdPatch(ticcmd_t *src, net_ticdiff_t *diff, ticcmd_t *dest)
{
    memcpy(dest, src, sizeof(ticcmd_t));

    // Apply the diff

    if (diff->diff & NET_TICDIFF_FORWARD)
        dest->forwardmove = diff->cmd.forwardmove;
    if (diff->diff & NET_TICDIFF_SIDE)
        dest->sidemove = diff->cmd.sidemove;
    if (diff->diff & NET_TICDIFF_TURN)
        dest->angleturn = diff->cmd.angleturn;
    if (diff->diff & NET_TICDIFF_BUTTONS)
        dest->buttons = diff->cmd.buttons;
    if (diff->diff & NET_TICDIFF_CONSISTANCY)
        dest->consistancy = diff->cmd.consistancy;

    if (diff->diff & NET_TICDIFF_CHATCHAR)
        dest->chatchar = diff->cmd.chatchar;
    else
        dest->chatchar = 0;
}

// 
// net_full_ticcmd_t
// 

boolean NET_ReadFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, boolean lowres_turn)
{
    unsigned int bitfield;
    int i;

    // Regenerate playeringame from the "header" bitfield

    if (!NET_ReadInt8(packet, &bitfield))
    {
        return false;
    }
          
    for (i=0; i<MAXPLAYERS; ++i)
    {
        cmd->playeringame[i] = (bitfield & (1 << i)) != 0;
    }
        
    // Read cmds

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            if (!NET_ReadTiccmdDiff(packet, &cmd->cmds[i], lowres_turn))
            {
                return false;
            }
        }
    }

    return true;
}

void NET_WriteFullTiccmd(net_packet_t *packet, net_full_ticcmd_t *cmd, boolean lowres_turn)
{
    unsigned int bitfield;
    int i;

    // Write "header" byte indicating which players are active
    // in this ticcmd

    bitfield = 0;
    
    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            bitfield |= 1 << i;
        }
    }
    
    NET_WriteInt8(packet, bitfield);

    // Write player ticcmds

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (cmd->playeringame[i])
        {
            NET_WriteTiccmdDiff(packet, &cmd->cmds[i], lowres_turn);
        }
    }
}

