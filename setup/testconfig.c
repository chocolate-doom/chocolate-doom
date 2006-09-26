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

// Code for invoking Doom to test the current configuration.

#include <stdlib.h>

#include "textscreen.h"

static void TestCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
    txt_window_t *testwindow;
    txt_label_t *label;
    
    testwindow = TXT_NewWindow("Starting Doom");

    label = TXT_NewLabel("Starting Doom to test the\n"
                         "settings.  Please wait.");
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_AddWidget(testwindow, label);
    TXT_DrawDesktop();
    
#ifdef _WIN32
    // On windows, just run the command - we aren't installed anywhere
    // special.
    system("chocolate-doom -testcontrols");
#else
    // Use the location where we are installed
    system(INSTALL_DIR "/chocolate-doom -testcontrols");
#endif

    TXT_CloseWindow(testwindow);
}

txt_window_action_t *TestConfigAction(void)
{
    txt_window_action_t *test_action;
    
    test_action = TXT_NewWindowAction('t', "Test");
    TXT_SignalConnect(test_action, "pressed", TestCallback, NULL);

    return test_action;
}

