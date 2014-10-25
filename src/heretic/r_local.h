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
// R_local.h

#ifndef __R_LOCAL__
#define __R_LOCAL__

#include "i_video.h"
#include "v_patch.h"

#define	ANGLETOSKYSHIFT		22      // sky map is 256*128*4 maps

#define	BASEYCENTER			100

#define MAXWIDTH			1120
#define	MAXHEIGHT			832

#define	PI					3.141592657

#define	CENTERY				(SCREENHEIGHT/2)

#define	MINZ			(FRACUNIT*4)

#define	FIELDOFVIEW		2048    // fineangles in the SCREENWIDTH wide window

//
// lighting constants
//
#define	LIGHTLEVELS			16
#define	LIGHTSEGSHIFT		4
#define	MAXLIGHTSCALE		48
#define	LIGHTSCALESHIFT		12
#define	MAXLIGHTZ			128
#define	LIGHTZSHIFT			20
#define	NUMCOLORMAPS		32      // number of diminishing
#define	INVERSECOLORMAP		32

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

//================ used by play and refresh

typedef struct
{
    fixed_t x, y;
} vertex_t;

struct line_s;

typedef struct
{
    fixed_t floorheight, ceilingheight;
    short floorpic, ceilingpic;
    short lightlevel;
    short special, tag;

    int soundtraversed;         // 0 = untraversed, 1,2 = sndlines -1
    mobj_t *soundtarget;        // thing that made a sound (or null)

    int blockbox[4];            // mapblock bounding box for height changes
    degenmobj_t soundorg;       // for any sounds played by the sector

    int validcount;             // if == validcount, already checked
    mobj_t *thinglist;          // list of mobjs in sector
    void *specialdata;          // thinker_t for reversable actions
    int linecount;
    struct line_s **lines;      // [linecount] size
} sector_t;

typedef struct
{
    fixed_t textureoffset;      // add this to the calculated texture col
    fixed_t rowoffset;          // add this to the calculated texture top
    short toptexture, bottomtexture, midtexture;
    sector_t *sector;
} side_t;

typedef enum
{ ST_HORIZONTAL, ST_VERTICAL, ST_POSITIVE, ST_NEGATIVE } slopetype_t;

typedef struct line_s
{
    vertex_t *v1, *v2;
    fixed_t dx, dy;             // v2 - v1 for side checking
    short flags;
    short special, tag;
    short sidenum[2];           // sidenum[1] will be -1 if one sided
    fixed_t bbox[4];
    slopetype_t slopetype;      // to aid move clipping
    sector_t *frontsector, *backsector;
    int validcount;             // if == validcount, already checked
    void *specialdata;          // thinker_t for reversable actions
} line_t;


typedef struct subsector_s
{
    sector_t *sector;
    short numlines;
    short firstline;
} subsector_t;

typedef struct
{
    vertex_t *v1, *v2;
    fixed_t offset;
    angle_t angle;
    side_t *sidedef;
    line_t *linedef;
    sector_t *frontsector;
    sector_t *backsector;       // NULL for one sided lines
} seg_t;

typedef struct
{
    fixed_t x, y, dx, dy;       // partition line
    fixed_t bbox[2][4];         // bounding box for each child
    unsigned short children[2]; // if NF_SUBSECTOR its a subsector
} node_t;


/*
==============================================================================

						OTHER TYPES

==============================================================================
*/

typedef byte lighttable_t;      // this could be wider for >8 bit display

#define	MAXVISPLANES	128
#define	MAXOPENINGS		SCREENWIDTH*64

typedef struct
{
    fixed_t height;
    int picnum;
    int lightlevel;
    int special;
    int minx, maxx;
    byte pad1;                  // leave pads for [minx-1]/[maxx+1]
    byte top[SCREENWIDTH];
    byte pad2;
    byte pad3;
    byte bottom[SCREENWIDTH];
    byte pad4;
} visplane_t;

