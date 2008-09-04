
//**************************************************************************
//**
//** PO_MAN.C : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: po_man.c,v $
//** $Revision: 1.22 $
//** $Date: 95/09/28 18:20:56 $
//** $Author: cjr $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define PO_MAXPOLYSEGS 64

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

boolean PO_MovePolyobj(int num, int x, int y);
boolean PO_RotatePolyobj(int num, angle_t angle);
void PO_Init(int lump);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static polyobj_t *GetPolyobj(int polyNum);
static int GetPolyobjMirror(int poly);
static void ThrustMobj(mobj_t *mobj, seg_t *seg, polyobj_t *po);
static void UpdateSegBBox(seg_t *seg);
static void RotatePt(int an, fixed_t *x, fixed_t *y, fixed_t startSpotX,
	fixed_t startSpotY);
static void UnLinkPolyobj(polyobj_t *po);
static void LinkPolyobj(polyobj_t *po);
static boolean CheckMobjBlocking(seg_t *seg, polyobj_t *po);
static void InitBlockMap(void);
static void IterFindPolySegs(int x, int y, seg_t **segList);
static void SpawnPolyobj(int index, int tag, boolean crush);
static void TranslateToStartSpot(int tag, int originX, int originY);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern seg_t *segs;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

polyblock_t **PolyBlockMap;
polyobj_t *polyobjs; // list of all poly-objects on the level
int po_NumPolyobjs;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int PolySegCount;
static fixed_t PolyStartX;
static fixed_t PolyStartY;

// CODE --------------------------------------------------------------------

// ===== Polyobj Event Code =====

//==========================================================================
//
// T_RotatePoly
//
//==========================================================================

void T_RotatePoly(polyevent_t *pe)
{
	int absSpeed;
	polyobj_t *poly;

	if(PO_RotatePolyobj(pe->polyobj, pe->speed))
	{
		absSpeed = abs(pe->speed);

		if(pe->dist == -1)
		{ // perpetual polyobj
			return;
		}
		pe->dist -= absSpeed;
		if(pe->dist <= 0)
		{
			poly = GetPolyobj(pe->polyobj);
			if(poly->specialdata == pe)
			{
				poly->specialdata = NULL;
			}
			SN_StopSequence((mobj_t *)&poly->startSpot);
			P_PolyobjFinished(poly->tag);
			P_RemoveThinker(&pe->thinker);
		}
		if(pe->dist < absSpeed)
		{
			pe->speed = pe->dist*(pe->speed < 0 ? -1 : 1);
		}
	}
}

//==========================================================================
//
// EV_RotatePoly
//
//==========================================================================

boolean EV_RotatePoly(line_t *line, byte *args, int direction, boolean 
	overRide)
{
	int mirror;
	int polyNum;
	polyevent_t *pe;
	polyobj_t *poly;

	polyNum = args[0];
	if(poly = GetPolyobj(polyNum))
	{
		if(poly->specialdata && !overRide)
		{ // poly is already moving
			return false;
		}
	}
	else
	{
		I_Error("EV_RotatePoly:  Invalid polyobj num: %d\n", polyNum);
	}
	pe = Z_Malloc(sizeof(polyevent_t), PU_LEVSPEC, 0);
	P_AddThinker(&pe->thinker);
	pe->thinker.function = T_RotatePoly;
	pe->polyobj = polyNum;
	if(args[2])
	{
		if(args[2] == 255)
		{
			pe->dist = -1;
		}
		else
		{
			pe->dist = args[2]*(ANGLE_90/64); // Angle
		}
	}
	else
	{
		pe->dist = ANGLE_MAX-1;
	}
	pe->speed = (args[1]*direction*(ANGLE_90/64))>>3;
	poly->specialdata = pe;
	SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
		poly->seqType);
	
	while(mirror = GetPolyobjMirror(polyNum))
	{
		poly = GetPolyobj(mirror);
		if(poly && poly->specialdata && !overRide)
		{ // mirroring poly is already in motion
			break;
		}
		pe = Z_Malloc(sizeof(polyevent_t), PU_LEVSPEC, 0);
		P_AddThinker(&pe->thinker);
		pe->thinker.function = T_RotatePoly;
		poly->specialdata = pe;
		pe->polyobj = mirror;
		if(args[2])
		{
			if(args[2] == 255)
			{
				pe->dist = -1;
			}
			else
			{
				pe->dist = args[2]*(ANGLE_90/64); // Angle
			}
		}
		else
		{
			pe->dist = ANGLE_MAX-1;
		}
		if(poly = GetPolyobj(polyNum))
		{
			poly->specialdata = pe;
		}
		else
		{
			I_Error("EV_RotatePoly:  Invalid polyobj num: %d\n", polyNum);
		}
		direction = -direction;
		pe->speed = (args[1]*direction*(ANGLE_90/64))>>3;
		polyNum = mirror;
		SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
			poly->seqType);
	}
	return true;
}

//==========================================================================
//
// T_MovePoly
//
//==========================================================================

