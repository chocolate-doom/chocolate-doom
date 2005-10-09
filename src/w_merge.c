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
// Revision 1.2  2005/10/09 00:25:49  fraggle
// Improved sprite merging
//
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

typedef struct
{
    char sprname[4];
    char frame;
    int angles;
} replace_frame_t;

static searchlist_t iwad;
static searchlist_t pwad;

static searchlist_t iwad_flats;
static searchlist_t pwad_sprites;
static searchlist_t pwad_flats;

// lumps with these sprites must be replaced in the IWAD
static replace_frame_t *replace_frames;
static int num_replace_frames;
static int replace_frames_alloced;

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

// Initialise the replace list

static void InitReplaceList(void)
{
    if (replace_frames == NULL)
    {
        replace_frames_alloced = 128;
        replace_frames = Z_Malloc(sizeof(*replace_frames) * replace_frames_alloced,
                                 PU_STATIC, NULL);
    }

    num_replace_frames = 0;
}

// Add new sprite to the replace list

static void AddReplaceFrame(replace_frame_t *frame)
{
    int i;

    // Find if this is already in the list

    for (i=0; i<num_replace_frames; ++i)
    {
        if (!strncasecmp(replace_frames[i].sprname, frame->sprname, 4)
         && replace_frames[i].frame == frame->frame)
        {
            replace_frames[i].angles |= frame->angles;
            return;
        }
    }
    
    // Need to add to the list

    if (num_replace_frames >= replace_frames_alloced)
    {
        replace_frame_t *newframes;

        newframes = Z_Malloc(replace_frames_alloced * 2 * sizeof(*replace_frames),
                             PU_STATIC, NULL);
        memcpy(newframes, replace_frames,
               replace_frames_alloced * sizeof(*replace_frames));
        Z_Free(replace_frames);
        replace_frames_alloced *= 2;
        replace_frames = newframes;
    }

    // Add to end of list
    
    replace_frames[num_replace_frames++] = *frame;
}

// Converts a sprite name into an replace_frame_t

static void ParseSpriteName(char *name, replace_frame_t *result)
{
    int angle_num;

    strncpy(result->sprname, name, 4);
    result->frame = name[4];

    angle_num = name[5] - '0';
    
    if (angle_num == 0)
    {
        // '0' sprites are used for all angles

        result->angles = 0xffff;
    }
    else
    {
       result->angles = 1 << angle_num;
    }

    if (name[6] != '\0')
    {
        // second angle

        angle_num = name[7] - '0';

        if (angle_num == 0)
        {
            result->angles = 0xffff;
        }
        else
        {
            result->angles |= 1 << angle_num;
        }
    }
}

// Check if a sprite is in the replace list

static boolean InReplaceList(char *name)
{
    replace_frame_t igsprite;
    int i;
    
    ParseSpriteName(name, &igsprite);

    for (i=0; i<num_replace_frames; ++i)
    {
        if (!strncasecmp(replace_frames[i].sprname, igsprite.sprname, 4)
         && replace_frames[i].frame == igsprite.frame
         && (replace_frames[i].angles & igsprite.angles) != 0)
        {
            return true;
        }
    }
    
    return false;
}

// Generate the list.  Run at the start, before merging

static void GenerateReplaceList(void)
{
    replace_frame_t igsprite;
    int i;

    InitReplaceList();

    for (i=0; i<pwad_sprites.numlumps; ++i)
    {
        ParseSpriteName(pwad_sprites.lumps[i].name, &igsprite);
        AddReplaceFrame(&igsprite);
    }
}

// Perform the merge.
//
// The merge code creates a new lumpinfo list, adding entries from the
// IWAD first followed by the PWAD.
//
// For the IWAD:
//  * Flats are added.  If a flat with the same name is in the PWAD, 
//    it is ignored(deleted).  At the end of the section, all flats in the 
//    PWAD are inserted.  This is consistent with the behavior of 
//    deutex/deusf.
//  * Sprites are added.  The "replace list" is generated before the merge
//    from the list of sprites in the PWAD.  Any sprites in the IWAD found
//    to match the replace list are removed.  At the end of the section,
//    the sprites from the PWAD are inserted.
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
                    // add all the pwad sprites

                    for (n=0; n<pwad_sprites.numlumps; ++n)
                    {
                        newlumps[num_newlumps++] = pwad_sprites.lumps[n];
                    }

                    // copy the ending
                    newlumps[num_newlumps++] = *lump;

                    // back to normal reading
                    current_section = SECTION_NORMAL;
                }
                else
                {
                    // Is this lump holding a sprite to be replaced in the
                    // PWAD? If so, wait until the end to add it.

                    if (!InReplaceList(lump->name))
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

    // Generate list of sprites to be replaced by the PWAD

    GenerateReplaceList();

    // Perform the merge

    DoMerge();
}


