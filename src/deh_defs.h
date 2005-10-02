// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_defs.h 153 2005-10-02 23:49:01Z fraggle $
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
// $Log$
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Definitions for use in the dehacked code
//
//-----------------------------------------------------------------------------

#ifndef DEH_DEFS_H
#define DEH_DEFS_H

typedef struct deh_context_s deh_context_t;
typedef struct deh_section_s deh_section_t;
typedef void (*deh_section_init_t)(void);
typedef void *(*deh_section_start_t)(deh_context_t *context, char *line);
typedef void (*deh_section_end_t)(deh_context_t *context, void *tag);
typedef void (*deh_line_parser_t)(deh_context_t *context, char *line, void *tag);

struct deh_section_s
{
    char *name;

    // Called on startup to initialise code

    deh_section_init_t init;
    
    // This is called when a new section is started.  The pointer
    // returned is used as a tag for the following calls.

    deh_section_start_t start;

    // This is called for each line in the section

    deh_line_parser_t line_parser;

    // This is called at the end of the section for any cleanup

    deh_section_end_t end;
};

#endif /* #ifndef DEH_DEFS_H */


