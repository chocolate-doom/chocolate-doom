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
#include "doomtype.h"

#include "configfile.h"

#include "execute.h"

#include "multiplayer.h"

#define NUM_WADS 10
#define NUM_EXTRA_PARAMS 10

typedef struct
{
    char *filename;
    char *description;
    int mask;
} iwad_t;

typedef enum
{
    WARP_DOOM1,
    WARP_DOOM2,
} warptype_t;

typedef enum
{
    JOIN_AUTO_LAN,
    JOIN_ADDRESS,
} jointype_t;

static iwad_t iwads[] = 
{
    { "doom.wad",     "Doom",                 IWAD_DOOM },
    { "doom2.wad",    "Doom 2",               IWAD_DOOM2 },
    { "tnt.wad",      "Final Doom: TNT",      IWAD_TNT },
    { "plutonia.wad", "Final Doom: Plutonia", IWAD_PLUTONIA },
    { "doom1.wad",    "Doom shareware",       IWAD_DOOM1 },
    { "chex.wad",     "Chex Quest",           IWAD_CHEX },
};

// Array of IWADs found to be installed

static char *found_iwads[6];

// Index of the currently selected IWAD

static int found_iwad_selected;

// Filename to pass to '-iwad'.

static char *iwadfile;

static char *skills[] = 
{
    "I'm too young to die.",
    "Hey, not too rough.",
    "Hurt me plenty.",
    "Ultra-Violence.",
    "NIGHTMARE!",
};

static char *chex_skills[] = 
{
    "Easy does it",
    "Not so sticky",
    "Gobs of goo",
    "Extreme ooze",
    "SUPER SLIMEY!"
};

static char *gamemodes[] = 
{
    "Co-operative",
    "Deathmatch",
    "Deathmatch 2.0",
};

char *net_player_name;
char *chat_macros[10];

static int jointype = JOIN_ADDRESS;

static char *wads[NUM_WADS];
static char *extra_params[NUM_EXTRA_PARAMS];
static int skill = 2;
static int nomonsters = 0;
static int deathmatch = 0;
static int fast = 0;
static int respawn = 0;
static int udpport = 2342;
static int timer = 0;
static int privateserver = 0;

static txt_dropdown_list_t *skillbutton;
static txt_button_t *warpbutton;
static warptype_t warptype = WARP_DOOM2;
static int warpepisode = 1;
static int warpmap = 1;

// Address to connect to when joining a game

static char *connect_address = NULL;

// Find an IWAD from its description

static iwad_t *GetIWADForDescription(char *description)
{
    unsigned int i;

    for (i=0; i<arrlen(iwads); ++i)
    {
        if (!strcmp(iwads[i].description, description))
        {
            return &iwads[i];
        }
    }

    return NULL;
}

static iwad_t *GetCurrentIWAD(void)
{
    return GetIWADForDescription(found_iwads[found_iwad_selected]);
}


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

static void AddIWADParameter(execute_context_t *exec)
{
    if (iwadfile != NULL)
    {
        AddCmdLineParameter(exec, "-iwad %s", iwadfile);
    }
}

// Callback function invoked to launch the game.
// This is used when starting a server and also when starting a
// single player game via the "warp" menu.

static void StartGame(int multiplayer)
{
    execute_context_t *exec;

    exec = NewExecuteContext();

    // Extra parameters come first, before all others; this way,
    // they can override any of the options set in the dialog.

    AddExtraParameters(exec);

    AddIWADParameter(exec);
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

    if (warptype == WARP_DOOM1)
    {
        // TODO: select IWAD based on warp type
        AddCmdLineParameter(exec, "-warp %i %i", warpepisode, warpmap);
    }
    else if (warptype == WARP_DOOM2)
    {
        AddCmdLineParameter(exec, "-warp %i", warpmap);
    }

    // Multiplayer-specific options:

    if (multiplayer)
    {
        AddCmdLineParameter(exec, "-server");
        AddCmdLineParameter(exec, "-port %i", udpport);

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

        if (privateserver)
        {
            AddCmdLineParameter(exec, "-privateserver");
        }
    }

    AddWADs(exec);

    TXT_Shutdown();
    
    M_SaveDefaults();
    PassThroughArguments(exec);

    ExecuteDoom(exec);

    exit(0);
}

static void StartServerGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    StartGame(1);
}

static void StartSinglePlayerGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    StartGame(0);
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

static void UpdateSkillButton(void)
{
    iwad_t *iwad = GetCurrentIWAD();

    if (iwad->mask == IWAD_CHEX)
    {
        skillbutton->values = chex_skills;
    }
    else
    {
        skillbutton->values = skills;
    }
}

static void SetDoom1Warp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int) val;

    warpepisode = l / 10;
    warpmap = l % 10;

    UpdateWarpButton();
}

