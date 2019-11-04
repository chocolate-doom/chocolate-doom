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

fixed_t D_CoarseOrFineCosine(angle_t angle);
fixed_t D_CoarseOrFineSine(angle_t angle);

void D_SetTurboScale(int scale);
void D_GetScaledMove(fixed_t *forward, fixed_t *side, boolean justattacked);

void D_SetConstantsForGameversion(void);
