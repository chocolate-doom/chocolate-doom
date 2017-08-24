//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Refresh of things, i.e. objects represented by sprites.
//




#include <stdio.h>
#include <stdlib.h>


#include "deh_main.h"
#include "doomdef.h"

#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

#include "doomstat.h"

#include "v_trans.h" // [crispy] colored blood sprites
#include "p_local.h" // [crispy] MLOOKUNIT


#define MINZ				(FRACUNIT*4)
#define BASEYCENTER			(ORIGHEIGHT/2)

//void R_DrawColumn (void);
//void R_DrawFuzzColumn (void);



typedef struct
{
    int		x1;
    int		x2;
	
    int		column;
    int		topclip;
    int		bottomclip;

} maskdraw_t;


laserspot_t laserspot_m = {0, 0, 0};
laserspot_t *laserspot = &laserspot_m;

// [crispy] extendable, but the last char element must be zero,
// keep in sync with multiitem_t multiitem_crosshairtype[] in m_menu.c
laserpatch_t laserpatch_m[] = {
	{'+', "cross1", 0, 0, 0},
	{'^', "cross2", 0, 0, 0},
	{'.', "cross3", 0, 0, 0},
	{0, "", 0, 0, 0},
};
laserpatch_t *laserpatch = laserpatch_m;

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t		pspritescale;
fixed_t		pspriteiscale;

lighttable_t**	spritelights;

// constant arrays
//  used for psprite clipping and initializing clipping
int		negonearray[SCREENWIDTH]; // [crispy] 32-bit integer math
int		screenheightarray[SCREENWIDTH]; // [crispy] 32-bit integer math


//
// INITIALIZATION FUNCTIONS
//

// variables used to look up
//  and range check thing_t sprites patches
spritedef_t*	sprites;
int		numsprites;

spriteframe_t	sprtemp[29];
int		maxframe;
char*		spritename;




//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void
R_InstallSpriteLump
( int		lump,
  unsigned	frame,
  char		rot,
  boolean	flipped )
{
    int		r;
    // [crispy] support 16 sprite rotations
    unsigned rotation = (rot >= 'A') ? rot - 'A' + 10 : (rot >= '0') ? rot - '0' : 17;
	
    if (frame >= 29 || rotation > 16) // [crispy] support 16 sprite rotations
	I_Error("R_InstallSpriteLump: "
		"Bad frame characters in lump %i", lump);
	
    if ((int)frame > maxframe)
	maxframe = frame;
		
    if (rotation == 0)
    {
	// the lump should be used for all rotations
	if (sprtemp[frame].rotate == false)
	    I_Error ("R_InitSprites: Sprite %s frame %c has "
		     "multip rot=0 lump", spritename, 'A'+frame);

	// [crispy] make non-fatal
	if (sprtemp[frame].rotate == true)
	    fprintf (stderr, "R_InitSprites: Sprite %s frame %c has rotations "
		     "and a rot=0 lump\n", spritename, 'A'+frame);
			
	sprtemp[frame].rotate = false;
	for (r=0 ; r<16 ; r++) // [crispy] support 16 sprite rotations
	{
	    sprtemp[frame].lump[r] = lump - firstspritelump;
	    sprtemp[frame].flip[r] = (byte)flipped;
	}
	return;
    }
	
    // the lump is only used for one rotation
    if (sprtemp[frame].rotate == false)
	I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
		 "and a rot=0 lump", spritename, 'A'+frame);
		
    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;		
    if (sprtemp[frame].lump[rotation] != -1)
	I_Error ("R_InitSprites: Sprite %s : %c : %c "
		 "has two lumps mapped to it",
		 spritename, 'A'+frame, '1'+rotation);
		
    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}




