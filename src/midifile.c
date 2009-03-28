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
//    Reading of MIDI files.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_swap.h"
#include "midifile.h"

#define HEADER_CHUNK_ID "MThd"
#define TRACK_CHUNK_ID  "MTrk"

typedef struct
{
    byte chunk_id[4];
    unsigned int chunk_size;
} chunk_header_t;

typedef struct
{
    chunk_header_t chunk_header;
    unsigned short format_type;
    unsigned short num_tracks;
    unsigned int time_division;
} midi_header_t;

struct midi_file_s
{
    FILE *stream;
    midi_header_t header;
    unsigned int data_len;
};

static boolean CheckChunkHeader(chunk_header_t *chunk,
                                char *expected_id)
{
    boolean result;
    
    result = (strcmp((char *) chunk->chunk_id, expected_id) == 0);

    if (!result)
    {
        fprintf(stderr, "CheckChunkHeader: Expected '%s' chunk header!\n",
                        expected_id);
    }

    return result;
}

// Read and check the header chunk.

static boolean ReadHeaderChunk(midi_file_t *file)
{
    size_t bytes_read;

    bytes_read = fread(&file->header, sizeof(midi_header_t), 1, file->stream);

    if (bytes_read < sizeof(midi_header_t))
    {
        return false;
    }

    if (!CheckChunkHeader(&file->header.chunk_header, HEADER_CHUNK_ID)
     || LONG(file->header.chunk_header.chunk_size) != 6)
    {
        fprintf(stderr, "ReadHeaderChunk: Invalid MIDI chunk header!\n");
        return false;
    }

    if (SHORT(file->header.format_type) != 0
     || SHORT(file->header.num_tracks) != 1)
    {
        fprintf(stderr, "ReadHeaderChunk: Only single track, "
                                        "type 0 MIDI files supported!\n");
        return false;
    }

    return true;
}

// Read and check the track chunk header

static boolean ReadTrackChunk(midi_file_t *file)
{
    size_t bytes_read;
    chunk_header_t chunk_header;

    bytes_read = fread(&chunk_header, sizeof(chunk_header_t), 1, file->stream);

    if (bytes_read < sizeof(chunk_header))
    {
        return false;
    }

    if (!CheckChunkHeader(&chunk_header, TRACK_CHUNK_ID))
    {
        return false;
    }

    file->data_len = LONG(chunk_header.chunk_size);

    return true;
}

midi_file_t *MIDI_OpenFile(char *filename)
{
    midi_file_t *file;

    file = malloc(sizeof(midi_file_t));

    if (file == NULL)
    {
        return NULL;
    }

    // Open file

    file->stream = fopen(filename, "rb");

    if (file->stream == NULL)
    {
        fprintf(stderr, "Failed to open '%s'\n", filename);
        free(file);
        return NULL;
    }

    // Read MIDI file header

    if (!ReadHeaderChunk(file))
    {
        fclose(file->stream);
        free(file);
        return NULL;
    }

    // Read track header

    if (!ReadTrackChunk(file))
    {
        fclose(file->stream);
        free(file);
        return NULL;
    }

    return file;
}

void MIDI_CloseFile(midi_file_t *file)
{
    fclose(file->stream);
    free(file);
}

// Read a MIDI channel event.
// two_param indicates that the event type takes two parameters
// (three byte) otherwise it is single parameter (two byte)

static boolean ReadChannelEvent(midi_file_t *file, midi_event_t *event,
                                int event_type, boolean two_param)
{
    int c;

    // Set basics:

    event->event_type = event_type >> 4;
    event->data.channel.channel = event_type & 0xf;

    // Read parameters:

    c = fgetc(file->stream);

    if (c == EOF)
    {
        return false;
    }

    event->data.channel.param1 = c;

    // Second parameter:

    if (two_param)
    {
        c = fgetc(file->stream);

        if (c == EOF)
        {
            return false;
        }

        event->data.channel.param2 = c;
    }

    return true;
}

// Read sysex event:

static boolean ReadSysExEvent(midi_file_t *file, midi_event_t *event,
                              int event_type)
{
    // TODO
    return false;
}

// Read meta event:

static boolean ReadMetaEvent(midi_file_t *file, midi_event_t *event)
{
    // TODO
    return false;
}

boolean MIDI_ReadEvent(midi_file_t *file, midi_event_t *event)
{
    int event_type;

    event_type = fgetc(file->stream);

    if (event_type == EOF)
    {
        return false;
    }

    // Check event type:

    switch (event_type >> 4)
    {
        // Two parameter channel events:

        case MIDI_EVENT_NOTE_OFF:
        case MIDI_EVENT_NOTE_ON:
        case MIDI_EVENT_AFTERTOUCH:
        case MIDI_EVENT_CONTROLLER:
        case MIDI_EVENT_PITCH_BEND:
            return ReadChannelEvent(file, event, event_type, true);

        // Single parameter channel events:

        case MIDI_EVENT_PROGRAM_CHANGE:
        case MIDI_EVENT_CHAN_AFTERTOUCH:
            return ReadChannelEvent(file, event, event_type, false);

        // Other event types:

        case 0xf:
            if (event_type == MIDI_EVENT_SYSEX
             || event_type == MIDI_EVENT_SYSEX_SPLIT)
            {
                return ReadSysExEvent(file, event, event_type);
            }
            else if (event_type == MIDI_EVENT_META)
            {
                return ReadMetaEvent(file, event);
            }

        // Fall-through deliberate -

        default:
            fprintf(stderr, "Unknown MIDI event type: 0x%x\n", event_type);
            return false;
    }
}

