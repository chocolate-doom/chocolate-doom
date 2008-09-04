
//**************************************************************************
//**
//** p_local.h : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_local.h,v $
//** $Revision: 1.64 $
//** $Date: 95/10/13 03:00:16 $
//** $Author: cjr $
//**
//**************************************************************************

#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.h"
#endif

#define STARTREDPALS    1
#define STARTBONUSPALS  9
#define STARTPOISONPALS 13
#define STARTICEPAL		21
#define STARTHOLYPAL	22
#define STARTSCOURGEPAL 25
#define NUMREDPALS      8
#define NUMBONUSPALS    4
#define NUMPOISONPALS	8

#define TOCENTER -8
#define FLOATSPEED (FRACUNIT*4)

#define MAXHEALTH 100
#define MAXMORPHHEALTH 30
#define VIEWHEIGHT (48*FRACUNIT)

// mapblocks are used to check movement against lines and things
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK                (MAPBLOCKSIZE-1)
#define MAPBTOFRAC              (MAPBLOCKSHIFT-FRACBITS)

// player radius for movement checking
#define PLAYERRADIUS 16*FRACUNIT

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger, but we don't have any moving sectors
// nearby
#define MAXRADIUS 32*FRACUNIT

#define GRAVITY FRACUNIT
#define MAXMOVE (30*FRACUNIT)

#define USERANGE (64*FRACUNIT)
#define MELEERANGE (64*FRACUNIT)
#define MISSILERANGE (32*64*FRACUNIT)

typedef enum
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
} dirtype_t;

#define BASETHRESHOLD 100 // follow a player exlusively for 3 seconds

// ***** P_TICK *****

extern thinker_t thinkercap; // both the head and tail of the thinker list
extern int TimerGame; // tic countdown for deathmatch

void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);

// ***** P_PSPR *****

#define USE_MANA1	1
#define USE_MANA2	1

void P_SetPsprite(player_t *player, int position, statenum_t stnum);
void P_SetPspriteNF(player_t *player, int position, statenum_t stnum);
void P_SetupPsprites(player_t *curplayer);
void P_MovePsprites(player_t *curplayer);
void P_DropWeapon(player_t *player);
void P_ActivateMorphWeapon(player_t *player);
void P_PostMorphWeapon(player_t *player, weapontype_t weapon);

// ***** P_USER *****

extern int PStateNormal[NUMCLASSES]; 
extern int PStateRun[NUMCLASSES];
extern int PStateAttack[NUMCLASSES]; 
extern int PStateAttackEnd[NUMCLASSES];

void P_PlayerThink(player_t *player);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);
void P_PlayerRemoveArtifact(player_t *player, int slot);
void P_PlayerUseArtifact(player_t *player, artitype_t arti);
boolean P_UseArtifact(player_t *player, artitype_t arti);
int P_GetPlayerNum(player_t *player);
void P_TeleportOther(mobj_t *victim);
void ResetBlasted(mobj_t *mo);

// ***** P_MOBJ *****

// Any floor type >= FLOOR_LIQUID will floorclip sprites
enum
{
	FLOOR_SOLID,
	FLOOR_ICE,
	FLOOR_LIQUID,
	FLOOR_WATER,
	FLOOR_LAVA,
	FLOOR_SLUDGE
};

#define ONFLOORZ MININT
#define ONCEILINGZ MAXINT
#define FLOATRANDZ (MAXINT-1)
#define FROMCEILINGZ128 (MAXINT-2)

