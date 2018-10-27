//
// Copyright(C) 2018 Simon Howard
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
// File globbing API. This allows the contents of the filesystem
// to be interrogated.
//

// TODO: Merge win_opendir.[ch] into this file for MSVC implementation.

#include <stdlib.h>

#include "i_glob.h"
#include "m_misc.h"
#include "config.h"

#ifdef HAVE_DIRENT_H
#include "dirent.h"

struct glob_s
{
    char *glob;
    DIR *dir;
    char *directory;
    char *last_filename;
};

glob_t *I_StartGlob(const char *directory, const char *glob)
{
    glob_t *result;

    result = malloc(sizeof(glob_t));
    if (result == NULL)
    {
        return NULL;
    }

    result->dir = opendir(directory);
    if (result->dir == NULL)
    {
        free(result);
        return NULL;
    }

    result->directory = M_StringDuplicate(directory);
    result->glob = M_StringDuplicate(glob);
    result->last_filename = NULL;
    return result;
}

void I_EndGlob(glob_t *glob)
{
    if (glob == NULL)
    {
        return;
    }

    free(glob->directory);
    free(glob->last_filename);
    free(glob->glob);
    (void) closedir(glob->dir);
    free(glob);
}

static boolean MatchesGlob(const char *name, const char *glob)
{
    while (*glob != '\0')
    {
        if (*glob == '*')
        {
            // To handle *-matching we skip past the * and recurse
            // to check each subsequent character in turn. If none
            // match then the whole match is a failure.
            while (*name != '\0')
            {
                if (MatchesGlob(name, glob + 1))
                {
                    return true;
                }
                ++name;
            }
            return false;
        }
        else if (*glob != '?' && *name != *glob)
        {
            // For normal characters the name must match the glob,
            // but for ? we don't care what the character is.
            return false;
        }

        ++name;
        ++glob;
    }

    // Match successful when glob and name end at the same time.
    return *name == '\0';
}

const char *I_NextGlob(glob_t *glob)
{
    struct dirent *de;

    do
    {
        de = readdir(glob->dir);
        if (de == NULL)
        {
            return NULL;
        }
    } while (de->d_type != DT_REG || !MatchesGlob(de->d_name, glob->glob));

    // Return the fully-qualified path, not just the bare filename.
    free(glob->last_filename);
    glob->last_filename = M_StringJoin(glob->directory, DIR_SEPARATOR_S,
                                       de->d_name, NULL);
    return glob->last_filename;
}

#else /* #ifndef HAVE_DIRENT_H */

#warning No native implementation of file globbing.

glob_t *I_StartGlob(const char *directory, const char *glob)
{
    return NULL;
}

void I_EndGlob(glob_t *glob)
{
}

const char *I_NextGlob(glob_t *glob)
{
    return "";
}

#endif /* #ifndef HAVE_DIRENT_H */

