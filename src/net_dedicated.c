// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: $
//
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
//-----------------------------------------------------------------------------
//
// Dedicated server code.
// 

#include "doomtype.h"

#include "i_system.h"

#include "net_defs.h"
#include "net_sdl.h"
#include "net_server.h"

void NET_DedicatedServer(void)
{
    NET_SV_Init();

    NET_SV_AddModule(&net_sdl_module);

    while (true)
    {
        NET_SV_Run();
        I_Sleep(10);
    }
}

