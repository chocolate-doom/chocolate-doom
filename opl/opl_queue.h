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
//     OPL callback queue.
//
//-----------------------------------------------------------------------------

#ifndef OPL_QUEUE_H
#define OPL_QUEUE_H

#include "opl.h"

typedef struct opl_callback_queue_s opl_callback_queue_t;

opl_callback_queue_t *OPL_Queue_Create(void);
int OPL_Queue_IsEmpty(opl_callback_queue_t *queue);
void OPL_Queue_Clear(opl_callback_queue_t *queue);
void OPL_Queue_Destroy(opl_callback_queue_t *queue);
void OPL_Queue_Push(opl_callback_queue_t *queue,
                    opl_callback_t callback, void *data,
                    unsigned int time);
int OPL_Queue_Pop(opl_callback_queue_t *queue,
                  opl_callback_t *callback, void **data);
unsigned int OPL_Queue_Peek(opl_callback_queue_t *queue);

#endif /* #ifndef OPL_QUEUE_H */

