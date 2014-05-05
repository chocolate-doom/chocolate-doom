//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


// HEADER FILES ------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "h2def.h"
#include "i_system.h"
#include "m_misc.h"

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
static void OpenScript(char *name, int type);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char *sc_String;
int sc_Number;
int sc_Line;
boolean sc_End;
boolean sc_Crossed;
boolean sc_FileScripts = false;
char *sc_ScriptsDir = "";

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
// SC_Open
//
//==========================================================================

void SC_Open(char *name)
{
    char fileName[128];

    if (sc_FileScripts == true)
    {
        M_snprintf(fileName, sizeof(fileName), "%s%s.txt", sc_ScriptsDir, name);
        SC_OpenFile(fileName);
    }
    else
    {
        SC_OpenLump(name);
    }
}

//==========================================================================
//
// SC_OpenLump
//
// Loads a script (from the WAD files) and prepares it for parsing.
//
//==========================================================================

void SC_OpenLump(char *name)
{
    OpenScript(name, LUMP_SCRIPT);
}

//==========================================================================
//
// SC_OpenFile
//
// Loads a script (from a file) and prepares it for parsing.  Uses the
// zone memory allocator for memory allocation and de-allocation.
//
//==========================================================================

void SC_OpenFile(char *name)
{
    OpenScript(name, FILE_ZONE_SCRIPT);
}

//==========================================================================
//
// OpenScript
//
//==========================================================================

static void OpenScript(char *name, int type)
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

void SC_Close(void)
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

boolean SC_GetString(void)
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
        while (*ScriptPtr <= 32)
        {
            if (ScriptPtr >= ScriptEndPtr)
            {
                sc_End = true;
                return false;
            }
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
// SC_MustGetString
//
//==========================================================================

void SC_MustGetString(void)
{
    if (SC_GetString() == false)
    {
        SC_ScriptError("Missing string.");
    }
}

//==========================================================================
//
// SC_MustGetStringName
//
//==========================================================================

void SC_MustGetStringName(char *name)
{
    SC_MustGetString();
    if (SC_Compare(name) == false)
    {
        SC_ScriptError(NULL);
    }
}

//==========================================================================
//
// SC_GetNumber
//
//==========================================================================

boolean SC_GetNumber(void)
{
    char *stopper;

    CheckOpen();
    if (SC_GetString())
    {
        sc_Number = strtol(sc_String, &stopper, 0);
        if (*stopper != 0)
        {
            I_Error("SC_GetNumber: Bad numeric constant \"%s\".\n"
                    "Script %s, Line %d", sc_String, ScriptName, sc_Line);
        }
        return true;
    }
    else
    {
        return false;
    }
}

//==========================================================================
//
// SC_MustGetNumber
//
//==========================================================================

void SC_MustGetNumber(void)
{
    if (SC_GetNumber() == false)
    {
        SC_ScriptError("Missing integer.");
    }
}

//==========================================================================
//
// SC_UnGet
//
// Assumes there is a valid string in sc_String.
//
//==========================================================================

void SC_UnGet(void)
{
    AlreadyGot = true;
}

//==========================================================================
//
// SC_Check
//
// Returns true if another token is on the current line.
//
//==========================================================================

/*
boolean SC_Check(void)
{
	char *text;

	CheckOpen();
	text = ScriptPtr;
	if(text >= ScriptEndPtr)
	{
		return false;
	}
	while(*text <= 32)
	{
		if(*text == '\n')
		{
			return false;
		}
		text++;
		if(text == ScriptEndPtr)
		{
			return false;
		}
	}
	if(*text == ASCII_COMMENT)
	{
		return false;
	}
	return true;
}
*/

//==========================================================================
//
// SC_MatchString
//
// Returns the index of the first match to sc_String from the passed
// array of strings, or -1 if not found.
//
//==========================================================================

int SC_MatchString(char **strings)
{
    int i;

    for (i = 0; *strings != NULL; i++)
    {
        if (SC_Compare(*strings++))
        {
            return i;
        }
    }
    return -1;
}

//==========================================================================
//
// SC_MustMatchString
//
//==========================================================================

int SC_MustMatchString(char **strings)
{
    int i;

    i = SC_MatchString(strings);
    if (i == -1)
    {
        SC_ScriptError(NULL);
    }
    return i;
}

//==========================================================================
//
// SC_Compare
//
//==========================================================================

boolean SC_Compare(char *text)
{
    if (strcasecmp(text, sc_String) == 0)
    {
        return true;
    }
    return false;
}

//==========================================================================
//
// SC_ScriptError
//
//==========================================================================

void SC_ScriptError(char *message)
{
    if (message == NULL)
    {
        message = "Bad syntax.";
    }
    I_Error("Script error, \"%s\" line %d: %s", ScriptName, sc_Line, message);
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
