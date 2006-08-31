#include <stdio.h>
#include <stdlib.h>

#include "textscreen.h"

#define NUM_WADS 10

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

char *wads[NUM_WADS] = {};
int skill = 0;
int nomonsters = 0;
int deathmatch = 0;
int fast = 0;
int respawn = 0;
int udpport = 4815;

static void StartGame(void)
{
    printf("Now we start the game.\n");
    exit(0);
}

static txt_window_action_t *StartGameAction(void)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_F10, "Start");
    TXT_SignalConnect(action, "pressed", StartGame, NULL);

    return action;
}

static void OpenWadsWindow(void)
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

    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator("Level warp"));

    table = TXT_NewTable(2);
    TXT_AddWidget(table, TXT_NewStrut(12, 0));
    TXT_AddWidget(table, TXT_NewStrut(12, 0));
    TXT_AddWidget(table, TXT_NewLabel("Doom 1"));
    TXT_AddWidget(table, TXT_NewButton("E1M1"));
    TXT_AddWidget(table, TXT_NewLabel("Doom 2"));
    TXT_AddWidget(table, TXT_NewButton("MAP01"));
    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator("Monsters"));
    TXT_AddWidget(window, TXT_NewInvertedCheckBox("Monsters", &nomonsters));
    TXT_AddWidget(window, TXT_NewCheckBox("Fast monsters", &fast));
    TXT_AddWidget(window, TXT_NewCheckBox("Respawning monsters", &respawn));

}

