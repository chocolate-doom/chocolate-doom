// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//
// Main dehacked code
//
//-----------------------------------------------------------------------------

#include <ctype.h>

#include "doomdef.h"
#include "doomtype.h"
#include "d_iwad.h"
#include "m_argv.h"
#include "w_wad.h"

#include "deh_defs.h"
#include "deh_io.h"

static char *deh_signatures[] = 
{
    "Patch File for DeHackEd v2.3",
    "Patch File for DeHackEd v3.0",
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

// If true, we can do long string replacements.

boolean deh_allow_long_strings = false;

// If true, we can do cheat replacements longer than the originals.

boolean deh_allow_long_cheats = false;

// If false, dehacked cheat replacements are ignored.

boolean deh_apply_cheats = true;

//
// List of section types:
//

static deh_section_t *section_types[] =
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
};

void DEH_Checksum(md5_digest_t digest)
{
    md5_context_t md5_context;
    unsigned int i;

    MD5_Init(&md5_context);

    for (i=0; i<arrlen(section_types); ++i)
    {
        if (section_types[i]->md5_hash != NULL)
        {
            section_types[i]->md5_hash(&md5_context);
        }
    }

    MD5_Final(digest, &md5_context);
}

// Called on startup to call the Init functions

static void InitializeSections(void)
{
    unsigned int i;

    for (i=0; i<arrlen(section_types); ++i)
    {
        if (section_types[i]->init != NULL)
        {
            section_types[i]->init();
        }
    }
}

// Given a section name, get the section structure which corresponds

static deh_section_t *GetSectionByName(char *name)
{
    unsigned int i;

    for (i=0; i<arrlen(section_types); ++i)
    {
        if (!strcasecmp(section_types[i]->name, name))
        {
            return section_types[i];
        }
    }

    return NULL;
}

// Is the string passed just whitespace?

static boolean IsWhitespace(char *s)
{
    for (; *s; ++s)
    {
        if (!isspace(*s))
            return false;
    }

    return true;
}

// Strip whitespace from the start and end of a string

static char *CleanString(char *s)
{
    char *strending;

    // Leading whitespace

    while (*s && isspace(*s))
        ++s;

    // Trailing whitespace
   
    strending = s + strlen(s) - 1;

    while (strlen(s) > 0 && isspace(*strending))
    {
        *strending = '\0';
        --strending;
    }

    return s;
}

// This pattern is used a lot of times in different sections, 
// an assignment is essentially just a statement of the form:
//
// Variable Name = Value
//
// The variable name can include spaces or any other characters.
// The string is split on the '=', essentially.
//
// Returns true if read correctly

boolean DEH_ParseAssignment(char *line, char **variable_name, char **value)
{
    char *p;

    // find the equals
    
    p = strchr(line, '=');

    if (p == NULL && p-line > 2)
    {
        return false;
    }

    // variable name at the start
    // turn the '=' into a \0 to terminate the string here

    *p = '\0';
    *variable_name = CleanString(line);
    
    // value immediately follows the '='
    
    *value = CleanString(p+1);
    
    return true;
}

static boolean CheckSignatures(deh_context_t *context)
{
    size_t i;
    char *line;
    
    // Read the first line

    line = DEH_ReadLine(context);

    if (line == NULL)
    {
        return false;
    }

    // Check all signatures to see if one matches

    for (i=0; i<arrlen(deh_signatures); ++i)
    {
        if (!strcmp(deh_signatures[i], line))
        {
            return true;
        }
    }

    return false;
}

// Parses a comment string in a dehacked file.

static void DEH_ParseComment(char *comment)
{
    // Allow comments containing this special value to allow string
    // replacements longer than those permitted by DOS dehacked.
    // This allows us to use a dehacked patch for doing string 
    // replacements for emulating Chex Quest.
    //
    // If you use this, your dehacked patch may not work in Vanilla
    // Doom.

    if (strstr(comment, "*allow-long-strings*") != NULL)
    {
        deh_allow_long_strings = true;
    }

    // Allow magic comments to allow longer cheat replacements than
    // those permitted by DOS dehacked.  This is also for Chex
    // Quest.

    if (strstr(comment, "*allow-long-cheats*") != NULL)
    {
        deh_allow_long_cheats = true;
    }
}

