
//**************************************************************************
//**
//** p_setup.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_setup.c,v $
//** $Revision: 1.39 $
//** $Date: 96/03/12 12:04:11 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include <stdlib.h>
#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// MACROS ------------------------------------------------------------------

#define MAPINFO_SCRIPT_NAME "MAPINFO"
#define MCMD_SKY1 1
#define MCMD_SKY2 2
#define MCMD_LIGHTNING 3
#define MCMD_FADETABLE 4
#define MCMD_DOUBLESKY 5
#define MCMD_CLUSTER 6
#define MCMD_WARPTRANS 7
#define MCMD_NEXT 8
#define MCMD_CDTRACK 9
#define MCMD_CD_STARTTRACK 10
#define MCMD_CD_END1TRACK 11
#define MCMD_CD_END2TRACK 12
#define MCMD_CD_END3TRACK 13
#define MCMD_CD_INTERTRACK 14
#define MCMD_CD_TITLETRACK 15

#define UNKNOWN_MAP_NAME "DEVELOPMENT MAP"
#define DEFAULT_SKY_NAME "SKY1"
#define DEFAULT_SONG_LUMP "DEFSONG"
#define DEFAULT_FADE_TABLE "COLORMAP"

// TYPES -------------------------------------------------------------------

typedef struct mapInfo_s mapInfo_t;
struct mapInfo_s
{
	short cluster;
	short warpTrans;
	short nextMap;
	short cdTrack;
	char name[32];
	short sky1Texture;
	short sky2Texture;
	fixed_t sky1ScrollDelta;
	fixed_t sky2ScrollDelta;
	boolean doubleSky;
	boolean lightning;
	int fadetable;
	char songLump[10];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void P_SpawnMapThing(mapthing_t *mthing);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int QualifyMap(int map);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int MapCount;
mapthing_t deathmatchstarts[MAXDEATHMATCHSTARTS], *deathmatch_p;
mapthing_t playerstarts[MAX_PLAYER_STARTS][MAXPLAYERS];
int numvertexes;
vertex_t *vertexes;
int numsegs;
seg_t *segs;
int numsectors;
sector_t *sectors;
int numsubsectors;
subsector_t *subsectors;
int numnodes;
node_t *nodes;
int numlines;
line_t *lines;
int numsides;
side_t *sides;
short *blockmaplump; // offsets in blockmap are from here
short *blockmap;
int bmapwidth, bmapheight; // in mapblocks
fixed_t bmaporgx, bmaporgy; // origin of block map
mobj_t **blocklinks; // for thing chains
byte *rejectmatrix; // for fast sight rejection

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static mapInfo_t MapInfo[99];
static char *MapCmdNames[] =
{
	"SKY1",
	"SKY2",
	"DOUBLESKY",
	"LIGHTNING",
	"FADETABLE",
	"CLUSTER",
	"WARPTRANS",
	"NEXT",
	"CDTRACK",
	"CD_START_TRACK",
	"CD_END1_TRACK",
	"CD_END2_TRACK",
	"CD_END3_TRACK",
	"CD_INTERMISSION_TRACK",
	"CD_TITLE_TRACK",
	NULL
};
static int MapCmdIDs[] =
{
	MCMD_SKY1,
	MCMD_SKY2,
	MCMD_DOUBLESKY,
	MCMD_LIGHTNING,
	MCMD_FADETABLE,
	MCMD_CLUSTER,
	MCMD_WARPTRANS,
	MCMD_NEXT,
	MCMD_CDTRACK,
	MCMD_CD_STARTTRACK,
	MCMD_CD_END1TRACK,
	MCMD_CD_END2TRACK,
	MCMD_CD_END3TRACK,
	MCMD_CD_INTERTRACK,
	MCMD_CD_TITLETRACK
};

static int cd_NonLevelTracks[6]; // Non-level specific song cd track numbers 

// CODE --------------------------------------------------------------------

/*
=================
=
= P_LoadVertexes
=
=================
*/

void P_LoadVertexes (int lump)
{
	byte            *data;
	int                     i;
	mapvertex_t     *ml;
	vertex_t        *li;

	numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);
	vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);
	data = W_CacheLumpNum (lump,PU_STATIC);

	ml = (mapvertex_t *)data;
	li = vertexes;
	for (i=0 ; i<numvertexes ; i++, li++, ml++)
	{
		li->x = SHORT(ml->x)<<FRACBITS;
		li->y = SHORT(ml->y)<<FRACBITS;
	}

	Z_Free (data);
}


