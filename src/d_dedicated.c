// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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
// Code specific to the standalone dedicated server.
//

#include <stdlib.h>
#include <stdarg.h>

#include "config.h"

#include "m_argv.h"
#include "net_defs.h"

#include "net_dedicated.h"
#include "net_server.h"
#include "z_zone.h"

void NET_CL_Run(void)
{
    // No client present :-)
    //
    // This is here because the server code sometimes runs this 
    // to let the client do some processing if it needs to.
    // In a standalone dedicated server, we don't have a client.
}

//
// I_Error
//
// We have our own I_Error function for the dedicated server.  
// The normal one does extra things like shutdown graphics, etc.

void I_Error (char *error, ...)
{
    va_list	argptr;

    // Message first.
    va_start(argptr,error);
    fprintf(stderr, "Error: ");
    vfprintf(stderr,error,argptr);
    fprintf(stderr, "\n");
    va_end(argptr);

    fflush(stderr);

    exit(-1);
}


void D_DoomMain(void)
{
    printf(PACKAGE_NAME " standalone dedicated server\n");

    Z_Init();

    NET_DedicatedServer();
}