extern mobjtype_t PuffType;
extern mobj_t *MissileMobj;

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void P_RemoveMobj(mobj_t *th);
boolean P_SetMobjState(mobj_t *mobj, statenum_t state);
boolean P_SetMobjStateNF(mobj_t *mobj, statenum_t state);
void P_ThrustMobj(mobj_t *mo, angle_t angle, fixed_t move);
int P_FaceMobj(mobj_t *source, mobj_t *target, angle_t *delta);
boolean P_SeekerMissile(mobj_t *actor, angle_t thresh, angle_t turnMax);
void P_MobjThinker(mobj_t *mobj);
void P_BlasterMobjThinker(mobj_t *mobj);
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage);
void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t *originator);
void P_BloodSplatter2(fixed_t x, fixed_t y, fixed_t z, mobj_t *originator);
void P_RipperBlood(mobj_t *mo);
int P_GetThingFloorType(mobj_t *thing);
int P_HitFloor(mobj_t *thing);
boolean P_CheckMissileSpawn(mobj_t *missile);
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnMissileXYZ(fixed_t x, fixed_t y, fixed_t z,
	mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnMissileAngle(mobj_t *source, mobjtype_t type,
	angle_t angle, fixed_t momz);
mobj_t *P_SpawnMissileAngleSpeed(mobj_t *source, mobjtype_t type,
	angle_t angle, fixed_t momz, fixed_t speed);
mobj_t *P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);
mobj_t *P_SPMAngle(mobj_t *source, mobjtype_t type, angle_t angle);
mobj_t *P_SPMAngleXYZ(mobj_t *source, fixed_t x, fixed_t y, 
	fixed_t z, mobjtype_t type, angle_t angle);
void P_CreateTIDList(void);
void P_RemoveMobjFromTIDList(mobj_t *mobj);
void P_InsertMobjIntoTIDList(mobj_t *mobj, int tid);
mobj_t *P_FindMobjFromTID(int tid, int *searchPosition);
mobj_t *P_SpawnKoraxMissile(fixed_t x, fixed_t y, fixed_t z,
	mobj_t *source, mobj_t *dest, mobjtype_t type);

// ***** P_ENEMY *****

void P_NoiseAlert (mobj_t *target, mobj_t *emmiter);
int P_Massacre(void);
boolean A_RaiseMobj(mobj_t *actor);
boolean A_SinkMobj(mobj_t *actor);
void A_NoBlocking(mobj_t *actor);
boolean P_LookForMonsters(mobj_t *actor);
void P_InitCreatureCorpseQueue(boolean corpseScan);
void A_DeQueueCorpse(mobj_t *actor);


// ***** P_MAPUTL *****

typedef struct
{
	fixed_t x, y, dx, dy;
} divline_t;

typedef struct
{
	fixed_t         frac;           // along trace line
	boolean         isaline;
	union {
		mobj_t  *thing;
		line_t  *line;
	}                       d;
} intercept_t;

#define MAXINTERCEPTS   128
extern  intercept_t             intercepts[MAXINTERCEPTS], *intercept_p;
typedef boolean (*traverser_t) (intercept_t *in);


fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int     P_PointOnLineSide (fixed_t x, fixed_t y, line_t *line);
int     P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t *line);
void    P_MakeDivline (line_t *li, divline_t *dl);
fixed_t P_InterceptVector (divline_t *v2, divline_t *v1);
int     P_BoxOnLineSide (fixed_t *tmbox, line_t *ld);

extern  fixed_t opentop, openbottom, openrange;
extern  fixed_t lowfloor;
void    P_LineOpening (line_t *linedef);

boolean P_BlockLinesIterator (int x, int y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator (int x, int y, boolean(*func)(mobj_t*) );

#define PT_ADDLINES             1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT             4

extern  divline_t       trace;
boolean P_PathTraverse (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
	int flags, boolean (*trav) (intercept_t *));

void    P_UnsetThingPosition (mobj_t *thing);
void    P_SetThingPosition (mobj_t *thing);
mobj_t *P_RoughMonsterSearch(mobj_t *mo, int distance);

// ***** P_MAP *****

extern boolean floatok;                         // if true, move would be ok if
extern fixed_t tmfloorz, tmceilingz;    // within tmfloorz - tmceilingz
extern int tmfloorpic;
extern mobj_t *BlockingMobj;

extern line_t *ceilingline;
boolean P_TestMobjLocation(mobj_t *mobj);
boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
mobj_t *P_CheckOnmobj(mobj_t *thing);
void P_FakeZMovement(mobj_t *mo);
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y);
void P_SlideMove(mobj_t *mo);
void P_BounceWall(mobj_t *mo);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);
void P_UseLines(player_t *player);
boolean P_UsePuzzleItem(player_t *player, int itemType);
void PIT_ThrustSpike(mobj_t *actor);