/*
=================
=
= P_LoadSegs
=
=================
*/

void P_LoadSegs (int lump)
{
	byte            *data;
	int                     i;
	mapseg_t        *ml;
	seg_t           *li;
	line_t  *ldef;
	int                     linedef, side;

	numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
	segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);
	memset (segs, 0, numsegs*sizeof(seg_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	ml = (mapseg_t *)data;
	li = segs;
	for (i=0 ; i<numsegs ; i++, li++, ml++)
	{
		li->v1 = &vertexes[SHORT(ml->v1)];
		li->v2 = &vertexes[SHORT(ml->v2)];

		li->angle = (SHORT(ml->angle))<<16;
		li->offset = (SHORT(ml->offset))<<16;
		linedef = SHORT(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;
		side = SHORT(ml->side);
		li->sidedef = &sides[ldef->sidenum[side]];
		li->frontsector = sides[ldef->sidenum[side]].sector;
		if (ldef-> flags & ML_TWOSIDED)
			li->backsector = sides[ldef->sidenum[side^1]].sector;
		else
			li->backsector = 0;
	}

	Z_Free (data);
}


/*
=================
=
= P_LoadSubsectors
=
=================
*/

void P_LoadSubsectors (int lump)
{
	byte                    *data;
	int                             i;
	mapsubsector_t  *ms;
	subsector_t             *ss;

	numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
	subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);
	data = W_CacheLumpNum (lump,PU_STATIC);

	ms = (mapsubsector_t *)data;
	memset (subsectors,0, numsubsectors*sizeof(subsector_t));
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
	{
		ss->numlines = SHORT(ms->numsegs);
		ss->firstline = SHORT(ms->firstseg);
	}

	Z_Free (data);
}


/*
=================
=
= P_LoadSectors
=
=================
*/

void P_LoadSectors (int lump)
{
	byte                    *data;
	int                             i;
	mapsector_t             *ms;
	sector_t                *ss;

	numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
	sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);
	memset (sectors, 0, numsectors*sizeof(sector_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	ms = (mapsector_t *)data;
	ss = sectors;

	// Make sure primary lumps are used for flat searching
	W_UsePrimary();

	for(i = 0; i < numsectors; i++, ss++, ms++)
	{
		ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
		ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
		ss->floorpic = R_FlatNumForName(ms->floorpic);
		ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
		ss->lightlevel = SHORT(ms->lightlevel);
		ss->special = SHORT(ms->special);
		ss->tag = SHORT(ms->tag);
		ss->thinglist = NULL;
		ss->seqType = SEQTYPE_STONE; // default seqType
	}
	if(DevMaps)
	{
		W_UseAuxiliary();
	}
	Z_Free(data);
}


/*
=================
=
= P_LoadNodes
=
=================
*/

void P_LoadNodes (int lump)
{
	byte            *data;
	int                     i,j,k;
	mapnode_t       *mn;
	node_t          *no;

	numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
	nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);
	data = W_CacheLumpNum (lump,PU_STATIC);

	mn = (mapnode_t *)data;
	no = nodes;
	for (i=0 ; i<numnodes ; i++, no++, mn++)
	{
		no->x = SHORT(mn->x)<<FRACBITS;
		no->y = SHORT(mn->y)<<FRACBITS;
		no->dx = SHORT(mn->dx)<<FRACBITS;
		no->dy = SHORT(mn->dy)<<FRACBITS;
		for (j=0 ; j<2 ; j++)
		{
			no->children[j] = SHORT(mn->children[j]);
			for (k=0 ; k<4 ; k++)
				no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
		}
	}
	Z_Free (data);
}

//==========================================================================
//
// P_LoadThings
//
//==========================================================================

void P_LoadThings(int lump)
{
	byte *data;
	int i;
	mapthing_t *mt;
	int numthings;
	int playerCount;
	int deathSpotsCount;

	data = W_CacheLumpNum(lump, PU_STATIC);
	numthings = W_LumpLength(lump)/sizeof(mapthing_t);

	mt = (mapthing_t *)data;
	for(i = 0; i < numthings; i++, mt++)
	{
		mt->tid = SHORT(mt->tid);
		mt->x = SHORT(mt->x);
		mt->y = SHORT(mt->y);
		mt->height = SHORT(mt->height);
		mt->angle = SHORT(mt->angle);
		mt->type = SHORT(mt->type);
		mt->options = SHORT(mt->options);
		P_SpawnMapThing(mt);
	}
	P_CreateTIDList();
	P_InitCreatureCorpseQueue(false); // false = do NOT scan for corpses
	Z_Free(data);

	if(!deathmatch)
	{ // Don't need to check deathmatch spots
		return;
	}
	playerCount = 0;
	for(i = 0; i < MAXPLAYERS; i++)
	{
		playerCount += playeringame[i];
	}
	deathSpotsCount = deathmatch_p-deathmatchstarts;
	if(deathSpotsCount < playerCount)
	{
		I_Error("P_LoadThings: Player count (%d) exceeds deathmatch "
			"spots (%d)", playerCount, deathSpotsCount);
	}
}

/*
=================
=
= P_LoadLineDefs
=
=================
*/

void P_LoadLineDefs(int lump)
{
	byte *data;
	int i;
	maplinedef_t *mld;
	line_t *ld;
	vertex_t *v1, *v2;

	numlines = W_LumpLength(lump)/sizeof(maplinedef_t);
	lines = Z_Malloc(numlines*sizeof(line_t), PU_LEVEL, 0);
	memset(lines, 0, numlines*sizeof(line_t));
	data = W_CacheLumpNum(lump, PU_STATIC);

	mld = (maplinedef_t *)data;
	ld = lines;
	for(i = 0; i < numlines; i++, mld++, ld++)
	{
		ld->flags = SHORT(mld->flags);

		// Old line special info ...
		//ld->special = SHORT(mld->special);
		//ld->tag = SHORT(mld->tag);

		// New line special info ...
		ld->special = mld->special;
		ld->arg1 = mld->arg1;
		ld->arg2 = mld->arg2;
		ld->arg3 = mld->arg3;
		ld->arg4 = mld->arg4;
		ld->arg5 = mld->arg5;

		v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
		v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
		ld->dx = v2->x - v1->x;
		ld->dy = v2->y - v1->y;
		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv (ld->dy , ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}

		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}
		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}
		ld->sidenum[0] = SHORT(mld->sidenum[0]);
		ld->sidenum[1] = SHORT(mld->sidenum[1]);
		if (ld->sidenum[0] != -1)
			ld->frontsector = sides[ld->sidenum[0]].sector;
		else
			ld->frontsector = 0;
		if (ld->sidenum[1] != -1)
			ld->backsector = sides[ld->sidenum[1]].sector;
		else
			ld->backsector = 0;
	}

	Z_Free (data);
}