typedef struct drawseg_s
{
    seg_t *curline;
    int x1, x2;
    fixed_t scale1, scale2, scalestep;
    int silhouette;             // 0=none, 1=bottom, 2=top, 3=both
    fixed_t bsilheight;         // don't clip sprites above this
    fixed_t tsilheight;         // don't clip sprites below this
// pointers to lists for sprite clipping
    short *sprtopclip;          // adjusted so [x1] is first value
    short *sprbottomclip;       // adjusted so [x1] is first value
    short *maskedtexturecol;    // adjusted so [x1] is first value
} drawseg_t;

#define	SIL_NONE	0
#define	SIL_BOTTOM	1
#define SIL_TOP		2
#define	SIL_BOTH	3

#define	MAXDRAWSEGS		256

// A vissprite_t is a thing that will be drawn during a refresh
typedef struct vissprite_s
{
    struct vissprite_s *prev, *next;
    int x1, x2;
    fixed_t gx, gy;             // for line side calculation
    fixed_t gz, gzt;            // global bottom / top for silhouette clipping
    fixed_t startfrac;          // horizontal position of x1
    fixed_t scale;
    fixed_t xiscale;            // negative if flipped
    fixed_t texturemid;
    int patch;
    lighttable_t *colormap;
    int mobjflags;              // for color translation and shadow draw
    boolean psprite;            // true if psprite
    fixed_t footclip;           // foot clipping
} vissprite_t;


extern visplane_t *floorplane, *ceilingplane;

// Sprites are patches with a special naming convention so they can be 
// recognized by R_InitSprites.  The sprite and frame specified by a 
// thing_t is range checked at run time.
// a sprite is a patch_t that is assumed to represent a three dimensional
// object and may have multiple rotations pre drawn.  Horizontal flipping 
// is used to save space. Some sprites will only have one picture used
// for all views.  

typedef struct
{
    boolean rotate;             // if false use 0 for any position
    short lump[8];              // lump to use for view angles 0-7
    byte flip[8];               // flip (1 = flip) to use for view angles 0-7
} spriteframe_t;

typedef struct
{
    int numframes;
    spriteframe_t *spriteframes;
} spritedef_t;

extern spritedef_t *sprites;
extern int numsprites;

//=============================================================================

extern int numvertexes;
extern vertex_t *vertexes;

extern int numsegs;
extern seg_t *segs;

extern int numsectors;
extern sector_t *sectors;

extern int numsubsectors;
extern subsector_t *subsectors;

extern int numnodes;
extern node_t *nodes;

extern int numlines;
extern line_t *lines;

extern int numsides;
extern side_t *sides;



extern fixed_t viewx, viewy, viewz;
extern angle_t viewangle;
extern player_t *viewplayer;


extern angle_t clipangle;

extern int viewangletox[FINEANGLES / 2];
extern angle_t xtoviewangle[SCREENWIDTH + 1];

extern fixed_t rw_distance;
extern angle_t rw_normalangle;

//
// R_main.c
//
extern int viewwidth, viewheight, viewwindowx, viewwindowy;
extern int scaledviewwidth;
extern int centerx, centery;
extern int flyheight;
extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t projection;

extern int validcount;

extern int sscount, linecount, loopcount;
extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

extern int extralight;
extern lighttable_t *fixedcolormap;

extern fixed_t viewcos, viewsin;

extern int detailshift;         // 0 = high, 1 = low

extern void (*colfunc) (void);
extern void (*basecolfunc) (void);
extern void (*tlcolfunc) (void);
extern void (*spanfunc) (void);

int R_PointOnSide(fixed_t x, fixed_t y, node_t * node);
int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t * line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
fixed_t R_PointToDist(fixed_t x, fixed_t y);
fixed_t R_ScaleFromGlobalAngle(angle_t visangle);
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);
void R_AddPointToBox(int x, int y, fixed_t * box);


//
// R_bsp.c
//
extern seg_t *curline;
extern side_t *sidedef;
extern line_t *linedef;
extern sector_t *frontsector, *backsector;

extern int rw_x;
extern int rw_stopx;

extern boolean segtextured;
extern boolean markfloor;       // false if the back side is the same plane
extern boolean markceiling;
extern boolean skymap;

