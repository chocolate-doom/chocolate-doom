//
// Copyright(C) 2022 ceski
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
//      MIDI instrument fallback support
//

#ifdef _WIN32

#ifndef MIDIFALLBACK_H
#define MIDIFALLBACK_H

#include "doomtype.h"
#include "midifile.h"

typedef enum midi_fallback_type_t
{
    FALLBACK_NONE,
    FALLBACK_BANK_MSB,
    FALLBACK_BANK_LSB,
    FALLBACK_DRUMS,
} midi_fallback_type_t;

typedef struct midi_fallback_t
{
    midi_fallback_type_t type;
    byte value;
} midi_fallback_t;

void MIDI_CheckFallback(const midi_event_t *event, midi_fallback_t *fallback,
                        boolean allow_sysex);
void MIDI_ResetFallback(void);
void MIDI_InitFallback(void);

#endif // MIDIFALLBACK_H

#endif // _WIN32
