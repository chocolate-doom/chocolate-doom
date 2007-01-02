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

#ifndef WINDOWS
    #include <sys/wait.h>
#else
    #define WEXITSTATUS(stat_val) ((stat_val) & 255) 
    #define WIFEXITED(stat_val) (((stat_val) & 0xff00) == 0)
#endif

#include "textscreen.h"

#include "configfile.h"
#include "execute.h"
#include "m_argv.h"

#ifdef _WIN32
#define DOOM_BINARY "chocolate-doom.exe"
#else
#define DOOM_BINARY INSTALL_DIR "/chocolate-doom"
#endif

struct execute_context_s
{
    char *response_file;
    FILE *stream;
};

execute_context_t *NewExecuteContext(void)
{
    execute_context_t *result;

    result = malloc(sizeof(execute_context_t));
    
#ifdef _WIN32
    result->response_file = "chocolat.rsp";
#else
    result->response_file = "/tmp/chocolate.rsp";
#endif

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

int ExecuteDoom(execute_context_t *context)
{
    char *cmdline;
    int result;
    
    fclose(context->stream);

    // Build the command line

    cmdline = malloc(strlen(DOOM_BINARY) 
                     + strlen(context->response_file) + 20);

    sprintf(cmdline, "%s @%s", DOOM_BINARY, context->response_file);
    
    // Run the command
    result = system(cmdline);

    free(cmdline);
    
    // Destroy context 
    remove(context->response_file);
    free(context);

    if (WIFEXITED(result))
    {
        return WEXITSTATUS(result);
    }
    else
    {
        return -1;
    }
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

#ifdef _WIN32
    main_cfg = "tmp.cfg";
    extra_cfg = "extratmp.cfg";
#else
    main_cfg = "/tmp/tmp.cfg";
    extra_cfg = "/tmp/extratmp.cfg";
#endif

    M_SaveMainDefaults(main_cfg);
    M_SaveExtraDefaults(extra_cfg);

    // Run with the -testcontrols parameter

    exec = NewExecuteContext();
    AddCmdLineParameter(exec, "-testcontrols");
    AddCmdLineParameter(exec, "-config %s", main_cfg);
    AddCmdLineParameter(exec, "-extraconfig %s", extra_cfg);
    ExecuteDoom(exec);

    TXT_CloseWindow(testwindow);

    // Delete the temporary config files

    remove(main_cfg);
    remove(extra_cfg);
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

