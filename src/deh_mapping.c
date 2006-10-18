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
// Dehacked "mapping" code
// Allows the fields in structures to be mapped out and accessed by
// name
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i_system.h"
#include "deh_mapping.h"

//
// Set the value of a particular field in a structure by name
//

boolean DEH_SetMapping(deh_context_t *context, deh_mapping_t *mapping, 
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
                DEH_Warning(context, "Field '%s' is unsupported", name);
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
                default:
                    DEH_Error(context, "Unknown field type for '%s' (BUG)", name);
                    return false;
            }

            return true;
        }
    }

    // field with this name not found

    DEH_Warning(context, "Field named '%s' not found", name);

    return false;
}

void DEH_StructMD5Sum(md5_context_t *context, deh_mapping_t *mapping,
                      void *structptr)
{
    int i;

    // Go through each mapping

    for (i=0; mapping->entries[i].name != NULL; ++i)
    {
        deh_mapping_entry_t *entry = &mapping->entries[i];
        void *location;

        if (entry->location == NULL)
        {
            // Unsupported field

            continue;
        }

        // Add in data for this field

        location = structptr + (entry->location - mapping->base);

        switch (entry->size)
        {
            case 1:
                MD5_UpdateInt32(context, *((unsigned char *) location));
                break;
            case 2:
                MD5_UpdateInt32(context, *((unsigned short *) location));
                break;
            case 4:
                MD5_UpdateInt32(context, *((unsigned int *) location));
                break;
            default:
                I_Error("Unknown dehacked mapping field type for '%s' (BUG)", 
                        entry->name);
                break;
        }
    }
}