void T_MovePoly(polyevent_t *pe)
{
	int absSpeed;
	polyobj_t *poly;

	if(PO_MovePolyobj(pe->polyobj, pe->xSpeed, pe->ySpeed))
	{
		absSpeed = abs(pe->speed);
		pe->dist -= absSpeed;
		if(pe->dist <= 0)
		{
			poly = GetPolyobj(pe->polyobj);
			if(poly->specialdata == pe)
			{
				poly->specialdata = NULL;
			}
			SN_StopSequence((mobj_t *)&poly->startSpot);
			P_PolyobjFinished(poly->tag);
			P_RemoveThinker(&pe->thinker);
		}
		if(pe->dist < absSpeed)
		{
			pe->speed = pe->dist*(pe->speed < 0 ? -1 : 1);
			pe->xSpeed = FixedMul(pe->speed, finecosine[pe->angle]);
			pe->ySpeed = FixedMul(pe->speed, finesine[pe->angle]);
		}
	}
}

//==========================================================================
//
// EV_MovePoly
//
//==========================================================================

boolean EV_MovePoly(line_t *line, byte *args, boolean timesEight, boolean 
	overRide)
{
	int mirror;
	int polyNum;
	polyevent_t *pe;
	polyobj_t *poly;
	angle_t an;

	polyNum = args[0];
	if(poly = GetPolyobj(polyNum))
	{
		if(poly->specialdata && !overRide)
		{ // poly is already moving
			return false;
		}
	}
	else
	{
		I_Error("EV_MovePoly:  Invalid polyobj num: %d\n", polyNum);
	}
	pe = Z_Malloc(sizeof(polyevent_t), PU_LEVSPEC, 0);
	P_AddThinker(&pe->thinker);
	pe->thinker.function = T_MovePoly;
	pe->polyobj = polyNum;
	if(timesEight)
	{
		pe->dist = args[3]*8*FRACUNIT;
	}
	else
	{
		pe->dist = args[3]*FRACUNIT; // Distance
	}
	pe->speed = args[1]*(FRACUNIT/8);
	poly->specialdata = pe;

	an = args[2]*(ANGLE_90/64);

	pe->angle = an>>ANGLETOFINESHIFT;
	pe->xSpeed = FixedMul(pe->speed, finecosine[pe->angle]);
	pe->ySpeed = FixedMul(pe->speed, finesine[pe->angle]);
	SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
		poly->seqType);

	while(mirror = GetPolyobjMirror(polyNum))
	{
		poly = GetPolyobj(mirror);
		if(poly && poly->specialdata && !overRide)
		{ // mirroring poly is already in motion
			break;
		}
		pe = Z_Malloc(sizeof(polyevent_t), PU_LEVSPEC, 0);
		P_AddThinker(&pe->thinker);
		pe->thinker.function = T_MovePoly;
		pe->polyobj = mirror;
		poly->specialdata = pe;
		if(timesEight)
		{
			pe->dist = args[3]*8*FRACUNIT;
		}
		else
		{
			pe->dist = args[3]*FRACUNIT; // Distance
		}
		pe->speed = args[1]*(FRACUNIT/8);
		an = an+ANGLE_180; // reverse the angle
		pe->angle = an>>ANGLETOFINESHIFT;
		pe->xSpeed = FixedMul(pe->speed, finecosine[pe->angle]);
		pe->ySpeed = FixedMul(pe->speed, finesine[pe->angle]);
		polyNum = mirror;
		SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
			poly->seqType);
	}
	return true;
}

//==========================================================================
//
// T_PolyDoor
//
//==========================================================================

void T_PolyDoor(polydoor_t *pd)
{
	int absSpeed;
	polyobj_t *poly;

	if(pd->tics)
	{
		if(!--pd->tics)
		{
			poly = GetPolyobj(pd->polyobj);
			SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
				poly->seqType);
		}
		return;
	}
	switch(pd->type)
	{
		case PODOOR_SLIDE:
			if(PO_MovePolyobj(pd->polyobj, pd->xSpeed, pd->ySpeed))
			{
				absSpeed = abs(pd->speed);
				pd->dist -= absSpeed;
				if(pd->dist <= 0)
				{
					poly = GetPolyobj(pd->polyobj);
					SN_StopSequence((mobj_t *)&poly->startSpot);
					if(!pd->close)
					{
						pd->dist = pd->totalDist;
						pd->close = true;
						pd->tics = pd->waitTics;
						pd->direction = (ANGLE_MAX>>ANGLETOFINESHIFT)-
							pd->direction;
						pd->xSpeed = -pd->xSpeed;
						pd->ySpeed = -pd->ySpeed;					
					}
					else
					{
						if(poly->specialdata == pd)
						{
							poly->specialdata = NULL;
						}
						P_PolyobjFinished(poly->tag);
						P_RemoveThinker(&pd->thinker);
					}
				}
			}
			else
			{
				poly = GetPolyobj(pd->polyobj);
				if(poly->crush || !pd->close)
				{ // continue moving if the poly is a crusher, or is opening
					return;
				}
				else
				{ // open back up
					pd->dist = pd->totalDist-pd->dist;
					pd->direction = (ANGLE_MAX>>ANGLETOFINESHIFT)-
						pd->direction;
					pd->xSpeed = -pd->xSpeed;
					pd->ySpeed = -pd->ySpeed;
					pd->close = false;
					SN_StartSequence((mobj_t *)&poly->startSpot,
						SEQ_DOOR_STONE+poly->seqType);
				}
			}
			break;
		case PODOOR_SWING:
			if(PO_RotatePolyobj(pd->polyobj, pd->speed))
			{
				absSpeed = abs(pd->speed);
				if(pd->dist == -1)
				{ // perpetual polyobj
					return;
				}
				pd->dist -= absSpeed;
				if(pd->dist <= 0)
				{
					poly = GetPolyobj(pd->polyobj);
					SN_StopSequence((mobj_t *)&poly->startSpot);
					if(!pd->close)
					{
						pd->dist = pd->totalDist;
						pd->close = true;
						pd->tics = pd->waitTics;
						pd->speed = -pd->speed;
					}
					else
					{
						if(poly->specialdata == pd)
						{
							poly->specialdata = NULL;
						}
						P_PolyobjFinished(poly->tag);
						P_RemoveThinker(&pd->thinker);
					}
				}
			}
			else
			{
				poly = GetPolyobj(pd->polyobj);
				if(poly->crush || !pd->close)
				{ // continue moving if the poly is a crusher, or is opening
					return;
				}
				else
				{ // open back up and rewait
					pd->dist = pd->totalDist-pd->dist;
					pd->speed = -pd->speed;
					pd->close = false;
					SN_StartSequence((mobj_t *)&poly->startSpot,
						SEQ_DOOR_STONE+poly->seqType);
				}
			}			
			break;
		default:
			break;
	}
}

