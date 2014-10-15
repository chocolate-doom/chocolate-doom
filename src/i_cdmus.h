//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

// i_cdmus.h

#ifndef __ICDMUS__
#define __ICDMUS__

#define CDERR_NOTINSTALLED   10 // MSCDEX not installed
#define CDERR_NOAUDIOSUPPORT 11 // CD-ROM Doesn't support audio
#define CDERR_NOAUDIOTRACKS  12 // Current CD has no audio tracks
#define CDERR_BADDRIVE       20 // Bad drive number
#define CDERR_BADTRACK       21 // Bad track number
#define CDERR_IOCTLBUFFMEM   22 // Not enough low memory for IOCTL
#define CDERR_DEVREQBASE     100        // DevReq errors

extern int cd_Error;

int I_CDMusInit(void);
void I_CDMusPrintStartup(void);
int I_CDMusPlay(int track);
int I_CDMusStop(void);
int I_CDMusResume(void);
int I_CDMusSetVolume(int volume);
int I_CDMusFirstTrack(void);
int I_CDMusLastTrack(void);
int I_CDMusTrackLength(int track);

#endif