//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant. 
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs (char** namelist) 
{ 
    char**	check;
    int		i;
    int		l;
    int		frame;
    int		rotation;
    int		start;
    int		end;
    int		patched;
		
    // count the number of sprite names
    check = namelist;
    while (*check != NULL)
	check++;

    numsprites = check-namelist;
	
    if (!numsprites)
	return;
		
    sprites = Z_Malloc(numsprites *sizeof(*sprites), PU_STATIC, NULL);
	
    start = firstspritelump-1;
    end = lastspritelump+1;
	
    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i=0 ; i<numsprites ; i++)
    {
	spritename = DEH_String(namelist[i]);
	memset (sprtemp,-1, sizeof(sprtemp));
		
	maxframe = -1;
	
	// scan the lumps,
	//  filling in the frames for whatever is found
	for (l=start+1 ; l<end ; l++)
	{
	    if (!strncasecmp(lumpinfo[l]->name, spritename, 4))
	    {
		frame = lumpinfo[l]->name[4] - 'A';
		rotation = lumpinfo[l]->name[5];

		if (modifiedgame)
		    patched = W_GetNumForName (lumpinfo[l]->name);
		else
		    patched = l;

		R_InstallSpriteLump (patched, frame, rotation, false);

		if (lumpinfo[l]->name[6])
		{
		    frame = lumpinfo[l]->name[6] - 'A';
		    rotation = lumpinfo[l]->name[7];
		    R_InstallSpriteLump (l, frame, rotation, true);
		}
	    }
	}
	
	// check the frames that were found for completeness
	if (maxframe == -1)
	{
	    sprites[i].numframes = 0;
	    continue;
	}
		
	maxframe++;
	
	for (frame = 0 ; frame < maxframe ; frame++)
	{
	    switch ((int)sprtemp[frame].rotate)
	    {
	      case -1:
		// no rotations were found for that frame at all
		// [crispy] make non-fatal
		fprintf (stderr, "R_InitSprites: No patches found "
			 "for %s frame %c\n", spritename, frame+'A');
		break;
		
	      case 0:
		// only the first rotation is needed
		break;
			
	      case 1:
		// must have all 8 frames
		for (rotation=0 ; rotation<8 ; rotation++)
		    if (sprtemp[frame].lump[rotation] == -1)
			I_Error ("R_InitSprites: Sprite %s frame %c "
				 "is missing rotations",
				 spritename, frame+'A');

		// [crispy] support 16 sprite rotations
		sprtemp[frame].rotate = 2;
		for ( ; rotation<16 ; rotation++)
		    if (sprtemp[frame].lump[rotation] == -1)
		    {
			sprtemp[frame].rotate = 1;
			break;
		    }

		break;
	    }
	}
	
	// allocate space for the frames present and copy sprtemp to it
	sprites[i].numframes = maxframe;
	sprites[i].spriteframes = 
	    Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
	memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }

}




//
// GAME FUNCTIONS
//
vissprite_t*	vissprites = NULL;
vissprite_t*	vissprite_p;
int		newvissprite;
static int	numvissprites;



//
// R_InitSprites
// Called at program start.
//
void R_InitSprites (char** namelist)
{
    int		i;
	
    for (i=0 ; i<SCREENWIDTH ; i++)
    {
	negonearray[i] = -1;
    }
	
    R_InitSpriteDefs (namelist);
}



//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites (void)
{
    vissprite_p = vissprites;
}


//
// R_NewVisSprite
//
vissprite_t	overflowsprite;

vissprite_t* R_NewVisSprite (void)
{
    // [crispy] remove MAXVISSPRITE Vanilla limit
    if (vissprite_p == &vissprites[numvissprites])
    {
	static int max;
	int numvissprites_old = numvissprites;

	// [crispy] cap MAXVISSPRITES limit at 4096
	if (!max && numvissprites == 32 * MAXVISSPRITES)
	{
	    fprintf(stderr, "R_NewVisSprite: MAXVISSPRITES limit capped at %d.\n", numvissprites);
	    max++;
	}

	if (max)
	return &overflowsprite;

	numvissprites = numvissprites ? 2 * numvissprites : MAXVISSPRITES;
	vissprites = I_Realloc(vissprites, numvissprites * sizeof(*vissprites));
	memset(vissprites + numvissprites_old, 0, (numvissprites - numvissprites_old) * sizeof(*vissprites));

	vissprite_p = vissprites + numvissprites_old;

	if (numvissprites_old)
	    fprintf(stderr, "R_NewVisSprite: Hit MAXVISSPRITES limit at %d, raised to %d.\n", numvissprites_old, numvissprites);
    }
    
    vissprite_p++;
    return vissprite_p-1;
}



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
int*		mfloorclip; // [crispy] 32-bit integer math
int*		mceilingclip; // [crispy] 32-bit integer math

