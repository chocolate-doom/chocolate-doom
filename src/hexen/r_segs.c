
//**************************************************************************
//**
//** r_segs.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: r_segs.c,v $
//** $Revision: 1.4 $
//** $Date: 95/08/28 17:02:44 $
//** $Author: cjr $
//**
//** This version has the tall-sector-crossing-precision-bug fixed.
//**
//**************************************************************************

#include "h2def.h"
#include "r_local.h"

// OPTIMIZE: closed two sided lines as single sided

boolean         segtextured;    // true if any of the segs textures might be vis
boolean         markfloor;              // false if the back side is the same plane
boolean         markceiling;
boolean         maskedtexture;
int                     toptexture, bottomtexture, midtexture;


angle_t         rw_normalangle;
int                     rw_angle1;              // angle to line origin

//
// wall
//
int                     rw_x;
int                     rw_stopx;
angle_t         rw_centerangle;
fixed_t         rw_offset;
fixed_t         rw_distance;
fixed_t         rw_scale;
fixed_t         rw_scalestep;
fixed_t         rw_midtexturemid;
fixed_t         rw_toptexturemid;
fixed_t         rw_bottomtexturemid;

int                     worldtop, worldbottom, worldhigh, worldlow;

fixed_t         pixhigh, pixlow;
fixed_t         pixhighstep, pixlowstep;
fixed_t         topfrac, topstep;
fixed_t         bottomfrac, bottomstep;


lighttable_t    **walllights;

short           *maskedtexturecol;

/*
================
=
= R_RenderMaskedSegRange
=
================
*/

void R_RenderMaskedSegRange (drawseg_t *ds, int x1, int x2)
{
	unsigned        index;
	column_t        *col;
	int                     lightnum;
	int                     texnum;

//
// calculate light table
// use different light tables for horizontal / vertical / diagonal
// OPTIMIZE: get rid of LIGHTSEGSHIFT globally
	curline = ds->curline;
	frontsector = curline->frontsector;
	backsector = curline->backsector;
	texnum = texturetranslation[curline->sidedef->midtexture];

	lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT)+extralight;
	//if (curline->v1->y == curline->v2->y)
	//	lightnum--;
	//else if (curline->v1->x == curline->v2->x)
	//	lightnum++;
	//if (lightnum < 0)
	//	walllights = scalelight[0];
	if (lightnum >= LIGHTLEVELS)
		walllights = scalelight[LIGHTLEVELS-1];
	else
		walllights = scalelight[lightnum];

	maskedtexturecol = ds->maskedtexturecol;

	rw_scalestep = ds->scalestep;
	spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
	mfloorclip = ds->sprbottomclip;
	mceilingclip = ds->sprtopclip;

//
// find positioning
//
	if (curline->linedef->flags & ML_DONTPEGBOTTOM)
	{
		dc_texturemid = frontsector->floorheight > backsector->floorheight
		? frontsector->floorheight : backsector->floorheight;
		dc_texturemid = dc_texturemid + textureheight[texnum] - viewz;
	}
	else
	{
		dc_texturemid =frontsector->ceilingheight<backsector->ceilingheight
		? frontsector->ceilingheight : backsector->ceilingheight;
		dc_texturemid = dc_texturemid - viewz;
	}
	dc_texturemid += curline->sidedef->rowoffset;

	if (fixedcolormap)
		dc_colormap = fixedcolormap;
//
// draw the columns
//
	for (dc_x = x1 ; dc_x <= x2 ; dc_x++)
	{
	// calculate lighting
		if (maskedtexturecol[dc_x] != MAXSHORT)
		{
			if (!fixedcolormap)
			{
				index = spryscale>>LIGHTSCALESHIFT;
				if (index >=  MAXLIGHTSCALE )
					index = MAXLIGHTSCALE-1;
				dc_colormap = walllights[index];
			}

			sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
			dc_iscale = 0xffffffffu / (unsigned)spryscale;

	//
	// draw the texture
	//
			col = (column_t *)(
				(byte *)R_GetColumn(texnum,maskedtexturecol[dc_x]) -3);

			R_DrawMaskedColumn (col, -1);
			maskedtexturecol[dc_x] = MAXSHORT;
		}
		spryscale += rw_scalestep;
	}

}

