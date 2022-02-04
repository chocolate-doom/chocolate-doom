# Crispy Doom
[![Crispy Doom Icon](https://www.chocolate-doom.org/wiki/images/b/be/Crispy-doom.png)](https://github.com/fabiangreffrath/crispy-doom)

[![Top Language](https://img.shields.io/github/languages/top/fabiangreffrath/crispy-doom.svg)](https://github.com/fabiangreffrath/crispy-doom)
[![Code Size](https://img.shields.io/github/languages/code-size/fabiangreffrath/crispy-doom.svg)](https://github.com/fabiangreffrath/crispy-doom)
[![License](https://img.shields.io/github/license/fabiangreffrath/crispy-doom.svg?logo=gnu)](https://github.com/fabiangreffrath/crispy-doom/blob/master/COPYING.md)
[![Release](https://img.shields.io/github/release/fabiangreffrath/crispy-doom.svg)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Release Date](https://img.shields.io/github/release-date/fabiangreffrath/crispy-doom.svg)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Downloads](https://img.shields.io/github/downloads/fabiangreffrath/crispy-doom/latest/total.svg)](https://github.com/fabiangreffrath/crispy-doom/releases)
[![Commits](https://img.shields.io/github/commits-since/fabiangreffrath/crispy-doom/latest.svg)](https://github.com/fabiangreffrath/crispy-doom/commits/master)
[![Last Commit](https://img.shields.io/github/last-commit/fabiangreffrath/crispy-doom.svg)](https://github.com/fabiangreffrath/crispy-doom/commits/master)
[![Build Status](https://github.com/fabiangreffrath/crispy-doom/actions/workflows/main.yml/badge.svg)](https://github.com/fabiangreffrath/crispy-doom/actions/workflows/main.yml)

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
 * `-pistolstart` reset health, armor and inventory at start of each level in Doom (new in 5.9.2)
 * `-wandstart` reset health, armor and inventory at start of each level in Heretic (new in 5.9.2)
 * `-doubleammo` doubles ammo pickup rate in Doom and Strife (new in 5.11).
 * `-moreammo` increases ammo pickup rate by 50% in Heretic (new in 5.11).
 * `-moremana` increases mana pickup rate by 50% in Hexen (new in 5.11).
 * `-fast` enables fast monsters in Heretic and Hexen (new in 5.11).
 * `-autohealth` enables automatic use of Quartz flasks and Mystic urns in Heretic and Hexen (new in 5.11).
 * `-keysloc` enables display of keys on the automap in Heretic (new in 5.11).

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
https://github.com/fabiangreffrath/crispy-doom/releases/download/crispy-doom-5.9.2/crispy-doom-5.9.2-win32.zip

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

### Crispy Doom 5.10.3

Crispy Doom 5.10.3 is released on Aug 17, 2021. It is a bug-fix release fixing a regression in savegame restoring introduced by the A11Y features from the 5.10.2 release.

**New Features and Improvements**

 * The translucency map is now always recalculated and no more loaded from a file or lump.
 * Autoload directories are now also supported for "sideloaded" PWADs - i.e. nerve.wad, masterlevels.wad and sigil.wad (thanks @Raddatoons).

**Bug Fixes**

 * Rendered sector lightlevels are now saved in savegames if they are different from the logical lightlevels, fixing a regression intruduced with A11Y support (thanks Alaux).
 * The green color translation range has been fine-tuned so that light-blue isn't preferred over green anymore (thanks maxmanium).
 * The CMake build system has been updated for A11Y (thanks @vanfanel).

Crispy Doom 5.10.3 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`3524a67d`](https://github.com/chocolate-doom/chocolate-doom/commit/3524a67dd3c7d48a172c83d3ae170a7176fb3cfa).

### Crispy Doom 5.10.2

Crispy Doom 5.10.2 is released on Aug 10, 2021. It is a minor release introducing basic accessibility features and accumulated fixes since the previous release.

**New Features and Improvements**

 * Smooth automap rotation and srolling have been implemented (by @JNechaevsky).
 * Autoload directories for PWADs are now supported (thanks @OpenRift412 for the suggestion and @rfomin for the prior implementation in Woof!).
 * Basic accessibility features have been added and can be toggled in the Accessibility menu of the setup tool:
   * Flickering Sector Lighting (disables sectors changing their light levels)
   * Weapon Flash Lighting (disables weapon flashes changing the ambient light levels)
   * Weapon Flash Sprite (disables rendering of weapon flashes sprites)
   * Palette Changes (disables palette changes upon damage, item pickup, or when wearing the radiation suit)
   * Invulnerability Colormap (disables colormap changes during invulnerability)

**Bug Fixes**

 * In NRFTL the TITLEPIC is only replaced with the INTERPIC if the former is from the IWAD (thanks @OpenRift412).
 * Overlaid automap remainings are now cleared from the demo loop (by @JNechaevsky).
 * Adjusting of the BLOCKMAP boundaries to match the Vanilla algorithm has been reverted. Although this was done in the Vanilla algorithm, it doesn't match what's done in the algorithms used by MBF and Boom - and thus PrBoom+ which uses the latter. This fixes sync for one demo reported by galileo31dos01 on 5L1C.wad MAP01.
 * The Compatibility menu has been removed from the setup tool, it is obsolete for all games now.
 * The translucency table is now always calculated at gamma level 0, fixing potentially incorrect entries (by @JNechaevsky).
 * The episode menu is now rendered with the HUD font if the graphics are both from an IWAD and if the patch width for "Hell on Earth" is longer than "No Rest for the Living" (thanks thanks @hackneyed-one).
 * Automap rotation variables are now properly initialized, preventing line shaking (by @JNechaevsky).

**Crispy Heretic**

 * The `-demoext` parameter (even though enabled by default) only applies to demos loaded on the command line.

**Crispy Hexen**

 * Hexen: Restore pointers to `mobj_t` with garbage identity as `NULL` pointers (by @Dasperal).

Crispy Doom 5.10.2 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`3524a67d`](https://github.com/chocolate-doom/chocolate-doom/commit/3524a67dd3c7d48a172c83d3ae170a7176fb3cfa).

### Crispy Doom 5.10.1

Crispy Doom 5.10.1 is released on Mar 24, 2021. It is a minor release containing the accumulated fixes of the past weeks.

**New Features and Improvements**

 * Some colored text has been reverted back to the pristine Vanilla experience. As a rule of thumb, UI that has been inherited from Vanilla does doesn't get any colorization anymore (thanks @OpenRift412).
 * Screenwidth values are now rounded *down* to the nearest multiple of 4 in hires mode, and *up* in lores mode. This makes sure we end up with a screenwidth of 852 px for a 16:9 ratio in hires mode, which is exact twice the width of the widescreen assets, and with a screenwidth of 428 px in lores mode, which is the next integer multiple of 4 (thanks @buvk).
 * A CMake toggle has been added for the truecolor mode (by @zx64).

**Bug Fixes**

 * The weapon sprite coordinates now remain unchanged if neither variable bobbing nor weapon sprite centering is enabled. Coincidently, this will bring back the sloppy bobbing of the chainsaw weapon sprite during its idle frames.
 * Interpolation of the Archvile's fire sprite is now suppressed to mitigate it being spawned at the wrong location.
 * Status bar positioning, drawing of fullscreen patches and the bunny scroll screen have been fixed on big-endian systems.
 * The window height is now prevented from shrinking when changing widescreen modes.
 * The smooth automap lines features has been fixed for truecolor mode (by @zx64).

**Crispy Heretic**

 * Weapon pickup messages are now shown in cooperative multiplayer mode (by @xttl).
 * All Crispy Doom specific cheats have been ported over and adapted accordingly (by @xttl).
 * An Automap overlay mode has been added.
 * There are now separate mouse sensitivities for turn, strafe and y-axis.
 * Heretic now has a "demowarp" feature, i.e. support for using both `-playdemo` and `-warp` on the command line (thanks @thom-wye).

Crispy Doom 5.10.1 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`5003ab52`](https://github.com/chocolate-doom/chocolate-doom/commit/5003ab5283ff27c951c97b064c26cdde2bb0f427).

### Crispy Doom 5.10.0

Crispy Doom 5.10.0 is released on Jan 12, 2021. Its major new feature is the addition of support for the widescreen assets found e.g. in the Unity version of Doom.

**New Features and Improvements**

 * Proper support for widescreen assets has been added (by @SmileTheory, thanks @ghost and @chungy).
 * The bezel bottom edge to the left and right of the status bar in widescreen mode is now preserved (thanks braders1986 and @SmileTheory).
 * Special treatment is now applied to the No Rest for the Living and The Masterlevels expansions - even during network games, demo recording and playback. This includes level transitions to the secret maps and back, finale text screens, par times, etc. (thanks Coincident and Keyboard_Doomer for input from the DSDA community).
 * Menu scrolling with the mouse wheel has been improved to be more responsive (by @JNechaevsky).
 * All textures are now always composed, whether they are multi-patched or not. Furthermore, two separate composites are created, one for opaque and one for translucent mid-textures on 2S walls. Additionally, textures may now be arbitrarily tall.
 * Freedoom Phase 2 and FreeDM are now explicitly named in the Doom 2 Episode menu.
 * The status bar is now redrawn in the Main, Episode and Skill menus, where it could get overridden by custom graphics (thanks @JNechaevsky).

**Bug Fixes**

 * A crash has been fixed when the -record and -pistolstart parameters were used simultaneously (thanks Spie812).
 * An optimization inherited from MBF has been fixed which led to sprites not being rendered on the lowest possible floor (thanks @retro65).
 * Only non-sky flats are now checked for the swirling effect.
 * Crushed non-bleeding monsters are not removed off the map anymore, their sprites are replaced with the invisible SPR_TNT1 instead (thanks ZeroMaster010 and sorry for the desyncing demo).
 * Sigil is not auto-loaded anymore with the Freedoom Phase 1 IWAD, since Sigil's own texture definitions may clash with the ones from Freedoom (thanks @Some1NamedNate).
 * A brightmap definition for an animated flat sequence in HacX has been fixed.
 * Some fixes to the "--enable-truecolor" configure option have been implemented (i.e. the --disable-truecolor option, the rendering of the status bar bezel, fuzzy column drawing and the translucency factor - thanks xttl).
 * Window height adjustment when changing window size has been brought back at the cost of the window shrinking when repeatedly changing the widescreen option.
 * Parts of the status bar being visible during the initial wipe in widescreen mode has been fixed (thanks xttl).

**Crispy Heretic**

 * The level restart key now restarts the current demo recording from the map it was started, but under a new name (thanks @thom-wye).
 * Demo file names may now have arbitrary length (inherited from Chocolate Doom, also applied to the Hexen sources).
 * The demo file size limit has been removed (also applied to the Hexen and Strife sources).
 * The top border not always being drawn correctly in hires mode for all reduced screen sizes has been fixed (thanks @xttl).

**Known Issues**

 * Users who insist on the pure Vanilla experience that was formerly applied to the No Rest for the Living and The Masterlevels expansions or who need it to properly play back demos recorded with a previous release will have to rename their PWAD files and explicitly load them on the command line.

Crispy Doom 5.10.0 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`b26157ac`](https://github.com/chocolate-doom/chocolate-doom/commit/b26157aca5b12049b35d8dfcf969c51967a369f6).

## More documentation

 * **[Changelog](https://github.com/fabiangreffrath/crispy-doom/wiki/Changelog)**
 * **[Compatibility](https://github.com/fabiangreffrath/crispy-doom/wiki/Compatibility)**
 * **[Crispness](https://github.com/fabiangreffrath/crispy-doom/wiki/Crispness)**
 * **[FAQ](https://github.com/fabiangreffrath/crispy-doom/wiki/FAQ)**

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
