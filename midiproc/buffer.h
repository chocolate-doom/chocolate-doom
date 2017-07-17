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

#ifndef __BUFFER__
#define __BUFFER__

#include "../src/doomtype.h"

#define BUFFER_SIZE 1024

typedef struct {
    byte  buffer[BUFFER_SIZE]; // Buffer.
    byte *buffer_end;          // End of Buffer.
    byte *data;                // Start of actual data.
    byte *data_end;            // End of actual data.
    int   data_len;            // Length of actual data.
} buffer_t;

typedef struct {
    buffer_t *buffer;
    byte     *pos;
} buffer_reader_t;

buffer_t *NewBuffer();
void DeleteBuffer(buffer_t* buf);
int Buffer_Data(buffer_t *buf, byte **data);
boolean Buffer_Push(buffer_t *buf, const void *data, int len);
void Buffer_Shift(buffer_t *buf, int len);
void Buffer_Clear(buffer_t *buf);

buffer_reader_t *NewReader(buffer_t* buffer);
void DeleteReader(buffer_reader_t *reader);
int Reader_BytesRead(buffer_reader_t *reader);
boolean Reader_ReadInt8(buffer_reader_t *reader, uint8_t *out);
boolean Reader_ReadInt16(buffer_reader_t *reader, uint16_t *out);
boolean Reader_ReadInt32(buffer_reader_t *reader, uint32_t *out);
char *Reader_ReadString(buffer_reader_t *reader);

#endif

