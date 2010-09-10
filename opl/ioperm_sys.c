// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2002, 2003 Marcel Telka
// Copyright(C) 2009 Simon Howard
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
// DESCRIPTION:
//     Interface to the ioperm.sys driver, based on code from the
//     Cygwin ioperm library.
//
//-----------------------------------------------------------------------------

#ifdef _WIN32

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>

#include <errno.h>

#include "ioperm_sys.h"

#define IOPERM_FILE L"\\\\.\\ioperm"

#define IOCTL_IOPERM               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA00, METHOD_BUFFERED, FILE_ANY_ACCESS)

struct ioperm_data
{
    unsigned long from;
    unsigned long num;
    int turn_on;
};

// Function pointers for advapi32.dll.  This DLL does not exist on
// Windows 9x, so they are dynamically loaded from the DLL at runtime.

// haleyjd 09/09/10: Moved calling conventions into ()'s

static SC_HANDLE (WINAPI *MyOpenSCManagerW)(wchar_t *lpMachineName,
                                            wchar_t *lpDatabaseName,
                                            DWORD dwDesiredAccess) = NULL;
static SC_HANDLE (WINAPI *MyCreateServiceW)(SC_HANDLE hSCManager,
                                            wchar_t *lpServiceName,
                                            wchar_t *lpDisplayName,
                                            DWORD dwDesiredAccess,
                                            DWORD dwServiceType,
                                            DWORD dwStartType,
                                            DWORD dwErrorControl,
                                            wchar_t *lpBinaryPathName,
                                            wchar_t *lpLoadOrderGroup,
                                            LPDWORD lpdwTagId,
                                            wchar_t *lpDependencies,
                                            wchar_t *lpServiceStartName,
                                            wchar_t *lpPassword);
static SC_HANDLE (WINAPI *MyOpenServiceW)(SC_HANDLE hSCManager,
                                          wchar_t *lpServiceName,
                                          DWORD dwDesiredAccess);
static BOOL (WINAPI *MyStartServiceW)(SC_HANDLE hService,
                                      DWORD dwNumServiceArgs,
                                      wchar_t **lpServiceArgVectors);
static BOOL (WINAPI *MyControlService)(SC_HANDLE hService,
                                       DWORD dwControl,
                                       LPSERVICE_STATUS lpServiceStatus);
static BOOL (WINAPI *MyCloseServiceHandle)(SC_HANDLE hSCObject);
static BOOL (WINAPI *MyDeleteService)(SC_HANDLE hService);

static struct
{
    char *name;
    void **fn;
} dll_functions[] = {
    { "OpenSCManagerW",     (void **) &MyOpenSCManagerW },
    { "CreateServiceW",     (void **) &MyCreateServiceW },
    { "OpenServiceW",       (void **) &MyOpenServiceW },
    { "StartServiceW",      (void **) &MyStartServiceW },
    { "ControlService",     (void **) &MyControlService },
    { "CloseServiceHandle", (void **) &MyCloseServiceHandle },
    { "DeleteService",      (void **) &MyDeleteService },
};

// Globals

static SC_HANDLE scm = NULL;
static SC_HANDLE svc = NULL;
static int service_was_created = 0;
static int service_was_started = 0;

static int LoadLibraryPointers(void)
{
    HMODULE dll;
    int i;

    // Already loaded?

    if (MyOpenSCManagerW != NULL)
    {
        return 1;
    }

    dll = LoadLibraryW(L"advapi32.dll");

    if (dll == NULL)
    {
        fprintf(stderr, "LoadLibraryPointers: Failed to open advapi32.dll\n");
        return 0;
    }

    for (i = 0; i < sizeof(dll_functions) / sizeof(*dll_functions); ++i)
    {
        *dll_functions[i].fn = GetProcAddress(dll, dll_functions[i].name);

        if (*dll_functions[i].fn == NULL)
        {
            fprintf(stderr, "LoadLibraryPointers: Failed to get address "
                            "for '%s'\n", dll_functions[i].name);
            return 0;
        }
    }

    return 1;
}

