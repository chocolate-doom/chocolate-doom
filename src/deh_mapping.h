// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_mapping.h 175 2005-10-08 20:54:16Z fraggle $
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
// Revision 1.3  2005/10/08 20:54:16  fraggle
// Proper dehacked error/warning framework.  Catch a load more errors.
//
// Revision 1.2  2005/10/08 20:14:24  fraggle
// Add the ability to specify unsupported fields
//
// Revision 1.1  2005/10/03 10:25:37  fraggle
// Add mapping code to map out structures and switch thing/frame code to use
// this.
//
//
//-----------------------------------------------------------------------------
//
// Dehacked "mapping" code
// Allows the fields in structures to be mapped out and accessed by
// name
//
//-----------------------------------------------------------------------------

#ifndef DEH_MAPPING_H
#define DEH_MAPPING_H

#include "doomtype.h"
#include "deh_io.h"

#define DEH_BEGIN_MAPPING(mapping_name, structname)           \
    static structname deh_mapping_base;                       \
    static deh_mapping_t mapping_name =                       \
    {                                                         \
        &deh_mapping_base,                                    \
        {

#define DEH_MAPPING(deh_name, fieldname)                      \
             {deh_name, &deh_mapping_base.fieldname,          \
                 sizeof(deh_mapping_base.fieldname)},

#define DEH_UNSUPPORTED_MAPPING(deh_name)                     \
             {deh_name, NULL, -1}
            
#define DEH_END_MAPPING                                       \
             {NULL}                                           \
        }                                                     \
    };

    

#define MAX_MAPPING_ENTRIES 32

typedef struct deh_mapping_s deh_mapping_t;
typedef struct deh_mapping_entry_s deh_mapping_entry_t;

struct deh_mapping_entry_s 
{
    // field name
   
    char *name;

    // location relative to the base in the deh_mapping_t struct
    // If this is NULL, it is an unsupported mapping

    void *location;

    // field size

    int size;
};

struct deh_mapping_s
{
    void *base;
    deh_mapping_entry_t entries[MAX_MAPPING_ENTRIES];
};

boolean DEH_SetMapping(deh_context_t *context, deh_mapping_t *mapping, 
                       void *structptr, char *name, int value);

#endif /* #ifndef DEH_MAPPING_H */

