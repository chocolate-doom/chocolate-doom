
//**************************************************************************
//**
//** r_bsp.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: r_bsp.c,v $
//** $Revision: 1.2 $
//** $Date: 95/07/13 15:17:10 $
//** $Author: cjr $
//**
//**************************************************************************

#include "h2def.h"
#include "r_local.h"

seg_t           *curline;
side_t          *sidedef;
line_t          *linedef;
sector_t        *frontsector, *backsector;

drawseg_t       drawsegs[MAXDRAWSEGS], *ds_p;

void R_StoreWallRange (int start, int stop);

/*
====================
=
= R_ClearDrawSegs
=
====================
*/

void R_ClearDrawSegs (void)
{
	ds_p = drawsegs;
}

//=============================================================================


/*
===============================================================================
=
= ClipWallSegment
=
= Clips the given range of columns and includes it in the new clip list
===============================================================================
*/

typedef struct
{
	int     first, last;
} cliprange_t;

#define MAXSEGS 32

cliprange_t     solidsegs[MAXSEGS], *newend;    // newend is one past the last valid seg


void R_ClipSolidWallSegment (int first, int last)
{
	cliprange_t     *next, *start;

// find the first range that touches the range (adjacent pixels are touching)
	start = solidsegs;
	while (start->last < first-1)
		start++;

	if (first < start->first)
	{
		if (last < start->first-1)
		{       // post is entirely visible (above start), so insert a new clippost
			R_StoreWallRange (first, last);
			next = newend;
			newend++;
			while (next != start)
			{
				*next = *(next-1);
				next--;
			}
			next->first = first;
			next->last = last;
			return;
		}

	  // there is a fragment above *start
		R_StoreWallRange (first, start->first - 1);
		start->first = first;           // adjust the clip size
	}

	if (last <= start->last)
		return;                 // bottom contained in start

	next = start;
	while (last >= (next+1)->first-1)
	{
		// there is a fragment between two posts
		R_StoreWallRange (next->last + 1, (next+1)->first - 1);
		next++;
		if (last <= next->last)
		{       // bottom is contained in next
			start->last = next->last;       // adjust the clip size
			goto crunch;
		}
	}

	// there is a fragment after *next
	R_StoreWallRange (next->last + 1, last);
	start->last = last;             // adjust the clip size


// remove start+1 to next from the clip list,
// because start now covers their area
crunch:
	if (next == start)
		return;                 // post just extended past the bottom of one post

	while (next++ != newend)        // remove a post
		*++start = *next;
	newend = start+1;
}

/*
===============================================================================
=
= R_ClipPassWallSegment
=
= Clips the given range of columns, but does not includes it in the clip list
===============================================================================
*/

void R_ClipPassWallSegment (int first, int last)
{
	cliprange_t      *start;

// find the first range that touches the range (adjacent pixels are touching)
	start = solidsegs;
	while (start->last < first-1)
		start++;

	if (first < start->first)
	{
		if (last < start->first-1)
		{       // post is entirely visible (above start)
			R_StoreWallRange (first, last);
			return;
		}

	  // there is a fragment above *start
		R_StoreWallRange (first, start->first - 1);
	}

	if (last <= start->last)
		return;                 // bottom contained in start

	while (last >= (start+1)->first-1)
	{
		// there is a fragment between two posts
		R_StoreWallRange (start->last + 1, (start+1)->first - 1);
		start++;
		if (last <= start->last)
			return;
	}

	// there is a fragment after *next
	R_StoreWallRange (start->last + 1, last);
}



/*
====================
=
= R_ClearClipSegs
=
====================
*/

void R_ClearClipSegs (void)
{
	solidsegs[0].first = -0x7fffffff;
	solidsegs[0].last = -1;
	solidsegs[1].first = viewwidth;
	solidsegs[1].last = 0x7fffffff;
	newend = solidsegs+2;
}


//=============================================================================

/*
======================
=
= R_AddLine
=
= Clips the given segment and adds any visible pieces to the line list
=
======================
*/

void R_AddLine (seg_t *line)
{
	int                     x1, x2;
	angle_t         angle1, angle2, span, tspan;

#ifdef __NeXT__
	RD_DrawLineCheck (line);
#endif
	curline = line;

// OPTIMIZE: quickly reject orthogonal back sides

	angle1 = R_PointToAngle (line->v1->x, line->v1->y);
	angle2 = R_PointToAngle (line->v2->x, line->v2->y);

//
// clip to view edges
// OPTIMIZE: make constant out of 2*clipangle (FIELDOFVIEW)
	span = angle1 - angle2;
	if (span >= ANG180)
		return;         // back side

	rw_angle1 = angle1;             // global angle needed by segcalc
	angle1 -= viewangle;
	angle2 -= viewangle;

	tspan = angle1 + clipangle;
	if (tspan > 2*clipangle)
	{
		tspan -= 2*clipangle;
		if (tspan >= span)
			return; // totally off the left edge
		angle1 = clipangle;
	}
	tspan = clipangle - angle2;
	if (tspan > 2*clipangle)
	{
		tspan -= 2*clipangle;
		if (tspan >= span)
			return; // totally off the left edge
		angle2 = -clipangle;
	}

//
// the seg is in the view range, but not necessarily visible
//
	angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
	angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
	x1 = viewangletox[angle1];
	x2 = viewangletox[angle2];
	if (x1 == x2)
		return;                         // does not cross a pixel

	backsector = line->backsector;

	if (!backsector)
		goto clipsolid;         // single sided line

	if (backsector->ceilingheight <= frontsector->floorheight
	|| backsector->floorheight >= frontsector->ceilingheight)
		goto clipsolid;         // closed door

	if (backsector->ceilingheight != frontsector->ceilingheight
	|| backsector->floorheight != frontsector->floorheight)
		goto clippass;          // window

// reject empty lines used for triggers and special events
	if (backsector->ceilingpic == frontsector->ceilingpic
	&& backsector->floorpic == frontsector->floorpic
	&& backsector->lightlevel == frontsector->lightlevel
	&& backsector->special == frontsector->special
	&& curline->sidedef->midtexture == 0)
		return;

clippass:
	R_ClipPassWallSegment (x1, x2-1);
	return;

clipsolid:
	R_ClipSolidWallSegment (x1, x2-1);
}

