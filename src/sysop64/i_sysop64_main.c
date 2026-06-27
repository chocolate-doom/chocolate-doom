//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
//     Sysop-64 main program. Initializes Chocolate Doom without SDL startup,
//     handles Sysop-specific help/options, then calls D_DoomMain.
//

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"

void D_DoomMain(void);

// Sysop builds use their own entry point so the executable can avoid SDL
// startup while still feeding Chocolate Doom the normal global argv state.

static int IsSysopCleanAllArg(const char *arg)
{
    return arg != NULL
        && (!strcmp(arg, "--sysop-clean-all")
            || !strcmp(arg, "-sysop-clean-all")
            || !strcmp(arg, "--sysop-all-clean")
            || !strcmp(arg, "-sysop-all-clean")
            || !strcmp(arg, "--sysop-clean=all")
            || !strcmp(arg, "-sysop-clean=all"));
}

static void ExpandSysopCleanAllArgs(void)
{
    // The clean overlay options are checked in different parts of the Doom
    // frontend and sysop backend. Expanding the aggregate option keeps those
    // call sites simple and makes --sysop-clean-all behave like the explicit
    // flags even if a subsystem only knows about its own switch.
    static const char *clean_args[] = {
        "--sysop-clean-messages",
        "--sysop-clean-menu",
        "--sysop-clean-status",
        "--sysop-clean-intermission",
    };
    int extra_args = 0;
    int out = 0;
    char **newargv;

    for (int i = 1; i < myargc; ++i) {
        if (IsSysopCleanAllArg(myargv[i])) {
            extra_args += arrlen(clean_args);
        }
    }

    if (extra_args == 0) {
        return;
    }

    // myargv is owned by this startup shim. Any inserted args must be
    // duplicated so later Chocolate Doom argument handling can treat them the
    // same way as command-line and response-file arguments.
    newargv = malloc((myargc + extra_args) * sizeof(*newargv));
    assert(newargv != NULL);

    for (int i = 0; i < myargc; ++i) {
        newargv[out++] = myargv[i];

        if (IsSysopCleanAllArg(myargv[i])) {
            for (int j = 0; j < arrlen(clean_args); ++j) {
                newargv[out++] = M_StringDuplicate(clean_args[j]);
            }
        }
    }

    free(myargv);
    myargv = newargv;
    myargc = out;

    puts("Sysop clean-all expanded to clean message/menu/status/intermission options.");
}

