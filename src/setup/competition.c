//
// Copyright(C) Zvonimir Bužanić
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

// Competition options

#include <stdlib.h>

#include "m_config.h"
#include "textscreen.h"
#include "mode.h"
#include "m_controls.h"
#include "competition.h"

void ConfigCompetition(void)
{
    txt_window_t *window;

    window = TXT_NewWindow("Competition");

    TXT_AddWidgets(window, 
            TXT_NewHorizBox(TXT_NewLabel("Competition Doom ID: "),
                    TXT_NewInputBox(&cn_meta_id, 4),
                    NULL),
            NULL);
}
