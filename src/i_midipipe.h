//
// Copyright(C) 2013 James Haley et al.
// Copyright(C) 2017 Alex Mayfield
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
//     Client Interface to RPC Midi Server
//

#ifndef __I_MIDISOCKET__
#define __I_MIDISOCKET__

#if _WIN32

#include "doomtype.h"

boolean I_MidiPipeInitServer();
boolean I_MidiPipeInitClient();
void I_MidiPipeClientShutDown();
boolean I_MidiPipeReady();

boolean I_MidiPipeRegisterSong(const char *filename);
boolean I_MidiPipePlaySong(boolean looping);
boolean I_MidiPipeStopSong();
boolean I_MidiPipeSetVolume(int volume);
boolean I_MidiPipePauseSong();
boolean I_MidiPipeResumeSong();

#endif

#endif

