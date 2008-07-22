// Emacs style mode select   -*- C++ -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    StealthCP              (?)
// -----------------------------------------------------------------------------
// Copyright (C) 2008 ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Correct READ, WRITE memory handling

#include "byteptr.h"

#define BP_MERGE(a,b) a##b
#define BP_READ(x) x BP_MERGE(Read,x)(x **Ptr)\
{\
	x Ret;\
	\
	if (!Ptr || !(*Ptr))\
		return 0;\
	\
	Ret = **Ptr;\
	(*Ptr)++;\
	return Ret;\
}

BP_READ(Int8)
BP_READ(Int16)
BP_READ(Int32)
BP_READ(Int64)
BP_READ(UInt8)
BP_READ(UInt16)
BP_READ(UInt32)
BP_READ(UInt64)
#define BP_WRITE(x) void BP_MERGE(Write,x)(x **Ptr, x Val)\
{\
	if (!Ptr || !(*Ptr))\
		return;\
	**Ptr = Val;\
	(*Ptr)++;\
}
BP_WRITE(Int8)
BP_WRITE(Int16)
BP_WRITE(Int32)
BP_WRITE(Int64)
BP_WRITE(UInt8)
BP_WRITE(UInt16)
BP_WRITE(UInt32)
BP_WRITE(UInt64)

void WriteString(char **Ptr, char *Val)
{
	if (!Ptr || !(*Ptr) || !Val)
		return;

	while (*Val)
	{
		**Ptr = *Val;
		(*Ptr)++;
		Val++;
	}
}

void WriteStringN(char **Ptr, char *Val, size_t n)
{
	size_t i = 0;

	if (!Ptr || !(*Ptr) || !Val || n == 0)
		return;

	while (*Val && i < n)
	{
		**Ptr = *Val;
		(*Ptr)++;
		Val++;
		i++;
	}

	while (i < n)				// Pad
	{
		**Ptr = 0;
		(*Ptr)++;
		i++;
	}
}
