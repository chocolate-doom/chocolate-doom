//
// Copyright(C) 2005-2014 Simon Howard
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

#include "AppController.h"

#include "config.h"

@implementation AppController

+ (void)initialize
{
    NSMutableDictionary *defaults = [NSMutableDictionary dictionary];

    /*
     * Register your app's defaults here by adding objects to the
     * dictionary, eg
     *
     * [defaults setObject:anObject forKey:keyForThatObject];
     *
     */

    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (id)init
{
    if ((self = [super init]))
    {
    }

    self->filesAdded = NO;

    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)awakeFromNib
{
    [[NSApp mainMenu] setTitle:@PACKAGE_NAME];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotif
{
// Uncomment if your application is Renaissance-based
//  [NSBundle loadGSMarkupNamed:@"Main" owner:self];
}

- (BOOL)applicationShouldTerminate:(id)sender
{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotif
{
}

- (BOOL) application:(NSApplication *) application
         openFile:(NSString *) fileName
{
    NSString *extension;

    // This may be an IWAD.  If so, add it to the IWAD configuration;
    // don't add it like a PWAD.

    if ([self->launcherManager addIWADPath: fileName])
    {
        return YES;
    }

    // If this is the first file added, clear out the existing
    // command line.  This allows us to select multiple files
    // in the finder and open them all together (for TCs, etc).

    if (!self->filesAdded)
    {
        [self->launcherManager clearCommandLine];
    }

    // Add file with appropriate command line option based on extension:

    extension = [fileName pathExtension];

    if (![extension caseInsensitiveCompare: @"wad"])
    {
        [self->launcherManager addFileToCommandLine: fileName
                               forArgument: @"-merge"];
    }
    else if (![extension caseInsensitiveCompare: @"lmp"])
    {
        [self->launcherManager addFileToCommandLine: fileName
                               forArgument: @"-playdemo"];
    }
    else if (![extension caseInsensitiveCompare: @"deh"])
    {
        [self->launcherManager addFileToCommandLine: fileName
                               forArgument: @"-deh"];
        [self->launcherManager selectGameByName: "doom"];
    }
    else if (![extension caseInsensitiveCompare: @"hhe"])
    {
        [self->launcherManager addFileToCommandLine: fileName
                               forArgument: @"-deh"];
        [self->launcherManager selectGameByName: "heretic"];
    }
    else if (![extension caseInsensitiveCompare: @"seh"])
    {
        [self->launcherManager addFileToCommandLine: fileName
                               forArgument: @"-deh"];
        [self->launcherManager selectGameByName: "strife"];
    }
    else
    {
        return NO;
    }

    self->filesAdded = YES;

    return YES;
}

- (void)showPrefPanel:(id)sender
{
}

@end

