// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_main.c 214 2005-10-17 23:48:05Z fraggle $
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
// Revision 1.9  2005/10/17 23:48:05  fraggle
// DEH_CheckCommandLine -> DEH_Init, for consistency with other Init
// functions
//
// Revision 1.8  2005/10/17 23:16:09  fraggle
// Add a "loading" message for each dehacked patch we load, to be orthogonal
// to the WAD loading code
//
// Revision 1.7  2005/10/17 21:14:31  fraggle
// Ignore comments in dehacked files.
//
// Revision 1.6  2005/10/09 20:06:50  fraggle
// Check the header of dehacked files and make sure we only load valid
// dehacked files.
//
// Revision 1.5  2005/10/08 20:14:38  fraggle
// Dehacked "Sound" section support
//
// Revision 1.4  2005/10/04 22:10:32  fraggle
// Dehacked "Misc" section parser (currently a dummy)
//
// Revision 1.3  2005/10/04 21:41:42  fraggle
// Rewrite cheats code.  Add dehacked cheat replacement.
//
// Revision 1.2  2005/10/03 11:08:16  fraggle
// Replace end of section functions with NULLs as they arent currently being
// used for anything.
//
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Main dehacked code
//
//-----------------------------------------------------------------------------

#include <ctype.h>
#include <strings.h>

#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"

#include "deh_defs.h"
#include "deh_io.h"

static char deh_signature[] = "Patch File for DeHackEd v3.0\n";

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
    &deh_section_text,
    &deh_section_thing,
    &deh_section_weapon,
};

static int num_section_types = sizeof(section_types) / sizeof(*section_types);

// Called on startup to call the Init functions

static void InitialiseSections(void)
{
    int i;

    for (i=0; i<num_section_types; ++i)
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
    int i;

    for (i=0; i<num_section_types; ++i)
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

static boolean CheckSignature(deh_context_t *context)
{
    int i;

    for (i=0; i<strlen(deh_signature); ++i)
    {
        int c = DEH_GetChar(context);

        if (c != deh_signature[i])
            return false;
    }

    return true;
}

// Parses a dehacked file by reading from the context

static void DEH_ParseContext(deh_context_t *context)
{
    deh_section_t *current_section = NULL;
    char section_name[20];
    void *tag = NULL;
    char *line;
    
    // Read the header and check it matches the signature

    if (!CheckSignature(context))
    {
        DEH_Error(context, "This is not a dehacked v3.0 patch file!");
    }
    
    // Read the file
    
    for (;;) 
    {
        // read a new line
 
        line = DEH_ReadLine(context);

        // end of file?

        if (line == NULL)
            return;

        while (line[0] != '\0' && isspace(line[0]))
            ++line;

        if (line[0] == '#')
        {
            // comment

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

static void DEH_ParseFile(char *filename)
{
    deh_context_t *context;

    context = DEH_OpenFile(filename);

    if (context == NULL)
    {
        fprintf(stderr, "DEH_ParseFile: Unable to open %s\n", filename);
        return;
    }
    
    DEH_ParseContext(context);
    
    DEH_CloseFile(context);
}

// Checks the command line for -deh argument

void DEH_Init(void)
{
    int argc;

    InitialiseSections();

    argc = M_CheckParm("-deh");

    if (argc > 0)
    {
        printf(" loading %s\n", myargv[argc+1]);
        DEH_ParseFile(myargv[argc+1]);
    }
}


