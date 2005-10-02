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
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Dehacked entrypoint and common code
//
//-----------------------------------------------------------------------------

#ifndef DEH_MAIN_H
#define DEH_MAIN_H

#include "doomtype.h"

void DEH_CheckCommandLine(void);

boolean DEH_ParseAssignment(char *line, char **variable_name, char **value);

#endif /* #ifndef DEH_MAIN_H */

