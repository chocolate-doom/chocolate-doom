// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//-----------------------------------------------------------------------------

// M_misc.c

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

#include <ctype.h>

#include "doomdef.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_argv.h"
#include "s_sound.h"

//---------------------------------------------------------------------------
//
// FUNC M_ValidEpisodeMap
//
//---------------------------------------------------------------------------

boolean M_ValidEpisodeMap(int episode, int map)
{
    if (episode < 1 || map < 1 || map > 9)
    {
        return false;
    }
    if (gamemode == shareware)
    {                           // Shareware version checks
        if (episode != 1)
        {
            return false;
        }
    }
    else if (ExtendedWAD)
    {                           // Extended version checks
        if (episode == 6)
        {
            if (map > 3)
            {
                return false;
            }
        }
        else if (episode > 5)
        {
            return false;
        }
    }
    else
    {                           // Registered version checks
        if (episode == 4)
        {
            if (map != 1)
            {
                return false;
            }
        }
        else if (episode > 3)
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------

void M_ForceUppercase(char *text)
{
    char c;

    while ((c = *text) != 0)
    {
        if (c >= 'a' && c <= 'z')
        {
            *text++ = c - ('a' - 'A');
        }
        else
        {
            text++;
        }
    }
}

