
//**************************************************************************
//**
//** p_spec.h : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_spec.h,v $
//** $Revision: 1.38 $
//** $Date: 96/01/06 18:37:35 $
//** $Author: bgokey $
//**
//**************************************************************************

extern int *TerrainTypes;

//
//      scrolling line specials
//

#define MAXLINEANIMS 64
extern short numlinespecials;
extern line_t *linespeciallist[MAXLINEANIMS];

//      Define values for map objects
#define MO_TELEPORTMAN 14

// at game start
void P_InitTerrainTypes(void);
void P_InitLava(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

// when needed
boolean P_ExecuteLineSpecial(int special, byte *args, line_t *line, int side,
	mobj_t *mo);
boolean P_ActivateLine(line_t *ld, mobj_t *mo, int side, int activationType);
//boolean P_UseSpecialLine ( mobj_t *thing, line_t *line);
//void    P_ShootSpecialLine ( mobj_t *thing, line_t *line);
//void    P_CrossSpecialLine (int linenum, int side, mobj_t *thing);

void P_PlayerInSpecialSector(player_t *player);
void P_PlayerOnSpecialFlat(player_t *player, int floorType);

//int twoSided(int sector,int line);
//sector_t *getSector(int currentSector,int line,int side);
//side_t  *getSide(int currentSector,int line, int side);
fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);
fixed_t P_FindNextHighestFloor(sector_t *sec,int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);
//int P_FindSectorFromLineTag(line_t  *line,int start);
int P_FindSectorFromTag(int tag, int start);
//int P_FindMinSurroundingLight(sector_t *sector,int max);
sector_t *getNextSector(line_t *line,sector_t *sec);
line_t *P_FindLine(int lineTag, int *searchPosition);

//
//      SPECIAL
//
//int EV_DoDonut(line_t *line);

//-------------------------------
// P_anim.c
//-------------------------------

void P_AnimateSurfaces(void);
void P_InitFTAnims(void);
void P_InitLightning(void);
void P_ForceLightning(void);

/*
===============================================================================

							P_LIGHTS

===============================================================================
*/

typedef enum
{
	LITE_RAISEBYVALUE,
	LITE_LOWERBYVALUE,
	LITE_CHANGETOVALUE,
	LITE_FADE,
	LITE_GLOW,
	LITE_FLICKER,
	LITE_STROBE
} lighttype_t;

typedef struct
{
	thinker_t  	thinker;
	sector_t	*sector;
	lighttype_t	type;
	int 		value1;
	int			value2;
	int			tics1;
	int			tics2;
	int 		count;
} light_t;
	
typedef struct
{
	thinker_t       thinker;
	sector_t        *sector;
	int index;
	int base;
} phase_t;

#define LIGHT_SEQUENCE_START    2
#define LIGHT_SEQUENCE          3
#define LIGHT_SEQUENCE_ALT      4

void T_Phase(phase_t *phase);
void T_Light(light_t *light);
void P_SpawnPhasedLight(sector_t *sector, int base, int index);
void P_SpawnLightSequence(sector_t *sector, int indexStep);
boolean EV_SpawnLight(line_t *line, byte *arg, lighttype_t type);

#if 0
typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	int count;
	int maxlight;
	int minlight;
	int maxtime;
	int mintime;
} lightflash_t;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	int count;
	int minlight;
	int maxlight;
	int darktime;
	int brighttime;
} strobe_t;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	int minlight;
	int maxlight;
	int direction;
} glow_t;

typedef struct
{
	thinker_t       thinker;
	sector_t        *sector;
	int index;
	int base;
} phase_t;

#define GLOWSPEED 8
#define STROBEBRIGHT 5
#define FASTDARK 15
#define SLOWDARK 35

#define LIGHT_SEQUENCE_START    2
#define LIGHT_SEQUENCE                  3
#define LIGHT_SEQUENCE_ALT      4

void T_LightFlash (lightflash_t *flash);
void P_SpawnLightFlash (sector_t *sector);
void T_StrobeFlash (strobe_t *flash);
void P_SpawnStrobeFlash (sector_t *sector, int fastOrSlow, int inSync);
void EV_StartLightStrobing(line_t *line);
void EV_TurnTagLightsOff(line_t      *line);
void EV_LightTurnOn(line_t *line, int bright);
void T_Glow(glow_t *g);
void P_SpawnGlowingLight(sector_t *sector);
void T_Phase(phase_t *phase);
void P_SpawnPhasedLight(sector_t *sector, int base, int index);
void P_SpawnLightSequence(sector_t *sector, int indexStep);
#endif

/*
===============================================================================

							P_SWITCH

===============================================================================
*/
typedef struct
{
	char name1[9];
	char name2[9];
	int soundID;
} switchlist_t;

typedef enum
{
	SWTCH_TOP,
	SWTCH_MIDDLE,
	SWTCH_BOTTOM
} bwhere_e;

