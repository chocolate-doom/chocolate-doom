//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2005-2006 Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] support MUSINFO lump (dynamic music changing)
//

// [crispy] adapted from chocolate-doom/src/hexen/sc_man.c:18-470

// HEADER FILES ------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "doomstat.h"
#include "i_system.h"
#include "m_misc.h"
#include "r_defs.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#include "s_musinfo.h"

// MACROS ------------------------------------------------------------------

#define MAX_STRING_SIZE 64
#define ASCII_COMMENT (';')
#define ASCII_QUOTE (34)
#define LUMP_SCRIPT 1
#define FILE_ZONE_SCRIPT 2

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void CheckOpen(void);
static void OpenScript(const char *name, int type);
static void SC_OpenLump(const char *name);
static void SC_Close(void);
static boolean SC_Compare(const char *text);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

static char *sc_String;
static int sc_Line;
static boolean sc_End;
static boolean sc_Crossed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char ScriptName[16];
static char *ScriptBuffer;
static char *ScriptPtr;
static char *ScriptEndPtr;
static char StringBuffer[MAX_STRING_SIZE];
static int ScriptLumpNum;
static boolean ScriptOpen = false;
static int ScriptSize;
static boolean AlreadyGot = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// SC_OpenLump
//
// Loads a script (from the WAD files) and prepares it for parsing.
//
//==========================================================================

static void SC_OpenLump(const char *name)
{
    OpenScript(name, LUMP_SCRIPT);
}

//==========================================================================
//
// OpenScript
//
//==========================================================================

static void OpenScript(const char *name, int type)
{
    SC_Close();
    if (type == LUMP_SCRIPT)
    {                           // Lump script
        ScriptLumpNum = W_GetNumForName(name);
        ScriptBuffer = (char *) W_CacheLumpNum(ScriptLumpNum, PU_STATIC);
        ScriptSize = W_LumpLength(ScriptLumpNum);
        M_StringCopy(ScriptName, name, sizeof(ScriptName));
    }
    else if (type == FILE_ZONE_SCRIPT)
    {                           // File script - zone
        ScriptLumpNum = -1;
        ScriptSize = M_ReadFile(name, (byte **) & ScriptBuffer);
        M_ExtractFileBase(name, ScriptName);
    }
    ScriptPtr = ScriptBuffer;
    ScriptEndPtr = ScriptPtr + ScriptSize;
    sc_Line = 1;
    sc_End = false;
    ScriptOpen = true;
    sc_String = StringBuffer;
    AlreadyGot = false;
}

//==========================================================================
//
// SC_Close
//
//==========================================================================

static void SC_Close(void)
{
    if (ScriptOpen)
    {
        if (ScriptLumpNum >= 0)
        {
            W_ReleaseLumpNum(ScriptLumpNum);
        }
        else
        {
            Z_Free(ScriptBuffer);
        }
        ScriptOpen = false;
    }
}

//==========================================================================
//
// SC_GetString
//
//==========================================================================

static boolean SC_GetString(void)
{
    char *text;
    boolean foundToken;

    CheckOpen();
    if (AlreadyGot)
    {
        AlreadyGot = false;
        return true;
    }
    foundToken = false;
    sc_Crossed = false;
    if (ScriptPtr >= ScriptEndPtr)
    {
        sc_End = true;
        return false;
    }
    while (foundToken == false)
    {
        while (ScriptPtr < ScriptEndPtr && *ScriptPtr <= 32)
        {
            if (*ScriptPtr++ == '\n')
            {
                sc_Line++;
                sc_Crossed = true;
            }
        }
        if (ScriptPtr >= ScriptEndPtr)
        {
            sc_End = true;
            return false;
        }
        if (*ScriptPtr != ASCII_COMMENT)
        {                       // Found a token
            foundToken = true;
        }
        else
        {                       // Skip comment
            while (*ScriptPtr++ != '\n')
            {
                if (ScriptPtr >= ScriptEndPtr)
                {
                    sc_End = true;
                    return false;
                }
            }
            sc_Line++;
            sc_Crossed = true;
        }
    }
    text = sc_String;
    if (*ScriptPtr == ASCII_QUOTE)
    {                           // Quoted string
        ScriptPtr++;
        while (*ScriptPtr != ASCII_QUOTE)
        {
            *text++ = *ScriptPtr++;
            if (ScriptPtr == ScriptEndPtr
                || text == &sc_String[MAX_STRING_SIZE - 1])
            {
                break;
            }
        }
        ScriptPtr++;
    }
    else
    {                           // Normal string
        while ((*ScriptPtr > 32) && (*ScriptPtr != ASCII_COMMENT))
        {
            *text++ = *ScriptPtr++;
            if (ScriptPtr == ScriptEndPtr
                || text == &sc_String[MAX_STRING_SIZE - 1])
            {
                break;
            }
        }
    }
    *text = 0;
    return true;
}

//==========================================================================
//
// SC_Compare
//
//==========================================================================

static boolean SC_Compare(const char *text)
{
    if (strcasecmp(text, sc_String) == 0)
    {
        return true;
    }
    return false;
}

//==========================================================================
//
// CheckOpen
//
//==========================================================================

static void CheckOpen(void)
{
    if (ScriptOpen == false)
    {
        I_Error("SC_ call before SC_Open().");
    }
}

// [crispy] adapted from prboom-plus/src/s_advsound.c:54-159

musinfo_t musinfo = {0};

//
// S_ParseMusInfo
// Parses MUSINFO lump.
//

void S_ParseMusInfo (const char *mapid)
{
  if (W_CheckNumForName("MUSINFO") != -1)
  {
    int num, lumpnum;
    int inMap = false;

    SC_OpenLump("MUSINFO");

    while (SC_GetString())
    {
      if (inMap || SC_Compare(mapid))
      {
        if (!inMap)
        {
          SC_GetString();
          inMap = true;
        }

        if (sc_String[0] == 'E' || sc_String[0] == 'e' ||
            sc_String[0] == 'M' || sc_String[0] == 'm')
        {
          break;
        }

        // Check number in range
        if (M_StrToInt(sc_String, &num) && num > 0 && num < MAX_MUS_ENTRIES)
        {
          if (SC_GetString())
          {
            lumpnum = W_CheckNumForName(sc_String);

            if (lumpnum > 0)
            {
              musinfo.items[num] = lumpnum;
//            printf("S_ParseMusInfo: (%d) %s\n", num, sc_String);
            }
            else
            {
              fprintf(stderr, "S_ParseMusInfo: Unknown MUS lump %s\n", sc_String);
            }
          }
        }
        else
        {
          fprintf(stderr, "S_ParseMusInfo: Number not in range 1 to %d\n", MAX_MUS_ENTRIES - 1);
        }
      }
    }

    SC_Close();
  }
}

void T_MusInfo (void)
{
  if (musinfo.tics < 0 || !musinfo.mapthing)
  {
    return;
  }

  if (musinfo.tics > 0)
  {
    musinfo.tics--;
  }
  else
  {
    if (!musinfo.tics && musinfo.lastmapthing != musinfo.mapthing)
    {
      // [crispy] encode music lump number in mapthing health
      int arraypt = musinfo.mapthing->health - 1000;

      if (arraypt >= 0 && arraypt < MAX_MUS_ENTRIES)
      {
        int lumpnum = musinfo.items[arraypt];

        if (lumpnum > 0 && lumpnum < numlumps)
        {
          S_ChangeMusInfoMusic(lumpnum, true);
        }
      }

      musinfo.tics = -1;
    }
  }
}
