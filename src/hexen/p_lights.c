
//**************************************************************************
//**
//** p_lights.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_lights.c,v $
//** $Revision: 1.2 $
//** $Date: 95/07/11 10:23:36 $
//** $Author: cjr $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"

//============================================================================
//
//	T_Light
//
//============================================================================

void T_Light(light_t *light)
{
	if(light->count)
	{
		light->count--;	
		return;
	}
	switch(light->type)
	{
		case LITE_FADE:
			light->sector->lightlevel = ((light->sector->lightlevel<<FRACBITS)
				+light->value2)>>FRACBITS;
			if(light->tics2 == 1)
			{
				if(light->sector->lightlevel >= light->value1)
				{
					light->sector->lightlevel = light->value1;
					P_RemoveThinker(&light->thinker);
				}
			}
			else if(light->sector->lightlevel <= light->value1)
			{
				light->sector->lightlevel = light->value1;
				P_RemoveThinker(&light->thinker);
			}
			break;
		case LITE_GLOW:
			light->sector->lightlevel = ((light->sector->lightlevel<<FRACBITS)
				+light->tics1)>>FRACBITS;
			if(light->tics2 == 1)
			{
				if(light->sector->lightlevel >= light->value1)
				{
					light->sector->lightlevel = light->value1;
					light->tics1 = -light->tics1;
					light->tics2 = -1; // reverse direction
				}
			}
			else if(light->sector->lightlevel <= light->value2)
			{
				light->sector->lightlevel = light->value2;
				light->tics1 = -light->tics1;
				light->tics2 = 1; // reverse direction
			}
			break;
		case LITE_FLICKER:
			if(light->sector->lightlevel == light->value1)
			{
				light->sector->lightlevel = light->value2;
				light->count = (P_Random()&7)+1;
			}
			else
			{
				light->sector->lightlevel = light->value1;
				light->count = (P_Random()&31)+1;
			}
			break;
		case LITE_STROBE:
			if(light->sector->lightlevel == light->value1)
			{
				light->sector->lightlevel = light->value2;
				light->count = light->tics2;
			}
			else
			{
				light->sector->lightlevel = light->value1;
				light->count = light->tics1;
			}
			break;
		default:
			break;
	}
}

//============================================================================
//
//	EV_SpawnLight
//
//============================================================================

boolean EV_SpawnLight(line_t *line, byte *arg, lighttype_t type)
{
	light_t *light;
	sector_t *sec;
	int secNum;	
	int arg1, arg2, arg3, arg4;
	boolean think;
	boolean rtn;
	
	arg1 = arg[1] > 255 ? 255 : arg[1];
	arg1 = arg1 < 0 ? 0 : arg1;
	arg2 = arg[2] > 255 ? 255 : arg[2];
	arg2 = arg2 < 0 ? 0 : arg2;
	arg3 = arg[3] > 255 ? 255 : arg[3];
	arg3 = arg3 < 0 ? 0 : arg3;
	arg4 = arg[4] > 255 ? 255 : arg[4];
	arg4 = arg4 < 0 ? 0 : arg4;

	secNum = -1;
	rtn = false;
	think = false;
	while((secNum = P_FindSectorFromTag(arg[0], secNum)) >= 0)
	{
		think = false;
		sec = &sectors[secNum];

		light = (light_t *)Z_Malloc(sizeof(light_t), PU_LEVSPEC, 0);
		light->type = type;
		light->sector = sec;
		light->count = 0;
		rtn = true;
		switch(type)
		{
			case LITE_RAISEBYVALUE:
				sec->lightlevel += arg1;
				if(sec->lightlevel > 255)
				{
					sec->lightlevel = 255;
				}
				break;
			case LITE_LOWERBYVALUE:
				sec->lightlevel -= arg1;
				if(sec->lightlevel < 0)
				{
					sec->lightlevel = 0;
				}
				break;
			case LITE_CHANGETOVALUE:
				sec->lightlevel = arg1;
				if(sec->lightlevel < 0)
				{
					sec->lightlevel = 0;
				}
				else if(sec->lightlevel > 255)
				{
					sec->lightlevel = 255;
				}
				break;
			case LITE_FADE:
				think = true;
				light->value1 = arg1; // destination lightlevel
				light->value2 = FixedDiv((arg1-sec->lightlevel)<<FRACBITS,
					arg2<<FRACBITS);  // delta lightlevel
				if(sec->lightlevel <= arg1)
				{
					light->tics2 = 1; // get brighter
				}
				else
				{
					light->tics2 = -1;
				}
				break;
			case LITE_GLOW:
				think = true;
				light->value1 = arg1; // upper lightlevel
				light->value2 = arg2; // lower lightlevel
				light->tics1 = FixedDiv((arg1-sec->lightlevel)<<FRACBITS,
					arg3<<FRACBITS);  // lightlevel delta
				if(sec->lightlevel <= arg1)
				{
					light->tics2 = 1; // get brighter
				}
				else
				{
					light->tics2 = -1;
				}
				break;
			case LITE_FLICKER:
				think = true;
				light->value1 = arg1; // upper lightlevel
				light->value2 = arg2; // lower lightlevel
				sec->lightlevel = light->value1;
				light->count = (P_Random()&64)+1;
				break;
			case LITE_STROBE:
				think = true;
				light->value1 = arg1; // upper lightlevel
				light->value2 = arg2; // lower lightlevel
				light->tics1 = arg3;  // upper tics
				light->tics2 = arg4;  // lower tics
				light->count = arg3;
				sec->lightlevel = light->value1;
				break;
			default:
				rtn = false;
				break;
		}
		if(think)
		{
			P_AddThinker(&light->thinker);
			light->thinker.function = T_Light;
		}
		else
		{
			Z_Free(light);
		}
	}
	return rtn;
}