/*
================
=
= R_RenderSegLoop
=
= Draws zero, one, or two textures (and possibly a masked texture) for walls
= Can draw or mark the starting pixel of floor and ceiling textures
=
= CALLED: CORE LOOPING ROUTINE
================
*/

#define HEIGHTBITS      12
#define HEIGHTUNIT      (1<<HEIGHTBITS)

void R_RenderSegLoop (void)
{
	angle_t         angle;
	unsigned        index;
	int                     yl, yh, mid;
	fixed_t         texturecolumn;
	int                     top, bottom;

//      texturecolumn = 0;                              // shut up compiler warning

	for ( ; rw_x < rw_stopx ; rw_x++)
	{
//
// mark floor / ceiling areas
//
		yl = (topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;
		if (yl < ceilingclip[rw_x]+1)
			yl = ceilingclip[rw_x]+1;       // no space above wall
		if (markceiling)
		{
			top = ceilingclip[rw_x]+1;
			bottom = yl-1;
			if (bottom >= floorclip[rw_x])
				bottom = floorclip[rw_x]-1;
			if (top <= bottom)
			{
				ceilingplane->top[rw_x] = top;
				ceilingplane->bottom[rw_x] = bottom;
			}
		}

		yh = bottomfrac>>HEIGHTBITS;
		if (yh >= floorclip[rw_x])
			yh = floorclip[rw_x]-1;
		if (markfloor)
		{
			top = yh+1;
			bottom = floorclip[rw_x]-1;
			if (top <= ceilingclip[rw_x])
				top = ceilingclip[rw_x]+1;
			if (top <= bottom)
			{
				floorplane->top[rw_x] = top;
				floorplane->bottom[rw_x] = bottom;
			}
		}

//
// texturecolumn and lighting are independent of wall tiers
//
		if (segtextured)
		{
		// calculate texture offset
			angle = (rw_centerangle + xtoviewangle[rw_x])>>ANGLETOFINESHIFT;
			texturecolumn = rw_offset-FixedMul(finetangent[angle],rw_distance);
			texturecolumn >>= FRACBITS;
		// calculate lighting
			index = rw_scale>>LIGHTSCALESHIFT;
			if (index >=  MAXLIGHTSCALE )
				index = MAXLIGHTSCALE-1;
			dc_colormap = walllights[index];
			dc_x = rw_x;
			dc_iscale = 0xffffffffu / (unsigned)rw_scale;
		}

//
// draw the wall tiers
//
		if (midtexture)
		{       // single sided line
			dc_yl = yl;
			dc_yh = yh;
			dc_texturemid = rw_midtexturemid;
			dc_source = R_GetColumn(midtexture,texturecolumn);
			colfunc ();
			ceilingclip[rw_x] = viewheight;
			floorclip[rw_x] = -1;
		}
		else
		{       // two sided line
			if (toptexture)
			{       // top wall
				mid = pixhigh>>HEIGHTBITS;
				pixhigh += pixhighstep;
				if (mid >= floorclip[rw_x])
					mid = floorclip[rw_x]-1;
				if (mid >= yl)
				{
					dc_yl = yl;
					dc_yh = mid;
					dc_texturemid = rw_toptexturemid;
					dc_source = R_GetColumn(toptexture,texturecolumn);
					colfunc ();
					ceilingclip[rw_x] = mid;
				}
				else
					ceilingclip[rw_x] = yl-1;
			}
			else
			{       // no top wall
				if (markceiling)
					ceilingclip[rw_x] = yl-1;
			}

			if (bottomtexture)
			{       // bottom wall
				mid = (pixlow+HEIGHTUNIT-1)>>HEIGHTBITS;
				pixlow += pixlowstep;
				if (mid <= ceilingclip[rw_x])
					mid = ceilingclip[rw_x]+1;      // no space above wall
				if (mid <= yh)
				{
					dc_yl = mid;
					dc_yh = yh;
					dc_texturemid = rw_bottomtexturemid;
					dc_source = R_GetColumn(bottomtexture,
						 texturecolumn);
					colfunc ();
					floorclip[rw_x] = mid;
				}
				else
					floorclip[rw_x] = yh+1;
			}
			else
			{       // no bottom wall
				if (markfloor)
					floorclip[rw_x] = yh+1;
			}

			if (maskedtexture)
			{       // save texturecol for backdrawing of masked mid texture
				maskedtexturecol[rw_x] = texturecolumn;
			}
		}

		rw_scale += rw_scalestep;
		topfrac += topstep;
		bottomfrac += bottomstep;
	}

}



/*
=====================
=
= R_StoreWallRange
=
= A wall segment will be drawn between start and stop pixels (inclusive)
=
======================
*/

void R_StoreWallRange (int start, int stop)
{
	fixed_t         hyp;
	fixed_t         sineval;
	angle_t         distangle, offsetangle;
	fixed_t         vtop;
	int                     lightnum;

	if (ds_p == &drawsegs[MAXDRAWSEGS])
		return;         // don't overflow and crash

#ifdef RANGECHECK
	if (start >=viewwidth || start > stop)
		I_Error ("Bad R_RenderWallRange: %i to %i", start , stop);
#endif
#ifdef __NeXT__
	RD_DrawLine (curline);
#endif

	sidedef = curline->sidedef;
	linedef = curline->linedef;

// mark the segment as visible for auto map
	linedef->flags |= ML_MAPPED;

//
// calculate rw_distance for scale calculation
//
	rw_normalangle = curline->angle + ANG90;
	offsetangle = abs(rw_normalangle-rw_angle1);
	if (offsetangle > ANG90)
		offsetangle = ANG90;
	distangle = ANG90 - offsetangle;
	hyp = R_PointToDist (curline->v1->x, curline->v1->y);
	sineval = finesine[distangle>>ANGLETOFINESHIFT];
	rw_distance = FixedMul (hyp, sineval);


	ds_p->x1 = rw_x = start;
	ds_p->x2 = stop;
	ds_p->curline = curline;
	rw_stopx = stop+1;

//
// calculate scale at both ends and step
//
	ds_p->scale1 = rw_scale =
		R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);
	if (stop > start )
	{
		ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
		ds_p->scalestep = rw_scalestep =
			(ds_p->scale2 - rw_scale) / (stop-start);
	}
	else
	{
	//
	// try to fix the stretched line bug
	//
#if 0
		if (rw_distance < FRACUNIT/2)
		{
			fixed_t         trx,try;
			fixed_t         gxt,gyt;

			trx = curline->v1->x - viewx;
			try = curline->v1->y - viewy;

			gxt = FixedMul(trx,viewcos);
			gyt = -FixedMul(try,viewsin);
			ds_p->scale1 = FixedDiv(projection, gxt-gyt);
		}
#endif
		ds_p->scale2 = ds_p->scale1;
	}


//
// calculate texture boundaries and decide if floor / ceiling marks
// are needed
//
	worldtop = frontsector->ceilingheight - viewz;
	worldbottom = frontsector->floorheight - viewz;

	midtexture = toptexture = bottomtexture = maskedtexture = 0;
	ds_p->maskedtexturecol = NULL;

	if (!backsector)
	{
//
// single sided line
//
		midtexture = texturetranslation[sidedef->midtexture];
		// a single sided line is terminal, so it must mark ends
		markfloor = markceiling = true;
		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			vtop = frontsector->floorheight +
			 textureheight[sidedef->midtexture];
			rw_midtexturemid = vtop - viewz;        // bottom of texture at bottom
		}
		else
			rw_midtexturemid = worldtop;            // top of texture at top
		rw_midtexturemid += sidedef->rowoffset;
		ds_p->silhouette = SIL_BOTH;
		ds_p->sprtopclip = screenheightarray;
		ds_p->sprbottomclip = negonearray;
		ds_p->bsilheight = MAXINT;
		ds_p->tsilheight = MININT;
	}
	else
	{
//
// two sided line
//
		ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
		ds_p->silhouette = 0;
		if (frontsector->floorheight > backsector->floorheight)
		{
			ds_p->silhouette = SIL_BOTTOM;
			ds_p->bsilheight = frontsector->floorheight;
		}
		else if (backsector->floorheight > viewz)
		{
			ds_p->silhouette = SIL_BOTTOM;
			ds_p->bsilheight = MAXINT;
//                      ds_p->sprbottomclip = negonearray;
		}
		if (frontsector->ceilingheight < backsector->ceilingheight)
		{
			ds_p->silhouette |= SIL_TOP;
			ds_p->tsilheight = frontsector->ceilingheight;
		}
		else if (backsector->ceilingheight < viewz)
		{
			ds_p->silhouette |= SIL_TOP;
			ds_p->tsilheight = MININT;
//                      ds_p->sprtopclip = screenheightarray;
		}

		if (backsector->ceilingheight <= frontsector->floorheight)
		{
			ds_p->sprbottomclip = negonearray;
			ds_p->bsilheight = MAXINT;
			ds_p->silhouette |= SIL_BOTTOM;
		}
		if (backsector->floorheight >= frontsector->ceilingheight)
		{
			ds_p->sprtopclip = screenheightarray;
			ds_p->tsilheight = MININT;
			ds_p->silhouette |= SIL_TOP;
		}
		worldhigh = backsector->ceilingheight - viewz;
		worldlow = backsector->floorheight - viewz;

		// hack to allow height changes in outdoor areas
		if (frontsector->ceilingpic == skyflatnum
		&& backsector->ceilingpic == skyflatnum)
			worldtop = worldhigh;

		if (worldlow != worldbottom
		|| backsector->floorpic != frontsector->floorpic
		|| backsector->lightlevel != frontsector->lightlevel
		|| backsector->special != frontsector->special)
			markfloor = true;
		else
			markfloor = false;                              // same plane on both sides

		if (worldhigh != worldtop
		|| backsector->ceilingpic != frontsector->ceilingpic
		|| backsector->lightlevel != frontsector->lightlevel)
			markceiling = true;
		else
			markceiling = false;                    // same plane on both sides

		if (backsector->ceilingheight <= frontsector->floorheight
		|| backsector->floorheight >= frontsector->ceilingheight)
			markceiling = markfloor = true;         // closed door

		if (worldhigh < worldtop)
		{       // top texture
			toptexture = texturetranslation[sidedef->toptexture];
			if (linedef->flags & ML_DONTPEGTOP)
				rw_toptexturemid = worldtop;            // top of texture at top
			else
			{
				vtop = backsector->ceilingheight +
					textureheight[sidedef->toptexture];
				rw_toptexturemid = vtop - viewz;        // bottom of texture
			}
		}
		if (worldlow > worldbottom)
		{       // bottom texture
			bottomtexture = texturetranslation[sidedef->bottomtexture];
			if (linedef->flags & ML_DONTPEGBOTTOM )
			{               // bottom of texture at bottom
				rw_bottomtexturemid = worldtop;// top of texture at top
			}
			else    // top of texture at top
				rw_bottomtexturemid = worldlow;
		}
		rw_toptexturemid += sidedef->rowoffset;
		rw_bottomtexturemid += sidedef->rowoffset;

		//
		// allocate space for masked texture tables
		//
		if (sidedef->midtexture)
		{       // masked midtexture
			maskedtexture = true;
			ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
			lastopening += rw_stopx - rw_x;
		}
	}

