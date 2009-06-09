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
#include <string.h>

#include "doomtype.h"

#include "config.h"
#include "textscreen.h"

#include "doomtype.h"
#include "d_mode.h"
#include "d_iwad.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"

#include "compatibility.h"
#include "display.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

#include "mode.h"

GameMission_t gamemission;
static iwad_t **iwads;

typedef struct
{
    char *label;
    GameMission_t mission;
    int mask;
    char *name;
    char *config_file;
    char *extra_config_file;
    char *executable;
} mission_config_t;

// Default mission to fall back on, if no IWADs are found at all:

#define DEFAULT_MISSION (&mission_configs[0])

static mission_config_t mission_configs[] =
{
    {
        "Doom",
        doom,
        IWAD_MASK_DOOM,
        "doom",
        "default.cfg",
        PROGRAM_PREFIX "doom.cfg",
        PROGRAM_PREFIX "doom"
    },
    {
        "Heretic",
        heretic,
        IWAD_MASK_HERETIC,
        "heretic",
        "heretic.cfg",
        PROGRAM_PREFIX "heretic.cfg",
        PROGRAM_PREFIX "heretic"
    },
    {
        "Hexen",
        hexen,
        IWAD_MASK_HEXEN,
        "hexen",
        "hexen.cfg",
        PROGRAM_PREFIX "hexen.cfg",
        PROGRAM_PREFIX "hexen"
    }
};

static GameSelectCallback game_selected_callback;

// Miscellaneous variables that aren't used in setup.

static int showMessages = 1;
static int screenblocks = 9;
static int detailLevel = 0;
static char *savedir = NULL;
static char *executable = NULL;

static void BindMiscVariables(void)
{
    M_BindVariable("screenblocks",      &screenblocks);

    if (gamemission == doom)
    {
        M_BindVariable("detaillevel",       &detailLevel);
        M_BindVariable("show_messages",     &showMessages);
    }

    if (gamemission == hexen)
    {
        M_BindVariable("savedir",           &savedir);
        M_BindVariable("messageson",        &showMessages);
    }
}

//
// Initialise all configuration file bindings.
//

void InitBindings(void)
{
    // Keyboard, mouse, joystick controls

    M_BindBaseControls();
    M_BindWeaponControls();
    M_BindMapControls();
    M_BindMenuControls();

    if (gamemission == heretic || gamemission == hexen)
    {
        M_BindHereticControls();
    }

    if (gamemission == hexen)
    {
        M_BindHexenControls();
    }

    // All other variables

    BindCompatibilityVariables();
    BindDisplayVariables();
    BindJoystickVariables();
    BindKeyboardVariables();
    BindMouseVariables();
    BindSoundVariables();
    BindMiscVariables();
    BindMultiplayerVariables();
}

// Set the name of the executable program to run the game:

static void SetExecutable(mission_config_t *config)
{
    free(executable);

#ifdef _WIN32
    executable = malloc(strlen(config->executable) + 5);
    sprintf(executable, "%s.exe", config->executable);
#else
    executable = malloc(strlen(INSTALL_DIR) + strlen(config->executable) + 2);
    sprintf(executable, "%s%c%s", INSTALL_DIR, DIR_SEPARATOR,
                                  config->executable);
#endif
}

static void SetMission(mission_config_t *config)
{
    iwads = D_FindAllIWADs(config->mask);
    gamemission = config->mission;
    SetExecutable(config);
    M_SetConfigFilenames(config->config_file, config->extra_config_file);
}

static mission_config_t *GetMissionForName(char *name)
{
    int i;

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        if (!strcmp(mission_configs[i].name, name))
        {
            return &mission_configs[i];
        }
    }

    return NULL;
}

static void GameSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(config))
{
    TXT_CAST_ARG(mission_config_t, config);

    SetMission(config);
    game_selected_callback();
}

static void OpenGameSelectDialog(GameSelectCallback callback)
{
    mission_config_t *mission = NULL;
    txt_window_t *window;
    iwad_t **iwads;
    int num_games;
    int i;

    window = TXT_NewWindow("Select game");

    TXT_AddWidget(window, TXT_NewLabel("Select a game to configure:\n"));
    num_games = 0;

    // Add a button for each game.

    for (i=0; i<arrlen(mission_configs); ++i)
    {
        // Do we have any IWADs for this game installed?
        // If so, add a button.

        iwads = D_FindAllIWADs(mission_configs[i].mask);

        if (iwads[0] != NULL)
        {
            mission = &mission_configs[i];
            TXT_AddWidget(window, TXT_NewButton2(mission_configs[i].label,
                                                 GameSelected,
                                                 &mission_configs[i]));
            ++num_games;
        }

        free(iwads);
    }

    TXT_AddWidget(window, TXT_NewStrut(0, 1));

    // No IWADs found at all?  Fall back to doom, then.

    if (num_games == 0)
    {
        TXT_CloseWindow(window);
        SetMission(DEFAULT_MISSION);
        callback();
        return;
    }

    // Only one game? Use that game, and don't bother with a dialog.

    if (num_games == 1)
    {
        TXT_CloseWindow(window);
        SetMission(mission);
        callback();
        return;
    }

    game_selected_callback = callback;
}

void SetupMission(GameSelectCallback callback)
{
    mission_config_t *config;
    char *mission_name;
    int p;

    //!
    // @arg <game>
    //
    // Specify the game to configure the settings for.  Valid
    // values are 'doom', 'heretic' and 'hexen'.
    //

    p = M_CheckParm("-game");

    if (p > 0)
    {
        mission_name = myargv[p + 1];

        config = GetMissionForName(mission_name);

        if (config == NULL)
        {
            I_Error("Invalid parameter - '%s'", mission_name);
        }

        SetMission(config);
        callback();
    }
    else
    {
        OpenGameSelectDialog(callback);
    }
}

char *GetExecutableName(void)
{
    return executable;
}

iwad_t **GetIwads(void)
{
    return iwads;
}

