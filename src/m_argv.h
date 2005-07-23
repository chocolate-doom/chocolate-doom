// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_argv.h 4 2005-07-23 16:19:41Z fraggle $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//  Nil.
//    
//-----------------------------------------------------------------------------


#ifndef __M_ARGV__
#define __M_ARGV__

//
// MISC
//
extern  int	myargc;
extern  char**	myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm (char* check);


#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2005/07/23 16:20:49  fraggle
// Initial revision
//
//
//-----------------------------------------------------------------------------
