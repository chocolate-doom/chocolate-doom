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

#include <stdlib.h>
#include <string.h>
#include <AppKit/AppKit.h>
#include "IWADController.h"

typedef enum
{
    IWAD_DOOM1,
    IWAD_DOOM2,
    IWAD_TNT,
    IWAD_PLUTONIA,
    IWAD_CHEX,
    IWAD_HERETIC,
    IWAD_HEXEN,
    IWAD_STRIFE,
    IWAD_FREEDM,
    NUM_IWAD_TYPES
} IWAD;

static NSString *IWADLabels[NUM_IWAD_TYPES] =
{
    @"Doom",
    @"Doom II: Hell on Earth",
    @"Final Doom: TNT: Evilution",
    @"Final Doom: Plutonia Experiment",
    @"Chex Quest",
    @"Heretic",
    @"Hexen",
    @"Strife",
    @"FreeDM",
};

static NSString *IWADFilenames[NUM_IWAD_TYPES + 1] =
{
    @"doom.wad",
    @"doom2.wad",
    @"tnt.wad",
    @"plutonia.wad",
    @"chex.wad",
    @"heretic.wad",
    @"hexen.wad",
    @"strife.wad",
    @"freedm.wad",
    @"undefined"
};

@implementation IWADController

- (void) getIWADList: (NSPathControl **) iwadList
{
    iwadList[IWAD_DOOM1] = self->doom1;
    iwadList[IWAD_DOOM2] = self->doom2;
    iwadList[IWAD_TNT] = self->tnt;
    iwadList[IWAD_PLUTONIA] = self->plutonia;
    iwadList[IWAD_CHEX] = self->chex;
    iwadList[IWAD_HERETIC] = self->heretic;
    iwadList[IWAD_HEXEN] = self->hexen;
    iwadList[IWAD_STRIFE] = self->strife;
    iwadList[IWAD_FREEDM] = self->freedm;
}

- (IWAD) getSelectedIWAD
{
    unsigned int i;

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        if ([self->iwadSelector titleOfSelectedItem] == IWADLabels[i])
        {
            return i;
        }
    }

    return NUM_IWAD_TYPES;
}

// Get the location of the selected IWAD.

- (NSString *) getIWADLocation
{
    IWAD selectedIWAD;
    NSPathControl *iwadList[NUM_IWAD_TYPES];

    selectedIWAD = [self getSelectedIWAD];

    if (selectedIWAD == NUM_IWAD_TYPES)
    {
        return nil;
    }
    else
    {
        [self getIWADList: iwadList];

	return [[iwadList[selectedIWAD] URL] path];
    }
}

static const char *NameForIWAD(IWAD iwad)
{
    switch (iwad)
    {
        case IWAD_HERETIC:
            return "heretic";

        case IWAD_HEXEN:
            return "hexen";

        case IWAD_STRIFE:
            return "strife";

        default:
            return "doom";
    }
}

// Get the name used for the executable for the selected IWAD.

- (const char *) getGameName
{
    return NameForIWAD([self getSelectedIWAD]);
}

- (void) setIWADConfig
{
    NSPathControl *iwadList[NUM_IWAD_TYPES];
    NSUserDefaults *defaults;
    NSString *key;
    NSString *value;
    unsigned int i;

    [self getIWADList: iwadList];

    // Load IWAD filename paths

    defaults = [NSUserDefaults standardUserDefaults];

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        key = IWADFilenames[i];
        value = [defaults stringForKey:key];

        if (value != nil)
        {
            [iwadList[i] setURL: [NSURL fileURLWithPath: value]];
        }
    }
}

// On startup, set the selected item in the IWAD dropdown

- (void) setDropdownSelection
{
    NSUserDefaults *defaults;
    NSString *selected;
    unsigned int i;

    defaults = [NSUserDefaults standardUserDefaults];
    selected = [defaults stringForKey: @"selected_iwad"];

    if (selected == nil)
    {
        return;
    }

    // Find this IWAD in the filenames list, and select it.

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        if ([selected isEqualToString:IWADFilenames[i]])
        {
            [self->iwadSelector selectItemWithTitle:IWADLabels[i]];
            break;
        }
    }
}

// Set the dropdown list to include an entry for each IWAD that has
// been configured.  Returns true if at least one IWAD is configured.

- (BOOL) setDropdownList
{
    NSPathControl *iwadList[NUM_IWAD_TYPES];
    BOOL have_wads;
    id location;
    unsigned int i;
    unsigned int enabled_wads;

    // Build the new list.

    [self getIWADList: iwadList];
    [self->iwadSelector removeAllItems];

    enabled_wads = 0;

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        location = [[iwadList[i] URL] path];

        if (location != nil && [location length] > 0)
        {
            [self->iwadSelector addItemWithTitle: IWADLabels[i]];
            ++enabled_wads;
        }
    }

    // Enable/disable the dropdown depending on whether there
    // were any configured IWADs.

    have_wads = enabled_wads > 0;
    [self->iwadSelector setEnabled: have_wads];

    // Restore the old selection.

    [self setDropdownSelection];

    return have_wads;
}