static void SetDoom2Warp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int) val;

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
    iwad_t *iwad;
    char buf[10];
    int x, y;
    int l;
    int i;

    window = TXT_NewWindow("Select level");

    table = TXT_NewTable(4);

    TXT_AddWidget(window, table);

    if (warptype == WARP_DOOM1)
    {
        // ExMy levels
        
        iwad = GetCurrentIWAD();

        for (i=0; i<4 * 9; ++i)
        {
            x = (i % 4) + 1;
            y = (i / 4) + 1;

            // chex.wad only has E1M1-E1M5.

            if (iwad->mask == IWAD_CHEX && (x > 1 || y > 5))
            {
                continue;
            }

            // doom1.wad only has E1

            if (iwad->mask == IWAD_DOOM1 && x > 1)
            {
                continue;
            }

            sprintf(buf, " E%iM%i ", x, y);
            button = TXT_NewButton(buf);
            TXT_SignalConnect(button, "pressed",
                              SetDoom1Warp, (void *) (x * 10 + y));
            TXT_SignalConnect(button, "pressed",
                              CloseLevelSelectDialog, window);
            TXT_AddWidget(table, button);

            if (warpepisode == x && warpmap == y)
            {
                TXT_SelectWidget(table, button);
            }
        }
    }
    else
    {
        for (i=0; i<32; ++i)
        {
            x = i % 4;
            y = i / 4;

            l = x * 8 + y + 1;
          
            sprintf(buf, " MAP%02i ", l);
            button = TXT_NewButton(buf);
            TXT_SignalConnect(button, "pressed", 
                              SetDoom2Warp, (void *) l);
            TXT_SignalConnect(button, "pressed",
                              CloseLevelSelectDialog, window);
            TXT_AddWidget(table, button);

            if (warpmap == l)
            {
                TXT_SelectWidget(table, button);
            }
        }
    }
}

static void IWADSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    iwad_t *iwad;

    // Find the iwad_t selected

    iwad = GetCurrentIWAD();

    // Update iwadfile

    iwadfile = iwad->filename;
}

// Called when the IWAD button is changed, to update warptype.

static void UpdateWarpType(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    warptype_t new_warptype;
    iwad_t *iwad;
    
    // Get the selected IWAD

    iwad = GetIWADForDescription(found_iwads[found_iwad_selected]);

    // Find the new warp type

    if (iwad->mask & (IWAD_DOOM | IWAD_DOOM1 | IWAD_CHEX))
    {
        new_warptype = WARP_DOOM1;
    }
    else
    {
        new_warptype = WARP_DOOM2;
    }

    // Reset to E1M1 / MAP01 when the warp type is changed.

    if (new_warptype != warptype)
    {
        warpepisode = 1;
        warpmap = 1;
    }

    warptype = new_warptype;

    UpdateWarpButton();
    UpdateSkillButton();
}

static txt_widget_t *IWADSelector(void)
{
    txt_dropdown_list_t *dropdown;
    txt_widget_t *result;
    int installed_iwads;
    int num_iwads;
    unsigned int i;

    // Find out what WADs are installed
    
    installed_iwads = FindInstalledIWADs();

    // Build a list of the descriptions for all installed IWADs

    num_iwads = 0;

    for (i=0; i<arrlen(iwads); ++i)
    {
        if (installed_iwads & iwads[i].mask)
        {
            found_iwads[num_iwads] = iwads[i].description;
            ++num_iwads;
        }
    }

    // If no IWADs are found, provide Doom 2 as an option, but
    // we're probably screwed.

    if (num_iwads == 0)
    {
        found_iwads[0] = "Doom 2";
        num_iwads = 1;
    }

    // Build a dropdown list of IWADs

    if (num_iwads < 2)
    {
        // We have only one IWAD.  Show as a label.

        result = (txt_widget_t *) TXT_NewLabel(found_iwads[0]);
    }
    else
    {
        // Dropdown list allowing IWAD to be selected.

        dropdown = TXT_NewDropdownList(&found_iwad_selected, 
                                       found_iwads, num_iwads);

        TXT_SignalConnect(dropdown, "changed", IWADSelected, NULL);

        result = (txt_widget_t *) dropdown;
    }

    // Select first in the list.

    found_iwad_selected = 0;
    IWADSelected(NULL, NULL);

    return result;
}

// Create the window action button to start the game.  This invokes
// a different callback depending on whether to start a multiplayer
// or single player game.

static txt_window_action_t *StartGameAction(int multiplayer)
{
    txt_window_action_t *action;
    TxtWidgetSignalFunc callback;

    action = TXT_NewWindowAction(KEY_F10, "Start");

    if (multiplayer)
    {
        callback = StartServerGame;
    }
    else
    {
        callback = StartSinglePlayerGame;
    }

    TXT_SignalConnect(action, "pressed", callback, NULL);

    return action;
}