extern drawseg_t drawsegs[MAXDRAWSEGS], *ds_p;

extern lighttable_t **hscalelight, **vscalelight, **dscalelight;

typedef void (*drawfunc_t) (int start, int stop);
void R_ClearClipSegs(void);

void R_ClearDrawSegs(void);
void R_InitSkyMap(void);
void R_RenderBSPNode(int bspnum);

//
// R_segs.c
//
extern int rw_angle1;           // angle to line origin

void R_RenderMaskedSegRange(drawseg_t * ds, int x1, int x2);


//
// R_plane.c
//
typedef void (*planefunction_t) (int top, int bottom);
extern planefunction_t floorfunc, ceilingfunc;

extern int skyflatnum;

extern short openings[MAXOPENINGS], *lastopening;

extern short floorclip[SCREENWIDTH];
extern short ceilingclip[SCREENWIDTH];

extern fixed_t yslope[SCREENHEIGHT];
extern fixed_t distscale[SCREENWIDTH];

void R_InitPlanes(void);
void R_ClearPlanes(void);
void R_MapPlane(int y, int x1, int x2);
void R_MakeSpans(int x, int t1, int b1, int t2, int b2);
void R_DrawPlanes(void);

visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel,
                        int special);
visplane_t *R_CheckPlane(visplane_t * pl, int start, int stop);


//
// R_debug.m
//
extern int drawbsp;

//
// R_data.c
//
extern fixed_t *textureheight;  // needed for texture pegging
extern fixed_t *spritewidth;    // needed for pre rendering (fracs)
extern fixed_t *spriteoffset;
extern fixed_t *spritetopoffset;
extern lighttable_t *colormaps;
extern int firstflat;
extern int numflats;

extern int *flattranslation;    // for global animation
extern int *texturetranslation; // for global animation

extern int firstspritelump, lastspritelump, numspritelumps;

byte *R_GetColumn(int tex, int col);
void R_InitData(void);
void R_PrecacheLevel(void);


//
// R_things.c
//
#define	MAXVISSPRITES	128

extern vissprite_t vissprites[MAXVISSPRITES], *vissprite_p;
extern vissprite_t vsprsortedhead;

// constant arrays used for psprite clipping and initializing clipping
extern short negonearray[SCREENWIDTH];
extern short screenheightarray[SCREENWIDTH];

// vars for R_DrawMaskedColumn
extern short *mfloorclip;
extern short *mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t sprbotscreen;

extern fixed_t pspritescale, pspriteiscale;


void R_DrawMaskedColumn(column_t * column, signed int baseclip);


void R_SortVisSprites(void);

void R_AddSprites(sector_t * sec);
void R_AddPSprites(void);
void R_DrawSprites(void);
void R_InitSprites(char **namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);
void R_ClipVisSprite(vissprite_t * vis, int xl, int xh);

//=============================================================================
//
// R_draw.c
//
//=============================================================================

extern lighttable_t *dc_colormap;
extern int dc_x;
extern int dc_yl;
extern int dc_yh;
extern fixed_t dc_iscale;
extern fixed_t dc_texturemid;
extern byte *dc_source;         // first pixel in a column

void R_DrawColumn(void);
void R_DrawColumnLow(void);
void R_DrawTLColumn(void);
void R_DrawTLColumnLow(void);
void R_DrawTranslatedColumn(void);
void R_DrawTranslatedTLColumn(void);
void R_DrawTranslatedColumnLow(void);

extern int ds_y;
extern int ds_x1;
extern int ds_x2;
extern lighttable_t *ds_colormap;
extern fixed_t ds_xfrac;
extern fixed_t ds_yfrac;
extern fixed_t ds_xstep;
extern fixed_t ds_ystep;
extern byte *ds_source;         // start of a 64*64 tile image

extern byte *translationtables;
extern byte *dc_translation;

void R_DrawSpan(void);
void R_DrawSpanLow(void);

void R_InitBuffer(int width, int height);
void R_InitTranslationTables(void);

#endif // __R_LOCAL__
