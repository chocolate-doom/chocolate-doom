
//**************************************************************************
//**
//** h2_main.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: h2_main.c,v $
//** $Revision: 1.50 $
//** $Date: 96/01/16 13:02:28 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#ifdef __WATCOMC__
#include <dos.h>
#include <sys\types.h>
#include <direct.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// MACROS ------------------------------------------------------------------

#define CONFIG_FILE_NAME "hexen.cfg"
#define MAXWADFILES 20

// TYPES -------------------------------------------------------------------

typedef struct
{
	char *name;
	void (*func)(char **args, int tag);
	int requiredArgs;
	int tag;
} execOpt_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void R_ExecuteSetViewSize(void);
void D_CheckNetGame(void);
void G_BuildTiccmd(ticcmd_t *cmd);
void F_Drawer(void);
boolean F_Responder(event_t *ev);
void I_StartupKeyboard(void);
void I_StartupJoystick(void);
void I_ShutdownKeyboard(void);
void S_InitScript(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void H2_ProcessEvents(void);
void H2_DoAdvanceDemo(void);
void H2_AdvanceDemo(void);
void H2_StartTitle(void);
void H2_PageTicker(void);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void DrawMessage(void);
static void PageDrawer(void);
static void HandleArgs(void);
static void CheckRecordFrom(void);
static void AddWADFile(char *file);
static void DrawAndBlit(void);
static void ExecOptionFILE(char **args, int tag);
static void ExecOptionSCRIPTS(char **args, int tag);
static void ExecOptionDEVMAPS(char **args, int tag);
static void ExecOptionSKILL(char **args, int tag);
static void ExecOptionPLAYDEMO(char **args, int tag);
static void ExecOptionMAXZONE(char **args, int tag);
static void CreateSavePath(void);
static void WarpCheck(void);

#ifdef TIMEBOMB
static void DoTimeBomb(void);
#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean automapactive;
extern boolean MenuActive;
extern boolean askforquit;
extern char *SavePath;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean DevMaps;			// true = Map development mode
char *DevMapsDir = "";		// development maps directory
boolean shareware;			// true if only episode 1 present
boolean nomonsters;			// checkparm of -nomonsters
boolean respawnparm;		// checkparm of -respawn
boolean randomclass;		// checkparm of -randclass
boolean debugmode;			// checkparm of -debug
boolean ravpic;				// checkparm of -ravpic
boolean cdrom;				// true if cd-rom mode active
boolean cmdfrag;			// true if a CMD_FRAG packet should be sent out
boolean singletics;			// debug flag to cancel adaptiveness
boolean artiskip;			// whether shift-enter skips an artifact
int maxzone = 0x800000;		// Maximum allocated for zone heap (8meg default)
skill_t startskill;
int startepisode;
int startmap;
boolean autostart;
boolean advancedemo;
FILE *debugfile;
event_t events[MAXEVENTS];
int eventhead;
int eventtail;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int WarpMap;
static int demosequence;
static int pagetic;
static char *pagename;
#ifdef __NeXT__
static char *wadfiles[MAXWADFILES] =
{
	"/Novell/H2/source/hexen.wad"
};
#else
static char *wadfiles[MAXWADFILES] =
{
	"hexen.wad"
};
#endif
static execOpt_t ExecOptions[] =
{
	{ "-file", ExecOptionFILE, 1, 0 },
	{ "-scripts", ExecOptionSCRIPTS, 1, 0 },
	{ "-devmaps", ExecOptionDEVMAPS, 1, 0 },
	{ "-skill", ExecOptionSKILL, 1, 0 },
	{ "-playdemo", ExecOptionPLAYDEMO, 1, 0 },
	{ "-timedemo", ExecOptionPLAYDEMO, 1, 0 },
	{ "-maxzone", ExecOptionMAXZONE, 1, 0 },
	{ NULL, NULL, 0, 0 } // Terminator
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// H2_Main
//
//==========================================================================
void InitMapMusicInfo(void);

void H2_Main(void)
{
	int p;

	M_FindResponseFile();
	setbuf(stdout, NULL);
	startepisode = 1;
	autostart = false;
	startskill = sk_medium;
	startmap = 1;
	shareware = false; // Always false for Hexen

	HandleArgs();

	// Initialize subsystems

	ST_Message("V_Init: allocate screens.\n");
	V_Init();

	// Load defaults before initing other systems
	ST_Message("M_LoadDefaults: Load system defaults.\n");
	M_LoadDefaults(CONFIG_FILE_NAME);

	// Now that the savedir is loaded from .CFG, make sure it exists
	CreateSavePath();

	// HEXEN MODIFICATION:
	// There is a realloc() in W_AddFile() that might fail if the zone
	// heap has been previously allocated, so we need to initialize the
	// WAD files BEFORE the zone memory initialization.
	ST_Message("W_Init: Init WADfiles.\n");
	W_InitMultipleFiles(wadfiles);

#ifdef TIMEBOMB
	DoTimeBomb();
#endif

	ST_Message("Z_Init: Init zone memory allocation daemon.\n");
	Z_Init();

#ifdef __WATCOMC__
	I_StartupKeyboard();
	I_StartupJoystick();
#endif

	ST_Message("MN_Init: Init menu system.\n");
	MN_Init();

	ST_Message("CT_Init: Init chat mode data.\n");
	CT_Init();

	InitMapMusicInfo();		// Init music fields in mapinfo

#ifdef __WATCOMC__
	ST_Message("S_InitScript\n");
	S_InitScript();
#endif

	ST_Message("SN_InitSequenceScript: Registering sound sequences.\n");
	SN_InitSequenceScript();
	ST_Message("I_Init: Setting up machine state.\n");
	I_Init();

	ST_Message("ST_Init: Init startup screen.\n");
	ST_Init();

	S_StartSongName("orb", true);

	// Show version message now, so it's visible during R_Init()
	ST_Message("Executable: "VERSIONTEXT".\n");

	ST_Message("R_Init: Init Hexen refresh daemon");
	R_Init();
	ST_Message("\n");

	if (M_CheckParm("-net")) ST_NetProgress();	// Console player found

	ST_Message("P_Init: Init Playloop state.\n");
	P_Init();

	// Check for command line warping. Follows P_Init() because the
	// MAPINFO.TXT script must be already processed.
	WarpCheck();

	if(autostart)
	{
		ST_Message("Warp to Map %d (\"%s\":%d), Skill %d\n",
			WarpMap, P_GetMapName(startmap), startmap, startskill+1);
	}

	ST_Message("D_CheckNetGame: Checking network game status.\n");
	D_CheckNetGame();

	ST_Message("SB_Init: Loading patches.\n");
	SB_Init();
	
	CheckRecordFrom();

	p = M_CheckParm("-record");
	if(p && p < myargc-1)
	{
		G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p+1]);
		H2_GameLoop(); // Never returns
	}

	p = M_CheckParm("-playdemo");
	if(p && p < myargc-1)
	{
		singledemo = true; // Quit after one demo
		G_DeferedPlayDemo(myargv[p+1]);
		H2_GameLoop(); // Never returns
	}

	p = M_CheckParm("-timedemo");
	if(p && p < myargc-1)
	{
		G_TimeDemo(myargv[p+1]);
		H2_GameLoop(); // Never returns
	}

	p = M_CheckParm("-loadgame");
	if(p && p < myargc-1)
	{
		G_LoadGame(atoi(myargv[p+1]));
	}

	if(gameaction != ga_loadgame)
	{
		UpdateState |= I_FULLSCRN;
		BorderNeedRefresh = true;
		if(autostart || netgame)
		{
			G_StartNewInit();
			G_InitNew(startskill, startepisode, startmap);
		}
		else
		{
			H2_StartTitle();
		}
	}
	H2_GameLoop(); // Never returns
}

