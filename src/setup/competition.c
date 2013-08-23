// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Zvonimir Bužanić
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
                TXT_NewSeparator("* Required *"),
                TXT_NewStrut(0, 1),
                   TXT_NewHorizBox(TXT_NewLabel("Competition Doom ID: "),
                               TXT_NewIntInputBox(&cn_meta_id, 4),
                               NULL),
                TXT_NewStrut(0, 1),
                TXT_NewSeparator("* Optional *"),
                   TXT_NewHorizBox(TXT_NewLabel("First name: "),
                               TXT_NewInputBox(&cn_meta_firstname, 32),
                               NULL),
                   TXT_NewHorizBox(TXT_NewLabel("Last name: "),
                               TXT_NewInputBox(&cn_meta_lastname, 32),
                               NULL),
                   TXT_NewHorizBox(TXT_NewLabel("Nickname: "),
                               TXT_NewInputBox(&cn_meta_nickname, 32),
                               NULL),
                   TXT_NewHorizBox(TXT_NewLabel("Birthday (yyyy-mm-dd): "),
                               TXT_NewInputBox(&cn_meta_birthdate, 32),
                               NULL),
                   TXT_NewHorizBox(TXT_NewLabel("Country (eg. us): "),
                               TXT_NewInputBox(&cn_meta_country, 2),
                               NULL),
                   TXT_NewHorizBox(TXT_NewLabel("E-mail: "),
                               TXT_NewInputBox(&cn_meta_email, 32),
                               NULL),
                   TXT_NewHorizBox(TXT_NewLabel("WWW: "),
                               TXT_NewInputBox(&cn_meta_url, 32),
                               NULL),
                   NULL);
}