fixed_t		spryscale;
int64_t		sprtopscreen; // [crispy] WiggleFix

void R_DrawMaskedColumn (column_t* column)
{
    int64_t	topscreen; // [crispy] WiggleFix
    int64_t 	bottomscreen; // [crispy] WiggleFix
    fixed_t	basetexturemid;
	
    basetexturemid = dc_texturemid;
    dc_texheight = 0; // [crispy] Tutti-Frutti fix
	
    for ( ; column->topdelta != 0xff ; ) 
    {
	// calculate unclipped screen coordinates
	//  for post
	topscreen = sprtopscreen + spryscale*column->topdelta;
	bottomscreen = topscreen + spryscale*column->length;

	dc_yl = (int)((topscreen+FRACUNIT-1)>>FRACBITS); // [crispy] WiggleFix
	dc_yh = (int)((bottomscreen-1)>>FRACBITS); // [crispy] WiggleFix
		
	if (dc_yh >= mfloorclip[dc_x])
	    dc_yh = mfloorclip[dc_x]-1;
	if (dc_yl <= mceilingclip[dc_x])
	    dc_yl = mceilingclip[dc_x]+1;

	if (dc_yl <= dc_yh)
	{
	    dc_source = (byte *)column + 3;
	    dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);
	    // dc_source = (byte *)column + 3 - column->topdelta;

	    // Drawn by either R_DrawColumn
	    //  or (SHADOW) R_DrawFuzzColumn.
	    colfunc ();	
	}
	column = (column_t *)(  (byte *)column + column->length + 4);
    }
	
    dc_texturemid = basetexturemid;
}