//
// calculate rw_offset (only needed for textured lines)
//
	segtextured = midtexture | toptexture | bottomtexture | maskedtexture;

	if (segtextured)
	{
		offsetangle = rw_normalangle-rw_angle1;
		if (offsetangle > ANG180)
			offsetangle = -offsetangle;
		if (offsetangle > ANG90)
			offsetangle = ANG90;
		sineval = finesine[offsetangle >>ANGLETOFINESHIFT];
		rw_offset = FixedMul (hyp, sineval);
		if (rw_normalangle-rw_angle1 < ANG180)
			rw_offset = -rw_offset;
		rw_offset += sidedef->textureoffset + curline->offset;
		rw_centerangle = ANG90 + viewangle - rw_normalangle;

	//
	// calculate light table
	// use different light tables for horizontal / vertical / diagonal
	// OPTIMIZE: get rid of LIGHTSEGSHIFT globally
		if (!fixedcolormap)
		{
			lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT)+extralight;
			//if (curline->v1->y == curline->v2->y)
			//	lightnum--;
			//else if (curline->v1->x == curline->v2->x)
			//	lightnum++;
			//if (lightnum < 0)
			//	walllights = scalelight[0];
			if (lightnum >= LIGHTLEVELS)
				walllights = scalelight[LIGHTLEVELS-1];
			else
				walllights = scalelight[lightnum];
		}
	}


