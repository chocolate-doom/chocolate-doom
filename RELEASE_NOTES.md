## 3.1.0 (2024-08-01)

### General
  * Drag and drop loading of WAD, Dehacked and demo files is now supported on
    Windows - simply drag the files onto chocolate-doom.exe (thanks Fabian).
  * WAD file autoloading was added - WAD and DEH files can be copied into
    an autoload folder to be automatically included on every game start.
  * Music pack configuration has been significantly simplified. By simply
    copying .flac/.ogg music files into a folder they will be automatically
    detected by filename and used.
  * Music packs can now be used with OPL as a fallback, and music pack
    config files can have any name ending in '.cfg'.
  * MP3 music packs are now supported.
  * Network synchronization now uses a PID controller by default, which
    makes games more smooth and more stable, especially for Internet play.
  * UDP hole punching is now used to make servers behind NAT gateways
    automatically accessible to the Internet.
  * OPL emulation now uses Nuked OPL3 v1.8 (thanks nukeykt)
  * Allow simultaneous PC speaker emulation and OPL emulation
    (thanks Michael Day).
  * The setup tool now uses "Romero Blue" as a background (see the wiki:
    <https://doomwiki.org/wiki/Romero_Blue> for more info).
  * The 0 and 5 keys on the number pad can now be bound independently
    of any other keyboard key (thanks BlooD2ool).
  * With aspect ratio correction disabled, the game can scale to any
    arbitrary size and remove all black borders in full screen mode.
    (thanks chungy)
  * The executable's location is now checked when looking for IWADs.
  * The IWAD files installed by Steam-on-Linux are now detected (thanks
    chungy).
  * It's now possible to use `-response` to load response files.
  * Default savegame name now includes the WAD filename (thanks Fabian).
  * Mouse movement is no longer read when the game window is inactive
    (thanks Julia Nechaevskaya).
  * Chocolate Doom now by default generates a "pet name" for the user
    to respect privacy (thanks Jon!)
  * The Freedoom single-player IWAD files are now officially supported,
    since recent versions changed all levels to be vanilla compatible.
  * Add native support for the FluidSynth midi synthesizer.
  * Add a fsynth_gain config key to fine tune the FluidSynth output level
    (thanks Fabian).
  * Redesign of native MIDI support in Windows (thanks ceski and Roman Fomin):
    - Emulates DMX MPU-401 feature set by default, change with
      `winmm_complevel`.
    - MIDI device reset between songs using "GM System On", change with
      `winmm_reset_type`.
    - Configurable delay after reset for legacy MIDI hardware (e.g. Roland
      SC-55), change with `winmm_reset_delay`.
  * Add improved gamepad support via the SDL\_GameController interface. This
    includes support for analog triggers, modern dual-stick default bindings
    (based on Unity Doom), descriptive button names for common controller types
    and configurable dead zones for stick axes. (Michael Day).
  * All games now have shiny new icons!
  * The -display parameter was added to specify the display number on which to show the screen (thanks Robin Emeršič).

### Build systems
  * SDL2_mixer and SDL2_net can now be optionally disabled to increase
    portability to systems that might not support audio or networking
    (thanks turol).
  * Travis CI was replaced with GitHub Actions (thanks Michael Francis).
  * mingw CI was added for automatic Windows-building (thanks turol).
  * A cpp-linter CI was added to improve the quality and formatting of code
    (thanks turol).
  * The `--disable-zpool` autoconf argument for disabling memory pooling was
    added to make debugging easier (thanks turol).
  * Python 2 has been deprecated and Python 3 is now used exclusively
  (thanks Fabian).

### Refactorings
  * CMake project files have been added, replacing the Microsoft Visual
    Studio and Code::Blocks files. CMake maintains support for multiple
    IDEs and versions thereof, and reduces developer overhead when updating
    Chocolate Doom (huge thanks to AlexMax for this work).
  * Source code has been retrofitted to fix many compiler warnings and
    add const annotations to many variables (thanks turol).
  * Several functions have been hardened against incomplete reads and
    error conditions, and made safer (thanks turol).
  * Man page generation has been reworked to use autoconf macro
    substitution, making it eaiser for downstream forks to change the
    project name (thanks Jon).
  * We now print a meaningful error message when a savegame cannot be
    loaded (thanks Zodomaniac, chungy).
  * There's now a log file feature for the network code to aid in tracking
    down multiplayer bugs.
  * AppData files were updated to the AppStream standard (thanks Mike).
  * Many abuses of the extern keyword have been cleaned up to prevent
    undefined behavior (thanks turol).
  * Install of bash completion scripts was fixed (thanks Mike Swanson).

### Bug fixes
  * Fixed an exception thrown by the Windows kernel when debugging with
    GDB (thanks AXDOOMER).
  * Loop metadata now works properly with music packs on Windows.
  * Mouse movement is ignored when the game window isn't active (thanks
    Julia Nechaevskaya).
  * A bug was fixed where music would not play after pausing on an
    intermission screen (thanks Julia Nechaevskaya).
  * Timeouts when connecting to a network server were fixed (thanks
    @bradc6).
  * A long-standing bug where some visplane overflows caused crashes was
    fixed (thanks Michael Francis).
  * A multiplayer deadlock bug where clients would stop sending tics after
    missing tics from the server was fixed. There are both client- and
    server- side fixes to fix the problem when playing with older versions
    (thanks MadDog and Mortrixs for help tracking this down).
  * The macOS launcher now quits automatically when all windows are closed.
  * The mouse speed calibration thermometer was fixed.
  * Some improvements have been made to PC speaker emulation to better match
    Vanilla (thanks NY00123).
  * Filenames and paths with non-latin characters now work on Windows
    (thanks Roman Fomin).
  * Properly handle orphan carriage returns when parsing deh files
    (thanks Michael Day).

