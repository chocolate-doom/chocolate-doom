//
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
//     A simple buffer and reader implementation.
//

#include "buffer.h"

//
// Create a new buffer.
//
buffer_t *NewBuffer()
{
    buffer_t *buf = malloc(sizeof(buffer_t));

    buf->buffer_end = buf->buffer + BUFFER_SIZE;
    Buffer_Clear(buf);

    return buf;
}

//
// Free a buffer.
//
void DeleteBuffer(buffer_t* buf)
{
    free(buf);
}

//
// Return the data in the buffer.
//
int Buffer_Data(buffer_t *buf, byte **data)
{
    *data = buf->data;
    return buf->data_len;
}

//
// Push data onto the end of the buffer.
//
boolean Buffer_Push(buffer_t *buf, const void *data, int len)
{
    if (len <= 0)
    {
        return true;
    }

    ptrdiff_t space_begin = buf->data - buf->buffer;
    ptrdiff_t space_end = buf->buffer_end - buf->data_end;

    if (len > space_end)
    {
        if (len > space_begin + space_end)
        {
            // Don't overflow the buffer.
            return false;
        }

        // Move our data to the front of the buffer.
        memmove(buf->buffer, buf->data, buf->data_len);
        buf->data = buf->buffer;
        buf->data_end = buf->buffer + buf->data_len;
    }

    // Append to the buffer.
    memcpy(buf->data_end, data, len);
    buf->data_len += len;
    buf->data_end = buf->data + buf->data_len;

    return true;
}


//
// Shift len bytes off of the front of the buffer.
//
void Buffer_Shift(buffer_t *buf, int len)
{
    if (len <= 0)
    {
        return;
    }

    ptrdiff_t max_shift = buf->data_end - buf->data;
    if (len >= max_shift)
    {
        Buffer_Clear(buf);
    }
    else
    {
        buf->data += len;
        buf->data_len -= len;
    }
}

//
// Clear the buffer.
//
void Buffer_Clear(buffer_t *buf)
{
    buf->data = buf->buffer;
    buf->data_end = buf->buffer;
    buf->data_len = 0;
}

//
// Create a new buffer reader
//
buffer_reader_t *NewReader(buffer_t* buffer)
{
    buffer_reader_t *reader = malloc(sizeof(buffer_reader_t));

    reader->buffer = buffer;
    reader->pos = 0;

    return reader;
}

//
// Delete a buffer reader
//
void DeleteReader(buffer_reader_t *reader)
{
    free(reader);
}
