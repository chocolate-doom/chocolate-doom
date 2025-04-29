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
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // [crispy] time_t, time(), struct tm, localtime()

#include "config.h"
#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "sounds.h"

#include "d_iwad.h"
#include "d_pwad.h" // [crispy] D_Load{Sigil,Nerve,Masterlevels}Wad()

#include "z_zone.h"
#include "w_main.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_diskicon.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_menu.h"
#include "p_saveg.h"

#include "i_endoom.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"

#include "g_game.h"
#include "a11y.h" // [crispy] A11Y

#include "hu_stuff.h"
#include "v_snow.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "net_client.h"
#include "net_dedicated.h"
#include "net_query.h"

#include "p_setup.h"
#include "r_local.h"
#include "statdump.h"

#include "d_main.h"

#include "doom_icon.c"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);

static char *gamedescription;

// Location where savegames are stored

char *          savegamedir;

// location of IWAD and WAD files

char *          iwadfile;


boolean		devparm;	// started game with -devparm
boolean         nomonsters;	// checkparm of -nomonsters
boolean         respawnparm;	// checkparm of -respawn
boolean         fastparm;	// checkparm of -fast
boolean         coop_spawns = false;	// [crispy] checkparm of -coop_spawns



skill_t		startskill;
int             startepisode;
int		startmap;
boolean		autostart;
int             startloadgame;

boolean		advancedemo;

// Store demo, do not accept any inputs
boolean         storedemo;

// If true, the main game loop has started.
boolean         main_loop_started = false;

int             show_endoom = 0; // [crispy] disable
int             show_diskicon = 1;


void D_ConnectNetGame(void);
void D_CheckNetGame(void);


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*	ev;
	
    // IF STORE DEMO, DO NOT ACCEPT INPUT
    if (storedemo)
        return;
	
    while ((ev = D_PopEvent()) != NULL)
    {
	if (M_Responder (ev))
	    continue;               // menu ate the event
	G_Responder (ev);
    }
}




//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t     wipegamestate = GS_DEMOSCREEN;

