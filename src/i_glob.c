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

#include <stdlib.h>
#include <ctype.h>

#include "i_glob.h"
#include "m_misc.h"
#include "config.h"

#if defined(_MSC_VER)
// For Visual C++, we need to include the win_opendir module.
#include <win_opendir.h>
#elif defined(HAVE_DIRENT_H)
#include <dirent.h>
#elif defined(__WATCOMC__)
// Watcom has the same API in a different header.
#include <direct.h>
#else
#define NO_DIRENT_IMPLEMENTATION
#endif

#ifndef NO_DIRENT_IMPLEMENTATION

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Only the fields d_name and (as an XSI extension) d_ino are specified
// in POSIX.1.  Other than Linux, the d_type field is available mainly
// only on BSD systems.  The remaining fields are available on many, but
// not all systems.
static boolean IsDirectory(char *dir, struct dirent *de)
{
#if defined(_DIRENT_HAVE_D_TYPE)
    if (de->d_type != DT_UNKNOWN && de->d_type != DT_LNK)
    {
        return de->d_type == DT_DIR;
    }
    else
#endif
    {
        char *filename;
        struct stat sb;
        int result;

        filename = M_StringJoin(dir, DIR_SEPARATOR_S, de->d_name, NULL);
        result = stat(filename, &sb);
        free(filename);

        if (result != 0)
        {
            return false;
        }

        return S_ISDIR(sb.st_mode);
    }
}

struct glob_s
{
    char *glob;
    int flags;
    DIR *dir;
    char *directory;
    char *last_filename;
};

glob_t *I_StartGlob(const char *directory, const char *glob, int flags)
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
    result->flags = flags;
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

static boolean MatchesGlob(const char *name, const char *glob, int flags)
{
    int n, g;

    while (*glob != '\0')
    {
        n = *name;
        g = *glob;

        if ((flags & GLOB_FLAG_NOCASE) != 0)
        {
            n = tolower(n);
            g = tolower(g);
        }

        if (g == '*')
        {
            // To handle *-matching we skip past the * and recurse
            // to check each subsequent character in turn. If none
            // match then the whole match is a failure.
            while (*name != '\0')
            {
                if (MatchesGlob(name, glob + 1, flags))
                {
                    return true;
                }
                ++name;
            }
            return glob[1] == '\0';
        }
        else if (g != '?' && n != g)
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
    } while (IsDirectory(glob->directory, de)
          || !MatchesGlob(de->d_name, glob->glob, glob->flags));

    // Return the fully-qualified path, not just the bare filename.
    free(glob->last_filename);
    glob->last_filename = M_StringJoin(glob->directory, DIR_SEPARATOR_S,
                                       de->d_name, NULL);
    return glob->last_filename;
}

#else /* #ifdef NO_DIRENT_IMPLEMENTATION */

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

#endif /* #ifdef NO_DIRENT_IMPLEMENTATION */

