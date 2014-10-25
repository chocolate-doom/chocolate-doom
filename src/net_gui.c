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
// Graphical stuff related to the networking code:
//
//  * The client waiting screen when we are waiting for the server to
//    start the game.
//   

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "doomkeys.h"

#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_misc.h"

#include "net_client.h"
#include "net_gui.h"
#include "net_query.h"
#include "net_server.h"

#include "textscreen.h"

static txt_window_t *window;
static int old_max_players;
static txt_label_t *player_labels[NET_MAXPLAYERS];
static txt_label_t *ip_labels[NET_MAXPLAYERS];
static txt_label_t *drone_label;
static txt_label_t *master_msg_label;
static boolean had_warning;

// Number of players we expect to be in the game. When the number is
// reached, we auto-start the game (if we're the controller). If
// zero, do not autostart.
static int expected_nodes;

static void EscapePressed(TXT_UNCAST_ARG(widget), void *unused)
{
    TXT_Shutdown();
    I_Quit();
}

static void StartGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    NET_CL_LaunchGame();
}

static void OpenWaitDialog(void)
{
    txt_window_action_t *cancel;

    TXT_SetDesktopTitle(PACKAGE_STRING);

    window = TXT_NewWindow("Waiting for game start...");

    TXT_AddWidget(window, TXT_NewLabel("\nPlease wait...\n\n"));

    cancel = TXT_NewWindowAction(KEY_ESCAPE, "Cancel");
    TXT_SignalConnect(cancel, "pressed", EscapePressed, NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, cancel);
    TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_BOTTOM,
                                  TXT_SCREEN_W / 2, TXT_SCREEN_H - 9);

    old_max_players = 0;
}

static void BuildWindow(void)
{
    char buf[50];
    txt_table_t *table;
    int i;

    TXT_ClearTable(window);
    table = TXT_NewTable(3);
    TXT_AddWidget(window, table);

    // Add spacers

    TXT_AddWidget(table, NULL);
    TXT_AddWidget(table, TXT_NewStrut(25, 1));
    TXT_AddWidget(table, TXT_NewStrut(17, 1));

    // Player labels

    for (i = 0; i < net_client_wait_data.max_players; ++i)
    {
        M_snprintf(buf, sizeof(buf), " %i. ", i + 1);
        TXT_AddWidget(table, TXT_NewLabel(buf));
        player_labels[i] = TXT_NewLabel("");
        ip_labels[i] = TXT_NewLabel("");
        TXT_AddWidget(table, player_labels[i]);
        TXT_AddWidget(table, ip_labels[i]);
    }

    drone_label = TXT_NewLabel("");

    TXT_AddWidget(window, drone_label);
}

static void UpdateGUI(void)
{
    txt_window_action_t *startgame;
    char buf[50];
    unsigned int i;

    // If the value of max_players changes, we must rebuild the
    // contents of the window. This includes when the first
    // waiting data packet is received.

    if (net_client_received_wait_data)
    {
        if (net_client_wait_data.max_players != old_max_players)
        {
            BuildWindow();
        }
    }
    else
    {
        return;
    }

    for (i = 0; i < net_client_wait_data.max_players; ++i)
    {
        txt_color_t color = TXT_COLOR_BRIGHT_WHITE;

        if ((signed) i == net_client_wait_data.consoleplayer)
        {
            color = TXT_COLOR_YELLOW;
        }

        TXT_SetFGColor(player_labels[i], color);
        TXT_SetFGColor(ip_labels[i], color);

        if (i < net_client_wait_data.num_players)
        {
            TXT_SetLabel(player_labels[i],
                         net_client_wait_data.player_names[i]);
            TXT_SetLabel(ip_labels[i],
                         net_client_wait_data.player_addrs[i]);
        }
        else
        {
            TXT_SetLabel(player_labels[i], "");
            TXT_SetLabel(ip_labels[i], "");
        }
    }

    if (net_client_wait_data.num_drones > 0)
    {
        M_snprintf(buf, sizeof(buf), " (+%i observer clients)",
                   net_client_wait_data.num_drones);
        TXT_SetLabel(drone_label, buf);
    }
    else
    {
        TXT_SetLabel(drone_label, "");
    }

    if (net_client_wait_data.is_controller)
    {
        startgame = TXT_NewWindowAction(' ', "Start game");
        TXT_SignalConnect(startgame, "pressed", StartGame, NULL);
    }
    else
    {
        startgame = NULL;
    }

    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, startgame);
}

