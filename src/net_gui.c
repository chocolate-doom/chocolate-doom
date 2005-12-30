// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_gui.c 235 2005-12-30 18:58:22Z fraggle $
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
// Graphical stuff related to the networking code:
//
//  * The client waiting screen when we are waiting for the server to
//    start the game.
//   

#include "net_gui.h"
#include "d_event.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

extern void M_WriteText(int x, int y, char *string);

// if TRUE, this client is the controller of the game

boolean net_client_controller = false;

// Number of clients currently connected to the server

int net_clients_in_game;

void NET_Drawer(void)
{
    patch_t *backdrop;
    int backdrop_lumpnum;
    char buf[128];

    // Use INTERPIC or TITLEPIC if we don't have it

    backdrop_lumpnum = W_CheckNumForName("INTERPIC");

    if (backdrop_lumpnum < 0)
    {
        backdrop_lumpnum = W_CheckNumForName("TITLEPIC");
    }

    backdrop = (patch_t *) W_CacheLumpNum(backdrop_lumpnum, PU_CACHE);

    // draw the backdrop
    
    V_DrawPatch(0, 0, 0, backdrop);

    sprintf(buf, "%i clients connected to server.", net_clients_in_game);

    M_WriteText(32, 100, buf);

    if (net_client_controller)
    {
        M_WriteText(32, 150, "Press space to start the game...");
    }
    else
    {
        M_WriteText(32, 150, "Waiting for the game to start...");
    }
}

boolean NET_Responder(event_t *event)
{
    return true;
}

