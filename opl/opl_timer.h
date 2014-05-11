//
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
// DESCRIPTION:
//     OPL timer thread.
//

#ifndef OPL_TIMER_H
#define OPL_TIMER_H

#include "opl.h"

int OPL_Timer_StartThread(void);
void OPL_Timer_StopThread(void);
void OPL_Timer_SetCallback(uint64_t us, opl_callback_t callback, void *data);
void OPL_Timer_ClearCallbacks(void);
void OPL_Timer_Lock(void);
void OPL_Timer_Unlock(void);
void OPL_Timer_SetPaused(int paused);
void OPL_Timer_AdjustCallbacks(float factor);

#endif /* #ifndef OPL_TIMER_H */