//==========================================================================
//
// EV_OpenPolyDoor
//
//==========================================================================

boolean EV_OpenPolyDoor(line_t *line, byte *args, podoortype_t type)
{
	int mirror;
	int polyNum;
	polydoor_t *pd;
	polyobj_t *poly;
	angle_t an;

	polyNum = args[0];
	if(poly = GetPolyobj(polyNum))
	{
		if(poly->specialdata)
		{ // poly is already moving
			return false;
		}
	}
	else
	{
		I_Error("EV_OpenPolyDoor:  Invalid polyobj num: %d\n", polyNum);
	}
	pd = Z_Malloc(sizeof(polydoor_t), PU_LEVSPEC, 0);
	memset(pd, 0, sizeof(polydoor_t));
	P_AddThinker(&pd->thinker);
	pd->thinker.function = T_PolyDoor;
	pd->type = type;
	pd->polyobj = polyNum;
	if(type == PODOOR_SLIDE)
	{
		pd->waitTics = args[4];
		pd->speed = args[1]*(FRACUNIT/8);
		pd->totalDist = args[3]*FRACUNIT; // Distance
		pd->dist = pd->totalDist;
		an = args[2]*(ANGLE_90/64);
		pd->direction = an>>ANGLETOFINESHIFT;
		pd->xSpeed = FixedMul(pd->speed, finecosine[pd->direction]);
		pd->ySpeed = FixedMul(pd->speed, finesine[pd->direction]);
		SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
			poly->seqType);
	}
	else if(type == PODOOR_SWING)
	{
		pd->waitTics = args[3];
		pd->direction = 1; // ADD:  PODOOR_SWINGL, PODOOR_SWINGR
		pd->speed = (args[1]*pd->direction*(ANGLE_90/64))>>3;
		pd->totalDist = args[2]*(ANGLE_90/64);
		pd->dist = pd->totalDist;
		SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
			poly->seqType);
	}

	poly->specialdata = pd;

	while(mirror = GetPolyobjMirror(polyNum))
	{
		poly = GetPolyobj(mirror);
		if(poly && poly->specialdata)
		{ // mirroring poly is already in motion
			break;
		}
		pd = Z_Malloc(sizeof(polydoor_t), PU_LEVSPEC, 0);
		memset(pd, 0, sizeof(polydoor_t));
		P_AddThinker(&pd->thinker);
		pd->thinker.function = T_PolyDoor;
		pd->polyobj = mirror;
		pd->type = type;
		poly->specialdata = pd;
		if(type == PODOOR_SLIDE)
		{
			pd->waitTics = args[4];
			pd->speed = args[1]*(FRACUNIT/8);
			pd->totalDist = args[3]*FRACUNIT; // Distance
			pd->dist = pd->totalDist;
			an = an+ANGLE_180; // reverse the angle
			pd->direction = an>>ANGLETOFINESHIFT;
			pd->xSpeed = FixedMul(pd->speed, finecosine[pd->direction]);
			pd->ySpeed = FixedMul(pd->speed, finesine[pd->direction]);
			SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
				poly->seqType);
		}
		else if(type == PODOOR_SWING)
		{
			pd->waitTics = args[3];
			pd->direction = -1; // ADD:  same as above
			pd->speed = (args[1]*pd->direction*(ANGLE_90/64))>>3;
			pd->totalDist = args[2]*(ANGLE_90/64);
			pd->dist = pd->totalDist;
			SN_StartSequence((mobj_t *)&poly->startSpot, SEQ_DOOR_STONE+
				poly->seqType);
		}
		polyNum = mirror;
	}
	return true;
}
	
// ===== Higher Level Poly Interface code =====

//==========================================================================
//
// GetPolyobj
//
//==========================================================================

static polyobj_t *GetPolyobj(int polyNum)
{
	int i;

	for(i = 0; i < po_NumPolyobjs; i++)
	{
		if(polyobjs[i].tag == polyNum)
		{
			return &polyobjs[i];
		}
	}
	return NULL;
}

//==========================================================================
//
// GetPolyobjMirror
//
//==========================================================================

static int GetPolyobjMirror(int poly)
{
	int i;

	for(i = 0; i < po_NumPolyobjs; i++)
	{
		if(polyobjs[i].tag == poly)
		{
			return((*polyobjs[i].segs)->linedef->arg2);
		}
	}
	return 0;
}