typedef struct
{
	line_t *line;
	bwhere_e where;
	int btexture;
	int btimer;
	mobj_t *soundorg;
} button_t;

#define MAXSWITCHES 50              // max # of wall switches in a level
#define MAXBUTTONS 16              // 4 players, 4 buttons each at once, max.
#define BUTTONTIME 35              // 1 second

extern button_t buttonlist[MAXBUTTONS];

void P_ChangeSwitchTexture(line_t *line, int useAgain);
void P_InitSwitchList(void);

/*
===============================================================================

							P_PLATS

===============================================================================
*/

typedef enum
{
	PLAT_UP,
	PLAT_DOWN,
	PLAT_WAITING,
//	PLAT_IN_STASIS
} plat_e;

typedef enum
{
	PLAT_PERPETUALRAISE,
	PLAT_DOWNWAITUPSTAY,
	PLAT_DOWNBYVALUEWAITUPSTAY,
	PLAT_UPWAITDOWNSTAY,
	PLAT_UPBYVALUEWAITDOWNSTAY,
	//PLAT_RAISEANDCHANGE,
	//PLAT_RAISETONEARESTANDCHANGE
} plattype_e;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	fixed_t speed;
	fixed_t low;
	fixed_t high;
	int wait;
	int count;
	plat_e status;
	plat_e oldstatus;
	int crush;
	int tag;
	plattype_e type;
} plat_t;

#define PLATWAIT 3
#define PLATSPEED FRACUNIT
#define MAXPLATS 30

extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t *plat);
int EV_DoPlat(line_t *line, byte *args, plattype_e type, int amount);
void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
void EV_StopPlat(line_t *line, byte *args);

/*
===============================================================================

							P_DOORS

===============================================================================
*/
typedef enum
{
	DREV_NORMAL,
	DREV_CLOSE30THENOPEN,
	DREV_CLOSE,
	DREV_OPEN,
	DREV_RAISEIN5MINS,
} vldoor_e;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	vldoor_e type;
	fixed_t topheight;
	fixed_t speed;
	int direction; // 1 = up, 0 = waiting at top, -1 = down
	int topwait; // tics to wait at the top (keep in case a door going down is reset)
	int topcountdown;   // when it reaches 0, start going down
} vldoor_t;

#define VDOORSPEED FRACUNIT*2
#define VDOORWAIT 150

boolean EV_VerticalDoor(line_t *line, mobj_t *thing);
int EV_DoDoor(line_t *line, byte *args, vldoor_e type);
void T_VerticalDoor(vldoor_t *door);
//void P_SpawnDoorCloseIn30(sector_t *sec);
//void P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum);

/*
===============================================================================

							P_CEILNG

===============================================================================
*/
typedef enum
{
	CLEV_LOWERTOFLOOR,
	CLEV_RAISETOHIGHEST,
	CLEV_LOWERANDCRUSH,
	CLEV_CRUSHANDRAISE,
	CLEV_LOWERBYVALUE,
	CLEV_RAISEBYVALUE,
	CLEV_CRUSHRAISEANDSTAY,
	CLEV_MOVETOVALUETIMES8
} ceiling_e;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	ceiling_e type;
	fixed_t bottomheight, topheight;
	fixed_t speed;
	int crush;
	int direction; // 1 = up, 0 = waiting, -1 = down
	int tag; // ID
	int olddirection;
} ceiling_t;

#define CEILSPEED FRACUNIT
#define CEILWAIT 150
#define MAXCEILINGS 30

extern ceiling_t *activeceilings[MAXCEILINGS];

int EV_DoCeiling(line_t *line, byte *args, ceiling_e type);
void T_MoveCeiling(ceiling_t *ceiling);
void P_AddActiveCeiling(ceiling_t *c);
void P_RemoveActiveCeiling(ceiling_t *c);
int EV_CeilingCrushStop(line_t *line, byte *args);

/*
===============================================================================

							P_FLOOR

===============================================================================
*/
typedef enum
{
	FLEV_LOWERFLOOR,             // lower floor to highest surrounding floor
	FLEV_LOWERFLOORTOLOWEST,     // lower floor to lowest surrounding floor
	FLEV_LOWERFLOORBYVALUE,
	FLEV_RAISEFLOOR,             // raise floor to lowest surrounding CEILING
	FLEV_RAISEFLOORTONEAREST,  // raise floor to next highest surrounding floor
	FLEV_RAISEFLOORBYVALUE,
	FLEV_RAISEFLOORCRUSH,
	FLEV_RAISEBUILDSTEP,        // One step of a staircase
	FLEV_RAISEBYVALUETIMES8,
	FLEV_LOWERBYVALUETIMES8,
	FLEV_LOWERTIMES8INSTANT,
	FLEV_RAISETIMES8INSTANT,
	FLEV_MOVETOVALUETIMES8
} floor_e;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	floor_e type;
	int crush;
	int direction;
	int newspecial;
	short texture;
	fixed_t floordestheight;
	fixed_t speed;
	int	delayCount;
	int delayTotal;
	fixed_t stairsDelayHeight;
	fixed_t stairsDelayHeightDelta;
	fixed_t resetHeight;
	short resetDelay;
	short resetDelayCount;
	byte textureChange;
} floormove_t;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	int ceilingSpeed;
	int floorSpeed;
	int floordest;
	int ceilingdest;
	int direction;
	int crush;
} pillar_t;