/*
=================
=
= P_LoadSideDefs
=
=================
*/

void P_LoadSideDefs (int lump)
{
	byte                    *data;
	int                             i;
	mapsidedef_t    *msd;
	side_t                  *sd;

	numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
	sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);
	memset (sides, 0, numsides*sizeof(side_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	msd = (mapsidedef_t *)data;
	sd = sides;

	// Make sure primary lumps are used for texture searching
	W_UsePrimary();

	for(i = 0; i < numsides; i++, msd++, sd++)
	{
		sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
		sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		sd->midtexture = R_TextureNumForName(msd->midtexture);
		sd->sector = &sectors[SHORT(msd->sector)];
	}
	if(DevMaps)
	{
		W_UseAuxiliary();
	}
	Z_Free(data);
}

/*
=================
=
= P_LoadBlockMap
=
=================
*/

void P_LoadBlockMap (int lump)
{
	int             i, count;

	blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
	blockmap = blockmaplump+4;
	count = W_LumpLength (lump)/2;
	for (i=0 ; i<count ; i++)
		blockmaplump[i] = SHORT(blockmaplump[i]);

	bmaporgx = blockmaplump[0]<<FRACBITS;
	bmaporgy = blockmaplump[1]<<FRACBITS;
	bmapwidth = blockmaplump[2];
	bmapheight = blockmaplump[3];

// clear out mobj chains
	count = sizeof(*blocklinks)* bmapwidth*bmapheight;
	blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	memset (blocklinks, 0, count);
}




/*
=================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers
= Finds block bounding boxes for sectors
=================
*/

void P_GroupLines (void)
{
	line_t          **linebuffer;
	int                     i, j, total;
	line_t          *li;
	sector_t        *sector;
	subsector_t     *ss;
	seg_t           *seg;
	fixed_t         bbox[4];
	int                     block;

// look up sector number for each subsector
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++)
	{
		seg = &segs[ss->firstline];
		ss->sector = seg->sidedef->sector;
	}

// count number of lines in each sector
	li = lines;
	total = 0;
	for (i=0 ; i<numlines ; i++, li++)
	{
		total++;
		li->frontsector->linecount++;
		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}

// build line tables for each sector
	linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
	sector = sectors;
	for (i=0 ; i<numsectors ; i++, sector++)
	{
		M_ClearBox (bbox);
		sector->lines = linebuffer;
		li = lines;
		for (j=0 ; j<numlines ; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer++ = li;
				M_AddToBox (bbox, li->v1->x, li->v1->y);
				M_AddToBox (bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			I_Error ("P_GroupLines: miscounted");

		// set the degenmobj_t to the middle of the bounding box
		sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
		sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;

		// adjust bounding box to map blocks
		block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block >= bmapheight ? bmapheight-1 : block;
		sector->blockbox[BOXTOP]=block;

		block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXBOTTOM]=block;

		block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block >= bmapwidth ? bmapwidth-1 : block;
		sector->blockbox[BOXRIGHT]=block;

		block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXLEFT]=block;
	}

}