boolean D_Display (void)
{
    static  boolean		viewactivestate = false;
    static  boolean		menuactivestate = false;
    static  boolean		inhelpscreensstate = false;
    static  boolean		fullscreen = false;
    static  gamestate_t		oldgamestate = -1;
    static  int			borderdrawcount;
    int				y;
    boolean			wipe;
    boolean			redrawsbar;
		
    redrawsbar = false;
    
    if (crispy->uncapped)
    {
        I_StartDisplay();
        G_FastResponder();
        G_PrepTiccmd();
    }

    // change the view size if needed
    if (setsizeneeded)
    {
	R_ExecuteSetViewSize ();
	oldgamestate = -1;                      // force background redraw
	borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
	wipe = true;
	wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
	wipe = false;

    if (gamestate == GS_LEVEL && gametic)
	HU_Erase();
    
    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
	if (!gametic)
	    break;
	if (automapactive && !crispy->automapoverlay)
	{
	    // [crispy] update automap while playing
	    R_RenderPlayerView (&players[displayplayer]);
	    AM_Drawer ();
	}
	if (wipe || (viewheight != SCREENHEIGHT && fullscreen))
	    redrawsbar = true;
	if (inhelpscreensstate && !inhelpscreens)
	    redrawsbar = true;              // just put away the help screen
	ST_Drawer (viewheight == SCREENHEIGHT, redrawsbar );
	fullscreen = viewheight == SCREENHEIGHT;
	break;

      case GS_INTERMISSION:
	WI_Drawer ();
	break;

      case GS_FINALE:
	F_Drawer ();
	break;

      case GS_DEMOSCREEN:
	D_PageDrawer ();
	break;
    }
    
    // draw buffered stuff to screen
    I_UpdateNoBlit ();
    
    // draw the view directly
    if (gamestate == GS_LEVEL && (!automapactive || crispy->automapoverlay) && gametic)
    {
	R_RenderPlayerView (&players[displayplayer]);

        // [crispy] Crispy HUD
        if (screenblocks >= CRISPY_HUD)
            ST_Drawer(false, true);
    }

    // [crispy] in automap overlay mode,
    // the HUD is drawn on top of everything else
    if (gamestate == GS_LEVEL && gametic && !(automapactive && crispy->automapoverlay))
	HU_Drawer ();
    
    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
#ifndef CRISPY_TRUECOLOR
	I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
#else
	I_SetPalette (0);
#endif

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
	viewactivestate = false;        // view was not active
	R_FillBackScreen ();    // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && (!automapactive || crispy->automapoverlay) && scaledviewwidth != SCREENWIDTH)
    {
	if (menuactive || menuactivestate || !viewactivestate)
	    borderdrawcount = 3;
	if (borderdrawcount)
	{
	    R_DrawViewBorder ();    // erase old menu stuff
	    borderdrawcount--;
	}

    }

    if (testcontrols)
    {
        // Box showing current mouse speed

        V_DrawMouseSpeedBox(testcontrols_mousespeed);
    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;
    
    // [crispy] in automap overlay mode,
    // draw the automap and HUD on top of everything else
    if (automapactive && crispy->automapoverlay)
    {
	AM_Drawer ();
	HU_Drawer ();

	// [crispy] force redraw of status bar and border
	viewactivestate = false;
	inhelpscreensstate = true;
    }

    // [crispy] Snow
    if (crispy->snowflakes)
    {
	V_SnowDraw();

	// [crispy] force redraw of status bar and border
	viewactivestate = false;
	inhelpscreensstate = true;
    }

    // [crispy] draw neither pause pic nor menu when taking a clean screenshot
    if (crispy->cleanscreenshot)
    {
	return false;
    }

    // draw pause pic
    if (paused)
    {
	if (automapactive && !crispy->automapoverlay)
	    y = 4;
	else
	    y = (viewwindowy >> crispy->hires)+4;
	V_DrawPatchDirect((viewwindowx >> crispy->hires) + ((scaledviewwidth >> crispy->hires) - 68) / 2 - WIDESCREENDELTA, y,
                          W_CacheLumpName (DEH_String("M_PAUSE"), PU_CACHE));
    }


    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation

    return wipe;
}

void EnableLoadingDisk(void) // [crispy] un-static
{
    const char *disk_lump_name;

    if (show_diskicon)
    {
        if (M_CheckParm("-cdrom") > 0)
        {
            disk_lump_name = DEH_String("STCDROM");
        }
        else
        {
            disk_lump_name = DEH_String("STDISK");
        }

        V_EnableLoadingDisk(disk_lump_name,
                            SCREENWIDTH - LOADING_DISK_W,
                            SCREENHEIGHT - LOADING_DISK_H);
    }
}

//
// Add configuration file variable bindings.
//


static const char * const chat_macro_defaults[10] =
{
    HUSTR_CHATMACRO0,
    HUSTR_CHATMACRO1,
    HUSTR_CHATMACRO2,
    HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4,
    HUSTR_CHATMACRO5,
    HUSTR_CHATMACRO6,
    HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8,
    HUSTR_CHATMACRO9
};


void D_BindVariables(void)
{
    int i;

    M_ApplyPlatformDefaults();

    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();

    M_BindBaseControls();
    M_BindWeaponControls();
    M_BindMapControls();
    M_BindMenuControls();
    M_BindChatControls(MAXPLAYERS);

    key_multi_msgplayer[0] = HUSTR_KEYGREEN;
    key_multi_msgplayer[1] = HUSTR_KEYINDIGO;
    key_multi_msgplayer[2] = HUSTR_KEYBROWN;
    key_multi_msgplayer[3] = HUSTR_KEYRED;

    NET_BindVariables();

    M_BindIntVariable("mouse_sensitivity",      &mouseSensitivity);
    M_BindIntVariable("mouse_sensitivity_x2",   &mouseSensitivity_x2); // [crispy]
    M_BindIntVariable("mouse_sensitivity_y",    &mouseSensitivity_y); // [crispy]
    M_BindIntVariable("sfx_volume",             &sfxVolume);
    M_BindIntVariable("music_volume",           &musicVolume);
    M_BindIntVariable("show_messages",          &showMessages);
    M_BindIntVariable("screenblocks",           &screenblocks);
    M_BindIntVariable("detaillevel",            &detailLevel);
    M_BindIntVariable("snd_channels",           &snd_channels);
    // [crispy] unconditionally disable savegame and demo limits
//  M_BindIntVariable("vanilla_savegame_limit", &vanilla_savegame_limit);
//  M_BindIntVariable("vanilla_demo_limit",     &vanilla_demo_limit);
    M_BindIntVariable("a11y_sector_lighting",   &a11y_sector_lighting);
    M_BindIntVariable("a11y_extra_lighting",    &a11y_extra_lighting);
    M_BindIntVariable("a11y_weapon_flash",      &a11y_weapon_flash);
    M_BindIntVariable("a11y_weapon_pspr",       &a11y_weapon_pspr);
    M_BindIntVariable("a11y_palette_changes",   &a11y_palette_changes);
    M_BindIntVariable("a11y_invul_colormap",    &a11y_invul_colormap);
    M_BindIntVariable("show_endoom",            &show_endoom);
    M_BindIntVariable("show_diskicon",          &show_diskicon);

    // Multiplayer chat macros

    for (i=0; i<10; ++i)
    {
        char buf[12];

        chat_macros[i] = M_StringDuplicate(chat_macro_defaults[i]);
        M_snprintf(buf, sizeof(buf), "chatmacro%i", i);
        M_BindStringVariable(buf, &chat_macros[i]);
    }

    // [crispy] bind "crispness" config variables
    M_BindIntVariable("crispy_automapoverlay",  &crispy->automapoverlay);
    M_BindIntVariable("crispy_automaprotate",   &crispy->automaprotate);
    M_BindIntVariable("crispy_automapstats",    &crispy->automapstats);
    M_BindIntVariable("crispy_bobfactor",       &crispy->bobfactor);
    M_BindIntVariable("crispy_btusetimer",      &crispy->btusetimer);
    M_BindIntVariable("crispy_brightmaps",      &crispy->brightmaps);
    M_BindIntVariable("crispy_centerweapon",    &crispy->centerweapon);
    M_BindIntVariable("crispy_coloredblood",    &crispy->coloredblood);
    M_BindIntVariable("crispy_coloredhud",      &crispy->coloredhud);
    M_BindIntVariable("crispy_crosshair",       &crispy->crosshair);
    M_BindIntVariable("crispy_crosshairhealth", &crispy->crosshairhealth);
    M_BindIntVariable("crispy_crosshairtarget", &crispy->crosshairtarget);
    M_BindIntVariable("crispy_crosshairtype",   &crispy->crosshairtype);
    M_BindIntVariable("crispy_defaultskill",    &crispy->defaultskill);
    M_BindIntVariable("crispy_demobar",         &crispy->demobar);
    M_BindIntVariable("crispy_demotimer",       &crispy->demotimer);
    M_BindIntVariable("crispy_demotimerdir",    &crispy->demotimerdir);
    M_BindIntVariable("crispy_extautomap",      &crispy->extautomap);
    M_BindIntVariable("crispy_flipcorpses",     &crispy->flipcorpses);
    M_BindIntVariable("crispy_fpslimit",        &crispy->fpslimit);
    M_BindIntVariable("crispy_freeaim",         &crispy->freeaim);
    M_BindIntVariable("crispy_freelook",        &crispy->freelook);
    M_BindIntVariable("crispy_gamma",           &crispy->gamma);
    M_BindIntVariable("crispy_hires",           &crispy->hires);
    M_BindIntVariable("crispy_jump",            &crispy->jump);
    M_BindIntVariable("crispy_leveltime",       &crispy->leveltime);
    M_BindIntVariable("crispy_mouselook",       &crispy->mouselook);
    M_BindIntVariable("crispy_neghealth",       &crispy->neghealth);
    M_BindIntVariable("crispy_overunder",       &crispy->overunder);
    M_BindIntVariable("crispy_pitch",           &crispy->pitch);
    M_BindIntVariable("crispy_playercoords",    &crispy->playercoords);
    M_BindIntVariable("crispy_secretmessage",   &crispy->secretmessage);
    M_BindIntVariable("crispy_smoothlight",     &crispy->smoothlight);
    M_BindIntVariable("crispy_smoothmap",       &crispy->smoothmap);
    M_BindIntVariable("crispy_smoothscaling",   &smooth_pixel_scaling);
    M_BindIntVariable("crispy_soundfix",        &crispy->soundfix);
    M_BindIntVariable("crispy_soundfull",       &crispy->soundfull);
    M_BindIntVariable("crispy_soundmono",       &crispy->soundmono);
    M_BindIntVariable("crispy_statsformat",     &crispy->statsformat);
    M_BindIntVariable("crispy_translucency",    &crispy->translucency);
#ifdef CRISPY_TRUECOLOR
    M_BindIntVariable("crispy_truecolor",       &crispy->truecolor);
#endif
    M_BindIntVariable("crispy_uncapped",        &crispy->uncapped);
    M_BindIntVariable("crispy_vsync",           &crispy->vsync);
    M_BindIntVariable("crispy_widescreen",      &crispy->widescreen);
}

//
// D_GrabMouseCallback
//
// Called to determine whether to grab the mouse pointer
//

boolean D_GrabMouseCallback(void)
{
    // Drone players don't need mouse focus

    if (drone)
        return false;

    // when menu is active or game is paused, release the mouse 
 
    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL) && !demoplayback && !advancedemo;
}

