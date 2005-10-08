// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_io.h 175 2005-10-08 20:54:16Z fraggle $
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
// Revision 1.2  2005/10/08 20:54:16  fraggle
// Proper dehacked error/warning framework.  Catch a load more errors.
//
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Dehacked I/O code (does all reads from dehacked files)
//
//-----------------------------------------------------------------------------

#ifndef DEH_IO_H
#define DEH_IO_H

#include "deh_defs.h"

deh_context_t *DEH_OpenFile(char *filename);
void DEH_CloseFile(deh_context_t *context);
int DEH_GetChar(deh_context_t *context);
char *DEH_ReadLine(deh_context_t *context);
void DEH_Error(deh_context_t *context, char *msg, ...);
void DEH_Warning(deh_context_t *context, char *msg, ...);

#endif /* #ifndef DEH_IO_H */

