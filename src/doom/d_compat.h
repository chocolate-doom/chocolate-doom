//
// Copyright(C) 2019 James Canete
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

#include "doomtype.h"

#include "tables.h"

extern int angletocoarseshift;

extern const fixed_t *coarsecosine;
extern const fixed_t *coarsesine;

void D_SetTurboScale(int scale);
void D_GetScaledMove(fixed_t *forward, fixed_t *side, boolean justattacked);

void D_SetConstantsForGameversion(void);