//
//  D_RunFrame
//
void D_RunFrame()
{
    int nowtime;
    int tics;
    static int wipestart;
    static boolean wipe;
    static int oldgametic;

    if (wipe)
    {
        do
        {
            nowtime = I_GetTime ();
            tics = nowtime - wipestart;
            I_Sleep(1);
        } while (tics <= 0);

        wipestart = nowtime;
        wipe = !wipe_ScreenWipe(wipe_Melt
                               , 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
        I_UpdateNoBlit ();
        M_Drawer ();                            // menu is drawn even on top of wipes
        I_FinishUpdate ();                      // page flip or blit buffer
        return;
    }

    // frame syncronous IO operations
    I_StartFrame ();

    TryRunTics (); // will run at least one tic

    if (oldgametic < gametic)
    {
        S_UpdateSounds (players[displayplayer].mo);// move positional sounds
        oldgametic = gametic;
    }

    // Update display, next frame, with current state if no profiling is on
    if (screenvisible && !nodrawers)
    {
        if ((wipe = D_Display ()))
        {
            // start wipe on this frame
            wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

            wipestart = I_GetTime () - 1;
        } else {
            // normal update
            I_FinishUpdate ();              // page flip or blit buffer
        }
    }

	// [crispy] post-rendering function pointer to apply config changes
	// that affect rendering and that are better applied after the current
	// frame has finished rendering
	if (crispy->post_rendering_hook && !wipe)
	{
		crispy->post_rendering_hook();
		crispy->post_rendering_hook = NULL;
	}
}

//
//  D_DoomLoop
//
void D_DoomLoop (void)
{
    if (gamevariant == bfgedition &&
        (demorecording || (gameaction == ga_playdemo) || netgame))
    {
        printf(" WARNING: You are playing using one of the Doom Classic\n"
               " IWAD files shipped with the Doom 3: BFG Edition. These are\n"
               " known to be incompatible with the regular IWAD files and\n"
               " may cause demos and network games to get out of sync.\n");
    }

    // [crispy] no need to write a demo header in demo continue mode
    if (demorecording && gameaction != ga_playdemo)
	G_BeginRecording ();

    main_loop_started = true;

    I_SetWindowTitle(gamedescription);
    I_GraphicsCheckCommandLine();
    I_SetGrabMouseCallback(D_GrabMouseCallback);
    I_RegisterWindowIcon(doom_icon_data, doom_icon_w, doom_icon_h);
    I_InitGraphics();
    EnableLoadingDisk();

    TryRunTics();

    V_RestoreBuffer();
    R_ExecuteSetViewSize();

    D_StartGameLoop();

    if (testcontrols)
    {
        wipegamestate = gamestate;
    }

    while (1)
    {
        D_RunFrame();
    }
}



//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
const char                    *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
	D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
    V_DrawPatchFullScreen (W_CacheLumpName(pagename, PU_CACHE), crispy->fliplevels);
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;
    // [crispy] update the "singleplayer" variable
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);


    // The Ultimate Doom executable changed the demo sequence to add
    // a DEMO4 demo.  Final Doom was based on Ultimate, so also
    // includes this change; however, the Final Doom IWADs do not
    // include a DEMO4 lump, so the game bombs out with an error
    // when it reaches this point in the demo sequence.

    // However! There is an alternate version of Final Doom that
    // includes a fixed executable.

    // [crispy] get rid of this demo sequence breaking bug
    /*
    if (gameversion == exe_ultimate || gameversion == exe_final)
    */
    if (W_CheckNumForName(DEH_String("demo4")) >= 0)
      demosequence = (demosequence+1)%7;
    else
      demosequence = (demosequence+1)%6;
    
    switch (demosequence)
    {
      case 0:
	if ( gamemode == commercial )
	    pagetic = TICRATE * 11;
	else
	    pagetic = 170;
	gamestate = GS_DEMOSCREEN;
	pagename = DEH_String("TITLEPIC");
	if ( gamemode == commercial )
	  S_StartMusic(mus_dm2ttl);
	else
	  S_StartMusic (mus_intro);
	break;
      case 1:
	G_DeferedPlayDemo(DEH_String("demo1"));
	break;
      case 2:
	pagetic = 200;
	gamestate = GS_DEMOSCREEN;
	pagename = DEH_String("CREDIT");
	break;
      case 3:
	G_DeferedPlayDemo(DEH_String("demo2"));
	break;
      case 4:
	gamestate = GS_DEMOSCREEN;
	if ( gamemode == commercial)
	{
	    pagetic = TICRATE * 11;
	    pagename = DEH_String("TITLEPIC");
	    S_StartMusic(mus_dm2ttl);
	}
	else
	{
	    pagetic = 200;

	    if (gameversion >= exe_ultimate)
	      pagename = DEH_String("CREDIT");
	    else
	      pagename = DEH_String("HELP2");
	}
	break;
      case 5:
	G_DeferedPlayDemo(DEH_String("demo3"));
	break;
        // THE DEFINITIVE DOOM Special Edition demo
      case 6:
	G_DeferedPlayDemo(DEH_String("demo4"));
	break;
    }

    // The Doom 3: BFG Edition version of doom2.wad does not have a
    // TITLETPIC lump. Use INTERPIC instead as a workaround.
    if (gamevariant == bfgedition && !strcasecmp(pagename, "TITLEPIC")
        && W_CheckNumForName("titlepic") < 0)
    {
        // [crispy] use DMENUPIC instead of TITLEPIC, it's awesome
        pagename = DEH_String("DMENUPIC");
    }
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
    gameaction = ga_nothing;
    automapactive = false; // [crispy] clear overlaid automap remainings
    demosequence = -1;
    D_AdvanceDemo ();
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static const char *banners[] =
{
    // doom2.wad
    "                         "
    "DOOM 2: Hell on Earth v%i.%i"
    "                           ",
    // doom2.wad v1.666
    "                         "
    "DOOM 2: Hell on Earth v%i.%i66"
    "                          ",
    // doom1.wad
    "                            "
    "DOOM Shareware Startup v%i.%i"
    "                           ",
    // doom.wad
    "                            "
    "DOOM Registered Startup v%i.%i"
    "                           ",
    // Registered DOOM uses this
    "                          "
    "DOOM System Startup v%i.%i"
    "                          ",
    // Doom v1.666
    "                          "
    "DOOM System Startup v%i.%i66"
    "                          "
    // doom.wad (Ultimate DOOM)
    "                         "
    "The Ultimate DOOM Startup v%i.%i"
    "                        ",
    // tnt.wad
    "                     "
    "DOOM 2: TNT - Evilution v%i.%i"
    "                           ",
    // plutonia.wad
    "                   "
    "DOOM 2: Plutonia Experiment v%i.%i"
    "                           ",
};

//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// 

static char *GetGameName(const char *gamename)
{
    size_t i;

    for (i=0; i<arrlen(banners); ++i)
    {
        const char *deh_sub;
        // Has the banner been replaced?

        deh_sub = DEH_String(banners[i]);

        if (deh_sub != banners[i])
        {
            size_t gamename_size;
            int version;
            char *deh_gamename;

            // Has been replaced.
            // We need to expand via printf to include the Doom version number
            // We also need to cut off spaces to get the basic name

            gamename_size = strlen(deh_sub) + 10;
            deh_gamename = malloc(gamename_size);
            if (deh_gamename == NULL)
            {
                I_Error("GetGameName: Failed to allocate new string");
            }
            version = G_VanillaVersionCode();
            DEH_snprintf(deh_gamename, gamename_size, banners[i],
                         version / 100, version % 100);

            while (deh_gamename[0] != '\0' && isspace(deh_gamename[0]))
            {
                memmove(deh_gamename, deh_gamename + 1, gamename_size - 1);
            }

            while (deh_gamename[0] != '\0' && isspace(deh_gamename[strlen(deh_gamename)-1]))
            {
                deh_gamename[strlen(deh_gamename) - 1] = '\0';
            }

            return deh_gamename;
        }
    }

    return M_StringDuplicate(gamename);
}

static void SetMissionForPackName(const char *pack_name)
{
    int i;
    static const struct
    {
        const char *name;
        int mission;
    } packs[] = {
        { "doom2",    doom2 },
        { "tnt",      pack_tnt },
        { "plutonia", pack_plut },
    };

    for (i = 0; i < arrlen(packs); ++i)
    {
        if (!strcasecmp(pack_name, packs[i].name))
        {
            gamemission = packs[i].mission;
            return;
        }
    }

    printf("Valid mission packs are:\n");

    for (i = 0; i < arrlen(packs); ++i)
    {
        printf("\t%s\n", packs[i].name);
    }

    I_Error("Unknown mission pack name: %s", pack_name);
}

//
// Find out what version of Doom is playing.
//

