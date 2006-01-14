// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_gui.c 294 2006-01-14 00:27:16Z fraggle $
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
// Revision 1.12  2006/01/14 00:27:16  fraggle
// Set the window caption and title
//
// Revision 1.11  2006/01/14 00:13:04  fraggle
// Detect if disconnected from the server while waiting for the game start.
//
// Revision 1.10  2006/01/14 00:10:54  fraggle
// Change the format of color commands.  Reorganise the waiting dialog.
//
// Revision 1.9  2006/01/13 23:56:00  fraggle
// Add text-mode I/O functions.
// Use text-mode screen for the waiting screen.
//
// Revision 1.8  2006/01/12 02:11:52  fraggle
// Game start packets
//
// Revision 1.7  2006/01/10 22:14:13  fraggle
// Shut up compiler warnings
//
// Revision 1.6  2006/01/09 02:03:39  fraggle
// Send clients their player number, and indicate on the waiting screen
// which client we are.
//
// Revision 1.5  2006/01/08 17:52:45  fraggle
// Play some random music for the players while waiting for the game to
// start.
//
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

#include <ctype.h>

#include "config.h"
#include "doomstat.h"

#include "i_system.h"
#include "i_video.h"

#include "net_client.h"
#include "net_gui.h"
#include "net_server.h"

#include "txt_main.h"
#include "txt_gui.h"
#include "txt_io.h"

static void ProcessEvents(void)
{
    int c;
    
    while ((c = TXT_GetChar()) > 0)
    {
        switch (tolower(c))
        {
            case 27:
            case 'q':
                TXT_Shutdown();
                I_Quit();
                break;

            case ' ':
                NET_CL_StartGame();
                break;
        }
    }
}

#define WINDOW_X 15
#define WINDOW_Y 5
#define WINDOW_W 50
#define WINDOW_H 12

static void DrawScreen(void)
{
    char buf[40];
    int i;

    TXT_DrawDesktop(PACKAGE_STRING);
    TXT_DrawWindow("Waiting for game start...", 
                    WINDOW_X, WINDOW_Y, 
                    WINDOW_W, WINDOW_H);

    TXT_BGColor(TXT_COLOR_BLUE, 0);

    for (i=0; i<MAXPLAYERS; ++i)
    {
        if (i == net_player_number)
            TXT_FGColor(TXT_COLOR_YELLOW);
        else if (i < net_clients_in_game)
            TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
        else
            TXT_FGColor(TXT_COLOR_GREY);

        snprintf(buf, 39, "%i. ", i + 1);
        TXT_GotoXY(WINDOW_X + 2, WINDOW_Y + 4 + i);
        TXT_Puts(buf);

        if (i < net_clients_in_game)
        {
            snprintf(buf, 15, "%s", net_player_names[i]);
            TXT_GotoXY(WINDOW_X + 5, WINDOW_Y + 4 + i);
            TXT_Puts(buf);

            snprintf(buf, 16, "%s", net_player_addresses[i]);
            TXT_GotoXY(WINDOW_X + 33, WINDOW_Y + 4 + i);
            TXT_Puts(buf);
        }
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_GotoXY(WINDOW_X + 2, WINDOW_Y + WINDOW_H - 2);
    TXT_Puts("<brightgreen>ESC</><brightcyan>=</>Abort");

    if (net_client_controller)
    {
        TXT_GotoXY(WINDOW_X + WINDOW_W - 18, WINDOW_Y + WINDOW_H - 2);
        TXT_Puts("<brightgreen>SPACE</><brightcyan>=</>Start game");
    }
    
    TXT_DrawSeparator(WINDOW_X, WINDOW_Y + WINDOW_H - 3, WINDOW_W);

    TXT_UpdateScreen();
}

void NET_WaitForStart(void)
{
    TXT_Init();
    I_SetWindowCaption();
    I_SetWindowIcon();

    while (net_waiting_for_start)
    {
        ProcessEvents();
        DrawScreen();

        NET_CL_Run();
        NET_SV_Run();

        if (!net_client_connected)
        {
            I_Error("Disconnected from server");
        }

        I_Sleep(50);
    }
    
    TXT_Shutdown();
}

