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

#ifdef _WIN32

#include "buffer.h"

#include <stdlib.h>

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
void DeleteBuffer(buffer_t *buf)
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
    ptrdiff_t space_begin, space_end;

    if (len <= 0)
    {
        // Do nothing, successfully.
        return true;
    }

    space_begin = buf->data - buf->buffer;
    space_end = buf->buffer_end - buf->data_end;

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
    ptrdiff_t max_shift;

    if (len <= 0)
    {
        // Do nothing.
        return;
    }

    max_shift = buf->data_end - buf->data;
    if (len >= max_shift)
    {
        // If the operation would clear the buffer, just zero everything.
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
// Create a new buffer reader.
//
// WARNING: This reader will invalidate if the underlying buffer changes.
//          Use it, then delete it before you touch the underlying buffer again.
//
buffer_reader_t *NewReader(buffer_t* buffer)
{
    buffer_reader_t *reader = malloc(sizeof(buffer_reader_t));

    reader->buffer = buffer;
    reader->pos = buffer->data;

    return reader;
}

//
// Delete a buffer reader.
//
void DeleteReader(buffer_reader_t *reader)
{
    free(reader);
}

//
// Count the number of bytes read thus far.
//
int Reader_BytesRead(buffer_reader_t *reader)
{
    return reader->pos - reader->buffer->data;
}

//
// Read an unsigned byte from a buffer.
//
boolean Reader_ReadInt8(buffer_reader_t *reader, uint8_t *out)
{
    byte *data, *data_end;
    int len = Buffer_Data(reader->buffer, &data);

    data_end = data + len;

    if (data_end - reader->pos < 1)
    {
        return false;
    }

    *out = (uint8_t)*reader->pos;
    reader->pos += 1;

    return true;
}

//
// Read an unsigned short from a buffer.
//
boolean Reader_ReadInt16(buffer_reader_t *reader, uint16_t *out)
{
    byte *data, *data_end, *dp;
    int len = Buffer_Data(reader->buffer, &data);

    data_end = data + len;
    dp = reader->pos;

    if (data_end - reader->pos < 2)
    {
        return false;
    }

    *out = (uint16_t)((dp[0] << 8) | dp[1]);
    reader->pos += 2;

    return true;
}

//
// Read an unsigned int from a buffer.
//
boolean Reader_ReadInt32(buffer_reader_t *reader, uint32_t *out)
{
    byte *data, *data_end, *dp;
    int len = Buffer_Data(reader->buffer, &data);

    data_end = data + len;
    dp = reader->pos;

    if (data_end - reader->pos < 4)
    {
        return false;
    }

    *out = (uint32_t)((dp[0] << 24) | (dp[1] << 16) | (dp[2] << 8) | dp[3]);
    reader->pos += 4;

    return true;
}

//
// Read a string from a buffer.
//
char *Reader_ReadString(buffer_reader_t *reader)
{
    byte *data, *data_start, *data_end, *dp;
    int len = Buffer_Data(reader->buffer, &data);

    data_start = reader->pos;
    data_end = data + len;
    dp = reader->pos;

    while (dp < data_end && *dp != '\0')
    {
        dp++;
    }

    if (dp >= data_end)
    {
        // Didn't see a null terminator, not a complete string.
        return NULL;
    }

    reader->pos = dp + 1;
    return (char*)data_start;
}

#endif // #ifdef _WIN32