void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.

    if (gamemission == none)
    {
        unsigned int i;

        for (i=0; i<numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i]->name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            } 
            else if (!strncasecmp(lumpinfo[i]->name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
        {
            // Still no idea.  I don't think this is going to work.

            I_Error("Unknown or invalid IWAD file.");
        }
    }

    // Make sure gamemode is set up correctly

    if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (W_CheckNumForName("E4M1") > 0)
        {
            // Ultimate Doom

            gamemode = retail;
        } 
        else if (W_CheckNumForName("E3M1") > 0)
        {
            gamemode = registered;
        }
        else
        {
            gamemode = shareware;
        }
    }
    else
    {
        int p;

        // Doom 2 of some kind.
        gamemode = commercial;

        // We can manually override the gamemission that we got from the
        // IWAD detection code. This allows us to eg. play Plutonia 2
        // with Freedoom and get the right level names.

        //!
        // @category compat
        // @arg <pack>
        //
        // Explicitly specify a Doom II "mission pack" to run as, instead of
        // detecting it based on the filename. Valid values are: "doom2",
        // "tnt" and "plutonia".
        //
        p = M_CheckParmWithArgs("-pack", 1);
        if (p > 0)
        {
            SetMissionForPackName(myargv[p + 1]);
        }
    }
}

// Set the gamedescription string

static void D_SetGameDescription(void)
{
    if (logical_gamemission == doom)
    {
        // Doom 1.  But which version?

        if (gamevariant == freedoom)
        {
            gamedescription = GetGameName("Freedoom: Phase 1");
        }
        else if (gamemode == retail)
        {
            // Ultimate Doom

            gamedescription = GetGameName("The Ultimate DOOM");
        }
        else if (gamemode == registered)
        {
            gamedescription = GetGameName("DOOM Registered");
        }
        else if (gamemode == shareware)
        {
            gamedescription = GetGameName("DOOM Shareware");
        }
    }
    else
    {
        // Doom 2 of some kind.  But which mission?

        if (gamevariant == freedm)
        {
            gamedescription = GetGameName("FreeDM");
        }
        else if (gamevariant == freedoom)
        {
            gamedescription = GetGameName("Freedoom: Phase 2");
        }
        else if (logical_gamemission == doom2)
        {
            gamedescription = GetGameName("DOOM 2: Hell on Earth");
        }
        else if (logical_gamemission == pack_plut)
        {
            gamedescription = GetGameName("DOOM 2: Plutonia Experiment"); 
        }
        else if (logical_gamemission == pack_tnt)
        {
            gamedescription = GetGameName("DOOM 2: TNT - Evilution");
        }
        else if (logical_gamemission == pack_nerve)
        {
            gamedescription = GetGameName("DOOM 2: No Rest For The Living");
        }
        else if (logical_gamemission == pack_master)
        {
            gamedescription = GetGameName("Master Levels for DOOM 2");
        }
    }

    if (gamedescription == NULL)
    {
        gamedescription = M_StringDuplicate("Unknown");
    }
}

//      print title for every printed line
char            title[128];

static boolean D_AddFile(char *filename)
{
    wad_file_t *handle;

    printf(" adding %s\n", filename);
    handle = W_AddFile(filename);

    return handle != NULL;
}

// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are 
// replaced by dehacked.

static const char *copyright_banners[] =
{
    "===========================================================================\n"
    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
    "        You will not receive technical support for modified games.\n"
    "                      press enter to continue\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                 Commercial product - do not distribute!\n"
    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                                Shareware!\n"
    "===========================================================================\n"
};

// Prints a message only if it has been modified by dehacked.

void PrintDehackedBanners(void)
{
    size_t i;

    for (i=0; i<arrlen(copyright_banners); ++i)
    {
        const char *deh_s;

        deh_s = DEH_String(copyright_banners[i]);

        if (deh_s != copyright_banners[i])
        {
            printf("%s", deh_s);

            // Make sure the modified banner always ends in a newline character.
            // If it doesn't, add a newline.  This fixes av.wad.

            if (deh_s[strlen(deh_s) - 1] != '\n')
            {
                printf("\n");
            }
        }
    }
}

static const struct
{
    const char *description;
    const char *cmdline;
    GameVersion_t version;
} gameversions[] = {
    {"Doom 1.2",             "1.2",        exe_doom_1_2},
    {"Doom 1.5",             "1.5",        exe_doom_1_5},
    {"Doom 1.666",           "1.666",      exe_doom_1_666},
    {"Doom 1.7/1.7a",        "1.7",        exe_doom_1_7},
    {"Doom 1.8",             "1.8",        exe_doom_1_8},
    {"Doom 1.9",             "1.9",        exe_doom_1_9},
    {"Hacx",                 "hacx",       exe_hacx},
    {"Ultimate Doom",        "ultimate",   exe_ultimate},
    {"Final Doom",           "final",      exe_final},
    {"Final Doom (alt)",     "final2",     exe_final2},
    {"Chex Quest",           "chex",       exe_chex},
    { NULL,                  NULL,         0},
};

// Initialize the game version

static void InitGameVersion(void)
{
    byte *demolump;
    char demolumpname[6];
    int demoversion;
    int p;
    int i;
    boolean status;

    //!
    // @arg <version>
    // @category compat
    //
    // Emulate a specific version of Doom. Valid values are "1.2", 
    // "1.5", "1.666", "1.7", "1.8", "1.9", "ultimate", "final", 
    // "final2", "hacx" and "chex".
    //

    p = M_CheckParmWithArgs("-gameversion", 1);

    if (p)
    {
        for (i=0; gameversions[i].description != NULL; ++i)
        {
            if (!strcmp(myargv[p+1], gameversions[i].cmdline))
            {
                gameversion = gameversions[i].version;
                break;
            }
        }

        if (gameversions[i].description == NULL)
        {
            printf("Supported game versions:\n");

            for (i=0; gameversions[i].description != NULL; ++i)
            {
                printf("\t%s (%s)\n", gameversions[i].cmdline,
                        gameversions[i].description);
            }

            I_Error("Unknown game version '%s'", myargv[p+1]);
        }
    }
    else
    {
        // Determine automatically

        if (gamemission == pack_chex)
        {
            // chex.exe - identified by iwad filename

            gameversion = exe_chex;
        }
        else if (gamemission == pack_hacx)
        {
            // hacx.exe: identified by iwad filename

            gameversion = exe_hacx;
        }
        else if (gamemode == shareware || gamemode == registered
              || (gamemode == commercial && gamemission == doom2))
        {
            // original
            gameversion = exe_doom_1_9;

            // Detect version from demo lump
            for (i = 1; i <= 3; ++i)
            {
                M_snprintf(demolumpname, 6, "demo%i", i);
                if (W_CheckNumForName(demolumpname) > 0)
                {
                    demolump = W_CacheLumpName(demolumpname, PU_STATIC);
                    demoversion = demolump[0];
                    W_ReleaseLumpName(demolumpname);
                    status = true;
                    switch (demoversion)
                    {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            gameversion = exe_doom_1_2;
                            break;
                        case 106:
                            gameversion = exe_doom_1_666;
                            break;
                        case 107:
                            gameversion = exe_doom_1_7;
                            break;
                        case 108:
                            gameversion = exe_doom_1_8;
                            break;
                        case 109:
                            gameversion = exe_doom_1_9;
                            break;
                        default:
                            status = false;
                            break;
                    }
                    if (status)
                    {
                        break;
                    }
                }
            }
        }
        else if (gamemode == retail)
        {
            gameversion = exe_ultimate;
        }
        else if (gamemode == commercial)
        {
            // Final Doom: tnt or plutonia
            // Defaults to emulating the first Final Doom executable,
            // which has the crash in the demo loop; however, having
            // this as the default should mean that it plays back
            // most demos correctly.

            gameversion = exe_final;
        }
    }

    // Deathmatch 2.0 did not exist until Doom v1.4
    if (gameversion <= exe_doom_1_2 && deathmatch == 2)
    {
        deathmatch = 1;
    }

    // The original exe does not support retail - 4th episode not supported

    if (gameversion < exe_ultimate && gamemode == retail)
    {
        gamemode = registered;
    }

    // EXEs prior to the Final Doom exes do not support Final Doom.

    if (gameversion < exe_final && gamemode == commercial
     && (gamemission == pack_tnt || gamemission == pack_plut))
    {
        gamemission = doom2;
    }
}