//
// if a floor / ceiling plane is on the wrong side of the view plane
// it is definately invisible and doesn't need to be marked
//
	if (frontsector->floorheight >= viewz)
		markfloor = false;                              // above view plane
	if (frontsector->ceilingheight <= viewz
	&& frontsector->ceilingpic != skyflatnum)
		markceiling = false;                    // below view plane

//
// calculate incremental stepping values for texture edges
//
	worldtop >>= 4;
	worldbottom >>= 4;

	topstep = -FixedMul (rw_scalestep, worldtop);
	topfrac = (centeryfrac>>4) - FixedMul (worldtop, rw_scale);

	bottomstep = -FixedMul (rw_scalestep,worldbottom);
	bottomfrac = (centeryfrac>>4) - FixedMul (worldbottom, rw_scale);

	if (backsector)
	{
		worldhigh >>= 4;
		worldlow >>= 4;

		if (worldhigh < worldtop)
		{
			pixhigh = (centeryfrac>>4) - FixedMul (worldhigh, rw_scale);
			pixhighstep = -FixedMul (rw_scalestep,worldhigh);
		}
		if (worldlow > worldbottom)
		{
			pixlow = (centeryfrac>>4) - FixedMul (worldlow, rw_scale);
			pixlowstep = -FixedMul (rw_scalestep,worldlow);
		}
	}

//
// render it
//
	if (markceiling)
		ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
	if (markfloor)
		floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);

	R_RenderSegLoop ();

//
// save sprite clipping info
//
	if ( ((ds_p->silhouette & SIL_TOP) || maskedtexture) && !ds_p->sprtopclip)
	{
		memcpy (lastopening, ceilingclip+start, 2*(rw_stopx-start));
		ds_p->sprtopclip = lastopening - start;
		lastopening += rw_stopx - start;
	}
	if ( ((ds_p->silhouette & SIL_BOTTOM) || maskedtexture) && !ds_p->sprbottomclip)
	{
		memcpy (lastopening, floorclip+start, 2*(rw_stopx-start));
		ds_p->sprbottomclip = lastopening - start;
		lastopening += rw_stopx - start;
	}
	if (maskedtexture && !(ds_p->silhouette&SIL_TOP))
	{
		ds_p->silhouette |= SIL_TOP;
		ds_p->tsilheight = MININT;
	}
	if (maskedtexture && !(ds_p->silhouette&SIL_BOTTOM))
	{
		ds_p->silhouette |= SIL_BOTTOM;
		ds_p->bsilheight = MAXINT;
	}
	ds_p++;
}

