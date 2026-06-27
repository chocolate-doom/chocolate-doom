//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
// DESCRIPTION:
//     Public interface for the Sysop-64 mega converter HTTP tuner.
//

#ifndef SYSOP64_TUNE_HTTP_H
#define SYSOP64_TUNE_HTTP_H

// Configure the tuner listener from command line text such as off, on, port, or
// host:port.
void Sysop_TuneHttpConfigure(const char *spec);
// Return nonzero when the tuner has been enabled by configuration.
int Sysop_TuneHttpEnabled(void);
// Start the tuner listener thread and load the external HTML page.
void Sysop_TuneHttpStart(void);
// Stop the listener thread and release cached page data.
void Sysop_TuneHttpStop(void);

#endif
