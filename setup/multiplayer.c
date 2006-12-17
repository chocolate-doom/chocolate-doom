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
#include <string.h>

#include "d_englsh.h"
#include "textscreen.h"

#include "configfile.h"

#include "execute.h"

#include "multiplayer.h"

#define NUM_WADS 10
#define NUM_EXTRA_PARAMS 10

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

char *net_player_name;
char *chat_macros[10];

static char *wads[NUM_WADS];
static char *extra_params[NUM_EXTRA_PARAMS];
static int skill = 2;
static int nomonsters = 0;
static int deathmatch = 0;
static int fast = 0;
static int respawn = 0;
static int udpport = 4815;
static int timer = 0;

static txt_button_t *warpbutton;
static warptype_t warptype = WARP_DOOM2;
static int warpepisode = 1;
static int warpmap = 1;

// Address to connect to when joining a game

static char *connect_address = NULL;

static void AddWADs(execute_context_t *exec)
{
    int have_wads = 0;
    int i;
    
    for (i=0; i<NUM_WADS; ++i)
    {
        if (wads[i] != NULL && strlen(wads[i]) > 0)
        {
            if (!have_wads)
            {
                AddCmdLineParameter(exec, "-file");
            }

            AddCmdLineParameter(exec, "\"%s\"", wads[i]);
        }
    }
}

static void AddExtraParameters(execute_context_t *exec)
{
    int i;
    
    for (i=0; i<NUM_EXTRA_PARAMS; ++i)
    {
        if (extra_params[i] != NULL && strlen(extra_params[i]) > 0)
        {
            AddCmdLineParameter(exec, extra_params[i]);
        }
    }
}

static void StartGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    execute_context_t *exec;

    exec = NewExecuteContext();
    AddCmdLineParameter(exec, "-server");
    AddCmdLineParameter(exec, "-skill %i", skill + 1);

    if (nomonsters)
    {
        AddCmdLineParameter(exec, "-nomonsters");
    }

    if (fast)
    {
        AddCmdLineParameter(exec, "-fast");
    }

    if (respawn)
    {
        AddCmdLineParameter(exec, "-respawn");
    }

    if (deathmatch == 1)
    {
        AddCmdLineParameter(exec, "-deathmatch");
    }
    else if (deathmatch == 2)
    {
        AddCmdLineParameter(exec, "-altdeath");
    }

    if (timer > 0)
    {
        AddCmdLineParameter(exec, "-timer %i", timer);
    }

    if (warptype == WARP_DOOM1)
    {
        // TODO: select IWAD based on warp type
        AddCmdLineParameter(exec, "-warp %i %i", warpepisode, warpmap);
    }
    else if (warptype == WARP_DOOM2)
    {
        AddCmdLineParameter(exec, "-warp %i", warpmap);
    }

    AddCmdLineParameter(exec, "-port %i", udpport);

    AddWADs(exec);
    AddExtraParameters(exec);

    TXT_Shutdown();
    
    M_SaveDefaults();
    AddConfigParameters(exec);

    ExecuteDoom(exec);

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