//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void
R_DrawVisSprite
( vissprite_t*		vis,
  int			x1,
  int			x2 )
{
    column_t*		column;
    int			texturecolumn;
    fixed_t		frac;
    patch_t*		patch;
	
	
    patch = W_CacheLumpNum (vis->patch+firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;
    
    if (!dc_colormap)
    {
	// NULL colormap = shadow draw
	colfunc = fuzzcolfunc;
    }
    else if (vis->mobjflags & MF_TRANSLATION)
    {
	colfunc = transcolfunc;
	dc_translation = translationtables - 256 +
	    ( (vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
    }
    // [crispy] color-translated sprites (i.e. blood)
    else if (vis->translation)
    {
	colfunc = transcolfunc;
	dc_translation = vis->translation;
    }
    // [crispy] translucent sprites
    else if (crispy_translucency && vis->mobjflags & MF_TRANSLUCENT)
    {
	if (!(vis->mobjflags & (MF_NOGRAVITY | MF_COUNTITEM)) ||
	    (vis->mobjflags & MF_NOGRAVITY && crispy_translucency & TRANSLUCENCY_MISSILE) ||
	    (vis->mobjflags & MF_COUNTITEM && crispy_translucency & TRANSLUCENCY_ITEM))
	{
	    colfunc = tlcolfunc;
	}
    }
	
    dc_iscale = abs(vis->xiscale)>>(detailshift && !hires);
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);
	
    for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, frac += vis->xiscale)
    {
	static boolean error = false;
	texturecolumn = frac>>FRACBITS;
#ifdef RANGECHECK
	if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
	{
	    // [crispy] make non-fatal
	    if (!error)
	    {
	    fprintf (stderr, "R_DrawSpriteRange: bad texturecolumn\n");
	    error = true;
	    }
	    continue;
	}
#endif
	column = (column_t *) ((byte *)patch +
			       LONG(patch->columnofs[texturecolumn]));
	R_DrawMaskedColumn (column);
    }

    colfunc = basecolfunc;
}



//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite (mobj_t* thing)
{
    fixed_t		tr_x;
    fixed_t		tr_y;
    
    fixed_t		gxt;
    fixed_t		gyt;
    
    fixed_t		tx;
    fixed_t		tz;

    fixed_t		xscale;
    
    int			x1;
    int			x2;

    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    
    unsigned		rot;
    boolean		flip;
    
    int			index;

    vissprite_t*	vis;
    
    angle_t		ang;
    fixed_t		iscale;
    
    fixed_t             interpx;
    fixed_t             interpy;
    fixed_t             interpz;
    fixed_t             interpangle;

    // [AM] Interpolate between current and last position,
    //      if prudent.
    if (crispy_uncapped &&
        // Don't interpolate if the mobj did something
        // that would necessitate turning it off for a tic.
        thing->interp == true &&
        // Don't interpolate during a paused state.
        !paused && !menuactive)
    {
        interpx = thing->oldx + FixedMul(thing->x - thing->oldx, fractionaltic);
        interpy = thing->oldy + FixedMul(thing->y - thing->oldy, fractionaltic);
        interpz = thing->oldz + FixedMul(thing->z - thing->oldz, fractionaltic);
        interpangle = R_InterpolateAngle(thing->oldangle, thing->angle, fractionaltic);
    }
    else
    {
        interpx = thing->x;
        interpy = thing->y;
        interpz = thing->z;
        interpangle = thing->angle;
    }

    // transform the origin point
    tr_x = interpx - viewx;
    tr_y = interpy - viewy;
	
    gxt = FixedMul(tr_x,viewcos); 
    gyt = -FixedMul(tr_y,viewsin);
    
    tz = gxt-gyt; 

    // thing is behind view plane?
    if (tz < MINZ)
	return;
    
    xscale = FixedDiv(projection, tz);
	
    gxt = -FixedMul(tr_x,viewsin); 
    gyt = FixedMul(tr_y,viewcos); 
    tx = -(gyt+gxt); 

    // too far off the side?
    if (abs(tx)>(tz<<2))
	return;
    
    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned int) thing->sprite >= (unsigned int) numsprites)
	I_Error ("R_ProjectSprite: invalid sprite number %i ",
		 thing->sprite);
#endif
    sprdef = &sprites[thing->sprite];
    // [crispy] the TNT1 sprite is not supposed to be rendered anyway
    if (!sprdef->numframes && thing->sprite == SPR_TNT1)
    {
	return;
    }
#ifdef RANGECHECK
    if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
	I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ",
		 thing->sprite, thing->frame);
