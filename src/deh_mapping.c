//
// Copyright(C) 2005-2014 Simon Howard
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
//
// Dehacked "mapping" code
// Allows the fields in structures to be mapped out and accessed by
// name
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_misc.h"

#include "deh_mapping.h"

static deh_mapping_entry_t *GetMappingEntryByName(deh_context_t *context,
                                                  deh_mapping_t *mapping,
                                                  char *name)
{
    int i;

    for (i=0; mapping->entries[i].name != NULL; ++i)
    {
        deh_mapping_entry_t *entry = &mapping->entries[i];

        if (!strcasecmp(entry->name, name))
        {
            if (entry->location == NULL)
            {
                DEH_Warning(context, "Field '%s' is unsupported", name);
                return NULL;
            }

            return entry;
        }
    }

    // Not found.

    DEH_Warning(context, "Field named '%s' not found", name);

    return NULL;
}

//
// Get the location of the specified field in the specified structure.
//

static void *GetStructField(void *structptr,
                            deh_mapping_t *mapping,
                            deh_mapping_entry_t *entry)
{
    unsigned int offset;

    offset = (uint8_t *)entry->location - (uint8_t *)mapping->base;

    return (uint8_t *)structptr + offset;
}

//
// Set the value of a particular field in a structure by name
//

boolean DEH_SetMapping(deh_context_t *context, deh_mapping_t *mapping,
                       void *structptr, char *name, int value)
{
    deh_mapping_entry_t *entry;
    void *location;

    entry = GetMappingEntryByName(context, mapping, name);

    if (entry == NULL)
    {
        return false;
    }

    // Sanity check:

    if (entry->is_string)
    {
        DEH_Error(context, "Tried to set '%s' as integer (BUG)", name);
        return false;
    }

    location = GetStructField(structptr, mapping, entry);

    //       printf("Setting %p::%s to %i (%i bytes)\n",
    //               structptr, name, value, entry->size);

    // Set field content based on its type:

    switch (entry->size)
    {
        case 1:
            * ((uint8_t *) location) = value;
            break;
        case 2:
            * ((uint16_t *) location) = value;
            break;
        case 4:
            * ((uint32_t *) location) = value;
            break;
        default:
            DEH_Error(context, "Unknown field type for '%s' (BUG)", name);
            return false;
    }

    return true;
}

//
// Set the value of a string field in a structure by name
//

boolean DEH_SetStringMapping(deh_context_t *context, deh_mapping_t *mapping,
                             void *structptr, char *name, char *value)
{
    deh_mapping_entry_t *entry;
    void *location;

    entry = GetMappingEntryByName(context, mapping, name);

    if (entry == NULL)
    {
        return false;
    }

    // Sanity check:

    if (!entry->is_string)
    {
        DEH_Error(context, "Tried to set '%s' as string (BUG)", name);
        return false;
    }

    location = GetStructField(structptr, mapping, entry);

    // Copy value into field:

    M_StringCopy(location, value, entry->size);

    return true;
}

void DEH_StructSHA1Sum(sha1_context_t *context, deh_mapping_t *mapping,
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

        location = (uint8_t *)structptr + ((uint8_t *)entry->location - (uint8_t *)mapping->base);

        switch (entry->size)
        {
            case 1:
                SHA1_UpdateInt32(context, *((uint8_t *) location));
                break;
            case 2:
                SHA1_UpdateInt32(context, *((uint16_t *) location));
                break;
            case 4:
                SHA1_UpdateInt32(context, *((uint32_t *) location));
                break;
            default:
                I_Error("Unknown dehacked mapping field type for '%s' (BUG)", 
                        entry->name);
                break;
        }
    }
}

