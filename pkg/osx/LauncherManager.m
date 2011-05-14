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

// Get the next command line argument from the command line.
// The position counter used to iterate over arguments is in 'pos'.
// The index of the argument that was found is saved in arg_pos.

static NSString *GetNextArgument(NSString *commandLine, int *pos, int *arg_pos)
{
    NSRange arg_range;

    // Skip past any whitespace

    while (*pos < [commandLine length]
        && isspace([commandLine characterAtIndex: *pos]))
    {
        ++*pos;
    }

    if (*pos >= [commandLine length])
    {
        *arg_pos = *pos;
        return nil;
    }

    // We are at the start of the argument.  This may be a quoted
    // string argument, or a "normal" one.

    if ([commandLine characterAtIndex: *pos] == '\"')
    {
        // Quoted string, skip past first quote

        ++*pos;

        // Save start position:

        *arg_pos = *pos;

        while (*pos < [commandLine length]
            && [commandLine characterAtIndex: *pos] != '\"')
        {
            ++*pos;
        }

        // Unexpected end of string?

        if (*pos >= [commandLine length])
        {
            return nil;
        }

        arg_range = NSMakeRange(*arg_pos, *pos - *arg_pos);

        // Skip past last quote

        ++*pos;
    }
    else
    {
        // Normal argument

        // Save position:

        *arg_pos = *pos;

        // Read until end:

        while (*pos < [commandLine length]
            && !isspace([commandLine characterAtIndex: *pos]))
        {
            ++*pos;
        }

        arg_range = NSMakeRange(*arg_pos, *pos - *arg_pos);
    }

    return [commandLine substringWithRange: arg_range];
}

// Given the specified command line argument, find the index
// to insert the new file within the command line.  Returns -1 if the 
// argument is not already within the arguments string.

static int GetFileInsertIndex(NSString *commandLine, NSString *needle)
{
    NSString *arg;
    int arg_pos;
    int pos;

    pos = 0;

    // Find the command line parameter we are searching
    // for (-merge, -deh, etc)

    for (;;)
    {
        arg = GetNextArgument(commandLine, &pos, &arg_pos);

        // Searched to end of string and never found?

        if (arg == nil)
        {
            return -1;
        }

        if (![arg caseInsensitiveCompare: needle])
        {
            break;
        }
    }

    // Now skip over existing files.  For example, if we
    // have -file foo.wad bar.wad, the new file should be appended
    // to the end of the list.

    for (;;)
    {
        arg = GetNextArgument(commandLine, &pos, &arg_pos);

        // If we search to the end of the string now, it is fine;
        // the new string should be added to the end of the command
        // line.  Otherwise, if we find an argument that begins
        // with '-', it is a new command line parameter and the end
        // of the list.

        if (arg == nil || [arg characterAtIndex: 0] == '-')
        {
            break;
        }
    }

    // arg_pos should now contain the offset to insert the new filename.

    return arg_pos;
}

// Given the specified string, append a filename, quoted if necessary.

static NSString *AppendQuotedFilename(NSString *str, NSString *fileName)
{
    int i;

    // Search the filename for spaces, and quote if necessary.

    for (i=0; i<[fileName length]; ++i)
    {
        if (isspace([fileName characterAtIndex: i]))
        {
            str = [str stringByAppendingString: @" \""];
            str = [str stringByAppendingString: fileName];
            str = [str stringByAppendingString: @"\" "];

            return str;
        }
    }

    str = [str stringByAppendingString: @" "];
    str = [str stringByAppendingString: fileName];

    return str;
}

// Clear out the existing command line options.
// Invoked before the first file is added.

- (void) clearCommandLine
{
    [self->commandLineArguments setStringValue: @""];
}

// Add a file to the command line to load with the game.

- (void) addFileToCommandLine: (NSString *) fileName
         forArgument: (NSString *) arg
{
    NSString *commandLine;
    int insert_pos;

    // Get the current command line

    commandLine = [self->commandLineArguments stringValue];

    // Find the location to insert the new filename:

    insert_pos = GetFileInsertIndex(commandLine, arg);

    // If position < 0, we should add the new argument and filename
    // to the end.  Otherwise, append the new filename to the existing
    // list of files.

    if (insert_pos < 0)
    {
        commandLine = [commandLine stringByAppendingString: @" "];
        commandLine = [commandLine stringByAppendingString: arg];
        commandLine = AppendQuotedFilename(commandLine, fileName);
    }
    else
    {
        NSString *start;
        NSString *end;

        // Divide existing command line in half:

        start = [commandLine substringToIndex: insert_pos];
        end = [commandLine substringFromIndex: insert_pos];

        // Construct new command line:

        commandLine = AppendQuotedFilename(start, fileName);
        commandLine = [commandLine stringByAppendingString: @" "];
        commandLine = [commandLine stringByAppendingString: end];
    }

    [self->commandLineArguments setStringValue: commandLine];
}

- (void) launch: (id)sender
{
    NSString *iwad;
    NSString *args;

    [self saveConfig];

    iwad = [self->iwadController getIWADLocation];
    args = [self->commandLineArguments stringValue];

    if (iwad == nil)
    {
        NSRunAlertPanel(@"No IWAD selected",
                        @"You have not selected an IWAD (game) file.\n\n"
                         "You must configure and select a valid IWAD file "
                         "in order to launch the game.",
                        @"OK", nil, nil);
        return;
    }

    ExecuteProgram(PROGRAM_PREFIX "doom", [iwad UTF8String],
                                          [args UTF8String]);
    [NSApp terminate:sender];
}

// Invoked when the "Setup Tool" button is clicked, to run the setup tool:

- (void) runSetup: (id)sender
{
    [self saveConfig];

    [self->iwadController setEnvironment];
    ExecuteProgram(PROGRAM_PREFIX "setup", NULL, NULL);
}

// Invoked when the "Terminal" option is selected from the menu, to open
// a terminal window.

- (void) openTerminal: (id) sender
{
    char *doomwadpath;

    [self saveConfig];

    doomwadpath = [self->iwadController doomWadPath];

    OpenTerminalWindow(doomwadpath);

    free(doomwadpath);
}

- (void) openREADME: (id) sender
{
    OpenDocumentation("README");
}

- (void) openINSTALL: (id) sender
{
    OpenDocumentation("INSTALL");
}

- (void) openCMDLINE: (id) sender
{
    OpenDocumentation("CMDLINE");
}

- (void) openCOPYING: (id) sender
{
    OpenDocumentation("COPYING");
}

- (void) openDocumentation: (id) sender
{
    OpenDocumentation("");
}

- (void) awakeFromNib
{
    [self->launcherWindow setTitle: @PACKAGE_NAME " Launcher"];
    [self->launcherWindow center];
    [self->launcherWindow setDefaultButtonCell: [self->launchButton cell]];
    [self setConfig];
}

- (BOOL) addIWADPath: (NSString *) path
{
    return [self->iwadController addIWADPath: path];
}

@end