//==========================================================================
//
// ThrustMobj
//
//==========================================================================

static void ThrustMobj(mobj_t *mobj, seg_t *seg, polyobj_t *po)
{
	int thrustAngle;
	int thrustX;
	int thrustY;
	polyevent_t *pe;

	int force;

	if(!(mobj->flags&MF_SHOOTABLE) && !mobj->player)
	{
		return;
	}
	thrustAngle = (seg->angle-ANGLE_90)>>ANGLETOFINESHIFT;

	pe = po->specialdata;
	if(pe)
	{
		if(pe->thinker.function == T_RotatePoly)
		{
			force = pe->speed>>8;
		}
		else
		{
			force = pe->speed>>3;
		}
		if(force < FRACUNIT)
		{
			force = FRACUNIT;
		}
		else if(force > 4*FRACUNIT)
		{
			force = 4*FRACUNIT;
		}
	}
	else
	{
		force = FRACUNIT;
	}

	thrustX = FixedMul(force, finecosine[thrustAngle]);
	thrustY = FixedMul(force, finesine[thrustAngle]);
	mobj->momx += thrustX;
	mobj->momy += thrustY;
	if(po->crush)
	{
		if(!P_CheckPosition(mobj, mobj->x+thrustX, mobj->y+thrustY))
		{
			P_DamageMobj(mobj, NULL, NULL, 3);
		}
	}
}

//==========================================================================
//
// UpdateSegBBox
//
//==========================================================================

static void UpdateSegBBox(seg_t *seg)
{
	line_t *line;

	line = seg->linedef;

	if(seg->v1->x < seg->v2->x)
	{
		line->bbox[BOXLEFT] = seg->v1->x;
		line->bbox[BOXRIGHT] = seg->v2->x;
	}
	else
	{
		line->bbox[BOXLEFT] = seg->v2->x;
		line->bbox[BOXRIGHT] = seg->v1->x;
	}
	if(seg->v1->y < seg->v2->y)
	{
		line->bbox[BOXBOTTOM] = seg->v1->y;
		line->bbox[BOXTOP] = seg->v2->y;
	}
	else
	{
		line->bbox[BOXBOTTOM] = seg->v2->y;
		line->bbox[BOXTOP] = seg->v1->y;
	}

	// Update the line's slopetype
	line->dx = line->v2->x - line->v1->x;
	line->dy = line->v2->y - line->v1->y;
	if(!line->dx)
	{
		line->slopetype = ST_VERTICAL;
	}
	else if(!line->dy)
	{
		line->slopetype = ST_HORIZONTAL;
	}
	else
	{
		if(FixedDiv(line->dy, line->dx) > 0)
		{
			line->slopetype = ST_POSITIVE;
		}
		else
		{
			line->slopetype = ST_NEGATIVE;
		}
	}
}

//==========================================================================
//
// PO_MovePolyobj
//
//==========================================================================

boolean PO_MovePolyobj(int num, int x, int y)
{
	int count;
	seg_t **segList;
	seg_t **veryTempSeg;
	polyobj_t *po;
	vertex_t *prevPts;
	boolean blocked;

	if(!(po = GetPolyobj(num)))
	{
		I_Error("PO_MovePolyobj:  Invalid polyobj number: %d\n", num);
	}

	UnLinkPolyobj(po);

	segList = po->segs;
	prevPts = po->prevPts;
	blocked = false;

	validcount++;
	for(count = po->numsegs; count; count--, segList++, prevPts++)
	{
		if((*segList)->linedef->validcount != validcount)
		{
			(*segList)->linedef->bbox[BOXTOP] += y;
			(*segList)->linedef->bbox[BOXBOTTOM] += y;
			(*segList)->linedef->bbox[BOXLEFT] += x;
			(*segList)->linedef->bbox[BOXRIGHT] += x;
			(*segList)->linedef->validcount = validcount;
		}
		for(veryTempSeg = po->segs; veryTempSeg != segList;
			veryTempSeg++)
		{
			if((*veryTempSeg)->v1 == (*segList)->v1)
			{
				break;
			}
		}
		if(veryTempSeg == segList)
		{
			(*segList)->v1->x += x;
			(*segList)->v1->y += y;
		}
		(*prevPts).x += x; // previous points are unique for each seg
		(*prevPts).y += y;
	}
	segList = po->segs;
	for(count = po->numsegs; count; count--, segList++)
	{
		if(CheckMobjBlocking(*segList, po))
		{
			blocked = true;
		}
	}
	if(blocked)
	{
		count = po->numsegs;
		segList = po->segs;
		prevPts = po->prevPts;
		validcount++;
		while(count--)
		{
			if((*segList)->linedef->validcount != validcount)
			{
				(*segList)->linedef->bbox[BOXTOP] -= y;
				(*segList)->linedef->bbox[BOXBOTTOM] -= y;
				(*segList)->linedef->bbox[BOXLEFT] -= x;
				(*segList)->linedef->bbox[BOXRIGHT] -= x;
				(*segList)->linedef->validcount = validcount;
			}
			for(veryTempSeg = po->segs; veryTempSeg != segList;
				veryTempSeg++)
			{
				if((*veryTempSeg)->v1 == (*segList)->v1)
				{
					break;
				}
			}
			if(veryTempSeg == segList)
			{
				(*segList)->v1->x -= x;
				(*segList)->v1->y -= y;
			}
			(*prevPts).x -= x;
			(*prevPts).y -= y;
			segList++;
			prevPts++;
		}
		LinkPolyobj(po);
		return false;
	}
	po->startSpot.x += x;
	po->startSpot.y += y;
	LinkPolyobj(po);
	return true;
}

