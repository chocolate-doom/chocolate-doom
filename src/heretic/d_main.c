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

// D_main.c

#include <stdio.h>
#include <stdlib.h>

#include "txt_main.h"
#include "txt_io.h"

#include "net_client.h"

#include "config.h"
#include "ct_chat.h"
#include "doomdef.h"
#include "deh_main.h"
#include "d_iwad.h"
#include "i_endoom.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_sound.h"
#include "i_swap.h" // [crispy] SHORT()
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "a11y.h" // [crispy] A11Y
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "w_main.h"
#include "v_video.h"
#include "am_map.h"
#include "v_trans.h" // [crispy] dp_translation

#include "heretic_icon.c"

#define CT_KEY_GREEN    'g'
#define CT_KEY_YELLOW   'y'
#define CT_KEY_RED      'r'
#define CT_KEY_BLUE     'b'

#define STARTUP_WINDOW_X 17
#define STARTUP_WINDOW_Y 7

GameMode_t gamemode = indetermined;
const char *gamedescription = "unknown";

boolean nomonsters;             // checkparm of -nomonsters
boolean respawnparm;            // checkparm of -respawn
boolean debugmode;              // checkparm of -debug
boolean ravpic;                 // checkparm of -ravpic
boolean coop_spawns = false;    // [crispy] checkparm of -coop_spawns
boolean cdrom;                  // true if cd-rom mode active
boolean noartiskip;             // whether shift-enter skips an artifact

skill_t startskill;
int startepisode;
int startmap;
int UpdateState;
static int graphical_startup = 0;
static boolean using_graphical_startup;
static boolean main_loop_started = false;
boolean autostart;

boolean advancedemo;

FILE *debugfile;

static int show_endoom = 0;

void D_ConnectNetGame(void);
void D_CheckNetGame(void);
void D_PageDrawer(void);
void D_AdvanceDemo(void);
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
// PROC DrawCenterMessage
//
// [crispy]
//
//---------------------------------------------------------------------------

void DrawCenterMessage(void)
{
    player_t* player;

    player = &players[consoleplayer];
    if (player->centerMessageTics <= 0 || !player->centerMessage)
    {                           // No message
        return;
    }
    // Place message above quit game message position so they don't overlap
    dp_translation = cr[CR_GOLD];
    MN_DrTextA(player->centerMessage, 160 - MN_TextAWidth(player->centerMessage) / 2, 70);
    dp_translation = NULL;
}

//---------------------------------------------------------------------------
//
// PROC D_Display
//
// Draw current display, possibly wiping it from the previous.
//
//---------------------------------------------------------------------------

int left_widget_w, right_widget_w; // [crispy]

