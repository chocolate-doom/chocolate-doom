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
#define MAX_BUFFER_SIZE 0x10000

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

    // Data buffer used to store data read for SysEx or meta events:
    byte *buffer;
    unsigned int buffer_size;
};

static boolean CheckChunkHeader(chunk_header_t *chunk,
                                char *expected_id)
{
    boolean result;
    
    result = (strcmp((char *) chunk->chunk_id, expected_id) == 0);

    if (!result)
    {
        fprintf(stderr, "CheckChunkHeader: Expected '%s' chunk header\n",
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

    file->buffer = NULL;
    file->buffer_size = 0;

    // Open file

    file->stream = fopen(filename, "rb");

    if (file->stream == NULL)
    {
        fprintf(stderr, "MIDI_OpenFile: Failed to open '%s'\n", filename);
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
    free(file->buffer);
    free(file);
}

// Read a single byte.  Returns false on error.

static boolean ReadByte(midi_file_t *file, byte *result)
{
    int c;

    c = fgetc(file->stream);

    if (c == EOF)
    {
        fprintf(stderr, "ReadByte: Unexpected end of file\n");
        return false;
    }
    else
    {
        *result = (byte) c;

        return true;
    }
}

// Read a variable-length value.

static boolean ReadVariableLength(midi_file_t *file, unsigned int *result)
{
    int i;
    byte b;

    *result = 0;

    for (i=0; i<4; ++i)
    {
        if (!ReadByte(file, &b))
        {
            fprintf(stderr, "ReadVariableLength: Error while reading "
                            "variable-length value\n");
            return false;
        }

        // Insert the bottom seven bits from this byte.

        *result <<= 7;
        *result |= b & 0x7f;

        // If the top bit is not set, this is the end.

        if ((b & 0x80) == 0)
        {
            return true;
        }
    }

    fprintf(stderr, "ReadVariableLength: Variable-length value too "
                    "long: maximum of four bytes\n");
    return false;
}

// Expand the size of the buffer used for SysEx/Meta events:

static boolean ExpandBuffer(midi_file_t *file, unsigned int new_size)
{
    byte *new_buffer;

    if (new_size > MAX_BUFFER_SIZE)
    {
        fprintf(stderr, "ExpandBuffer: Tried to expand buffer to %u bytes\n",
                        new_size);
        return false;
    }

    if (file->buffer_size < new_size)
    {
        // Reallocate to a larger size:

        new_buffer = realloc(file->buffer, new_size);

        if (new_buffer == NULL)
        {
            fprintf(stderr, "ExpandBuffer: Failed to expand buffer to %u "
                            "bytes\n", new_size);
            return false;
        }

        file->buffer = new_buffer;
        file->buffer_size = new_size;
    }

    return true;
}

// Read a byte sequence into the data buffer.

static boolean ReadByteSequence(midi_file_t *file, unsigned int num_bytes)
{
    unsigned int i;

    // Check that we have enough space:

    if (!ExpandBuffer(file, num_bytes))
    {
        return false;
    }

    for (i=0; i<num_bytes; ++i)
    {
        if (!ReadByte(file, &file->buffer[i]))
        {
            fprintf(stderr, "ReadByteSequence: Error while reading byte %u\n",
                            i);
            return false;
        }
    }

    return true;
}

// Read a MIDI channel event.
// two_param indicates that the event type takes two parameters
// (three byte) otherwise it is single parameter (two byte)

static boolean ReadChannelEvent(midi_file_t *file, midi_event_t *event,
                                byte event_type, boolean two_param)
{
    byte b;

    // Set basics:

    event->event_type = event_type >> 4;
    event->data.channel.channel = event_type & 0xf;

    // Read parameters:

    if (!ReadByte(file, &b))
    {
        fprintf(stderr, "ReadChannelEvent: Error while reading channel "
                        "event parameters\n");
        return false;
    }

    event->data.channel.param1 = b;

    // Second parameter:

    if (two_param)
    {
        if (!ReadByte(file, &b))
        {
            fprintf(stderr, "ReadChannelEvent: Error while reading channel "
                            "event parameters\n");
            return false;
        }

        event->data.channel.param2 = b;
    }

    return true;
}

// Read sysex event:

static boolean ReadSysExEvent(midi_file_t *file, midi_event_t *event,
                              int event_type)
{
    event->event_type = event_type;

    if (!ReadVariableLength(file, &event->data.sysex.length))
    {
        fprintf(stderr, "ReadSysExEvent: Failed to read length of "
                                        "SysEx block\n");
        return false;
    }

    // Read the byte sequence:

    if (!ReadByteSequence(file, event->data.sysex.length))
    {
        fprintf(stderr, "ReadSysExEvent: Failed while reading SysEx event\n");
        return false;
    }

    event->data.sysex.data = file->buffer;

    return true;
}

// Read meta event:

static boolean ReadMetaEvent(midi_file_t *file, midi_event_t *event)
{
    byte b;

    // Read meta event type:

    if (!ReadByte(file, &b))
    {
        fprintf(stderr, "ReadMetaEvent: Failed to read meta event type\n");
        return false;
    }

    event->data.meta.type = b;

    // Read length of meta event data:

    if (!ReadVariableLength(file, &event->data.meta.length))
    {
        fprintf(stderr, "ReadSysExEvent: Failed to read length of "
                                        "SysEx block\n");
        return false;
    }

    // Read the byte sequence:

    if (!ReadByteSequence(file, event->data.meta.length))
    {
        fprintf(stderr, "ReadSysExEvent: Failed while reading SysEx event\n");
        return false;
    }

    event->data.meta.data = file->buffer;

    return true;
}

boolean MIDI_ReadEvent(midi_file_t *file, midi_event_t *event)
{
    byte event_type;

    if (!ReadVariableLength(file, &event->delta_time))
    {
        fprintf(stderr, "MIDI_ReadEvent: Failed to read event timestamp\n");
        return false;
    }

    if (!ReadByte(file, &event_type))
    {
        fprintf(stderr, "MIDI_ReadEvent: Failed to read event type\n");
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

        // --- Fall-through deliberate ---
        // Other 0xfx event types are unknown

        default:
            fprintf(stderr, "Unknown MIDI event type: 0x%x\n", event_type);
            return false;
    }
}