// Parses a dehacked file by reading from the context

static void DEH_ParseContext(deh_context_t *context)
{
    deh_section_t *current_section = NULL;
    char section_name[20];
    void *tag = NULL;
    char *line;
    
    // Read the header and check it matches the signature

    if (!CheckSignatures(context))
    {
        DEH_Error(context, "This is not a valid dehacked patch file!");
    }

    // Read the file
    
    for (;;) 
    {
        // read a new line
 
        line = DEH_ReadLine(context);

        // end of file?

        if (line == NULL)
        {
            return;
        }

        while (line[0] != '\0' && isspace(line[0]))
            ++line;

        if (line[0] == '#')
        {
            // comment

            DEH_ParseComment(line);
            continue;
        }

        if (IsWhitespace(line))
        {
            if (current_section != NULL)
            {
                // end of section

                if (current_section->end != NULL)
                {
                    current_section->end(context, tag);
                }

                //printf("end %s tag\n", current_section->name);
                current_section = NULL;
            }
        }
        else
        {
            if (current_section != NULL)
            {
                // parse this line

                current_section->line_parser(context, line, tag);
            }
            else
            {
                // possibly the start of a new section

                sscanf(line, "%19s", section_name);

                current_section = GetSectionByName(section_name);
                
                if (current_section != NULL)
                {
                    tag = current_section->start(context, line);
                    //printf("started %s tag\n", section_name);
                }
                else
                {
                    //printf("unknown section name %s\n", section_name);
                }
            }
        }
    }
}

// Parses a dehacked file

int DEH_LoadFile(char *filename)
{
    deh_context_t *context;

    // Vanilla dehacked files don't allow long string or cheat replacements.

    deh_allow_long_strings = false;
    deh_allow_long_cheats = false;

    printf(" loading %s\n", filename);

    context = DEH_OpenFile(filename);

    if (context == NULL)
    {
        fprintf(stderr, "DEH_LoadFile: Unable to open %s\n", filename);
        return 0;
    }
    
    DEH_ParseContext(context);
    
    DEH_CloseFile(context);

    return 1;
}

// Load dehacked file from WAD lump.

int DEH_LoadLump(int lumpnum)
{
    deh_context_t *context;

    // If it's in a lump, it's probably designed for a modern source port,
    // so allow it to do long string and cheat replacements.

    deh_allow_long_strings = true;
    deh_allow_long_cheats = true;

    context = DEH_OpenLump(lumpnum);

    if (context == NULL)
    {
        fprintf(stderr, "DEH_LoadFile: Unable to open lump %i\n", lumpnum);
        return 0;
    }

    DEH_ParseContext(context);

    DEH_CloseFile(context);

    return 1;
}

int DEH_LoadLumpByName(char *name)
{
    int lumpnum;

    lumpnum = W_CheckNumForName(name);

    if (lumpnum == -1)
    {
        fprintf(stderr, "DEH_LoadLumpByName: '%s' lump not found\n", name);
        return 0;
    }

    return DEH_LoadLump(lumpnum);
}

// Checks the command line for -deh argument

void DEH_Init(void)
{
    char *filename;
    int p;

    InitializeSections();

    //!
    // @category mod
    //
    // Ignore cheats in dehacked files.
    //

    if (M_CheckParm("-nocheats") > 0) 
    {
	deh_apply_cheats = false;
    }

    //!
    // @arg <files>
    // @category mod
    //
    // Load the given dehacked patch(es)
    //

    p = M_CheckParm("-deh");

    if (p > 0)
    {
        ++p;

        while (p < myargc && myargv[p][0] != '-')
        {
            filename = D_TryFindWADByName(myargv[p]);
            DEH_LoadFile(filename);
            ++p;
        }
    }
}

