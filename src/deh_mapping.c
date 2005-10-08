// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deh_mapping.h"

//
// Set the value of a particular field in a structure by name
//

boolean DEH_SetMapping(deh_mapping_t *mapping, 
                       void *structptr, char *name, int value)
{
    int i;

    for (i=0; mapping->entries[i].name != NULL; ++i)
    {
        deh_mapping_entry_t *entry = &mapping->entries[i];

        if (!strcasecmp(entry->name, name))
        {
            void *location;

            if (entry->location == NULL)
            {
                fprintf(stderr, "DEH_SetMapping: Field '%s' is unsupported.\n",
                        name);
                return false;
            }

            location = structptr + (entry->location - mapping->base);

     //       printf("Setting %p::%s to %i (%i bytes)\n",
     //               structptr, name, value, entry->size);
                
            switch (entry->size)
            {
                case 1:
                    * ((unsigned char *) location) = value;
                    break;
                case 2:
                    * ((unsigned short *) location) = value;
                    break;
                case 4:
                    * ((unsigned int *) location) = value;
                    break;
                case 8:
                    * ((unsigned long long *) location) = value;
                    break;
                default:
                    fprintf(stderr, "DEH_SetMapping: Unknown field type for %s\n", name);
                    return false;
            }

            return true;
        }
    }

    // field with this name not found

    fprintf(stderr, "DEH_SetMapping: field named '%s' not found\n",
            name);

    return false;
}