### Doom
  * Doom 1.2 demo support was added (thanks James Canete!)
  * Map33 intermission screen and map33-map35 automap names are
    emulated (thanks CapnClever).
  * We now exit gracefully when player starts are missing (thanks Michael
    Francis).
  * We now exit gracefully on levels with a boss brain and no boss spitter
    things (thanks Jason Benaim).
  * It's now possible to play multiplayer with gameversion=1.2.
  * When the game hits the limit for the number of wall scrollers, a count
    of the number of scrollers is now displayed (thanks Jon).
  * Sector special 17 (random light flicker) is now accurately emulated to
    only appear in gameversion > 1.2 (thanks tpoppins).
  * Fixed mouse events causing unwanted key presses (thanks Michael Day).
  * French Doom II files doom2f.wad and french.deh are now automatically
    detected (thanks Acts19quiz).
  * Mouse buttons are now bindable to run (thanks Archenoth).
  * The "Sky never changes in Doom II" bug is now accurately emulated
    (thanks Michael Francis).
  * Mouse buttons are now bindable to turn left/right (thanks Fabian).
  * Fix possible undefined behavior and accurately emulate vanilla when -skill
    parameter is given a negative value (thanks Henrique Jorge).

### Heretic
  * P\_FindNextHighestFloor was changed to match vanilla behavior (thanks
    AXDOOMER).
  * WAD hash table is now generated for speed (thanks Michael Francis).
  * HHE level name replacements now apply on the intermission screen
    (thanks ETTiNGRiNDER).
  * Mouse buttons are now bindable to scroll through inventory and
    "use artifact" (thanks Michael Day).
  * Mouse buttons are now bindable to run (thanks Archenoth).
  * Multiplayer demo playback was fixed (thanks Ryan Krafnick).
  * The GOG.com release is now detected (thanks Mike Swanson).
  * Mouse buttons are now bindable to turn left/right (thanks Fabian).
  * Fix NULL backsector crash (thanks kitchen-ace and Julia Nechaevskaya).

### Hexen
  * ACS code has been hardened against potential security vulnerabilities.
  * WAD hash table is now generated for speed (thanks AXDOOMER).
  * Mouse buttons are now bindable to scroll through inventory and
    "use artifact" (thanks Michael Day).
  * Mouse buttons are now bindable to run (thanks Archenoth).
  * Multiplayer demo playback was fixed (thanks Ryan Krafnick).
  * The GOG.com release is now detected (thanks Mike Swanson).
  * Mouse buttons are now bindable to turn left/right (thanks Fabian).
  * Fixed demo desyncs in P_LookForPlayers (thanks Michael Day)
  * Add support for two different v1.1 variants through -gameversion
    argument (thank NY00123).

### Strife
  * Sehacked replacements of the "empty slot" string now work.
  * VOICES.WAD is now found in a case-insensitive way (thanks Michael Francis).
  * PC speaker sound effects are now supported (thanks ceski).
  * Mouse buttons are now bindable to turn left/right (thanks Fabian).
  * Fixed demo desyncs in P_LookForPlayers (thanks Michael Day)
  * Fix possible undefined behavior and accurately emulate vanilla when -skill
    parameter is given a negative value (thanks Henrique Jorge).