#endif
    sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
	// choose a different rotation based on player view
	ang = R_PointToAngle (interpx, interpy);
	// [crispy] now made non-fatal
	if (sprframe->rotate == -1)
	{
	    return;
	}
	else
	// [crispy] support 16 sprite rotations
	if (sprframe->rotate == 2)
	{
	    const unsigned rot2 = (ang-interpangle+(unsigned)(ANG45/4)*17);
	    rot = (rot2>>29) + ((rot2>>25)&8);
	}
	else
	{
	rot = (ang-interpangle+(unsigned)(ANG45/2)*9)>>29;
	}
	lump = sprframe->lump[rot];
	flip = (boolean)sprframe->flip[rot];
    }
    else
    {
	// use single rotation for all views
	lump = sprframe->lump[0];
	flip = (boolean)sprframe->flip[0];
    }

    // [crispy] randomly flip corpse, blood and death animation sprites
    if (crispy_flipcorpses)
    {
	flip = flip ^ thing->flipsprite;
    }
    
    // calculate edges of the shape
    // [crispy] fix sprite offsets for mirrored sprites
    tx -= flip ? spritewidth[lump] - spriteoffset[lump] : spriteoffset[lump];
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
	return;
    
    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;
    
    // store information in a vissprite
    vis = R_NewVisSprite ();
    vis->translation = NULL; // [crispy] no color translation
    vis->mobjflags = thing->flags;
    vis->scale = xscale<<(detailshift && !hires);
    vis->gx = interpx;
    vis->gy = interpy;
    vis->gz = interpz;
    vis->gzt = interpz + spritetopoffset[lump];
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    iscale = FixedDiv (FRACUNIT, xscale);

    if (flip)
    {
	vis->startfrac = spritewidth[lump]-1;
	vis->xiscale = -iscale;
    }
    else
    {
	vis->startfrac = 0;
	vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale*(vis->x1-x1);
    vis->patch = lump;
    
    // get light level
    if (thing->flags & MF_SHADOW)
    {
	// shadow draw
	vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
	// fixed map
	vis->colormap = fixedcolormap;
    }
    else if (thing->frame & FF_FULLBRIGHT)
    {
	// full bright
	vis->colormap = colormaps;
    }
    
    else
    {
	// diminished light
	index = xscale>>(LIGHTSCALESHIFT-detailshift+hires);

	if (index >= MAXLIGHTSCALE) 
	    index = MAXLIGHTSCALE-1;

	vis->colormap = spritelights[index];
    }	

    // [crispy] colored blood
    if ((((crispy_coloredblood & COLOREDBLOOD_BLOOD) && thing->type == MT_BLOOD) ||
        ((crispy_coloredblood & COLOREDBLOOD_CORPSE) && thing->sprite == SPR_POL5)) // [crispy] S_GIBS
        && thing->target)
    {
	// [crispy] Thorn Things in Hacx bleed green blood
	if (gamemission == pack_hacx)
	{
	    if (thing->target->type == MT_BABY)
	    {
		vis->translation = cr[CR_RED2GREEN];
	    }
	}
	else
	{
	    // [crispy] Barons of Hell and Hell Knights bleed green blood
	    if (thing->target->type == MT_BRUISER || thing->target->type == MT_KNIGHT)
	    {
		vis->translation = cr[CR_RED2GREEN];
	    }
	    else
	    // [crispy] Cacodemons bleed blue blood
	    if (thing->target->type == MT_HEAD)
	    {
		vis->translation = cr[CR_RED2BLUE];
	    }
	}
    }
}

// [crispy] generate a vissprite for the laser spot
static void R_DrawLSprite (void)
{
    fixed_t		xscale;
    fixed_t		tx, tz;
    vissprite_t*	vis;

    static int		lump;
    static patch_t*	patch;

    extern void	P_LineLaser (mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope);

    if (weaponinfo[viewplayer->readyweapon].ammo == am_noammo ||
        viewplayer->playerstate != PST_LIVE)
	return;

    if (lump != laserpatch[crispy_crosshairtype].l)
    {
	lump = laserpatch[crispy_crosshairtype].l;
	patch = W_CacheLumpNum(lump, PU_STATIC);
    }

    crispy_crosshair |= CROSSHAIR_INTERCEPT; // [crispy] intercepts overflow guard
    P_LineLaser(viewplayer->mo, viewangle,
                16*64*FRACUNIT, CRISPY_SLOPE(viewplayer));
    crispy_crosshair &= ~CROSSHAIR_INTERCEPT; // [crispy] intercepts overflow guard

    if (!laserspot->x &&
        !laserspot->y &&
        !laserspot->z)
	return;

    tz = FixedMul(laserspot->x - viewx, viewcos) +
         FixedMul(laserspot->y - viewy, viewsin);

    if (tz < MINZ)
	return;

    xscale = FixedDiv(projection, tz);
    // [crispy] the original patch has 5x5 pixels, cap the projection at 20x20
    xscale = (xscale > 4*FRACUNIT) ? 4*FRACUNIT : xscale;

    tx = -(FixedMul(laserspot->y - viewy, viewcos) -
           FixedMul(laserspot->x - viewx, viewsin));

    if (abs(tx) > (tz<<2))
	return;

    vis = R_NewVisSprite();
    memset(vis, 0, sizeof(*vis)); // [crispy] set all fields to NULL, except ...
    vis->patch = lump - firstspritelump; // [crispy] not a sprite patch
    vis->colormap = fixedcolormap ? fixedcolormap : colormaps; // [crispy] always full brightness
//  vis->mobjflags |= MF_TRANSLUCENT;
    vis->xiscale = FixedDiv (FRACUNIT, xscale);
    vis->texturemid = laserspot->z - viewz;
    vis->scale = xscale<<(detailshift && !hires);

    tx -= SHORT(patch->width/2)<<FRACBITS;
    vis->x1 =  (centerxfrac + FixedMul(tx, xscale))>>FRACBITS;
    tx += SHORT(patch->width)<<FRACBITS;
    vis->x2 = ((centerxfrac + FixedMul(tx, xscale))>>FRACBITS) - 1;

    if (vis->x1 < 0 || vis->x1 >= viewwidth ||
        vis->x2 < 0 || vis->x2 >= viewwidth)
	return;

    R_DrawVisSprite (vis, vis->x1, vis->x2);
}



