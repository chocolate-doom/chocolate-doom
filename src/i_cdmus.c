// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------
//
// SDL implementation of the Hexen CD interface.
//
//-----------------------------------------------------------------------------

#include "SDL.h"
#include "SDL_cdrom.h"

#include "doomtype.h"

#include "i_cdmus.h"

static SDL_CD *cd_handle = NULL;

int cd_Error;

int I_CDMusInit(void)
{
    int drive_num = 0;

    // TODO: config variable to control CDROM to use.
   
    cd_handle = SDL_CDOpen(drive_num);

    if (cd_handle == NULL)
    {
        fprintf(stderr, "I_CDMusInit: Failed to open CD-ROM drive.\n");
        cd_Error = 1;
        return -1;
    }

    if (!CD_INDRIVE(cd_handle->status))
    {
        fprintf(stderr, "I_CDMusInit: '%s': no CD in drive.\n",
                        SDL_CDName(drive_num));
        cd_Error = 1;
        return -1;
    }

    cd_Error = 0;

    return 0;
}

int I_CDMusPlay(int track)
{
    int result;

    if (cd_handle == NULL)
    {
        cd_Error = 1;
        return -1;
    }

    // Play one track
    // Track is indexed from 1.

    result = SDL_CDPlayTracks(cd_handle, track - 1, 0, 1, 0);

    cd_Error = 0;
    return result;
}

int I_CDMusStop(void)
{
    int result;

    result = SDL_CDStop(cd_handle);

    cd_Error = 0;

    return result;
}

int I_CDMusResume(void)
{
    int result;

    result = SDL_CDResume(cd_handle);

    cd_Error = 0;

    return result;
}

int I_CDMusSetVolume(int volume)
{
    /* Not supported yet */

    cd_Error = 0;

    return 0;
}

int I_CDMusFirstTrack(void)
{
    int i;

    if (cd_handle == NULL)
    {
        cd_Error = 1;
        return -1;
    }

    // Find the first audio track.

    for (i=0; i<cd_handle->numtracks; ++i) 
    {
        if (cd_handle->track[i].type == SDL_AUDIO_TRACK)
        {
            cd_Error = 0;

            // Tracks are indexed from 1.
            return i + 1;
        }
    }

    /* Don't know? */

    cd_Error = 1;

    return -1;
}

int I_CDMusLastTrack(void)
{
    if (cd_handle == NULL)
    {
        cd_Error = 1;
        return -1;
    }

    cd_Error = 0;

    return cd_handle->numtracks;
}

int I_CDMusTrackLength(int track_num)
{
    SDL_CDtrack *track;

    if (cd_handle == NULL || track_num < 1 || track_num > cd_handle->numtracks)
    {
        cd_Error = 1;
        return -1;
    }

    // Track number is indexed from 1.

    track = &cd_handle->track[track_num - 1];

    // Round up to the next second

    cd_Error = 0;

    return (track->length + CD_FPS - 1) / CD_FPS;
}

