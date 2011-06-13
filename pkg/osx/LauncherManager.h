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

#ifndef LAUNCHER_LAUNCHERMANAGER_H
#define LAUNCHER_LAUNCHERMANAGER_H

#include <AppKit/AppKit.h>
#include <AppKit/NSNibLoading.h>
#include "IWADController.h"

@interface LauncherManager : NSObject
{
    IWADController *iwadController;

    id launcherWindow;
    id launchButton;

    id commandLineArguments;
}

- (void) launch: (id)sender;
- (void) runSetup: (id)sender;
- (void) awakeFromNib;
- (void) clearCommandLine;
- (BOOL) addIWADPath: (NSString *) path;
- (void) addFileToCommandLine: (NSString *) fileName
         forArgument: (NSString *) args;
- (void) openTerminal: (id) sender;

- (void) openREADME: (id) sender;
- (void) openINSTALL: (id) sender;
- (void) openCMDLINE: (id) sender;
- (void) openCOPYING: (id) sender;
- (void) openDocumentation: (id) sender;

@end

#endif /* #ifndef LAUNCHER_LAUNCHERMANAGER_H */