- (void) saveConfig
{
    NSPathControl *iwadList[NUM_IWAD_TYPES];
    IWAD selectedIWAD;
    NSUserDefaults *defaults;
    NSString *key;
    NSString *value;
    unsigned int i;

    [self getIWADList: iwadList];

    // Store all IWAD locations to user defaults.

    defaults = [NSUserDefaults standardUserDefaults];

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        key = IWADFilenames[i];
        value = [[iwadList[i] URL] path];

        [defaults setObject:value forKey:key];
    }

    // Save currently selected IWAD.

    selectedIWAD = [self getSelectedIWAD];
    [defaults setObject:IWADFilenames[selectedIWAD]
              forKey:@"selected_iwad"];
}

// Callback method invoked when the configuration button in the main
// window is clicked.

- (void) openConfigWindow: (id)sender
{
    if (![self->configWindow isVisible])
    {
        [self->configWindow makeKeyAndOrderFront: sender];
    }
}

// Callback method invoked when the close button is clicked.

- (void) closeConfigWindow: (id)sender
{
    [self->configWindow orderOut: sender];
    [self saveConfig];
    [self setDropdownList];
}

- (void) awakeFromNib
{
    [self->configWindow center];

    // Set configuration for all IWADs from configuration file.

    [self setIWADConfig];

    // Populate the dropdown IWAD list.

    if ([self setDropdownList])
    {
        [self setDropdownSelection];
    }
}

// Generate a value to set for the DOOMWADPATH environment variable
// that contains each of the configured IWAD files.

- (char *) doomWadPath
{
    NSPathControl *iwadList[NUM_IWAD_TYPES];
    NSString *location;
    unsigned int i;
    BOOL first;
    char *env;
    size_t env_len;

    [self getIWADList: iwadList];

    // Calculate length of environment string.

    env_len = 0;

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        location = [[iwadList[i] URL] path];

        if (location != nil && [location length] > 0)
        {
            env_len += [location length] + 1;
        }
    }

    // Build string.

    env = malloc(env_len);
    strlcpy(env, "", env_len);

    first = YES;

    for (i=0; i<NUM_IWAD_TYPES; ++i)
    {
        location = [[iwadList[i] URL] path];

        if (location != nil && [location length] > 0)
        {
            if (!first)
            {
                strlcat(env, ":", env_len);
            }

            strlcat(env, [location UTF8String], env_len);
            first = NO;
        }
    }

    return env;
}

// Set the DOOMWADPATH environment variable to contain the path to each
// of the configured IWAD files.

- (void) setEnvironment
{
    char *doomwadpath;
    char *env;

    // Get the value for the path.

    doomwadpath = [self doomWadPath];

    asprintf(&env, "DOOMWADPATH=%s", doomwadpath);

    free(doomwadpath);

    // Load into environment:

    putenv(env);

    //free(env);
}

// Examine a path to a WAD and determine whether it is an IWAD file.
// If so, it is added to the IWAD configuration, and true is returned.

- (BOOL) addIWADPath: (NSString *) path
{
    NSPathControl *iwadList[NUM_IWAD_TYPES];
    NSArray *pathComponents;
    NSString *filename;
    unsigned int i;

    [self getIWADList: iwadList];

    // Find an IWAD file that matches the filename in the path that we
    // have been given.

    pathComponents = [path pathComponents];
    filename = [pathComponents objectAtIndex: [pathComponents count] - 1];

    for (i = 0; i < NUM_IWAD_TYPES; ++i)
    {
        if ([filename caseInsensitiveCompare: IWADFilenames[i]] == 0)
        {
            // Configure this IWAD.

            [iwadList[i] setURL: [NSURL fileURLWithPath: path]];

            // Rebuild dropdown list and select the new IWAD.

            [self setDropdownList];
            [self->iwadSelector selectItemWithTitle:IWADLabels[i]];
            return YES;
        }
    }

    // No IWAD found with this name.

    return NO;
}

- (BOOL) selectGameByName: (const char *) name
{
    NSPathControl *iwadList[NUM_IWAD_TYPES];
    NSString *location;
    const char *name2;
    int i;

    // Already selected an IWAD of the desired type? Just return
    // success.
    if (!strcmp(name, [self getGameName]))
    {
        return YES;
    }

    // Search through the configured IWADs and try to select the
    // desired game.
    [self getIWADList: iwadList];

    for (i = 0; i < NUM_IWAD_TYPES; ++i)
    {
        location = [[iwadList[i] URL] path];
        name2 = NameForIWAD(i);

        if (!strcmp(name, name2)
         && location != nil && [location length] > 0)
        {
            [self->iwadSelector selectItemWithTitle:IWADLabels[i]];
            return YES;
        }
    }

    // User hasn't configured any WAD(s) for the desired game type.
    return NO;
}

@end

