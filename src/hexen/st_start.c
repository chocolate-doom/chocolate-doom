
//**************************************************************************
//**
//** st_start.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: st_start.c,v $
//** $Revision: 1.21 $
//** $Date: 95/12/21 15:03:51 $
//** $Author: bgokey $
//**
//**************************************************************************


// HEADER FILES ------------------------------------------------------------
#ifdef __WATCOMC__
	#include <sys\stat.h>
	#include <sys\types.h>
	#include <io.h>
#else
	#include <libc.h>
	#include <ctype.h>
	#define O_BINARY 0
#endif
#include "h2def.h"
#include <fcntl.h>
#include <stdarg.h>				// Needed for next as well as dos
#include "st_start.h"


// MACROS ------------------------------------------------------------------
#define ST_MAX_NOTCHES		32
#define ST_NOTCH_WIDTH		16
#define ST_NOTCH_HEIGHT		23
#define ST_PROGRESS_X		64			// Start of notches x screen pos.
#define ST_PROGRESS_Y		441			// Start of notches y screen pos.

#define ST_NETPROGRESS_X		288
#define ST_NETPROGRESS_Y		32
#define ST_NETNOTCH_WIDTH		8
#define ST_NETNOTCH_HEIGHT		16
#define ST_MAX_NETNOTCHES		8

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------
extern void SetVideoModeHR(void);
extern void ClearScreenHR(void);
extern void SlamHR(char *buffer);
extern void SlamBlockHR(int x, int y, int w, int h, char *src);
extern void InitPaletteHR(void);
extern void SetPaletteHR(byte *palette);
extern void GetPaletteHR(byte *palette);
extern void FadeToPaletteHR(byte *palette);
extern void FadeToBlackHR(void);
extern void BlackPaletteHR(void);
extern void I_StartupReadKeys(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------
char *ST_LoadScreen(void);
void ST_UpdateNotches(int notchPosition);
void ST_UpdateNetNotches(int notchPosition);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------
char *bitmap = NULL;

char notchTable[]=
{
	// plane 0
	0x00, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40,
	0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x03, 0xC0,
	0x0F, 0x90, 0x1B, 0x68, 0x3D, 0xBC, 0x3F, 0xFC, 0x20, 0x08, 0x20, 0x08,
	0x2F, 0xD8, 0x37, 0xD8, 0x37, 0xF8, 0x1F, 0xF8, 0x1C, 0x50,

	// plane 1
	0x00, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x02, 0x40,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0xA0,
	0x30, 0x6C, 0x24, 0x94, 0x42, 0x4A, 0x60, 0x0E, 0x60, 0x06, 0x7F, 0xF6,
	0x7F, 0xF6, 0x7F, 0xF6, 0x5E, 0xF6, 0x38, 0x16, 0x23, 0xAC,

	// plane 2
	0x00, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x02, 0x40, 0x02, 0x40,
	0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x03, 0xE0,
	0x30, 0x6C, 0x24, 0x94, 0x52, 0x6A, 0x7F, 0xFE, 0x60, 0x0E, 0x60, 0x0E,
	0x6F, 0xD6, 0x77, 0xD6, 0x56, 0xF6, 0x38, 0x36, 0x23, 0xAC,

	// plane 3
	0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
	0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0x80, 0x02, 0x40,
	0x0F, 0x90, 0x1B, 0x68, 0x3D, 0xB4, 0x1F, 0xF0, 0x1F, 0xF8, 0x1F, 0xF8,
	0x10, 0x28, 0x08, 0x28, 0x29, 0x08, 0x07, 0xE8, 0x1C, 0x50
};


// Red Network Progress notches
char netnotchTable[]=
{
	// plane 0
	0x80, 0x50, 0xD0, 0xf0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xD0, 0xF0, 0xC0,
	0x70, 0x50, 0x80, 0x60,

	// plane 1
	0x60, 0xE0, 0xE0, 0xA0, 0xA0, 0xA0, 0xE0, 0xA0, 0xA0, 0xA0, 0xE0, 0xA0,
	0xA0, 0xE0, 0x60, 0x00,

	// plane 2
	0x80, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
	0x10, 0x10, 0x80, 0x60,

	// plane 3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

// CODE --------------------------------------------------------------------



//--------------------------------------------------------------------------
//
// Startup Screen Functions
//
//--------------------------------------------------------------------------


//==========================================================================
//
// ST_Init - Do the startup screen
//
//==========================================================================

void ST_Init(void)
{
#ifdef __WATCOMC__
	char *pal;
	char *buffer;

	if (!debugmode)
	{
		// Set 640x480x16 mode
		SetVideoModeHR();
		ClearScreenHR();
		InitPaletteHR();
		BlackPaletteHR();

		// Load graphic
		buffer = ST_LoadScreen();
		pal = buffer;
		bitmap = buffer + 16*3;

		SlamHR(bitmap);
		FadeToPaletteHR(pal);
		Z_Free(buffer);
	}
#endif
}


void ST_Done(void)
{
#ifdef __WATCOMC__
	ClearScreenHR();
#endif
}


//==========================================================================
//
// ST_UpdateNotches
//
//==========================================================================

void ST_UpdateNotches(int notchPosition)
{
#ifdef __WATCOMC__
	int x = ST_PROGRESS_X + notchPosition*ST_NOTCH_WIDTH;
	int y = ST_PROGRESS_Y;
	SlamBlockHR(x,y, ST_NOTCH_WIDTH,ST_NOTCH_HEIGHT, notchTable);
#endif
}


//==========================================================================
//
// ST_UpdateNetNotches - indicates network progress
//
//==========================================================================

void ST_UpdateNetNotches(int notchPosition)
{
#ifdef __WATCOMC__
	int x = ST_NETPROGRESS_X + notchPosition*ST_NETNOTCH_WIDTH;
	int y = ST_NETPROGRESS_Y;
	SlamBlockHR(x,y, ST_NETNOTCH_WIDTH, ST_NETNOTCH_HEIGHT, netnotchTable);
#endif
}


//==========================================================================
//
// ST_Progress - increments progress indicator
//
//==========================================================================

void ST_Progress(void)
{
#ifdef __WATCOMC__
	static int notchPosition=0;

	// Check for ESC press -- during startup all events eaten here
	I_StartupReadKeys();

	if (debugmode)
	{
		printf(".");
	}
	else
	{
		if(notchPosition<ST_MAX_NOTCHES)
		{
			ST_UpdateNotches(notchPosition);
			S_StartSound(NULL, SFX_STARTUP_TICK);
			notchPosition++;
		}
	}
#else
	printf(".");
#endif
}


//==========================================================================
//
// ST_NetProgress - indicates network progress
//
//==========================================================================

void ST_NetProgress(void)
{
#ifdef __WATCOMC__
	static int netnotchPosition=0;
	if (debugmode)
	{
		printf("*");
	}
	else
	{
		if(netnotchPosition<ST_MAX_NETNOTCHES)
		{
			ST_UpdateNetNotches(netnotchPosition);
			S_StartSound(NULL, SFX_DRIP);
			netnotchPosition++;
		}
	}
#endif
}


//==========================================================================
//
// ST_NetDone - net progress complete
//
//==========================================================================
void ST_NetDone(void)
{
	S_StartSound(NULL, SFX_PICKUP_WEAPON);
}


//==========================================================================
//
// ST_Message - gives debug message
//
//==========================================================================

void ST_Message(char *message, ...)
{
	va_list argptr;
	char buffer[80];

	va_start(argptr, message);
	vsprintf(buffer, message, argptr);
	va_end(argptr);

	if ( strlen(buffer) >= 80 )
	{
		I_Error("Long debug message has overwritten memory");
	}

#ifdef __WATCOMC__
	if (debugmode)
	{
		printf(buffer);
	}
#else
	printf(buffer);
#endif
}

//==========================================================================
//
// ST_RealMessage - gives user message
//
//==========================================================================

void ST_RealMessage(char *message, ...)
{
	va_list argptr;
	char buffer[80];

	va_start(argptr, message);
	vsprintf(buffer, message, argptr);
	va_end(argptr);

	if ( strlen(buffer) >= 80 )
	{
		I_Error("Long debug message has overwritten memory\n");
	}

	printf(buffer);		// Always print these messages
}



//==========================================================================
//
// ST_LoadScreen - loads startup graphic
//
//==========================================================================


char *ST_LoadScreen(void)
{
	int length,lump;
	char *buffer;

	lump = W_GetNumForName("STARTUP");
	length = W_LumpLength(lump);
	buffer = (char *)Z_Malloc(length, PU_STATIC, NULL);
	W_ReadLump(lump, buffer);
	return(buffer);
}
