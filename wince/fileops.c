//
// "Extension" implementation of ANSI C file functions for Windows CE.
//
// I (Simon Howard) release this file to the public domain.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "fileops.h"

int remove(const char *pathname)
{
    wchar_t temp[MAX_PATH + 1];

    MultiByteToWideChar(CP_OEMCP,
                        0,
                        pathname,
                        strlen(pathname) + 1,
                        temp,
                        MAX_PATH);

    return DeleteFileW(temp) != 0;
}

int rename(const char *oldpath, const char *newpath)
{
    wchar_t oldpath1[MAX_PATH + 1];
    wchar_t newpath1[MAX_PATH + 1];

    MultiByteToWideChar(CP_OEMCP,
                        0,
                        oldpath,
                        strlen(oldpath) + 1,
                        oldpath1,
                        MAX_PATH);
    MultiByteToWideChar(CP_OEMCP,
                        0,
                        newpath,
                        strlen(newpath) + 1,
                        newpath1,
                        MAX_PATH);

    return MoveFileW(oldpath1, newpath1);
}

