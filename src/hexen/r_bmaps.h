//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2017 Brad Harding
// Copyright(C) 2017-2022 Fabian Greffrath
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
//	Brightmaps for walls, floors, sprites and weapon states
//

#ifndef __R_BMAPS__
#define __R_BMAPS__

extern void R_InitBrightmaps ();

extern const byte *R_BrightmapForTexName (const char *texname);
extern const byte *R_BrightmapForSprite (const int state);
extern const byte *R_BrightmapForState (const int state);

extern const byte **texturebrightmap;

#endif
