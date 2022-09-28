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
//
// Top-level dehacked definitions for Strife dehacked (sehacked)
//

#include <stdlib.h>
#include "deh_defs.h"
#include "deh_main.h"

const char *deh_signatures[] =
{
    "Patch File for SeHackEd v0.4",
    "Patch File for SeHackEd v0.3",
    NULL
};


//
// List of section types:
//

deh_section_t *deh_section_types[] =
{
    &deh_section_ammo,
    &deh_section_cheat,
    &deh_section_frame,
    &deh_section_misc,
    &deh_section_pointer,
    &deh_section_sound,
    &deh_section_text,
    &deh_section_thing,
    &deh_section_weapon,
    NULL
};

