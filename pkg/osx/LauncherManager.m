/* ... */
//-----------------------------------------------------------------------------
//
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
//-----------------------------------------------------------------------------

#include <AppKit/AppKit.h>
#include "Execute.h"
#include "LauncherManager.h"
#include "config.h"

@implementation LauncherManager

// Save configuration.  Invoked when we launch the game or quit.

- (void) saveConfig
{
    NSUserDefaults *defaults;

    // Save IWAD configuration and selected IWAD.

    [self->iwadController saveConfig];

    // Save command line arguments.

    defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:[self->commandLineArguments stringValue]
              forKey:@"command_line_args"];
}

// Load configuration, invoked on startup.

- (void) setConfig
{
    NSUserDefaults *defaults;
    NSString *args;

    defaults = [NSUserDefaults standardUserDefaults];

    args = [defaults stringForKey:@"command_line_args"];

    if (args != nil)
    {
        [self->commandLineArguments setStringValue:args];
    }
}

- (void) launch: (id)sender
{
    NSString *iwad;
    NSString *args;

    [self saveConfig];

    iwad = [self->iwadController getIWADLocation];
    args = [self->commandLineArguments stringValue];

    if (iwad != nil)
    {
        ExecuteProgram(PACKAGE_TARNAME, [iwad UTF8String],
                                        [args UTF8String]);
        [NSApp terminate:sender];
    }
}

- (void) runSetup: (id)sender
{
    [self saveConfig];

    [self->iwadController setEnvironment];
    ExecuteProgram("chocolate-setup", NULL, NULL);
}

- (void) awakeFromNib
{
    [self->packageLabel setStringValue: @PACKAGE_STRING];
    [self->launcherWindow setTitle: @PACKAGE_NAME " Launcher"];
    [self setConfig];
}

@end

