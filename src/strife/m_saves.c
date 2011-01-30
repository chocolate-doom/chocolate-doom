// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1996 Rogue Entertainment / Velocity, Inc.
// Copyright(C) 2010 James Haley, Samuel Villareal
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
//
// [STRIFE] New Module
//
// Strife Hub Saving Code
//
//-----------------------------------------------------------------------------

// For GNU C and POSIX targets, dirent.h should be available. Otherwise, for
// Visual C++, we need to include the win_opendir module.
#if defined(_MSC_VER)
#include <win_opendir.h>
#elif defined(__GNUC__) || defined(POSIX)
#include <dirent.h>
#else
#error Need an include for dirent.h!
#endif

#include "z_zone.h"
#include "i_system.h"
#include "d_player.h"
#include "deh_str.h"
#include "m_misc.h"
#include "p_dialog.h"

//
// ClearTmp
//
// Clear the temporary save directory
//
void ClearTmp(void)
{
}

//
// ClearSlot
//
// Clear a single save slot folder
//
void ClearSlot(void)
{
}

//
// FromCurr
//
// Moving files from one directory to another...
// STRIFE-TODO: figure out exactly what this is for.
//
void FromCurr(void)
{
}

//
// ?????
//
// More file moving; don't even know what to call it yet.
//
void sub_1B2F4(void)
{
}

//
// M_SaveMoveMapToHere
//
// Moves a map to the "HERE" save.
//
void M_SaveMoveMapToHere(void)
{
}

//
// M_SaveMoveHereToMap
//
// Moves the "HERE" save to a map.
//
void M_SaveMoveHereToMap(void)
{
}

//
// M_SaveMisObj
//
// Writes the mission objective into the MIS_OBJ file.
//
boolean M_SaveMisObj(const char *path)
{
    char destpath[100]; // WARNING: not large enough for modern file paths!

    DEH_snprintf(destpath, sizeof(destpath), "%smis_obj", path);
    return M_WriteFile(destpath, mission_objective, OBJECTIVE_LEN);
}

//
// M_ReadMisObj
//
// Reads the mission objective from the MIS_OBJ file.
//
void M_ReadMisObj(void)
{
}

// EOF