int IOperm_EnablePortRange(unsigned int from, unsigned int num, int turn_on)
{
    HANDLE h;
    struct ioperm_data ioperm_data;
    DWORD BytesReturned;
    BOOL r;

    h = CreateFileW(IOPERM_FILE, GENERIC_READ, 0, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (h == INVALID_HANDLE_VALUE)
    {
        errno = ENODEV;
        return -1;
    }

    ioperm_data.from = from;
    ioperm_data.num = num;
    ioperm_data.turn_on = turn_on;

    r = DeviceIoControl(h, IOCTL_IOPERM,
                        &ioperm_data, sizeof ioperm_data,
                        NULL, 0,
                        &BytesReturned, NULL);

    if (!r)
    {
        errno = EPERM;
    }

    CloseHandle(h);

    return r != 0;
}

// Load ioperm.sys driver.
// Returns 1 for success, 0 for failure.
// Remember to call IOperm_UninstallDriver to uninstall the driver later.

int IOperm_InstallDriver(void)
{
    wchar_t driver_path[MAX_PATH];
    int error;
    int result = 1;

    if (!LoadLibraryPointers())
    {
        return 0;
    }

    scm = MyOpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (scm == NULL)
    {
        error = GetLastError();
        fprintf(stderr, "IOperm_InstallDriver: OpenSCManager failed (%i).\n",
                        error);
        return 0;
    }

    // Get the full path to the driver file.

    GetFullPathNameW(L"ioperm.sys", MAX_PATH, driver_path, NULL);

    // Create the service.

    svc = MyCreateServiceW(scm,
                           L"ioperm",
                           L"ioperm support for Cygwin driver",
                           SERVICE_ALL_ACCESS,
                           SERVICE_KERNEL_DRIVER,
                           SERVICE_AUTO_START,
                           SERVICE_ERROR_NORMAL,
                           driver_path,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           NULL);

    if (svc == NULL)
    {
        error = GetLastError();

        if (error != ERROR_SERVICE_EXISTS)
        {
            fprintf(stderr,
                    "IOperm_InstallDriver: Failed to create service (%i).\n",
                    error);
        }
        else
        {
            svc = MyOpenServiceW(scm, L"ioperm", SERVICE_ALL_ACCESS);

            if (svc == NULL)
            {
                error = GetLastError();

                fprintf(stderr,
                        "IOperm_InstallDriver: Failed to open service (%i).\n",
                        error);
            }
        }

        if (svc == NULL)
        {
            MyCloseServiceHandle(scm);
            return 0;
        }
    }
    else
    {
        service_was_created = 1;
    }

    // Start the service.  If the service already existed, it might have
    // already been running as well.

    if (!MyStartServiceW(svc, 0, NULL))
    {
        error = GetLastError();

        if (error != ERROR_SERVICE_ALREADY_RUNNING)
        {
            fprintf(stderr, "IOperm_InstallDriver: Failed to start service (%i).\n",
                            error);

            result = 0;
        }
        else
        {
            printf("IOperm_InstallDriver: ioperm driver already running.\n");
        }
    }
    else
    {
        printf("IOperm_InstallDriver: ioperm driver installed.\n");
        service_was_started = 1;
    }

    // If we failed to start the driver running, we need to clean up
    // before finishing.

    if (result == 0)
    {
        IOperm_UninstallDriver();
    }

    return result;
}

int IOperm_UninstallDriver(void)
{
    SERVICE_STATUS stat;
    int result = 1;
    int error;

    // If we started the service, stop it.

    if (service_was_started)
    {
        if (!MyControlService(svc, SERVICE_CONTROL_STOP, &stat))
        {
            error = GetLastError();

            if (error == ERROR_SERVICE_NOT_ACTIVE)
            {
                fprintf(stderr,
                        "IOperm_UninstallDriver: Service not active? (%i)\n",
                        error);
            }
            else
            {
                fprintf(stderr,
                        "IOperm_UninstallDriver: Failed to stop service (%i).\n",
                        error);
                result = 0;
            }
        }
    }

    // If we created the service, delete it.

    if (service_was_created)
    {
        if (!MyDeleteService(svc))
        {
            error = GetLastError();

            fprintf(stderr,
                    "IOperm_UninstallDriver: DeleteService failed (%i).\n",
                    error);

            result = 0;
        }
        else if (service_was_started)
        {
            printf("IOperm_UnInstallDriver: ioperm driver uninstalled.\n");
        }
    }

    // Close handles.

    if (svc != NULL)
    {
        MyCloseServiceHandle(svc);
        svc = NULL;
    }

    if (scm != NULL)
    {
        MyCloseServiceHandle(scm);
        scm = NULL;
    }

    service_was_created = 0;
    service_was_started = 0;

    return result;
}

#endif /* #ifndef _WIN32 */