//==========================================================================
//
// RotatePt
//
//==========================================================================

static void RotatePt(int an, fixed_t *x, fixed_t *y, fixed_t startSpotX, fixed_t startSpotY)
{
	fixed_t trx, try;
	fixed_t gxt, gyt;

	trx = *x;
	try = *y;

	gxt = FixedMul(trx, finecosine[an]);
	gyt = FixedMul(try, finesine[an]);
	*x = (gxt-gyt)+startSpotX;

	gxt = FixedMul(trx, finesine[an]);
	gyt = FixedMul(try, finecosine[an]);
	*y = (gyt+gxt)+startSpotY;
}

//==========================================================================
//
// PO_RotatePolyobj
//
//==========================================================================

boolean PO_RotatePolyobj(int num, angle_t angle)
{
	int count;
	seg_t **segList;
	vertex_t *originalPts;
	vertex_t *prevPts;
	int an;
	polyobj_t *po;
	boolean blocked;

	if(!(po = GetPolyobj(num)))
	{
		I_Error("PO_RotatePolyobj:  Invalid polyobj number: %d\n", num);
	}
	an = (po->angle+angle)>>ANGLETOFINESHIFT;

	UnLinkPolyobj(po);

	segList = po->segs;
	originalPts = po->originalPts;
	prevPts = po->prevPts;

	for(count = po->numsegs; count; count--, segList++, originalPts++,
		prevPts++)
	{
		prevPts->x = (*segList)->v1->x;
		prevPts->y = (*segList)->v1->y;
		(*segList)->v1->x = originalPts->x;
		(*segList)->v1->y = originalPts->y;
		RotatePt(an, &(*segList)->v1->x, &(*segList)->v1->y, po->startSpot.x,
			po->startSpot.y);
	}
	segList = po->segs;
	blocked = false;
	validcount++;
	for(count = po->numsegs; count; count--, segList++)
	{
		if(CheckMobjBlocking(*segList, po))
		{
			blocked = true;
		}
		if((*segList)->linedef->validcount != validcount)
		{
			UpdateSegBBox(*segList);
			(*segList)->linedef->validcount = validcount;
		}
		(*segList)->angle += angle;
	}
	if(blocked)
	{
		segList = po->segs;
		prevPts = po->prevPts;
		for(count = po->numsegs; count; count--, segList++, prevPts++)
		{
			(*segList)->v1->x = prevPts->x;
			(*segList)->v1->y = prevPts->y;
		}
		segList = po->segs;
		validcount++;
		for(count = po->numsegs; count; count--, segList++, prevPts++)
		{
			if((*segList)->linedef->validcount != validcount)
			{
				UpdateSegBBox(*segList);
				(*segList)->linedef->validcount = validcount;
			}
			(*segList)->angle -= angle;
		}
		LinkPolyobj(po);
		return false;
	}
	po->angle += angle;
	LinkPolyobj(po);
	return true;
}

//==========================================================================
//
// UnLinkPolyobj
//
//==========================================================================