static void CrispyDrawStats (void)
{
    static short height, coord_x, coord_w;
    char str[32];
    player_t *const player = &players[consoleplayer];
    int left_widget_x, right_widget_x;

    if (!height || !coord_x || !coord_w)
    {
        const int FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
        const patch_t *const p = W_CacheLumpNum(FontABaseLump + 'A' - 33, PU_CACHE);

        height = SHORT(p->height) + 1;
        coord_w = 7 * SHORT(p->width);
        coord_x = ORIGWIDTH - coord_w;
    }

    left_widget_w = right_widget_w = 0;
    left_widget_x = 0 - WIDESCREENDELTA;
    right_widget_x = coord_x + WIDESCREENDELTA;

    if (crispy->automapstats == WIDGETS_ALWAYS
            || (automapactive && (crispy->automapstats == WIDGETS_AUTOMAP
                                || crispy->automapstats == WIDGETS_STBAR))
            || (screenblocks > 10 && crispy->automapstats == WIDGETS_STBAR))
    {
        M_snprintf(str, sizeof(str), "K %d/%d", player->killcount, totalkills);
        MN_DrTextA(str, left_widget_x, 1*height);
        left_widget_w = MN_TextAWidth(str); // Assume that kills is longest string

        M_snprintf(str, sizeof(str), "I %d/%d", player->itemcount, totalitems);
        MN_DrTextA(str, left_widget_x, 2*height);

        M_snprintf(str, sizeof(str), "S %d/%d", player->secretcount, totalsecret);
        MN_DrTextA(str, left_widget_x, 3*height);
    }
    else if (crispy->automapstats == WIDGETS_STBAR)
    {
        M_snprintf(str, sizeof(str), "K %d/%d I %d/%d S %d/%d",
                    player->killcount, totalkills,
                    player->itemcount, totalitems,
                    player->secretcount, totalsecret);
        MN_DrTextA(str, 20, 145); // same location as level name in automap
    }

    if (crispy->leveltime == WIDGETS_ALWAYS || (automapactive && crispy->leveltime == WIDGETS_AUTOMAP))
    {
        const int time = leveltime / TICRATE;

        M_snprintf(str, sizeof(str), "%02d:%02d", time/60, time%60);
        MN_DrTextA(str, left_widget_x, 4*height);
    }

    if (crispy->playercoords == WIDGETS_ALWAYS || (automapactive && crispy->playercoords == WIDGETS_AUTOMAP))
    {
        right_widget_w = coord_w;

        M_snprintf(str, sizeof(str), "X %-5d", player->mo->x>>FRACBITS);
        MN_DrTextA(str, right_widget_x, 1*height);

        M_snprintf(str, sizeof(str), "Y %-5d", player->mo->y>>FRACBITS);
        MN_DrTextA(str, right_widget_x, 2*height);

        M_snprintf(str, sizeof(str), "A %-5d", player->mo->angle/ANG1);
        MN_DrTextA(str, right_widget_x, 3*height);

        if (player->cheats & CF_SHOWFPS)
        {
            M_snprintf(str, sizeof(str), "%d FPS", crispy->fps);
            MN_DrTextA(str, right_widget_x, 4*height + 1);
        }
    }
    else if (player->cheats & CF_SHOWFPS)
    {
        right_widget_w = coord_w;

        M_snprintf(str, sizeof(str), "%d FPS", crispy->fps);
        MN_DrTextA(str, right_widget_x, 1*height);
    }
}

