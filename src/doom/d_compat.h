//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2016 Alexey Khokholov
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
//  Older Doom EXEs emulation.
//

#ifndef __D_COMPAT__
#define __D_COMPAT__

#include "doomdef.h"
#include "doomstat.h"

#include "info.h"

void D_Compat_Init(void);

spritenum_t D_Compat_SpriteToOld(spritenum_t sprite);
spritenum_t D_Compat_SpriteToNew(spritenum_t sprite);
boolean D_Compat_SpriteExists(spritenum_t sprite);

statenum_t D_Compat_StateToOld(statenum_t state);
statenum_t D_Compat_StateToNew(statenum_t state);
boolean D_Compat_StateExists(statenum_t state);

mobjtype_t D_Compat_MobjToOld(mobjtype_t mobj);
mobjtype_t D_Compat_MobjToNew(mobjtype_t mobj);
boolean D_Compat_MobjExists(mobjtype_t mobj);

#endif
