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
//     MIDI file parsing.
//
//-----------------------------------------------------------------------------

#ifndef MIDIFILE_H
#define MIDIFILE_H

typedef struct midi_file_s midi_file_t;
typedef struct midi_track_iter_s midi_track_iter_t;

#define MIDI_CHANNELS_PER_TRACK 16

typedef enum
{
    MIDI_EVENT_NOTE_OFF        = 0x80,
    MIDI_EVENT_NOTE_ON         = 0x90,
    MIDI_EVENT_AFTERTOUCH      = 0xa0,
    MIDI_EVENT_CONTROLLER      = 0xb0,
    MIDI_EVENT_PROGRAM_CHANGE  = 0xc0,
    MIDI_EVENT_CHAN_AFTERTOUCH = 0xd0,
    MIDI_EVENT_PITCH_BEND      = 0xe0,

    MIDI_EVENT_SYSEX           = 0xf0,
    MIDI_EVENT_SYSEX_SPLIT     = 0xf7,
    MIDI_EVENT_META            = 0xff,
} midi_event_type_t;

typedef enum
{
    MIDI_CONTROLLER_BANK_SELECT     = 0x0,
    MIDI_CONTROLLER_MODULATION      = 0x1,
    MIDI_CONTROLLER_BREATH_CONTROL  = 0x2,
    MIDI_CONTROLLER_FOOT_CONTROL    = 0x3,
    MIDI_CONTROLLER_PORTAMENTO      = 0x4,
    MIDI_CONTROLLER_DATA_ENTRY      = 0x5,

    MIDI_CONTROLLER_MAIN_VOLUME     = 0x7,
    MIDI_CONTROLLER_PAN             = 0xa
} midi_controller_t;

typedef enum
{
    MIDI_META_SEQUENCE_NUMBER       = 0x0,

    MIDI_META_TEXT                  = 0x1,
    MIDI_META_COPYRIGHT             = 0x2,
    MIDI_META_TRACK_NAME            = 0x3,
    MIDI_META_INSTR_NAME            = 0x4,
    MIDI_META_LYRICS                = 0x5,
    MIDI_META_MARKER                = 0x6,
    MIDI_META_CUE_POINT             = 0x7,

    MIDI_META_CHANNEL_PREFIX        = 0x20,
    MIDI_META_END_OF_TRACK          = 0x2f,

    MIDI_META_SET_TEMPO             = 0x51,
    MIDI_META_SMPTE_OFFSET          = 0x54,
    MIDI_META_TIME_SIGNATURE        = 0x58,
    MIDI_META_KEY_SIGNATURE         = 0x59,
    MIDI_META_SEQUENCER_SPECIFIC    = 0x7f,
} midi_meta_event_type_t;

typedef struct
{
    // Meta event type:

    unsigned int type;

    // Length:

    unsigned int length;

    // Meta event data:

    byte *data;
} midi_meta_event_data_t;

typedef struct
{
    // Length:

    unsigned int length;

    // Event data:

    byte *data;
} midi_sysex_event_data_t;

typedef struct
{
    // The channel number to which this applies:

    unsigned int channel;

    // Extra parameters:

    unsigned int param1;
    unsigned int param2;
} midi_channel_event_data_t;

typedef struct
{
    // Time between the previous event and this event.
    unsigned int delta_time;

    // Type of event:
    midi_event_type_t event_type;

    union
    {
        midi_channel_event_data_t channel;
        midi_meta_event_data_t meta;
        midi_sysex_event_data_t sysex;
    } data;
} midi_event_t;

// Load a MIDI file.

midi_file_t *MIDI_LoadFile(char *filename);

// Free a MIDI file.

void MIDI_FreeFile(midi_file_t *file);

// Get the time division value from the MIDI header.

unsigned int MIDI_GetFileTimeDivision(midi_file_t *file);

// Get the number of tracks in a MIDI file.

unsigned int MIDI_NumTracks(midi_file_t *file);

// Start iterating over the events in a track.

midi_track_iter_t *MIDI_IterateTrack(midi_file_t *file, unsigned int track_num);

// Free an iterator.

void MIDI_FreeIterator(midi_track_iter_t *iter);

// Get the time until the next MIDI event in a track.

unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter);

// Get a pointer to the next MIDI event.

int MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event);

// Reset an iterator to the beginning of a track.

void MIDI_RestartIterator(midi_track_iter_t *iter);

#endif /* #ifndef MIDIFILE_H */

