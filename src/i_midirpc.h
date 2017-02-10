// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2013 James Haley et al.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
// Additional terms and conditions compatible with the GPLv3 apply. See the
// file COPYING-EE for details.
//
//----------------------------------------------------------------------------
//
// DESCRIPTION:
//
// Client Interface to RPC Midi Server
//
//-----------------------------------------------------------------------------

#ifndef I_MIDIRPC_H__
#define I_MIDIRPC_H__

#ifdef EE_FEATURE_MIDIRPC

bool I_MidiRPCInitServer();
bool I_MidiRPCInitClient();
void I_MidiRPCClientShutDown();
bool I_MidiRPCReady();

bool I_MidiRPCRegisterSong(void *data, int size);
bool I_MidiRPCPlaySong(bool looping);
bool I_MidiRPCStopSong();
bool I_MidiRPCSetVolume(int volume);
bool I_MidiRPCPauseSong();
bool I_MidiRPCResumeSong();

#endif

#endif

// EOF