void PrintGameVersion(void)
{
    int i;

    for (i=0; gameversions[i].description != NULL; ++i)
    {
        if (gameversions[i].version == gameversion)
        {
            printf("Emulating the behavior of the "
                   "'%s' executable.\n", gameversions[i].description);
            break;
        }
    }
}

// Function called at exit to display the ENDOOM screen

static void D_Endoom(void)
{
    byte *endoom;

    // Don't show ENDOOM if we have it disabled, or we're running
    // in screensaver or control test mode. Only show it once the
    // game has actually started.

    if (!show_endoom || !main_loop_started
     || screensaver_mode || M_CheckParm("-testcontrols") > 0)
    {
        return;
    }

    endoom = W_CacheLumpName(DEH_String("ENDOOM"), PU_STATIC);

    I_Endoom(endoom);
}

boolean IsFrenchIWAD(void)
{
    return (gamemission == doom2 && W_CheckNumForName("M_RDTHIS") < 0
          && W_CheckNumForName("M_EPISOD") < 0 && W_CheckNumForName("M_EPI1") < 0
          && W_CheckNumForName("M_EPI2") < 0 && W_CheckNumForName("M_EPI3") < 0
          && W_CheckNumForName("WIOSTF") < 0 && W_CheckNumForName("WIOBJ") >= 0);
}

// Load dehacked patches needed for certain IWADs.
static void LoadIwadDeh(void)
{
    // The Freedoom IWADs have DEHACKED lumps that must be loaded.
    if (gamevariant == freedoom || gamevariant == freedm)
    {
        // Old versions of Freedoom (before 2014-09) did not have technically
        // valid DEHACKED lumps, so ignore errors and just continue if this
        // is an old IWAD.
        DEH_LoadLumpByName("DEHACKED", false, true);
    }

    else // [crispy]
    // If this is the HACX IWAD, we need to load the DEHACKED lump.
    if (gameversion == exe_hacx)
    {
        if (!DEH_LoadLumpByName("DEHACKED", true, false))
        {
            I_Error("DEHACKED lump not found.  Please check that this is the "
                    "Hacx v1.2 IWAD.");
        }
    }

    else // [crispy]
    // Chex Quest needs a separate Dehacked patch which must be downloaded
    // and installed next to the IWAD.
    if (gameversion == exe_chex)
    {
        char *chex_deh = NULL;
        char *dirname;

        // Look for chex.deh in the same directory as the IWAD file.
        dirname = M_DirName(iwadfile);
        chex_deh = M_StringJoin(dirname, DIR_SEPARATOR_S, "chex.deh", NULL);
        free(dirname);

        // If the dehacked patch isn't found, try searching the WAD
        // search path instead.  We might find it...
        if (!M_FileExists(chex_deh))
        {
            free(chex_deh);
            chex_deh = D_FindWADByName("chex.deh");
        }

        // Still not found?
        if (chex_deh == NULL)
        {
            I_Error("Unable to find Chex Quest dehacked file (chex.deh).\n"
                    "The dehacked file is required in order to emulate\n"
                    "chex.exe correctly.  It can be found in your nearest\n"
                    "/idgames repository mirror at:\n\n"
                    "   themes/chex/chexdeh.zip");
        }

        if (!DEH_LoadFile(chex_deh))
        {
            I_Error("Failed to load chex.deh needed for emulating chex.exe.");
        }
    }
    // [crispy] try anyway...
    else if (W_CheckNumForName("DEHACKED") != -1)
    {
        DEH_LoadLumpByName("DEHACKED", true, true);
    }

    if (IsFrenchIWAD())
    {
        char *french_deh = NULL;
        char *dirname;

        // Look for french.deh in the same directory as the IWAD file.
        dirname = M_DirName(iwadfile);
        french_deh = M_StringJoin(dirname, DIR_SEPARATOR_S, "french.deh", NULL);
        printf("French version\n");
        free(dirname);

        // If the dehacked patch isn't found, try searching the WAD
        // search path instead.  We might find it...
        if (!M_FileExists(french_deh))
        {
            free(french_deh);
            french_deh = D_FindWADByName("french.deh");
        }

        // Still not found?
        if (french_deh == NULL)
        {
            I_Error("Unable to find French Doom II dehacked file\n"
                    "(french.deh).  The dehacked file is required in order to\n"
                    "emulate French doom2.exe correctly.  It can be found in\n"
                    "your nearest /idgames repository mirror at:\n\n"
                    "   utils/exe_edit/patches/french.zip");
        }

        if (!DEH_LoadFile(french_deh))
        {
            I_Error("Failed to load french.deh needed for emulating French\n"
                    "doom2.exe.");
        }
    }
}

static void G_CheckDemoStatusAtExit (void)
{
    G_CheckDemoStatus();
}

static const char *const loadparms[] = {"-file", "-merge", NULL};

