//
// "Extension" implementation of getenv for Windows CE.
//
// I (Simon Howard) release this file to the public domain.
//

#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <lmcons.h>
#include <secext.h>
#include <shlobj.h>

#include "env.h"

static int buffers_loaded = 0;
static char username_buf[UNLEN + 1];
static char temp_buf[MAX_PATH + 1];
static char home_buf[MAX_PATH + 1];

static void WCharToChar(wchar_t *src, char *dest, int buf_len)
{
    unsigned int len;

    len = wcslen(src);

    WideCharToMultiByte(CP_OEMCP, 0, src, len, dest, buf_len, NULL, NULL);
}

static void LoadBuffers(void)
{
    wchar_t temp[MAX_PATH];
    DWORD buf_len;

    // Username:

    buf_len = UNLEN;
    GetUserNameExW(NameDisplay, temp, &buf_len);
    WCharToChar(temp, temp_buf, MAX_PATH);

    // Temp dir:

    GetTempPathW(MAX_PATH, temp);
    WCharToChar(temp, temp_buf, MAX_PATH);

    // Use My Documents dir as home:

    SHGetSpecialFolderPath(NULL, temp, CSIDL_PERSONAL, 0);
    WCharToChar(temp, home_buf, MAX_PATH);
}

char *getenv(const char *name)
{
    if (!buffers_loaded)
    {
        LoadBuffers();
        buffers_loaded = 1;
    }

    if (!strcmp(name, "USER") || !strcmp(name, "USERNAME"))
    {
        return username_buf;
    }
    else if (!strcmp(name, "TEMP"))
    {
        return temp_buf;
    }
    else if (!strcmp(name, "HOME"))
    {
        return home_buf;
    }
    else
    {
        return NULL;
    }
}