//==========================================================================
//
// HandleArgs
//
//==========================================================================

static void HandleArgs(void)
{
	int p;
	execOpt_t *opt;

	nomonsters = M_ParmExists("-nomonsters");
	respawnparm = M_ParmExists("-respawn");
	randomclass = M_ParmExists("-randclass");
	ravpic = M_ParmExists("-ravpic");
	artiskip = M_ParmExists("-artiskip");
	debugmode = M_ParmExists("-debug");
	deathmatch = M_ParmExists("-deathmatch");
	cdrom = M_ParmExists("-cdrom");
	cmdfrag = M_ParmExists("-cmdfrag");

	// Process command line options
	for(opt = ExecOptions; opt->name != NULL; opt++)
	{
		p = M_CheckParm(opt->name);
		if(p && p < myargc-opt->requiredArgs)
		{
			opt->func(&myargv[p], opt->tag);
		}
	}

	// Look for an external device driver
	I_CheckExternDriver();
}

//==========================================================================
//
// WarpCheck
//
//==========================================================================

static void WarpCheck(void)
{
	int p;
	int map;

	p = M_CheckParm("-warp");
	if(p && p < myargc-1)
	{
		WarpMap = atoi(myargv[p+1]);
		map = P_TranslateMap(WarpMap);
		if(map == -1)
		{ // Couldn't find real map number
			startmap = 1;
			ST_Message("-WARP: Invalid map number.\n");
		}
		else
		{ // Found a valid startmap
			startmap = map;
			autostart = true;
		}
	}
	else
	{
		WarpMap = 1;
		startmap = P_TranslateMap(1);
		if(startmap == -1)
		{
			startmap = 1;
		}
	}
}