//
// D_DoomMain
//
void D_DoomMain (void)
{
    int p;
    char file[256];
    char demolumpname[9] = {0};

    // [crispy] unconditionally initialize DEH tables
    DEH_Init();

    I_AtExit(D_Endoom, false);

    // print banner

    I_PrintBanner(PACKAGE_STRING);

    DEH_printf("Z_Init: Init zone memory allocation daemon. \n");
    Z_Init ();

    //!
    // @category net
    //
    // Start a dedicated server, routing packets but not participating
    // in the game itself.
    //

    if (M_CheckParm("-dedicated") > 0)
    {
        printf("Dedicated server mode.\n");
        NET_DedicatedServer();

        // Never returns
    }

    //!
    // @category net
    //
    // Query the Internet master server for a global list of active
    // servers.
    //

    if (M_CheckParm("-search"))
    {
        NET_MasterQuery();
        exit(0);
    }

    //!
    // @arg <address>
    // @category net
    //
    // Query the status of the server running on the given IP
    // address.
    //

    p = M_CheckParmWithArgs("-query", 1);

    if (p)
    {
        NET_QueryAddress(myargv[p+1]);
        exit(0);
    }

    //!
    // @category net
    //
    // Search the local LAN for running servers.
    //

    if (M_CheckParm("-localsearch"))
    {
        NET_LANQuery();
        exit(0);
    }

    //!
    // @category game
    // @vanilla
    //
    // Disable monsters.
    //
	
    nomonsters = M_CheckParm ("-nomonsters");

    //!
    // @category game
    // @vanilla
    //
    // Monsters respawn after being killed.
    //

    respawnparm = M_CheckParm ("-respawn");

    //!
    // @category game
    // @vanilla
    //
    // Monsters move faster.
    //

    fastparm = M_CheckParm ("-fast");

    //!
    // @vanilla
    //
    // Developer mode. F1 saves a screenshot in the current working
    // directory.
    //

    devparm = M_CheckParm ("-devparm");

    I_DisplayFPSDots(devparm);

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch game.
    //

    if (M_CheckParm ("-deathmatch"))
	deathmatch = 1;

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch 2.0 game.  Weapons do not stay in place and
    // all items respawn after 30 seconds.
    //

    if (M_CheckParm ("-altdeath"))
	deathmatch = 2;

    //!
    // @category net
    // @vanilla
    //
    // Start a deathmatch 3.0 game.  Weapons stay in place and
    // all items respawn after 30 seconds.
    //

    if (M_CheckParm ("-dm3"))
	deathmatch = 3;

    if (devparm)
	DEH_printf(D_DEVSTR);
    
    // find which dir to use for config files

#ifdef _WIN32

    //!
    // @category obscure
    // @platform windows
    // @vanilla
    //
    // Save configuration data and savegames in c:\doomdata,
    // allowing play from CD.
    //

    if (M_ParmExists("-cdrom"))
    {
        printf(D_CDROM);

        M_SetConfigDir("c:\\doomdata\\");
    }
    else
#endif
    {
        // Auto-detect the configuration dir.

        M_SetConfigDir(NULL);
    }

    //!
    // @category game
    // @arg <x>
    // @vanilla
    //
    // Turbo mode.  The player's speed is multiplied by x%.  If unspecified,
    // x defaults to 200.  Values are rounded up to 10 and down to 400.
    //

    if ( (p=M_CheckParm ("-turbo")) )
    {
	int     scale = 200;
	
	if (p<myargc-1)
	    scale = atoi (myargv[p+1]);
	if (scale < 10)
	    scale = 10;
	if (scale > 400)
	    scale = 400;
        DEH_printf("turbo scale: %i%%\n", scale);
	forwardmove[0] = forwardmove[0]*scale/100;
	forwardmove[1] = forwardmove[1]*scale/100;
	sidemove[0] = sidemove[0]*scale/100;
	sidemove[1] = sidemove[1]*scale/100;
    }
    
    // init subsystems
    DEH_printf("V_Init: allocate screens.\n");
    V_Init ();

    // Load configuration files before initialising other subsystems.
    DEH_printf("M_LoadDefaults: Load system defaults.\n");
    M_SetConfigFilenames("default.cfg", PROGRAM_PREFIX "doom.cfg");
    D_BindVariables();
    M_LoadDefaults();

    // Save configuration at exit.
    I_AtExit(M_SaveDefaults, true); // [crispy] always save configuration at exit

    // Find main IWAD file and load it.
    iwadfile = D_FindIWAD(IWAD_MASK_DOOM, &gamemission);

    // None found?

    if (iwadfile == NULL)
    {
        I_Error("Game mode indeterminate.  No IWAD file was found.  Try\n"
                "specifying one with the '-iwad' command line parameter.\n");
    }

    modifiedgame = false;

    DEH_printf("W_Init: Init WADfiles.\n");
    D_AddFile(iwadfile);

    W_CheckCorrectIWAD(doom);

    // Now that we've loaded the IWAD, we can figure out what gamemission
    // we're playing and which version of Vanilla Doom we need to emulate.
    D_IdentifyVersion();
    InitGameVersion();

    // Check which IWAD variant we are using.

    if (W_CheckNumForName("FREEDOOM") >= 0)
    {
        if (W_CheckNumForName("FREEDM") >= 0)
        {
            gamevariant = freedm;
        }
        else
        {
            gamevariant = freedoom;
        }
    }
    else if (W_CheckNumForName("DMENUPIC") >= 0)
    {
        gamevariant = bfgedition;
    }

    //!
    // @category mod
    //
    // Disable automatic loading of Dehacked patches for certain
    // IWAD files.
    //
    if (!M_ParmExists("-nodeh"))
    {
        // Some IWADs have dehacked patches that need to be loaded for
        // them to be played properly.
        LoadIwadDeh();
    }

    // Doom 3: BFG Edition includes modified versions of the classic
    // IWADs which can be identified by an additional DMENUPIC lump.
    // Furthermore, the M_GDHIGH lumps have been modified in a way that
    // makes them incompatible to Vanilla Doom and the modified version
    // of doom2.wad is missing the TITLEPIC lump.
    // We specifically check for DMENUPIC here, before PWADs have been
    // loaded which could probably include a lump of that name.

    if (gamevariant == bfgedition)
    {
        printf("BFG Edition: Using workarounds as needed.\n");

        // BFG Edition changes the names of the secret levels to
        // censor the Wolfenstein references. It also has an extra
        // secret level (MAP33). In Vanilla Doom (meaning the DOS
        // version), MAP33 overflows into the Plutonia level names
        // array, so HUSTR_33 is actually PHUSTR_1.
        DEH_AddStringReplacement(HUSTR_31, "level 31: idkfa");
        DEH_AddStringReplacement(HUSTR_32, "level 32: keen");
        DEH_AddStringReplacement(PHUSTR_1, "level 33: betray");

        // The BFG edition doesn't have the "low detail" menu option (fair
        // enough). But bizarrely, it reuses the M_GDHIGH patch as a label
        // for the options menu (says "Fullscreen:"). Why the perpetrators
        // couldn't just add a new graphic lump and had to reuse this one,
        // I don't know.
        //
        // The end result is that M_GDHIGH is too wide and causes the game
        // to crash. As a workaround to get a minimum level of support for
        // the BFG edition IWADs, use the "ON"/"OFF" graphics instead.
        DEH_AddStringReplacement("M_GDHIGH", "M_MSGON");
        DEH_AddStringReplacement("M_GDLOW", "M_MSGOFF");

        // The BFG edition's "Screen Size:" graphic has also been changed
        // to say "Gamepad:". Fortunately, it (along with the original
        // Doom IWADs) has an unused graphic that says "Display". So we
        // can swap this in instead, and it kind of makes sense.
        DEH_AddStringReplacement("M_SCRNSZ", "M_DISP");
    }

    //!
    // @category game
    //
    // Automatic pistol start when advancing from one level to the next. At the
    // beginning of each level, the player's health is reset to 100, their
    // armor to 0 and their inventory is reduced to the following: pistol,
    // fists and 50 bullets. This option is not allowed when recording a demo,
    // playing back a demo or when starting a network game.
    //

    crispy->pistolstart = M_ParmExists("-pistolstart");

    //!
    // @category game
    //
    // Double ammo pickup rate. This option is not allowed when recording a
    // demo, playing back a demo or when starting a network game.
    //

    crispy->moreammo = M_ParmExists("-doubleammo");

    //!
    // @category mod
    //
    // Disable auto-loading of .wad and .deh files.
    //
    if (!M_ParmExists("-noautoload") && gamemode != shareware)
    {
        char *autoload_dir;

        // common auto-loaded files for all Doom flavors

        if (gamemission < pack_chex && gamevariant != freedoom)
        {
            autoload_dir = M_GetAutoloadDir("doom-all", true);
            if (autoload_dir != NULL)
            {
                DEH_AutoLoadPatches(autoload_dir);
                W_AutoLoadWADs(autoload_dir);
                free(autoload_dir);
            }
        }

        // auto-loaded files per IWAD
        autoload_dir = M_GetAutoloadDir(D_SaveGameIWADName(gamemission, gamevariant), true);
        if (autoload_dir != NULL)
        {
            DEH_AutoLoadPatches(autoload_dir);
            W_AutoLoadWADs(autoload_dir);
            free(autoload_dir);
        }
    }

    // Load Dehacked patches specified on the command line with -deh.
    // Note that there's a very careful and deliberate ordering to how
    // Dehacked patches are loaded. The order we use is:
    //  1. IWAD dehacked patches.
    //  2. Command line dehacked patches specified with -deh.
    //  3. PWAD dehacked patches in DEHACKED lumps.
    DEH_ParseCommandLine();

    // Load PWAD files.
    modifiedgame = W_ParseCommandLine();

    //!
    // @arg <file>
    // @category mod
    //
    // [crispy] experimental feature: in conjunction with -merge <files>
    // merges PWADs into the main IWAD and writes the merged data into <file>
    //

    p = M_CheckParm("-mergedump");

    if (p)
    {
	p = M_CheckParmWithArgs("-mergedump", 1);

	if (p)
	{
	    int merged;

	    if (M_StringEndsWith(myargv[p+1], ".wad"))
	    {
		M_StringCopy(file, myargv[p+1], sizeof(file));
	    }
	    else
	    {
		DEH_snprintf(file, sizeof(file), "%s.wad", myargv[p+1]);
	    }

	    merged = W_MergeDump(file);
	    I_Error("W_MergeDump: Merged %d lumps into file '%s'.", merged, file);
	}
	else
	{
	    I_Error("W_MergeDump: The '-mergedump' parameter requires an argument.");
	}
    }

    //!
    // @arg <file>
    // @category mod
    //
    // [crispy] experimental feature: dump lump data into a new LMP file <file>
    //

    p = M_CheckParm("-lumpdump");

    if (p)
    {
	p = M_CheckParmWithArgs("-lumpdump", 1);

	if (p)
	{
	    int dumped;

	    M_StringCopy(file, myargv[p+1], sizeof(file));

	    dumped = W_LumpDump(file);

	    if (dumped < 0)
	    {
		I_Error("W_LumpDump: Failed to write lump '%s'.", file);
	    }
	    else
	    {
		I_Error("W_LumpDump: Dumped lump into file '%s.lmp'.", file);
	    }
	}
	else
	{
	    I_Error("W_LumpDump: The '-lumpdump' parameter requires an argument.");
	}
    }

    // Debug:
//    W_PrintDirectory();

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

    //!
    // @arg <demo>
    // @category demo
    // @vanilla
    //
    // Play back the demo named demo.lmp.
    //

    p = M_CheckParmWithArgs ("-playdemo", 1);

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

        // With Vanilla you have to specify the file without extension,
        // but make that optional.
        if (M_StringEndsWith(uc_filename, ".LMP"))
        {
            M_StringCopy(file, myargv[p + 1], sizeof(file));
        }
        else
        {
            DEH_snprintf(file, sizeof(file), "%s.lmp", myargv[p+1]);
        }

        free(uc_filename);

        if (D_AddFile(file))
        {
	    int i;
	    // [crispy] check if the demo file name gets truncated to a lump name that is already present
	    if ((i = W_CheckNumForNameFromTo(lumpinfo[numlumps - 1]->name, numlumps - 2, 0)) != -1)
	    {
		printf("Demo lump name collision detected with lump \'%.8s\' from %s.\n",
		        lumpinfo[i]->name, W_WadNameForLump(lumpinfo[i]));
		// [FG] the DEMO1 lump is almost certainly always a demo lump
		M_StringCopy(lumpinfo[numlumps - 1]->name, "DEMO1", 6);
	    }

            M_StringCopy(demolumpname, lumpinfo[numlumps - 1]->name,
                         sizeof(demolumpname));
        }
        else
        {
            // If file failed to load, still continue trying to play
            // the demo in the same way as Vanilla Doom.  This makes
            // tricks like "-playdemo demo1" possible.

            M_StringCopy(demolumpname, myargv[p + 1], sizeof(demolumpname));
        }

        printf("Playing demo %s.\n", file);
    }

    I_AtExit(G_CheckDemoStatusAtExit, true);

    // Generate the WAD hash table.  Speed things up a bit.
    W_GenerateHashTable();

    // [crispy] allow overriding of special-casing

    //!
    // @category mod
    //
    // Disable automatic loading of Master Levels, No Rest for the Living and
    // Sigil.
    //
    if (!M_ParmExists("-nosideload") && gamemode != shareware && !demolumpname[0])
    {
	if (gamemode == retail &&
	    gameversion == exe_ultimate &&
	    gamevariant != freedoom &&
	    strncasecmp(M_BaseName(iwadfile), "rekkr", 5))
	{
		D_LoadSigilWads();
	}

	if (gamemission == doom2)
	{
		D_LoadNerveWad();
		D_LoadMasterlevelsWad();
	}
    }

    // Load DEHACKED lumps from WAD files - but only if we give the right
    // command line parameter.

    // [crispy] load DEHACKED lumps by default, but allow overriding

    //!
    // @category mod
    //
    // Disable automatic loading of embedded DEHACKED lumps in wad files.
    //
    if (!M_ParmExists("-nodehlump") && !M_ParmExists("-nodeh"))
    {
        int i, loaded = 0;
        int numiwadlumps = numlumps;

        while (!W_IsIWADLump(lumpinfo[numiwadlumps - 1]))
        {
            numiwadlumps--;
        }

        for (i = numiwadlumps; i < numlumps; ++i)
        {
            if (!strncmp(lumpinfo[i]->name, "DEHACKED", 8))
            {
                DEH_LoadLump(i, true, true); // [crispy] allow long, allow error
                loaded++;
            }
        }

        printf("  loaded %i DEHACKED lumps from PWAD files.\n", loaded);
    }

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

    // Set the gamedescription string. This is only possible now that
    // we've finished loading Dehacked patches.
    D_SetGameDescription();

    savegamedir = M_GetSaveGameDir(D_SaveGameIWADName(gamemission, gamevariant));

    // Check for -file in shareware
    if (modifiedgame && (gamevariant != freedoom))
    {
	// These are the lumps that will be checked in IWAD,
	// if any one is not present, execution will be aborted.
	char name[23][8]=
	{
	    "e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
	    "e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
	    "dphoof","bfgga0","heada1","cybra1","spida1d1"
	};
	int i;
	
	if ( gamemode == shareware)
	    I_Error(DEH_String("\nYou cannot -file with the shareware "
			       "version. Register!"));

	// Check for fake IWAD with right name,
	// but w/o all the lumps of the registered version. 
	if (gamemode == registered)
	    for (i = 0;i < 23; i++)
		if (W_CheckNumForName(name[i])<0)
		    I_Error(DEH_String("\nThis is not the registered version."));
    }

