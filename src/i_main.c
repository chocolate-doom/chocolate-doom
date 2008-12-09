// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------


#include "config.h"

#include "SDL.h"

#include <signal.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef HAVE_SCHED_SETAFFINITY
#include <unistd.h>
#include <sched.h>
#endif

#include "doomdef.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_main.h"

#if !defined(_WIN32) && !defined(HAVE_SCHED_SETAFFINITY)
#warning No known way to set processor affinity on this platform.
#warning You may experience crashes due to SDL_mixer.
#endif

int main(int argc, char **argv)
{
    // save arguments

    myargc = argc;
    myargv = argv;

#ifdef _WIN32

    // Set the process affinity mask so that all threads
    // run on the same processor.  This is a workaround for a bug in
    // SDL_mixer that causes occasional crashes.

    if (!SetProcessAffinityMask(GetCurrentProcess(), 1))
    {
        fprintf(stderr, "Failed to set process affinity mask (%d)\n",
                (int) GetLastError());
    }

#endif

#ifdef HAVE_SCHED_SETAFFINITY

    // Linux version:

    {
        cpu_set_t set;

        CPU_ZERO(&set);
        CPU_SET(0, &set);

        sched_setaffinity(getpid(), sizeof(set), &set);
    }

#endif

    // start doom

    D_DoomMain ();

    return 0;
}

