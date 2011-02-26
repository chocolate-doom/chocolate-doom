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

#ifdef _WIN32_WCE
#include "libc_wince.h"
#endif

#include "config.h"
#include "textscreen.h"

#include "execute.h"

#include "configfile.h"
#include "m_argv.h"

#include "setup_icon.c"

#include "compatibility.h"
#include "display.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

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
    mousebprevweapon = 4;
    mousebnextweapon = 3;
    snd_musicdevice = 3;
    joybspeed = 29;
    vanilla_savegame_limit = 0;
    vanilla_keyboard_mapping = 0;
    vanilla_demo_limit = 0;
    show_endoom = 0;
    dclick_use = 0;
    novert = 1;
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

            window = TXT_NewWindow(NULL);
            TXT_AddWidget(window, TXT_NewLabel("    \x01    "));
            TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);

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

void MainMenu(void)
{
    txt_window_t *window;
    txt_window_action_t *quit_action;
    txt_window_action_t *warp_action;

    window = TXT_NewWindow("Main Menu");

    TXT_AddWidgets(window,
          TXT_NewButton2("Configure Display", 
                         (TxtWidgetSignalFunc) ConfigDisplay, NULL),
          TXT_NewButton2("Configure Joystick", 
                         (TxtWidgetSignalFunc) ConfigJoystick, NULL),
          TXT_NewButton2("Configure Keyboard", 
                         (TxtWidgetSignalFunc) ConfigKeyboard, NULL),
          TXT_NewButton2("Configure Mouse", 
                         (TxtWidgetSignalFunc) ConfigMouse, NULL),
          TXT_NewButton2("Configure Sound", 
                         (TxtWidgetSignalFunc) ConfigSound, NULL),
          TXT_NewButton2("Compatibility", 
                         (TxtWidgetSignalFunc) CompatibilitySettings, NULL),
          TXT_NewButton2("Save parameters and launch DOOM", LaunchDoom, NULL),
          TXT_NewStrut(0, 1),
          TXT_NewButton2("Start a Network Game", 
                         (TxtWidgetSignalFunc) StartMultiGame, NULL),
          TXT_NewButton2("Join a Network Game", 
                         (TxtWidgetSignalFunc) JoinMultiGame, NULL),
          TXT_NewButton2("Multiplayer Configuration", 
                         (TxtWidgetSignalFunc) MultiplayerConfig, NULL),
          NULL);

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, "Quit");
    warp_action = TXT_NewWindowAction(KEY_F1, "Warp");
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
    M_ApplyPlatformDefaults();

    SetChatMacroDefaults();
    SetPlayerNameDefault();

    M_SetConfigDir();
    M_LoadDefaults();
}

//
// Application icon
//

static void SetIcon(void)
{
    SDL_Surface *surface;
    Uint8 *mask;
    int i;

    // Generate the mask
  
    mask = malloc(setup_icon_w * setup_icon_h / 8);
    memset(mask, 0, setup_icon_w * setup_icon_h / 8);

    for (i=0; i<setup_icon_w * setup_icon_h; ++i) 
    {
        if (setup_icon_data[i * 3] != 0x00
         || setup_icon_data[i * 3 + 1] != 0x00
         || setup_icon_data[i * 3 + 2] != 0x00)
        {
            mask[i / 8] |= 1 << (7 - i % 8);
        }
    }


    surface = SDL_CreateRGBSurfaceFrom(setup_icon_data,
                                       setup_icon_w,
                                       setup_icon_h,
                                       24,
                                       setup_icon_w * 3,
                                       0xff << 0,
                                       0xff << 8,
                                       0xff << 16,
                                       0);

    SDL_WM_SetIcon(surface, mask);
    SDL_FreeSurface(surface);
    free(mask);
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

    TXT_SetDesktopTitle(PACKAGE_NAME " Setup ver " PACKAGE_VERSION);
    SetIcon();
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
    MainMenu();

    TXT_GUIMainLoop();
}

int main(int argc, char *argv[])
{
    myargc = argc;
    myargv = argv;

#ifdef _WIN32_WCE

    // Windows CE has no environment, but SDL provides an implementation.
    // Populate the environment with the values we normally find.

    PopulateEnvironment();

#endif

    InitConfig();
    RunGUI();

    return 0;
}

