// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: w_merge.c 168 2005-10-08 18:23:18Z fraggle $
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
// Revision 1.1  2005/10/08 18:23:18  fraggle
// WAD merging code
//
//
// DESCRIPTION:
// Handles merging of PWADs, similar to deutex's -merge option
//
// Ideally this should work exactly the same as in deutex, but trying to
// read the deutex source code made my brain hurt.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"

typedef enum 
{ 
    SECTION_NORMAL, 
    SECTION_FLATS, 
    SECTION_SPRITES,
} section_t;

typedef struct
{
    lumpinfo_t *lumps;
    int numlumps;
} searchlist_t;

static searchlist_t iwad;
static searchlist_t pwad;

static searchlist_t iwad_flats;
static searchlist_t pwad_sprites;
static searchlist_t pwad_flats;

// Search in a list to find a lump with a particular name
// Linear search (slow!)
//
// Returns -1 if not found

static int FindInList(searchlist_t *list, char *name)
{
    int i;

    for (i=0; i<list->numlumps; ++i)
    {
        if (!strncasecmp(list->lumps[i].name, name, 8))
            return i;
    }

    return -1;
}

// Sets up the sprite/flat search lists

static void SetupLists(void)
{
    int startlump, endlump;
    
    // IWAD
    // look for the flats section

    startlump = FindInList(&iwad, "F_START");
    endlump = FindInList(&iwad, "F_END");

    if (startlump < 0 || endlump < 0)
    {
        I_Error("Flats section not found in IWAD");
    }

    iwad_flats.lumps = iwad.lumps + startlump + 1;
    iwad_flats.numlumps = endlump - startlump - 1;

    // PWAD
    // look for a flats section

    pwad_flats.numlumps = 0;
    startlump = FindInList(&pwad, "FF_START");

    if (startlump < 0)
    {
        startlump = FindInList(&pwad, "F_START");
    }

    if (startlump >= 0)
    {
        endlump = FindInList(&pwad, "FF_END");

        if (endlump < 0)
        {
            endlump = FindInList(&pwad, "F_END");
        }

        if (endlump > startlump)
        {
            pwad_flats.lumps = pwad.lumps + startlump + 1;
            pwad_flats.numlumps = endlump - startlump - 1;
        }
    }

    // look for a sprites section

    pwad_sprites.numlumps = 0;
    startlump = FindInList(&pwad, "SS_START");

    if (startlump < 0)
    {
        startlump = FindInList(&pwad, "S_START");
    }

    if (startlump >= 0)
    {
        endlump = FindInList(&pwad, "SS_END");

        if (endlump < startlump)
        {
            endlump = FindInList(&pwad, "S_END");
        }

        if (endlump > startlump)
        {
            pwad_sprites.lumps = pwad.lumps + startlump + 1;
            pwad_sprites.numlumps = endlump - startlump - 1;
        }
    }
}

// Perform the merge.
//
// The merge code creates a new lumpinfo list, adding entries from the
// IWAD first followed by the PWAD.
//
// For the IWAD:
//  * Flats are added.  If a flat with the same name is in the PWAD, 
//    it is ignored.  At the end of the section, all flats in the PWAD 
//    are inserted.  This is consistent with the behavior of deutex/deusf.
//  * Sprites are added.  If a sprite with the same name exists in the PWAD,
//    it is used to replace the sprite.
// 
// For the PWAD:
//  * All Sprites and Flats are ignored, with the assumption they have 
//    already been merged into the IWAD's sections.