// [crispy] disable meaningless warning, we always use "-merge" anyway
#if 0
    if (W_CheckNumForName("SS_START") >= 0
     || W_CheckNumForName("FF_END") >= 0)
    {
        I_PrintDivider();
        printf(" WARNING: The loaded WAD file contains modified sprites or\n"
               " floor textures.  You may want to use the '-merge' command\n"
               " line option instead of '-file'.\n");
    }
#endif

    I_PrintStartupBanner(gamedescription);
    PrintDehackedBanners();

    DEH_printf("I_Init: Setting up machine state.\n");
    I_CheckIsScreensaver();
    I_InitTimer();
    I_InitJoystick();
    I_InitSound(doom);
    I_InitMusic();

    // [crispy] check for SSG resources
    crispy->havessg =
    (
        gamemode == commercial ||
        (
            W_CheckNumForName("sht2a0")         != -1 && // [crispy] wielding/firing sprite sequence
            I_GetSfxLumpNum(&S_sfx[sfx_dshtgn]) != -1 && // [crispy] firing sound
            I_GetSfxLumpNum(&S_sfx[sfx_dbopn])  != -1 && // [crispy] opening sound
            I_GetSfxLumpNum(&S_sfx[sfx_dbload]) != -1 && // [crispy] reloading sound
            I_GetSfxLumpNum(&S_sfx[sfx_dbcls])  != -1    // [crispy] closing sound
        )
    );

    // [crispy] check for presence of a 5th episode
    crispy->haved1e5 = (gameversion == exe_ultimate) &&
                       (W_CheckNumForName("m_epi5") != -1) &&
                       (W_CheckNumForName("e5m1") != -1) &&
                       (W_CheckNumForName("wilv40") != -1);

    // [crispy] check for presence of a 6th episode
    crispy->haved1e6 = (gameversion == exe_ultimate) &&
                       (W_CheckNumForName("m_epi6") != -1) &&
                       (W_CheckNumForName("e6m1") != -1) &&
                       (W_CheckNumForName("wilv50") != -1);

    // [crispy] check for presence of E1M10
    crispy->havee1m10 = (gamemode == retail) &&
                       (W_CheckNumForName("e1m10") != -1) &&
                       (W_CheckNumForName("sewers") != -1);

    // [crispy] check for presence of MAP33
    crispy->havemap33 = (gamemode == commercial) &&
                       (W_CheckNumForName("map33") != -1) &&
                       (W_CheckNumForName("cwilv32") != -1);

    // [crispy] change level name for MAP33 if not already changed
    if (crispy->havemap33 && !DEH_HasStringReplacement(PHUSTR_1))
    {
        DEH_AddStringReplacement(PHUSTR_1, "level 33: betray");
    }

    printf ("NET_Init: Init network subsystem.\n");
    NET_Init ();

    // Initial netgame startup. Connect to server etc.
    D_ConnectNetGame();

    // get skill / episode / map from parms

    // HMP (or skill #2) being the default, had to be placed at index 0 when drawn in the menu,
    // so all difficulties 'real' positions had to be scaled by -2, hence +2 being added
    // below in order to get the correct skill.
    startskill = (crispy->defaultskill + SKILL_HMP) % NUM_SKILLS;

    startepisode = 1;
    startmap = 1;
    autostart = false;

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
	startskill = myargv[p+1][0]-'1';
	autostart = true;
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
	startepisode = myargv[p+1][0]-'0';
	startmap = 1;
	autostart = true;
    }
	
    timelimit = 0;

    //! 
    // @arg <n>
    // @category net
    // @vanilla
    //
    // For multiplayer games: exit each level after n minutes.
    //

    p = M_CheckParmWithArgs("-timer", 1);

    if (p)
    {
	timelimit = atoi(myargv[p+1]);
    }

    //!
    // @category net
    // @vanilla
    //
    // Austin Virtual Gaming: end levels after 20 minutes.
    //

    p = M_CheckParm ("-avg");

    if (p)
    {
	timelimit = 20;
    }

    //!
    // @category game
    // @arg [<x> <y> | <xy>]
    // @vanilla
    //
    // Start a game immediately, warping to ExMy (Doom 1) or MAPxy
    // (Doom 2)
    //

    p = M_CheckParmWithArgs("-warp", 1);

    if (p)
    {
        if (gamemode == commercial)
            startmap = atoi (myargv[p+1]);
        else
        {
            startepisode = myargv[p+1][0]-'0';

            // [crispy] only if second argument is not another option
            if (p + 2 < myargc && myargv[p+2][0] != '-')
            {
                startmap = myargv[p+2][0]-'0';
            }
            else
            {
                // [crispy] allow second digit without space in between for Doom 1
                startmap = myargv[p+1][1]-'0';
            }
        }
        autostart = true;
        // [crispy] if used with -playdemo, fast-forward demo up to the desired map
        crispy->demowarp = startmap;
    }

    // Undocumented:
    // Invoked by setup to test the controls.

    p = M_CheckParm("-testcontrols");

    if (p > 0)
    {
        startepisode = 1;
        startmap = 1;
        autostart = true;
        testcontrols = true;
    }

    // [crispy] port level flipping feature over from Strawberry Doom