//==========================================================================
//
// ExecOptionSKILL
//
//==========================================================================

static void ExecOptionSKILL(char **args, int tag)
{
	startskill = args[1][0]-'1';
	autostart = true;
}

//==========================================================================
//
// ExecOptionFILE
//
//==========================================================================

static void ExecOptionFILE(char **args, int tag)
{
	int p;

	p = M_CheckParm("-file");
	while(++p != myargc && myargv[p][0] != '-')
	{
		AddWADFile(myargv[p]);
	}
}


//==========================================================================
//
// ExecOptionPLAYDEMO
//
//==========================================================================

static void ExecOptionPLAYDEMO(char **args, int tag)
{
	char file[256];

	sprintf(file, "%s.lmp", args[1]);
	AddWADFile(file);
	ST_Message("Playing demo %s.lmp.\n", args[1]);
}

//==========================================================================
//
// ExecOptionSCRIPTS
//
//==========================================================================

static void ExecOptionSCRIPTS(char **args, int tag)
{
	sc_FileScripts = true;
	sc_ScriptsDir = args[1];
}

//==========================================================================
//
// ExecOptionDEVMAPS
//
//==========================================================================

static void ExecOptionDEVMAPS(char **args, int tag)
{
	DevMaps = true;
	ST_Message("Map development mode enabled:\n");
	ST_Message("[config    ] = %s\n", args[1]);
	SC_OpenFileCLib(args[1]);
	SC_MustGetStringName("mapsdir");
	SC_MustGetString();
	ST_Message("[mapsdir   ] = %s\n", sc_String);
	DevMapsDir = malloc(strlen(sc_String)+1);
	strcpy(DevMapsDir, sc_String);
	SC_MustGetStringName("scriptsdir");
	SC_MustGetString();
	ST_Message("[scriptsdir] = %s\n", sc_String);
	sc_FileScripts = true;
	sc_ScriptsDir = malloc(strlen(sc_String)+1);
	strcpy(sc_ScriptsDir, sc_String);
	while(SC_GetString())
	{
		if(SC_Compare("file"))
		{
			SC_MustGetString();
			AddWADFile(sc_String);
		}
		else
		{
			SC_ScriptError(NULL);
		}
	}
	SC_Close();
}


long superatol(char *s)
{
	long int n=0, r=10, x, mul=1;
	char *c=s;

	for (; *c; c++)
	{
		x = (*c & 223) - 16;

		if (x == -3)
		{
			mul = -mul;
		}
		else if (x == 72 && r == 10)
		{
			n -= (r=n);
			if (!r) r=16;
			if (r<2 || r>36) return -1;
		}
		else
		{
			if (x>10) x-=39;
			if (x >= r) return -1;
			n = (n*r) + x;
		}
	}
	return(mul*n);
}


static void ExecOptionMAXZONE(char **args, int tag)
{
	int size;
	
	size = superatol(args[1]);
	if (size < MINIMUM_HEAP_SIZE) size = MINIMUM_HEAP_SIZE;
	if (size > MAXIMUM_HEAP_SIZE) size = MAXIMUM_HEAP_SIZE;
	maxzone = size;
}

//==========================================================================
//
// H2_GameLoop
//
//==========================================================================

