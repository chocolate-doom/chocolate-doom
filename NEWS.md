## 3.0.1 (2020-06-24)

  This is a point release that fixes a security vulnerability
  (CVE-2020-14983). An unchecked field in the Chocolate Doom server logic
  could allow a malicious attacker to trigger arbitrary code execution
  against Chocolate Doom servers. Thanks to Michał Dardas from LogicalTrust
  for discovering the vulnerability.

## 3.0.0 (2017-12-30)

  Chocolate Doom 3.0 is a new major revision. The main change is that
  the codebase has been ported to SDL 2.0. This brings a number of
  benefits, although there have also been some other minor changes (all
  listed below).

  Huge thanks go to the entire Chocolate Doom team for working on the
  port to SDL2, and to all the testers who have found and reported bugs
  during its development.

### General
  * All screen scaling is now performed in hardware, meaning that the
    game can run in arbitrary window sizes in high quality. It can also
    scale to very large resolutions sizes without using large amounts of
    CPU or suffering degraded performance (thanks Fabian).
  * It is now possible to switch between windowed and full screen modes
    while the game is running by pressing alt + enter (thanks Jon)
  * Windows binaries now ship with several previously-optional DLLs.
    This means it is now possible to take PNG screenshots and to use
    digital music packs (FLAC/Ogg Vorbis formats).
  * The game now remembers your preferred monitor and will start on the
    same monitor you were using the last time you played. Windows appear
    centered on the screen.
  * The OS X launcher was tweaked somewhat, and now uses proper path
    controls for choosing files. FreeDM was added as an IWAD.
  * Configuration files on Mac OS X and Unix are now stored in locations
    compliant with the XDG standard (thanks chungy):
    - On Unix: `~/.local/share/chocolate-doom/`
    - On OS X: `~/Library/Application Support/chocolate-doom/`
  * Icons when the game is running are now a higher resolution.
  * Keyboard input is improved and uses the new SDL input API; on
    systems with on-screen keyboards, this should activate the on-screen
    keyboard when it is appropriate.
  * Menu navigation with the joystick is now much more practical, and
    it's possible to bind a joystick axis to look up/down in games which
    support it (thanks Jon, Wintermute0110).
  * Several command line options were removed that were judged to be
    useless: `-grabmouse`, `-novert` and `-nonovert`. The mouse grabbing
    and novert settings can still be configured in the setup tool.
  * There is no longer any option in the setup tool to specify a screen
    resolution, since in full screen mode the game just runs at the
    desktop resolution without changing screen modes. If necessary, the
    config file options `fullscreen_width` and `fullscreen_height` can
    be used to explicitly set a screen resolution.
  * There is no longer a soft dependency on Zenity on Unix systems; the
    SDL API is now used to display error dialogs.
  * Joysticks are identified more precisely using GUID now.
  * A new parameter, `-savedir` allows users to specify a directory from
    which to load and save games. (thanks CapnClever)
  * The midiproc code from Eternity Engine has been imported, improving
    native MIDI playback on Windows and fixing a long-standing bug with
    music volume adjustment (thanks AlexMax, Quasar).
  * VGA "porch" emulation was added (thanks Jon).
  * The codebase now compiles with OpenWatcom (thanks Stephen Finniss).

### Doom
  * The GOG install of Doom 3: BFG Edition is now detected (thanks chungy)
  * A `-shorttics` command line parameter was added that simulates
    recording a vanilla demo without actually recording a demo.

### Hexen
  * The CD audio option for music playback has been removed; the CD
    playback API has been removed from SDL 2.0. However, it is possible
    to use digital music packs as an alternative.

### Strife
  * `voices.wad` is now correctly loaded before PWADs (thanks
    @Catoptromancy)

### libtextscreen
  * On OS X on machines with retina displays, text screens are rendered
    using a high detail font.
  * File selector widgets now look more visually distinctive.
  * There is now a convenience widget for conditionally hiding widgets.
  * Font handling was restructured to be based around PNG format fonts
    which are converted during the build and can be more easily edited.
  * Handling of code pages was cleaned up, so it is easier to change the
    code to work with a different code page now.
  * Lots of the UI code was changed to use UTF-8 strings.
  * File extensions when using the Zenity file selector are now case
    insensitive (thanks Jon).

## 2.3.0 (2016-12-29)

### General
  * Bash completion scripts are included (thanks Fabian)
  * The OS X launcher now supports the .lmp file format (thanks Jon)
  * Pitch-shifting from early versions of Doom, Heretic, and Hexen.
    is now supported (thanks Jon)
  * Aspect ratio-corrected 1600×1200 PNGs are now written (thanks Jon)
  * OPL emulation is more accurate (thanks Nuke.YKT)
  * DMX bugs with GUS cards are now better emulated (thanks Nuke.YKT)
  * The disk activity floppy disk icon is now shown (thanks Fabian, Jon)
  * Checksum calculations are fixed on big endian systems, allowing
    multiplayer games to be played in mixed little/big-endian
    environments (thanks GhostlyDeath, njankowski)
  * The NES30, SNES30, and SFC30 gamepads are detected and configured
    automatically by the Setup tool. The automap can also be configured
    to a joystick button (thanks Jon)
  * The vanilla limit of 4046 lumps per WAD is now enforced (thanks
    Jon, Quasar, Edward-san)
  * Solidsegs overflow is emulated like in vanilla (thanks Quasar)
  * Multiple capitalizations are now tried when searching for WAD files,
    for convenience when running on case sensitive filesystems (thanks
    Fabian).
  * A new command line argument, `-strictdemos`, was added, to allow
    more careful control over demo format extensions. Such extensions
    are now forbidden in WAD files and warning messages are shown.

### Build systems
  * There is better compatibility with BSD Make (thanks R.Rebello)
  * “./configure --with-PACKAGE” checks were repaired to behave
    logically, rather than disabling the feature (thanks R.Rebello)
  * Games are now installed to ${bindir} by default, eg.
    /usr/local/bin, rather than /usr/local/games (thanks chungy)
  * Visual Studio 2015 is now supported (thanks Azarien)
  * SDL headers and libraries can now exist in the Microsoft Visual
    Studio project directory (thanks Quasar)
  * CodeBlocks projects were repaired by removing non-existent files
    from the project files (thanks krystalgamer)

