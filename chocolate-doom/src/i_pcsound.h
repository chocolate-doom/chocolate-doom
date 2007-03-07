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
//	System interface for PC speaker sound.
//
//-----------------------------------------------------------------------------

#ifndef __I_PCSOUND_H__
#define __I_PCSOUND_H__

int I_PCS_StartSound(int id,
                     int channel,
                     int vol,
                     int sep,
                     int pitch,
                     int priority);
void I_PCS_StopSound(int handle);
int I_PCS_SoundIsPlaying(int handle);
void I_PCS_InitSound(void);

#endif /* #ifndef __I_PCSOUND_H__ */