static void OpenWadsWindow(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Add WADs");

    for (i=0; i<NUM_WADS; ++i)
    {
        TXT_AddWidget(window, TXT_NewInputBox(&wads[i], 60));
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

// "Start game" menu.  This is used for the start server window
// and the single player warp menu.  The parameters specify
// the window title and whether to display multiplayer options.

static void StartGameMenu(char *window_title, int multiplayer)
{
    txt_window_t *window;
    txt_table_t *gameopt_table;
    txt_table_t *advanced_table;
    txt_widget_t *iwad_selector;

    window = TXT_NewWindow(window_title);

    TXT_AddWidgets(window, 
                   gameopt_table = TXT_NewTable(2),
                   TXT_NewSeparator("Monster options"),
                   TXT_NewInvertedCheckBox("Monsters enabled", &nomonsters),
                   TXT_NewCheckBox("Fast monsters", &fast),
                   TXT_NewCheckBox("Respawning monsters", &respawn),
                   TXT_NewSeparator("Advanced"),
                   advanced_table = TXT_NewTable(2),
                   NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, StartGameAction(multiplayer));
    
    TXT_SetColumnWidths(gameopt_table, 12, 6);

    TXT_AddWidgets(gameopt_table,
           TXT_NewLabel("Game"),
           iwad_selector = IWADSelector(),
           TXT_NewLabel("Skill"),
           skillbutton = TXT_NewDropdownList(&skill, skills, 5),
           TXT_NewLabel("Level warp"),
           warpbutton = TXT_NewButton2("????", LevelSelectDialog, NULL),
           NULL);

    if (multiplayer)
    {
        TXT_AddWidgets(gameopt_table,
               TXT_NewLabel("Game type"),
               TXT_NewDropdownList(&deathmatch, gamemodes, 3),
               TXT_NewLabel("Time limit"),
               TXT_NewHorizBox(TXT_NewIntInputBox(&timer, 2),
                               TXT_NewLabel("minutes"),
                               NULL),
               NULL);

        TXT_AddWidget(window,
                      TXT_NewInvertedCheckBox("Register with master server",
                                              &privateserver));

        TXT_AddWidgets(advanced_table,
                       TXT_NewLabel("UDP port"),
                       TXT_NewIntInputBox(&udpport, 5),
                       NULL);
    }

    TXT_AddWidget(window,
                  TXT_NewButton2("Add extra parameters...", 
                                 OpenExtraParamsWindow, NULL));

    TXT_SetColumnWidths(advanced_table, 12, 6);

    TXT_SignalConnect(iwad_selector, "changed", UpdateWarpType, NULL);

    UpdateWarpType(NULL, NULL);
    UpdateWarpButton();
}

void StartMultiGame(void)
{
    StartGameMenu("Start multiplayer game", 1);
}

void WarpMenu(void)
{
    StartGameMenu("Level Warp", 0);
}

static void DoJoinGame(void *unused1, void *unused2)
{
    execute_context_t *exec;

    exec = NewExecuteContext();

    if (jointype == JOIN_ADDRESS)
    {
        AddCmdLineParameter(exec, "-connect %s", connect_address);
    }
    else if (jointype == JOIN_AUTO_LAN)
    {
        AddCmdLineParameter(exec, "-autojoin");
    }

    // Extra parameters come first, so that they can be used to override
    // the other parameters.

    AddExtraParameters(exec);
    AddIWADParameter(exec);
    AddWADs(exec);

    TXT_Shutdown();
    
    M_SaveDefaults();

    PassThroughArguments(exec);

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

// When an address is entered, select "address" mode.

static void SelectAddressJoin(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    jointype = JOIN_ADDRESS;
}

void JoinMultiGame(void)
{
    txt_window_t *window;
    txt_table_t *gameopt_table;
    txt_table_t *serveropt_table;
    txt_inputbox_t *address_box;

    window = TXT_NewWindow("Join multiplayer game");

    TXT_AddWidgets(window, 
        gameopt_table = TXT_NewTable(2),
        TXT_NewSeparator("Server"),
        serveropt_table = TXT_NewTable(2),
        TXT_NewStrut(0, 1),
        TXT_NewButton2("Add extra parameters...", OpenExtraParamsWindow, NULL),
        NULL);

    TXT_SetColumnWidths(gameopt_table, 12, 12);

    TXT_AddWidgets(gameopt_table,
                   TXT_NewLabel("Game"),
                   IWADSelector(),
                   NULL);

    TXT_AddWidgets(serveropt_table,
                   TXT_NewRadioButton("Connect to address:",
                                      &jointype, JOIN_ADDRESS),
                   address_box = TXT_NewInputBox(&connect_address, 30),
                   TXT_NewRadioButton("Auto-join LAN game",
                                      &jointype, JOIN_AUTO_LAN),
                   NULL);

    TXT_SignalConnect(address_box, "changed", SelectAddressJoin, NULL);
    TXT_SelectWidget(window, address_box);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
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
    
    // If the chat macros have not been set, initialize with defaults.

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

