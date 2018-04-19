//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
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
// DESCRIPTION:
//	[crispy] support maps with NODES in compressed or uncompressed ZDBSP
// 	format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
//


#ifndef __P_EXTNODES__
#define __P_EXTNODES__

typedef enum
{
    MFMT_DOOMBSP = 0x000,
    MFMT_DEEPBSP = 0x001,
    MFMT_ZDBSPX  = 0x002,
    MFMT_ZDBSPZ  = 0x004,
    MFMT_HEXEN   = 0x100,
} mapformat_t;

extern mapformat_t P_CheckMapFormat (int lumpnum);

extern void P_LoadSegs_DeePBSP (int lump);
extern void P_LoadSubsectors_DeePBSP (int lump);
extern void P_LoadNodes_DeePBSP (int lump);
extern void P_LoadNodes_ZDBSP (int lump, boolean compressed);
extern void P_LoadThings_Hexen (int lump);
extern void P_LoadLineDefs_Hexen (int lump);

#endif
