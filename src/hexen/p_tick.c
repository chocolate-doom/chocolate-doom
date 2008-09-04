
//**************************************************************************
//**
//** p_tick.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_tick.c,v $
//** $Revision: 1.5 $
//** $Date: 95/10/08 21:53:00 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void RunThinkers(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int leveltime;
int TimerGame;
thinker_t thinkercap; // The head and tail of the thinker list

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// P_Ticker
//
//==========================================================================

void P_Ticker(void)
{
	int i;

	if(paused)
	{
		return;
	}
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(playeringame[i])
		{
			P_PlayerThink(&players[i]);
		}
	}
	if(TimerGame)
	{
		if(!--TimerGame)
		{
			G_Completed(P_TranslateMap(P_GetMapNextMap(gamemap)), 0);
		}
	}
	RunThinkers();
	P_UpdateSpecials();
	P_AnimateSurfaces();
	leveltime++;
}

//==========================================================================
//
// RunThinkers
//
//==========================================================================

static void RunThinkers(void)
{
	thinker_t *currentthinker;

	currentthinker = thinkercap.next;
	while(currentthinker != &thinkercap)
	{
		if(currentthinker->function == (think_t)-1)
		{ // Time to remove it
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			Z_Free(currentthinker);
		}
		else if(currentthinker->function)
		{
			currentthinker->function(currentthinker);
		}
		currentthinker = currentthinker->next;
	}
}

//==========================================================================
//
// P_InitThinkers
//
//==========================================================================

void P_InitThinkers(void)
{
	thinkercap.prev = thinkercap.next  = &thinkercap;
}

//==========================================================================
//
// P_AddThinker
//
// Adds a new thinker at the end of the list.
//
//==========================================================================

void P_AddThinker(thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

//==========================================================================
//
// P_RemoveThinker
//
// Deallocation is lazy -- it will not actually be freed until its
// thinking turn comes up.
//
//==========================================================================

void P_RemoveThinker(thinker_t *thinker)
{
	thinker->function = (think_t)-1;
}
