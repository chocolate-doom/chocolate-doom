// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
// Revision 1.1  2005/12/29 17:48:25  fraggle
// Add initial client/server connect code.  Reorganise sources list in
// Makefile.am.
//
//
// Network server code
//

#ifndef NET_SERVER_H
#define NET_SERVER_H

// initialise server and wait for connections

void NET_ServerInit(void);

// run server: check for new packets received etc.

void NET_ServerRun(void);

#endif /* #ifndef NET_SERVER_H */