boolean P_ChangeSector (sector_t *sector, int crunch);

extern mobj_t *PuffSpawned; // true if a puff was spawned
extern  mobj_t          *linetarget;              // who got hit (or NULL)
fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, fixed_t distance);

void P_LineAttack (mobj_t *t1, angle_t angle, fixed_t distance, fixed_t slope, int damage);

void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage, int distance,
	boolean damageSource);

// ***** P_SETUP *****

extern byte *rejectmatrix;                              // for fast sight rejection
extern short *blockmaplump;                             // offsets in blockmap are from here
extern short *blockmap;
extern int bmapwidth, bmapheight;               // in mapblocks
extern fixed_t bmaporgx, bmaporgy;              // origin of block map
extern mobj_t **blocklinks;                             // for thing chains

// ***** P_INTER *****

extern int clipmana[NUMMANA];

void P_SetMessage(player_t *player, char *message, boolean ultmsg);
void P_SetYellowMessage(player_t *player, char *message, boolean ultmsg);
void P_ClearMessage(player_t *player);
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher);
void P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source,
	int damage);
void P_FallingDamage(player_t *player);
void P_PoisonPlayer(player_t *player, mobj_t *poisoner, int poison);
void P_PoisonDamage(player_t *player, mobj_t *source, int damage,
	boolean playPainSound);
boolean P_GiveMana(player_t *player, manatype_t mana, int count);
boolean P_GiveArtifact(player_t *player, artitype_t arti, mobj_t *mo);
boolean P_GiveArmor(player_t *player, armortype_t armortype, int amount);
boolean P_GiveBody(player_t *player, int num);
boolean P_GivePower(player_t *player, powertype_t power);
boolean P_MorphPlayer(player_t *player);

// ***** AM_MAP *****

boolean AM_Responder(event_t *ev);
void AM_Ticker(void);
void AM_Drawer(void);

// ***** A_ACTION *****
boolean A_LocalQuake(byte *args, mobj_t *victim);
void P_SpawnDirt(mobj_t *actor, fixed_t radius);
void A_BridgeRemove(mobj_t *actor);

// ***** SB_BAR *****

extern int SB_state;
extern int ArtifactFlash;
void SB_PaletteFlash(boolean forceChange);

// ===== PO_MAN =====

typedef enum
{
	PODOOR_NONE,
	PODOOR_SLIDE,
	PODOOR_SWING,
} podoortype_t;

typedef struct
{
	thinker_t thinker;
	int polyobj;
	int speed;
	unsigned int dist;
	int angle;
	fixed_t xSpeed; // for sliding walls
	fixed_t ySpeed;
} polyevent_t;

typedef struct
{
	thinker_t thinker;
	int polyobj;
	int speed;
	int dist;
	int totalDist;
	int direction;
	fixed_t xSpeed, ySpeed;
	int tics;
	int waitTics;
	podoortype_t type;
	boolean close;
} polydoor_t;

enum
{
	PO_ANCHOR_TYPE = 3000,
	PO_SPAWN_TYPE,
	PO_SPAWNCRUSH_TYPE
};

#define PO_LINE_START 1 // polyobj line start special
#define PO_LINE_EXPLICIT 5

extern polyobj_t *polyobjs; // list of all poly-objects on the level
extern int po_NumPolyobjs;

void T_PolyDoor(polydoor_t *pd);
void T_RotatePoly(polyevent_t *pe);
boolean EV_RotatePoly(line_t *line, byte *args, int direction, boolean 
	overRide);
void T_MovePoly(polyevent_t *pe);
boolean EV_MovePoly(line_t *line, byte *args, boolean timesEight, boolean
	overRide);
boolean EV_OpenPolyDoor(line_t *line, byte *args, podoortype_t type);

boolean PO_MovePolyobj(int num, int x, int y);
boolean PO_RotatePolyobj(int num, angle_t angle);
void PO_Init(int lump);
boolean PO_Busy(int polyobj);

#include "p_spec.h"

#endif // __P_LOCAL__
