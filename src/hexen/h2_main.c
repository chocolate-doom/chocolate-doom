// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2008 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------


// HEADER FILES ------------------------------------------------------------

// haleyjd: removed WATCOMC
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"

#include "h2def.h"
#include "ct_chat.h"
#include "d_iwad.h"
#include "d_mode.h"
#include "m_misc.h"
#include "s_sound.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "p_local.h"
#include "v_video.h"
#include "w_main.h"

// MACROS ------------------------------------------------------------------

#define MAXWADFILES 20

// TYPES -------------------------------------------------------------------

typedef struct
{
    char *name;
    void (*func) (char **args, int tag);
    int requiredArgs;
    int tag;
} execOpt_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void R_ExecuteSetViewSize(void);
void D_CheckNetGame(void);
void G_BuildTiccmd(ticcmd_t * cmd);
void F_Drawer(void);
boolean F_Responder(event_t * ev);
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
static void DrawAndBlit(void);
static void ExecOptionSCRIPTS(char **args, int tag);
static void ExecOptionSKILL(char **args, int tag);
static void ExecOptionPLAYDEMO(char **args, int tag);
static void CreateSavePath(void);
static void WarpCheck(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean automapactive;
extern boolean MenuActive;
extern boolean askforquit;
extern char *SavePath;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

GameMode_t gamemode;
GameMission_t gamemission;
char *iwadfile;
boolean nomonsters;             // checkparm of -nomonsters
boolean respawnparm;            // checkparm of -respawn
boolean randomclass;            // checkparm of -randclass
boolean debugmode;              // checkparm of -debug
boolean ravpic;                 // checkparm of -ravpic
boolean cdrom = false;          // true if cd-rom mode active
boolean cmdfrag;                // true if a CMD_FRAG packet should be sent out
boolean singletics;             // debug flag to cancel adaptiveness
boolean artiskip;               // whether shift-enter skips an artifact
int maxzone = 0x800000;         // Maximum allocated for zone heap (8meg default)
skill_t startskill;
int startepisode;
int startmap;
boolean autostart;
boolean advancedemo;
FILE *debugfile;
int UpdateState;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int WarpMap;
static int demosequence;
static int pagetic;
static char *pagename;

static execOpt_t ExecOptions[] = {
    {"-scripts", ExecOptionSCRIPTS, 1, 0},
    {"-skill", ExecOptionSKILL, 1, 0},
    {"-playdemo", ExecOptionPLAYDEMO, 1, 0},
    {"-timedemo", ExecOptionPLAYDEMO, 1, 0},
    {NULL, NULL, 0, 0}          // Terminator
};

// CODE --------------------------------------------------------------------

void D_BindVariables(void)
{
    int i;

    M_ApplyPlatformDefaults();

    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
    M_BindMapControls();
    M_BindMenuControls();
    M_BindWeaponControls();
    M_BindChatControls(MAXPLAYERS);
    M_BindHereticControls();
    M_BindHexenControls();

    M_BindVariable("graphical_startup",      &graphical_startup);
    M_BindVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindVariable("sfx_volume",             &snd_MaxVolume);
    M_BindVariable("music_volume",           &snd_MusicVolume);
    M_BindVariable("messageson",             &messageson);
    M_BindVariable("screenblocks",           &screenblocks);
    M_BindVariable("snd_channels",           &snd_Channels);
    M_BindVariable("savedir",                &SavePath);

    // Multiplayer chat macros

    for (i=0; i<10; ++i)
    {
        char buf[12];

        sprintf(buf, "chatmacro%i", i);
        M_BindVariable(buf, &chat_macros[i]);
    }
}

// Set the default directory where hub savegames are saved.

static void D_SetDefaultSavePath(void)
{
    SavePath = M_GetSaveGameDir("hexen.wad");

    // If we are not using a savegame path (probably because we are on
    // Windows and not using a config dir), behave like Vanilla Hexen
    // and use hexndata/:

    if (!strcmp(SavePath, ""))
    {
        SavePath = malloc(10);
	sprintf(SavePath, "hexndata%c", DIR_SEPARATOR);
    }
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

static boolean D_GrabMouseCallback(void)
{
    // when menu is active or game is paused, release the mouse 
 
    if (MenuActive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback;
}

// Message displayed when quitting Hexen

static void D_HexenQuitMessage(void)
{
    printf("\nHexen: Beyond Heretic\n");
}

static void D_AddFile(char *filename)
{
    printf("  adding %s\n", filename);

    W_AddFile(filename);
}

//==========================================================================
//
// H2_Main
//
//==========================================================================
void InitMapMusicInfo(void);

void D_DoomMain(void)
{
    int p;

    I_AtExit(D_HexenQuitMessage, false);
    startepisode = 1;
    autostart = false;
    startskill = sk_medium;
    startmap = 1;
    gamemode = commercial;

    // Initialize subsystems

    ST_Message("V_Init: allocate screens.\n");
    V_Init();

    // Load defaults before initing other systems
    ST_Message("M_LoadDefaults: Load system defaults.\n");
    D_BindVariables();

#ifdef _WIN32

    //!
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\hexndata,
    // allowing play from CD.
    //

    cdrom = M_ParmExists("-cdrom");
#endif

    if (cdrom)
    {
        M_SetConfigDir("c:\\hexndata\\");
    }
    else
    {
        M_SetConfigDir(NULL);
    }

    D_SetDefaultSavePath();
    M_SetConfigFilenames("hexen.cfg", PROGRAM_PREFIX "hexen.cfg");
    M_LoadDefaults();

    I_AtExit(M_SaveDefaults, false);


    // Now that the savedir is loaded from .CFG, make sure it exists
    CreateSavePath();

    ST_Message("Z_Init: Init zone memory allocation daemon.\n");
    Z_Init();

    // haleyjd: removed WATCOMC

    ST_Message("W_Init: Init WADfiles.\n");

    iwadfile = D_FindIWAD(IWAD_MASK_HEXEN, &gamemission);

    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate. No IWAD was found. Try specifying\n"
                "one with the '-iwad' command line parameter.");
    }

    D_AddFile(iwadfile);

    HandleArgs();

    I_PrintStartupBanner("Hexen");

    ST_Message("MN_Init: Init menu system.\n");
    MN_Init();

    ST_Message("CT_Init: Init chat mode data.\n");
    CT_Init();

    InitMapMusicInfo();         // Init music fields in mapinfo

    ST_Message("S_InitScript\n");
    S_InitScript();

    ST_Message("SN_InitSequenceScript: Registering sound sequences.\n");
    SN_InitSequenceScript();
    ST_Message("I_Init: Setting up machine state.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();

    S_Init();
    S_Start();

    ST_Message("ST_Init: Init startup screen.\n");
    ST_Init();

    // Show version message now, so it's visible during R_Init()
    ST_Message("R_Init: Init Hexen refresh daemon");
    R_Init();
    ST_Message("\n");

    if (M_CheckParm("-net"))
        ST_NetProgress();       // Console player found

    ST_Message("P_Init: Init Playloop state.\n");
    P_Init();

    // Check for command line warping. Follows P_Init() because the
    // MAPINFO.TXT script must be already processed.
    WarpCheck();

    if (autostart)
    {
        ST_Message("Warp to Map %d (\"%s\":%d), Skill %d\n",
                   WarpMap, P_GetMapName(startmap), startmap, startskill + 1);
    }

    ST_Message("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame();

    ST_Message("SB_Init: Loading patches.\n");
    SB_Init();

    ST_Done();

    CheckRecordFrom();

    p = M_CheckParm("-record");
    if (p && p < myargc - 1)
    {
        G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p + 1]);
        H2_GameLoop();          // Never returns
    }

    p = M_CheckParm("-playdemo");
    if (p && p < myargc - 1)
    {
        singledemo = true;      // Quit after one demo
        G_DeferedPlayDemo(myargv[p + 1]);
        H2_GameLoop();          // Never returns
    }

    p = M_CheckParm("-timedemo");
    if (p && p < myargc - 1)
    {
        G_TimeDemo(myargv[p + 1]);
        H2_GameLoop();          // Never returns
    }

    p = M_CheckParm("-loadgame");
    if (p && p < myargc - 1)
    {
        G_LoadGame(atoi(myargv[p + 1]));
    }

    if (gameaction != ga_loadgame)
    {
        UpdateState |= I_FULLSCRN;
        BorderNeedRefresh = true;
        if (autostart || netgame)
        {
            G_StartNewInit();
            G_InitNew(startskill, startepisode, startmap);
        }
        else
        {
            H2_StartTitle();
        }
    }
    H2_GameLoop();              // Never returns
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

    cmdfrag = M_ParmExists("-cmdfrag");

    // Check WAD file command line options
    W_ParseCommandLine();

    // Process command line options
    for (opt = ExecOptions; opt->name != NULL; opt++)
    {
        p = M_CheckParm(opt->name);
        if (p && p < myargc - opt->requiredArgs)
        {
            opt->func(&myargv[p], opt->tag);
        }
    }
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
    if (p && p < myargc - 1)
    {
        WarpMap = atoi(myargv[p + 1]);
        map = P_TranslateMap(WarpMap);
        if (map == -1)
        {                       // Couldn't find real map number
            startmap = 1;
            ST_Message("-WARP: Invalid map number.\n");
        }
        else
        {                       // Found a valid startmap
            startmap = map;
            autostart = true;
        }
    }
    else
    {
        WarpMap = 1;
        startmap = P_TranslateMap(1);
        if (startmap == -1)
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
    startskill = args[1][0] - '1';
    autostart = true;
}

//==========================================================================
//
// ExecOptionPLAYDEMO
//
//==========================================================================

static void ExecOptionPLAYDEMO(char **args, int tag)
{
    char file[256];

    strcpy(file, args[1]);

    if (strcasecmp(file + strlen(file) - 4, ".lmp") != 0)
    {
        strcat(file, ".lmp");
    }

    W_AddFile(file);
    ST_Message("Playing demo %s.\n", file);
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
// H2_GameLoop
//
//==========================================================================

void H2_GameLoop(void)
{
    if (M_CheckParm("-debugfile"))
    {
        char filename[20];
        sprintf(filename, "debug%i.txt", consoleplayer);
        debugfile = fopen(filename, "w");
    }
    I_SetWindowTitle("Hexen");
    I_InitGraphics();
    I_SetGrabMouseCallback(D_GrabMouseCallback);

    while (1)
    {
        // Frame syncronous IO operations
        I_StartFrame();

        // Process one or more tics
        if (singletics)
        {
            I_StartTic();
            H2_ProcessEvents();
            G_BuildTiccmd(&netcmds[consoleplayer][maketic % BACKUPTICS]);
            if (advancedemo)
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

    for (;;)
    {
        ev = D_PopEvent();

        if (ev == NULL)
        {
            break;
        }

        if (F_Responder(ev))
        {
            continue;
        }
        if (MN_Responder(ev))
        {
            continue;
        }
        G_Responder(ev);
    }
}

//==========================================================================
//
// DrawAndBlit
//
//==========================================================================

static void DrawAndBlit(void)
{
    // Change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
    }

    // Do buffered drawing
    switch (gamestate)
    {
        case GS_LEVEL:
            if (!gametic)
            {
                break;
            }
            if (automapactive)
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

    if (paused && !MenuActive && !askforquit)
    {
        if (!netgame)
        {
            V_DrawPatch(160, viewwindowy + 5, W_CacheLumpName("PAUSED",
                                                              PU_CACHE));
        }
        else
        {
            V_DrawPatch(160, 70, W_CacheLumpName("PAUSED", PU_CACHE));
        }
    }

    // Draw current message
    DrawMessage();

    // Draw Menu
    MN_Drawer();

    // Send out any new accumulation
    NetUpdate();

    // Flush buffered stuff to screen
    I_FinishUpdate();
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
    if (player->messageTics <= 0 || !player->message)
    {                           // No message
        return;
    }
    if (player->yellowMessage)
    {
        MN_DrTextAYellow(player->message,
                         160 - MN_TextAWidth(player->message) / 2, 1);
    }
    else
    {
        MN_DrTextA(player->message, 160 - MN_TextAWidth(player->message) / 2,
                   1);
    }
}

//==========================================================================
//
// H2_PageTicker
//
//==========================================================================

void H2_PageTicker(void)
{
    if (--pagetic < 0)
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
    if (demosequence == 1)
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
    players[consoleplayer].playerstate = PST_LIVE;      // don't reborn
    advancedemo = false;
    usergame = false;           // can't save/end game here
    paused = false;
    gameaction = ga_nothing;
    demosequence = (demosequence + 1) % 7;
    switch (demosequence)
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
    if (!p || p > myargc - 2)
    {                           // Bad args
        return;
    }
    G_LoadGame(atoi(myargv[p + 1]));
    G_DoLoadGame();             // Load the gameskill etc info from savegame
    G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p + 2]);

    H2_GameLoop();              // Never returns
}

// haleyjd: removed WATCOMC
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

//==========================================================================
//
// CreateSavePath
//
//==========================================================================

static void CreateSavePath(void)
{
    M_MakeDirectory(SavePath);
}

