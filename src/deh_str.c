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
// Parses Text substitution sections in dehacked files
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "doomtype.h"
#include "deh_str.h"
#include "m_misc.h"

#include "z_zone.h"

typedef struct 
{
    char *from_text;
    char *to_text;
} deh_substitution_t;

static deh_substitution_t **hash_table = NULL;
static int hash_table_entries;
static int hash_table_length = -1;

// This is the algorithm used by glib

static unsigned int strhash(const char *s)
{
    const char *p = s;
    unsigned int h = *p;
  
    if (h)
    {
        for (p += 1; *p; p++)
            h = (h << 5) - h + *p;
    }

    return h;
}

static deh_substitution_t *SubstitutionForString(const char *s)
{
    int entry;

    // Fallback if we have not initialized the hash table yet
    if (hash_table_length < 0)
	return NULL;

    entry = strhash(s) % hash_table_length;

    while (hash_table[entry] != NULL)
    {
        if (!strcmp(hash_table[entry]->from_text, s))
        {
            // substitution found!
            return hash_table[entry];
        }

        entry = (entry + 1) % hash_table_length;
    }

    // no substitution found
    return NULL;
}

// Look up a string to see if it has been replaced with something else
// This will be used throughout the program to substitute text

char *DEH_String(char *s)
{
    deh_substitution_t *subst;

    subst = SubstitutionForString(s);

    if (subst != NULL)
    {
        return subst->to_text;
    }
    else
    {
        return s;
    }
}

static void InitHashTable(void)
{
    // init hash table
    
    hash_table_entries = 0;
    hash_table_length = 16;
    hash_table = Z_Malloc(sizeof(deh_substitution_t *) * hash_table_length,
                          PU_STATIC, NULL);
    memset(hash_table, 0, sizeof(deh_substitution_t *) * hash_table_length);
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

void DEH_AddStringReplacement(const char *from_text, const char *to_text)
{
    deh_substitution_t *sub;
    size_t len;

    // Initialize the hash table if this is the first time
    if (hash_table_length < 0)
    {
        InitHashTable();
    }

    // Check to see if there is an existing substitution already in place.
    sub = SubstitutionForString(from_text);

    if (sub != NULL)
    {
        Z_Free(sub->to_text);

        len = strlen(to_text) + 1;
        sub->to_text = Z_Malloc(len, PU_STATIC, NULL);
        memcpy(sub->to_text, to_text, len);
    }
    else
    {
        // We need to allocate a new substitution.
        sub = Z_Malloc(sizeof(*sub), PU_STATIC, 0);

        // We need to create our own duplicates of the provided strings.
        len = strlen(from_text) + 1;
        sub->from_text = Z_Malloc(len, PU_STATIC, NULL);
        memcpy(sub->from_text, from_text, len);

        len = strlen(to_text) + 1;
        sub->to_text = Z_Malloc(len, PU_STATIC, NULL);
        memcpy(sub->to_text, to_text, len);

        DEH_AddToHashtable(sub);
    }
}

typedef enum
{
    FORMAT_ARG_INVALID,
    FORMAT_ARG_INT,
    FORMAT_ARG_FLOAT,
    FORMAT_ARG_CHAR,
    FORMAT_ARG_STRING,
    FORMAT_ARG_PTR,
    FORMAT_ARG_SAVE_POS
} format_arg_t;

// Get the type of a format argument.
// We can mix-and-match different format arguments as long as they
// are for the same data type.

static format_arg_t FormatArgumentType(char c)
{
    switch (c)
    {
        case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            return FORMAT_ARG_INT;

        case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
        case 'a': case 'A':
            return FORMAT_ARG_FLOAT;

        case 'c': case 'C':
            return FORMAT_ARG_CHAR;

        case 's': case 'S':
            return FORMAT_ARG_STRING;

        case 'p':
            return FORMAT_ARG_PTR;

        case 'n':
            return FORMAT_ARG_SAVE_POS;

        default:
            return FORMAT_ARG_INVALID;
    }
}

// Given the specified string, get the type of the first format
// string encountered.

static format_arg_t NextFormatArgument(const char **str)
{
    format_arg_t argtype;

    // Search for the '%' starting the next string.

    while (**str != '\0')
    {
        if (**str == '%')
        {
            ++*str;

            // Don't stop for double-%s.

            if (**str != '%')
            {
                break;
            }
        }

        ++*str;
    }

    // Find the type of the format string.

    while (**str != '\0')
    {
        argtype = FormatArgumentType(**str);

        if (argtype != FORMAT_ARG_INVALID)
        {
            ++*str;

            return argtype;
        }

        ++*str;
    }

    // Stop searching, we have reached the end.

    *str = NULL;

    return FORMAT_ARG_INVALID;
}

// Check if the specified argument type is a valid replacement for
// the original.

static boolean ValidArgumentReplacement(format_arg_t original,
                                        format_arg_t replacement)
{
    // In general, the original and replacement types should be
    // identical.  However, there are some cases where the replacement
    // is valid and the types don't match.

    // Characters can be represented as ints.

    if (original == FORMAT_ARG_CHAR && replacement == FORMAT_ARG_INT)
    {
        return true;
    }

    // Strings are pointers.

    if (original == FORMAT_ARG_STRING && replacement == FORMAT_ARG_PTR)
    {
        return true;
    }

    return original == replacement;
}

// Return true if the specified string contains no format arguments.

static boolean ValidFormatReplacement(const char *original, const char *replacement)
{
    const char *rover1;
    const char *rover2;
    int argtype1, argtype2;

    // Check each argument in turn and compare types.

    rover1 = original; rover2 = replacement;

    for (;;)
    {
        argtype1 = NextFormatArgument(&rover1);
        argtype2 = NextFormatArgument(&rover2);

        if (argtype2 == FORMAT_ARG_INVALID)
        {
            // No more arguments left to read from the replacement string.

            break;
        }
        else if (argtype1 == FORMAT_ARG_INVALID)
        {
            // Replacement string has more arguments than the original.

            return false;
        }
        else if (!ValidArgumentReplacement(argtype1, argtype2))
        {
            // Not a valid replacement argument.

            return false;
        }
    }

    return true;
}

// Get replacement format string, checking arguments.

static char *FormatStringReplacement(char *s)
{
    char *repl;

    repl = DEH_String(s);

    if (!ValidFormatReplacement(s, repl))
    {
        printf("WARNING: Unsafe dehacked replacement provided for "
               "printf format string: %s\n", s);

        return s;
    }

    return repl;
}

// printf(), performing a replacement on the format string.

void DEH_printf(char *fmt, ...)
{
    va_list args;
    char *repl;

    repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    vprintf(repl, args);

    va_end(args);
}

// fprintf(), performing a replacement on the format string.

void DEH_fprintf(FILE *fstream, char *fmt, ...)
{
    va_list args;
    char *repl;

    repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    vfprintf(fstream, repl, args);

    va_end(args);
}

// snprintf(), performing a replacement on the format string.

void DEH_snprintf(char *buffer, size_t len, char *fmt, ...)
{
    va_list args;
    char *repl;

    repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    M_vsnprintf(buffer, len, repl, args);

    va_end(args);
}

