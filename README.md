# Crispy Doom
[![Crispy Doom Icon](https://www.chocolate-doom.org/wiki/images/b/be/Crispy-doom.png)](https://github.com/fabiangreffrath/crispy-doom)

[![Top Language](https://img.shields.io/github/languages/top/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom)
[![Code Size](https://img.shields.io/github/languages/code-size/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom)
[![License](https://img.shields.io/github/license/fabiangreffrath/crispy-doom.svg?style=flat&logo=gnu)](https://github.com/fabiangreffrath/crispy-doom/blob/master/COPYING.md)
[![Release](https://img.shields.io/github/release/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Release Date](https://img.shields.io/github/release-date/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Downloads](https://img.shields.io/github/downloads/fabiangreffrath/crispy-doom/latest/total.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Commits](https://img.shields.io/github/commits-since/fabiangreffrath/crispy-doom/latest.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/commits/master)
[![Last Commit](https://img.shields.io/github/last-commit/fabiangreffrath/crispy-doom.svg?style=flat)](https://github.com/fabiangreffrath/crispy-doom/commits/master)
[![Travis Build Status](https://img.shields.io/travis/com/fabiangreffrath/crispy-doom.svg?style=flat&logo=travis)](https://travis-ci.com/fabiangreffrath/crispy-doom/)

Crispy Doom is a limit-removing enhanced-resolution Doom source port based on [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom).

Its name means that its internal 640x400 resolution looks "crisp" and is also a [slight reference](http://www.mathsisfun.com/recipie.html) to its origin.

## Synopsis

Crispy Doom is a friendly fork of [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom) that provides a higher display resolution, removes the [static limits](https://doomwiki.org/wiki/Static_limits) of the Doom engine and offers further optional visual, tactical and physical enhancements while remaining entirely config file, savegame, netplay and demo compatible with the original.

## Objectives and features

Crispy Doom is a source port that aims to provide a faithful Doom gaming experience while also featuring some user-requested improvements and enhancements. It is forked off of Chocolate Doom to take advantage of its free and open-source code base, portability, accuracy and compatibility with Vanilla Doom.

Its core features are:

 * Enhanced 640x400 display resolution, with the original 320x200 resolution still available in the "High Resolution Rendering: Off" mode.
 * Widescreen rendering for using all the available horizontal space of screens with aspect ratios up to 24:9.
 * Uncapped rendering framerate with interpolation and optional vertical synchronization (VSync) with the screen refresh rate.
 * Intermediate gamma correction levels (0.5, 1.5, 2.5 and 3.5).
 * Removal of all static engine limits, or at least raising of the less crucial ones.
 * Full support for the "Doom Classic" WADs shipped with the "Doom 3: BFG Edition", especially the "No Rest For The Living" episode shipped in the NERVE.WAD file.
 * Support for all versions of John Romero's Episode 5: Sigil for Ultimate Doom.

Furthermore, the following optional user-visible and audible features are available:

 * Jumping.
 * Free vertical looking, including mouse look and vertical aiming.
 * Aiming support by a crosshair that may get directly rendered into the game world.
 * A new minimal Crispy HUD, displaying only the status bar numbers.
 * Clean Screenshot feature, enabling to take screenshots without HUD elements and even without status bar numbers and weapon sprites at higher screen sizes.
 * Colorized status bar numbers, HUD texts and blood sprites for certain monsters.
 * Translucency for certain sprites and status bar elements in the Crispy HUD.
 * Randomly mirrored death animations and corpse sprites.
 * Command line options to allow for playing with flipped player weapon sprites and/or entirely flipped level geometry.
 * Players may walk over or under monsters and hanging corpses.
 * Centered Weapons when firing, weapon recoil thrust and pitch.
 * Reports whenever a secret is revealed.
 * Level statistics and extended coloring in the Automap.
 * Playing sounds in full length, and misc. other sound fixes.
 * Demo recording and/or playback timers and progress bar.
 * Demo continue and take-over features, handing controls over to the player when demo playback is finished or interrupted.

Most of these features are disabled by default and need to get enabled either in the in-game "Crispness" menu, in the crispy-doom-setup tool or as command line parameters. They are implemented in a way that preserves demo-compatibility with Vanilla Doom and network game compatibility with Chocolate Doom. Furthermore, Crispy Doom's savegames and config files are compatible, though not identical (see the [Compatibility section in the Wiki](https://github.com/fabiangreffrath/crispy-doom/wiki/Compatibility)), to Vanilla Doom's.

Crispy Doom strives for maximum compatibility with all "limit-removing Vanilla" maps -- but not Boom or ZDoom maps. More specifically, Crispy Doom supports some select advanced features such as [ANIMATED](https://doomwiki.org/wiki/ANIMATED) and [SWITCHES](https://doomwiki.org/wiki/SWITCHES) lumps, MBF sky transfers, SMMU swirling flats and [MUSINFO](https://doomwiki.org/wiki/MUSINFO) -- but neither generalized linedef and sector types nor DECORATE and MAPINFO.

Many additional less user-visible features have been implemented, e.g. fixed engine limitations and crashes, fixed rendering bugs, fixed harmless game logic bugs, full support for DEHACKED files and lumps in BEX format, additional and improved cheat codes, an improved Automap, and many more! Due to the extra DEHACKED states added from [MBF](https://doomwiki.org/wiki/MBF), Crispy Doom supports [enhancer](https://www.doomworld.com/forum/topic/84859-black-ops-smooth-weapons-dehacked-mod) [mods](https://www.doomworld.com/forum/topic/85991-smoothed-smooth-monsters-for-doom-retro-and-crispy-doom) that can make the gameplay even more pleasing to the eyes. For a detailed list of features and changes please refer to the release notes below.

### New controls (with default bindings)

 * Move Forward (alt.) <kbd>W</kbd>
 * Move Backward (alt.) <kbd>S</kbd>
 * Strafe Left (alt.) <kbd>A</kbd>
 * Strafe Right (alt.) <kbd>D</kbd>
 * Jump (bindable to joystick and mouse buttons as well) <kbd>/</kbd> (as in Hexen and Strife)
 * Quick Reverse (bindable to mouse buttons as well)
 * Mouse Look (bindable to mouse buttons or permanent)
 * Look up (bindable to joystick axes as well) <kbd>PgDn</kbd> (as in Heretic)
 * Look down (bindable to joystick axes as well) <kbd>Del</kbd> (as in Heretic)
 * Center view <kbd>End</kbd> (as in Heretic)
 * Toggle always run <kbd>&#8682;</kbd>
 * Toggle vertical mouse movement (new in 5.4)
 * Delete savegame <kbd>Del</kbd>
 * Go to next level
 * Reload current level
 * Save a clean screenshot
 * Toggle Automap overlay mode <kbd>O</kbd>
 * Toggle Automap rotate mode <kbd>R</kbd>
 * Resurrect from savegame (single player mode only) "Run" + "Use"

### New command line parameters

 * `-dm3` specifies the Deathmatch 3.0 rules (weapons stay, items respawn) for netgames (since 4.1).
 * `-episode 1` launches Hell on Earth and `-episode 2` launches No Rest for the Living episode if the Doom 2 IWAD shipped with the Doom 3: BFG Edition is used.
 * `-warp 1a` warps to the secret level E1M10: Sewers of XBox Doom IWAD (since 2.3).
 * `-mergedump <file>` merges the PWAD file(s) given on the command line with the IWAD file and writes the resulting data into the `<file>` given as argument. May be considered as a replacement for the `DEUSF.EXE` tool (since 2.3).
 * `-lumpdump` dumps raw content of a lump into a file (since 5.7).
 * `-blockmap` forces a (re-)building of the BLOCKMAP lumps for loaded maps (since 2.3).
 * `-playdemo demoname -warp N` plays back fast-forward up to the requested map (since 3.0).
 * `-loadgame N -record demoname` and `-loadgame N -playdemo demoname` allow to record and play demos starting from a savegame instead of the level start (since 4.0).
 * `-playdemo demoname1 -record demoname2` plays back fast-forward until the end of demoname1 and continues recording as demoname2 (new in 5.5).
 * `-fliplevels` loads mirrored versions of the maps (this was the default on April 1st up to version 5.0).
 * `-flipweapons` flips the player's weapons (new in 5.3).
 * `-levelstat` prints a levelstat.txt file with statistics for each completed level (new in 5.9.0).

### New cheat codes

 * `TNTWEAP` followed by a weapon number gives or removes this weapon (8 = Chainsaw, 9 = SSG). `TNTWEAP0` takes away all weapons and ammo except for the pistol and 50 bullets. Try to load Doom 1 with `DOOM2.WAD` as a PWAD and type `TNTWEAP9` to play the SSG in Doom 1.
 * `TNTEM`, `KILLEM` or `FHHALL` kill all monsters on the current map (and disables all cube spitters).
 * `SPECHITS` triggers all [Linedef actions](https://doomwiki.org/wiki/Linedef_type) on a map at once, no matter if they are enabled by pushing, walking over or shooting or whether they require a key or not. It also triggers all boss monster and Commander Keen actions if possible.
 * `NOTARGET` or `FHSHH` toggle deaf and blind monsters that do not act until attacked.
 * `TNTHOM` toggles the flashing [HOM](https://doomwiki.org/wiki/Hall_of_mirrors_effect) indicator (disabled by default).
 * `SHOWFPS` or `IDRATE` toggle printing the FPS in the upper right corner.
 * `NOMOMENTUM` toggles a debug aid for pixel-perfect positioning on a map (not recommended to use in-game).
 * `GOOBERS` triggers an easter egg, i.e. an "homage to an old friend". ;-)
 * `IDBEHOLD0` disables all currently active power-ups (since 2.2).
 * `IDCLEV00` restarts the current level (since 2.0).
 * `IDMUS00` restarts the current music (new in 5.1).
 * `VERSION` shows the engine version, build date and SDL version (new in 5.1).
 * `SKILL` shows the current skill level (new in 5.5.2).

## Download

Binaries for Windows XP / Vista / 7 / 8.1 / 10 (32-bit binaries compatible with both x86 and x64 editions) are available here:
https://github.com/fabiangreffrath/crispy-doom/releases/download/crispy-doom-5.9.1/crispy-doom-5.9.1-win32.zip

To install on Ubuntu ("Eoan Ermine" 19.10 and later)/Debian ("Buster" 10 and later) based systems:
```bash
sudo apt-get install crispy-doom
```

Daily builds of Crispy Doom can be found here:
http://latest.chocolate-doom.org/

Crispy Doom can play nearly all variants of Doom. If you don't own any, you may download the [Shareware version of Doom](http://cdn.debian.net/debian/pool/non-free/d/doom-wad-shareware/doom-wad-shareware_1.9.fixed.orig.tar.gz), extract it and copy the DOOM1.WAD file into your Crispy Doom directory. Alternatively, you may want to play Crispy Doom with [Freedoom](https://www.chocolate-doom.org/wiki/index.php/Freedoom) and a MegaWAD.

### Sources
[![Open Hub](https://www.openhub.net/p/crispy-doom/widgets/project_thin_badge?style=flat&format=gif)](https://www.openhub.net/p/crispy-doom)

The Crispy Doom source code is available at GitHub: https://github.com/fabiangreffrath/crispy-doom.
It can be [downloaded in either ZIP or TAR.GZ format](https://github.com/fabiangreffrath/crispy-doom/releases) 
or cloned via

```
 git clone https://github.com/fabiangreffrath/crispy-doom.git
```

Brief instructions to set up a build system on Windows can be found [in the Crispy Doom Wiki](https://github.com/fabiangreffrath/crispy-doom/wiki/Building-on-Windows). A much more detailed guide is provided [in the Chocolate Doom Wiki](https://www.chocolate-doom.org/wiki/index.php/Building_Chocolate_Doom_on_Windows), but applies to Crispy Doom as well for most parts.

Compilation on Debian systems (Debian 10 "buster" or later) should be as simple as

```
 sudo apt install build-essential automake git
 sudo apt build-dep crispy-doom
```

to install the prerequisites and then

```
 cd crispy-doom
 autoreconf -fiv
 ./configure
 make
```

After successful compilation the resulting binaries can be found in the `src/` directory.

## News

### Crispy Doom 5.9.2

Crispy Doom 5.9.2 is currently under development...

**New Features and Improvements**

 * Support for the "new" Nerve PWAD has been improved: If the Nerve PWAD is explicitly loaded and contains a TITLEPIC lump, use it - else use the INTERPIC lump. Consequently, if the Nerve PWAD gets auto-loaded and contains TITLEPIC and an INTERPIC lumps, rename them (thanks @buvk).
 * Graphic patch lumps in widescreen format are now properly centered - but still squashed to Vanilla aspect ratio (thanks @buvk).
 * A `-pistolstart` command line option has been added (by @mikeday0, thanks @Asais10).

**Bug Fixes**

 * The Sigil PWAD is now only pre-loaded if the gameversion is The Ultimate Doom. This fixes a glitched texture file when Chex Quest is loaded as the IWAD (by @kitchen-ace, thanks Mr.Unsmiley)
 * Check if the map name graphics lumps are actually from the Masterlevels PWAD before renaming them. This fixes an issue with unofficial Masterlevels compilations which do not contain these lumps (thanks @Dark-Jaguar).
 * A string buffer size calculation bug has been fixed in the `-levelstat` implementation (thanks Eric Claus).

**Crispy Heretic**

 * A `-wandstart` command line option has been added (by @mikeday0, thanks @Asais10).

### Crispy Doom 5.9.1

Crispy Doom 5.9.1 is released on September 04, 2020 to fix some minor bugs.

**Bug Fixes**

 * Building without Python has been fixed again (inherited from Chocolate Doom, by @vilhelmgray, thanks Michael Bäuerle).
 * An old bug has been fixed which was caused by SDL2_Mixer opening a different number of audio channels than requested (inherited from Chocolate Doom, thanks Edward850).
 * Auto-loading of the Sigil PWAD has been fixed on file systems with case-sensitive file names (thanks @kitchen-ace and @kbterkelsen).

**Crispy Heretic**

 * Final intermissions screens are now shown after each episode (by @kraflab).

Crispy Doom 5.9.1 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`f7007449`](https://github.com/chocolate-doom/chocolate-doom/commit/f700744969ac867649aa581ae19447a4c172179e).

### Crispy Doom 5.9.0

Crispy Doom 5.9.0 is released on August 21, 2020 and introduces auto-loading of both official Doom 2 expansions for any supported Doom 2 IWAD, additional DEHACKED fields to de-hardcode some Vanilla behavior and many other improvements and bug fixes.

**New Features and Improvements**

 * NERVE.wad is now automatically loaded even with the Vanilla DOOM2.wad IWAD (thanks @hkight).
 * MASTERLEVELS.wad is now automatically loaded alongside DOOM2.wad just like NERVE.wad was for the BFG Edition DOOM2.wad IWAD before. However, this will now work for *any* DOOM2.wad IWAD as well (thanks @buvk for reporting a bug in the initial implementation).
 * Support has been added for automatically loading all the 20 separate Master Levels PWADs and arranging them as if they came from the single MASTERLEVELS.wad PWAD.
 * Alternative text entries have been added to the skill menu. Thus, the entries are still there and rendered consistently even if the Nightmare! skill graphics lump is missing when playing with a v1.2 or earlier IWAD (thanks @zx64).
 * The unconditional "fixing" of reversed stereo channels in OPL music playback has been reverted. People got so much used to the flipped channels that the correct channel order has been regarded as a bug (thanks @drivetheory).
 * Two separate finale texts for the Master Levels have been introduced, depending on whether you have found and finished the secret MAP21 or not. The actual texts have been taken from @MadDeCoDeR's Classic-RBDOOM-3-BFG project with some minor wording fixes applied (thanks @icecreamoverlord).
 * Optional Automap lines smoothing has been added with a toggle in the Crispness menu (by @zx64).
 * Doors locked with a key now flash on the Automap, if extended map colours are enabled (by @kitchen-ace).
 * Extended map colours have been adjusted to be closer to Vanilla's (by @kitchen-ace).
 * A "use" button timer has been added and can be enabled in the "Demos" Crispness menu section, apparently useful for practicing speed-running (thanks Looper).
 * A check for segs referencing non-existent linedefs has been ported over from PrBoom+ (thanks @tpoppins).
 * Demo joins are now tracked (by @kraflab).
 * Support for the "Dropped Item" DEHACKED field has been added, which allows to specify the Thing Id to be spawned after the Thing dies. It's a generalization of the same behavior that is hardcoded for MT_WOLFSS, MT_POSSESSED, MT_SHOTGUY and MT_CHAINGUY in Vanilla Doom (by @Ferk).
 * More generalizations de-hardcoding some Vanilla Doom behavior have been added (by @Ferk).
   * The following DEHACKED fields for Things have been added:
     * Melee threshold: Distance to switch from missile to melee attack.
     * Max target range: Maximum distance to start shooting (0 for unlimited).
     * Min missile chance: Minimum chance for firing a missile.
     * Missile chance multiplier: This de-hardcodes the double missile chance that vanilla Cyberdemon, Spider Mastermind, Revent and Lost Souls have. The value is FRACUNIT based (65536 = normal firing chance), also note that the lower the value, the higher the chance.
 * A `-levelstat` option has been added (by @kraflab).

**Bug Fixes**

 * The sky in NRFTL Maps 04-08 is now fixed if loaded from command line (thanks @Zodomaniac).
 * HUD texts are now printed up to the right screen edge again (thanks Grizzly).
 * The DSSECRET fallback sound is now checked for availability. This fixes a crash when playing with a v1.2 or earlier IWAD (thanks @zx64).
 * The HUD widget coordinate re-calculation has been moved from thinker to drawer functions. This should fix the racing condition that caused the wide status bar widget alignment being drawn to the automap status bar with the brick border (thanks @kitchen-ace).
 * The IDCLEV cheat has been fixed for the Master Levels (thanks @buvk).
 * Thing coordinates in the rotated automap have been fixed when the "extended automap colors" feature is disabled (thanks @icecreamoverlord).
 * A segmentation fault has been fixed when changing episodes during the intermission screen (thanks @icecreamoverlord).
 * The "go to next level" cheat key for the Master Levels has been fixed.
 * Endianess issues when loading extended nodes have been fixed (thanks Michael Bäuerle).
 * The IDDT cheat is now reset when restarting a map during demo recording, but not each time the Automap is opened (thanks galileo31dos01).
 * Missing server-side num_players validation (CVE-2020-14983) and missing client-side ticdup validation have been fixed (found by Michał Dardas from LogicalTrust, fixes inherited from Chocolate Doom).
 * Automap panning by keyboard and mouse is now accumulated (thanks @kitchen-ace).
 * Invalid texture/flat animation sequences are now skipped instead of erroring out (thanks @kitchen-ace).
 * The Automap shaking for one frame after changing the view angle has been fixed (thanks @JNechaevsky).
 * The top-right HU widgets have been moved one char to the left to allow for display of multi-thousand FPS (thanks @JNechaevsky).

**Crispy Heretic**

 * The alternative WASD movement scheme key bindings have been added (thanks @zx64).
 * The Heretic Crispness menu has been colorized.
 * Morph Ovum is now allowed to be bound to a key (by @kitchen-ace).
 * The chat sound is now played when a secret is found. It's a message after all.
 * Seconds are now always displayed in intermission time, bringing time display in line with Doom (by @kitchen-ace).
 * Level time is now always shown in extended maps, i.e. episodes 4 and above (by @kitchen-ace).
 * The demoextend and shortticfix features are now enabled by default (by @kitchen-ace, also applied to the Hexen sources).
 * Interpolation of the puff object is now suppressed for the first tic, so the snapping of the puff to the floor level isn't interpolated (thanks Wagi).
 * Centered messages are now cleared on intermission and finale screens (by @JNechaevsky).
 * Centered messages are now cleared from border and bezel (by @JNechaevsky).
 * Top border refresh has been fixed for centered messages (by @JNechaevsky).
 * Support dedicated music tracks for each map has been added (by @kitchen-ace, thanks Dwars).
 * The ENGAGExy cheat (and some others) are now allowed in Nightmare.
   * Cheats enabled in Black Plague Possesses Thee, i.e. Heretic's Nightmare (by @kitchen-ace):
     * ENGAGExy: warp to level
     * NOISE: sound debug info
   * Cheats enabled in BPPT as well as netgames:
     * TICKER: show ticks-per-frame counter
     * SHOWFPS: show FPS counter (Crispy specific cheat)
 * Key bindings to restart the level/demo and go to next level have been added (by @kitchen-ace).
 * A `-levelstat` option has been added (by @kraflab).
 * Total level time is now tracked for levelstat and is also added to the intermission screen  (by @kraflab).

**Known Issues**

 * When the 20 separate Master Levels PWADs are automatically loaded, their individual sky textures have to get removed, else they would override the regular sky textures for Doom 2 and NRFTL (thanks @tpoppins for noticing). If you insist to play the Master Levels each with their designated sky, load the individual PWADs on the command line.

Crispy Doom 5.9.0 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`6ba89d0c`](https://github.com/chocolate-doom/chocolate-doom/commit/6ba89d0c05e4b93bdb64460b64a9ba3bdcc4bf6e).

## Documentation

 * **[Changelog](https://github.com/fabiangreffrath/crispy-doom/wiki/Changelog)**
 * **[Compatibility](https://github.com/fabiangreffrath/crispy-doom/wiki/Compatibility)**
 * **[Crispness](https://github.com/fabiangreffrath/crispy-doom/wiki/Crispness)**
 * **[FAQ](https://github.com/fabiangreffrath/crispy-doom/wiki/FAQ)**

## Versioning

Crispy Doom's major version number is increased whenever a new Chocolate Doom (pre-)release got merged into its code base. The minor version number is increased for intermediate releases that do only contain Crispy-specific changes or unreleased changes to the Chocolate Doom code base. The micro or patch version is reserved for post-release hotfixes, it remained unused until the 5.5 release.

## Contact

The canonical homepage for Crispy Doom is https://github.com/fabiangreffrath/crispy-doom

Crispy Doom is maintained by [Fabian Greffrath](mailto:fabian@greffXremovethisXrath.com). 

Please report any bugs, glitches or crashes that you encounter to the GitHub [Issue Tracker](https://github.com/fabiangreffrath/crispy-doom/issues).

## Acknowledgement

Although I have played the thought of hacking on Chocolate Doom's renderer for quite some time already, it was Brad Harding's [Doom Retro](https://www.chocolate-doom.org/wiki/index.php/Doom_Retro) that provided the incentive to finally do it. However, his fork aims at a different direction and I did not take a single line of code from it. Lee Killough's [MBF](https://doomwiki.org/wiki/WinMBF) was studied and used to debug the code, especially in the form of Team Eternity's [WinMBF](https://doomwiki.org/wiki/WinMBF) source port, which made it easier to compile and run on my machine. And of course there is fraggle's [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom) with its exceptionally clean and legible source code. Please let me take this opportunity to appreciate all these authors for their work!

Also, thanks to plums of the [Doomworld forums](https://www.doomworld.com/vb/) for beta testing, "release manager" Zodomaniac and "art director" JNechaevsky for the continuous flow of support and inspiration during the post-3.x development cycle and (last but not the least) [Cacodemon9000](http://www.moddb.com/members/cacodemon9000) for his [Infested Outpost](http://www.moddb.com/games/doom-ii/addons/infested-outpost) map that helped to track down quite a few bugs!

Furthermore, thanks to VGA for his aid with adding support for his two mods: [PerK & NightFright's Black Ops smooth weapons add-on converted to DEHACKED](https://www.doomworld.com/forum/topic/84859-black-ops-smooth-weapons-dehacked-mod) and [Gifty's Smooth Doom smooth monster animations converted to DEHACKED](https://www.doomworld.com/forum/topic/85991-smoothed-smooth-monsters-for-doom-retro-and-crispy-doom) that can make the gameplay even more pleasing to the eyes.

## Legalese

Doom is © 1993-1996 Id Software, Inc.; 
Boom 2.02 is © 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman;
PrBoom+ is © 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman,
© 1999-2000 Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze,
© 2005-2006 Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko;
Chocolate Doom is © 1993-1996 Id Software, Inc., © 2005 Simon Howard; 
Chocolate Hexen is © 1993-1996 Id Software, Inc., © 1993-2008 Raven Software, © 2008 Simon Howard;
Strawberry Doom is © 1993-1996 Id Software, Inc., © 2005 Simon Howard, © 2008-2010 GhostlyDeath; 
Crispy Doom is additionally © 2014-2019 Fabian Greffrath;
all of the above are released under the [GPL-2+](https://www.gnu.org/licenses/gpl-2.0.html).

SDL 2.0, SDL_mixer 2.0 and SDL_net 2.0 are © 1997-2016 Sam Lantinga and are released under the [zlib license](http://www.gzip.org/zlib/zlib_license.html).

Secret Rabbit Code (libsamplerate) is © 2002-2011 Erik de Castro Lopo and is released under the [GPL-2+](http://www.gnu.org/licenses/gpl-2.0.html).
Libpng is © 1998-2014 Glenn Randers-Pehrson, © 1996-1997 Andreas Dilger, © 1995-1996 Guy Eric Schalnat, Group 42, Inc. and is released under the [libpng license](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt).
Zlib is © 1995-2013 Jean-loup Gailly and Mark Adler and is released under the [zlib license](http://www.zlib.net/zlib_license.html).

The Crispy Doom icon (as shown at the top of this page) is composed of the [Chocolate Doom icon](https://www.chocolate-doom.org/wiki/images/7/77/Chocolate-logo.png) and a [photo](https://en.wikipedia.org/wiki/File:Potato-Chips.jpg) of potato crisps (Utz-brand, grandma's kettle-cooked style) by [Evan-Amos](https://commons.wikimedia.org/wiki/User:Evan-Amos) who kindly released it into the [public domain](https://en.wikipedia.org/wiki/Public_domain). The current high-resolution version of this icon has been contributed by JNechaevsky (formerly by Zodomaniac).
