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
//	Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------


#ifndef __M_SWAP__
#define __M_SWAP__


#include "config.h"


// Endianess handling.
// WAD files are stored little endian.
#ifdef WORDS_BIGENDIAN
extern unsigned short	SwapSHORT(unsigned short);
extern unsigned int	SwapLONG(unsigned int);
#define SHORT(x)	((short)SwapSHORT((unsigned short) (x)))
#define LONG(x)         ((int)SwapLONG((unsigned int) (x)))
#else
#define SHORT(x)	(x)
#define LONG(x)         (x)
#endif




#endif