static void UnLinkPolyobj(polyobj_t *po)
{
	polyblock_t *link;
	int i, j;
	int index;

	// remove the polyobj from each blockmap section
	for(j = po->bbox[BOXBOTTOM]; j <= po->bbox[BOXTOP]; j++)
	{
		index = j*bmapwidth;
		for(i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
		{
			if(i >= 0 && i < bmapwidth && j >= 0 && j < bmapheight)
			{
				link = PolyBlockMap[index+i];
				while(link != NULL && link->polyobj != po)
				{
					link = link->next;
				}
				if(link == NULL)
				{ // polyobj not located in the link cell
					continue;
				}
				link->polyobj = NULL;
			}
		}
	}
}

//==========================================================================
//
// LinkPolyobj
//
//==========================================================================

static void LinkPolyobj(polyobj_t *po)
{
	int leftX, rightX;
	int topY, bottomY;
	seg_t **tempSeg;
	polyblock_t **link;
	polyblock_t *tempLink;
	int i, j;

	// calculate the polyobj bbox
	tempSeg = po->segs;
	rightX = leftX = (*tempSeg)->v1->x;
	topY = bottomY = (*tempSeg)->v1->y;

	for(i = 0; i < po->numsegs; i++, tempSeg++)
	{
		if((*tempSeg)->v1->x > rightX)
		{
			rightX = (*tempSeg)->v1->x;
		}
		if((*tempSeg)->v1->x < leftX)
		{
			leftX = (*tempSeg)->v1->x;
		}
		if((*tempSeg)->v1->y > topY)
		{
			topY = (*tempSeg)->v1->y;
		}
		if((*tempSeg)->v1->y < bottomY)
		{
			bottomY = (*tempSeg)->v1->y;
		}
	}
	po->bbox[BOXRIGHT] = (rightX-bmaporgx)>>MAPBLOCKSHIFT;
	po->bbox[BOXLEFT] = (leftX-bmaporgx)>>MAPBLOCKSHIFT;
	po->bbox[BOXTOP] = (topY-bmaporgy)>>MAPBLOCKSHIFT;
	po->bbox[BOXBOTTOM] = (bottomY-bmaporgy)>>MAPBLOCKSHIFT;
	// add the polyobj to each blockmap section
	for(j = po->bbox[BOXBOTTOM]*bmapwidth; j <= po->bbox[BOXTOP]*bmapwidth;
		j += bmapwidth)
	{
		for(i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
		{
			if(i >= 0 && i < bmapwidth && j >= 0 && j < bmapheight*bmapwidth)
			{
				link = &PolyBlockMap[j+i];
				if(!(*link))
				{ // Create a new link at the current block cell
					*link = Z_Malloc(sizeof(polyblock_t), PU_LEVEL, 0);
					(*link)->next = NULL;
					(*link)->prev = NULL;
					(*link)->polyobj = po;
					continue;
				}
				else
				{
					tempLink = *link;
					while(tempLink->next != NULL && tempLink->polyobj != NULL)
					{
						tempLink = tempLink->next;
					}
				}
				if(tempLink->polyobj == NULL)
				{
					tempLink->polyobj = po;
					continue;
				}
				else
				{
					tempLink->next = Z_Malloc(sizeof(polyblock_t), 
						PU_LEVEL, 0);
					tempLink->next->next = NULL;
					tempLink->next->prev = tempLink;
					tempLink->next->polyobj = po;
				}
			}
			// else, don't link the polyobj, since it's off the map
		}
	}
}

//==========================================================================
//
// CheckMobjBlocking
//
//==========================================================================

static boolean CheckMobjBlocking(seg_t *seg, polyobj_t *po)
{
	mobj_t *mobj;
	int i, j;
	int left, right, top, bottom;
	int tmbbox[4];
	line_t *ld;
	boolean blocked;

	ld = seg->linedef;

	top = (ld->bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
	bottom = (ld->bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
	left = (ld->bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
	right = (ld->bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;

	blocked = false;

	bottom = bottom < 0 ? 0 : bottom;
	bottom = bottom >= bmapheight ? bmapheight-1 : bottom;
	top = top < 0 ? 0 : top;
	top = top >= bmapheight  ? bmapheight-1 : top;
	left = left < 0 ? 0 : left;
	left = left >= bmapwidth ? bmapwidth-1 : left;
	right = right < 0 ? 0 : right;
	right = right >= bmapwidth ?  bmapwidth-1 : right;

	for(j = bottom*bmapwidth; j <= top*bmapwidth; j += bmapwidth)
	{
		for(i = left; i <= right; i++)
		{
			for(mobj = blocklinks[j+i]; mobj; mobj = mobj->bnext)
			{
				if(mobj->flags&MF_SOLID || mobj->player)
				{
					tmbbox[BOXTOP] = mobj->y+mobj->radius;
					tmbbox[BOXBOTTOM] = mobj->y-mobj->radius;
					tmbbox[BOXLEFT] = mobj->x-mobj->radius;
					tmbbox[BOXRIGHT] = mobj->x+mobj->radius;

					if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
						||      tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
						||      tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
						||      tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
					{
						continue;
					}
					if(P_BoxOnLineSide(tmbbox, ld) != -1)
					{
						continue;
					}
					ThrustMobj(mobj, seg, po);
					blocked = true;
				}
			}
		}
	}
	return blocked;
}

//==========================================================================
//
// InitBlockMap
//
//==========================================================================

static void InitBlockMap(void)
{
	int i;

	int j;
	seg_t **segList;
	int area;
	int leftX, rightX;
	int topY, bottomY;

	PolyBlockMap = Z_Malloc(bmapwidth*bmapheight*sizeof(polyblock_t *),
		PU_LEVEL, 0);
	memset(PolyBlockMap, 0, bmapwidth*bmapheight*sizeof(polyblock_t *));

	for(i = 0; i < po_NumPolyobjs; i++)
	{
		LinkPolyobj(&polyobjs[i]);

		// calculate a rough area
		// right now, working like shit...gotta fix this...
		segList = polyobjs[i].segs;
		leftX = rightX = (*segList)->v1->x;
		topY = bottomY = (*segList)->v1->y;
		for(j = 0; j < polyobjs[i].numsegs; j++, segList++)
		{
			if((*segList)->v1->x < leftX)
			{
				leftX = (*segList)->v1->x;
			}
			if((*segList)->v1->x > rightX)
			{
				rightX = (*segList)->v1->x;
			}
			if((*segList)->v1->y < bottomY)
			{
				bottomY = (*segList)->v1->y;
			}
			if((*segList)->v1->y > topY)
			{
				topY = (*segList)->v1->y;
			}
		}
		area = ((rightX>>FRACBITS)-(leftX>>FRACBITS))*
			((topY>>FRACBITS)-(bottomY>>FRACBITS));

//    fprintf(stdaux, "Area of Polyobj[%d]: %d\n", polyobjs[i].tag, area);
//    fprintf(stdaux, "\t[%d]\n[%d]\t\t[%d]\n\t[%d]\n", topY>>FRACBITS, 
//    		leftX>>FRACBITS,
//    	rightX>>FRACBITS, bottomY>>FRACBITS);
	}
}

//==========================================================================
//
// IterFindPolySegs
//
//              Passing NULL for segList will cause IterFindPolySegs to
//      count the number of segs in the polyobj
//==========================================================================

static void IterFindPolySegs(int x, int y, seg_t **segList)
{
	int i;

	if(x == PolyStartX && y == PolyStartY)
	{
		return;
	}
	for(i = 0; i < numsegs; i++)
	{
		if(segs[i].v1->x == x && segs[i].v1->y == y)
		{
			if(!segList)
			{
				PolySegCount++;
			}
			else
			{
				*segList++ = &segs[i];
			}
			IterFindPolySegs(segs[i].v2->x, segs[i].v2->y, segList);
			return;
		}
	}
	I_Error("IterFindPolySegs:  Non-closed Polyobj located.\n");
}


//==========================================================================
//
// SpawnPolyobj
//
//==========================================================================

static void SpawnPolyobj(int index, int tag, boolean crush)
{
	int i;
	int j;
	int psIndex;
	int psIndexOld;
	seg_t *polySegList[PO_MAXPOLYSEGS];

	for(i = 0; i < numsegs; i++)
	{
		if(segs[i].linedef->special == PO_LINE_START &&
			segs[i].linedef->arg1 == tag)
		{
			if(polyobjs[index].segs)
			{
				I_Error("SpawnPolyobj:  Polyobj %d already spawned.\n", tag);
			}
			segs[i].linedef->special = 0;
			segs[i].linedef->arg1 = 0;
			PolySegCount = 1;
			PolyStartX = segs[i].v1->x;
			PolyStartY = segs[i].v1->y;
			IterFindPolySegs(segs[i].v2->x, segs[i].v2->y, NULL);

			polyobjs[index].numsegs = PolySegCount;
			polyobjs[index].segs = Z_Malloc(PolySegCount*sizeof(seg_t *),
				PU_LEVEL, 0);
			*(polyobjs[index].segs) = &segs[i]; // insert the first seg
			IterFindPolySegs(segs[i].v2->x, segs[i].v2->y,
				polyobjs[index].segs+1);
			polyobjs[index].crush = crush;
			polyobjs[index].tag = tag;
			polyobjs[index].seqType = segs[i].linedef->arg3;
			if(polyobjs[index].seqType < 0 
				|| polyobjs[index].seqType >= SEQTYPE_NUMSEQ)
			{
				polyobjs[index].seqType = 0;
			}
			break;
		}
	}
	if(!polyobjs[index].segs)
	{ // didn't find a polyobj through PO_LINE_START
		psIndex = 0;
		polyobjs[index].numsegs = 0;
		for(j = 1; j < PO_MAXPOLYSEGS; j++)
		{
			psIndexOld = psIndex;
			for (i = 0; i < numsegs; i++)
			{
				if(segs[i].linedef->special == PO_LINE_EXPLICIT &&
					segs[i].linedef->arg1 == tag)
				{
					if(!segs[i].linedef->arg2)
					{
						I_Error("SpawnPolyobj:  Explicit line missing order number (probably %d) in poly %d.\n",
							j+1, tag);
					}
					if(segs[i].linedef->arg2 == j)
					{
						polySegList[psIndex] = &segs[i];
						polyobjs[index].numsegs++;
						psIndex++;
						if(psIndex > PO_MAXPOLYSEGS)
						{
							I_Error("SpawnPolyobj:  psIndex > PO_MAXPOLYSEGS\n");
						}
					}
				}
			}
			// Clear out any specials for these segs...we cannot clear them out
			// 	in the above loop, since we aren't guaranteed one seg per
			//		linedef.
			for(i = 0; i < numsegs; i++)
			{
				if(segs[i].linedef->special == PO_LINE_EXPLICIT &&
					segs[i].linedef->arg1 == tag && segs[i].linedef->arg2 == j)
				{
					segs[i].linedef->special = 0;
					segs[i].linedef->arg1 = 0;
				}
			}
			if(psIndex == psIndexOld)
			{ // Check if an explicit line order has been skipped
				// A line has been skipped if there are any more explicit
				// lines with the current tag value
				for(i = 0; i < numsegs; i++)
				{
					if(segs[i].linedef->special == PO_LINE_EXPLICIT &&
						segs[i].linedef->arg1 == tag)
					{
						I_Error("SpawnPolyobj:  Missing explicit line %d for poly %d\n",
							j, tag);
					}
				}
			}
		}
		if(polyobjs[index].numsegs)
		{
			PolySegCount = polyobjs[index].numsegs; // PolySegCount used globally
			polyobjs[index].crush = crush;
			polyobjs[index].tag = tag;
			polyobjs[index].segs = Z_Malloc(polyobjs[index].numsegs
				*sizeof(seg_t *), PU_LEVEL, 0);
			for(i = 0; i < polyobjs[index].numsegs; i++)
			{
				polyobjs[index].segs[i] = polySegList[i];
			}
			polyobjs[index].seqType = (*polyobjs[index].segs)->linedef->arg4;
		}
		// Next, change the polyobjs first line to point to a mirror
		//		if it exists
		(*polyobjs[index].segs)->linedef->arg2 =
			(*polyobjs[index].segs)->linedef->arg3;
	}
}

//==========================================================================
//
// TranslateToStartSpot
//
//==========================================================================

static void TranslateToStartSpot(int tag, int originX, int originY)
{
	seg_t **tempSeg;
	seg_t **veryTempSeg;
	vertex_t *tempPt;
	subsector_t *sub;
	polyobj_t *po;
	int deltaX;
	int deltaY;
	vertex_t avg; // used to find a polyobj's center, and hence subsector
	int i;

	po = NULL;
	for(i = 0; i < po_NumPolyobjs; i++)
	{
		if(polyobjs[i].tag == tag)
		{
			po = &polyobjs[i];
			break;
		}
	}
	if(!po)
	{ // didn't match the tag with a polyobj tag
		I_Error("TranslateToStartSpot:  Unable to match polyobj tag: %d\n",
			tag);
	}
	if(po->segs == NULL)
	{
		I_Error("TranslateToStartSpot:  Anchor point located without a StartSpot point: %d\n", tag);
	}
	po->originalPts = Z_Malloc(po->numsegs*sizeof(vertex_t), PU_LEVEL, 0);
	po->prevPts = Z_Malloc(po->numsegs*sizeof(vertex_t), PU_LEVEL, 0);
	deltaX = originX-po->startSpot.x;
	deltaY = originY-po->startSpot.y;

	tempSeg = po->segs;
	tempPt = po->originalPts;
	avg.x = 0;
	avg.y = 0;

	validcount++;
	for(i = 0; i < po->numsegs; i++, tempSeg++, tempPt++)
	{
		if((*tempSeg)->linedef->validcount != validcount)
		{
			(*tempSeg)->linedef->bbox[BOXTOP] -= deltaY;
			(*tempSeg)->linedef->bbox[BOXBOTTOM] -= deltaY;
			(*tempSeg)->linedef->bbox[BOXLEFT] -= deltaX;
			(*tempSeg)->linedef->bbox[BOXRIGHT] -= deltaX;
			(*tempSeg)->linedef->validcount = validcount;
		}
		for(veryTempSeg = po->segs; veryTempSeg != tempSeg; veryTempSeg++)
		{
			if((*veryTempSeg)->v1 == (*tempSeg)->v1)
			{
				break;
			}
		}
		if(veryTempSeg == tempSeg)
		{ // the point hasn't been translated, yet
			(*tempSeg)->v1->x -= deltaX;
			(*tempSeg)->v1->y -= deltaY;
		}
		avg.x += (*tempSeg)->v1->x>>FRACBITS;
		avg.y += (*tempSeg)->v1->y>>FRACBITS;
		// the original Pts are based off the startSpot Pt, and are
		// unique to each seg, not each linedef
		tempPt->x = (*tempSeg)->v1->x-po->startSpot.x;
		tempPt->y = (*tempSeg)->v1->y-po->startSpot.y;
	}
	avg.x /= po->numsegs;
	avg.y /= po->numsegs;
	sub = R_PointInSubsector(avg.x<<FRACBITS, avg.y<<FRACBITS);
	if(sub->poly != NULL)
	{
		I_Error("PO_TranslateToStartSpot:  Multiple polyobjs in a single subsector.\n");
	}
	sub->poly = po;
}

//==========================================================================
//
// PO_Init
//
//==========================================================================

void PO_Init(int lump)
{
	byte                    *data;
	int                             i;
	mapthing_t              *mt;
	int                             numthings;
	int polyIndex;

	polyobjs = Z_Malloc(po_NumPolyobjs*sizeof(polyobj_t), PU_LEVEL, 0);
	memset(polyobjs, 0, po_NumPolyobjs*sizeof(polyobj_t));

	data = W_CacheLumpNum(lump, PU_STATIC);
	numthings = W_LumpLength(lump)/sizeof(mapthing_t);
	mt = (mapthing_t *)data;
	polyIndex = 0; // index polyobj number
	// Find the startSpot points, and spawn each polyobj
	for (i = 0; i < numthings; i++, mt++)
	{
		mt->x = SHORT(mt->x);
		mt->y = SHORT(mt->y);
		mt->angle = SHORT(mt->angle);
		mt->type = SHORT(mt->type);

		// 3001 = no crush, 3002 = crushing
		if(mt->type == PO_SPAWN_TYPE || mt->type == PO_SPAWNCRUSH_TYPE)
		{ // Polyobj StartSpot Pt.
			polyobjs[polyIndex].startSpot.x = mt->x<<FRACBITS;
			polyobjs[polyIndex].startSpot.y = mt->y<<FRACBITS;
			SpawnPolyobj(polyIndex, mt->angle, (mt->type == PO_SPAWNCRUSH_TYPE));
			polyIndex++;
		}
	}
	mt = (mapthing_t *)data;
	for (i = 0; i < numthings; i++, mt++)
	{
		mt->x = SHORT(mt->x);
		mt->y = SHORT(mt->y);
		mt->angle = SHORT(mt->angle);
		mt->type = SHORT(mt->type);
		if(mt->type == PO_ANCHOR_TYPE)
		{ // Polyobj Anchor Pt.
			TranslateToStartSpot(mt->angle, mt->x<<FRACBITS, mt->y<<FRACBITS);
		}
	}
	Z_Free (data);
	// check for a startspot without an anchor point
	for(i = 0; i < po_NumPolyobjs; i++)
	{
		if(!polyobjs[i].originalPts)
		{
			I_Error("PO_Init:  StartSpot located without an Anchor point: %d\n",
				polyobjs[i].tag);
		}
	}
	InitBlockMap();
}

//==========================================================================
//
// PO_Busy
//
//==========================================================================

boolean PO_Busy(int polyobj)
{
	polyobj_t *poly;

	poly = GetPolyobj(polyobj);
	if(!poly->specialdata)
	{
		return false;
	}
	else
	{
		return true;
	}
}
