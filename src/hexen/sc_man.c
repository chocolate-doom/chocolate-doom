
//**************************************************************************
//**
//** sc_man.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: sc_man.c,v $
//** $Revision: 1.3 $
//** $Date: 96/01/06 03:23:43 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "h2def.h"

// MACROS ------------------------------------------------------------------

#define MAX_STRING_SIZE 64
#define ASCII_COMMENT (';')
#define ASCII_QUOTE (34)
#define LUMP_SCRIPT 1
#define FILE_ZONE_SCRIPT 2
#define FILE_CLIB_SCRIPT 3

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
static boolean ScriptOpen = false;
static boolean ScriptFreeCLib; // true = de-allocate using free()
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

	if(sc_FileScripts == true)
	{
		sprintf(fileName, "%s%s.txt", sc_ScriptsDir, name);
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
// SC_OpenFileCLib
//
// Loads a script (from a file) and prepares it for parsing.  Uses C
// library function calls for memory allocation and de-allocation.
//
//==========================================================================

void SC_OpenFileCLib(char *name)
{
	OpenScript(name, FILE_CLIB_SCRIPT);
}

//==========================================================================
//
// OpenScript
//
//==========================================================================

static void OpenScript(char *name, int type)
{
	SC_Close();
	if(type == LUMP_SCRIPT)
	{ // Lump script
		ScriptBuffer = (char *)W_CacheLumpName(name, PU_STATIC);
		ScriptSize = W_LumpLength(W_GetNumForName(name));
		strcpy(ScriptName, name);
		ScriptFreeCLib = false; // De-allocate using Z_Free()
	}
	else if(type == FILE_ZONE_SCRIPT)
	{ // File script - zone
		ScriptSize = M_ReadFile(name, (byte **)&ScriptBuffer);
		M_ExtractFileBase(name, ScriptName);
		ScriptFreeCLib = false; // De-allocate using Z_Free()
	}
	else
	{ // File script - clib
		ScriptSize = M_ReadFileCLib(name, (byte **)&ScriptBuffer);
		M_ExtractFileBase(name, ScriptName);
		ScriptFreeCLib = true; // De-allocate using free()
	}
	ScriptPtr = ScriptBuffer;
	ScriptEndPtr = ScriptPtr+ScriptSize;
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
	if(ScriptOpen)
	{
		if(ScriptFreeCLib == true)
		{
			free(ScriptBuffer);
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
	if(AlreadyGot)
	{
		AlreadyGot = false;
		return true;
	}
	foundToken = false;
	sc_Crossed = false;
	if(ScriptPtr >= ScriptEndPtr)
	{
		sc_End = true;
		return false;
	}
	while(foundToken == false)
	{
		while(*ScriptPtr <= 32)
		{
			if(ScriptPtr >= ScriptEndPtr)
			{
				sc_End = true;
				return false;
			}
			if(*ScriptPtr++ == '\n')
			{
				sc_Line++;
				sc_Crossed = true;
			}
		}
		if(ScriptPtr >= ScriptEndPtr)
		{
			sc_End = true;
			return false;
		}
		if(*ScriptPtr != ASCII_COMMENT)
		{ // Found a token
			foundToken = true;
		}
		else
		{ // Skip comment
			while(*ScriptPtr++ != '\n')
			{
				if(ScriptPtr >= ScriptEndPtr)
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
	if(*ScriptPtr == ASCII_QUOTE)
	{ // Quoted string
		ScriptPtr++;
		while(*ScriptPtr != ASCII_QUOTE)
		{
			*text++ = *ScriptPtr++;
			if(ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE-1])
			{
				break;
			}
		}
		ScriptPtr++;
	}
	else
	{ // Normal string
		while((*ScriptPtr > 32) && (*ScriptPtr != ASCII_COMMENT))
		{
			*text++ = *ScriptPtr++;
			if(ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE-1])
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
	if(SC_GetString() == false)
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
	if(SC_Compare(name) == false)
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
	if(SC_GetString())
	{
		sc_Number = strtol(sc_String, &stopper, 0);
		if(*stopper != 0)
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
	if(SC_GetNumber() == false)
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

	for(i = 0; *strings != NULL; i++)
	{
		if(SC_Compare(*strings++))
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
	if(i == -1)
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
	if(strcasecmp(text, sc_String) == 0)
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
	if(message == NULL)
	{
		message = "Bad syntax.";
	}
	I_Error("Script error, \"%s\" line %d: %s", ScriptName,
		sc_Line, message);
}

//==========================================================================
//
// CheckOpen
//
//==========================================================================

static void CheckOpen(void)
{
	if(ScriptOpen == false)
	{
		I_Error("SC_ call before SC_Open().");
	}
}
