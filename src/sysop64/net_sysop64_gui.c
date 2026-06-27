//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
// DESCRIPTION:
//     Sysop-64 network GUI stubs for builds without SDL windows.
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "net_client.h"
#include "net_gui.h"
#include "net_server.h"

static int expected_nodes = 0;

// Read Sysop network launcher options that replace the interactive SDL lobby.
static void ParseCommandLineArgs(void)
{
    int i = M_CheckParmWithArgs("-nodes", 1);
    if (i > 0) {
        expected_nodes = atoi(myargv[i + 1]);
    }
}

// When this client is the controller, auto-launch once the requested node
// count has joined.
static void CheckAutoLaunch(void)
{
    int nodes;

    if (net_client_received_wait_data
     && net_client_wait_data.is_controller
     && expected_nodes > 0) {
        nodes = net_client_wait_data.num_players + net_client_wait_data.num_drones;
        if (nodes >= expected_nodes) {
            NET_CL_LaunchGame();
            expected_nodes = 0;
        }
    }
}

// Run the network client/server wait loop using console progress output instead
// of Chocolate Doom's normal SDL network GUI.
void NET_WaitForLaunch(void)
{
    ParseCommandLineArgs();
    printf("Waiting for netgame launch");
    fflush(stdout);

    while (net_waiting_for_launch) {
        NET_CL_Run();
        NET_SV_Run();
        CheckAutoLaunch();

        if (!net_client_connected) {
            I_Error("Lost connection to server");
        }

        if (net_client_received_wait_data) {
            printf("\rWaiting for netgame launch: %d/%d players ",
                   net_client_wait_data.num_players,
                   net_client_wait_data.max_players);
            fflush(stdout);
        }

        I_Sleep(100);
    }

    printf("\nNetgame launched.\n");
}
