// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005-8 Simon Howard
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
// DESCRIPTION:
//    Exit text-mode ENDOOM screen.
//
//-----------------------------------------------------------------------------

#include <string.h>

#include "doomtype.h"
#include "i_video.h"

#include "txt_main.h"

// 
// Displays the text mode ending screen after the game quits
//

void I_Endoom(byte *endoom_data)
{
    unsigned char *screendata;

    // Set up text mode screen

    TXT_Init();

    // Write the data to the screen memory
  
    screendata = TXT_GetScreenData();
    memcpy(screendata, endoom_data, 4000);

    // Wait for a keypress

    while (true)
    {
        TXT_UpdateScreen();

        if (TXT_GetChar() >= 0)
        {
            break;
        }

        TXT_Sleep(0);
    }

    // Shut down text mode screen

    TXT_Shutdown();
}

