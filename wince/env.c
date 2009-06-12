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

static void WCharToChar(wchar_t *src, char *dest, int buf_len)
{
    unsigned int len;

    len = wcslen(src) + 1;

    WideCharToMultiByte(CP_OEMCP, 0, src, len, dest, buf_len, NULL, NULL);
}

static void SetEnvironment(char *env_string, wchar_t *wvalue)
{
    char value[MAX_PATH + 10];
    int env_len;

    // Construct the string for putenv: NAME=value

    env_len = strlen(env_string);
    strcpy(value, env_string);

    WCharToChar(wvalue, value + env_len, sizeof(value) - env_len);

    // Set the environment variable:

    putenv(value);
}

static int ReadOwnerName(wchar_t *value, DWORD len)
{
    HKEY key;
    DWORD valtype;

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"\\ControlPanel\\Owner", 0,
                      KEY_READ, &key) != ERROR_SUCCESS)
    {
        return 0;
    }

    valtype = REG_SZ;

    if (RegQueryValueExW(key, L"Name", NULL, &valtype,
                         (LPBYTE) value, &len) != ERROR_SUCCESS)
    {
        return 0;
    }

    // Close the key

    RegCloseKey(key);

    return 1;
}

void PopulateEnvironment(void)
{
    wchar_t temp[MAX_PATH];

    // Username:

    if (ReadOwnerName(temp, MAX_PATH))
    {
        SetEnvironment("USER=", temp);
        SetEnvironment("USERNAME=", temp);
    }

    // Temp dir:

    GetTempPathW(MAX_PATH, temp);
    SetEnvironment("TEMP=", temp);

    // Use My Documents dir as home:

    SHGetSpecialFolderPath(NULL, temp, CSIDL_PERSONAL, 0);
    SetEnvironment("HOME=", temp);
}

