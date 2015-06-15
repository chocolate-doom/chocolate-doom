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

// Advanced settings

#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"
#include "m_controls.h"
#include "advanced.h"
#include "display.h"

#define WINDOW_HELP_URL "http://www.chocolate-doom.org/wiki/index.php/CnDoom"

int vanilla_savegame_limit = 0;
int vanilla_demo_limit = 0;

void AdvancedSettings(void)
{
    txt_window_t *window;

    window = TXT_NewWindow("Advanced settings");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                    TXT_NewSeparator("Demo playback and ingame timer"),
            TXT_NewCheckBox("Show \"Secret is revealed!\" message",
                                   &cn_secret_message),
            TXT_NewCheckBox("Show ingame timer",
                                   &cn_timer_enabled),

                    TXT_NewSeparator("Startup delay & quickstart"),
            TXT_NewHorizBox(TXT_NewLabel("Startup delay (ms): "),
                TXT_NewSpinControl(&startup_delay, 0, 5000),
                    NULL),
            TXT_NewHorizBox(TXT_NewLabel("Quickstart (ms*10?): "),
                TXT_NewSpinControl(&cn_quickstart_delay, 0, 300),
                    NULL),

                    TXT_NewSeparator("Keyboard extra setup"),
            TXT_NewHorizBox(TXT_NewLabel("Delay between keypreses: "),
                TXT_NewSpinControl(&cn_typematic_delay, 0, 1000),
                    NULL),
            TXT_NewHorizBox(TXT_NewLabel("Character rate per second: "),
                TXT_NewSpinControl(&cn_typematic_rate, 0, 30),
                    NULL),

                    TXT_NewSeparator("Compatibility"),
            TXT_NewCheckBox("Vanilla savegame limit",
                                   &vanilla_savegame_limit),
            TXT_NewCheckBox("Vanilla demo limit",
                                   &vanilla_demo_limit),

            NULL);
}

void BindAdvancedVariables(void)
{
    if (gamemission == doom || gamemission == strife)
    {
        M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
        M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    }
}