//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites (sector_t* sec)
{
    mobj_t*		thing;
    int			lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
	return;		

    // Well, now it will be done.
    sec->validcount = validcount;
	
    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT)+extralight;

    if (lightnum < 0)		
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS-1];
    else
	spritelights = scalelight[lightnum];

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
	R_ProjectSprite (thing);
}

// [crispy] apply bobbing (or centering) to the player's weapon sprite
static inline void R_ApplyWeaponBob (fixed_t *sx, boolean bobx, fixed_t *sy, boolean boby)
{
	const angle_t angle = (128 * leveltime) & FINEMASK;

	if (sx)
	{
		*sx = FRACUNIT;

		if (bobx)
		{
			 *sx += FixedMul(viewplayer->bob, finecosine[angle]);
		}
	}

	if (sy)
	{
		*sy = 32 * FRACUNIT; // [crispy] WEAPONTOP

		if (boby)
		{
			*sy += FixedMul(viewplayer->bob, finesine[angle & (FINEANGLES / 2 - 1)]);
		}
	}
}

//
// R_DrawPSprite
//
void R_DrawPSprite (pspdef_t* psp, psprnum_t psprnum) // [crispy] differentiate gun from flash sprites
{
    fixed_t		tx;
    int			x1;
    int			x2;
    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    boolean		flip;
    vissprite_t*	vis;
    vissprite_t		avis;
    fixed_t		psp_sx = psp->sx, psp_sy = psp->sy;
    const int state = viewplayer->psprites[ps_weapon].state - states;
    
    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= (unsigned int) numsprites)
	I_Error ("R_ProjectSprite: invalid sprite number %i ",
		 psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
    // [crispy] the TNT1 sprite is not supposed to be rendered anyway
    if (!sprdef->numframes && psp->state->sprite == SPR_TNT1)
    {
	return;
    }
#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
	I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ",
		 psp->state->sprite, psp->state->frame);