static void PrintSysopHelp(void)
{
    // This intentionally lists only the sysop additions and leaves the full
    // Chocolate Doom option set to the upstream documentation.
    puts(PACKAGE_STRING " sysop-64");
    puts("");
    puts("Usage: chocolate-doom-sysop64 [DOOM options] [sysop options]");
    puts("");
    puts("Sysop audio, display, and input options:");
    puts("  --sysop-mouse                    Enable sysop mouse input");
    puts("  --sysop-no-mouse                 Disable sysop mouse input");
    puts("  --sysop-mouse-turn-only          Ignore mouse Y movement");
    puts("  --sysop-mouse-vertical           Allow mouse Y forward/back");
    puts("  --sysop-mouse-wasd               Use W/S move, A/D strafe with mouse");
    puts("  --sysop-no-mouse-wasd            Disable mouse WASD bindings");
    puts("  --sysop-joystick                 Enable C64 joystick port 2 controls");
    puts("  --sysop-no-joystick              Disable C64 joystick input");
    puts("  --sysop-key-debug                Print sysop key debug info");
    puts("  --idkfa                          Start each single-player spawn with IDKFA loadout");
    puts("  --sysop-display-tune             Enable display tuning hotkeys");
    puts("  --sysop-no-display-tune          Disable display tuning hotkeys");
    puts("  --sysop-framebuffer-debug        Show raw Doom buffer via sysop framebuffer");
    puts("  --sysop-framebuffer-split        Overlay raw left half over normal C64 output");
    puts("                                    Port 2 left/right adjusts split if joystick play is off");
    puts("  --sysop-framebuffer-split-demo   Auto-sweep the framebuffer split line");
    puts("  --sysop-display=framebuffer|split|split-demo|c64  Select debug display mode");
    puts("  --sysop-pcm-sfx                  Enable sysop PCM sound effects");
    puts("  --sysop-no-pcm-sfx               Disable PCM SFX, keep SID music");
    puts("  --sysop-sid PATH                 Use a specific SID music file");
    puts("  --sysop-no-sid                   Disable SID music playback");
    puts("");
    puts("Sysop image conversion options:");
    puts("  Default: mega converter, adaptive palette, mega palette=pepto-ntsc-sony");
    puts("  --sysop-mega-http[=HOST:PORT]    Enable mega converter and HTTP tuner");
    puts("  --sysop-no-mega-http             Disable the HTTP tuner");
    puts("  --sysop-mega-set=name=value      Set any mega converter option");
    puts("  --sysop-mega-set=fast_tables=on  Use cached mega lookup tables, on by default");
    puts("  --sysop-mega-dither=PATTERN      off,bayer2,bayer4,bayer8,bayer8x16,checker,diagonal,dot,hash");
    puts("  --sysop-mega-palette=current|custom|VICE_NAME  Mega target palette; current is Sysop default");
    puts("  --sysop-mega-set=palette_color_N=#RRGGBB  Set custom target color 0..15");
    puts("  --sysop-mega-strength=N          Mega dither strength, 0..150");
    puts("  --sysop-mega-brightness=N        Mega brightness, -96..96");
    puts("  --sysop-mega-contrast=N          Mega contrast, 25..250");
    puts("  --sysop-mega-gamma=N             Mega gamma curve, 35..220");
    puts("  --sysop-mega-saturation=N        Mega saturation, 0..240");
    puts("  --sysop-mega-vibrance=N          Mega vibrance, 0..160");
    puts("  --sysop-mega-detail=N            Mega detail pop, 0..240");
    puts("  --sysop-mega-surface=N           Mega surface-detail palette bias, 0..200");
    puts("");
    puts("Sysop menu/message/status rendering:");
    puts("  --sysop-clean-all                Enable all clean C64 overlays");
    puts("  --sysop-clean=all                Same as --sysop-clean-all");
    puts("  --sysop-hud=fb|clean|bitmap|off  Message rendering mode");
    puts("  --sysop-hud-fb                   Use framebuffer overlay messages");
    puts("  --sysop-clean-messages           Use clean C64 bitmap messages");
    puts("  --sysop-hud-bitmap               Use legacy bitmap messages");
    puts("  --sysop-hud-off                  Disable sysop message rendering");
    puts("  --sysop-menu=clean|doom|off      Menu rendering mode");
    puts("  --sysop-clean-menu               Use clean C64 menu rendering");
    puts("  --sysop-status=clean|off         Status bar rendering mode");
    puts("  --sysop-clean-status             Use clean C64 status bar rendering");
    puts("  --sysop-status-off               Use original Doom status bar");
    puts("  --sysop-intermission=clean|off   Intermission stats rendering mode");
    puts("  --sysop-clean-intermission       Use clean C64 intermission stats");
    puts("  --sysop-intermission-off         Use original Doom intermission");
    puts("");
    puts("Sysop menu/intermission color tuning:");
    puts("  --sysop-menu-dither=sharp|poster|off");
    puts("  --sysop-menu-dither-sharp        Improve red patch text readability");
    puts("  --sysop-menu-dither-off          Use normal frame dithering");
    puts("  --sysop-menu-palette=sharp|off");
    puts("  --sysop-menu-palette-sharp       Tune red/gold patch colors");
    puts("  --sysop-menu-palette-off         Use normal frame palette");
    puts("  --sysop-palette=static|adaptive|status-split|adaptive-status");
    puts("  --sysop-palette-rate=N           Adaptive palette update interval");
    puts("  --sysop-palette-interval=N       Same as --sysop-palette-rate=N");
    puts("");
    puts("Standard Chocolate Doom options are still accepted.");
}

int main(int argc, char **argv)
{
    // Chocolate Doom expects mutable process arguments in the global myargc /
    // myargv pair. Build that copy before calling any M_* argument helpers.
    myargc = argc;
    myargv = malloc(argc * sizeof(char *));
    assert(myargv != NULL);

    for (int i = 0; i < argc; ++i) {
        myargv[i] = M_StringDuplicate(argv[i]);
    }

    // Handle cheap information-only exits before response-file expansion or
    // any sysop backend setup, so --help and --version never touch hardware.
    if (M_ParmExists("-version") || M_ParmExists("--version")) {
        puts(PACKAGE_STRING);
        exit(0);
    }

    if (M_ParmExists("-help") || M_ParmExists("--help")
     || M_ParmExists("-h") || M_ParmExists("--sysop-help")
     || M_ParmExists("-sysop-help")) {
        PrintSysopHelp();
        exit(0);
    }

    // Response files can contain sysop switches too, so expand them before
    // translating aggregate options such as --sysop-clean-all.
    M_FindResponseFile();
    ExpandSysopCleanAllArgs();

    // From here on, run through the normal Doom startup path. The platform
    // hooks selected by Makefile.sysop64 provide the sysop video/audio/input
    // implementation underneath D_DoomMain().
    M_SetExeDir();
    D_DoomMain();

    return 0;
}
