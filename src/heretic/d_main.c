// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

// D_main.c

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "ct_chat.h"
#include "doomdef.h"
#include "d_iwad.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

GameMission_t gamemission = heretic;
GameMode_t gamemode = indetermined;
char *gamedescription = "unknown";

boolean nomonsters;             // checkparm of -nomonsters
boolean respawnparm;            // checkparm of -respawn
boolean debugmode;              // checkparm of -debug
boolean ravpic;                 // checkparm of -ravpic
boolean cdrom;                  // true if cd-rom mode active
boolean singletics;             // debug flag to cancel adaptiveness
boolean noartiskip;             // whether shift-enter skips an artifact

skill_t startskill;
int startepisode;
int startmap;
int UpdateState;
boolean autostart;
extern boolean automapactive;

boolean advancedemo;

FILE *debugfile;

static int show_endoom = 1;

void D_CheckNetGame(void);
void D_ProcessEvents(void);
void G_BuildTiccmd(ticcmd_t * cmd);
void D_DoAdvanceDemo(void);
void D_PageDrawer(void);
void D_AdvanceDemo(void);
void F_Drawer(void);
boolean F_Responder(event_t * ev);

//---------------------------------------------------------------------------
//
// PROC D_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain.
//
//---------------------------------------------------------------------------

