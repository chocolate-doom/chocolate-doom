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
// Parses Text substitution sections in dehacked files
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomtype.h"

#include "z_zone.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"

typedef struct 
{
    char *from_text;
    char *to_text;
} deh_substitution_t;

static deh_substitution_t **hash_table;
static int hash_table_entries;
static int hash_table_length = -1;

// This is the algorithm used by glib

static unsigned int strhash(char *s)
{
    char *p = s;
    unsigned int h = *p;
  
    if (h)
    {
        for (p += 1; *p; p++)
            h = (h << 5) - h + *p;
    }

    return h;
}

// Look up a string to see if it has been replaced with something else
// This will be used throughout the program to substitute text

char *DEH_String(char *s)
{
    int entry;

    // Fallback if we have not initialised the hash table yet

    if (hash_table_length < 0)
	return s;

    entry = strhash(s) % hash_table_length;

    while (hash_table[entry] != NULL)
    {
        if (!strcmp(hash_table[entry]->from_text, s))
        {
            // substitution found!

            return hash_table[entry]->to_text;
        }

        entry = (entry + 1) % hash_table_length;
    }

    // no substitution found

    return s;
}

static void DEH_AddToHashtable(deh_substitution_t *sub);

static void IncreaseHashtable(void)
{
    deh_substitution_t **old_table;
    int old_table_length;
    int i;
    
    // save the old table

    old_table = hash_table;
    old_table_length = hash_table_length;
    
    // double the size 

    hash_table_length *= 2;
    hash_table = Z_Malloc(sizeof(deh_substitution_t *) * hash_table_length,
                          PU_STATIC, NULL);
    memset(hash_table, 0, sizeof(deh_substitution_t *) * hash_table_length);

    // go through the old table and insert all the old entries

    for (i=0; i<old_table_length; ++i)
    {
        if (old_table[i] != NULL)
        {
            DEH_AddToHashtable(old_table[i]);
        }
    }

    // free the old table

    Z_Free(old_table);
}

static void DEH_AddToHashtable(deh_substitution_t *sub)
{
    int entry;

    // if the hash table is more than 60% full, increase its size

    if ((hash_table_entries * 10) / hash_table_length > 6)
    {
        IncreaseHashtable();
    }
    
    // find where to insert it
    
    entry = strhash(sub->from_text) % hash_table_length;

    while (hash_table[entry] != NULL)
    {
        entry = (entry + 1) % hash_table_length;
    }

    hash_table[entry] = sub;
    ++hash_table_entries;
}

// Given a string length, find the maximum length of a 
// string that can replace it.

static int TXT_MaxStringLength(int len)
{
    // Enough bytes for the string and the NUL terminator

    len += 1;

    // All strings in doom.exe are on 4-byte boundaries, so we may be able
    // to support a slightly longer string.
    // Extend up to the next 4-byte boundary

    len += (4 - (len % 4)) % 4;
            
    // Less one for the NUL terminator.

    return len - 1;
}

static void DEH_TextInit(void)
{
    // init hash table
    
    hash_table_entries = 0;
    hash_table_length = 16;
    hash_table = Z_Malloc(sizeof(deh_substitution_t *) * hash_table_length,
                          PU_STATIC, NULL);
    memset(hash_table, 0, sizeof(deh_substitution_t *) * hash_table_length);
}

static void *DEH_TextStart(deh_context_t *context, char *line)
{
    deh_substitution_t *sub;
    int fromlen, tolen;
    int i;
    
    if (sscanf(line, "Text %i %i", &fromlen, &tolen) != 2)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    // Only allow string replacements that are possible in Vanilla Doom.  
    // Chocolate Doom is unforgiving!

    if (!deh_allow_long_strings && tolen > TXT_MaxStringLength(fromlen))
    {
        DEH_Error(context, "Replacement string is longer than the maximum "
                           "possible in doom.exe");
        return NULL;
    }

    sub = Z_Malloc(sizeof(deh_substitution_t), PU_STATIC, NULL);
    sub->from_text = Z_Malloc(fromlen + 1, PU_STATIC, NULL);
    sub->to_text = Z_Malloc(tolen + 1, PU_STATIC, NULL);

    // read in the "from" text

    for (i=0; i<fromlen; ++i)
    {
        int c;

        c = DEH_GetChar(context);
            
        sub->from_text[i] = c;
    }

    sub->from_text[fromlen] = '\0';

    // read in the "to" text

    for (i=0; i<tolen; ++i)
    {
        int c;

        c = DEH_GetChar(context);
            
        sub->to_text[i] = c;
    }
    sub->to_text[tolen] = '\0';

    DEH_AddToHashtable(sub);
    
    return NULL;
}

static void DEH_TextParseLine(deh_context_t *context, char *line, void *tag)
{
    // not used
}

deh_section_t deh_section_text =
{
    "Text",
    DEH_TextInit,
    DEH_TextStart,
    DEH_TextParseLine,
    NULL,
    NULL,
};

