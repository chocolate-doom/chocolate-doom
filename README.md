# Crispy Doom
![](https://www.chocolate-doom.org/wiki/images/b/be/Crispy-doom.png)

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

Crispy Doom strives for maximum compatibility with all "limit-removing Vanilla" maps -- but not Boom or ZDoom maps. Many additional less user-visible features have been implemented, e.g. fixed engine limitations and crashes, fixed rendering bugs, fixed harmless game logic bugs, full support for DEHACKED files and lumps in BEX format, additional and improved cheat codes, an improved Automap, and many more! Due to the extra DEHACKED states added from [MBF](https://doomwiki.org/wiki/MBF), Crispy Doom supports [enhancer](https://www.doomworld.com/forum/topic/84859-black-ops-smooth-weapons-dehacked-mod) [mods](https://www.doomworld.com/forum/topic/85991-smoothed-smooth-monsters-for-doom-retro-and-crispy-doom) that can make the gameplay even more pleasing to the eyes. For a detailed list of features and changes please refer to the release notes below.

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

## Compatibility

 * Savegames saved by Crispy Doom are still **compatible with Vanilla Doom**, but are **not identical** anymore to the files that Vanilla Doom would have saved. This is caused by several reasons:
   * By default, Crispy Doom writes savegames in an **extended format** that stores additional game state information past the end-of-file marker of the original savegame format. Vanilla and Chocolate Doom are still able to read savegames in this format but are not able to write it. Writing savegames in the extended format can be disabled by setting the `crispy_extsaveg` key to `0` in `crispy-doom.cfg`.
   * In order to distinguish NRFTL levels from regular Doom 2 levels, Crispy Doom uses the `gameepisode` variable, which is used in Doom 1 to distinguish between the game episodes but is hitherto unused in Doom 2.
   * Crispy Doom preserves the `mobj->target` and `mobj->tracers` fields of map objects when saving a game by replacing their contents with specific indices. These indices are then converted back to the corresponding pointers when the game is restored again. In Vanilla and Chocolate Doom, however, the contents of the `mobj->target` and `mobj->tracers` fields are disregarded and overwritten with `NULL`.
   * Furthermore, Crispy Doom saves thinkers for active platforms in stasis.

 * If you are going to share savegames between Crispy Doom and Chocolate Doom, make sure to load all PWADs with the `-merge` parameter in the latter.

 * The Crispy HUD is displayed when `blocksize == 12`, which isn't supported by Chocolate Doom. To retain config file compatibility, quit the game with any other view size.

 * The "flipped levels" and "SSG available in Doom 1" features introduced in Crispy Doom 1.3 are considered strictly **experimental**! They may produce savegames, demo files or netgames that are not compatible with Chocolate Doom, Vanilla Doom or previous versions of Crispy Doom at all. Furthermore, the `SPECHITS` cheat introduced in Crispy Doom 1.5 may leave a map in a completely inconsistent state and games saved after using it may even cause Vanilla to crash by exceeding static limits.

## Download

Binaries for Windows XP / Vista / 7 / 8.1 / 10 (both x86 and x64 editions) are available here: 
http://www.greffrath.com/~fabian/crispy-doom_5.4.zip

A supplementary Music Pack that contains the fluidsynth library and a freely-available soundfont is available here:
http://www.greffrath.com/~fabian/crispy-doom-music-pack_5.1.zip

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

## History of changes

### Changes of Crispy Doom 5.4 from Crispy Doom 5.3

Crispy Doom 5.4 has been released on December XX, 2018. This version demonstrates that there's always room for perfection and improvement ;)

**Features**

 * In-game aspect ratio correction switching with options to force the 4:3 and 16:10 (native internal resolution) aspect ratios has been implemented per Zodomaniac's request.

**Improvements**

 * Loading a savegame while recording a demo is allowed again, as requested by Looper and ZeroMaster010.
 * The uncapped framerate feature is now independent of the network sync implementation, thanks to Wintermute0110 for the discussion. Formerly it was only implemented for the '-oldsync' implementation, but now that Choco has made 'newsync' the new default, it has been made available there as well.
 * In the Crispy HUD, missing keys will blink for a second after an unsuccessful attempt to use a linedef that requires them.
 * Some framebuffer overflow prevention measures have been added back that somehow got lost during the conversion to the resolution-agnostic patch drawing implementation. This fixes a crash when showing the TITLEPIC of MALGNANT.WAD.
 * The the Doom 2 cast sequence, seestate and deathstate sounds are now randomized (if misc. sound fixes are enabled) according to JNechaevsky's idea. Also, death sequences in the cast are now randomly flipped, if the corresponding feature is enabled. Furthermore, the attack sounds are now played based on state action functions (instead of mere state numbers) as Zodomaniac suggested, so that monsters from SMOOTHED.wad now play their attack sounds properly in the cast sequence. Finally, Doomguy now properly migrates from his aiming state to the firing state and even plays the SSG sound when firing in the cast sequence.
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

### Changes of Crispy Doom 5.3 from Crispy Doom 5.2

Crispy Doom 5.3 has been released on August 12, 2018. This version brings several fixes to deeper-lurking bugs and some improvements.

**Features**

 * A new `-flipweapons` command line argument has been introduced to flip the player's weapon sprites, suggested by Zodomaniac.

**Improvements**

 * A warning is now emitted to the terminal window when an Arch-Vile resurrects a monster that was crushed to a pool of gore before and thus gets turned into a "ghost monster".
 * FPS is capped to 35 if level time is not ticking (i.e. in non-level game states) which was requested and tested by JNechaevsky.
 * Changing the "High Resolution Rendering" setting now takes immediate effect and doesn't require an engine restart.
 * "Uncapped framerate" and "VSync" configure options are now separate.
 * `R_InstallSpriteLump()` has been made less error-prone on SiFi270's request.

**Bug Fixes**

 * Screen refresh hiccups at uncapped framerates have now been fixed, reported by vanfanel.
 * Player's `viewz` only updates if he is actually inside the moving sector instead of merely within its bounding box. Thanks to galileo31dos01 for reporting this issue and JNechaevsky for confirming it!
 * The `deathmatch` variable is now reset when loading a savegame from the menu, fixing the bug that carburetor and Dwaze noticed (i.e. when loading a savegame while a deathmatch demo was playing in the demo loop, the engine assumed the loaded game was a deathmatch as well and applied deathmatch rules).
 * An engine crash that occurred when encountering empty texture lumps (i.e. lumps which do not contain a single texture definition at all) in PWADs has been fixed, thanks to the report by galileo31dos01.

Crispy Doom 5.3 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`dd78c087`](https://github.com/chocolate-doom/chocolate-doom/commit/dd78c0873ec4756cc1bc430cf22ebe542ac3a23b)

### Changes of Crispy Doom 5.2 from Crispy Doom 5.1

Crispy Doom 5.2 has been released on June 20, 2018. This release introduces many new features and sections in the Crispness menu, including a new "Rendering" section, a "Crosshair" section for crosshair options including health-based coloring and highlighting on target, and a "Demos" section with options for demo recording and playback timers as well as the demo progress bar. Furthermore, there is smooth diminishing lighting, additional gamma correction levels, increased number of sound channels and support for taller textures as well as numerous other improvements and bug fixes.

**Features**

 * Smooth interpolated texture scrolling has been introduced for uncapped rendering and linedef specific 85 (Scroll texture right) is now supported, as suggested by JNechaevsky.
 * The crosshair may now indicate the player's health (as requested by JNechaevsky) and change its color ("highlight") when pointing on a target. Furthermore, crosshair options have been moved into their own submenu in the Crispness menu.
 * Intermediate gamma correction levels (`0.5`, `1.5`, `2.5` and `3.5`) have been implemented, on JNechaevsky's request.
 * Smooth diminishing lighting with 32 light levels instead of 16 may now be enabled in the "Visual" section of the Crispness menu.
 * The number of sound channels may now be 16 or 32 as an alternative to the default 8, adjustable in the "Audible" section of the Crispness menu.
 * The "Rendering" Crispness menu section has been introduced, comprising the high-resolution rendering, uncapped framerate and smooth pixel scaling options.
 * Support for DeePsea tall patches has been added to the main patch drawing functions. Furthermore, tall textures and sprites composed of DeePsea tall patches are now supported, as suggested by JNechaevsky.
 * Resolution-agnostic and fullscreen-stretching patch drawing functions have been implemented, mostly taken from Doom Retro. 
 * The "Demos" section of the Crispness menu has been introduced, including options for enabling the recording timer, choosing forward or backward counting playback timer and the demo progress bar (which is now 2 px high).
 * Support for MUSINFO lumps (dynamic music changing) has been added, mostly taken from PrBoom+.

**Improvements**

 * The Fuzz effect used when drawing "invisible" things is now scrolled independent of the rendering framerate and paused when the game is paused, on JNechaevsy's suggestion.
 * The "Uncapped" framerate setting resulting in 100% CPU usage has been removed from the Crispness menu. It still remains available as a benchmarking measure by manually setting `crispy_uncapped` to `1` in the `crispy-doom.cfg` config file.
 * CPU usage is now capped by limiting the rendering framerate to 35 if the window is minimized, thanks to JNechaevsky for the idea.
 * A game version mismatch notification is now thrown when attempting to load a savegame that has been saved with a different IWAD/game version. The engine just did nothing before which was discovered by Zodomaniac.
 * The "Miscellaneous Sound Fixes" option now includes playing a sound if the menu is activated with a different key than ESC (e.g. on game launch) as Zodomaniac suggested as well as playing the Arch-Vile's fire start sound (DSFLAMST), the absence of which was noticed by JNechaevsky.
 * The `red2green` color translation table has been toned down a bit removing the "metallic green gloss" from Barons' and Knights' blood splats while leaving the overall appearance (and that of the colored gore pools) intact after JNechaevsky's thorough investigation.
 * For disabled multi-items in the Crispness menu, "None" is now explicitly printed instead of leaving a blank space after Zodomaniac spotted some inconsistency.
 * The "Permanent Mouse Look" option has been moved from the "Mouse" menu to the "Tactical" page of the Crispness menu, as Zodomaniac and some Doomworld members suggested.
 * The "Negative Player Health" feature has been simplified to a choice of on/off for all gamemodes.
 * The "Extended Savegames" feature is now enabled by default and has been removed from the Crispness menu. Reasons are lack of space in the menu and redundancy as extended savegames are still compatible with Vanilla and Chocolate Doom.
 * The "Colored Blood and Corpses" and "Fix Spectre and Lost Soul blood" features have been combined into one menu item (i.e. colored blood now includes Spectre and Lost Soul blood fixes) and simplified to an on/off choice.
 * It is now possible to switch Crispness menu pages using PgUp/PgDn and to wrap around between the first and last menu page, according to JNechaevsky's and mfrancis95's suggestions.
 * A new and exclusive Crisps background texture for the Crispness menu has been created by JNechaevsky.
 * The "Mono SFX" option is now a Crispness menu item available in the "Audible" section. The corresponding `-monosfx` command line option has been removed.
 * Minor wording fixes have been applied to some Crispness menu items, now reading "Squat weapon down on impact" and "Crosshair shape".
 * Fonts in the finale text screens are now rendered with shadows.
 * The Player's view height is now immediately updated when his sector moves.
 * Warnings about mapthings without any skill tag set are now issued.
 * The "Always Run" and "Use native keyboard mapping" are now disabled by default, like in Chocolate Doom. This is in order not to confuse players installing Crispy for the first time. Zodomaniac requested this upon discovering the impossibility to type in savegame names in Cyrillic keyboard layout when using native keyboard mapping, which was also confirmed by JNechaevsky under different OSes. 
 * Palette changes now don't occur when the help screen or the Crispness menu is active, thanks to mfrancis95 for the contribution.

**Bug Fixes**

 * Fuzz effect going out of bounds noticed by JNechaevsky has been fixed.
 * A crash that occured when loading a map without any things has been fixed, thanks to mfrancis95 for noticing.
 * The status bar is now immediately getting redrawn after the help screens have been shown, spotted by JNechaevsky.
 * The blood fix for Lost Souls not getting toggled in-game immediately like the colored blood for other monsters has been fixed, pointed out by Zodomaniac. 
 * Demos are now played back with interpolation in case of uncapped rendering even if the menu is open, on mfrancis95's report.
 * Romero's head is not randomly flipped after death anymore, thanks to JNechaevsky for inspiring the flipped corpse feature overhaul.
 * Color translation dependence on gamma level spotted by JNechaevsky has been fixed.
 * Weapon sprite moving when looking up/down in the low resolution rendering mode has been fixed.
 * Planes rotating in the low resolution rendering mode have been fixed.
 * The game remaining paused after saving while recording a demo has been fixed on Zodomaniac's report.
 * The bug when music was starting to play while paused and changed volume spotted by JNechaevsky has been fixed.
 * The in-engine "fixes" replacing WolfSS by Zombiemen for the BFG edition IWAD and removing the TNT MAP31 yellow key erroneous "multiplayer only" flag have been removed because they make demo playback fail as Zodomaniac's investigation has shown. Instead, the PWAD fixes (https://forum.zdoom.org/viewtopic.php?f=19&t=53776 and http://www.teamtnt.com/other/tnt31fix.zip respectively) are recommended.
 * Crashes when completing IWAD bonus maps (E1M10: Sewers, MAP33: Betray) while recording a demo have been fixed on Zodomaniac's report.
 * The possibility of falling down into a wall after a savegame is loaded has been fixed. 
 * Zero music volume not muting OPL (and not only OPL) music fully has been fixed. Thanks to JNechaevsky, Zodomaniac and mfrancis95 for bringing this up again and nukeykt for clarifying OPL chip limitations and suggesting to pause music when it is muted.
 * The MAXBUTTONS limit has been removed (mostly because of the SPECHITS cheat and because of the more-then-one-switch-texture-per-line fix mentioned below).
 * The glitch of switch textures not changing in some segments has been fixed by means of registering up to three buttons at once for lines with more than one switch texture. Thanks to JNechaevsky for spotting this bug in the Vanilla code and Brad Harding and Jeff Doggett for presenting alternative approaches.
 * Savegames are now prevented from restoring out-of-range flats. This could happen if a PWAD was loaded in Choco with the -file parameter, a savegame had been saved and then restored in Crispy (which loads PWADs with the -merge parameter).
 * The brightmap for the COMP2 texture has been fixed in Brad Harding's way.
 * Taking a "Clean screenshot" could break the menu if the regular screenshot key was unbound. This has been fixed due to JNechaevsky's efforts to track this down.
 * Taking a screenshot without the 'screen shot' message has been repaired.
 * A crash when finishing NERVE MAP09 while recording/playing back a demo in case NERVE.WAD is auto-loaded with BFG Edition DOOM 2 IWAD has been fixed on Zodomaniac's report.
 * A crash when finishing NERVE MAP06 while recording a demo has been fixed, thanks to galileo31dos01 for pointing this out.
 * The status bar is now backed up with the arms widget background to fix the automap causing pixel replacements, reported by JNechaevsky.
 * Some consistency fixes have been applied to Chex Quest: the Episode menu is not shown anymore when returning from the Skill menu, the "Colored blood and corpses" and "Randomly mirrored corpses" Crispness menu entries are explicitly disabled, on request by JNechaevsky.
 * Resurrections of mirrored corpses staying mirrored have been fixed by mfrancis95.
 * Bullet puffs now don't slip to the edge of a sector anymore when passing through the plane. Bullets hitting the floors and ceilings in direct aiming mode now have their puffs displayed at the actual hit spots, thanks to JNechaevsky for noticing.
 * Window going out of vertical bounds at startup has been fixed, thanks to JNechaevsky's report.

**Known Issues**

 * Changing the "High Resolution Rendering" option requires a restart of the engine for the change to take effect. This is currently the only option that requires this. Switching this feature with immediate effect is considered a release goal for the next version of Crispy Doom.
 * If "Vertical Aiming" is set to "direct" and the "Highlight Crosshair on Target" feature is enabled, the crosshair will get highlighted even if the direct shot would miss but if it would hit with autoaiming enabled.
 * Demo timer shadows the FPS counter if both are enabled.

Crispy Doom 5.2 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`5329fb5d`](https://github.com/chocolate-doom/chocolate-doom/commit/5329fb5d75971138b20abf940ed63635bd2861e0)

**[Previous changes](https://www.chocolate-doom.org/wiki/index.php/Crispy_Doom/Changelog)**

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