void D_ProcessEvents(void)
{
    event_t *ev;

    while ((ev = D_PopEvent()) != NULL)
    {
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

//---------------------------------------------------------------------------
//
// PROC DrawMessage
//
//---------------------------------------------------------------------------

void DrawMessage(void)
{
    player_t *player;

    player = &players[consoleplayer];
    if (player->messageTics <= 0 || !player->message)
    {                           // No message
        return;
    }
    MN_DrTextA(player->message, 160 - MN_TextAWidth(player->message) / 2, 1);
}

//---------------------------------------------------------------------------
//
// PROC D_Display
//
// Draw current display, possibly wiping it from the previous.
//
//---------------------------------------------------------------------------

void R_ExecuteSetViewSize(void);

extern boolean finalestage;

void D_Display(void)
{
    extern boolean MenuActive;
    extern boolean askforquit;

    // Change the view size if needed
    if (setsizeneeded)
    {
        R_ExecuteSetViewSize();
    }

//
// do buffered drawing
//
    switch (gamestate)
    {
        case GS_LEVEL:
            if (!gametic)
                break;
            if (automapactive)
                AM_Drawer();
            else
                R_RenderPlayerView(&players[displayplayer]);
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
            D_PageDrawer();
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
    // Handle player messages
    DrawMessage();

    // Menu drawing
    MN_Drawer();

    // Send out any new accumulation
    NetUpdate();

    // Flush buffered stuff to screen
    I_FinishUpdate();
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // when menu is active or game is paused, release the mouse 
 
    if (MenuActive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback;
}

//---------------------------------------------------------------------------
//
// PROC D_DoomLoop
//
//---------------------------------------------------------------------------

void D_DoomLoop(void)
{
    if (M_CheckParm("-debugfile"))
    {
        char filename[20];
        sprintf(filename, "debug%i.txt", consoleplayer);
        debugfile = fopen(filename, "w");
    }
    I_InitGraphics();
    I_SetWindowTitle(gamedescription);
    I_SetGrabMouseCallback(D_GrabMouseCallback);

    while (1)
    {
        // Frame syncronous IO operations
        I_StartFrame();

        // Process one or more tics
        if (singletics)
        {
            I_StartTic();
            D_ProcessEvents();
            G_BuildTiccmd(&netcmds[consoleplayer][maketic % BACKUPTICS]);
            if (advancedemo)
                D_DoAdvanceDemo();
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
        S_UpdateSounds(players[consoleplayer].mo);
        D_Display();
    }
}

/*
===============================================================================

						DEMO LOOP

===============================================================================
*/

int demosequence;
int pagetic;
char *pagename;


/*
================
=
= D_PageTicker
=
= Handles timing for warped projection
=
================
*/

void D_PageTicker(void)
{
    if (--pagetic < 0)
        D_AdvanceDemo();
}


/*
================
=
= D_PageDrawer
=
================
*/

extern boolean MenuActive;

void D_PageDrawer(void)
{
    V_DrawRawScreen(W_CacheLumpName(pagename, PU_CACHE));
    if (demosequence == 1)
    {
        V_DrawPatch(4, 160, W_CacheLumpName("ADVISOR", PU_CACHE));
    }
    UpdateState |= I_FULLSCRN;
}

/*
=================
=
= D_AdvanceDemo
=
= Called after each demo or intro demosequence finishes
=================
*/

void D_AdvanceDemo(void)
{
    advancedemo = true;
}

void D_DoAdvanceDemo(void)
{
    players[consoleplayer].playerstate = PST_LIVE;      // don't reborn
    advancedemo = false;
    usergame = false;           // can't save / end game here
    paused = false;
    gameaction = ga_nothing;
    demosequence = (demosequence + 1) % 7;
    switch (demosequence)
    {
        case 0:
            pagetic = 210;
            gamestate = GS_DEMOSCREEN;
            pagename = "TITLE";
            S_StartSong(mus_titl, false);
            break;
        case 1:
            pagetic = 140;
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
            if (gamemode == shareware)
            {
                pagename = "ORDER";
            }
            else
            {
                pagename = "CREDIT";
            }
            break;
        case 6:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            G_DeferedPlayDemo("demo3");
            break;
    }
}


/*
=================
=
= D_StartTitle
=
=================
*/

void D_StartTitle(void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo();
}


/*
==============
=
= D_CheckRecordFrom
=
= -recordfrom <savegame num> <demoname>
==============
*/

void D_CheckRecordFrom(void)
{
    int p;
    char file[256];

    p = M_CheckParm("-recordfrom");
    if (!p || p > myargc - 2)
        return;

    if (cdrom)
    {
        sprintf(file, SAVEGAMENAMECD "%c.hsg", myargv[p + 1][0]);
    }
    else
    {
        sprintf(file, SAVEGAMENAME "%c.hsg", myargv[p + 1][0]);
    }
    G_LoadGame(file);
    G_DoLoadGame();             // load the gameskill etc info from savegame

    G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p + 2]);
    D_DoomLoop();               // never returns
}

/*
===============
=
= D_AddFile
=
===============
*/

// MAPDIR should be defined as the directory that holds development maps
// for the -wart # # command

#define MAPDIR "\\data\\"

#define SHAREWAREWADNAME "heretic1.wad"

static iwad_t iwads[] = {
    { "heretic.wad",        heretic },
    { "heretic1.wad",       heretic },
    { NULL,                 none },
};

char *iwadfile;

char *basedefault = "heretic.cfg";

void wadprintf(void)
{
    if (debugmode)
    {
        return;
    }
#ifdef __WATCOMC__
    _settextposition(23, 2);
    _setbkcolor(1);
    _settextcolor(0);
    _outtext(exrnwads);
    _settextposition(24, 2);
    _outtext(exrnwads2);
#endif
}

boolean D_AddFile(char *file)
{
    wad_file_t *handle;

    printf("  adding %s\n", file);

    handle = W_AddFile(file);

    return handle != NULL;
}

//==========================================================
//
//  Startup Thermo code
//
//==========================================================
#define MSG_Y       9
//#define THERM_X 15
//#define THERM_Y 16
//#define THERMCOLOR  3
#define THERM_X     14
#define THERM_Y     14

int thermMax;
int thermCurrent;
char *startup;                  // * to text screen
char smsg[80];                  // status bar line

//
//  Heretic startup screen shit
//

byte *hscreen;

void hgotoxy(int x, int y)
{
    hscreen = (byte *) (0xb8000 + y * 160 + x * 2);
}

void hput(unsigned char c, unsigned char a)
{
    *hscreen++ = c;
    *hscreen++ = a;
}

void hprintf(char *string, unsigned char a)
{
#ifdef __WATCOMC__
    int i;

    if (debugmode)
    {
        puts(string);
        return;
    }
    for (i = 0; i < strlen(string); i++)
    {
        hput(string[i], a);
    }
#endif
}

void drawstatus(void)
{
    if (debugmode)
    {
        return;
    }
#ifdef __WATCOMC__
    _settextposition(25, 2);
    _setbkcolor(1);
    _settextcolor(15);
    _outtext(smsg);
    _settextposition(25, 1);
#endif
}

void status(char *string)
{
    strcat(smsg, string);
    drawstatus();
}

void DrawThermo(void)
{
#ifdef __WATCOMC__
    unsigned char *screen;
    int progress;
    int i;

    if (debugmode)
    {
        return;
    }
#if 0
    progress = (98 * thermCurrent) / thermMax;
    screen = (char *) 0xb8000 + (THERM_Y * 160 + THERM_X * 2);
    for (i = 0; i < progress / 2; i++)
    {
        switch (i)
        {
            case 4:
            case 9:
            case 14:
            case 19:
            case 29:
            case 34:
            case 39:
            case 44:
                *screen++ = 0xb3;
                *screen++ = (THERMCOLOR << 4) + 15;
                break;
            case 24:
                *screen++ = 0xba;
                *screen++ = (THERMCOLOR << 4) + 15;
                break;
            default:
                *screen++ = 0xdb;
                *screen++ = 0x40 + THERMCOLOR;
                break;
        }
    }
    if (progress & 1)
    {
        *screen++ = 0xdd;
        *screen++ = 0x40 + THERMCOLOR;
    }
#else
    progress = (50 * thermCurrent) / thermMax + 2;
//  screen = (char *)0xb8000 + (THERM_Y*160 + THERM_X*2);
    hgotoxy(THERM_X, THERM_Y);
    for (i = 0; i < progress; i++)
    {
//      *screen++ = 0xdb;
//      *screen++ = 0x2a;
        hput(0xdb, 0x2a);
    }
#endif
#endif
}

#ifdef __WATCOMC__
void blitStartup(void)
{
    byte *textScreen;

    if (debugmode)
    {
        return;
    }

    // Blit main screen
    textScreen = (byte *) 0xb8000;
    memcpy(textScreen, startup, 4000);

    // Print version string
    _setbkcolor(4);             // Red
    _settextcolor(14);          // Yellow
    _settextposition(3, 47);
    _outtext(HERETIC_VERSION_TEXT);

    // Hide cursor
    _settextcursor(0x2000);
}
#endif

char tmsg[300];
void tprintf(char *msg, int initflag)
{
#ifdef __WATCOMC__
    char temp[80];
    int start;
    int add;
    int i;

    if (initflag)
        tmsg[0] = 0;
    strcat(tmsg, msg);
    blitStartup();
    DrawThermo();
    _setbkcolor(4);
    _settextcolor(15);
    for (add = start = i = 0; i <= strlen(tmsg); i++)
        if ((tmsg[i] == '\n') || (!tmsg[i]))
        {
            memset(temp, 0, 80);
            strncpy(temp, tmsg + start, i - start);
            _settextposition(MSG_Y + add, 40 - strlen(temp) / 2);
            _outtext(temp);
            start = i + 1;
            add++;
        }
    _settextposition(25, 1);
    drawstatus();
#else
    printf("%s", msg);
#endif
}

void CheckAbortStartup(void)
{
#ifdef __WATCOMC__
    extern int lastpress;

    if (lastpress == 1)
    {                           // Abort if escape pressed
        CleanExit();
    }
#endif
}

void IncThermo(void)
{
    thermCurrent++;
    DrawThermo();
    CheckAbortStartup();
}

void InitThermo(int max)
{
    thermMax = max;
    thermCurrent = 0;
}

#ifdef __WATCOMC__
void CleanExit(void)
{
    union REGS regs;

    I_ShutdownKeyboard();
    regs.x.eax = 0x3;
    int386(0x10, &regs, &regs);
    printf("Exited from HERETIC.\n");
    exit(1);
}
#endif

//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
    extern int screenblocks;
    extern int snd_Channels;
    int i;

    I_BindVariables();
    M_BindBaseControls();
    M_BindHereticControls();

    M_BindVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindVariable("sfx_volume",             &snd_MaxVolume);
    M_BindVariable("music_volume",           &snd_MusicVolume);
    M_BindVariable("screenblocks",           &screenblocks);
    M_BindVariable("snd_channels",           &snd_Channels);
    M_BindVariable("show_endoom",            &show_endoom);

    for (i=0; i<10; ++i)
    {
        char buf[12];

        sprintf(buf, "chatmacro%i", i);
        M_BindVariable(buf, &chat_macros[i]);
    }
}

// 
// Called at exit to display the ENDOOM screen (ENDTEXT in Heretic)
//

static void D_Endoom(void)
{
    byte *endoom_data;

    // Disable ENDOOM?

    if (!show_endoom)
    {
        return;
    }

    endoom_data = W_CacheLumpName("ENDTEXT", PU_STATIC);

    I_Endoom(endoom_data);
}

//---------------------------------------------------------------------------
//
// PROC D_DoomMain
//
//---------------------------------------------------------------------------

void D_DoomMain(void)
{
    int p;
    char file[256];

    I_PrintBanner(PACKAGE_STRING);

    I_AtExit(D_Endoom, false);

    M_FindResponseFile();
    setbuf(stdout, NULL);
    nomonsters = M_CheckParm("-nomonsters");
    respawnparm = M_CheckParm("-respawn");
    ravpic = M_CheckParm("-ravpic");
    noartiskip = M_CheckParm("-noartiskip");
    debugmode = M_CheckParm("-debug");
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

    // Check for -CDROM
    cdrom = false;
#ifdef __WATCOMC__
    if (M_CheckParm("-cdrom"))
    {
        cdrom = true;
        mkdir("c:\\heretic.cd");
    }
#endif

//
// get skill / episode / map from parms
//
    if (M_CheckParm("-deathmatch"))
    {
        deathmatch = true;
    }

    p = M_CheckParm("-skill");
    if (p && p < myargc - 1)
    {
        startskill = myargv[p + 1][0] - '1';
        autostart = true;
    }

    p = M_CheckParm("-episode");
    if (p && p < myargc - 1)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = 1;
        autostart = true;
    }

    p = M_CheckParm("-warp");
    if (p && p < myargc - 2)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = myargv[p + 2][0] - '0';
        autostart = true;
    }

//
// init subsystems
//
    printf("V_Init: allocate screens.\n");
    V_Init();

    M_SetConfigDir();

    // Load defaults before initing other systems
    printf("M_LoadDefaults: Load system defaults.\n");
    D_BindVariables();
    M_SetConfigFilenames("heretic.cfg", PROGRAM_PREFIX "heretic.cfg");
    M_LoadDefaults();

    I_AtExit(M_SaveDefaults, false);

    printf("Z_Init: Init zone memory allocation daemon.\n");
    Z_Init();

    printf("W_Init: Init WADfiles.\n");

    iwadfile = D_FindIWAD(iwads, &gamemission);

    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate. No IWAD was found. Try specifying\n"
                "one with the '-iwad' command line parameter.");
    }

    D_AddFile(iwadfile);

    // -FILE [filename] [filename] ...
    // Add files to the wad list.
    p = M_CheckParm("-file");

    if (p)
    {
        char *filename;

        // the parms after p are wadfile/lump names, until end of parms
        // or another - preceded parm

        while (++p != myargc && myargv[p][0] != '-')
        {
            filename = D_FindWADByName(myargv[p]);
            D_AddFile(filename);
        }
    }

    p = M_CheckParm("-playdemo");
    if (!p)
    {
        p = M_CheckParm("-timedemo");
    }
    if (p && p < myargc - 1)
    {
        sprintf(file, "%s.lmp", myargv[p + 1]);
        D_AddFile(file);
        printf("Playing demo %s.lmp.\n", myargv[p + 1]);
    }

    if (W_CheckNumForName("E2M1") == -1)
    {
        gamemode = shareware;
        gamedescription = "Heretic (shareware)";
    }
    else if (W_CheckNumForName("EXTENDED") != -1)
    {
        // Presence of the EXTENDED lump indicates the retail version

        gamemode = retail;
        gamedescription = "Heretic: Shadow of the Serpent Riders";
    }
    else
    {
        gamemode = registered;
        gamedescription = "Heretic (registered)";
    }

    I_PrintStartupBanner(gamedescription);

