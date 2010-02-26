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

#define IOPERM_FILE "\\\\.\\ioperm"

#define IOCTL_IOPERM               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA00, METHOD_BUFFERED, FILE_ANY_ACCESS)

struct ioperm_data
{
    unsigned long from;
    unsigned long num;
    int turn_on;
};

static SC_HANDLE scm = NULL;
static SC_HANDLE svc = NULL;
static int service_was_created = 0;
static int service_was_started = 0;

int IOperm_EnablePortRange(unsigned int from, unsigned int num, int turn_on)
{
    HANDLE h;
    struct ioperm_data ioperm_data;
    DWORD BytesReturned;
    BOOL r;

    h = CreateFile(IOPERM_FILE, GENERIC_READ, 0, NULL,
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
    int error;
    int result = 1;

    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (scm == NULL)
    {
        error = GetLastError();
        fprintf(stderr, "IOperm_InstallDriver: OpenSCManager failed (%i)\n",
                        error);
        return 0;
    }

    svc = CreateService(scm,
                        TEXT("ioperm"),
                        TEXT("I/O port access driver"),
                        SERVICE_ALL_ACCESS,
                        SERVICE_KERNEL_DRIVER,
                        SERVICE_AUTO_START,
                        SERVICE_ERROR_NORMAL,
                        "ioperm.sys",
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
                    "IOperm_InstallDriver: Failed to create service (%i)\n",
                    error);
        }
        else
        {
            svc = OpenService(scm, TEXT("ioperm"), SERVICE_ALL_ACCESS);

            if (svc == NULL)
            {
                error = GetLastError();

                fprintf(stderr,
                        "IOperm_InstallDriver: Failed to open service (%i)\n",
                        error);
            }
        }

        if (svc == NULL)
        {
            CloseServiceHandle(scm);
            return 0;
        }
    }
    else
    {
        service_was_created = 1;
    }

    if (!StartService(svc, 0, NULL))
    {
        error = GetLastError();

        if (error != ERROR_SERVICE_ALREADY_RUNNING)
        {
            fprintf(stderr, "IOperm_InstallDriver: Failed to start service (%i)\n",
                            error);
            result = 0;
        }
        else
        {
            printf("IOperm_InstallDriver: ioperm driver already running\n");
        }
    }
    else
    {
        printf("IOperm_InstallDriver: ioperm driver installed\n");
        service_was_started = 1;
    }

    if (result == 0)
    {
        CloseServiceHandle(svc);
        CloseServiceHandle(scm);
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
        if (!ControlService(svc, SERVICE_CONTROL_STOP, &stat))
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
                        "IOperm_UninstallDriver: Failed to stop service (%i)\n",
                        error);
                result = 0;
            }
        }
    }

    // If we created the service, delete it.

    if (service_was_created)
    {
        if (!DeleteService(svc))
        {
            error = GetLastError();

            fprintf(stderr,
                    "IOperm_UninstallDriver: DeleteService failed (%i)\n",
                    error);

            result = 0;
        }
    }

    // Close handles.

    if (svc != NULL)
    {
        CloseServiceHandle(svc);
        svc = NULL;
    }

    if (scm != NULL)
    {
        CloseServiceHandle(scm);
        scm = NULL;
    }

    service_was_created = 0;
    service_was_started = 0;

    return result;
}

#endif /* #ifndef _WIN32 */