//=============================================================================


/*
=================
=
= P_SetupLevel
=
=================
*/

#ifdef __WATCOMC__
	extern boolean i_CDMusic;
#endif

void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
	int i;
	int parm;
	char lumpname[9];
	char auxName[128];
	int lumpnum;
	mobj_t *mobj;

	for(i = 0; i < MAXPLAYERS; i++)
	{
		players[i].killcount = players[i].secretcount
			= players[i].itemcount = 0;
	}
	players[consoleplayer].viewz = 1; // will be set by player think

#ifdef __WATCOMC__
	if(i_CDMusic == false)
	{
		S_StartSongName("chess", true); // Waiting-for-level-load song
	}
#endif

	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1);

	P_InitThinkers();
	leveltime = 0;

	if(DevMaps)
	{
		sprintf(auxName, "%sMAP%02d.WAD", DevMapsDir, map);
		W_OpenAuxiliary(auxName);
	}
	sprintf(lumpname, "MAP%02d", map);
	lumpnum = W_GetNumForName(lumpname);
	//
	// Begin processing map lumps
	// Note: most of this ordering is important
	//
	P_LoadBlockMap(lumpnum+ML_BLOCKMAP);
	P_LoadVertexes(lumpnum+ML_VERTEXES);
	P_LoadSectors(lumpnum+ML_SECTORS);
	P_LoadSideDefs(lumpnum+ML_SIDEDEFS);
	P_LoadLineDefs(lumpnum+ML_LINEDEFS);
	P_LoadSubsectors(lumpnum+ML_SSECTORS);
	P_LoadNodes(lumpnum+ML_NODES);
	P_LoadSegs(lumpnum+ML_SEGS);
	rejectmatrix = W_CacheLumpNum(lumpnum+ML_REJECT, PU_LEVEL);
	P_GroupLines();
	bodyqueslot = 0;
	po_NumPolyobjs = 0;
	deathmatch_p = deathmatchstarts;
	P_LoadThings(lumpnum+ML_THINGS);
	PO_Init(lumpnum+ML_THINGS); // Initialize the polyobjs
	P_LoadACScripts(lumpnum+ML_BEHAVIOR); // ACS object code
	//
	// End of map lump processing
	//
	if(DevMaps)
	{
		// Close the auxiliary file, but don't free its loaded lumps.
		// The next call to W_OpenAuxiliary() will do a full shutdown
		// of the current auxiliary WAD (free lumps and info lists).
		W_CloseAuxiliaryFile();
		W_UsePrimary();
	}

	// If deathmatch, randomly spawn the active players
	TimerGame = 0;
	if(deathmatch)
	{
		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (playeringame[i])
			{   // must give a player spot before deathmatchspawn
				mobj = P_SpawnMobj (playerstarts[0][i].x<<16,
					playerstarts[0][i].y<<16,0, MT_PLAYER_FIGHTER);
				players[i].mo = mobj;
				G_DeathMatchSpawnPlayer (i);
				P_RemoveMobj (mobj);
			}
		}
		parm = M_CheckParm("-timer");
		if(parm && parm < myargc-1)
		{
			TimerGame = atoi(myargv[parm+1])*35*60;
		}
	}