#endif
    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];
    
    // [crispy] smoothen Chainsaw idle animation
    if (state == S_SAW || state == S_SAWB)
    {
        R_ApplyWeaponBob(&psp_sx, true, &psp_sy, true);
    }
    else
    // [crispy] center the weapon sprite horizontally and vertically
    if (crispy_centerweapon && viewplayer->attackdown && !psp->state->misc1)
    {
        const weaponinfo_t *const winfo = &weaponinfo[viewplayer->readyweapon];

        R_ApplyWeaponBob(&psp_sx, crispy_centerweapon == CENTERWEAPON_BOB, NULL, false);

        // [crispy] don't center vertically during lowering and raising states
        if (crispy_centerweapon >= CENTERWEAPON_HORVER &&
            state != winfo->downstate && state != winfo->upstate)
        {
            R_ApplyWeaponBob(NULL, false, &psp_sy, crispy_centerweapon == CENTERWEAPON_BOB);
        }
    }
    // calculate edges of the shape
    tx = psp_sx-(ORIGWIDTH/2)*FRACUNIT;
	
    tx -= spriteoffset[lump];	
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > viewwidth)
	return;		

    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;
    
    // store information in a vissprite
    vis = &avis;
    vis->translation = NULL; // [crispy] no color translation
    vis->mobjflags = 0;
    // [crispy] weapons drawn 1 pixel too high when player is idle
    vis->texturemid = (BASEYCENTER<<FRACBITS)+FRACUNIT/4-(psp_sy-spritetopoffset[lump]);
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    vis->scale = pspritescale<<(detailshift && !hires);
    
    if (flip)
    {
	vis->xiscale = -pspriteiscale;
	vis->startfrac = spritewidth[lump]-1;
    }
    else
    {
	vis->xiscale = pspriteiscale;
	vis->startfrac = 0;
    }
    
    // [crispy] free look
    vis->texturemid += FixedMul(((centery - viewheight / 2) << FRACBITS), vis->xiscale);

    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale*(vis->x1-x1);

    vis->patch = lump;

    if (viewplayer->powers[pw_invisibility] > 4*32
	|| viewplayer->powers[pw_invisibility] & 8)
    {
	// shadow draw
	vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
	// fixed color
	vis->colormap = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
	// full bright
	vis->colormap = colormaps;
    }
    else
    {
	// local light
	vis->colormap = spritelights[MAXLIGHTSCALE-1];
    }
	
    // [crispy] translucent gun flash sprites
    if (psprnum == ps_flash)
        vis->mobjflags |= MF_TRANSLUCENT;

    R_DrawVisSprite (vis, vis->x1, vis->x2);
}



//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites (void)
{
    int		i;
    int		lightnum;
    pspdef_t*	psp;
    
    // get light level
    lightnum =
	(viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) 
	+extralight;

    if (lightnum < 0)		
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS-1];
    else
	spritelights = scalelight[lightnum];
    
    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;
    
    if (crispy_crosshair == CROSSHAIR_PROJECTED)
	R_DrawLSprite();

    // add all active psprites
    for (i=0, psp=viewplayer->psprites;
	 i<NUMPSPRITES;
	 i++,psp++)
    {
	if (psp->state)
	    R_DrawPSprite (psp, i); // [crispy] pass gun or flash sprite
    }
}




//
// R_SortVisSprites
//
#ifdef HAVE_QSORT
// [crispy] use stdlib's qsort() function for sorting the vissprites[] array
static inline int cmp_vissprites (const void *a, const void *b)
{
    const vissprite_t *vsa = (const vissprite_t *) a;
    const vissprite_t *vsb = (const vissprite_t *) b;

    const int ret = vsa->scale - vsb->scale;

    return ret ? ret : vsa->next - vsb->next;
}

void R_SortVisSprites (void)
{
    int count;
    vissprite_t *ds;

    count = vissprite_p - vissprites;

    if (!count)
	return;

    // [crispy] maintain a stable sort for deliberately overlaid sprites
    for (ds = vissprites; ds < vissprite_p; ds++)
    {
	ds->next = ds + 1;
    }

    qsort(vissprites, count, sizeof(*vissprites), cmp_vissprites);
}
#else
vissprite_t	vsprsortedhead;


