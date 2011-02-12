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

#include "doomtype.h"
#include "textscreen.h"

#include "d_iwad.h"
#include "m_config.h"
#include "doom/d_englsh.h"
#include "m_controls.h"

#include "multiplayer.h"
#include "mode.h"
#include "execute.h"

#define NUM_WADS 10
#define NUM_EXTRA_PARAMS 10

typedef enum
{
    WARP_ExMy,
    WARP_MAPxy,
} warptype_t;

typedef enum
{
    JOIN_AUTO_LAN,
    JOIN_ADDRESS,
} jointype_t;

// Fallback IWAD if none are found to be installed

static iwad_t fallback_iwad = { "doom2.wad", doom2, commercial, "Doom II" };
static iwad_t *fallback_iwad_list[2] = { &fallback_iwad, NULL };

// Array of IWADs found to be installed

static iwad_t **found_iwads;
static char *iwad_labels[8];

// Index of the currently selected IWAD

static int found_iwad_selected;

// Filename to pass to '-iwad'.

static char *iwadfile;

static char *doom_skills[] = 
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

static char *heretic_skills[] =
{
    "Thou needeth a wet-nurse",
    "Yellowbellies-R-us",
    "Bringest them oneth",
    "Thou art a smite-meister",
    "Black plague possesses thee"
};

static char *hexen_skills[] =
{
    "Squire/Altar boy/Apprentice",
    "Knight/Acolyte/Enchanter",
    "Warrior/Priest/Sorceror",
    "Berserker/Cardinal/Warlock",
    "Titan/Pope/Archimage"
};

static char *character_classes[] =
{
    "Fighter",
    "Cleric",
    "Mage"
};

static struct
{
    GameMission_t mission;
    char **strings;
} skills[] =
{
    { doom,    doom_skills },
    { heretic, heretic_skills },
    { hexen,   hexen_skills }
};

static char *gamemodes[] =
{
    "Co-operative",
    "Deathmatch",
    "Deathmatch 2.0",
};

static char *net_player_name;
static char *chat_macros[10];

static int jointype = JOIN_ADDRESS;

static char *wads[NUM_WADS];
static char *extra_params[NUM_EXTRA_PARAMS];
static int character_class = 0;
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
static warptype_t warptype = WARP_MAPxy;
static int warpepisode = 1;
static int warpmap = 1;

// Address to connect to when joining a game

static char *connect_address = NULL;

// Find an IWAD from its description

static iwad_t *GetCurrentIWAD(void)
{
    return found_iwads[found_iwad_selected];
}

// Is the currently selected IWAD the Chex Quest chex.wad?

static boolean IsChexQuest(iwad_t *iwad)
{
    return !strcmp(iwad->name, "chex.wad");
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

    if (gamemission == hexen)
    {
        AddCmdLineParameter(exec, "-class %i", character_class);
    }

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

    if (warptype == WARP_ExMy)
    {
        // TODO: select IWAD based on warp type
        AddCmdLineParameter(exec, "-warp %i %i", warpepisode, warpmap);
    }
    else if (warptype == WARP_MAPxy)
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

    if (warptype == WARP_ExMy)
    {
        sprintf(buf, "E%iM%i", warpepisode, warpmap);
    }
    else if (warptype == WARP_MAPxy)
    {
        sprintf(buf, "MAP%02i", warpmap);
    }

    TXT_SetButtonLabel(warpbutton, buf);
}

static void UpdateSkillButton(void)
{
    iwad_t *iwad = GetCurrentIWAD();
    int i;

    if (IsChexQuest(iwad))
    {
        skillbutton->values = chex_skills;
    }
    else
    {
        for (i=0; i<arrlen(skills); ++i)
        {
            if (gamemission == skills[i].mission)
            {
                skillbutton->values = skills[i].strings;
                break;
            }
        }
    }
}

static void SetExMyWarp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int) val;

    warpepisode = l / 10;
    warpmap = l % 10;

    UpdateWarpButton();
}

static void SetMAPxyWarp(TXT_UNCAST_ARG(widget), void *val)
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
    int episodes;
    int x, y;
    int l;
    int i;

    window = TXT_NewWindow("Select level");
    iwad = GetCurrentIWAD();

    if (warptype == WARP_ExMy)
    {
        episodes = D_GetNumEpisodes(iwad->mission, iwad->mode);
        table = TXT_NewTable(episodes);

        // ExMy levels

        for (y=1; y<10; ++y)
        {
            for (x=1; x<=episodes; ++x)
            {
                if (IsChexQuest(iwad) && (x > 1 || y > 5))
                {
                    continue;
                }

                if (!D_ValidEpisodeMap(iwad->mission, iwad->mode, x, y))
                {
                    TXT_AddWidget(table, NULL);
                    continue;
                }

                sprintf(buf, " E%iM%i ", x, y);
                button = TXT_NewButton(buf);
                TXT_SignalConnect(button, "pressed",
                                  SetExMyWarp, (void *) (x * 10 + y));
                TXT_SignalConnect(button, "pressed",
                                  CloseLevelSelectDialog, window);
                TXT_AddWidget(table, button);

                if (warpepisode == x && warpmap == y)
                {
                    TXT_SelectWidget(table, button);
                }
            }
        }
    }
    else
    {
        table = TXT_NewTable(4);

        for (i=0; i<40; ++i)
        {
            x = i % 4;
            y = i / 4;

            l = x * 10 + y + 1;

            if (!D_ValidEpisodeMap(iwad->mission, iwad->mode, 1, l))
            {
                TXT_AddWidget(table, NULL);
                continue;
            }

            sprintf(buf, " MAP%02i ", l);
            button = TXT_NewButton(buf);
            TXT_SignalConnect(button, "pressed", 
                              SetMAPxyWarp, (void *) l);
            TXT_SignalConnect(button, "pressed",
                              CloseLevelSelectDialog, window);
            TXT_AddWidget(table, button);

            if (warpmap == l)
            {
                TXT_SelectWidget(table, button);
            }
        }
    }

    TXT_AddWidget(window, table);
}