// set up world state
	P_SpawnSpecials ();

// build subsector connect matrix
//      P_ConnectSubsectors ();

// Load colormap and set the fullbright flag
	i = P_GetMapFadeTable(gamemap);
	W_ReadLump(i, colormaps);
	if(i == W_GetNumForName("COLORMAP"))
	{
		LevelUseFullBright = true;
	}
	else
	{ // Probably fog ... don't use fullbright sprites
		LevelUseFullBright = false;
	}

// preload graphics
	if (precache)
		R_PrecacheLevel ();

	// Check if the level is a lightning level
	P_InitLightning();

	S_StopAllSound();
	SN_StopAllSequences();
	S_StartSong(gamemap, true);

//printf ("free memory: 0x%x\n", Z_FreeMemory());

}

//==========================================================================
//
// InitMapInfo
//
//==========================================================================

static void InitMapInfo(void)
{
	int map;
	int mapMax;
	int mcmdValue;
	mapInfo_t *info;
	char songMulch[10];

	mapMax = 1;

	// Put defaults into MapInfo[0]
	info = MapInfo;
	info->cluster = 0;
	info->warpTrans = 0;
	info->nextMap = 1; // Always go to map 1 if not specified
	info->cdTrack = 1;
	info->sky1Texture = R_TextureNumForName(DEFAULT_SKY_NAME);
	info->sky2Texture = info->sky1Texture;
	info->sky1ScrollDelta = 0;
	info->sky2ScrollDelta = 0;
	info->doubleSky = false;
	info->lightning = false;
	info->fadetable = W_GetNumForName(DEFAULT_FADE_TABLE);
	strcpy(info->name, UNKNOWN_MAP_NAME);

//	strcpy(info->songLump, DEFAULT_SONG_LUMP);
	SC_Open(MAPINFO_SCRIPT_NAME);
	while(SC_GetString())
	{
		if(SC_Compare("MAP") == false)
		{
			SC_ScriptError(NULL);
		}
		SC_MustGetNumber();
		if(sc_Number < 1 || sc_Number > 99)
		{ // 
			SC_ScriptError(NULL);
		}
		map = sc_Number;

		info = &MapInfo[map];

		// Save song lump name
		strcpy(songMulch, info->songLump);

		// Copy defaults to current map definition
		memcpy(info, &MapInfo[0], sizeof(*info));

		// Restore song lump name
		strcpy(info->songLump, songMulch);

		// The warp translation defaults to the map number
		info->warpTrans = map;

		// Map name must follow the number
		SC_MustGetString();
		strcpy(info->name, sc_String);

		// Process optional tokens
		while(SC_GetString())
		{
			if(SC_Compare("MAP"))
			{ // Start next map definition
				SC_UnGet();
				break;
			}
			mcmdValue = MapCmdIDs[SC_MustMatchString(MapCmdNames)];
			switch(mcmdValue)
			{
				case MCMD_CLUSTER:
					SC_MustGetNumber();
					info->cluster = sc_Number;
					break;
				case MCMD_WARPTRANS:
					SC_MustGetNumber();
					info->warpTrans = sc_Number;
					break;
				case MCMD_NEXT:
					SC_MustGetNumber();
					info->nextMap = sc_Number;
					break;
				case MCMD_CDTRACK:
					SC_MustGetNumber();
					info->cdTrack = sc_Number;
					break;
				case MCMD_SKY1:
					SC_MustGetString();
					info->sky1Texture = R_TextureNumForName(sc_String);
					SC_MustGetNumber();
					info->sky1ScrollDelta = sc_Number<<8;
					break;
				case MCMD_SKY2:
					SC_MustGetString();
					info->sky2Texture = R_TextureNumForName(sc_String);
					SC_MustGetNumber();
					info->sky2ScrollDelta = sc_Number<<8;
					break;
				case MCMD_DOUBLESKY:
					info->doubleSky = true;
					break;
				case MCMD_LIGHTNING:
					info->lightning = true;
					break;
				case MCMD_FADETABLE:
					SC_MustGetString();
					info->fadetable = W_GetNumForName(sc_String);
					break;
				case MCMD_CD_STARTTRACK:
				case MCMD_CD_END1TRACK:
				case MCMD_CD_END2TRACK:
				case MCMD_CD_END3TRACK:
				case MCMD_CD_INTERTRACK:
				case MCMD_CD_TITLETRACK:
					SC_MustGetNumber();
					cd_NonLevelTracks[mcmdValue-MCMD_CD_STARTTRACK] = 
						sc_Number;
					break;
			}
		}
		mapMax = map > mapMax ? map : mapMax;
	}
	SC_Close();
	MapCount = mapMax;
}

