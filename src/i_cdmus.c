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
//
// Hexen CD interface.
//

#include <stdio.h>

#include "SDL.h"

#include "doomtype.h"

#include "i_cdmus.h"

int cd_Error;

int I_CDMusInit(void)
{
    fprintf(stderr,
        "I_CDMusInit: CD music playback is no longer supported! "
        "Please use digital music packs instead:\n"
        "https://www.chocolate-doom.org/wiki/index.php/Digital_music_packs\n");
    return -1;
}

// We cannot print status messages inline during startup, they must
// be deferred until after I_CDMusInit has returned.

void I_CDMusPrintStartup(void)
{
}

int I_CDMusPlay(int track)
{
    return 0;
}

int I_CDMusStop(void)
{
    return 0;
}

int I_CDMusResume(void)
{
    return 0;
}

int I_CDMusSetVolume(int volume)
{
    return 0;
}

int I_CDMusFirstTrack(void)
{
    return 0;
}

int I_CDMusLastTrack(void)
{
    return 0;
}

int I_CDMusTrackLength(int track_num)
{
    return 0;
}

