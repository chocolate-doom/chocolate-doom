// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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

// Code for invoking Doom

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#if defined(_WIN32_WCE)
#include "libc_wince.h"
#endif

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

#else

#include <sys/wait.h>
#include <unistd.h>

#endif

#include "textscreen.h"

#include "config.h"
#include "execute.h"
#include "mode.h"
#include "m_argv.h"
#include "m_config.h"

struct execute_context_s
{
    char *response_file;
    FILE *stream;
};

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.

static char *TempFile(char *s)
{
    char *result;
    char *tempdir;

#ifdef _WIN32
    // Check the TEMP environment variable to find the location.

    tempdir = getenv("TEMP");

    if (tempdir == NULL)
    {
        tempdir = ".";
    }
#else
    // In Unix, just use /tmp.

    tempdir = "/tmp";
#endif

    result = malloc(strlen(tempdir) + strlen(s) + 2);
    sprintf(result, "%s%c%s", tempdir, DIR_SEPARATOR, s);

    return result;
}

execute_context_t *NewExecuteContext(void)
{
    execute_context_t *result;

    result = malloc(sizeof(execute_context_t));
    
    result->response_file = TempFile("chocolat.rsp");
    result->stream = fopen(result->response_file, "w");

    if (result->stream == NULL)
    {
        fprintf(stderr, "Error opening response file\n");
        exit(-1);
    }
    
    return result;
}

void AddConfigParameters(execute_context_t *context)
{
    int p;

    p = M_CheckParm("-config");

    if (p > 0)
    {
        AddCmdLineParameter(context, "-config \"%s\"", myargv[p + 1]);
    }

    p = M_CheckParm("-extraconfig");

    if (p > 0)
    {
        AddCmdLineParameter(context, "-extraconfig \"%s\"", myargv[p + 1]);
    }
}

void AddCmdLineParameter(execute_context_t *context, char *s, ...)
{
    va_list args;

    va_start(args, s);

    vfprintf(context->stream, s, args);
    fprintf(context->stream, "\n");
}

#if defined(_WIN32)

// Wait for the specified process to exit.  Returns the exit code.

static unsigned int WaitForProcessExit(HANDLE subprocess)
{
    DWORD exit_code;

    for (;;)
    {
        WaitForSingleObject(subprocess, INFINITE);

        if (!GetExitCodeProcess(subprocess, &exit_code))
        {
            return -1;
        }

        if (exit_code != STILL_ACTIVE)
        {
            return exit_code;
        }
    }
}

static wchar_t *GetFullExePath(const char *program)
{
    wchar_t *result;
    unsigned int path_len;
    char *sep;

    // Find the full path to the EXE to execute, by taking the path
    // to this program and concatenating the EXE name:

    sep = strrchr(myargv[0], DIR_SEPARATOR);

    if (sep == NULL)
    {
        path_len = 0;
        result = calloc(strlen(program) + 1, sizeof(wchar_t));
    }
    else
    {
        path_len = sep - myargv[0] + 1;

        result = calloc(path_len + strlen(program) + 1,
                        sizeof(wchar_t));
        MultiByteToWideChar(CP_OEMCP, 0,
                            myargv[0], path_len,
                            result, path_len);
    }

    MultiByteToWideChar(CP_OEMCP, 0,
                        program, strlen(program) + 1,
                        result + path_len, strlen(program) + 1);

    return result;
}

// Convert command line argument to wchar_t string and add surrounding
// "" quotes:

static wchar_t *GetPaddedWideArg(const char *arg)
{
    wchar_t *result;
    unsigned int len = strlen(arg);

    // Convert the command line arg to a wide char string:

    result = calloc(len + 3, sizeof(wchar_t));
    MultiByteToWideChar(CP_OEMCP, 0,
                        arg, len + 1,
                        result + 1, len + 1);

    // Surrounding quotes:

    result[0] = '"';
    result[len + 1] = '"';
    result[len + 2] = 0;

    return result;
}

static int ExecuteCommand(const char *program, const char *arg)
{
    PROCESS_INFORMATION proc_info;
    wchar_t *exe_path;
    wchar_t *warg;
    int result = 0;

    exe_path = GetFullExePath(program);
    warg = GetPaddedWideArg(arg);

    // Invoke the program:

    memset(&proc_info, 0, sizeof(proc_info));

    if (!CreateProcessW(exe_path, warg,
                        NULL, NULL, FALSE, 0, NULL, NULL, NULL,
                        &proc_info))
    {
        result = -1;
    }
    else
    {
        // Wait for the process to finish, and save the exit code.

        result = WaitForProcessExit(proc_info.hProcess);

        CloseHandle(proc_info.hProcess);
        CloseHandle(proc_info.hThread);
    }

    free(exe_path);
    free(warg);

    return result;
}

#else

static int ExecuteCommand(const char *program, const char *arg)
{
    pid_t childpid;
    int result;
    const char *argv[] = { program, arg, NULL };

    childpid = fork();

    if (childpid == 0) 
    {
        // This is the child.  Execute the command.

        execv(argv[0], (char **) argv);

        exit(-1);
    }
    else
    {
        // This is the parent.  Wait for the child to finish, and return
        // the status code.

        waitpid(childpid, &result, 0);

        if (WIFEXITED(result)) 
        {
            return WEXITSTATUS(result);
        }
        else
        {
            return -1;
        }
    }
}

#endif

int ExecuteDoom(execute_context_t *context)
{
    char *response_file_arg;
    int result;
    
    fclose(context->stream);

    // Build the command line

    response_file_arg = malloc(strlen(context->response_file) + 2);
    sprintf(response_file_arg, "@%s", context->response_file);

    // Run Doom

    result = ExecuteCommand(GetExecutableName(), response_file_arg);

    free(response_file_arg);

    // Destroy context
    remove(context->response_file);
    free(context->response_file);
    free(context);

    return result;
}

static void TestCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    execute_context_t *exec;
    char *main_cfg;
    char *extra_cfg;
    txt_window_t *testwindow;
    txt_label_t *label;

    testwindow = TXT_NewWindow("Starting Doom");

    label = TXT_NewLabel("Starting Doom to test the\n"
                         "settings.  Please wait.");
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_AddWidget(testwindow, label);
    TXT_DrawDesktop();

    // Save temporary configuration files with the current configuration

    main_cfg = TempFile("tmp.cfg");
    extra_cfg = TempFile("extratmp.cfg");

    M_SaveDefaultsAlternate(main_cfg, extra_cfg);

    // Run with the -testcontrols parameter

    exec = NewExecuteContext();
    AddCmdLineParameter(exec, "-testcontrols");
    AddCmdLineParameter(exec, "-config \"%s\"", main_cfg);
    AddCmdLineParameter(exec, "-extraconfig \"%s\"", extra_cfg);
    ExecuteDoom(exec);

    TXT_CloseWindow(testwindow);

    // Delete the temporary config files

    remove(main_cfg);
    remove(extra_cfg);
    free(main_cfg);
    free(extra_cfg);
}

txt_window_action_t *TestConfigAction(void)
{
    txt_window_action_t *test_action;
    
    test_action = TXT_NewWindowAction('t', "Test");
    TXT_SignalConnect(test_action, "pressed", TestCallback, NULL);

    return test_action;
}

