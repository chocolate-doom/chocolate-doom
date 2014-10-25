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

#include <AppKit/AppKit.h>
#include "IWADLocation.h"

static id WAD_TYPES[] =
{
    @"wad", @"WAD"
};

@implementation IWADLocation

- (void) setButtonClicked: (id)sender
{
    NSArray *wadTypes = [NSArray arrayWithObjects: WAD_TYPES count: 2];
    NSOpenPanel *openPanel;
    NSArray *filenames;
    int result;

    [wadTypes retain];

    // Open a file selector for the new file.

    openPanel = [NSOpenPanel openPanel];
    [openPanel setTitle: @"Add IWAD file"];
    [openPanel setCanChooseFiles: YES];
    [openPanel setCanChooseDirectories: NO];

    result = [openPanel runModalForTypes: wadTypes];

    // If the "OK" button was clicked, add the new IWAD file to the list.

    if (result == NSOKButton)
    {
        filenames = [openPanel filenames];
	[self setLocation: [filenames lastObject]];

        [self->iwadController saveConfig];
        [self->iwadController setDropdownList];
    }
}

- (NSString *) getLocation
{
    return [self->locationConfigBox stringValue];
}

- (void) setLocation: (NSString *) filename
{
    [self->locationConfigBox setStringValue: filename];
}

@end

