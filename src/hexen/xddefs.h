//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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


#ifndef __XDDEFS__
#define __XDDEFS__

#include "doomtype.h"
#include "v_patch.h"

//--------------------------------------------------------------------------
//
// Map level types
//
//--------------------------------------------------------------------------

// lump order in a map wad
enum
{
    ML_LABEL,
    ML_THINGS,
    ML_LINEDEFS,
    ML_SIDEDEFS,
    ML_VERTEXES,
    ML_SEGS,
    ML_SSECTORS,
    ML_NODES,
    ML_SECTORS,
    ML_REJECT,
    ML_BLOCKMAP,
    ML_BEHAVIOR
};

typedef PACKED_STRUCT (
{
    short x;
    short y;
})  mapvertex_t;

typedef PACKED_STRUCT (
{
    short textureoffset;
    short rowoffset;
    char toptexture[8];
    char bottomtexture[8];
    char midtexture[8];
    short sector;               // on viewer's side
}) mapsidedef_t;

typedef PACKED_STRUCT (
{
    short v1;
    short v2;
    short flags;
    byte special;
    byte arg1;
    byte arg2;
    byte arg3;
    byte arg4;
    byte arg5;
    short sidenum[2];           // sidenum[1] will be -1 if one sided
}) maplinedef_t;

#define	ML_BLOCKING			0x0001
#define	ML_BLOCKMONSTERS	0x0002
#define	ML_TWOSIDED			0x0004
#define	ML_DONTPEGTOP		0x0008
#define	ML_DONTPEGBOTTOM	0x0010
#define ML_SECRET			0x0020  // don't map as two sided: IT'S A SECRET!
#define ML_SOUNDBLOCK		0x0040  // don't let sound cross two of these
#define	ML_DONTDRAW			0x0080  // don't draw on the automap
#define	ML_MAPPED			0x0100  // set if already drawn in automap
#define ML_REPEAT_SPECIAL	0x0200  // special is repeatable
#define ML_SPAC_SHIFT		10
#define ML_SPAC_MASK		0x1c00
#define GET_SPAC(flags) ((flags&ML_SPAC_MASK)>>ML_SPAC_SHIFT)

// Special activation types
#define SPAC_CROSS		0       // when player crosses line
#define SPAC_USE		1       // when player uses line
#define SPAC_MCROSS		2       // when monster crosses line
#define SPAC_IMPACT		3       // when projectile hits line
#define SPAC_PUSH		4       // when player/monster pushes line
#define SPAC_PCROSS		5       // when projectile crosses line

typedef PACKED_STRUCT (
{
    short floorheight;
    short ceilingheight;
    char floorpic[8];
    char ceilingpic[8];
    short lightlevel;
    short special;
    short tag;
}) mapsector_t;

typedef PACKED_STRUCT (
{
    short numsegs;
    short firstseg;             // segs are stored sequentially
}) mapsubsector_t;

typedef PACKED_STRUCT (
{
    short v1;
    short v2;
    short angle;
    short linedef;
    short side;
    short offset;
}) mapseg_t;

#define	NF_SUBSECTOR	0x8000
typedef PACKED_STRUCT (
{
    short x, y, dx, dy;         // partition line
    short bbox[2][4];           // bounding box for each child
    unsigned short children[2]; // if NF_SUBSECTOR its a subsector
}) mapnode_t;

typedef PACKED_STRUCT (
{
    short tid;
    short x;
    short y;
    short height;
    short angle;
    short type;
    short options;
    byte special;
    byte arg1;
    byte arg2;
    byte arg3;
    byte arg4;
    byte arg5;
}) mapthing_t;

#define MTF_EASY		1
#define MTF_NORMAL		2
#define MTF_HARD		4
#define MTF_AMBUSH		8
#define MTF_DORMANT		16
#define MTF_FIGHTER		32
#define MTF_CLERIC		64
#define MTF_MAGE		128
#define MTF_GSINGLE		256
#define MTF_GCOOP		512
#define MTF_GDEATHMATCH	1024

//--------------------------------------------------------------------------
//
// Texture definition
//
//--------------------------------------------------------------------------

typedef PACKED_STRUCT (
{
    short originx;
    short originy;
    short patch;
    short stepdir;
    short colormap;
}) mappatch_t;

typedef PACKED_STRUCT (
{
    char name[8];
    boolean masked;
    short width;
    short height;
    int obsolete;
    short patchcount;
    mappatch_t patches[1];
}) maptexture_t;

#endif // __XDDEFS__
