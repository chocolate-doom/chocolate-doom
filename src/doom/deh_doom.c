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
// Top-level dehacked definitions for Doom dehacked.
//

#include <stdlib.h>
#include "deh_defs.h"
#include "deh_main.h"

char *deh_signatures[] =
{
    "Patch File for DeHackEd v2.3",
    "Patch File for DeHackEd v3.0",
    NULL
};

// deh_ammo.c:
extern deh_section_t deh_section_ammo;
// deh_cheat.c:
extern deh_section_t deh_section_cheat;
// deh_frame.c:
extern deh_section_t deh_section_frame;
// deh_misc.c:
extern deh_section_t deh_section_misc;
// deh_ptr.c:
extern deh_section_t deh_section_pointer;
// deh_sound.c
extern deh_section_t deh_section_sound;
// deh_text.c:
extern deh_section_t deh_section_text;
// deh_thing.c:
extern deh_section_t deh_section_thing;
// deh_weapon.c:
extern deh_section_t deh_section_weapon;
// deh_bexstr.c:
extern deh_section_t deh_section_bexstr;

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
    &deh_section_bexstr,
    NULL
};