static void BuildMasterStatusWindow(void)
{
    txt_window_t *master_window;

    master_window = TXT_NewWindow(NULL);
    master_msg_label = TXT_NewLabel("");
    TXT_AddWidget(master_window, master_msg_label);

    // This window is here purely for information, so it should be
    // in the background.

    TXT_LowerWindow(master_window);
    TXT_SetWindowPosition(master_window, TXT_HORIZ_CENTER, TXT_VERT_CENTER,
                                         TXT_SCREEN_W / 2, TXT_SCREEN_H - 4);
    TXT_SetWindowAction(master_window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(master_window, TXT_HORIZ_CENTER, NULL);
    TXT_SetWindowAction(master_window, TXT_HORIZ_RIGHT, NULL);
}

static void CheckMasterStatus(void)
{
    boolean added;

    if (!NET_Query_CheckAddedToMaster(&added))
    {
        return;
    }

    if (master_msg_label == NULL)
    {
        BuildMasterStatusWindow();
    }

    if (added)
    {
        TXT_SetLabel(master_msg_label,
            "Your server is now registered with the global master server.\n"
            "Other players can find your server online.");
    }
    else
    {
        TXT_SetLabel(master_msg_label,
            "Failed to register with the master server. Your server is not\n"
            "publicly accessible. You may need to reconfigure your Internet\n"
            "router to add a port forward for UDP port 2342. Look up\n"
            "information on port forwarding online.");
    }
}

static void PrintSHA1Digest(char *s, byte *digest)
{
    unsigned int i;

    printf("%s: ", s);

    for (i=0; i<sizeof(sha1_digest_t); ++i)
    {
        printf("%02x", digest[i]);
    }

    printf("\n");
}

static void CloseWindow(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_CloseWindow(window);
}

static void CheckSHA1Sums(void)
{
    boolean correct_wad, correct_deh;
    boolean same_freedoom;
    txt_window_t *window;
    txt_window_action_t *cont_button;

    if (!net_client_received_wait_data || had_warning)
    {
        return;
    }

    correct_wad = memcmp(net_local_wad_sha1sum,
                         net_client_wait_data.wad_sha1sum, 
                         sizeof(sha1_digest_t)) == 0;
    correct_deh = memcmp(net_local_deh_sha1sum,
                         net_client_wait_data.deh_sha1sum, 
                         sizeof(sha1_digest_t)) == 0;
    same_freedoom = net_client_wait_data.is_freedoom == net_local_is_freedoom;

    if (correct_wad && correct_deh && same_freedoom)
    {
        return;
    }

    if (!correct_wad)
    {
        printf("Warning: WAD SHA1 does not match server:\n");
        PrintSHA1Digest("Local", net_local_wad_sha1sum);
        PrintSHA1Digest("Server", net_client_wait_data.wad_sha1sum);
    }

    if (!same_freedoom)
    {
        printf("Warning: Mixing Freedoom with non-Freedoom\n");
        printf("Local: %i  Server: %i\n", 
               net_local_is_freedoom, 
               net_client_wait_data.is_freedoom);
    }

    if (!correct_deh)
    {
        printf("Warning: Dehacked SHA1 does not match server:\n");
        PrintSHA1Digest("Local", net_local_deh_sha1sum);
        PrintSHA1Digest("Server", net_client_wait_data.deh_sha1sum);
    }

    window = TXT_NewWindow("WARNING!");

    cont_button = TXT_NewWindowAction(KEY_ENTER, "Continue");
    TXT_SignalConnect(cont_button, "pressed", CloseWindow, window);

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, cont_button);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);

    if (!same_freedoom)
    {
        // If Freedoom and Doom IWADs are mixed, the WAD directory
        // will be wrong, but this is not neccessarily a problem.
        // Display a different message to the WAD directory message.

        if (net_local_is_freedoom)
        {
            TXT_AddWidget(window, TXT_NewLabel
            ("You are using the Freedoom IWAD to play with players\n"
             "using an official Doom IWAD.  Make sure that you are\n"
             "playing the same levels as other players.\n"));
        }
        else
        {
            TXT_AddWidget(window, TXT_NewLabel
            ("You are using an official IWAD to play with players\n"
             "using the Freedoom IWAD.  Make sure that you are\n"
             "playing the same levels as other players.\n"));
        }
    }
    else if (!correct_wad)
    {
        TXT_AddWidget(window, TXT_NewLabel
            ("Your WAD directory does not match other players in the game.\n"
             "Check that you have loaded the exact same WAD files as other\n"
             "players.\n"));
    }

    if (!correct_deh)
    {
        TXT_AddWidget(window, TXT_NewLabel
            ("Your dehacked signature does not match other players in the\n"
             "game.  Check that you have loaded the same dehacked patches\n"
             "as other players.\n"));
    }

    TXT_AddWidget(window, TXT_NewLabel
            ("If you continue, this may cause your game to desync."));

    had_warning = true;
}

static void ParseCommandLineArgs(void)
{
    int i;

    //!
    // @arg <n>
    // @category net
    //
    // Autostart the netgame when n nodes (clients) have joined the server.
    //

    i = M_CheckParmWithArgs("-nodes", 1);
    if (i > 0)
    {
        expected_nodes = atoi(myargv[i + 1]);
    }
}

static void CheckAutoLaunch(void)
{
    int nodes;

    if (net_client_received_wait_data
     && net_client_wait_data.is_controller
     && expected_nodes > 0)
    {
        nodes = net_client_wait_data.num_players
              + net_client_wait_data.num_drones;

        if (nodes >= expected_nodes)
        {
            StartGame(NULL, NULL);
            expected_nodes = 0;
        }
    }
}

void NET_WaitForLaunch(void)
{
    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialize GUI\n");
        exit(-1);
    }

    I_InitWindowIcon();

    ParseCommandLineArgs();
    OpenWaitDialog();
    had_warning = false;

    while (net_waiting_for_launch)
    {
        UpdateGUI();
        CheckAutoLaunch();
        CheckSHA1Sums();
        CheckMasterStatus();

        TXT_DispatchEvents();
        TXT_DrawDesktop();

        NET_CL_Run();
        NET_SV_Run();

        if (!net_client_connected)
        {
            I_Error("Lost connection to server");
        }

        TXT_Sleep(100);
    }

    TXT_Shutdown();
}