static void OpenExtraParamsWindow(TXT_UNCAST_ARG(widget), 
                                  TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Extra command line parameters");
    
    for (i=0; i<NUM_EXTRA_PARAMS; ++i)
    {
        TXT_AddWidget(window, TXT_NewInputBox(&extra_params[i], 70));
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
    txt_table_t *gameopt_table;
    txt_table_t *advanced_table;

    window = TXT_NewWindow("Start multiplayer game");

    TXT_AddWidgets(window, 
                   gameopt_table = TXT_NewTable(2),
                   TXT_NewSeparator("Monsters"),
                   TXT_NewInvertedCheckBox("Monsters", &nomonsters),
                   TXT_NewCheckBox("Fast monsters", &fast),
                   TXT_NewCheckBox("Respawning monsters", &respawn),
                   TXT_NewSeparator("Advanced"),
                   advanced_table = TXT_NewTable(2),
                   TXT_NewButton2("Add extra parameters...", 
                                  OpenExtraParamsWindow, NULL),
                   NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, StartGameAction());
    
    TXT_SetColumnWidths(gameopt_table, 12, 12);

    TXT_AddWidgets(gameopt_table,
           TXT_NewLabel("Skill"),
           TXT_NewDropdownList(&skill, skills, 5),
           TXT_NewLabel("Game type"),
           TXT_NewDropdownList(&deathmatch, gamemodes, 3),
           TXT_NewLabel("Level warp"),
           warpbutton = TXT_NewButton2("????", LevelSelectDialog, NULL),
           TXT_NewLabel("Time limit"),
           TXT_NewHorizBox(TXT_NewIntInputBox(&timer, 2),
                           TXT_NewLabel("minutes"),
                           NULL),
           NULL);

    TXT_SetColumnWidths(advanced_table, 12, 12);

    TXT_AddWidgets(advanced_table, 
                   TXT_NewLabel("UDP port"),
                   TXT_NewIntInputBox(&udpport, 5),
                   NULL);

    UpdateWarpButton();
}

static void DoJoinGame(void *unused1, void *unused2)
{
    execute_context_t *exec;

    exec = NewExecuteContext();

    AddCmdLineParameter(exec, "-connect %s", connect_address);
    AddExtraParameters(exec);
    AddWADs(exec);

    TXT_Shutdown();
    
    M_SaveDefaults();

    AddConfigParameters(exec);

    ExecuteDoom(exec);

    exit(0);
}

static txt_window_action_t *JoinGameAction(void)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_F10, "Connect");
    TXT_SignalConnect(action, "pressed", DoJoinGame, NULL);

    return action;
}


void JoinMultiGame(void)
{
    txt_window_t *window;

    window = TXT_NewWindow("Join multiplayer game");

    TXT_AddWidgets(window, 
        TXT_NewLabel("Connect to address: "),
        TXT_NewInputBox(&connect_address, 40),
        TXT_NewStrut(0, 1),
        TXT_NewButton2("Add extra parameters...", OpenExtraParamsWindow, NULL),
        TXT_NewButton2("Add WADs...", OpenWadsWindow, NULL),
        NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, JoinGameAction());
}

void SetChatMacroDefaults(void)
{
    int i;
    char *defaults[] = 
    {
        HUSTR_CHATMACRO1,
        HUSTR_CHATMACRO2,
        HUSTR_CHATMACRO3,
        HUSTR_CHATMACRO4,
        HUSTR_CHATMACRO5,
        HUSTR_CHATMACRO6,
        HUSTR_CHATMACRO7,
        HUSTR_CHATMACRO8,
        HUSTR_CHATMACRO9,
        HUSTR_CHATMACRO0,
    };
    
    // If the chat macros have not been set, initialise with defaults.

    for (i=0; i<10; ++i)
    {
        if (chat_macros[i] == NULL)
        {
            chat_macros[i] = strdup(defaults[i]);
        }
    }
}

void SetPlayerNameDefault(void)
{
    if (net_player_name == NULL)
    {
        net_player_name = getenv("USER");
    }

    if (net_player_name == NULL)
    {
        net_player_name = getenv("USERNAME");
    }

    if (net_player_name == NULL)
    {
        net_player_name = "player";
    }
}

void MultiplayerConfig(void)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_table_t *table;
    char buf[10];
    int i;

    window = TXT_NewWindow("Multiplayer Configuration");

    TXT_AddWidgets(window, 
                   TXT_NewStrut(0, 1),
                   TXT_NewHorizBox(TXT_NewLabel("Player name:  "),
                                   TXT_NewInputBox(&net_player_name, 25),
                                   NULL),
                   TXT_NewStrut(0, 1),
                   TXT_NewSeparator("Chat macros"),
                   NULL);

    table = TXT_NewTable(2);

    for (i=0; i<10; ++i)
    {
        sprintf(buf, "#%i ", i + 1);

        label = TXT_NewLabel(buf);
        TXT_SetFGColor(label, TXT_COLOR_BRIGHT_CYAN);

        TXT_AddWidgets(table,
                       label,
                       TXT_NewInputBox(&chat_macros[(i + 1) % 10], 40),
                       NULL);
    }
    
    TXT_AddWidget(window, table);
}