//============================================================================
//
//	T_Phase
//
//============================================================================

int PhaseTable[64] =
{
	128, 112, 96, 80, 64, 48, 32, 32,
	16, 16, 16, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 16, 16, 16,
	32, 32, 48, 64, 80, 96, 112, 128
};

void T_Phase(phase_t *phase)
{
	phase->index = (phase->index+1)&63;
	phase->sector->lightlevel = phase->base+PhaseTable[phase->index];
}

//==========================================================================
//
// P_SpawnPhasedLight
//
//==========================================================================

void P_SpawnPhasedLight(sector_t *sector, int base, int index)
{
	phase_t	*phase;

	phase = Z_Malloc(sizeof(*phase), PU_LEVSPEC, 0);
	P_AddThinker(&phase->thinker);
	phase->sector = sector;
	if(index == -1)
	{ // sector->lightlevel as the index
		phase->index = sector->lightlevel&63;
	}
	else
	{
		phase->index = index&63;
	}
	phase->base = base&255;
	sector->lightlevel = phase->base+PhaseTable[phase->index];
	phase->thinker.function = T_Phase;

	sector->special = 0;
}

//==========================================================================
//
// P_SpawnLightSequence
//
//==========================================================================

void P_SpawnLightSequence(sector_t *sector, int indexStep)
{
	sector_t *sec;
	sector_t *nextSec;
	sector_t *tempSec;
	int seqSpecial;
	int i;
	int count;
	fixed_t index;
	fixed_t indexDelta;
	int base;

	seqSpecial = LIGHT_SEQUENCE; // look for Light_Sequence, first
	sec = sector;
	count = 1;
	do
	{
		nextSec = NULL;
		sec->special = LIGHT_SEQUENCE_START; // make sure that the search doesn't back up.
		for(i = 0; i < sec->linecount; i++)
		{
			tempSec = getNextSector(sec->lines[i], sec);
			if(!tempSec)
			{
				continue;
			}
			if(tempSec->special == seqSpecial)
			{
				if(seqSpecial == LIGHT_SEQUENCE)
				{
					seqSpecial = LIGHT_SEQUENCE_ALT;
				}
				else
				{
					seqSpecial = LIGHT_SEQUENCE;
				}
				nextSec = tempSec;
				count++;
			}
		}
		sec = nextSec;
	} while(sec);

	sec = sector;
	count *= indexStep;
	index = 0;
	indexDelta = FixedDiv(64*FRACUNIT, count*FRACUNIT);
	base = sector->lightlevel;
	do
	{
		nextSec = NULL;
		if(sec->lightlevel)
		{
			base = sec->lightlevel;
		}
		P_SpawnPhasedLight(sec, base, index>>FRACBITS);
		index += indexDelta;
		for(i = 0; i < sec->linecount; i++)
		{
			tempSec = getNextSector(sec->lines[i], sec);
			if(!tempSec)
			{
				continue;
			}
			if(tempSec->special == LIGHT_SEQUENCE_START)
			{
				nextSec = tempSec;
			}
		}
		sec = nextSec;
	} while(sec);
}