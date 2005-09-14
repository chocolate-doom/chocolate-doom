// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_main.c 106 2005-09-14 22:08:29Z fraggle $
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
// $Log$
// Revision 1.6  2005/09/14 22:08:28  fraggle
// Fix startup messages displayed (build a console binary; remove CON
// redirection code)
//
// Revision 1.5  2005/09/07 22:58:34  fraggle
// No SIGHUP on Windows
//
// Revision 1.4  2005/09/07 21:40:28  fraggle
// Catch signals and exit cleanly
//
// Revision 1.3  2005/08/30 22:11:10  fraggle
// Windows fixes
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:32  fraggle
// Initial import
//
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_main.c 106 2005-09-14 22:08:29Z fraggle $";


#include <signal.h>

#include "doomdef.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_main.h"

void SignalHandler(int signum)
{
    I_Error("Aborting due to signal %i\n", signum);   
}

int main(int argc, char **argv) 
{ 

    // save arguments

    myargc = argc; 
    myargv = argv; 

    signal(SIGSEGV, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGILL, SignalHandler);
    signal(SIGINT, SignalHandler);
    signal(SIGFPE, SignalHandler);
    signal(SIGABRT, SignalHandler);
#ifdef SIGHUP
    signal(SIGHUP, SignalHandler);
#endif
#ifdef SIGPIPE
    signal(SIGHUP, SignalHandler);
#endif

    // start doom
 
    D_DoomMain (); 

    return 0;
} 