//==========================================================================
//
// P_GetMapCluster
//
//==========================================================================

int P_GetMapCluster(int map)
{
	return MapInfo[QualifyMap(map)].cluster;
}

//==========================================================================
//
// P_GetMapCDTrack
//
//==========================================================================

int P_GetMapCDTrack(int map)
{
	return MapInfo[QualifyMap(map)].cdTrack;
}

//==========================================================================
//
// P_GetMapWarpTrans
//
//==========================================================================

int P_GetMapWarpTrans(int map)
{
	return MapInfo[QualifyMap(map)].warpTrans;
}

//==========================================================================
//
// P_GetMapNextMap
//
//==========================================================================

int P_GetMapNextMap(int map)
{
	return MapInfo[QualifyMap(map)].nextMap;
}

//==========================================================================
//
// P_TranslateMap
//
// Returns the actual map number given a warp map number.
//
//==========================================================================

int P_TranslateMap(int map)
{
	int i;

	for(i = 1; i < 99; i++) // Make this a macro
	{
		if(MapInfo[i].warpTrans == map)
		{
			return i;
		}
	}
	// Not found
	return -1;
}

//==========================================================================
//
// P_GetMapSky1Texture
//
//==========================================================================

int P_GetMapSky1Texture(int map)
{
	return MapInfo[QualifyMap(map)].sky1Texture;
}

