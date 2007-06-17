// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007 Simon Howard
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
//    PC speaker interface.
//
//-----------------------------------------------------------------------------

#ifndef PCSOUND_H
#define PCSOUND_H

typedef void (*pcsound_callback_func)(int *duration, int *frequency);

// Initialise the PC speaker subsystem.  The given function is called
// periodically to request more sound data to play.

int PCSound_Init(pcsound_callback_func callback_func);

// Shut down the PC speaker subsystem.

void PCSound_Shutdown(void);

// Set the preferred output sample rate when emulating a PC speaker. 
// This must be called before PCSound_Init.

void PCSound_SetSampleRate(int rate);

#endif /* #ifndef PCSOUND_H */