//============================================================================


/*
===============================================================================
=
= R_CheckBBox
=
= Returns true if some part of the bbox might be visible
=
===============================================================================
*/

int     checkcoord[12][4] = {
{3,0, 2,1},
{3,0, 2,0},
{3,1, 2,0},
{0},
{2,0, 2,1},
{0,0,0,0},
{3,1, 3,0},
{0},
{2,0, 3,1},
{2,1, 3,1},
{2,1, 3,0} };


boolean R_CheckBBox (fixed_t *bspcoord)
{
	int                     boxx, boxy, boxpos;
	fixed_t         x1, y1, x2, y2;
	angle_t         angle1, angle2, span, tspan;
	cliprange_t     *start;
	int                     sx1, sx2;

#ifdef __NeXT__
	RD_DrawBBox (bspcoord);
#endif

// find the corners of the box that define the edges from current viewpoint
	if (viewx <= bspcoord[BOXLEFT])
		boxx = 0;
	else if (viewx < bspcoord[BOXRIGHT])
		boxx = 1;
	else
		boxx = 2;

	if (viewy >= bspcoord[BOXTOP])
		boxy = 0;
	else if (viewy > bspcoord[BOXBOTTOM])
		boxy = 1;
	else
		boxy = 2;

	boxpos = (boxy<<2)+boxx;
	if (boxpos == 5)
		return true;

	x1 = bspcoord[checkcoord[boxpos][0]];
	y1 = bspcoord[checkcoord[boxpos][1]];
	x2 = bspcoord[checkcoord[boxpos][2]];
	y2 = bspcoord[checkcoord[boxpos][3]];


#ifdef __NeXT__
//      RD_DisplayLine (x1, y1, x2, y2, 0.1);
#endif

//
// check clip list for an open space
//
	angle1 = R_PointToAngle (x1, y1) - viewangle;
	angle2 = R_PointToAngle (x2, y2) - viewangle;

	span = angle1 - angle2;
	if (span >= ANG180)
		return true;    // sitting on a line
	tspan = angle1 + clipangle;
	if (tspan > 2*clipangle)
	{
		tspan -= 2*clipangle;
		if (tspan >= span)
			return false;   // totally off the left edge
		angle1 = clipangle;
	}
	tspan = clipangle - angle2;
	if (tspan > 2*clipangle)
	{
		tspan -= 2*clipangle;
		if (tspan >= span)
			return false;   // totally off the left edge
		angle2 = -clipangle;
	}


// find the first clippost that touches the source post (adjacent pixels are touching)
	angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
	angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
	sx1 = viewangletox[angle1];
	sx2 = viewangletox[angle2];
	if (sx1 == sx2)
		return false;                           // does not cross a pixel
	sx2--;

	start = solidsegs;
	while (start->last < sx2)
		start++;
	if (sx1 >= start->first && sx2 <= start->last)
		return false;   // the clippost contains the new span

	return true;
}


/*
================
=
= R_Subsector
=
= Draw one or more segments
================
*/

void R_Subsector (int num)
{
	int                     count;
	seg_t           *line;
	subsector_t     *sub;
	int polyCount;
	seg_t **polySeg;

#ifdef RANGECHECK
	if (num>=numsubsectors)
		I_Error ("R_Subsector: ss %i with numss = %i",num, numsubsectors);
#endif

	sscount++;
	sub = &subsectors[num];
	frontsector = sub->sector;
	count = sub->numlines;
	line = &segs[sub->firstline];

	if(frontsector->floorheight < viewz)
	{
		floorplane = R_FindPlane(frontsector->floorheight,
			frontsector->floorpic, frontsector->lightlevel,
			frontsector->special);
	}
	else
	{
		floorplane = NULL;
	}

	if(frontsector->ceilingheight > viewz
		|| frontsector->ceilingpic == skyflatnum)
	{
		ceilingplane = R_FindPlane(frontsector->ceilingheight,
			frontsector->ceilingpic, frontsector->lightlevel, 0);
	}
	else
	{
		ceilingplane = NULL;
	}

	R_AddSprites(frontsector);
	if(sub->poly)
	{ // Render the polyobj in the subsector first
		polyCount = sub->poly->numsegs;
		polySeg = sub->poly->segs;
		while(polyCount--)
		{
			R_AddLine(*polySeg++);
		}
	}
	while (count--)
	{
		R_AddLine (line);
		line++;
	}
}


/*
===============================================================================
=
= RenderBSPNode
=
===============================================================================
*/

void R_RenderBSPNode (int bspnum)
{
	node_t          *bsp;
	int                     side;

	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
			R_Subsector (0);
		else
			R_Subsector (bspnum&(~NF_SUBSECTOR));
		return;
	}

	bsp = &nodes[bspnum];

#ifdef __NeXT__
	RD_DrawNodeLine (bsp);
#endif

//
// decide which side the view point is on
//
	side = R_PointOnSide (viewx, viewy, bsp);

	R_RenderBSPNode (bsp->children[side]); // recursively divide front space

	if (R_CheckBBox (bsp->bbox[side^1]))    // possibly divide back space
		R_RenderBSPNode (bsp->children[side^1]);
}