void H2_GameLoop(void)
{
	if(M_CheckParm("-debugfile"))
	{
		char filename[20];
		sprintf(filename, "debug%i.txt", consoleplayer);
		debugfile = fopen(filename,"w");
	}
	I_InitGraphics();
	while(1)
	{
		// Frame syncronous IO operations
		I_StartFrame();

		// Process one or more tics
		if(singletics)
		{
			I_StartTic();
			H2_ProcessEvents();
			G_BuildTiccmd(&netcmds[consoleplayer][maketic%BACKUPTICS]);
			if(advancedemo)
			{
				H2_DoAdvanceDemo();
			}
			G_Ticker();
			gametic++;
			maketic++;
		}
		else
		{
			// Will run at least one tic
			TryRunTics();
		}

		// Move positional sounds
		S_UpdateSounds(players[displayplayer].mo);

		DrawAndBlit();
	}
}

//==========================================================================
//
// H2_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain.
//
//==========================================================================

void H2_ProcessEvents(void)
{
	event_t *ev;

	for(; eventtail != eventhead; eventtail = (++eventtail)&(MAXEVENTS-1))
	{
		ev = &events[eventtail];
		if(F_Responder(ev))
		{
			continue;
		}
		if(MN_Responder(ev))
		{
			continue;
		}
		G_Responder(ev);
	}
}

//==========================================================================
//
// H2_PostEvent
//
// Called by the I/O functions when input is detected.
//
//==========================================================================

void H2_PostEvent(event_t *ev)
{
	events[eventhead] = *ev;
	eventhead = (++eventhead)&(MAXEVENTS-1);
}

//==========================================================================
//
// DrawAndBlit
//
//==========================================================================

static void DrawAndBlit(void)
{
	// Change the view size if needed
	if(setsizeneeded)
	{
		R_ExecuteSetViewSize();
	}

	// Do buffered drawing
	switch(gamestate)
	{
		case GS_LEVEL:
			if(!gametic)
			{
				break;
			}
			if(automapactive)
			{
				AM_Drawer();
			}
			else
			{
				R_RenderPlayerView(&players[displayplayer]);
			}
			CT_Drawer();
			UpdateState |= I_FULLVIEW;
			SB_Drawer();
			break;
		case GS_INTERMISSION:
			IN_Drawer();
			break;
		case GS_FINALE:
			F_Drawer();
			break;
		case GS_DEMOSCREEN:
			PageDrawer();
			break;
	}

	if(paused && !MenuActive && !askforquit)
	{
		if(!netgame)
		{
			V_DrawPatch(160, viewwindowy+5, W_CacheLumpName("PAUSED",
				PU_CACHE));
		}
		else
		{
			V_DrawPatch(160, 70, W_CacheLumpName("PAUSED",
				PU_CACHE));
		}
	}

	// Draw current message
	DrawMessage();

	// Draw Menu
	MN_Drawer();

	// Send out any new accumulation
	NetUpdate();

	// Flush buffered stuff to screen
	I_Update();
}

//==========================================================================
//
// DrawMessage
//
//==========================================================================

static void DrawMessage(void)
{
	player_t *player;

	player = &players[consoleplayer];
	if(player->messageTics <= 0 || !player->message)
	{ // No message
		return;
	}
	if(player->yellowMessage)
	{
		MN_DrTextAYellow(player->message, 
			160-MN_TextAWidth(player->message)/2, 1);
	}
	else
	{
		MN_DrTextA(player->message, 160-MN_TextAWidth(player->message)/2, 1);
	}
}

//==========================================================================
//
// H2_PageTicker
//
//==========================================================================

void H2_PageTicker(void)
{
	if(--pagetic < 0)
	{
		H2_AdvanceDemo();
	}
}

//==========================================================================
//
// PageDrawer
//
//==========================================================================