#ifdef __WATCOMC__
    I_StartupKeyboard();
    I_StartupJoystick();
#endif
    startup = W_CacheLumpName("LOADING", PU_CACHE);
#ifdef __WATCOMC__
    blitStartup();
#endif

    //
    //  Build status bar line!
    //
    smsg[0] = 0;
    if (deathmatch)
        status("DeathMatch...");
    if (nomonsters)
        status("No Monsters...");
    if (respawnparm)
        status("Respawning...");
    if (autostart)
    {
        char temp[64];
        sprintf(temp, "Warp to Episode %d, Map %d, Skill %d ",
                startepisode, startmap, startskill + 1);
        status(temp);
    }
    wadprintf();                // print the added wadfiles

    tprintf("MN_Init: Init menu system.\n", 1);
    MN_Init();

    CT_Init();

    tprintf("R_Init: Init Heretic refresh daemon.", 1);
    hgotoxy(17, 7);
    hprintf("Loading graphics", 0x3f);
    R_Init();
    tprintf("\n", 0);

    tprintf("P_Init: Init Playloop state.\n", 1);
    hgotoxy(17, 8);
    hprintf("Init game engine.", 0x3f);
    P_Init();
    IncThermo();

    tprintf("I_Init: Setting up machine state.\n", 1);
    I_Init();
    IncThermo();

    tprintf("S_Init: Setting up sound.\n", 1);
    S_Init();
    //IO_StartupTimer();
    S_Start();

    tprintf("D_CheckNetGame: Checking network game status.\n", 1);
    hgotoxy(17, 9);
    hprintf("Checking network game status.", 0x3f);
    D_CheckNetGame();
    IncThermo();