//==========================================================================
//
// P_GetMapSky2Texture
//
//==========================================================================

int P_GetMapSky2Texture(int map)
{
	return MapInfo[QualifyMap(map)].sky2Texture;
}

//==========================================================================
//
// P_GetMapName
//
//==========================================================================

char *P_GetMapName(int map)
{
	return MapInfo[QualifyMap(map)].name;
}

//==========================================================================
//
// P_GetMapSky1ScrollDelta
//
//==========================================================================

fixed_t P_GetMapSky1ScrollDelta(int map)
{
	return MapInfo[QualifyMap(map)].sky1ScrollDelta;
}

//==========================================================================
//
// P_GetMapSky2ScrollDelta
//
//==========================================================================

fixed_t P_GetMapSky2ScrollDelta(int map)
{
	return MapInfo[QualifyMap(map)].sky2ScrollDelta;
}

//==========================================================================
//
// P_GetMapDoubleSky
//
//==========================================================================

boolean P_GetMapDoubleSky(int map)
{
	return MapInfo[QualifyMap(map)].doubleSky;
}

//==========================================================================
//
// P_GetMapLightning
//
//==========================================================================

boolean P_GetMapLightning(int map)
{
	return MapInfo[QualifyMap(map)].lightning;
}

//==========================================================================
//
// P_GetMapFadeTable
//
//==========================================================================

boolean P_GetMapFadeTable(int map)
{
	return MapInfo[QualifyMap(map)].fadetable;
}

//==========================================================================
//
// P_GetMapSongLump
//
//==========================================================================

char *P_GetMapSongLump(int map)
{
	if(!strcasecmp(MapInfo[QualifyMap(map)].songLump, DEFAULT_SONG_LUMP))
	{
		return NULL;
	}
	else
	{
		return MapInfo[QualifyMap(map)].songLump;
	}
}

//==========================================================================
//
// P_PutMapSongLump
//
//==========================================================================

void P_PutMapSongLump(int map, char *lumpName)
{
	if(map < 1 || map > MapCount)
	{
		return;
	}
	strcpy(MapInfo[map].songLump, lumpName);
}

//==========================================================================
//
// P_GetCDStartTrack
//
//==========================================================================

int P_GetCDStartTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_STARTTRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd1Track
//
//==========================================================================

int P_GetCDEnd1Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END1TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd2Track
//
//==========================================================================

int P_GetCDEnd2Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END2TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd3Track
//
//==========================================================================

int P_GetCDEnd3Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END3TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDIntermissionTrack
//
//==========================================================================

int P_GetCDIntermissionTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_INTERTRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDTitleTrack
//
//==========================================================================

int P_GetCDTitleTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_TITLETRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// QualifyMap
//
//==========================================================================

static int QualifyMap(int map)
{
	return (map < 1 || map > MapCount) ? 0 : map;
}

//==========================================================================
//
// P_Init
//
//==========================================================================

void P_Init(void)
{
	InitMapInfo();
	P_InitSwitchList();
	P_InitFTAnims(); // Init flat and texture animations
	P_InitTerrainTypes();
	P_InitLava();
	R_InitSprites(sprnames);
}


// Special early initializer needed to start sound before R_Init()
void InitMapMusicInfo(void)
{
	int i;

	for (i=0; i<99; i++)
	{
		strcpy(MapInfo[i].songLump, DEFAULT_SONG_LUMP);
	}
	MapCount = 98;
}

/*
void My_Debug(void)
{
	int i;

	printf("My debug stuff ----------------------\n");
	printf("gamemap=%d\n",gamemap);
	for (i=0; i<10; i++)
	{
		printf("i=%d  songlump=%s\n",i,MapInfo[i].songLump);
	}
}
*/