static void PageDrawer(void)
{
	V_DrawRawScreen(W_CacheLumpName(pagename, PU_CACHE));
	if(demosequence == 1)
	{
		V_DrawPatch(4, 160, W_CacheLumpName("ADVISOR", PU_CACHE));
	}
	UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// H2_AdvanceDemo
//
// Called after each demo or intro demosequence finishes.
//
//==========================================================================

void H2_AdvanceDemo(void)
{
	advancedemo = true;
}

//==========================================================================
//
// H2_DoAdvanceDemo
//
//==========================================================================

void H2_DoAdvanceDemo(void)
{
	players[consoleplayer].playerstate = PST_LIVE; // don't reborn
	advancedemo = false;
	usergame = false; // can't save/end game here
	paused = false;
	gameaction = ga_nothing;
	demosequence = (demosequence+1)%7;
	switch(demosequence)
	{
		case 0:
			pagetic = 280;
			gamestate = GS_DEMOSCREEN;
			pagename = "TITLE";
			S_StartSongName("hexen", true);
			break;
		case 1:
			pagetic = 210;
			gamestate = GS_DEMOSCREEN;
			pagename = "TITLE";
			break;
		case 2:
			BorderNeedRefresh = true;
			UpdateState |= I_FULLSCRN;
			G_DeferedPlayDemo("demo1");
			break;
		case 3:
			pagetic = 200;
			gamestate = GS_DEMOSCREEN;
			pagename = "CREDIT";
			break;
		case 4:
			BorderNeedRefresh = true;
			UpdateState |= I_FULLSCRN;
			G_DeferedPlayDemo("demo2");
			break;
		case 5:
			pagetic = 200;
			gamestate = GS_DEMOSCREEN;
			pagename = "CREDIT";
			break;
		case 6:
			BorderNeedRefresh = true;
			UpdateState |= I_FULLSCRN;
			G_DeferedPlayDemo("demo3");
			break;
	}
}

//==========================================================================
//
// H2_StartTitle
//
//==========================================================================

void H2_StartTitle(void)
{
	gameaction = ga_nothing;
	demosequence = -1;
	H2_AdvanceDemo();
}

//==========================================================================
//
// CheckRecordFrom
//
// -recordfrom <savegame num> <demoname>
//
//==========================================================================

static void CheckRecordFrom(void)
{
	int p;

	p = M_CheckParm("-recordfrom");
	if(!p || p > myargc-2)
	{ // Bad args
		return;
	}
	G_LoadGame(atoi(myargv[p+1]));
	G_DoLoadGame(); // Load the gameskill etc info from savegame
	G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p+2]);
	H2_GameLoop(); // Never returns
}

//==========================================================================
//
// AddWADFile
//
//==========================================================================

static void AddWADFile(char *file)
{
	int i;
	char *new;

	ST_Message("Adding external file: %s\n", file);
	i = 0;
	while(wadfiles[i])
	{
		i++;
	}
	new = malloc(strlen(file)+1);
	strcpy(new, file);
	wadfiles[i] = new;
}

#ifdef __WATCOMC__
/*
void CleanExit(void)
{
	union REGS regs;

	I_ShutdownKeyboard();
	regs.x.eax = 0x3;
	int386(0x10, &regs, &regs);
	printf("Exited from HEXEN: Beyond Heretic.\n");
	exit(1);
}
*/
#endif

//==========================================================================
//
// FixedDiv
//
//==========================================================================

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	if((abs(a)>>14) >= abs(b))
	{
		return((a^b)<0 ? MININT : MAXINT);
	}
	return(FixedDiv2(a, b));
}


//==========================================================================
//
// CreateSavePath
//
//==========================================================================

static void CreateSavePath(void)
{
	char creationPath[121];
	int len;

	if(cdrom == true)
	{
		SavePath = "c:\\hexndata\\";
	}
	len = strlen(SavePath);
	if (len >= 120) I_Error("Save path too long\n");
	strcpy(creationPath, SavePath);
#ifdef __WATCOMC__
	creationPath[len-1] = 0;
	mkdir(creationPath);
#endif
}

#ifdef TIMEBOMB
//==========================================================================
//
// DoTimeBomb
//
//==========================================================================

static void DoTimeBomb(void)
{
#ifdef __WATCOMC__
	time_t timeOfDay;
	struct tm timeBuffer;

	timeOfDay = time(NULL);
	_localtime(&timeOfDay, &timeBuffer);
	if(timeBuffer.tm_year != TIMEBOMB_YEAR 
	|| timeBuffer.tm_yday < TIMEBOMB_STARTDATE 
	|| timeBuffer.tm_yday > TIMEBOMB_ENDDATE)
	{
		I_Error("W_InitWadfiles:  Wad file doesn't have IWAD or PWAD id\n");
	}
 
	printf("\n===============================================================================\n");
	printf("                             Hexen:  Beyond Heretic\n\n");
	printf("                           Beta -- Do Not Distribute!\n");
	printf("                           Press any key to continue.\n");
	printf("===============================================================================\n");
	
	getch();
	printf("\n");
#endif
}
#endif
