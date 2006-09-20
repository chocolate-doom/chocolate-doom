// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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
#include <stdio.h>
#include <stdlib.h>

#include "textscreen.h"

#define NUM_WADS 10

typedef enum
{
    WARP_DOOM1,
    WARP_DOOM2,
} warptype_t;

static char *skills[] = 
{
    "I'm too young to die!",
    "Hey, not too rough.",
    "Hurt me plenty.",
    "Ultra-violence",
    "NIGHTMARE!",
};

static char *gamemodes[] = 
{
    "Co-operative",
    "Deathmatch",
    "Deathmatch 2.0",
};

char *player_name;
char *chatmacros[10];

char *wads[NUM_WADS] = {};
int skill = 0;
int nomonsters = 0;
int deathmatch = 0;
int fast = 0;
int respawn = 0;
int udpport = 4815;

txt_button_t *warpbutton;
warptype_t warptype = WARP_DOOM2;
int warpepisode = 1;
int warpmap = 1;

static void StartGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    printf("Now we start the game.\n");
    exit(0);
}

static void UpdateWarpButton(void)
{
    char buf[10];

    if (warptype == WARP_DOOM1)
    {
        sprintf(buf, "E%iM%i", warpepisode, warpmap);
    }
    else if (warptype == WARP_DOOM2)
    {
        sprintf(buf, "MAP%02i", warpmap);
    }

    TXT_SetButtonLabel(warpbutton, buf);
}

static void SetDoom1Warp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int) val;

    warptype = WARP_DOOM1;
    warpepisode = l / 10;
    warpmap = l % 10;

    UpdateWarpButton();
}

static void SetDoom2Warp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int) val;

    warptype = WARP_DOOM2;
    warpmap = l;

    UpdateWarpButton();
}

static void CloseLevelSelectDialog(TXT_UNCAST_ARG(button), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_CloseWindow(window);            
}

static void LevelSelectDialog(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    txt_table_t *table;
    txt_button_t *button;
    char buf[10];
    int x, y;
    int l;

    window = TXT_NewWindow("Select level");

    table = TXT_NewTable(8);

    TXT_AddWidget(window, table);

    for (y=1; y<=9; ++y)
    {
        // MAP?? levels

        for (x=0; x<4; ++x) 
        {
            l = x * 9 + y;
            
            if (l > 32)
            {
                TXT_AddWidget(table, NULL);
            }
            else
            {
                sprintf(buf, " MAP%02i ", l);
                button = TXT_NewButton(buf);
                TXT_SignalConnect(button, "pressed", 
                                  SetDoom2Warp, (void *) l);
                TXT_SignalConnect(button, "pressed",
                                  CloseLevelSelectDialog, window);
                TXT_AddWidget(table, button);

                if (warptype == WARP_DOOM2
                 && warpmap == l)
                {
                    TXT_SelectWidget(table, button);
                }
            }
        }

        // ExMy levels
        
        for (x=1; x<=4; ++x)
        {
            if (y > 9)
            {
                TXT_AddWidget(table, NULL);
            }
            else
            {
                sprintf(buf, " E%iM%i ", x, y);
                button = TXT_NewButton(buf);
                TXT_SignalConnect(button, "pressed",
                                  SetDoom1Warp, (void *) (x * 10 + y));
                TXT_SignalConnect(button, "pressed",
                                  CloseLevelSelectDialog, window);
                TXT_AddWidget(table, button);

                if (warptype == WARP_DOOM1
                 && warpepisode == x && warpmap == y)
                {
                    TXT_SelectWidget(table, button);
                }
            }
        }
    }
}

static txt_window_action_t *StartGameAction(void)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_F10, "Start");
    TXT_SignalConnect(action, "pressed", StartGame, NULL);

    return action;
}

static void OpenWadsWindow(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Add WADs");

    for (i=0; i<NUM_WADS; ++i)
    {
        TXT_AddWidget(window, TXT_NewInputBox(&wads[i], 40));
    }
}

static txt_window_action_t *WadWindowAction(void)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction('w', "Add WADs");
    TXT_SignalConnect(action, "pressed", OpenWadsWindow, NULL);

    return action;
}

void StartMultiGame(void)
{
    txt_window_t *window;
    txt_table_t *table;

    window = TXT_NewWindow("Start multiplayer game");

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, StartGameAction());
    
    table = TXT_NewTable(2);
    TXT_AddWidget(table, TXT_NewStrut(12, 0));
    TXT_AddWidget(table, TXT_NewStrut(12, 0));
    TXT_AddWidget(table, TXT_NewLabel("Skill"));
    TXT_AddWidget(table, TXT_NewDropdownList(&skill, skills, 5));
    TXT_AddWidget(table, TXT_NewLabel("Game type"));
    TXT_AddWidget(table, TXT_NewDropdownList(&deathmatch, gamemodes, 3));
    TXT_AddWidget(table, TXT_NewLabel("UDP port"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&udpport, 5));

    TXT_AddWidget(table, TXT_NewLabel("Level warp"));

    warpbutton = TXT_NewButton("????");
    TXT_AddWidget(table, warpbutton);
    TXT_SignalConnect(warpbutton, "pressed", LevelSelectDialog, NULL);
    UpdateWarpButton();
    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator("Monsters"));
    TXT_AddWidget(window, TXT_NewInvertedCheckBox("Monsters", &nomonsters));
    TXT_AddWidget(window, TXT_NewCheckBox("Fast monsters", &fast));
    TXT_AddWidget(window, TXT_NewCheckBox("Respawning monsters", &respawn));

}

void MultiplayerConfig(void)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_table_t *table;
    char buf[10];
    int i;

    window = TXT_NewWindow("Multiplayer Configuration");

    TXT_AddWidget(window, TXT_NewStrut(0, 1));

    table = TXT_NewTable(2);

    TXT_AddWidget(table, TXT_NewLabel("Player name:  "));
    TXT_AddWidget(table, TXT_NewInputBox(&player_name, 25));

    TXT_AddWidget(window, table);
    TXT_AddWidget(window, TXT_NewStrut(0, 1));
    TXT_AddWidget(window, TXT_NewSeparator("Chat macros"));

    table = TXT_NewTable(2);

    for (i=0; i<10; ++i)
    {
        sprintf(buf, "#%i ", i + 1);

        label = TXT_NewLabel(buf);
        TXT_SetFGColor(label, TXT_COLOR_BRIGHT_CYAN);
        TXT_AddWidget(table, label);
        TXT_AddWidget(table, TXT_NewInputBox(&chatmacros[i], 40));
    }
    
    TXT_AddWidget(window, table);
}