static void DoMerge(void)
{
    section_t current_section;
    lumpinfo_t *newlumps;
    int num_newlumps;
    int lumpindex;
    int i, n;
    
    // Can't ever have more lumps than we already have
    newlumps = malloc(sizeof(lumpinfo_t) * numlumps);
    num_newlumps = 0;

    // Add IWAD lumps
    current_section = SECTION_NORMAL;

    for (i=0; i<iwad.numlumps; ++i)
    {
        lumpinfo_t *lump = &iwad.lumps[i];

        switch (current_section)
        {
            case SECTION_NORMAL:
                if (!strncasecmp(lump->name, "F_START", 8))
                {
                    current_section = SECTION_FLATS;
                }
                else if (!strncasecmp(lump->name, "S_START", 8))
                {
                    current_section = SECTION_SPRITES;
                }

                newlumps[num_newlumps++] = *lump;

                break;

            case SECTION_FLATS:

                // Have we reached the end of the section?

                if (!strncasecmp(lump->name, "F_END", 8))
                {
                    // Add all new flats from the PWAD to the end
                    // of the section

                    for (n=0; n<pwad_flats.numlumps; ++n)
                    {
                        newlumps[num_newlumps++] = pwad_flats.lumps[n];
                    }

                    newlumps[num_newlumps++] = *lump;

                    // back to normal reading
                    current_section = SECTION_NORMAL;
                }
                else
                {
                    // If there is a flat in the PWAD with the same name,
                    // do not add it now.  All PWAD flats are added to the
                    // end of the section. Otherwise, if it is only in the
                    // IWAD, add it now

                    lumpindex = FindInList(&pwad_flats, lump->name);

                    if (lumpindex < 0)
                    {
                        newlumps[num_newlumps++] = *lump;
                    }
                }

                break;

            case SECTION_SPRITES:

                // Have we reached the end of the section?

                if (!strncasecmp(lump->name, "S_END", 8))
                {
                    newlumps[num_newlumps++] = *lump;

                    // back to normal reading
                    current_section = SECTION_NORMAL;
                }
                else
                {
                    // If there is a sprite in the PWAD with the same name,
                    // replace this sprite

                    // Note: This is rather limited.  A PWAD sprite has to
                    // have EXACTLY the same name as that in the IWAD.  It
                    // does not allow the number of sides per frame to be
                    // changed in the PWAD. FIXME?

                    lumpindex = FindInList(&pwad_sprites, lump->name);

                    if (lumpindex >= 0)
                    {
                        newlumps[num_newlumps++] = pwad_sprites.lumps[lumpindex];
                    }
                    else
                    {
                        newlumps[num_newlumps++] = *lump;
                    }
                }

                break;
        }
    }
   
    // Add PWAD lumps
    current_section = SECTION_NORMAL;

    for (i=0; i<pwad.numlumps; ++i)
    {
        lumpinfo_t *lump = &pwad.lumps[i];

        switch (current_section)
        {
            case SECTION_NORMAL:
                if (!strncasecmp(lump->name, "F_START", 8)
                 || !strncasecmp(lump->name, "FF_START", 8))
                {
                    current_section = SECTION_FLATS;
                }
                else if (!strncasecmp(lump->name, "S_START", 8)
                      || !strncasecmp(lump->name, "SS_START", 8))
                {
                    current_section = SECTION_SPRITES;
                }
                else
                {
                    // Don't include the headers of sections
       
                    newlumps[num_newlumps++] = *lump;
                }
                break;

            case SECTION_FLATS:

                // PWAD flats are ignored (already merged)
  
                if (!strncasecmp(lump->name, "FF_END", 8)
                 || !strncasecmp(lump->name, "F_END", 8))
                {
                    // end of section
                    current_section = SECTION_NORMAL;
                }
                break;

            case SECTION_SPRITES:

                // PWAD sprites are ignored (already merged)

                if (!strncasecmp(lump->name, "SS_END", 8)
                 || !strncasecmp(lump->name, "S_END", 8))
                {
                    // end of section
                    current_section = SECTION_NORMAL;
                }
                break;
        }
    }

    // Switch to the new lumpinfo, and free the old one

    free(lumpinfo);
    lumpinfo = newlumps;
    numlumps = num_newlumps;
}

// Merge in a file by name

void W_MergeFile(char *filename)
{
    int old_numlumps;

    old_numlumps = numlumps;

    W_AddFile(filename);

    // failed to load?

    if (numlumps == old_numlumps)
        return;

    printf(" merging %s\n", filename);

    // iwad is at the start, pwad was appended to the end

    iwad.lumps = lumpinfo;
    iwad.numlumps = old_numlumps;

    pwad.lumps = lumpinfo + old_numlumps;
    pwad.numlumps = numlumps - old_numlumps;
    
    // Setup sprite/flat lists

    SetupLists();

    // Perform the merge

    DoMerge();
}