static void IWADSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    iwad_t *iwad;

    // Find the iwad_t selected

    iwad = GetCurrentIWAD();

    // Update iwadfile

    iwadfile = iwad->name;
}

// Called when the IWAD button is changed, to update warptype.

static void UpdateWarpType(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    warptype_t new_warptype;
    iwad_t *iwad;

    // Get the selected IWAD

    iwad = GetCurrentIWAD();

    // Find the new warp type

    if (D_IsEpisodeMap(iwad->mission))
    {
        new_warptype = WARP_ExMy;
    }
    else
    {
        new_warptype = WARP_MAPxy;
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
    int num_iwads;
    unsigned int i;

    // Find out what WADs are installed
    
    found_iwads = GetIwads();

    // Build a list of the descriptions for all installed IWADs

    num_iwads = 0;

    for (i=0; found_iwads[i] != NULL; ++i)
    {
        iwad_labels[i] = found_iwads[i]->description;
         ++num_iwads;
    }

    // If no IWADs are found, provide Doom 2 as an option, but
    // we're probably screwed.

    if (num_iwads == 0)
    {
        found_iwads = fallback_iwad_list;
        num_iwads = 1;
    }

    // Build a dropdown list of IWADs

    if (num_iwads < 2)
    {
        // We have only one IWAD.  Show as a label.

        result = (txt_widget_t *) TXT_NewLabel(found_iwads[0]->description);
    }
    else
    {
        // Dropdown list allowing IWAD to be selected.

        dropdown = TXT_NewDropdownList(&found_iwad_selected, 
                                       iwad_labels, num_iwads);

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
    int num_mult_types = 2;

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

    if (gamemission == doom)
    {
        num_mult_types = 3;
    }
    else
    {
        num_mult_types = 2;
    }

    TXT_AddWidgets(gameopt_table,
           TXT_NewLabel("Game"),
           iwad_selector = IWADSelector(),
           TXT_NewLabel("Skill"),
           skillbutton = TXT_NewDropdownList(&skill, doom_skills, 5),
           TXT_NewLabel("Level warp"),
           warpbutton = TXT_NewButton2("????", LevelSelectDialog, NULL),
           NULL);

    if (gamemission == hexen)
    {
        TXT_AddWidgets(gameopt_table,
                       TXT_NewLabel("Character class "),
                       TXT_NewDropdownList(&character_class,
                                           character_classes, 3),
                       NULL);
    }

    if (multiplayer)
    {
        TXT_AddWidgets(gameopt_table,
               TXT_NewLabel("Game type"),
               TXT_NewDropdownList(&deathmatch, gamemodes, num_mult_types),
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

    if (gamemission == hexen)
    {
        AddCmdLineParameter(exec, "-class %i", character_class);
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

    if (gamemission == hexen)
    {
        TXT_AddWidgets(gameopt_table,
                       TXT_NewLabel("Character class "),
                       TXT_NewDropdownList(&character_class,
                                           character_classes, 3),
                       NULL);
    }

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

void BindMultiplayerVariables(void)
{
    char buf[15];
    int i;

#ifdef FEATURE_MULTIPLAYER
    M_BindVariable("player_name", &net_player_name);
#endif

    for (i=0; i<10; ++i)
    {
        sprintf(buf, "chatmacro%i", i);
        M_BindVariable(buf, &chat_macros[i]);
    }

    switch (gamemission)
    {
        case doom:
            M_BindChatControls(4);
            key_multi_msgplayer[0] = 'g';
            key_multi_msgplayer[1] = 'i';
            key_multi_msgplayer[2] = 'b';
            key_multi_msgplayer[3] = 'r';
            break;

        case heretic:
            M_BindChatControls(4);
            key_multi_msgplayer[0] = 'g';
            key_multi_msgplayer[1] = 'y';
            key_multi_msgplayer[2] = 'r';
            key_multi_msgplayer[3] = 'b';
            break;

        case hexen:
            M_BindChatControls(8);
            key_multi_msgplayer[0] = 'b';
            key_multi_msgplayer[1] = 'r';
            key_multi_msgplayer[2] = 'y';
            key_multi_msgplayer[3] = 'g';
            key_multi_msgplayer[4] = 'j';
            key_multi_msgplayer[5] = 'w';
            key_multi_msgplayer[6] = 'h';
            key_multi_msgplayer[7] = 'p';
            break;

        default:
            break;
    }
}

