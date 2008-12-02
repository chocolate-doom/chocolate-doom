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

#include <string.h>

#include "config.h"

#include "doomtype.h"
#include "d_mode.h"
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

typedef struct
{
    GameMission_t mission;
    char *name;
    char *config_file;
    char *extra_config_file;
} mission_config_t;

static mission_config_t config_files[] =
{
    { doom,     "doom",    "default.cfg", PROGRAM_PREFIX "doom.cfg" },
    { heretic,  "heretic", "heretic.cfg", PROGRAM_PREFIX "heretic.cfg" },
    { hexen,    "hexen",   "hexen.cfg",   PROGRAM_PREFIX "hexen.cfg" },
};

// Miscellaneous variables that aren't used in setup.

static int showMessages = 1;
static int screenblocks = 9;
static int detailLevel = 0;
static char *savedir = NULL;

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

static void SetMission(mission_config_t *config)
{
    gamemission = config->mission;
    M_SetConfigFilenames(config->config_file, config->extra_config_file);
}

static mission_config_t *GetMissionForName(char *name)
{
    int i;

    for (i=0; i<arrlen(config_files); ++i)
    {
        if (!strcmp(config_files[i].name, name))
        {
            return &config_files[i];
        }
    }

    return NULL;
}

void SetupMission(void)
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
    }
    else
    {
        mission_name = "doom";
    }

    config = GetMissionForName(mission_name);

    if (config == NULL)
    {
        I_Error("Invalid parameter - '%s'", mission_name);
    }

    SetMission(config);
}

