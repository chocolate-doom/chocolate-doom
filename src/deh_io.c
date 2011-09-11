// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005 Simon Howard
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
//-----------------------------------------------------------------------------
//
// Dehacked I/O code (does all reads from dehacked files)
//
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"

#include "deh_defs.h"
#include "deh_io.h"

typedef enum
{
    DEH_INPUT_FILE,
    DEH_INPUT_LUMP
} deh_input_type_t;

struct deh_context_s
{
    deh_input_type_t type;
    char *filename;

    // If the input comes from a memory buffer, pointer to the memory
    // buffer.

    unsigned char *input_buffer;
    size_t input_buffer_len;
    unsigned int input_buffer_pos;
    int lumpnum;

    // If the input comes from a file, the file stream for reading
    // data.

    FILE *stream;

    // Current line number that we have reached:

    int linenum;

    // Used by DEH_ReadLine:

    boolean last_was_newline;
    char *readbuffer;
    int readbuffer_size;
};

static deh_context_t *DEH_NewContext(void)
{
    deh_context_t *context;

    context = Z_Malloc(sizeof(*context), PU_STATIC, NULL);

    // Initial read buffer size of 128 bytes

    context->readbuffer_size = 128;
    context->readbuffer = Z_Malloc(context->readbuffer_size, PU_STATIC, NULL);
    context->linenum = 0;
    context->last_was_newline = true;

    return context;
}

// Open a dehacked file for reading
// Returns NULL if open failed

deh_context_t *DEH_OpenFile(char *filename)
{
    FILE *fstream;
    deh_context_t *context;

    fstream = fopen(filename, "r");

    if (fstream == NULL)
        return NULL;

    context = DEH_NewContext();

    context->type = DEH_INPUT_FILE;
    context->stream = fstream;
    context->filename = strdup(filename);

    return context;
}

// Open a WAD lump for reading.

deh_context_t *DEH_OpenLump(int lumpnum)
{
    deh_context_t *context;
    void *lump;

    lump = W_CacheLumpNum(lumpnum, PU_STATIC);

    context = DEH_NewContext();

    context->type = DEH_INPUT_LUMP;
    context->lumpnum = lumpnum;
    context->input_buffer = lump;
    context->input_buffer_len = W_LumpLength(lumpnum);
    context->input_buffer_pos = 0;

    context->filename = malloc(9);
    strncpy(context->filename, lumpinfo[lumpnum].name, 8);
    context->filename[8] = '\0';

    return context;
}

// Close dehacked file

void DEH_CloseFile(deh_context_t *context)
{
    if (context->type == DEH_INPUT_FILE)
    {
        fclose(context->stream);
    }
    else if (context->type == DEH_INPUT_LUMP)
    {
        W_ReleaseLumpNum(context->lumpnum);
    }

    Z_Free(context->readbuffer);
    Z_Free(context);
}

int DEH_GetCharFile(deh_context_t *context)
{
    if (feof(context->stream))
    {
        // end of file

        return -1;
    }

    return fgetc(context->stream);
}

int DEH_GetCharLump(deh_context_t *context)
{
    int result;

    if (context->input_buffer_pos >= context->input_buffer_len)
    {
        return -1;
    }

    result = context->input_buffer[context->input_buffer_pos];
    ++context->input_buffer_pos;

    return result;
}

// Reads a single character from a dehacked file

int DEH_GetChar(deh_context_t *context)
{
    int result = 0;

    // Read characters, but ignore carriage returns
    // Essentially this is a DOS->Unix conversion

    do
    {
        switch (context->type)
        {
            case DEH_INPUT_FILE:
                result = DEH_GetCharFile(context);
                break;

            case DEH_INPUT_LUMP:
                result = DEH_GetCharLump(context);
                break;
        }
    } while (result == '\r');

    // Track the current line number

    if (context->last_was_newline)
    {
        ++context->linenum;
    }

    context->last_was_newline = result == '\n';

    return result;
}

// Increase the read buffer size

static void IncreaseReadBuffer(deh_context_t *context)
{
    char *newbuffer;
    int newbuffer_size;

    newbuffer_size = context->readbuffer_size * 2;
    newbuffer = Z_Malloc(newbuffer_size, PU_STATIC, NULL);

    memcpy(newbuffer, context->readbuffer, context->readbuffer_size);

    Z_Free(context->readbuffer);

    context->readbuffer = newbuffer;
    context->readbuffer_size = newbuffer_size;
}

// Read a whole line

char *DEH_ReadLine(deh_context_t *context)
{
    int c;
    int pos;

    for (pos = 0;;)
    {
        c = DEH_GetChar(context);

        if (c < 0)
        {
            // end of file

            return NULL;
        }

        // cope with lines of any length: increase the buffer size

        if (pos >= context->readbuffer_size)
        {
            IncreaseReadBuffer(context);
        }

        if (c == '\n')
        {
            // end of line: a full line has been read

            context->readbuffer[pos] = '\0';
            break;
        }
        else if (c != '\0')
        {
            // normal character; don't allow NUL characters to be
            // added.

            context->readbuffer[pos] = (char) c;
            ++pos;
        }
    }
    
    return context->readbuffer;
}

void DEH_Warning(deh_context_t *context, char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    
    fprintf(stderr, "%s:%i: warning: ", context->filename, context->linenum);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);
}

void DEH_Error(deh_context_t *context, char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    
    fprintf(stderr, "%s:%i: ", context->filename, context->linenum);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);

    I_Error("Error parsing dehacked file");
}