### Doom
  * Chex Quest’s level warp cheat (LEESNYDER##) now behaves more like
    like the original EXE (thanks Nuke.YKT)
  * It's now possible to start multiplayer Chex Quest games.
  * Freedoom: Phase 1 <= 0.10.1 can now be loaded with mods, with
    -gameversion older than ultimate (thanks Fabian, chungy)
  * The IWAD order preference for GOG.com installs matches vanilla
    Final Doom: doom2, plutonia, tnt, doom (thanks chungy)
  * There are better safety checks against write failures when saving
    a game, such as when the directory is read-only (thanks
    terrorcide)
  * Versions 1.666, 1.7, and 1.8 are emulated (thanks Nuke.YKT)
  * Crashes are now handled more gracefully when a linedef references
    nonexistent sidedefs (thanks Fabian)

### Heretic
  * Map names were added for Episode 6, fixing a crash after completing
    a level in this episode (thanks J.Benaim)
  * Support for unlimited demo/savegames was added (thanks CapnClever)
  * Demo support is expanded: "-demoextend" allows demos to last longer
    than a single level; "-shortticfix" adjusts low-resolution turning
    to match Doom's handling, and there is now "-maxdemo" and "-longtics"
    support (thanks CapnClever)

### Hexen
  * The MRJONES cheat code returns an identical string to vanilla, and
    enables fully reproducible builds (thanks Fabian)
  * An issue was fixed where the game crashed while killing the
    Wraithverge in 64-bit builds (thanks J.Benaim)
  * Support for unlimited demo/savegames was added (thanks CapnClever)
  * Mouse buttons for strafe left/right and move backward were added,
    as well as a "Double click acts as use" mouse option (thanks
    CapnClever)
  * Demo support is expanded: "-demoextend" allows demos to last longer
    than a single level; "-shortticfix" adjusts low-resolution turning
    to match Doom's handling, and there is now "-maxdemo" and "-longtics"
    support (thanks CapnClever)

### Strife
  * Support was added for automatic loading of the IWAD from the GOG.com
    release of Strife: Veteran Edition on Windows (thanks chungy)
  * Jumping can now be bound to a mouse button (thanks Gez)
  * Gibbing logic was changed to match vanilla behavior (thanks Quasar)
  * Several constants differences from vanilla were fixed (thanks
    Nuke.YKT, Quasar)
  * When using -iwad, voices.wad from the IWAD’s directory is prefered
    over auto-detected DOS/Steam/GOG.com installs (thanks Quasar)

### libtextscreen
  * The API for creating and managing tables and columns was simplified.
  * It's now possible to cycle through tables with the tab key.
  * Windows can now have multiple columns.

## 2.2.1 (2015-09-10)

  Chocolate Doom has not seen a great deal of “stable” patch releases
  in its history. While the development tree sees major new features
  and changes, the purpose of this release, and hopefully others to
  follow like it, is to repair some deficiencies that existed
  in 2.2.0.

### General
  * Preferences for the OS X launcher are now stored with a unique
    name to not conflict with other applications. (thanks
    Xeriphas1994)
  * Unix desktop entry files are now brought up to full desktop entry
    specification compliance. (thanks chungy, Fabian)
  * Unix AppData entries are now included, allowing software centers
    to display detailed information about the engines. (thanks chungy)
  * Partial XDG base directory specification compliance on Unix
    systems now exist to search for IWAD paths.  One benefit is that
    $HOME/.local/share/games/doom is now a valid location to store and
    automatically find IWADs. (thanks chungy)

### Build systems
  * The Microsoft Visual Studio build system was not fully functional
    in 2.2.0 and has been fixed. (thanks Linguica)
  * The autoconf build system checks for windres only for Windows
    toolchains.  Some Linux distributions mistakingly include the
    program in their native toolchains. (thanks Fabian)
  * A compiler hint for packed structs has been added, which otherwise
    broke the games when built under recent GCC releases for
    Windows. (thanks Fabian)

### Doom
  * The GOG.com releases of The Ultimate Doom, Doom II, and Final Doom
    are now detected and supported on Windows. (thanks chungy)
  * An integer overflow was used in spawn angle calculation, undefined
    C behavior which broke with Clang optimization.  (thanks David
    Majnemer for insight)

### Setup tool
  * The help URL for the level warp menu now points to the proper wiki
    page, rather than the multiplayer page.
  * The manifest has been updated for Windows 10 compatibility.
    (thanks chungy)

## 2.2.0 (2015-06-09)

  * The Hexen four level demo IWAD is now supported. Thanks to Fabian
    Greffrath for his careful investigation and emulation of the demo
    game’s behavior in developing this.
  * OPL music playback has been improved in a number of ways to match
    the behavior of Vanilla Doom’s DMX library much more closely. OPL3
    playback is also now supported. Thanks go to Alexey Khokholov for
    his excellent research into the Vanilla DMX library that enabled
    these improvements.
  * New gamepad configurations:
      - PS4 DualShock 4 (thanks Matt “3nT” Davis).
      - Xbox One controller on Linux (thanks chungy).
      - “Super Joy Box 7” USB/PC gameport adapter.
  * The Doom reload hack has been added back. See the wiki for more
    context on this: http://doomwiki.org/wiki/Reload_hack
  * The IWAD file from Strife: Veteran Edition is now detected
    automatically (thanks chungy).
  * It’s now possible to build outside of the source directory (thanks
    Dave Murphy).
  * MSVC project files were brought up to date (thanks dbrackett16).
  * M_StringDuplicate() has been added as a safer replacement for
    strdup() (thanks Quasar). M_StringCopy() now handles short buffers
    more gracefully.
  * The netgame discrepancy window is now dismissed by pressing enter
    to proceed, not escape (thanks Alexandre-Xavier).
  * A couple of source files that were in the previous release and
    were GPL3 have been replaced by GPL2 equivalents. Previous
    releases that included these files should be retroactively
    considered GPL3.

### Bug fixes
  * A long-standing bug that could cause every display frame to be
    rendered twice was fixed (thanks Linguica, Harha, Alexandre-
    Xavier).
  * Lots of endianness fixes were integrated that were found by Ronald
    Lasmanowicz during development of his Wii port of Chocolate Doom,
    including a fix for a bug that could cause monsters to become
    partially invisible.
  * DeHackEd files without a newline character at the EOF are now
    correctly parsed (thanks Fabian).
  * An infinite loop that could occur in the weapon cycling code was
    fixed (thanks raithe, Fabian).
  * Mouse input triggered by cursor warp was fixed (thanks Super6-4).
  * Loop tags in substitute music files are ignored if both of the
    loop tags are equal to zero. This makes us consistent with other
    source ports that support the tags.
  * It’s now possible to more conveniently play back demo .lmp files
    with names that end in the all-caps “.LMP” (thanks Ioan Chera).
  * Some code that accessed memory after freeing it was fixed. Two new
    parameters, -zonezero and -zonescan, were added to try to help
    detect these cases.
  * Mistaken assumptions about representations of booleans that
    affected some ARM systems were fixed (thanks floppes).
  * memcpy() uses on overlapping memory were changed to use memmove(),
    fixing abort traps on OpenBSD (thanks ryan-sg).
  * Hyphens in manpages were fixed (thanks chungy, Fabian).
  * Lots of compiler build warnings were fixed (thanks Fabian).

### Setup tool
  * The setup tool now has help buttons for its various different
    screens, which link to articles on the wiki that give more
    information (thanks to chungy for helping to put the wiki pages
    together).
  * A fix was applied for a buffer overrun that could occur if the
    user had lots of IWAD files installed (thanks Fabian).
  * A crash related to username lookup was fixed.
  * It’s now possible to connect via the setup tool to multiplayer
    servers that are not listening on the default port (thanks
    Alexandre-Xavier).

### Doom
  * Sky transitions when emulating the id anthology version of the
    Final Doom executable were fixed (thanks Alexandre-Xavier, Fabian,
    chungy).
  * Structure fields in the stair-building functions were fixed to be
    deterministic, fixing a desync in mm09-512.lmp (thanks Fabian).

### Hexen
  * A bug with texture names that had long names was fixed (thanks
    ETTiNGRiNDER).
  * Minotaur spawn time is now stored in little endian format, fixing
    a bug that affected compatibility with Vanilla savegames on big
    endian systems.
  * Code that starts ACS scripts is no longer compiler-dependent.

### Strife (all these are thanks to Quasar)
  * Sound priority was changed, so that the ticking sound that Stalker
    enemies make while active matches Vanilla behavior (thanks
    GeoffLedak).
  * Minor fixes to game behavior to match Vanilla, discovered during
    development of Strife: Veteran edition.
  * Behavior of descending stairs was fixed to match Vanilla.
  * Inventory items beyond the 8-bit range are now allowed in
    netgames.
  * Automap behavior better matches Vanilla now.
  * Multiplayer name changes were fixed.
  * Sound origin behavior for switches was fixed.
  * Teleport beacon behavior was fixed.
  * Default Strife skill level and screen size were changed to match
    Vanilla.
  * Bug was fixed where Rowan would not always take Beldin’s ring.
  * Totally-invisible objects are now displayed correctly, and a
    Vanilla glitch with Shadow Acolytes is correctly emulated.
  * The level name for MAP29 (Entity’s Lair) was fixed (thanks
    chungy).

### libtextscreen
  * The main loop now exits immediately once all windows are closed
    (thanks Alexander-Xavier).
  * The large font is no longer selected based entirely on screen
    size.

## 2.1.0 (2014-10-22)

  Chocolate Doom now supports high-quality substitute music packs that
  are used in place of the original MIDI music tracks. I’m hoping to
  put together high-quality recordings of the music for all supported
  games using the Roland SC-55 synthesizer originally used to compose
  Doom’s music (thanks twipley and MusicallyInspired).

  Support for joysticks and gamepads has been significantly improved
  in this version. Most gamepads should now work; if they don’t,
  please report a bug. A number of gamepads are now automatically
  detected and configured automatically; if yours is not, you can help
  by sending in details. See the following page:

  http://www.chocolate-doom.org/wiki/index.php/Adding_your_gamepad

  OPL MIDI playback has been significantly improved, and problems with
  most tracks should now be resolved. Multi-track MIDI files now play
  back properly, MIDI tempo meta events are now supported and problems
  with stuttering when playing certain tracks have been fixed. If you
  still have problems with OPL playback, let me know.

  Also of note is that Chocolate Doom now has a document that
  describes the philosophy of the project and the reasoning behind its
  design (see PHILOSOPHY distributed with the source).

### Other new features
  * There is now a -dehlump command line parameter to load Dehacked
    files contained inside WAD files (thanks Fabian Greffrath).
  * PNG format screenshots are now supported, and there is a dedicated
    key binding for taking screenshots without needing to always use
    -devparm (thanks Fabian Greffrath). The PrintScreen key can be
    used as a key binding (thanks Alexandre-Xavier).
  * There is now a config file variable (snd_maxslicetime_ms) to
    control the sound buffer size, and the default is more precise to
    reduce sound latency (thanks Holering).
  * You can now use an external command for music playback (thanks
    Holering).
  * All games now detect if you’re tring to play using the wrong type
    of IWAD (doom.wad with Hexen, etc.) and exit with a helpful error
    message. A couple of users made this mistake after the 2.0 release
    introduced support for the new games.
  * The OS X app now associates with .hhe and .seh files.
  * There is now a -nodes parameter that automatically starts a
    netgame when a desired number of players have joined the game.
  * There is now more extensive documentation about music
    configuration (README.Music).
  * On Linux, a GUI pop-up is used when the game quits with an error
    to show the error message (thanks Willy Barro).
  * There are now Linux .desktop files for all supported games (thanks
    Mike Swanson).
  * The -geometry command line parameter can now be used to specify
    fullscreen or windowed modes, eg. -geometry 640x480w or -geometry
    1024x768f. (thanks Mike Swanson)

### Doom
  * Minor workarounds were added to allow the BFG Edition IWADs to be
    used without crashing the game (thanks Fabian Greffrath).
  * GUS patch files included with the BFG Edition are now
    automatically detected.
  * The “no fog on spawn west” Vanilla bug is now correctly emulated
    (thanks xttl).
  * Behavior of older versions of Doom back to v1.666 can now be
    emulated.
  * The new Freedoom IWAD names are now recognized and supported.
  * Freedoom’s DEHACKED lump can now be parsed and is automatically
    loaded when a Freedoom IWAD file is used (thanks Fabian
    Greffrath). A new command line parameter, -nodeh, can be used to
    prevent this from being loaded.
  * Behavior of the M_EPI4 menu item is now correctly emulated based
    on game version (thanks Alexandre-Xavier).
  * IDCLEV up to MAP40 is now supported, to match Vanilla (thanks
    Alexandre-Xavier).
  * Level warping on the command line (-warp) to episodes higher than
    4 is possible, matching Vanilla behavior (thanks plumsinus).
  * The -cdrom command line parameter writes savegames to the correct
    directory now, matching Vanilla Doom behavior (thanks
    Alexandre-Xavier).
  * The Doom II mission pack to use can now be specified manually on
    the command line with the -pack parameter (thanks chungy)

### Heretic
  * Weapon cycling keys for mouse and joystick were fixed (thanks
    Sander van Dijk).
  * The -timedemo parameter has been fixed, and -playdemo now handles
    full paths correctly.
  * A bug when panning the map was fixed (thanks Chris Fielder).
  * A savegame bug where plat_t structures were not restored correctly
    was fixed (thanks romeroyakovlev).
  * Rebinding of the pause key was fixed (thanks Fabian Greffrath).

### Hexen
  * Music workarounds have been added so that it is possible to play
    using the Mac version of the Hexen IWAD file.
  * Weapon cycling keys for mouse and joystick were fixed (thanks
    Sander van Dijk).
  * The -timedemo parameter has been fixed, and -playdemo now handles
    full paths correctly.
  * There are now key bindings to allow the artifact keys to be
    rebound (thanks Fabian Greffrath).
  * Rebinding of the pause key was fixed (thanks Fabian Greffrath).
  * Maximum level number was extended to MAP60, allowing multiplayer
    games using the Deathkings add-on.
  * The startup screen can now be aborted by pressing escape, like in
    Vanilla.
  * Desync when playing back DEMO1 was fixed (thanks alexey.lysiuk).

### Strife
  * “Show mission” key is configured properly in setup (thanks Sander
    van Dijk).
  * Default music volume level now matches Vanilla (thanks
    Alexandre-Xavier).
  * Teleport beacon allegiance was fixed to match Vanilla (thanks
    Quasar).
  * The stair building code now more closely matches Vanilla (thanks
    Quasar).
  * Torpedo weapon changing behavior now matches Vanilla (thanks
    Quasar).

### Cleanups
  * The copyright headers at the top of all source files have been
    vastly simplified.
  * Unsafe string functions have been eliminated from the codebase.
    Thanks to Theo de Raadt for calling out Chocolate Doom by name
    (alongside many other packages) for still using unsafe functions
    like strcpy: http://marc.info/?l=openbsd-tech&m=138733933417096
  * vldoor_e enum values are now namespaced to avoid potential
    conflicts with POSIX standard functions.

### Bug fixes
  * WAD and Dehacked checksums are now sent to clients and checked
    correctly when setting up netgames.
  * A bug was fixed that caused sound not to work in multiplayer games
    (thanks to everyone who reported this, and for Alexandre-Xavier
    and Quasar for help in fixing it).
  * The “D_DDTBLU disease” bug affecting certain MIDI files has been
    fixed (thanks plumsinus, Brad Harding and Quasar).
  * Calculation of the -devparm “ticker” dots was fixed to match
    Vanilla behavior (thanks _bruce_ and Alexandre-Xavier).
  * The PC speaker code now supports the full range of sound
    frequencies (thanks Gez).
  * Annoying “jumping” behavior when grabbing the mouse cursor was
    fixed.
  * The screen is now initialized at the native bit depth by default,
    to avoid problems with systems that don’t handle 8-bit
    screenbuffers very well any more.
  * The --docdir argument to the configure script is now honored
    (thanks Jan Engelhardt).
  * Various issues with the build were fixed (thanks Jan Engelhardt
    and Fabian Greffrath).
  * Backwards parameters were fixed in the sound code (thanks
    proteal).
  * A crash was fixed when running fullscreen with the -2 parameter
    (thanks Fabian Greffrath).
  * A crash when using large values of snd_channels was fixed (thanks
    Alexandre-Xavier).
  * A resource leak in the BSD PC speaker code was fixed (thanks
    Edward-san).
  * Windows resource files were fixed for Windows 7 (thanks Brad
    Harding).
  * A hard to trigger crash caused by a realloc() in the WAD code was
    fixed (thanks Fabian Greffrath for debugging).
  * A bug has been fixed where Chocolate Doom would stay running in
    the background on Windows after quitting. SDL_Quit() is called now
    (thanks johnsirett, Brad Harding, Quasar).
  * String replacements in dehacked lumps can now be overridden if a
    subsequent dehacked patch replaces the same string.

### libtextscreen
  * Clicking on scrollbars now jumps to the correct position (thanks
    Alexandre-Xavier).
  * A use-after-free bug has been fixed where a click in a window that
    causes the window to close could lead to a crash (thanks DuClare).
  * Characters that are unprintable in the Extended ASCII chart are
    just ignored when they’re typed, rather than appearing as an
    upside-down question mark (thanks Alexandre-Xavier).

## 2.0.0 (2013-12-09)

  This is version 2.0 of Chocolate Doom! This new major version is
  released to celeberate the 20th anniversary of the first release of
  Doom in 1993. Happy Birthday Doom!

  This new version has some major changes compared to the 1.0 series:

  * The codebase now includes Chocolate Heretic and Chocolate
    Hexen. These are based on the GPL source code released by Raven
    Software.
  * Also included is Chocolate Strife. This was developed through a
    mammoth four year reverse engineering project conducted by James
    “Quasar” Haley and Samuel “Kaiser” Villareal. The result is the
    most accurate reproduction of Strife to date, including full demo
    and savegame compatibility. See README.Strife for more
    information.

### Minor features that are nonetheless worth mentioning
  * Chocolate Doom now includes a -statdump command line option, which
    emulates the output of the statdump.exe tool. This is used to
    implement a form of regression testing (statcheck) that directly
    compares against the Vanilla behavior.
  * Chocolate Heretic includes HHE patch file support, and I believe
    is the first Heretic port to include this feature.
  * GUS “pseudo-emulation” is now supported. This does not fully
    emulate a GUS, but Doom’s DMXGUS lump can be used to generate a
    Timidity configuration file that plays music using the GUS patch
    set.
  * The setup tool now includes a built-in server browser, for use
    when selecting a server to join.

  Version 2.0 of Chocolate Doom has been in development for a long
  time, and there have been many bugs fixed over this time, too many
  to list here. Thanks to all the people who have tested it and
  diligently reported bugs over this time, and to all the people who
  have tested the beta releases over the past couple of months.  Your
  contributions have been essential and invaluable.

## 1.7.0 (2012-06-09)

  * Fixed gnome-screensaver desktop file (thanks Rahul Sundaram).
  * Updated COPYING to current version of GPL2 (thanks Rahul
    Sundaram).
  * Running servers now re-resolve the address of the master server
    occasionally, to adapt to DNS address changes.
  * Error dialog is no longer shown on OS X when running from the
    console.
  * The Makefiles no longer use GNU make extensions, so the package
    builds on OpenBSD.
  * There is now an OPL MIDI debug option (-opldev), useful for when
    developing GENMIDI lumps.
  * A workaround for SDL mouse lag is now only used on Windows (where
    it is needed), and not on other systems. This fixes Chocolate Doom
    on AmigaOS (thanks Timo Sievänen).
  * UTF-8 usernames are supported, and Windows usernames with
    non-ASCII characters are now supported (thanks Alexandre Xavier).

### Compatibility
  * Palette accuracy is reduced to 6 bits per channel, to more
    accurately emulate the PC VGA hardware (thanks GhostlyDeath).
  * Fixed teleport behavior when emulating the alternate Final Doom
    executable (-gameversion final2) (thanks xttl).

### Bugs fixed
  * Fixed weapon cycling keys when playing in Shareware Doom and using
    the IDKFA cheat (thanks Alexandre Xavier).
  * Fixed the default mouse buttons in the setup tool (thanks
    Alexandre Xavier).
  * Chat macros now work when vanilla_keyboard_mapping is turned off.
  * Default chat macros were fixed in the setup tool.
  * Ping time calculation was fixed for LAN search, and made more
    accurate for all searches.
  * Fixed bug with detection of IWAD type by filename (thanks mether).

### libtextscreen
  * There is now limited UTF-8 text support in the textscreen library,
    used in the label and input box widgets.
  * Scroll bar behavior was fixed (thanks Alexandre Xavier).
  * Input boxes stop editing and save when they lose their focus,
    correcting a previous counterintuitive behavior (thanks Twelve).
  * The numeric keypad now works properly when entering text values
    (thanks Twelve).

## 1.6.0 (2011-05-17)

  * The instructions in the INSTALL file are now customized for
    different platforms, and each binary package contains a version
    with instructions specific to the platform that it is targetting.
    This should help to avoid confusion that some users have reported
    experiencing.
  * The display settings window in the setup tool has been reorganised
    to a better arrangement.
  * It is now possible to load .lmp files (and play back demos) with
    long filenames (thanks blzut3).
  * In the setup tool, it is now possible to hold down shift when
    changing key/mouse/joystick bindings to prevent other bindings to
    the same key from being cleared (thanks myk).
  * The joystick menu in the setup tool now has a test button (thanks
    Alexandre Xavier).
  * Specifying the -privateserver option implies -server (thanks
    Porsche Monty).
  * The Mac OS X .dmg package now has a background and looks generally
    more polished.
  * In Mac OS X, it is now possible to simply double click an IWAD
    file in the Finder to configure its location within the launcher.
  * Freedesktop.org desktop files are now installed for Doom and the
    setup tool, which will appear in the main menu on desktop
    environments such as Gnome and KDE (thanks Adrián Chaves
    Fernández).
  * The Chex Quest dehacked patch (chex.deh) will now be detected if
    it is in the same directory as the IWAD file.

### Compatibility
  * Added support for the alternate version of the Final Doom
    executable included in some later versions of the Id Anthology.
    This version fixed the demo loop crash that occurred with the
    “original” Final Doom executable.

    This executable can be selected on the command line with
    -gameversion final2. It has been made the default when playing
    with the Final Doom IWADs (the original behavior can be selected
    with -gameversion final).  (thanks Porsche Monty, Enjay).
  * Very short sound effects are not played, to better emulate the
    behavior of DMX in Vanilla Doom (thanks to Quasar for help in
    investigating this).
  * The null sector dereference emulation code has been imported from
    Prboom+ - this fixes a desync with CLNJ-506.LMP (thanks entryway).
  * The IDMUS cheat doesn’t work when emulating the v1.9 executable
    (thanks Alexandre Xavier).

### Bugs fixed
  * Menu navigation when using joystick/joypad (thanks Alexandre
    Xavier).
  * For configuration file value for shift keys, use scan code for
    right shift, not left shift (thanks Alexandre Xavier).
  * Default joystick buttons for the setup tool now match Vanilla
    (thanks twipley).
  * Visual Studio project files work again (thanks GhostlyDeath).
  * The default sfx/music volume set by the setup tool is now 8
    instead of 15, matching the game itself. (thanks Alexandre
    Xavier).
  * Weapon cycling from the shotgun to the chaingun in Doom 1 now
    works properly (thanks Alexandre Xavier).
  * MIDI playback that locked up when using an empty MUS / MIDI file
    (thanks Alexandre Xavier).
  * Default sampling rate used by setup tool changed to 44100Hz, to
    match the game default (thanks Alexandre Xavier).
  * Cheat codes and menu hot keys now work when shift is held down or
    capslock turned on (thanks Alexandre Xavier).

### libtextscreen
  * The background on GUI controls now lights up when hovering over
    them, so that it is more obvious what you are selecting.
  * It is now possible to type a “+” in input boxes (thanks Alexandre
    Xavier).
  * It is possible to use the mouse wheel to scroll through scroll
    panes.
  * Clicking on scroll bars now moves the scroll handle to a matching
    location.
  * Clicking outside a dropdown list popup window now dismisses the
    window.
  * Window hotkeys that are an alphabetical letter now work when shift
    is held down or capslock turned on (thanks Alexandre Xavier).

## 1.5.0 (2011-01-02)

  Big changes in this version:

  * The DOSbox OPL emulator (DBOPL) has been imported to replace the
    older FMOPL code.  The quality of OPL emulation is now therefore
    much better.
  * The game can now run in screen modes at any color depth (not just
    8-bit modes).  This is mainly to work around problems with Windows
    Vista/7, where 8-bit color modes don’t always work properly.
  * Multiplayer servers now register themselves with an Internet
    master server.  Use the -search command line parameter to find
    servers on the Internet to play on.  You can also use DoomSeeker
    (http://skulltag.net/doomseeker/) which supports this
    functionality.
  * When running in windowed mode, it is now possible to dynamically
    resize the window by dragging the window borders.
  * Names can be specified for servers with the -servername command
    line parameter.
  * There are now keyboard, mouse and joystick bindings to cycle
    through available weapons, making play with joypads or mobile
    devices (ie. without a proper keyboard) much more practical.
  * There is now a key binding to change the multiplayer spy key
    (usually F12).
  * The setup tool now has a “warp” button on the main menu, like
    Vanilla setup.exe (thanks Proteh).
  * Up to 8 mouse buttons are now supported (including the
    mousewheel).
  * A new command line parameter has been added (-solo-net) which can
    be used to simulate being in a single player netgame.
  * There is now a configuration file parameter to set the OPL I/O
    port, for cards that don’t use port 0x388.
  * The Python scripts used for building Chocolate Doom now work with
    Python 3 (but also continue to work with Python 2) (thanks arin).
  * There is now a NOT-BUGS file included that lists some common
    Vanilla Doom bugs/limitations that you might encounter (thanks to
    Sander van Dijk for feedback).

### Compatibility
  * The -timer and -avg options now work the same as Vanilla when
    playing back demos (thanks xttl)
  * A texture lookup bug was fixed that caused the wrong sky to be
    displayed in Spooky01.wad (thanks Porsche Monty).
  * The HacX v1.2 IWAD file is now supported, and can be used
    standalone without the need for the Doom II IWAD (thanks atyth).
  * The I_Error function doesn’t display “Error:” before the error
    message, matching the Vanilla behavior.  “Error” has also been
    removed from the title of the dialog box that appears on Windows
    when this happens.  This is desirable as not all such messages are
    actually errors (thanks Proteh).
  * The setup tool now passes through all command line arguments when
    launching the game (thanks AlexXav).
  * Demo loop behavior (ie. whether to play DEMO4) now depends on the
    version being emulated.  When playing Final Doom the game will
    exit unexpectedly as it tries to play the fourth demo - this is
    Vanilla behaviour (thanks AlexXav).

### Bugs fixed
  * A workaround has been a bug in old versions of SDL_mixer (v1.2.8
    and earlier) that could cause the game to lock up.  Please upgrade
    to a newer version if you haven’t already.
  * It is now possible to use OPL emulation at 11025Hz sound sampling
    rate, due to the new OPL emulator (thanks Porsche Monty).
  * The span renderer function (used for drawing floors and ceilings)
    now behaves the same as Vanilla Doom, so screenshots are
    pixel-perfect identical to Vanilla Doom (thanks Porsche Monty).
  * The zone memory system now aligns allocated memory to 8-byte
    boundaries on 64-bit systems, which may fix crashes on systems
    such as sparc64 (thanks Ryan Freeman and Edd Barrett).
  * The configure script now checks for libm, fixing compile problems
    on Fedora Linux (thanks Sander van Dijk).
  * Sound distortion with certain music files when played back using
    OPL (eg. Heretic title screen).
  * Error in Windows when reading response files (thanks Porsche
    Monty, xttl, Janizdreg).
  * Windows Vista/7 8-bit color mode issues (the default is now to run
    in 32-bit color depth on these versions) (thanks to everybody who
    reported this and helped test the fix).
  * Screen borders no longer flash when running on widescreen
    monitors, if you choose a true-color screen mode (thanks exp(x)).
  * The controller player in a netgame is the first player to join,
    instead of just being someone who gets lucky.
  * Command line arguments that take an option now check that an
    option is provided (thanks Sander van Dijk).
  * Skill level names in the setup tool are now written the same as
    they are on the in-game “new game” menu (thanks AlexXav).
  * There is no longer a limit on the lengths of filenames provided to
    the -record command line parameter (thanks AlexXav).
  * Window title is not lost in setup tool when changing video driver
    (thanks AlexXav).

### libtextscreen
  * The font used for the textscreen library can be forced by setting
    the TEXTSCREEN_FONT environment variable to “small” or “normal”.
  * Tables or scroll panes that don’t contain any selectable widgets
    are now themselves not selectable (thanks Proteh).
  * The actions displayed at the bottom of windows are now laid out in
    a more aesthetically pleasing way.

## 1.4.0 (2010-07-10)

  The biggest change in this version is the addition of OPL emulation.
  This emulates Vanilla Doom’s MIDI playback when using a Yamaha OPL
  synthesizer chip, as was found on SoundBlaster compatible cards.

  A software OPL emulator is included as most modern computers do not
  have a hardware OPL chip any more.  If you do have one, you can
  configure Chocolate Doom to use it; see README.OPL.

  The OPL playback feature is not yet perfect or 100% complete, but is
  judged to be good enough for general use.  If you find music that
  does not play back properly, please report it as a bug.

### Other changes
  * The REJECT overflow emulation code from PrBoom+ has been
    imported.  This fixes demo desync on some demos, although
    others will still desync.
  * Warnings are now generated for invalid dehacked replacements of
    printf format strings.  Some potential buffer overflows are also
    checked.
  * The installation instructions (INSTALL file) have been clarified
    and made more platform-agnostic.
  * The mouse is no longer warped to the center of the screen when the
    demo sequence advances.
  * Key bindings can now be changed for the demo recording quit key
    (normally ‘q’) and the multiplayer messaging keys (normally ‘t’,
    ‘g’, ‘i’, ‘b’ and ‘r’).

## 1.3.0 (2010-02-10)

  * Chocolate Doom now runs on Windows Mobile/Windows CE!
  * It is possible to rebind most/all of the keys that control the
    menu, shortcuts, automap and weapon switching.  The main reason
    for this is to support the Windows CE port and other platforms
    where a full keyboard may not be present.
  * Chocolate Doom now includes a proper Mac OS X package; it is no
    longer necessary to compile binaries for this system by hand.  The
    package includes a simple graphical launcher program and can be
    installed simply by dragging the “Chocolate Doom” icon to the
    Applications folder. (thanks to Rikard Lang for extensive testing
    and feedback)
  * The video mode auto-adjust code will automatically choose windowed
    mode if no fullscreen video modes are available.
  * The zone memory size is automatically reduced on systems with a
    small amount of memory.
  * The “join game” window in the setup tool now has an option to
    automatically join a game on the local network.
  * Chocolate Doom includes some initial hacks for compiling under
    SDL 1.3.
  * Recent versions of SDL_mixer include rewritten MIDI code on Mac OS
    X.  If you are using a version of SDL_mixer with the new code,
    music will now be enabled by default.
  * Windows Vista and Windows 7 no longer prompt for elevated
    privileges when running the setup tool (thanks hobbs and MikeRS).
  * The Windows binaries now have better looking icons (thanks
    MikeRS).
  * Magic values specified using the -spechit command line parameter
    can now be hexadecimal.
  * DOOMWADDIR/DOOMWADPATH can now specify the complete path to IWAD
    files, rather than the path to the directory that contains them.
  * When recording shorttics demos, errors caused by the reduced
    turning resolution are carried forward, possibly making turning
    smoother.
  * The source tarball can now be used to build an RPM package:
    rpmbuild -tb chocolate-doom-VER.tar.gz

### Compatibility
  * The A_BossDeath behavior in v1.9 emulation mode was fixed (thanks
    entryway)
  * The “loading” disk icon is drawn more like how it is drawn in
    Vanilla Doom, also fixing a bug with chook3.wad.
  * Desync on 64-bit systems with ep1-0500.lmp has (at long last) been
    fixed (thanks exp(x)).
  * Donut overrun emulation code imported from Prboom+ (thanks
    entryway).
  * The correct level name should now be shown in the automap for
    pl2.wad MAP33 (thanks Janizdreg).
  * In Chex Quest, the green radiation suit colormap is now used
    instead of the red colormaps normally used when taking damage or
    using the berserk pack.  This matches Vanilla chex.exe behavior
    (thanks Fuzztooth).
  * Impassible glass now displays and works the same as in Vanilla,
    fixing wads such as OTTAWAU.WAD (thanks Never_Again).

### Bugs fixed
  * Memory-mapped WAD I/O is disabled by default, as it caused various
    issues, including a slowdown/crash with Plutonia 2 MAP23.  It can
    be explicitly re-enabled using the “-mmap” command line parameter.
  * Crash when saving games due to the ~/.chocolate-doom/savegames
    directory not being created (thanks to everyone who reported
    this).
  * Chocolate Doom will now run under Win95/98, as the
    SetProcessAffinityMask function is looked up dynamically.
  * Compilation under Linux with older versions of libc will now work
    (the semantics for sched_setaffinity were different in older
    versions)
  * Sound clipping when using libsamplerate was improved (thanks David
    Flater)
  * The audio buffer size is now calculated based on the sample rate,
    so there is not a noticeable delay when using a lower sample rate.
  * The manpage documentation for the DOOMWADPATH variable was fixed
    (thanks MikeRS).
  * Compilation with FEATURE_MULTIPLAYER and FEATURE_SOUND disabled
    was fixed.
  * Fixed crash when using the donut special type and the joining
    linedef is one sided (thanks Alexander Waldmann).
  * Key settings in a configuration file that are out of range do not
    cause a crash (thanks entryway).
  * Fix ear-piercing whistle when playing the MAP05 MIDI music using
    timidity with EAWPATS (thanks entryway / HackNeyed).

### libtextscreen
  * There is now a second, small textscreen font, so that the ENDOOM
    screen and setup tool can be used on low resolution devices
    (eg. PDAs/embedded devices)
  * The textscreen library now has a scrollable pane widget. Thanks to
    LionsPhil for contributing code to scroll up and down using the
    keyboard.
  * Doxygen documentation was added for the textscreen library.

## 1.2.1 (2008-12-10)

  This version just fixes a crash at the intermission screen when
  playing Doom 1 levels.

## 1.2.0 (2008-12-10)

  Happy 15th Birthday, Doom!

  * Chocolate Doom now has an icon that is not based on the
    proprietary Doom artwork.
  * There is now memory-mapped WAD I/O support, which should be useful
    on some embedded systems.
  * Chex quest emulation support is now included, although an
    auxiliary dehacked patch is needed (chexdeh.zip in the idgames
    archive).

### Compatibility
  * The armor class is always set to 2 when picking up a megasphere
    (thanks entryway).
  * The quit screen prompts to quit “to dos” instead of just to quit
    (thanks MikeRS)
  * The “dimensional shambler” quit message was fixed.
  * Fix crash related to A_BFGSpray with NULL target when using
    dehacked patches - discovered with insaned2.deh (thanks CSonicGo)
  * NUL characters are stripped from dehacked files, to ensure correct
    behavior with some dehacked patches (eg. the one with portal.wad).

### Bugs fixed
  * “Python Image Library” should have been “Python Imaging Library”
    (thanks exp(x)).
  * The setup tool should no longer ask for elevated permissions on
    Windows Vista (this fix possibly may not work).
  * The application icon is set properly when running under Windows XP
    with the “Luna” theme.
  * Fix compilation under Cygwin to detect libraries and headers from
    the correct environment.
  * The video code does not try to read SDL events before SDL has been
    properly initialised - this was causing problems with some older
    versions of SDL.

## 1.1.1 (2008-04-20)

  The previous release (v1.1.0) included a bug that broke compilation
  when libsamplerate support was enabled.  The only change in this
  version is to fix this bug.

## 1.1.0 (2008-04-19)

  * The video mode code has been radically restructured.  The video
    mode is now chosen by directly specifying the mode to use; the
    scale factor is then chosen to fit the screen.  This is helpful
    when using widescreen monitors (thanks Linguica)
  * MSVC build project files (thanks GhostlyDeath and entryway).
  * Unix manpage improvements; the manpage now lists the environment
    variables that Chocolate Doom uses.  Manpages have been added for
    chocolate-setup and chocolate-server, from the versions for the
    Debian Chocolate Doom package (thanks Jon Dowland).
  * INSTALL file with installation instructions for installing
    Chocolate Doom on Unix systems.
  * Support for high quality resampling of sound effects using
    libsamplerate (thanks David Flater).
  * A low pass filter is applied when doing sound resampling in an
    attempt to filter out high frequency noise from the resampling
    process.
  * R_Main progress box is not displayed if stdout is a file (produces
    cleaner output).
  * Client/server version checking can be disabled to allow different
    versions of Chocolate Doom to play together, or Chocolate Doom
    clients to play with Strawberry Doom clients.
  * Unix manpages are now generated for the Chocolate Doom
    configuration files.
  * The BSD PC speaker driver now works on FreeBSD.

### Compatibility
  * Use the same spechits compatibility value as PrBoom+, for
    consistency (thanks Lemonzest).
  * The intercepts overrun code has been refactored to work on big
    endian machines.
  * The default startup delay has been set to one second, to allow
    time for the screen to settle before starting the game (some
    monitors have a delay before they come back on after changing
    modes).
  * If a savegame buffer overrun occurs, the savegame does not get
    saved and existing savegames are not overwritten (same behaviour
    as Vanilla).

### Bugs fixed
  * Desync with STRAIN demos and dehacked Misc values not being set
    properly (thanks Lemonzest)
  * Don’t grab the mouse if the mouse is disabled via -nomouse or
    use_mouse in the configuration file (thanks MikeRS).
  * Don’t center the mouse on startup if the mouse is disabled (thanks
    Siggi).
  * Reset the palette when the window is restored to clear any screen
    corruption (thanks Catoptromancy).
  * mus2mid.c should use `MEM_SEEK_SET`, not `SEEK_SET` (thanks
    Russell)
  * Fast/Respawn options were not being exchanged when starting
    netgames (thanks GhostlyDeath).
  * Letterbox mode is more accurately described as “pillarboxed” or
    “windowboxed” where appropriate (thanks MikeRS)
  * Process affinity mask is set to 1 on Windows, to work around a bug
    in SDL_mixer that can cause crashes on multi-core machines (thanks
    entryway).
  * Bugs in the joystick configuration dialog in the setup tool have
    been fixed.

## 1.0.0 (2007-12-10)

  This release is dedicated to Dylan “Toke” McIntosh, who was
  tragically killed in a car crash in 2006.  I knew Dylan from IRC and
  the Doomworld forums for several years, and he had a deep passion
  for this game.  He was also a huge help for me while developing
  Chocolate Doom, as he helped point out a lot of small quirks in
  Vanilla Doom that I didn’t know about. His death is a great loss.
  RIP Toke.

  This is the first release to reach full feature parity with Vanilla
  Doom.  As a result, I have made this version 1.0.0, so Chocolate
  Doom is no longer beta!

### Big new features
  * Multiplayer!  This version includes an entirely new multiplayer
    engine, based on a packet server architecture.  I’d like to thank
    joe, pritch, Meph and myk, and everyone else who has helped test
    the new code for their support, feedback and help in testing this.
    The new code still needs more testing, and I’m eager to hear any
    feedback on this.
  * A working setup tool.  This has the same look and feel as the
    original setup.exe.  I hope people like it!  Note that it has some
    advantages over the original setup.exe - for example, you can use
    the mouse.

### Other new features
  * New mus conversion code thanks to Ben Ryves.  This converts the
    Doom .mus format to .mid a lot better.  As one example, tnt.wad
    Map02 is now a lot closer to how Vanilla says.  Also, the music on
    the deca.wad titlescreen now plays!
  * x3, x4 and x5 display scale (thanks to MikeRS for x5 scale).
  * Fullscreen “letterbox” mode allows Chocolate Doom to run on
    machines where 1.6:1 aspect ratio modes are unavailable
    (320x200/640x400).  The game runs in 320x240/640x480 instead, with
    black borders.  The system automatically adjusts to this if closer
    modes are unavailable.
  * Aspect ratio correction: you can (also) run at 640x480 without
    black borders at the top and bottom of the screen.
  * PC speaker sound effect support.  Chocolate Doom can output real
    PC speaker sounds on Linux, or emulate a PC speaker through the
    sound card.
  * Working three-screen mode, as seen in early versions of Doom!  To
    test this out, put three computers on a LAN and type:

        chocolate-doom -server
        chocolate-doom -autojoin -left
        chocolate-doom -autojoin -right

  * Allow a delay to be specified on startup, to allow the display to
    settle after changing modes before starting the game.
  * Allow the full path and filename to be specified when loading
    demos: It is now possible to type “chocolate-doom -playdemo
    /tmp/foo.lmp” for example.
  * Savegames are now stored in separate directories depending on the
    IWAD: eg. the savegames for Doom II are stored in a different
    place to those for Doom I, Final Doom, etc. (this does not affect
    Windows).
  * New mouse acceleration code works based on a threshold and
    acceleration.  Hopefully this should be closer to what the DOS
    drivers do.  There is a ‘test’ feature in the setup tool to help
    in configuring this.
  * New “-nwtmerge” command line option that emulates NWT’s “-merge”
    option.  This allows TiC’s Obituary TC to be played.
  * The ENDOOM screen no longer closes automatically, you have to
    click the window to make it go away.
  * Spechit overrun fixes and improvements.  Thanks to entryway for
    his continued research on this topic (and because I stole your
    improvements :-).  Thanks to Quasar for reporting a bug as well.
  * Multiple dehacked patches can be specified on the command line, in
    the same way as with WADs - eg. -deh foo.deh bar.deh baz.deh.
  * Default zone memory size increased to 16MB; this can be controlled
    using the -mb command-line option.
  * It is now possible to record demos of unlimited length (by
    default, the Vanilla limit still applies, but it can now be
    disabled).
  * Autoadjusting the screen mode can now be disabled.
  * On Windows, the registry is queried to detect installed versions
    of Doom and automatically locate IWAD files.  IWADs installed
    through Steam are also autodetected.
  * Added DOOMWADPATH that can be used like PATH to specify multiple
    locations in which to search for IWAD files.  Also, “-iwad” is now
    enhanced, so that eg. “-iwad doom.wad” will now search all IWAD
    search paths for “doom.wad”.
  * Improved mouse tracking that should no longer lag.  Thanks to
    entryway for research into this.
  * The SDL driver can now be specified in the configuration file.
    The setup tool has an option on Windows to select between DirectX
    and windib.
  * Joystick support.
  * Configuration file option to change the sound sample rate.
  * More than three mouse buttons are now supported.

### Portability improvements
  * Chocolate Doom now compiles and runs cleanly on MacOS X.  Huge
    thanks go to Insomniak who kindly gave me an account on his
    machine so that I could debug this remotely.  Big thanks also go
    to athanatos on the Doomworld forums for his patience in testing
    various ideas as I tried to get Chocolate Doom up and running on
    MacOS.
  * Chocolate Doom now compiles and runs natively on AMD64.
  * Chocolate Doom now compiles and runs on Solaris/SPARC, including
    the Sun compiler.  Thanks to Mike Spooner for some portability
    fixes.
  * Improved audio rate conversion, so that sound should play properly
    on machines that don’t support low bitrate output.

### Compatibility fixes
  * Check for IWADs in the same order as Vanilla Doom.
  * Dehacked code will now not allow string replacements to be longer
    than those possible through DOS dehacked.
  * Fix sound effects playing too loud on level 8 (thanks to myk for
    his continued persistence in getting me to fix this)
  * Save demos when quitting normally - it is no longer necessary to
    press ‘q’ to quit and save a demo.
  * Fix spacing of -devparm mode dots.
  * Fix sky behavior to be the same as Vanilla Doom - when playing in
    Doom II, the skies never change from the sky on the first level
    unless the player loads from a savegame.
  * Make -nomouse and config file use_mouse work again.
  * Fix the -nomusic command-line parameter.  Make the snd_sfxdevice
    snd_musicdevice values in the configuration file work, so that it
    is possible to disable sound, as with Vanilla.
  * Repeat key presses when the key is held down (this is the Vanilla
    behavior) - thanks to Mad_Mac for pointing this out.
  * Don’t print a list of all arguments read from response files -
    Vanilla doesn’t do this.
  * Autorun only when joyb_speed >= 10, not >= 4.  Thanks to Janizdreg
    for this.
  * Emulate a bug in DOS dehacked that can overflow the dehacked frame
    table and corrupt the weaponinfo table.  Note that this means
    Batman Doom will no longer play properly (identical behavior to
    Vanilla); vbatman.deh needs to also be applied to fix it.  (Thanks
    grazza)
  * Allow dehacked 2.3 patches to be loaded.
  * Add more dehacked string replacements.
  * Compatibility option to enable or disable native key mappings.
    This means that people with non-US keyboards can decide whether to
    use their correct native mapping or behave like Vanilla mapping
    (which assumes all keyboards are US).
  * Emulate overflow bug in P_FindNextHighestFloor.  Thanks to
    entryway for the fix for this.
  * Add -netdemo command line parameter, for playing back netgame
    demos recorded with a single player.
  * The numeric keypad now behaves like Vanilla Doom does.
  * Fix some crashes when loading from savegames.
  * Add intercepts overrun emulation from PrBoom-plus.  Thanks again
    to entryway for his research on this subject.
  * Add playeringame overrun emulation.

### Bugs fixed
  * Fix crash when starting new levels due to the intermission screen
    being drawn after the WI_ subsystem is shut down (thanks pritch
    and joe)
  * Catch failures to initialise sound properly, and fail gracefully.
  * Fix crasher in 1427uv01.lmp (thanks ultdoomer)
  * Fix crash in udm1.wad.
  * Fix crash when loading a savegame with revenant tracer missiles.
  * Fix crash when loading a savegame when a mancubus was in the
    middle of firing.
  * Fix Doom 1 E1-3 intermission screen animations.
  * Fix loading of dehacked “sound” sections.
  * Make sure that modified copyright banners always end in a newline
    - this fixes a bug with av.wad (thanks myk)
  * Added missing quit message (“are you sure you want to quit this
    great game?”).
  * Fix when playing long sound effects - the death sound in
    marina.wad now plays properly, for example.
  * Fix buffer overrun on the quicksave prompt screen that caused a
    mysterious cycling character to appear.
  * IDCLEV should not work in net games (thanks Janizdreg)
  * Stop music playing at the ENDOOM screen.
  * Fix sound sample rate conversion crash.
  * Fix “pop” heard at the end of sound effects.
  * Fix crash when playing long sounds.
  * Fix bug with -timedemo accuracy over multi-level demos.
  * Fix bug with the automap always following player 1 in multiplayer
    mode (thanks Janizdreg).

## 0.1.4 (2006-02-13)

  * NWT-style merging command line options (allows Mordeth to be played)
  * Unix manpage (thanks Jon Dowland)
  * Dehacked improvements/fixes:
     * Allow changing the names of graphic lumps used in menu, status bar
       intermission screen, etc.
     * Allow changing skies, animated flats + textures
     * Allow changing more startup strings.
     * Allow text replacements on music + sfx lump names
  * Fix for plutonia map12 crash.
  * Fix bug with playing long sfx at odd sample rates.
  * Big Endian fixes (for MacOS X).  Thanks to athanatos for helping
    find some of these.
  * Install into /usr/games, rather than /usr/bin (thanks Jon Dowland)

## 0.1.3 (2006-01-20)

  * Imported the spechit overrun emulation code from prboom-plus.  Thanks to
    Andrey Budko for this.
  * New show_endoom option in the chocolate-doom.cfg config file allows
    the ENDOOM screen to be disabled.
  * Chocolate Doom is now savegame-compatible with Vanilla Doom.

  * Fixes for big endian machines (thanks locust)
  * Fixed the behavior of the dehacked maximum health setting.
  * Fix the “-skill 0” hack to play without any items (thanks to Janizdreg
    for pointing out that this was nonfunctional)
  * Fix playing of sounds at odd sample rates (again).  Sound effects
    at any sample rate now play, but only sounds with valid headers.
    This is the *real* way Vanilla Doom behaves.  Thanks to myk for
    pointing out the incorrect behavior.

## 0.1.2 (2005-10-29)

  * Silence sounds at odd sample rates (rather than bombing out); this
    is the way Vanilla Doom behaves.
  * Handle multiple replacements of the same sprite in a PWAD.
  * Support specifying a specific version to emulate via the command line
    (-gameversion)
  * Fix help screen orderings and skull positions.  Behave exactly as
    the original executables do.

## 0.1.1 (2005-10-18)

  * Display startup “banners” if they have been modified through
    dehacked.
  * Dehacked “Misc” section support.

### Bugs fixed
  * Doom 1 skies always using Episode 1 sky
  * Crash when switching applications while running fullscreen
  * Lost soul bounce logic (do not bounce in Registered/Shareware)
  * Mouse buttons mapped incorrectly (button 1 is right, 2 is middle)
  * Music not pausing when game is paused, when using SDL_mixer’s
    native MIDI playback.
  * Pink icon on startup (palette should be fully set before anything is
    loaded)

## 0.1.0 (2005-10-09)

  * Dehacked support
  * WAD merging for TCs
  * ENDOOM display
  * Fix bug with invalid MUS files causing crashes
  * Final Doom fixes

## 0.0.4 (2005-09-27)

  * Application icon and version info included in Windows .exe files
  * Fixes for non-x86 architectures
  * Fix uac_dead.wad (platform drop on e1m8 should occur when all
    bosses die, not just barons)
  * Fix “loading” icon to work for all graphics modes

## 0.0.3 (2005-09-17)

  * Mouse acceleration code to emulate the behaviour of old DOS mouse
    drivers (thanks to Toke for information about this and
    suggestions)
  * Lock surfaces properly when we have to (fixes crash under
    Windows 98)

## 0.0.2 (2005-09-13)

  * Remove temporary MIDI files generated by sound code.
  * Fix sound not playing at the right volume
  * Allow alt-tab away while running in fullscreen under Windows
  * Add second configuration file (chocolate-doom.cfg) to allow
    chocolate-doom specific settings.
  * Fix switches not changing in Ultimate Doom

## 0.0.1 (2005-09-07)

  First beta release