#ifdef __WATCOMC__
    I_CheckExternDriver();      // Check for an external device driver
#endif

    tprintf("SB_Init: Loading patches.\n", 1);
    SB_Init();
    IncThermo();

//
// start the apropriate game based on parms
//

    D_CheckRecordFrom();

    p = M_CheckParm("-record");
    if (p && p < myargc - 1)
    {
        G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParm("-playdemo");
    if (p && p < myargc - 1)
    {
        singledemo = true;      // Quit after one demo
        G_DeferedPlayDemo(myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParm("-timedemo");
    if (p && p < myargc - 1)
    {
        G_TimeDemo(myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParm("-loadgame");
    if (p && p < myargc - 1)
    {
        if (cdrom)
        {
            sprintf(file, SAVEGAMENAMECD "%c.hsg", myargv[p + 1][0]);
        }
        else
        {
            sprintf(file, SAVEGAMENAME "%c.hsg", myargv[p + 1][0]);
        }
        G_LoadGame(file);
    }

    // Check valid episode and map
    if (autostart || netgame)
    {
        if (!D_ValidEpisodeMap(gamemission, gamemode, startepisode, startmap))
        {
            startepisode = 1;
            startmap = 1;
        }
    }

    if (gameaction != ga_loadgame)
    {
        UpdateState |= I_FULLSCRN;
        BorderNeedRefresh = true;
        if (autostart || netgame)
        {
            G_InitNew(startskill, startepisode, startmap);
        }
        else
        {
            D_StartTitle();
        }
    }
#ifdef __WATCOMC__
    _settextcursor(0x0607);     // bring the cursor back
#endif
    D_DoomLoop();               // Never returns
}
