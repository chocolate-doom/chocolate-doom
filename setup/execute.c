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

#include "textscreen.h"

#include "execute.h"
#include "m_argv.h"

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

void ExecuteDoom(execute_context_t *context)
{
    char *cmdline;
    
    fclose(context->stream);

    // Build the command line

    cmdline = malloc(strlen(INSTALL_DIR) 
                     + strlen(context->response_file) + 20);

#ifdef _WIN32
    sprintf(cmdline, "chocolate-doom @%s", context->response_file);
#else
    sprintf(cmdline, INSTALL_DIR "/chocolate-doom @%s", context->response_file);
#endif
    
    // Run the command
    system(cmdline);

    free(cmdline);
    
    // Destroy context 
    remove(context->response_file);
    free(context);
}

static void TestCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    execute_context_t *exec;
    txt_window_t *testwindow;
    txt_label_t *label;
    
    testwindow = TXT_NewWindow("Starting Doom");

    label = TXT_NewLabel("Starting Doom to test the\n"
                         "settings.  Please wait.");
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_AddWidget(testwindow, label);
    TXT_DrawDesktop();
    
    // Run with the -testcontrols parameter

    exec = NewExecuteContext();
    AddCmdLineParameter(exec, "-testcontrols");
    ExecuteDoom(exec);

    TXT_CloseWindow(testwindow);
}

txt_window_action_t *TestConfigAction(void)
{
    txt_window_action_t *test_action;
    
    test_action = TXT_NewWindowAction('t', "Test");
    TXT_SignalConnect(test_action, "pressed", TestCallback, NULL);

    return test_action;
}

