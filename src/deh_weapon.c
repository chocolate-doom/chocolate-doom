// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_weapon.c 156 2005-10-03 11:02:08Z fraggle $
//
// Copyright(C) 2005 Simon Howard
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
// $Log$
// Revision 1.2  2005/10/03 11:02:08  fraggle
// Add a weaponinfo_t mapping
//
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Parses "Weapon" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomtype.h"

#include "d_items.h"

#include "deh_defs.h"
#include "deh_mapping.h"

DEH_BEGIN_MAPPING(weapon_mapping, weaponinfo_t)
  DEH_MAPPING("Ammo type",        ammo)
  DEH_MAPPING("Deselect frame",   upstate)
  DEH_MAPPING("Select frame",     downstate)
  DEH_MAPPING("Bobbing frame",    readystate)
  DEH_MAPPING("Shooting frame",   atkstate)
  DEH_MAPPING("Firing frame",     flashstate)
DEH_END_MAPPING

static void *DEH_WeaponStart(deh_context_t *context, char *line)
{
    return NULL;
}

static void DEH_WeaponEnd(deh_context_t *context, void *tag)
{
}

static void DEH_WeaponParseLine(deh_context_t *context, char *line, void *tag)
{
}

deh_section_t deh_section_weapon =
{
    "Weapon",
    NULL,
    DEH_WeaponStart,
    DEH_WeaponParseLine,
    DEH_WeaponEnd,
};

