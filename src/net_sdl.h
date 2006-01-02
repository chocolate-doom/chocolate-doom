// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: net_sdl.h 244 2006-01-02 20:11:49Z fraggle $
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
// $Log$
// Revision 1.2  2006/01/02 20:11:49  fraggle
// Rename i_net_module -> net_sdl_module.  Fix the AddrToString method.
//
// Revision 1.1  2005/10/30 19:56:15  fraggle
// Add foundation code for the new networking system
//
//
// DESCRIPTION:
//     Networking module which uses SDL_net
//
//-----------------------------------------------------------------------------

#ifndef NET_SDL_H
#define NET_SDL_H

#include "net_defs.h"

extern net_module_t net_sdl_module;

#endif /* #ifndef NET_SDL_H */

