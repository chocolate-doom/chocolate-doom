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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "textscreen.h"

#include "execute.h"

#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "z_zone.h"

#include "setup_icon.c"
#include "mode.h"

#include "compatibility.h"
#include "display.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup"

static const int cheat_sequence[] =
{
    KEY_UPARROW, KEY_UPARROW, KEY_DOWNARROW, KEY_DOWNARROW,
    KEY_LEFTARROW, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_RIGHTARROW,
    'b', 'a', KEY_ENTER, 0
};

static unsigned int cheat_sequence_index = 0;

// I think these are good "sensible" defaults:

static void SensibleDefaults(void)
{
    key_up = 'w';
    key_down = 's';
    key_strafeleft = 'a';
    key_straferight = 'd';
    key_jump = '/';
    key_lookup = KEY_PGUP;
    key_lookdown = KEY_PGDN;
    key_lookcenter = KEY_HOME;
    key_flyup = KEY_INS;
    key_flydown = KEY_DEL;
    key_flycenter = KEY_END;
    key_prevweapon = ',';
    key_nextweapon = '.';
    key_invleft = '[';
    key_invright = ']';
    key_message_refresh = '\'';
    key_mission = 'i';              // Strife keys
    key_invpop = 'o';
    key_invkey = 'p';
    key_multi_msgplayer[0] = 'g';
    key_multi_msgplayer[1] = 'h';
    key_multi_msgplayer[2] = 'j';
    key_multi_msgplayer[3] = 'k';
    key_multi_msgplayer[4] = 'v';
    key_multi_msgplayer[5] = 'b';
    key_multi_msgplayer[6] = 'n';
    key_multi_msgplayer[7] = 'm';
    mousebprevweapon = 4;           // Scroll wheel = weapon cycle
    mousebnextweapon = 3;
    snd_musicdevice = 3;
    joybspeed = 29;                 // Always run
    vanilla_savegame_limit = 0;
    vanilla_keyboard_mapping = 0;
    vanilla_demo_limit = 0;
    graphical_startup = 0;
    show_endoom = 0;
    dclick_use = 0;
    novert = 1;
    snd_dmxoption = "-opl3 -reverse";
    png_screenshots = 1;
}

static int MainMenuKeyPress(txt_window_t *window, int key, void *user_data)
{
    if (key == cheat_sequence[cheat_sequence_index])
    {
        ++cheat_sequence_index;

        if (cheat_sequence[cheat_sequence_index] == 0)
        {
            SensibleDefaults();
            cheat_sequence_index = 0;

            window = TXT_MessageBox(NULL, "    \x01    ");

            return 1;
        }
    }
    else
    {
        cheat_sequence_index = 0;
    }

    return 0;
}

static void DoQuit(void *widget, void *dosave)
{
    if (dosave != NULL)
    {
        M_SaveDefaults();
    }

    TXT_Shutdown();

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
                   label = TXT_NewLabel("Exiting setup.\nSave settings?"),
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
    PassThroughArguments(exec);
    ExecuteDoom(exec);

    exit(0);
}

static txt_button_t *GetLaunchButton(void)
{
    char *label;

    switch (gamemission)
    {
        case doom:
            label = "Save parameters and launch DOOM";
            break;
        case heretic:
            label = "Save parameters and launch Heretic";
            break;
        case hexen:
            label = "Save parameters and launch Hexen";
            break;
        case strife:
            label = "Save parameters and launch STRIFE!";
            break;
        default:
            label = "Save parameters and launch game";
            break;
    }

    return TXT_NewButton2(label, LaunchDoom, NULL);
}

void MainMenu(void)
{
    txt_window_t *window;
    txt_window_action_t *quit_action;
    txt_window_action_t *warp_action;

    window = TXT_NewWindow("Main Menu");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
        TXT_NewButton2("Configure Display",
                       (TxtWidgetSignalFunc) ConfigDisplay, NULL),
        TXT_NewButton2("Configure Sound",
                       (TxtWidgetSignalFunc) ConfigSound, NULL),
        TXT_NewButton2("Configure Keyboard",
                       (TxtWidgetSignalFunc) ConfigKeyboard, NULL),
        TXT_NewButton2("Configure Mouse",
                       (TxtWidgetSignalFunc) ConfigMouse, NULL),
        TXT_NewButton2("Configure Gamepad/Joystick",
                       (TxtWidgetSignalFunc) ConfigJoystick, NULL),
        TXT_NewButton2("Compatibility",
                       (TxtWidgetSignalFunc) CompatibilitySettings, NULL),
        GetLaunchButton(),
        TXT_NewStrut(0, 1),
        TXT_NewButton2("Start a Network Game",
                       (TxtWidgetSignalFunc) StartMultiGame, NULL),
        TXT_NewButton2("Join a Network Game",
                       (TxtWidgetSignalFunc) JoinMultiGame, NULL),
        TXT_NewButton2("Multiplayer Configuration",
                       (TxtWidgetSignalFunc) MultiplayerConfig, NULL),
        NULL);

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, "Quit");
    warp_action = TXT_NewWindowAction(KEY_F2, "Warp");
    TXT_SignalConnect(quit_action, "pressed", QuitConfirm, NULL);
    TXT_SignalConnect(warp_action, "pressed",
                      (TxtWidgetSignalFunc) WarpMenu, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, quit_action);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, warp_action);

    TXT_SetKeyListener(window, MainMenuKeyPress, NULL);
}

//
// Initialize all configuration variables, load config file, etc
//

static void InitConfig(void)
{
    M_SetConfigDir(NULL);
    InitBindings();

    SetChatMacroDefaults();
    SetPlayerNameDefault();

    M_LoadDefaults();
}

//
// Application icon
//

static void SetIcon(void)
{
    extern SDL_Window *TXT_SDLWindow;
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *) setup_icon_data, setup_icon_w,
                                       setup_icon_h, 32, setup_icon_w * 4,
                                       0xff << 24, 0xff << 16,
                                       0xff << 8, 0xff << 0);

    SDL_SetWindowIcon(TXT_SDLWindow, surface);
    SDL_FreeSurface(surface);
}

static void SetWindowTitle(void)
{
    char *title;

    title = M_StringReplace(PACKAGE_NAME " Setup ver " PACKAGE_VERSION,
                            "Doom",
                            GetGameTitle());


    TXT_SetDesktopTitle(title);

    free(title);
}

// Initialize the textscreen library.

static void InitTextscreen(void)
{
    SetDisplayDriver();

    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialize GUI\n");
        exit(-1);
    }

    SetIcon();
    SetWindowTitle();
}

// Restart the textscreen library.  Used when the video_driver variable
// is changed.

void RestartTextscreen(void)
{
    TXT_Shutdown();
    InitTextscreen();
}

// 
// Initialize and run the textscreen GUI.
//

static void RunGUI(void)
{
    InitTextscreen();

    TXT_GUIMainLoop();
}

static void MissionSet(void)
{
    SetWindowTitle();
    InitConfig();
    MainMenu();
}

void D_DoomMain(void)
{
    SetupMission(MissionSet);

    RunGUI();
}