typedef struct
{
	thinker_t thinker;
	sector_t *sector;
	fixed_t originalHeight;
	fixed_t accumulator;
	fixed_t accDelta;
	fixed_t targetScale;
	fixed_t scale;
	fixed_t scaleDelta;
	int ticker;
	int state;
} floorWaggle_t;

#define FLOORSPEED FRACUNIT

typedef enum
{
	RES_OK,
	RES_CRUSHED,
	RES_PASTDEST
} result_e;

typedef enum
{
	STAIRS_NORMAL,
	STAIRS_SYNC,
	STAIRS_PHASED
} stairs_e;

result_e T_MovePlane(sector_t *sector, fixed_t speed,
			fixed_t dest, int crush, int floorOrCeiling, int direction);

int EV_BuildStairs(line_t *line, byte *args, int direction, stairs_e type);
int EV_DoFloor(line_t *line, byte *args, floor_e floortype);
void T_MoveFloor(floormove_t *floor);
void T_BuildPillar(pillar_t *pillar);
void T_FloorWaggle(floorWaggle_t *waggle);
int EV_BuildPillar(line_t *line, byte *args, boolean crush);
int EV_OpenPillar(line_t *line, byte *args);
int EV_DoFloorAndCeiling(line_t *line, byte *args, boolean raise);
int EV_FloorCrushStop(line_t *line, byte *args);
boolean EV_StartFloorWaggle(int tag, int height, int speed, int offset,
	int timer);

//--------------------------------------------------------------------------
//
// p_telept
//
//--------------------------------------------------------------------------

boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, angle_t angle,
	boolean useFog);
boolean EV_Teleport(int tid, mobj_t *thing, boolean fog);

//--------------------------------------------------------------------------
//
// p_acs
//
//--------------------------------------------------------------------------

#define MAX_ACS_SCRIPT_VARS 10
#define MAX_ACS_MAP_VARS 32
#define MAX_ACS_WORLD_VARS 64
#define ACS_STACK_DEPTH 32
#define MAX_ACS_STORE 20

typedef enum
{
	ASTE_INACTIVE,
	ASTE_RUNNING,
	ASTE_SUSPENDED,
	ASTE_WAITINGFORTAG,
	ASTE_WAITINGFORPOLY,
	ASTE_WAITINGFORSCRIPT,
	ASTE_TERMINATING
} aste_t;

typedef struct acs_s acs_t;
typedef struct acsInfo_s acsInfo_t;

struct acsInfo_s
{
	int number;
	int *address;
	int argCount;
	aste_t state;
	int waitValue;
};

struct acs_s
{
	thinker_t thinker;
	mobj_t *activator;
	line_t *line;
	int side;
	int number;
	int infoIndex;
	int delayCount;
	int stack[ACS_STACK_DEPTH];
	int	stackPtr;
	int vars[MAX_ACS_SCRIPT_VARS];
	int *ip;
};

typedef struct
{
	int map;		// Target map
	int script;		// Script number on target map
	byte args[4];	// Padded to 4 for alignment
} acsstore_t;

void P_LoadACScripts(int lump);
boolean P_StartACS(int number, int map, byte *args, mobj_t *activator,
	line_t *line, int side);
boolean P_StartLockedACS(line_t *line, byte *args, mobj_t *mo, int side); 
boolean P_TerminateACS(int number, int map);
boolean P_SuspendACS(int number, int map);
void T_InterpretACS(acs_t *script);
void P_TagFinished(int tag);
void P_PolyobjFinished(int po);
void P_ACSInitNewGame(void);
void P_CheckACSStore(void);

extern int ACScriptCount;
extern byte *ActionCodeBase;
extern acsInfo_t *ACSInfo;
extern int MapVars[MAX_ACS_MAP_VARS];
extern int WorldVars[MAX_ACS_WORLD_VARS];
extern acsstore_t ACSStore[MAX_ACS_STORE+1]; // +1 for termination marker

//--------------------------------------------------------------------------
//
// p_things
//
//--------------------------------------------------------------------------

extern mobjtype_t TranslateThingType[];

boolean EV_ThingProjectile(byte *args, boolean gravity);
boolean EV_ThingSpawn(byte *args, boolean fog);
boolean EV_ThingActivate(int tid);
boolean EV_ThingDeactivate(int tid);
boolean EV_ThingRemove(int tid);
boolean EV_ThingDestroy(int tid);
