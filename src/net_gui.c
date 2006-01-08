// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_gui.c 269 2006-01-08 05:04:50Z fraggle $
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
// Revision 1.4  2006/01/08 05:04:50  fraggle
// Don't grab the mouse on the net waiting screen
//
// Revision 1.3  2006/01/07 20:08:11  fraggle
// Send player name and address in the waiting data packets.  Display these
// on the waiting screen, and improve the waiting screen appearance.
//
// Revision 1.2  2006/01/02 21:50:26  fraggle
// Restructure the waiting screen code.  Establish our own separate event
// loop while waiting for the game to start, to avoid affecting the original
// code too much.  Move some _gui variables to net_client.c.
//
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

#include "doomstat.h"

#include "net_client.h"
#include "net_gui.h"
#include "net_server.h"

#include "d_event.h"
#include "d_main.h"
#include "i_system.h"
#include "i_video.h"
#include "m_menu.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

static patch_t *player_face;
static patch_t *player_backdrops[4];

extern void M_WriteText(int x, int y, char *string);

static void Drawer(void)
{
    patch_t *backdrop;
    int backdrop_lumpnum;
    char buf[128];
    int i, y;

    // Use INTERPIC or TITLEPIC if we don't have it

    backdrop_lumpnum = W_CheckNumForName("INTERPIC");

    if (backdrop_lumpnum < 0)
    {
        backdrop_lumpnum = W_CheckNumForName("TITLEPIC");
    }

    backdrop = (patch_t *) W_CacheLumpNum(backdrop_lumpnum, PU_CACHE);

    // draw the backdrop
    
    V_DrawPatch(0, 0, 0, backdrop);

    // draw players

    y = 100 - 16 * net_clients_in_game - 24;
    
    M_WriteText(32, y, "Players currently waiting:");

    y += 24;

    for (i=0; i<net_clients_in_game; ++i)
    {
        V_DrawPatch(32, y, 0, player_backdrops[i]);
        V_DrawPatch(32, y, 0, player_face);
        M_WriteText(80, y+12, net_player_names[i]);
        M_WriteText(200, y+12, net_player_addresses[i]);
        y += 32;
    }

    y += 16;

    if (net_client_controller)
    {
        M_WriteText(32, y, "Press space to start the game...");
    }
    else
    {
        M_WriteText(32, y, "Waiting for the game to start...");
    }
}

static void ProcessEvents(void)
{
    event_t *ev;

    while ((ev = D_PopEvent()) != NULL)
    {
        if (M_Responder(ev))
        {
            continue;
        }

        // process event ...
    }
}

static void NET_InitGUI(void)
{
    char buf[8];
    int i;

    player_face = W_CacheLumpName("STFST01", PU_STATIC);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        sprintf(buf, "STPB%d", i);
        player_backdrops[i] = W_CacheLumpName(buf, PU_STATIC);
    }
}

// Displays a graphical screen while waiting for the game to start.

void NET_WaitForStart(void)
{
    int last_tic_time;
    int nowtime;
    int runtics;
    int i;

    if (!net_client_connected || !net_waiting_for_start)
    {
        return;
    }

    NET_InitGUI();

    // cheap hack: pretend to be on a demo screen so the mouse wont 
    // be grabbed

    gamestate = GS_DEMOSCREEN;
    
    last_tic_time = I_GetTime();

    while (net_waiting_for_start)
    {
        // Keyboard/mouse events, etc.
 
        I_StartTic();
        ProcessEvents();

        // Run the menu, etc.

        nowtime = I_GetTime();
        runtics = nowtime - last_tic_time;

        if (runtics > 0) 
        {
            for (i=0; i<runtics; ++i)
            {
                M_Ticker();
            }

            last_tic_time = nowtime;

            // Draw the screen
          
            Drawer();
            M_Drawer();
            I_FinishUpdate();
        }

        // Network stuff

        NET_CL_Run();
        NET_SV_Run();

        if (!net_client_connected)
        {
            I_Error("Disconnected from server");
        }

        // Don't hog the CPU

        I_Sleep(10);
    }
}