void D_Display(void)
{
    if (crispy->uncapped)
    {
        I_UpdateFracTic();

        if (!automapactive || crispy->automapoverlay)
        {
            I_StartDisplay();
            G_FastResponder();
            G_PrepTiccmd();
        }
    }

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
            if (automapactive && !crispy->automapoverlay)
            {
                // [crispy] update automap while playing
                R_RenderPlayerView (&players[displayplayer]);
                AM_Drawer();
            }
            else
                R_RenderPlayerView(&players[displayplayer]);
            if (automapactive && crispy->automapoverlay)
            {
                AM_Drawer();
                BorderNeedRefresh = true;
            }
            // [crispy] check for translucent HUD
            SB_Translucent(TRANSLUCENT_HUD && (!automapactive || crispy->automapoverlay));
            CT_Drawer();
            UpdateState |= I_FULLVIEW;
            SB_Drawer();
            CrispyDrawStats();
            SB_Translucent(false);
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

    if (testcontrols)
    {
        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    if (paused && !MenuActive && !askforquit)
    {
        if (!netgame)
        {
            V_DrawPatch(160, (viewwindowy >> crispy->hires) + 5, W_CacheLumpName(DEH_String("PAUSED"),
                                                              PU_CACHE));
        }
        else
        {
            V_DrawPatch(160, 70, W_CacheLumpName(DEH_String("PAUSED"), PU_CACHE));
        }
    }

    // [crispy] check for translucent HUD
    SB_Translucent(TRANSLUCENT_HUD && (!automapactive || crispy->automapoverlay));

    // Handle player messages
    DrawMessage();

    // [crispy] Handle centered player messages
    DrawCenterMessage();

    SB_Translucent(false);

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

    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}

//---------------------------------------------------------------------------
//
// PROC D_DoomLoop
//
//---------------------------------------------------------------------------

void D_DoomLoop(void)
{
    // [crispy] update the "singleplayer" variable
    CheckCrispySingleplayer(!demorecording && gameaction != ga_playdemo && !netgame);

    if (!crispy->singleplayer)
    {
        int i;

        const struct {
            boolean feature;
            const char *option;
        } custom_options[] = {
            {crispy->moreammo,   "-moreammo"},
            {crispy->fast,       "-fast"},
            {crispy->pistolstart, "-wandstart"},
            {crispy->autohealth, "-autohealth"},
        };

        for (i = 0; i < arrlen(custom_options); i++)
        {
            if (custom_options[i].feature)
            {
                I_Error("The %s option is not supported\n"
                        "for demos and network play.",
                        custom_options[i].option);
            }
        }
    }

    if (M_CheckParm("-debugfile"))
    {
        char filename[20];
        M_snprintf(filename, sizeof(filename), "debug%i.txt", consoleplayer);
        debugfile = M_fopen(filename, "w");
    }
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_RegisterWindowIcon(heretic_icon_data, heretic_icon_w, heretic_icon_h);
    I_InitGraphics();

    main_loop_started = true;

    while (1)
    {
        static int oldgametic;
        // Frame syncronous IO operations
        I_StartFrame();

        // Process one or more tics
        // Will run at least one tic
        TryRunTics();

        if (oldgametic < gametic)
        {
            // Move positional sounds
            S_UpdateSounds(players[consoleplayer].mo);
            oldgametic = gametic;
        }
        D_Display();

        // [crispy] post-rendering function pointer to apply config changes
        // that affect rendering and that are better applied after the current
        // frame has finished rendering
        if (crispy->post_rendering_hook)
        {
            crispy->post_rendering_hook();
            crispy->post_rendering_hook = NULL;
        }
    }
}

/*
===============================================================================

						DEMO LOOP

===============================================================================
*/

static int demosequence;
static int pagetic;
static const char *pagename;


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

void D_PageDrawer(void)
{
    V_DrawFullscreenRawOrPatch(W_GetNumForName(pagename));
    if (demosequence == 1)
    {
        V_DrawPatch(4, 160, W_CacheLumpName(DEH_String("ADVISOR"), PU_CACHE));
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
    demoextend = false; // [crispy] at this point demos should no longer be extended (demo-reel)
    gameaction = ga_nothing;
    demosequence = (demosequence + 1) % 7;
    switch (demosequence)
    {
        case 0:
            pagetic = 210;
            gamestate = GS_DEMOSCREEN;
            pagename = DEH_String("TITLE");
            S_StartSong(mus_titl, false);
            break;
        case 1:
            pagetic = 140;
            gamestate = GS_DEMOSCREEN;
            pagename = DEH_String("TITLE");
            break;
        case 2:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            G_DeferedPlayDemo(DEH_String("demo1"));
            break;
        case 3:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            pagename = DEH_String("CREDIT");
            break;
        case 4:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            G_DeferedPlayDemo(DEH_String("demo2"));
            break;
        case 5:
            pagetic = 200;
            gamestate = GS_DEMOSCREEN;
            if (gamemode == shareware)
            {
                pagename = DEH_String("ORDER");
            }
            else
            {
                pagename = DEH_String("CREDIT");
            }
            break;
        case 6:
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            G_DeferedPlayDemo(DEH_String("demo3"));
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
    char *filename;

    //!
    // @vanilla
    // @category demo
    // @arg <savenum> <demofile>
    //
    // Record a demo, loading from the given filename. Equivalent
    // to -loadgame <savenum> -record <demofile>.

    p = M_CheckParmWithArgs("-recordfrom", 2);
    if (!p)
        return;

    filename = SV_Filename(myargv[p + 1][0] - '0');
    G_LoadGame(filename);
    G_DoLoadGame();             // load the gameskill etc info from savegame

    G_RecordDemo(gameskill, 1, gameepisode, gamemap, myargv[p + 2]);
    D_DoomLoop();               // never returns
    free(filename);
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

char *iwadfile;


void wadprintf(void)
{
    if (debugmode)
    {
        return;
    }
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
#define THERM_X     14
#define THERM_Y     14

int thermMax;
int thermCurrent;
char smsg[80];                  // status bar line

//
//  Heretic startup screen shit
//

static int startup_line = STARTUP_WINDOW_Y;

void hprintf(const char *string)
{
    if (using_graphical_startup)
    {
        TXT_BGColor(TXT_COLOR_CYAN, 0);
        TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

        TXT_GotoXY(STARTUP_WINDOW_X, startup_line);
        ++startup_line;
        TXT_Puts(string);

        TXT_UpdateScreen();
    }

    // haleyjd: shouldn't be WATCOMC-only
    if (debugmode)
        puts(string);
}

void drawstatus(void)
{
    int i;

    TXT_GotoXY(1, 24);
    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    for (i=0; smsg[i] != '\0'; ++i) 
    {
        TXT_PutChar(smsg[i]);
    }
}

static void status(const char *string)
{
    if (using_graphical_startup)
    {
        M_StringConcat(smsg, string, sizeof(smsg));
        drawstatus();
    }
}

void DrawThermo(void)
{
    static int last_progress = -1;
    int progress;
    int i;

    if (!using_graphical_startup)
    {
        return;
    }

    // No progress? Don't update the screen.

    progress = (50 * thermCurrent) / thermMax + 2;

    if (last_progress == progress)
    {
        return;
    }

    last_progress = progress;

    TXT_GotoXY(THERM_X, THERM_Y);

    TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
    TXT_BGColor(TXT_COLOR_GREEN, 0);

    for (i = 0; i < progress; i++)
    {
        TXT_PutChar(0xdb);
    }

    TXT_UpdateScreen();
}

void initStartup(void)
{
    byte *textScreen;
    byte *loading;

    if (!graphical_startup || debugmode || testcontrols)
    {
        using_graphical_startup = false;
        return;
    }

    if (!TXT_Init()) 
    {
        using_graphical_startup = false;
        return;
    }

    I_InitWindowTitle();
    I_InitWindowIcon();

    // Blit main screen
    textScreen = TXT_GetScreenData();
    loading = W_CacheLumpName(DEH_String("LOADING"), PU_CACHE);
    memcpy(textScreen, loading, 4000);

    // Print version string

    TXT_BGColor(TXT_COLOR_RED, 0);
    TXT_FGColor(TXT_COLOR_YELLOW);
    TXT_GotoXY(46, 2);
    TXT_Puts(HERETIC_VERSION_TEXT);

    TXT_UpdateScreen();

    using_graphical_startup = true;
}

static void finishStartup(void)
{
    if (using_graphical_startup)
    {
        TXT_Shutdown();
    }
}

char tmsg[300];
void tprintf(const char *msg, int initflag)
{
    printf("%s", msg);
}

// haleyjd: moved up, removed WATCOMC code
void CleanExit(void)
{
    DEH_printf("Exited from HERETIC.\n");
    exit(1);
}

void CheckAbortStartup(void)
{
    // haleyjd: removed WATCOMC
    // haleyjd FIXME: this should actually work in text mode too, but how to
    // get input before SDL video init?
    if(using_graphical_startup)
    {
        if(TXT_GetChar() == 27)
            CleanExit();
    }
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

//
// Add configuration file variable bindings.
//

void D_BindVariables(void)
{
    int i;

    M_ApplyPlatformDefaults();

    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
    M_BindHereticControls();
    M_BindWeaponControls();
    M_BindChatControls(MAXPLAYERS);

    key_multi_msgplayer[0] = CT_KEY_GREEN;
    key_multi_msgplayer[1] = CT_KEY_YELLOW;
    key_multi_msgplayer[2] = CT_KEY_RED;
    key_multi_msgplayer[3] = CT_KEY_BLUE;

    M_BindMenuControls();
    M_BindMapControls();

    NET_BindVariables();

    M_BindIntVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindIntVariable("mouse_sensitivity_x2",   &mouseSensitivity_x2);
    M_BindIntVariable("mouse_sensitivity_y",    &mouseSensitivity_y);
    M_BindIntVariable("sfx_volume",             &snd_MaxVolume);
    M_BindIntVariable("music_volume",           &snd_MusicVolume);
    M_BindIntVariable("screenblocks",           &screenblocks);
    M_BindIntVariable("snd_channels",           &snd_Channels);
    M_BindIntVariable("a11y_sector_lighting",   &a11y_sector_lighting);
    M_BindIntVariable("a11y_extra_lighting",    &a11y_extra_lighting);
    M_BindIntVariable("a11y_weapon_flash",      &a11y_weapon_flash);
    M_BindIntVariable("a11y_weapon_pspr",       &a11y_weapon_pspr);
    M_BindIntVariable("a11y_palette_changes",   &a11y_palette_changes);
    M_BindIntVariable("a11y_invul_colormap",    &a11y_invul_colormap);
    M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
    M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    M_BindIntVariable("show_endoom",            &show_endoom);
    M_BindIntVariable("graphical_startup",      &graphical_startup);

    for (i=0; i<10; ++i)
    {
        char buf[12];

        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }

    // [crispy] bind "crispness" config variables
    M_BindIntVariable("crispy_hires",           &crispy->hires);
    M_BindIntVariable("crispy_smoothscaling",   &smooth_pixel_scaling);
    M_BindIntVariable("crispy_automapoverlay",  &crispy->automapoverlay);
    M_BindIntVariable("crispy_automaprotate",   &crispy->automaprotate);
    M_BindIntVariable("crispy_automapstats",    &crispy->automapstats);
    M_BindIntVariable("crispy_brightmaps",      &crispy->brightmaps);
    M_BindIntVariable("crispy_bobfactor",       &crispy->bobfactor);
    M_BindIntVariable("crispy_centerweapon",    &crispy->centerweapon);
    M_BindIntVariable("crispy_defaultskill",    &crispy->defaultskill);
    M_BindIntVariable("crispy_fpslimit",        &crispy->fpslimit);
    M_BindIntVariable("crispy_freelook",        &crispy->freelook_hh);
    M_BindIntVariable("crispy_gamma",           &crispy->gamma);
    M_BindIntVariable("crispy_leveltime",       &crispy->leveltime);
    M_BindIntVariable("crispy_mouselook",       &crispy->mouselook);
    M_BindIntVariable("crispy_playercoords",    &crispy->playercoords);
    M_BindIntVariable("crispy_secretmessage",   &crispy->secretmessage);
    M_BindIntVariable("crispy_smoothlight",     &crispy->smoothlight);
    M_BindIntVariable("crispy_soundmono",       &crispy->soundmono);
    M_BindIntVariable("crispy_lvlwpnsnd",       &crispy->lvlwpnsnd);
    M_BindIntVariable("crispy_translucency",    &crispy->translucency);
#ifdef CRISPY_TRUECOLOR
    M_BindIntVariable("crispy_truecolor",       &crispy->truecolor);
#endif
    M_BindIntVariable("crispy_uncapped",        &crispy->uncapped);
    M_BindIntVariable("crispy_vsync",           &crispy->vsync);
    M_BindIntVariable("crispy_widescreen",      &crispy->widescreen);
}

// 
// Called at exit to display the ENDOOM screen (ENDTEXT in Heretic)
//

static void D_Endoom(void)
{
    byte *endoom_data;

    // Disable ENDOOM?

    if (!show_endoom || testcontrols || !main_loop_started)
    {
        return;
    }

    endoom_data = W_CacheLumpName(DEH_String("ENDTEXT"), PU_STATIC);

    I_Endoom(endoom_data);
}

static const char *const loadparms[] = {"-file", "-merge", NULL}; // [crispy]

//---------------------------------------------------------------------------
//
// PROC D_DoomMain
//
//---------------------------------------------------------------------------

void D_DoomMain(void)
{
    GameMission_t gamemission;
    int p;
    char file[256];
    char demolumpname[9];

    I_PrintBanner(PACKAGE_STRING);

    I_AtExit(D_Endoom, false);

    //!
    // @category game
    // @vanilla
    //
    // Disable monsters.
    //

    nomonsters = M_ParmExists("-nomonsters");

    //!
    // @category game
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_ParmExists("-respawn");

    //!
    // @vanilla
    //
    // Take screenshots when F1 is pressed.
    //

    ravpic = M_ParmExists("-ravpic");

    //!
    // @category obscure
    // @vanilla
    //
    // Allow artifacts to be used when the run key is held down.
    //

    noartiskip = M_ParmExists("-noartiskip");

    debugmode = M_ParmExists("-debug");
    startepisode = 1;
    startmap = 1;
    autostart = false;

    crispy->freelook = FREELOOK_LOCK;
//
// get skill / episode / map from parms
//

    //!
    // @vanilla
    // @category net
    //
    // Start a deathmatch game.
    //

    if (M_ParmExists("-deathmatch"))
    {
        deathmatch = true;
    }

    //!
    // @category game
    // @arg <n>
    // @vanilla
    //
    // Start playing on episode n (1-4)
    //

    p = M_CheckParmWithArgs("-episode", 1);
    if (p)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = 1;
        autostart = true;
    }

    //!
    // @category game
    // @arg <x> <y>
    // @vanilla
    //
    // Start a game immediately, warping to level ExMy.
    //

    p = M_CheckParmWithArgs("-warp", 2);
    if (p && p < myargc - 2)
    {
        startepisode = myargv[p + 1][0] - '0';
        startmap = myargv[p + 2][0] - '0';
        autostart = true;

        // [crispy] if used with -playdemo, fast-forward demo up to the desired map
        crispy->demowarp = startmap;
    }

//
// init subsystems
//
    DEH_printf("V_Init: allocate screens.\n");
    V_Init();

    // Check for -CDROM

    cdrom = false;

#ifdef _WIN32

    //!
    // @category obscure
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\heretic.cd,
    // allowing play from CD.
    //

    if (M_CheckParm("-cdrom"))
    {
        cdrom = true;
    }
#endif

    if (cdrom)
    {
        M_SetConfigDir(DEH_String("c:\\heretic.cd"));
    }
    else
    {
        M_SetConfigDir(NULL);
    }

    // Load defaults before initing other systems
    DEH_printf("M_LoadDefaults: Load system defaults.\n");
    D_BindVariables();
    M_SetConfigFilenames("heretic.cfg", PROGRAM_PREFIX "heretic.cfg");
    M_LoadDefaults();

    I_AtExit(M_SaveDefaults, false);

    // [crispy] Set startskill after loading config to account for defaultskill
    startskill = (crispy->defaultskill + SKILL_HMP) % NUM_SKILLS; // [crispy]

    //!
    // @category game
    // @arg <skill>
    // @vanilla
    //
    // Set the game skill, 1-5 (1: easiest, 5: hardest).  A skill of
    // 0 disables all monsters.
    //

    p = M_CheckParmWithArgs("-skill", 1);
    if (p)
    {
        startskill = myargv[p + 1][0] - '1';
        autostart = true;
    }

    DEH_printf("Z_Init: Init zone memory allocation daemon.\n");
    Z_Init();

    DEH_printf("W_Init: Init WADfiles.\n");

    iwadfile = D_FindIWAD(IWAD_MASK_HERETIC, &gamemission);

    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate. No IWAD was found. Try specifying\n"
                "one with the '-iwad' command line parameter.");
    }

    D_AddFile(iwadfile);
    W_CheckCorrectIWAD(heretic);

    //!
    // @category game
    //
    // Automatic wand start when advancing from one level to the next. At the
    // beginning of each level, the player's health is reset to 100, their
    // armor to 0 and their inventory is reduced to the following: wand, staff
    // and 50 ammo for the wand. This option is not allowed when recording a
    // demo, playing back a demo or when starting a network game.
    //

    crispy->pistolstart = M_ParmExists("-wandstart");

    //!
    // @category game
    //
    // Ammo pickups give 50% more ammo. This option is not allowed when recording a
    // demo, playing back a demo or when starting a network game.
    //

    crispy->moreammo = M_ParmExists("-moreammo");

    //!
    // @category game
    //
    // Fast monsters. This option is not allowed when recording a demo,
    // playing back a demo or when starting a network game.
    //

    crispy->fast = M_ParmExists("-fast");

    //!
    // @category game
    //
    // Automatic use of Quartz flasks and Mystic urns.
    //

    crispy->autohealth = M_ParmExists("-autohealth");

    //!
    // @category game
    //
    // Show the location of keys on the automap.
    //

    crispy->keysloc = M_ParmExists("-keysloc");

    //!
    // @category mod
    //
    // Disable auto-loading of .wad files.
    //
    if (!M_ParmExists("-noautoload") && gamemode != shareware)
    {
        char *autoload_dir;
        autoload_dir = M_GetAutoloadDir("heretic.wad", true);
        if (autoload_dir != NULL)
        {
            DEH_AutoLoadPatches(autoload_dir);
            W_AutoLoadWADs(autoload_dir);
            free(autoload_dir);
        }
    }

    // Load dehacked patches specified on the command line.
    DEH_ParseCommandLine();

    // Load PWAD files.
    W_ParseCommandLine();

    // [crispy] add wad files from autoload PWAD directories

    if (!M_ParmExists("-noautoload") && gamemode != shareware)
    {
        int i;

        for (i = 0; loadparms[i]; i++)
        {
            int p;
            p = M_CheckParmWithArgs(loadparms[i], 1);
            if (p)
            {
                while (++p != myargc && myargv[p][0] != '-')
                {
                    char *autoload_dir;
                    if ((autoload_dir = M_GetAutoloadDir(M_BaseName(myargv[p]), false)))
                    {
                        W_AutoLoadWADs(autoload_dir);
                        free(autoload_dir);
                    }
                }
            }
        }
    }

    if (W_CheckNumForName("HEHACKED") != -1)
    {
        DEH_LoadLumpByName("HEHACKED", true, true);
    }

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (!p)
    {
        //!
        // @arg <demo>
        // @category demo
        // @vanilla
        //
        // Play back the demo named demo.lmp, determining the framerate
        // of the screen.
        //

        p = M_CheckParmWithArgs("-timedemo", 1);
    }

    if (p)
    {
        char *uc_filename = strdup(myargv[p + 1]);
        M_ForceUppercase(uc_filename);

        // In Vanilla, the filename must be specified without .lmp,
        // but make that optional.
        if (M_StringEndsWith(uc_filename, ".LMP"))
        {
            M_StringCopy(file, myargv[p + 1], sizeof(file));
        }
        else
        {
            DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p + 1]);
        }

        free(uc_filename);

        if (D_AddFile(file))
        {
            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name,
                         sizeof(demolumpname));
        }
        else
        {
            // The file failed to load, but copy the original arg as a
            // demo name to make tricks like -playdemo demo1 possible.
            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf("Playing demo %s.\n", file);
    }

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // [crispy] process .deh files from PWADs autoload directories

    if (!M_ParmExists("-noautoload") && gamemode != shareware)
    {
        int i;

        for (i = 0; loadparms[i]; i++)
        {
            int p;
            p = M_CheckParmWithArgs(loadparms[i], 1);
            if (p)
            {
                while (++p != myargc && myargv[p][0] != '-')
                {
                    char *autoload_dir;
                    if ((autoload_dir = M_GetAutoloadDir(M_BaseName(myargv[p]), false)))
                    {
                        DEH_AutoLoadPatches(autoload_dir);
                        free(autoload_dir);
                    }
                }
            }
        }
    }

    //!
    // @category demo
    //
    // Record or playback a demo, automatically quitting
    // after either level exit or player respawn.
    //

    demoextend = (!M_ParmExists("-nodemoextend"));
    //[crispy] make demoextend the default

    if (W_CheckNumForName(DEH_String("E2M1")) == -1)
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

    I_SetWindowTitle(gamedescription);

    savegamedir = M_GetSaveGameDir("heretic.wad");

    I_PrintStartupBanner(gamedescription);

    if (M_ParmExists("-testcontrols"))
    {
        startepisode = 1;
        startmap = 1;
        autostart = true;
        testcontrols = true;
    }

    I_InitTimer();
    I_InitSound(heretic);
    I_InitMusic();

    tprintf("NET_Init: Init network subsystem.\n", 1);
    NET_Init ();

    D_ConnectNetGame();

    // haleyjd: removed WATCOMC
    initStartup();

    //
    //  Build status bar line!
    //
    smsg[0] = 0;
    if (deathmatch)
        status(DEH_String("DeathMatch..."));
    if (nomonsters)
        status(DEH_String("No Monsters..."));
    if (respawnparm)
        status(DEH_String("Respawning..."));
    if (autostart)
    {
        char temp[64];
        DEH_snprintf(temp, sizeof(temp),
                     "Warp to Episode %d, Map %d, Skill %d ",
                     startepisode, startmap, startskill + 1);
        status(temp);
    }
    wadprintf();                // print the added wadfiles

    tprintf(DEH_String("MN_Init: Init menu system.\n"), 1);
    MN_Init();

    CT_Init();

    tprintf(DEH_String("R_Init: Init Heretic refresh daemon."), 1);
    hprintf(DEH_String("Loading graphics"));
    R_Init();
    tprintf("\n", 0);

    tprintf(DEH_String("P_Init: Init Playloop state.\n"), 1);
    hprintf(DEH_String("Init game engine."));
    P_Init();
    IncThermo();

    tprintf(DEH_String("I_Init: Setting up machine state.\n"), 1);
    I_CheckIsScreensaver();
    I_InitJoystick();
    IncThermo();

    tprintf(DEH_String("S_Init: Setting up sound.\n"), 1);
    S_Init();
    //IO_StartupTimer();
    S_Start();

    tprintf(DEH_String("D_CheckNetGame: Checking network game status.\n"), 1);
    hprintf(DEH_String("Checking network game status."));
    D_CheckNetGame();
    IncThermo();

    // haleyjd: removed WATCOMC

    tprintf(DEH_String("SB_Init: Loading patches.\n"), 1);
    SB_Init();
    IncThermo();

//
// start the appropriate game based on params
//

    //!
    // @category game
    //
    // Start single player game with items spawns as in cooperative netgame.
    //

    p = M_ParmExists("-coop_spawns");
    if (p)
    {
        coop_spawns = true;
    }

    D_CheckRecordFrom();

    //!
    // @arg <x>
    // @category demo
    // @vanilla
    //
    // Record a demo named x.lmp.
    //

    p = M_CheckParmWithArgs("-record", 1);
    if (p)
    {
        G_RecordDemo(startskill, 1, startepisode, startmap, myargv[p + 1]);
        D_DoomLoop();           // Never returns
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
        singledemo = true;      // Quit after one demo
        G_DeferedPlayDemo(demolumpname);
        D_DoomLoop();           // Never returns
    }

    // [crispy] we don't play a demo, so don't skip maps
    crispy->demowarp = 0;

    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
        G_TimeDemo(demolumpname);
        D_DoomLoop();           // Never returns
    }

    //!
    // @category game
    // @arg <s>
    // @vanilla
    //
    // Load the game in savegame slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    if (p && p < myargc - 1)
    {
        char *filename;

	filename = SV_Filename(myargv[p + 1][0] - '0');
        G_LoadGame(filename);
	free(filename);
    }

    // Check valid episode and map
    if (autostart || netgame)
    {
        if (!D_ValidEpisodeMap(heretic, gamemode, startepisode, startmap))
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

    finishStartup();

    D_DoomLoop();               // Never returns
}