void R_SortVisSprites (void)
{
    int			i;
    int			count;
    vissprite_t*	ds;
    vissprite_t*	best;
    vissprite_t		unsorted;
    fixed_t		bestscale;

    count = vissprite_p - vissprites;
	
    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
	return;
		
    for (ds=vissprites ; ds<vissprite_p ; ds++)
    {
	ds->next = ds+1;
	ds->prev = ds-1;
    }
    
    vissprites[0].prev = &unsorted;
    unsorted.next = &vissprites[0];
    (vissprite_p-1)->next = &unsorted;
    unsorted.prev = vissprite_p-1;
    
    // pull the vissprites out by scale

    vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
    for (i=0 ; i<count ; i++)
    {
	bestscale = INT_MAX;
        best = unsorted.next;
	for (ds=unsorted.next ; ds!= &unsorted ; ds=ds->next)
	{
	    if (ds->scale < bestscale)
	    {
		bestscale = ds->scale;
		best = ds;
	    }
	}
	best->next->prev = best->prev;
	best->prev->next = best->next;
	best->next = &vsprsortedhead;
	best->prev = vsprsortedhead.prev;
	vsprsortedhead.prev->next = best;
	vsprsortedhead.prev = best;
    }
}
#endif



//
// R_DrawSprite
//
void R_DrawSprite (vissprite_t* spr)
{
    drawseg_t*		ds;
    int		clipbot[SCREENWIDTH]; // [crispy] 32-bit integer math
    int		cliptop[SCREENWIDTH]; // [crispy] 32-bit integer math
    int			x;
    int			r1;
    int			r2;
    fixed_t		scale;
    fixed_t		lowscale;
    int			silhouette;
		
    for (x = spr->x1 ; x<=spr->x2 ; x++)
	clipbot[x] = cliptop[x] = -2;
    
    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)
    {
	// determine if the drawseg obscures the sprite
	if (ds->x1 > spr->x2
	    || ds->x2 < spr->x1
	    || (!ds->silhouette
		&& !ds->maskedtexturecol) )
	{
	    // does not cover sprite
	    continue;
	}
			
	r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
	r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

	if (ds->scale1 > ds->scale2)
	{
	    lowscale = ds->scale2;
	    scale = ds->scale1;
	}
	else
	{
	    lowscale = ds->scale1;
	    scale = ds->scale2;
	}
		
	if (scale < spr->scale
	    || ( lowscale < spr->scale
		 && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline) ) )
	{
	    // masked mid texture?
	    if (ds->maskedtexturecol)	
		R_RenderMaskedSegRange (ds, r1, r2);
	    // seg is behind sprite
	    continue;			
	}

	
	// clip this piece of the sprite
	silhouette = ds->silhouette;
	
	if (spr->gz >= ds->bsilheight)
	    silhouette &= ~SIL_BOTTOM;

	if (spr->gzt <= ds->tsilheight)
	    silhouette &= ~SIL_TOP;
			
	if (silhouette == 1)
	{
	    // bottom sil
	    for (x=r1 ; x<=r2 ; x++)
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
	}
	else if (silhouette == 2)
	{
	    // top sil
	    for (x=r1 ; x<=r2 ; x++)
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	}
	else if (silhouette == 3)
	{
	    // both
	    for (x=r1 ; x<=r2 ; x++)
	    {
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	    }
	}
		
    }
    
    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
	if (clipbot[x] == -2)		
	    clipbot[x] = viewheight;

	if (cliptop[x] == -2)
	    cliptop[x] = -1;
    }
		
    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}




//
// R_DrawMasked
//
void R_DrawMasked (void)
{
    vissprite_t*	spr;
    drawseg_t*		ds;
	
    R_SortVisSprites ();

    if (vissprite_p > vissprites)
    {
	// draw all vissprites back to front
#ifdef HAVE_QSORT
	for (spr = vissprites;
	     spr < vissprite_p;
	     spr++)
#else
	for (spr = vsprsortedhead.next ;
	     spr != &vsprsortedhead ;
	     spr=spr->next)
#endif
	{
	    
	    R_DrawSprite (spr);
	}
    }
    
    // render any remaining masked mid textures
    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)
	if (ds->maskedtexturecol)
	    R_RenderMaskedSegRange (ds, ds->x1, ds->x2);
    
    if (crispy_cleanscreenshot == 2)
        return;

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset)		
	R_DrawPlayerSprites ();
}



