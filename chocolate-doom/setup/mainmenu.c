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
#include <stdlib.h>

#include "config.h"
#include "textscreen.h"

#include "execute.h"

#include "configfile.h"
#include "m_argv.h"

#include "compatibility.h"
#include "display.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

static void DoQuit(void *widget, void *dosave)
{
    if (dosave != NULL)
    {
        M_SaveDefaults();
    }

    exit(0);
}

static void QuitConfirm(void *unused1, void *unused2)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_button_t *yes_button;
    txt_button_t *no_button;

    window = TXT_NewWindow(NULL);

    TXT_AddWidgets(window, 
                   label = TXT_NewLabel("Save settings and\n"
                                        "quit setup?"),
                   TXT_NewStrut(24, 0),
                   yes_button = TXT_NewButton2("  Yes  ", DoQuit, DoQuit),
                   no_button = TXT_NewButton2("  No   ", DoQuit, NULL),
                   NULL);

    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(yes_button, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(no_button, TXT_HORIZ_CENTER);

    // Only an "abort" button in the middle.
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
}

static void LaunchDoom(void *unused1, void *unused2)
{
    execute_context_t *exec;
    
    // Save configuration first

    M_SaveDefaults();

    // Shut down textscreen GUI

    TXT_Shutdown();

    // Launch Doom

    exec = NewExecuteContext();
    ExecuteDoom(exec);

    exit(0);
}

void MainMenu(void)
{
    txt_window_t *window;
    txt_window_action_t *quit_action;

    window = TXT_NewWindow("Main Menu");

    TXT_AddWidgets(window,
          TXT_NewButton2("Configure display", 
                         (TxtWidgetSignalFunc) ConfigDisplay, NULL),
          TXT_NewButton2("Configure keyboard", 
                         (TxtWidgetSignalFunc) ConfigKeyboard, NULL),
          TXT_NewButton2("Configure mouse", 
                         (TxtWidgetSignalFunc) ConfigMouse, NULL),
          TXT_NewButton2("Configure sound", 
                         (TxtWidgetSignalFunc) ConfigSound, NULL),
          TXT_NewButton2("Compatibility", 
                         (TxtWidgetSignalFunc) CompatibilitySettings, NULL),
          TXT_NewButton2("Save parameters and launch DOOM", LaunchDoom, NULL),
          TXT_NewStrut(0, 1),
          TXT_NewButton2("Start a Network game", 
                         (TxtWidgetSignalFunc) StartMultiGame, NULL),
          TXT_NewButton2("Join a Network game", 
                         (TxtWidgetSignalFunc) JoinMultiGame, NULL),
          TXT_NewButton2("Multiplayer configuration", 
                         (TxtWidgetSignalFunc) MultiplayerConfig, NULL),
          NULL);

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, "Quit");
    TXT_SignalConnect(quit_action, "pressed", QuitConfirm, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, quit_action);
}

//
// Initialise all configuration variables, load config file, etc
//

static void InitConfig(void)
{
    SetChatMacroDefaults();
    SetPlayerNameDefault();

    M_SetConfigDir();
    M_LoadDefaults();
}

// 
// Initialise and run the textscreen GUI.
//

static void RunGUI(void)
{
    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialise GUI\n");
        exit(-1);
    }

    TXT_SetDesktopTitle(PACKAGE_NAME " Setup ver " PACKAGE_VERSION);
    
    MainMenu();

    TXT_GUIMainLoop();
}

int main(int argc, char *argv[])
{
    myargc = argc;
    myargv = argv;

    InitConfig();
    RunGUI();

    return 0;
}

