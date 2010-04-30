// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
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
//     OPL timer thread.
//
//-----------------------------------------------------------------------------

#ifndef OPL_TIMER_H
#define OPL_TIMER_H

#include "opl.h"

int OPL_Timer_StartThread(void);
void OPL_Timer_StopThread(void);
void OPL_Timer_SetCallback(unsigned int ms,
                           opl_callback_t callback,
                           void *data);
void OPL_Timer_ClearCallbacks(void);
void OPL_Timer_Lock(void);
void OPL_Timer_Unlock(void);
void OPL_Timer_SetPaused(int paused);

#endif /* #ifndef OPL_TIMER_H */