#ifdef ENABLE_APRIL_1ST_JOKE
    {
        time_t curtime = time(NULL);
        struct tm *curtm = localtime(&curtime);

        if (curtm && curtm->tm_mon == 3 && curtm->tm_mday == 1)
            crispy->fliplevels = true;
    }
#endif

    p = M_CheckParm("-fliplevels");

    if (p > 0)
    {
        crispy->fliplevels = !crispy->fliplevels;
        crispy->flipweapons = !crispy->flipweapons;
    }

    p = M_CheckParm("-flipweapons");

    if (p > 0)
    {
        crispy->flipweapons = !crispy->flipweapons;
    }

    // Check for load game parameter
    // We do this here and save the slot number, so that the network code
    // can override it or send the load slot to other players.

    //!
    // @category game
    // @arg <s>
    // @vanilla
    //
    // Load the game in slot s.
    //

    p = M_CheckParmWithArgs("-loadgame", 1);
    
    if (p)
    {
        startloadgame = atoi(myargv[p+1]);
    }
    else
    {
        // Not loading a game
        startloadgame = -1;
    }

    DEH_printf("M_Init: Init miscellaneous info.\n");
    M_Init ();

    DEH_printf("R_Init: Init DOOM refresh daemon - ");
    R_Init ();

    DEH_printf("\nP_Init: Init Playloop state.\n");
    P_Init ();

    DEH_printf("S_Init: Setting up sound.\n");
    S_Init (sfxVolume * 8, musicVolume * 8);

    DEH_printf("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    PrintGameVersion();

    DEH_printf("HU_Init: Setting up heads up display.\n");
    HU_Init ();

    DEH_printf("ST_Init: Init status bar.\n");
    ST_Init ();

    // If Doom II without a MAP01 lump, this is a store demo.
    // Moved this here so that MAP01 isn't constantly looked up
    // in the main loop.

    if (gamemode == commercial && W_CheckNumForName("map01") < 0)
        storedemo = true;

    if (M_CheckParmWithArgs("-statdump", 1))
    {
        I_AtExit(StatDump, true);
        DEH_printf("External statistics registered.\n");
    }

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
	G_RecordDemo (myargv[p+1]);
	autostart = true;
    }

    p = M_CheckParmWithArgs("-playdemo", 1);
    if (p)
    {
	singledemo = true;              // quit after one demo
	G_DeferedPlayDemo (demolumpname);
	D_DoomLoop ();  // never returns
    }
    crispy->demowarp = 0; // [crispy] we don't play a demo, so don't skip maps
	
    p = M_CheckParmWithArgs("-timedemo", 1);
    if (p)
    {
	G_TimeDemo (demolumpname);
	D_DoomLoop ();  // never returns
    }
	
    if (startloadgame >= 0)
    {
        M_StringCopy(file, P_SaveGameFile(startloadgame), sizeof(file));
	G_LoadGame(file);
    }
	
    if (gameaction != ga_loadgame )
    {
	if (autostart || netgame)
	    G_InitNew (startskill, startepisode, startmap);
	else
	    D_StartTitle ();                // start up intro loop
    }

    D_DoomLoop ();  // never returns
}

