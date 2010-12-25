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
#include "configfile.h"
#include "execute.h"
#include "m_argv.h"

#ifdef _WIN32
#define DOOM_BINARY PACKAGE_TARNAME ".exe"
#else
#define DOOM_BINARY PACKAGE_TARNAME
#endif

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#define PATH_SEPARATOR ';'
#else
#define DIR_SEPARATOR '/'
#define PATH_SEPARATOR ':'
#endif

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

static int ArgumentNeedsEscape(char *arg)
{
    char *p;

    for (p = arg; *p != '\0'; ++p)
    {
        if (isspace(*p))
        {
            return 1;
        }
    }

    return 0;
}

// Arguments passed to the setup tool should be passed through to the
// game when launching a game.  Calling this adds all arguments from
// myargv to the output context.

void PassThroughArguments(execute_context_t *context)
{
    int i;

    for (i = 1; i < myargc; ++i)
    {
        if (ArgumentNeedsEscape(myargv[i]))
        {
            AddCmdLineParameter(context, "\"%s\"", myargv[i]);
        }
        else
        {
            AddCmdLineParameter(context, "%s", myargv[i]);
        }
    }
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

static void ConcatWCString(wchar_t *buf, const char *value)
{
    MultiByteToWideChar(CP_OEMCP, 0,
                        value, strlen(value) + 1,
                        buf + wcslen(buf), strlen(value) + 1);
}

// Build the command line string, a wide character string of the form:
//
// "program" "arg"

static wchar_t *BuildCommandLine(const char *program, const char *arg)
{
    wchar_t exe_path[MAX_PATH];
    wchar_t *result;
    wchar_t *sep;

    // Get the path to this .exe file.

    GetModuleFileNameW(NULL, exe_path, MAX_PATH);

    // Allocate buffer to contain result string.

    result = calloc(wcslen(exe_path) + strlen(program) + strlen(arg) + 6,
                    sizeof(wchar_t));

    wcscpy(result, L"\"");

    // Copy the path part of the filename (including ending \)
    // into the result buffer:

    sep = wcsrchr(exe_path, DIR_SEPARATOR);

    if (sep != NULL)
    {
        wcsncpy(result + 1, exe_path, sep - exe_path + 1);
        result[sep - exe_path + 2] = '\0';
    }

    // Concatenate the name of the program:

    ConcatWCString(result, program);

    // End of program name, start of argument:

    wcscat(result, L"\" \"");

    ConcatWCString(result, arg);

    wcscat(result, L"\"");

    return result;
}

static int ExecuteCommand(const char *program, const char *arg)
{
    STARTUPINFOW startup_info;
    PROCESS_INFORMATION proc_info;
    wchar_t *command;
    int result = 0;

    command = BuildCommandLine(program, arg);

    // Invoke the program:

    memset(&proc_info, 0, sizeof(proc_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!CreateProcessW(NULL, command,
                        NULL, NULL, FALSE, 0, NULL, NULL,
                        &startup_info, &proc_info))
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

    free(command);

    return result;
}

#else

// Given the specified program name, get the full path to the program,
// assuming that it is in the same directory as this program is.

static char *GetFullExePath(const char *program)
{
    char *result;
    char *sep;
    unsigned int path_len;

    sep = strrchr(myargv[0], DIR_SEPARATOR);

    if (sep == NULL)
    {
        result = strdup(program);
    }
    else
    {
        path_len = sep - myargv[0] + 1;

        result = malloc(strlen(program) + path_len + 1);

        strncpy(result, myargv[0], path_len);
        result[path_len] = '\0';

        strcat(result, program);
    }

    return result;
}

static int ExecuteCommand(const char *program, const char *arg)
{
    pid_t childpid;
    int result;
    const char *argv[3];

    childpid = fork();

    if (childpid == 0) 
    {
        // This is the child.  Execute the command.

        argv[0] = GetFullExePath(program);
        argv[1] = arg;
        argv[2] = NULL;

        execvp(argv[0], (char **) argv);

        exit(0x80);
    }
    else
    {
        // This is the parent.  Wait for the child to finish, and return
        // the status code.

        waitpid(childpid, &result, 0);

        if (WIFEXITED(result) && WEXITSTATUS(result) != 0x80) 
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

    result = ExecuteCommand(DOOM_BINARY, response_file_arg);

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

    M_SaveMainDefaults(main_cfg);
    M_SaveExtraDefaults(extra_cfg);

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

// Invokes Doom to find which IWADs are installed.
// This is a cheap hack to avoid duplication of the complicated install
// path searching code inside Doom.

int FindInstalledIWADs(void)
{
    execute_context_t *context;
    int result;

    context = NewExecuteContext();
    AddCmdLineParameter(context, "-findiwads");
    result = ExecuteDoom(context);

    if (result < 0)
    {
        return 0;
    }
    else
    {
        return result;
    }
}

