# Crispy Doom
![](https://www.chocolate-doom.org/wiki/images/b/be/Crispy-doom.png)

![](https://img.shields.io/github/languages/top/fabiangreffrath/crispy-doom.svg?style=flat)
![](https://img.shields.io/github/languages/code-size/fabiangreffrath/crispy-doom.svg?style=flat)
![](https://img.shields.io/github/license/fabiangreffrath/crispy-doom.svg?style=flat)
![](https://img.shields.io/github/release/fabiangreffrath/crispy-doom.svg?style=flat)
![](https://img.shields.io/github/release-date/fabiangreffrath/crispy-doom.svg?style=flat)
![](https://img.shields.io/github/downloads/fabiangreffrath/crispy-doom/latest/total.svg?style=flat)
![](https://img.shields.io/github/commits-since/fabiangreffrath/crispy-doom/latest.svg?style=flat)
![](https://img.shields.io/github/last-commit/fabiangreffrath/crispy-doom.svg?style=flat)
![](https://img.shields.io/travis/com/fabiangreffrath/crispy-doom.svg?style=flat)

Crispy Doom is a limit-removing enhanced-resolution Doom source port based on [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom).

Its name means that its internal 640x400 resolution looks "crisp" and is also a [slight reference](http://www.mathsisfun.com/recipie.html) to its origin.

## Synopsis

Crispy Doom is a friendly fork of [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom) that provides a higher display resolution, removes the [static limits](https://doomwiki.org/wiki/Static_limits) of the Doom engine and offers further optional visual, tactical and physical enhancements while remaining entirely config file, savegame, netplay and demo compatible with the original.

## Objectives and features

Crispy Doom is a source port that aims to provide a faithful Doom gaming experience while also featuring some user-requested improvements and enhancements. It is forked off of Chocolate Doom to take advantage of its free and open-source code base, portability, accuracy and compatibility with Vanilla Doom.

Its core features are:

 * Enhanced 640x400 display resolution, with the original 320x200 resolution still available in the "High Resolution Rendering: Off" mode.
 * Uncapped rendering framerate with interpolation and optional vertical synchronization (VSync) with the screen refresh rate.
 * Intermediate gamma correction levels (0.5, 1.5, 2.5 and 3.5).
 * Removal of all static engine limits, or at least raising of the less crucial ones.
 * Full support for the "Doom Classic" WADs shipped with the "Doom 3: BFG Edition", especially the "No Rest For The Living" episode shipped in the NERVE.WAD file.

Furthermore, the following optional user-visible and audible features are available:

 * Jumping.
 * Free vertical looking, including mouse look and vertical aiming.
 * Aiming support by a crosshair that may get directly rendered into the game world.
 * A new minimal Crispy HUD, displaying only the status bar numbers.
 * Clean Screenshot feature, enabling to take screenshots without burning the status bar and HUD messages into them.
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

All of these features are disabled by default and need to get enabled either in the in-game "Crispness" menu, in the crispy-doom-setup tool or as command line parameters. They are implemented in a way that preserves demo-compatibility with Vanilla Doom and network game compatibility with Chocolate Doom. Furthermore, Crispy Doom's savegames and config files are compatible, though not identical (see the Compatibility section below), to Vanilla Doom's.

Crispy Doom strives for maximum compatibility with all "limit-removing Vanilla" maps -- but not Boom or ZDoom maps. More specifically, Crispy Doom supports some select advanced features such as [ANIMATED](https://doomwiki.org/wiki/ANIMATED) and [SWITCHES](https://doomwiki.org/wiki/SWITCHES) lumps, MBF sky transfers, SMMU swirling flats and [MUSINFO](https://doomwiki.org/wiki/MUSINFO) -- but neither generalized linedef and sector types nor DECORATE and MAPINFO.

Many additional less user-visible features have been implemented, e.g. fixed engine limitations and crashes, fixed rendering bugs, fixed harmless game logic bugs, full support for DEHACKED files and lumps in BEX format, additional and improved cheat codes, an improved Automap, and many more! Due to the extra DEHACKED states added from [MBF](https://doomwiki.org/wiki/MBF), Crispy Doom supports [enhancer](https://www.doomworld.com/forum/topic/84859-black-ops-smooth-weapons-dehacked-mod) [mods](https://www.doomworld.com/forum/topic/85991-smoothed-smooth-monsters-for-doom-retro-and-crispy-doom) that can make the gameplay even more pleasing to the eyes. For a detailed list of features and changes please refer to the release notes below.

### New controls (with default bindings)

 * Move Forward (alt.) "W"
 * Move Backward (alt.) "S"
 * Strafe Left (alt.) "A"
 * Strafe Right (alt.) "D"
 * Jump (bindable to mouse buttons as well) "/" (like in Chocolate Hexen and Strife)
 * Quick Reverse (bindable to keys or mouse buttons as well)
 * Mouse Look (bindable to keys or mouse buttons or permanent)
 * Look up "PgDn" (bindable to joystick axes)
 * Look down "DELETE" (bindable to joystick axes)
 * Center view "END"
 * Toggle always run "CAPSLOCK"
 * Toggle vertical mouse movement (new in 5.4)
 * Delete savegame "DELETE"
 * Go to next level
 * Reload current level
 * Save a clean screenshot
 * (with automap open) Toggle overlay mode "O"
 * (with automap open) Toggle rotate mode "R"

### New command line parameters

 * `-dm3` specifies the Deathmatch 3.0 rules for the netgame (weapons stay, items respawn).
 * `-episode 1` launches Hell on Earth and `-episode 2` launches No Rest for the Living episode if the Doom 2 IWAD shipped with the Doom 3: BFG Edition is used.
 * `-warp 1a` warps to the secret level E1M10: Sewers of XBox Doom IWAD.
 * `-mergedump <file>` merges the PWAD file(s) given on the command line with the IWAD file and writes the resulting data into the `<file>` given as argument. Might be considered as a replacement for the `DEUSF.EXE` tool.
 * `-blockmap` forces a (re-)building of the BLOCKMAP lumps for loaded maps.
 * `-loadgame N -record demoname` and `-loadgame N -playdemo demoname` allow to record and play demos starting from a savegame, not from the level start.
 * `-fliplevels` loads mirrored versions of the maps (this was the default on April 1st up to version 5.0).
 * `-flipweapons` flips the player's weapons (new in 5.3).

### New cheat codes

 * `TNTWEAP` followed by a weapon number gives or removes this weapon (8 = Chainsaw, 9 = SSG). Try to load Doom 1 with `DOOM2.WAD` as a PWAD and type `TNTWEAP9` to play the SSG in Doom 1.
 * `TNTEM`, `KILLEM` or `FHHALL` kill all monsters on the current map (and disables all cube spitters).
 * `SPECHITS` triggers all [Linedef actions](https://doomwiki.org/wiki/Linedef_type) on a map at once, no matter if they are enabled by pushing, walking over or shooting or whether they require a key or not. It also triggers all boss monster and Commander Keen actions if possible.
 * `NOTARGET` or `FHSHH` toggle deaf and blind monsters that do not act until attacked.
 * `TNTHOM` toggles the flashing [HOM](https://doomwiki.org/wiki/Hall_of_mirrors_effect) indicator (disabled by default).
 * `SHOWFPS` or `IDRATE` toggle printing the FPS in the upper right corner.
 * `NOMOMENTUM` toggles a debug aid for pixel-perfect positioning on a map (not recommended to use in-game).
 * `GOOBERS` triggers an easter egg, i.e. an "homage to an old friend". ;-)
 * `IDBEHOLD0` disables all currently active power-ups.
 * `IDCLEV00` restarts the current level.
 * `IDMUS00` restarts the current music (new in 5.1).
 * `VERSION` shows the engine version, build date and SDL version (new in 5.1).

## Download

Binaries for Windows XP / Vista / 7 / 8.1 / 10 (both x86 and x64 editions) are available here: 
https://github.com/fabiangreffrath/crispy-doom/releases/download/crispy-doom-5.4/crispy-doom_5.4.zip

Daily builds of Crispy Doom can be found here:
http://latest.chocolate-doom.org/

Crispy Doom can play nearly all variants of Doom. If you don't own any, you may download the [Shareware version of Doom](http://cdn.debian.net/debian/pool/non-free/d/doom-wad-shareware/doom-wad-shareware_1.9.fixed.orig.tar.gz), extract it and copy the DOOM1.WAD file into your Crispy Doom directory. Alternatively, you may want to play Crispy Doom with [Freedoom](https://www.chocolate-doom.org/wiki/index.php/Freedoom) and a MegaWAD.

### Sources

The Crispy Doom source code is available at GitHub: https://github.com/fabiangreffrath/crispy-doom.
It can be [downloaded in either ZIP or TAR.GZ format](https://github.com/fabiangreffrath/crispy-doom/releases) 
or cloned via

```
 git clone https://github.com/fabiangreffrath/crispy-doom.git
```

Compilation on Debian/Ubuntu systems should be as simple as

```
 sudo apt-get install build-essential automake
 sudo apt-get build-dep chocolate-doom
```

to install the prerequisites and then

```
 cd crispy-doom
 autoreconf -vif
 ./configure
 make
```

After successful compilation the resulting binaries can be found in the `src/` directory.

## News

Crispy Doom 5.4 has been released on December 17, 2018. This version demonstrates that there's always room for perfection and improvement ;)

**Features**

 * In-game aspect ratio correction switching with options to force the 4:3 and 16:10 (native internal resolution) aspect ratios has been implemented per Zodomaniac's request.

**Improvements**

 * Loading a savegame while recording a demo is allowed again, as requested by Looper and ZeroMaster010.
 * The uncapped framerate feature is now independent of the network sync implementation, thanks to Wintermute0110 for the discussion. Formerly it was only implemented for the '-oldsync' implementation, but now that Choco has made 'newsync' the new default, it has been made available there as well.
 * In the Crispy HUD, missing keys will blink for a second after an unsuccessful attempt to use a linedef that requires them.
 * Some framebuffer overflow prevention measures have been added back that somehow got lost during the conversion to the resolution-agnostic patch drawing implementation. This fixes a crash when showing the TITLEPIC of MALGNANT.WAD.
 * In the Doom 2 cast sequence, seestate and deathstate sounds are now randomized (if misc. sound fixes are enabled) according to JNechaevsky's idea. Also, death sequences in the cast are now randomly flipped, if the corresponding feature is enabled. Furthermore, the attack sounds are now played based on state action functions (instead of mere state numbers) as Zodomaniac suggested, so that monsters from SMOOTHED.wad now play their attack sounds properly in the cast sequence. Finally, Doomguy now properly migrates from his aiming state to the firing state and even plays the SSG sound when firing in the cast sequence.
 * A key binding to toggle vertical mouse movement (novert) as suggested by Looper has been added.
 * Level times in the intermission screen are now displayed at most in hhhh:mm:ss format, eliminating the ambiguity for multi-day plays that JNechaevsky pointed out.
 * Only weapons available in the respective IWAD version (shareware/registered) are given when using cheat codes, as JNechaevsky suggested.
 * Loading a savegame from a WAD file different from the currently loaded one does not interrupt the current game anymore. Also, a check is performed if the requested map is actually available at all (e.g. MAP33 from BFG Edition IWAD when playing with the standard 32-map IWAD).
 * Using the `IDCLEV` cheat to non-existent levels doesn't exit the game anymore, according to mfrancis95's suggestion.
 * Sector interpolation during the 0th gametic is inhibited due to the request by JNechaevsky and Brad Harding, eliminating some visual glitches when loading a savegame from the command line.
 * Brightmaps for the SW2SATYR, SW2LION and SW2GARG textures have been contributed by JNechaevsky.
 * Composite textures are now pre-cached in `R_PrecacheLevel()`. This should prevent the last remaining rendering hiccups in uncapped framerate mode.
 * Weapon pickup messages are now even shown in multiplayer games, thanks to Zodomaniac for filing the bug nearly two years ago and to mfrancis95 for an implementation idea.

**Bug Fixes**

 * Screenshots without the `screen shot` message have been fixed, as Zodomaniac kept an eye on it.
 * Variable array lengths induced by making SCREENWIDTH non-const are now fixed thanks to zx64's pointer.
 * SSG availability is now reflected by the Shotgun (3) slot of the arms widget the way JNechaevsky and Brad Harding proposed.
 * Sound clipping in Doom 2 MAP08 and The Ultimate Doom E4M8 has been fixed as JNechaevsky suggested.
 * A crash in shareware/registered mode triggered by using `IDMUS` as spotted by JNechaevsky has been eliminated.
 * The minigun zombie's firing frames are now rendered full-bright.
 * Patchless columns are now treated the same as multi-patched ones. Thanks to RaphaelMode for providing a level that exposed a crash when a patchless column came into view, which is now fixed.
 * Updating the player's viewz on sector movement has been fixed again. Thanks to Dwaze for pointing out this this was *still* not properly working yet!
 * With HUD digits colorization enabled, digits in the armor widget are now blue if armor class >= 2, after Zodomaniac reported an ambiguity which becomes apparent in Strain.
 * Palette resetting by key pickup reported by mfrancis95 has been fixed.
 * The SDL audio backend is forcefully set to directsound on Windows, away from the buggy WASAPI default as Brad Harding requested.

Crispy Doom 5.4 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`482d302e`](https://github.com/chocolate-doom/chocolate-doom/commit/482d302ee846fb30d23d50ccff8549d300a81b75)

## Documentation

 * **[Changelog](https://github.com/fabiangreffrath/crispy-doom/wiki/Changelog)**
 * **[Compatibility](https://github.com/fabiangreffrath/crispy-doom/wiki/Compatibility)**
 * **[FAQ](https://github.com/fabiangreffrath/crispy-doom/wiki/FAQ)**

## Versioning

Crispy Doom maintains a major and a minor version number. The major version number is increased whenever a new official version of Chocolate Doom is released and the changes merged into Crispy Doom. The minor version number is increased whenever changes have been applied that are not part of an official Chocolate Doom release or do only affect Crispy Doom.

## Contact

The canonical homepage for Crispy Doom is https://github.com/fabiangreffrath/crispy-doom

Crispy Doom is maintained by [Fabian Greffrath](mailto:fabian@greffXremovethisXrath.com). 

Please report any bugs, glitches or crashes that you encounter to the GitHub [Issue Tracker](https://github.com/fabiangreffrath/crispy-doom/issues).

## Acknowledgement

Although I have played the thought of hacking on Chocolate Doom's renderer for quite some time already, it was Brad Harding's [Doom Retro](https://www.chocolate-doom.org/wiki/index.php/Doom_Retro) that provided the incentive to finally do it. However, his fork aims at a different direction and I did not take a single line of code from it. Lee Killough's [MBF](https://doomwiki.org/wiki/WinMBF) was studied and used to debug the code, especially in the form of Team Eternity's [WinMBF](https://doomwiki.org/wiki/WinMBF) source port, which made it easier to compile and run on my machine. And of course there is fraggle's [Chocolate Doom](https://www.chocolate-doom.org/wiki/index.php/Chocolate_Doom) with its exceptionally clean and legible source code. Please let me take this opportunity to appreciate all these authors for their work!

Also, thanks plums of the [Doomworld forums](https://www.doomworld.com/vb/) for beta testing, "release manager" Zodomaniac and "art director" JNechaevsky for the continuous flow of support and inspiration during the 3.x-5.x development cycle and (last but not the least) [Cacodemon9000](http://www.moddb.com/members/cacodemon9000) for his [Infested Outpost](http://www.moddb.com/games/doom-ii/addons/infested-outpost) map that helped to track down quite a few bugs!

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
Crispy Doom is additionally © 2014-2018 Fabian Greffrath;
all of the above are released under the [GPL-2+](https://www.gnu.org/licenses/gpl-2.0.html).

SDL 2.0, SDL_mixer 2.0 and SDL_net 2.0 are © 1997-2016 Sam Lantinga and are released under the [zlib license](http://www.gzip.org/zlib/zlib_license.html).

Secret Rabbit Code (libsamplerate) is © 2002-2011 Erik de Castro Lopo and is released under the [GPL-2+](http://www.gnu.org/licenses/gpl-2.0.html).
Libpng is © 1998-2014 Glenn Randers-Pehrson, © 1996-1997 Andreas Dilger, © 1995-1996 Guy Eric Schalnat, Group 42, Inc. and is released under the [libpng license](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt).
Zlib is © 1995-2013 Jean-loup Gailly and Mark Adler and is released under the [zlib license](http://www.zlib.net/zlib_license.html).

The Crispy Doom icon (as shown at the top of this page) is composed of the [Chocolate Doom icon](https://www.chocolate-doom.org/wiki/images/7/77/Chocolate-logo.png) and a [photo](https://en.wikipedia.org/wiki/File:Potato-Chips.jpg) of potato crisps (Utz-brand, grandma's kettle-cooked style) by [Evan-Amos](https://commons.wikimedia.org/wiki/User:Evan-Amos) who kindly released it into the [public domain](https://en.wikipedia.org/wiki/Public_domain). The current high-resolution version of this icon has been contributed by JNechaevsky (formerly by Zodomaniac).
