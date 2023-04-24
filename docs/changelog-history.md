### Crispy Doom 5.12

**New Features and Improvements**
* Add mouse smoothing when running uncapped. (by @mikeday0)
* New icon designs for all games and the setup executable (by @kiwphi).
* Brightmaps now working on mid-textures of two-sided lines (by @JNechaevsky).
* New stats widget option: above status bar.
* Many automap improvements: (by @JNechaevsky and @mikeday0)
    - New mouse button bindings for zoom, max zoom and follow.
    - Pan/zoom faster by holding run button.
    - Use overflow-safe map coordinate system from prboom-plus.
    - Apply frame interpolation to automap rendering when running uncapped.

**Bug Fixes**
* Ensure mouse sensitivity values are always initialized to defaults.
* Fix clipping issue when jumping in Doom.
* Fix cutscene rendering in truecolor in Doom.
* Fix potential Heretic crash on x64 systems.
* Armor icons now drawn in correct locations in widescreen for Hexen.
* Fix screen border glitch after leaving automap in low res mode in Hexen.
* Fix shuffling of items on automap after loading savegame.

**Crispy Doom**
* Support up to 8 pages of savegames.
* Brightmap now applied to SW2HOT texture (by @JNechaevsky).
* Improved brightmaps for SILVER3, COMPUTE2 and COMPUTE3 textures (by @JNechaevsky).
* Player arrows in multiplayer games now also interpolated and rotating with correct angles (by @JNechaevsky).
* Fix incorrect demo timer value and demo bar lenght in multiplayer demos (by @fabiangreffrath, @JNechaevsky and @rfomin).
* Status bar showing values and face of choosen player in multiplayer spy mode (by @JNechaevsky).

**Crispy Heretic**
* Remove ambient sound limit.
* Add automap overlay and rotation (by @mikeday0).
* Add PWAD autoload directories (by @mikeday0).
* Add brightmap support (by @JNechaevsky).
* Add mouselook support (by @mikeday0).
* Add automap mousewheel zoom and mouse panning (by @mikeday0).
* Add automap grid (by @JNechaevsky).

**Crispy Hexen**
* Fix tutti frutti and add DeePsea tall patch support (by @mikeday0).
* Add uncapped framerate option (by @mikeday0).
* Add automap overlay and rotation (by @mikeday0).
* Add PWAD autoload directories (by @mikeday0).
* Add mouselook support ((by @mikeday0).
* Add brightmap support (by @JNechaevsky).
* Add key toggles for autorun and vertical mouse movement (thanks @rxhxiii).
* Add automap mousewheel zoom and mouse panning (by @mikeday0).
* Add automap grid (by @JNechaevsky).

### Crispy Doom 5.11.1

Crispy Doom 5.11.1 is released on Feb 10, 2022. It it a bug-fix release to fix Heretic and Hexen config keys getting lost in setup.

### Crispy Doom 5.11

Crispy Doom 5.11 is released on Feb 09, 2022. It marks the return of Crispy Hexen and features highly appreciated community contributions of the past half year.

**New Features and Improvements**

 * REKKR has been added to the list of recognized IWADs (thanks @Some1NamedNate).
 * DEHACKED lumps from IWADs are now always loaded.
 * The Automap is now kept static if not following player in overlay mode (thanks @JNechaevsky).
 * Patches in PNG format are now detected and trigger an error message.
 * Custom translucency maps in TRANMAP lumps are now allowed again (by @NeuralStunner).
 * A customizable Crispness menu background may now be provided in a CRISPYBG lump (by @NeuralStunner).
 * The beta scepter and bible are now valid gettable things (by @NeuralStunner).
 * Support for some optional extra sounds has been added (by @NeuralStunner).
 * The `-nosideload` parameter has been added to prevent automatic loading of NRFTL, Masterlevels and Sigil.
 * Pre-v1.25 always active plats are now properly emulated (by @SmileTheory).
 * Some snow has been added as an easter, ne christmas, egg (by @hovertank3d).
 * A key binding to toggle demo fast-forward has been added (by @tpoppins).
 * A demo pause feature has been added (by @tpoppins).
 * Different formats ("ratio", "remaining", "percent" and "boolean") are now available for the level stats (thanks @dftf-stu).
 * A11Y: set amount of extra light to add to the game scene (thanks @dftf-stu and @JNechaevsky).
 * Quicksave/quickload questions are now skipped.
 * Beta BFG support has been improved (by @NeuralStunner).
 * Command line options to provide for custom difficulty parameters have been added (by @FozzeY):
   * `-doubleammo` doubles ammo pickup rate in Doom and Strife.
   * `-moreammo` increases ammo pickup rate by 50% in Heretic.
   * `-moremana` increases mana pickup rate by 50% in Hexen.
   * `-fast` enables fast monsters in Heretic and Hexen.
   * `-autohealth` enables automatic use of Quartz flasks and Mystic urns in Heretic and Hexen.
   * `-keysloc` enables display of keys on the automap in Heretic.

**Bug Fixes**

 * Boss endings are not triggered for auto-loaded Sigil E5 anymore.
 * The NRFTL and Masterlevels PWADs are not automatically sideloaded anymore if another PWAD already provides MAP01.
 * Woof's window size adjustment logic has been adapted.
 * Never sideload any PWAD if a single demo is played back.

**Possible Regressions**

 * Extended savegames are now mandatory.
 * All "blood fixes" have been removed from the "colored blood" feature. That is, spectres don't bleed spectre blood anymore and Lost Souls don't bleed puffs anymore, but bloodless objects still don't bleed or leave gibs when crushed.
 * The "Squat weapon down on impact" feature has been entirely removed.
 * The "Weapon Recoil Thrust" feature has been entirely removed.

**Crispy Heretic**

 * Extended demos are enabled for all demos again (thanks @thom-wye).
 * Support for widescreen rendering has been added (by @mikeday0).
 * Enemies remember their targets across savegames (thanks @SiFi270).
 * Check if map name starts with a map identifier before skipping it on the intermission screen.
 * Generate a default save slot name when the user saves to an empty slot or one that already begins with a map identifier.
 * Support up to 8 savegames.
 * Allow to delete a savegame from the menu.

**Crispy Hexen**

 * Seg texture clipping has been fixed (from dsda-doom, thanks @kraflab).
 * Support for widescreen rendering has been added (by @mikeday0).
 * A basic Crispness menu has been added (by @mikeday0).
 * Crispy Hexen is now built and installed by default again!

Crispy Doom 5.11 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [41865b17](https://github.com/chocolate-doom/chocolate-doom/commit/41865b179684eaf812fc9682936d9b79320f5a1d).

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

### Crispy Doom 5.9.2

Crispy Doom 5.9.2 is released on September 22, 2020 to fix some more bugs and add some minor improvements.

**New Features and Improvements**

 * Support for the "new" Nerve PWAD has been improved: If the Nerve PWAD is explicitly loaded and contains a TITLEPIC lump, use it - else use the INTERPIC lump. Consequently, if the Nerve PWAD gets auto-loaded and contains TITLEPIC and an INTERPIC lumps, rename them (thanks @buvk).
 * Graphic patch lumps in widescreen format are now properly centered - but still squashed to Vanilla aspect ratio (thanks @buvk).
 * A `-pistolstart` command line option has been added (by @mikeday0, thanks @Asais10).
 * The tally screen is now shown after finishing Chex Quest E1M5 (thanks kokrean).
 * Apparently random crashes have been fixed that occured when the (truncated) file name of a loaded demo happens to match the name of an already available lump. Now this lump name collision is detected and the offending demo lump renamed to DEMO1, which is most certainly always the name of a demo lump (thanks galileo31dos01 and plums).

**Bug Fixes**

 * The Sigil PWAD is now only pre-loaded if the gameversion is The Ultimate Doom. This fixes a glitched texture file when Chex Quest is loaded as the IWAD (by @kitchen-ace, thanks Mr.Unsmiley)
 * Check if the map name graphics lumps are actually from the Masterlevels PWAD before renaming them. This fixes an issue with unofficial Masterlevels compilations which do not contain these lumps (thanks @Dark-Jaguar).
 * A string buffer size calculation bug has been fixed in the `-levelstat` implementation (thanks Eric Claus).

**Crispy Heretic**

 * A `-wandstart` command line option has been added (by @mikeday0, thanks @Asais10).

Crispy Doom 5.9.2 is based on Chocolate Doom 3.0.1 and has merged all changes to the Chocolate Doom master branch up to commit [`f7007449`](https://github.com/chocolate-doom/chocolate-doom/commit/f700744969ac867649aa581ae19447a4c172179e).

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

### Crispy Doom 5.8.0

Crispy Doom 5.8.0 has been released on April 17, 2020 to bring the status bar to widescreen rendering mode, remove SPECHITS limit and fix some bugs as well as implement the core feature set in Crispy Heretic.

**Features**

 * Status bar and reduced screen sizes are now available in widescreen mode, requested by sovietmewtwo and many Doomworld members and debugged with the help from cnrm and Zodomaniac.
 * SPECHITS limit, the last persisting static limit, has been removed.

**Improvements**

 * Compilation with Code::Blocks and TDM-GCC 5.1 (missing includes) has been fixed by drfrag666.
 * Wide and compact HUD are now handled as separate screen sizes, and you can switch through them as normal.
 * Once the last screen size has been exceeded you loop over to the empty HUD.

**Bug Fixes**

 * SKY3 texture is now used for MAP04-MAP08 in NRftL, thanks to JNechaevsky for contributing the fix.
 * When calculating weapon bobbing, the check is now performed for attack key/button being held down (thanks to unRyker for helping to choose the criterion) instead of checking for `A_WeaponReady()` because weapon states could have been modified by DeHackEd. This fixes jerky weapon bobbing reported by kitchen-ace for some weapons in mods like Vanilla Smooth Weapons and DOOM 4 Vanilla.
 * Fix for segmentation fault when running on rgb565 screen has been pulled from Chocolate Doom, contributed by Wells Lu.

**Crispy Heretic changes**

 * JNechaevsky fixed the issue that new messages would not appear if a level was finished while an "ultimate message" was shown.
 * Support for mouse sensitivity up to 255 and, while at it, displaying numeric values next to the menu sliders has been contributed by Zodomaniac.
 * Cheat showing FPS has been added to Crispy Heretic by Jeff Green.
 * High resolution rendering toggle has been introduced in Crispy Heretic by Jeff Green.
 * Uncapped framerate has been implemented in Crispy Heretic by Jeff Green.
 * An implicit declaration warning has been fixed by drfrag666.
 * Crispy settings have been prevented from resetting in setup by Ryan Krafnick.
 * Secret message has been implemented by Jeff Green.
 * Always Run toggle key and Always Run + Run = Walk behavior has been introduced by Ryan Krafnick.
 * Mouse inventory buttons have been added by Ryan Krafnick to Chocolate Heretic and then merged from there.
 * The INTERCEPTS and SPECHITS limits have been removed entirely.
 * Vertical mouse movement (novert) toggle has been added by Ryan Krafnick.

Crispy Doom 5.8.0 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`daaaefa7`](https://github.com/chocolate-doom/chocolate-doom/commit/daaaefa7e75d056ab4c1b3b2b68fa29885e750fe).

### Crispy Doom 5.7.2

Crispy Doom 5.7.2 has been released on March 13, 2020 to introduce the Compact HUD for widescreen rendering mode and bring some general improvements and fixes.

**Features**

 * A new "Compact HUD" mode has been introduced for widescreen rendering mode which leaves the status bar widgets at the same position as in regular rendering mode instead of moving them to the edges of the screen. The latter is now available in the "Wide HUD" mode.

**Improvements**

 * Compatibility of extended savegames has been improved across forks which change the project name away from "crispy-doom". Zodomaniac noticed this issue with his So Doom fork.
 * No 'Oof' sounds are played anymore upon attempt to manipulate an inactive Crispness menu item, on Zodomaniac's remark.

**Bug Fixes**

 * HUD widgets are now re-initialized immediately after graphics initialization to mitigate the alignment issues that JNechaevsky and Zodomaniac experienced on launching a map from command line.
 * Previous colorization of "percent" status bar widgets is now remembered. This applies to the health and armor widget and forces them to update not only if their value changes, but also if their colorization changes, e.g. when entering the IDDQD cheat. Thanks to unerxai for the report.
 * Horizontal location of the "PAUSE" graphic in widescreen mode has been fixed, thanks to JNechaevsky for spotting this.
 * The widescreen rendering setting is not changed anymore by disabling aspect ratio correction, requested by Zodomaniac.
 * An undetected merge conflict has been fixed in `hexen/r_things.c`, thanks to NickZ for the bug report.

Crispy Doom 5.7.2 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f1fa7faf`](https://github.com/chocolate-doom/chocolate-doom/commit/f1fa7faf566e3e325c1b1fbf8ace8d5ac2db2b1e).

### Crispy Doom 5.7.1

Crispy Doom 5.7.1 has been released on March 03, 2020 to fix some bugs related to widescreen rendering.

**Improvements**

* In order to improve the discoverability of the "A Secret is Revealed!" feature, its corresponding Crispness menu item has been renamed to "Report Revealed Secrets", based on a discussion with oprypin.
* Widescreen rendering is now disabled in case of disabled aspect ratio correction, as Zodomaniac proved it made no sense.

**Bug Fixes**

* A crash when switching to high resolution or widescreen rendering during a screen wipe has been fixed, reported and investigated by Zodomaniac and JNechaevsky.
* The obtrusive MAP/WAD automap text widget is not drawn anymore in widescreen mode, thanks to plumsinus.
* Horizontal coordinates of automap markers in widescreen mode have been fixed, thanks to unerxai from Doomworld forums.

Crispy Doom 5.7.1 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f1fa7faf`](https://github.com/chocolate-doom/chocolate-doom/commit/f1fa7faf566e3e325c1b1fbf8ace8d5ac2db2b1e).

### Crispy Doom 5.7 

Crispy Doom 5.7 has been released on February 21, 2020. This release brings proper widescreen rendering as well as other improvements requested by the speed-running community.

**Features**

* Widescreen rendering has been implemented, thanks to JNechaevsky and ice44 for spotting the bugs that had been surfacing in the progress.
* Loading 16 bit RIFF wavs in .wads has been enabled, contributed by SmileTheory.
* `-lumpdump` command line parameter has been added that dumps raw content of a lump into a file.

**Improvements**

* `IDDT` cheat is now reset when re-starting map during demo recording, thanks to Looper for the suggestion.
* Time in demo timer widget is now printed in centiseconds instead of tics. Thanks to Looper and ZeroMaster010 for the proposal.

**Bug Fixes**

* Fix for GUS emulation in presence of midiproc.exe has been pulled from Chocolate Doom, whereto it was contributed by JNechaevsky.

Crispy Doom 5.7 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f1fa7faf`](https://github.com/chocolate-doom/chocolate-doom/commit/f1fa7faf566e3e325c1b1fbf8ace8d5ac2db2b1e).

### Crispy Doom 5.6.4 

Crispy Doom 5.6.4 has been released on January 14, 2020. This release addresses the community feedback received after 5.6.3.

**Features**

 * Toggling full screen using Enter on num. keyboard is now possible (inherited from Chocolate Doom).
 * `IDCLEV` is now allowed during demo playback and warps to the requested map (starting a new demo).
 * If a new game is started (current level is reloaded/next level is loaded on pressing the respective key) during demo recording, a new demo is started. Thanks to Looper from Doomworld forums for the feature request.
 * 'Intermediate Crispy HUD' without the status bar but with the face and its background in place has been introduced.

**Improvements**

 * `leveltime` is now shown in the demo timer widget during recording instead of the accumulated demo time, thanks to Looper from Doomworld forums for the input.
 * Windows binaries being 32-bit has been clarified, thanks to RetroDoomKid for the remark.
 * Demo timer widget is now reset when restarting a demo during recording.
 * `gamemap` is now set to `startmap` when restarting a demo during recording.
 * Blood splats and projectile puffs are now drawn as small squares in the Automap.
 * Level/demo restart key description has been adjusted to become self-explanatory.
 * Static demo file name suffix counter has been added. This avoids checks for an increasing number of file names being available by just remembering the latest given suffix number. Thanks to Looper for the suggestion.
 * Demos are now restarted from the map they were started, thanks to Looper for the input.
 * OSX: Freedoom single player IWADs have been added to launcher (inherited from Chocolate Doom).
 * The smooth chainsaw idle animation has been brought back.
 * Weapon bobbing has been reworked and made adjustable.
 * Disallowing the vertical mouse movement now disables controlling the menus with the mouse, thanks to bryc for requesting this.
 * TNTWEAP0 now removes the berserk strength, all weapons and ammo except for pistol and 50 bullets.
 * Early exit from the tally screen after ExM8 is now forced, which enables demos to progress to the end of game sequence.
 * TNTWEAP2 now removes the pistol.
 * Some clipping optimizations taken from JNechaevsky's Russian Doom (and there from MBF respectively) have been implemented.
 * Savegame name is automatically overridden on saving if it already starts with a map identifier, proposed by zebzorb.
 * Status bar optimizations, including numbers to be only redrawn if necessary, on JNechaevsky's suggestion.
 * In automap overlay mode the automap is now drawn on top of everything as JNechaevsky suggested, not beneath the bezel for decreased screen sizes.

**Bug Fixes**

 * Missing prototype for `calloc()` in `r_data.c` causing memory corruption on 64bit in Windows/MSVC builds has been fixed, thanks to zx64 for spotting this. 
 * Crash when the flag for the berserk pack sprite patch memory zone is changed has been fixed, thanks to IsBebs for the bug report and Zodomaniac, JNechaevsky and turol for helping with the analysis.
 * Zombie player crash on SELFDEAD has been fixed, thanks to tpoppins for the report and turol for the analysis.
 * `"doomstat.h"` is now included instead of `<doomstat.h>` in `doom/r_swirl.c`, fixing compilation with MSVC2017, and packed attribute for structs when compiling with MSVC has been fixed. Thanks to drfrag666 for reporting and confirming the fix.
 * Shadowed menu and text drawing has been removed, as it is bugged in wipe screens.
 * The par time for MAP33 is now determined correctly (inherited from Chocolate Doom).
 * Subsequent calls to `A_BrainAwake()` to reset the `braintargeton` variable are now allowed. This fixes demo sync for maps with more than one brain, e.g. PL2.
 * Player viewheight in NOMOMENTUM mode has been fixed on Zodomaniac's report.
 * The revenant sync bug (with homing or non-homing missiles) with New Game demos has hopefully been fixed.
 * No statdump output is generated now for ExM8, and updating the Archvile fire's `floorz` and `ceilingz` values has been reverted, which fixes demo desyncs that fraggle discovered. Thanks a lot!
 * Fuzz effect animation remaining static in one case has been fixed, this happened if the number of pixels to apply the fuzz effect to was an integer multiple of FUZZTABLE. Thanks to JNechaevsky for the suggestion!
 * Status bar face expression staying across level changes has been removed, thanks to JNechaevsky for pointing this out.
 * Automap panning in flippedlevels mode has been fixed, thanks to JNechaevsky for reporting.
 * Self-repeating states in `P_LatestSafeState()` are now handled.
 * Max-sized background buffer is now allocated for the bezel. This fixes a crash when the game is started with `crispy->hires == 0` and `scaledviewwidth != SCREENWIDTH` and then `crispy->hires` is switched to `1`.
 * Switching to the fist after typing a cheat expecting two parameters has been fixed. This affects IDMUSx1 and IDCLEVx1, thanks to maxmanium for pointing this out.
 * Automap marker coordinate for flipped levels has been fixed.
 * Loss of grid lines near the automap boundary has been fixed, spotted by JNechaevsky.
 * Overlayed automap blinking one tic on screen borders has been fixed, noticed by JNechaevsky.

Crispy Doom 5.6.4 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`f81b5c7b`](https://github.com/chocolate-doom/chocolate-doom/commit/f81b5c7b3e7c24364fce681ad3d7ba18119a867b).

### Crispy Doom 5.6.3 

Crispy Doom 5.6.3 has been released on October 04, 2019. This release addresses the community feedback received after 5.6.2 release and brings support for the updated Episode 5: Sigil v1.2/v1.21.

**Features**

 * Automap overlay and rotate modes are now stored as config variables, suggested by JNechaevsky.
 * Versions 1.2 and 1.21 of Episode 5: Sigil are now supported.

**Improvements**

 * Par times provided by Sigil 1.21 have been coded in, their introduction noticed by JNechaevsky.
 * Par times for Episode 4: Thy Flesh Consumed and Episode 5: Sigil can now be provided in BEX format.
 * A workaround has been implemented for missing textures in SWITCHES lumps: if one texture is missing, the whole pair is disabled. Thanks to Aurelius for reporting this issue with the OTEX 1.0 texture pack in the Doomworld forum.

**Bug Fixes**

 * Sigil's DEHACKED patch is no longer loaded when auto-loading the WAD, as this would break any episode-finishing demo for Doom 1.
 * Status bar background appearing at low framerates with Crispy HUD and automap overlay on when holding <kbd>TAB</kbd> key has been fixed, spotted by JNechaevsky and confirmed by Zodomaniac.
 * Configuration not being saved when exiting the game while recording a demo has been fixed, reported by Zodomaniac. Now configuration is always saved on exit.
 * Player weapon sound source is now set properly when loading a savegame, thanks to maxmanium from the Doomworld forum for bringing attention to this.

**Known Issues**

 * [No music and high-pitched sound effects](https://github.com/fabiangreffrath/crispy-doom/issues/454) occur with SDL2.dll v2.0.10 and SDL2_mixer.dll v2.0.4 on Windows in case of 5.1 speaker configuration, according to investigation by StasBFG. [An unofficial DLL pack fixing this and providing fluidsynth soundfont support](https://github.com/fabiangreffrath/crispy-doom/files/3616050/crispy-doom-DLL-fix-pack.zip) is provided by Zodomaniac.

Crispy Doom 5.6.3 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`ee9fc21f`](https://github.com/chocolate-doom/chocolate-doom/commit/ee9fc21fd6b7e50706fa093b9ccabd6dd56b02db).

### Crispy Doom 5.6.2 

Crispy Doom 5.6.2 has been released on September 13, 2019. The primary aim of this release is to fix the music-related bugs that surfaced in 5.6.1 and previous releases.

**Bug Fixes**
 
 * Pulled midiproc-related bug fixes from Chocolate Doom.
   * Use inherited handles to communicate with midiproc to prevent libraries that print error messages to standard streams from disrupting communication with the subprocess. Thanks to Zodomaniac for noticing this years ago when playing with the Memento Mori music PWAD, to Fabian Greffrath for spotting where this bug lurks and to AlexMax for finally fixing it!
   * Call `UnregisterSong()` where appropriate and do not unset `midi_server_registered` in `StopSong()`. This fixes the same song being played over and over again despite level changes when using MP3/OGG/FLAC music PWADs, pointed out by Zodomaniac.
 * Clean screenshots are now saved without demo progress bar after Zodomaniac spotted that it gets burned into them.
 * Screenshots are now saved without alpha channel, they were transparent before on MacOS as JamesDunne reported.

**Other Games**

 * Heretic's `BLOCKMAP` limit has been removed. Thanks to Jeff Green for the contribution.

Crispy Doom 5.6.2 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`ee9fc21f`](https://github.com/chocolate-doom/chocolate-doom/commit/ee9fc21fd6b7e50706fa093b9ccabd6dd56b02db).

### Crispy Doom 5.6.1 

Crispy Doom 5.6.1 has been released on August 23, 2019. It is dedicated to hotfixing the bugs reported by the community after the 5.6 release.

**Bug Fixes**

 * The `IDBEHOLD0` cheat not cancelling the player's invisibility has been fixed, thanks to maxmanium for being watchful.
 * The crash when a door that is actually a platform is opened again while going down has now actually been fixed, thanks to maxmanium for pointing this out at the Doomworld forums and Zodomaniac for the confirmation.
 * The door-closing sound playing even when the door is already closed has been fixed, thanks to Worm from the Doomworld forums for the heads-up. This especially affects repeatable walkover triggers.
 * SIGIL.wad is no longer auto-loaded anymore if another PWAD already modifies the texture files. This fixes the buttons in REKKR being rendered incorrectly, thanks to IsBebs for the report.

**Regressions**

 * The "Show Player Coords: Always" setting is now disabled to prevent cheating while speedrunning. Thanks to ZeroMaster010 for the repeated suggestions at the Doomworld forums.

Crispy Doom 5.6.1 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`b9d4c04c`](https://github.com/chocolate-doom/chocolate-doom/commit/b9d4c04c840321f5ec70787d8afb1256766aaa01).

### Crispy Doom 5.6

Crispy Doom 5.6 has been released on August 1, 2019. This release features support for the new Ultimate Doom Episode 5: Sigil by John Romero (with its MP3 soundtrack by Buckethead) and the Doom Metal Vol. 5 metal soundtrack mod for all IWADs.

**Features**

 * SIGIL.wad and SIGIL_SHREDS.wad are auto-loaded with Ultimate Doom IWAD when available, suggested by buvk. The Sigil art screen is only used when finishing episode 5. If you want to replace DMENUPIC and other art by Sigil's, load it manually.
 * Support for alternative music tracks for Final Doom has been implemented as introduced in DoomMetalVol5.wad, music replacement tables provided by Zodomaniac.

**Improvements**

 * Joystick jump button can now be assigned, contributed by Jeff Green.
 * Item position in Crispness menu is now remembered as well as in the rest of Doom menu, fixing the non-Doominess spotted by JNechaevsky.
 * Ambiguity in music backend name `Native MIDI` pointed out by pmjdebruijn has been eliminated, now it reads `MIDI/MP3/OGG/FLAC`.
 * Automap colors for different things (visible with IDDT) have been figured out by Zodomaniac: orange for projectiles, including Lost Souls, and dark gold for shootable things like barrels.
 * Extra Arch-Vile fire spawn sound is only played if available, which makes Capellan's SpecBoss.wad work with Doom 1 as IWAD.
 * Optional secret counting in the "secret revealed" message has been introduced, suggested by Ledmeister.
 * Green brightmap is applied to barrels according to JNechaevsky's idea.
 * Colors for HUD digits have been improved on artistic advice by JNechaevsky.
 * Zooming and moving Automap with the mouse wheel has been implemented, thanks to JNechaevsky for the suggestion and testing.
 * Tally screen is displayed after ExM8, requested by Sector 147 and tested by JNechaevsky.
 * Weapon pickup message is printed when using the `TNTWEAPx` cheat, requested by Zodomaniac.

**Bug Fixes**

 * Support for SMMU swirling flats has been repaired.
 * Playing with 32 sound channels is now actually enabled, thanks to seed and SiFi270 for pointing this out and providing examples.
 * More crashes with maps without map title graphics lump are prevented.
 * Level transitions back from MAP33 when playing Doom 2 extensions (e.g. NERVE) have been fixed, thanks to buvk for reporting.
 * Playing up to three sounds from lines with more than one switch texture has been fixed, squashing the button spamming sound bug reported by Looper in the forums.
 * A crash when a door that is actually a platform is closed manually has been fixed, spotted by glyphic from the forums.
 * An off-by-one typo in the par time drawing decision has been fixed.
 * The SSG reloading sounds being breakable have been fixed, reported by JNechaevsky.
 * Flat lumps are prevented from being mistaken as patches, at least when composing textures. This fixes a crash when loading any map with Sunder.wad (and who knows where else) spotted by JNechaevsky. If the flat lump name is unambiguous, though, then the one found is used, as Brad Harding pointed out. This fixes WOS.wad.
 * The ammo type is reset in `P_CheckAmmo()` when a weapon is removed (by the `TNTWEAPx` cheat) after Zodomaniac's report, so that even the chainsaw which consumes no ammo is removed properly.
 
**Regressions**

 * Crispy's own WAD autoload mechanism has been replaced by Choco's one, autoloading files from the `doom-all` subdirectory of the config directory.

Crispy Doom 5.6 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`485b939b`](https://github.com/chocolate-doom/chocolate-doom/commit/485b939b9b01e00ab47cd34a9de4a4e901d96a33).

### Crispy Doom 5.5.2

Crispy Doom 5.5.2 has been released on April 1st, 2019. This is another hotfix release bringing the feature of taking over a demo any moment and some subtle improvements as well.

**Features**
 
 * Hitting the 'quit recording demo' button when playing back a demo will 'take it over', i.e. hand the controls over to the player and record the forked timeline into a different file. Multiplayer demos are prevented from being taken over.

**Improvements**

 * Crispy color scheme is now applied to the network GUI as Zodomaniac insisted.
 * 100000 demos of the same name can now be recorded without overriding files, thanks to Looper for the suggestion.
 * New `SKILL` cheat suggested by Zodomaniac has been introduced displaying current skill level: Baby, Easy, Normal, Hard or Nightmare.
 * `IDCLEV` cheat now supports level number `IDCLEV1A` to warp to E1M10: Sewers even if No End In Sight `neis.wad` with its ExM0 levels is loaded with XBox Doom IWAD. This obscure case was pointed out by Zodomaniac. `IDCLEV10` still warps to E1M10: Sewers (along with `IDCLEV1A`) if XBox Doom IWAD is used without E1M0 map in PWADs.

**Bug Fixes**
 
 * Desyncing of demos continued by using `-playdemo` and `-record` in case of demos recorded with `-fast`, `-respawn` or `-nomonsters` has been fixed, spotted by Zodomaniac.
 * Game is now un-paused after loading a game while recording a demo, on Looper's report.
 
Crispy Doom 5.5.2 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`fd171dda`](https://github.com/chocolate-doom/chocolate-doom/commit/fd171dda546f38a9b7a6158ed2c3c8044e4ce72d).

### Crispy Doom 5.5.1

Crispy Doom 5.5.1 has been released on March 7, 2019. This is a hotfix release to fix a minor bug introduced in the previous version.

**Bug Fixes**

 * Revealed secret sectors are now highlighted in green (instead of gold) on the Automap only if the "Show Revealed Secrets" feature is enabled (the "Extended Automap Colors" feature alone isn't sufficient anymore).

Crispy Doom 5.5.1 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`ecab8d3a`](https://github.com/chocolate-doom/chocolate-doom/commit/ecab8d3ac0ca34fbb6cb04b608926a57b6dbdcc5).

### Crispy Doom 5.5

Crispy Doom 5.5 has been released on February 27, 2019. This is another release which mostly addresses community suggestions.

Forceful setting of the `SDL_AUDIODRIVER` variable on Windows has been dropped. Windows "releases" from 5.5 onward will be based on the daily builds and use their SDL libraries with the default audio backend.

**Features**

 * Demo recording can now be continued by using `-playdemo` and `-record` simultaneously, based on a pull request by Fraggle.
 * Menu switches for level stats, level time and player coords are now separate. Choices are "Always", "In Automap" or "Never". Also, Automap stats widgets have been condensed a bit ('K' for Kills -- or 'F' for Flemoids in Chex Quest, 'I' for Items and 'S' for Secrets). Right-aligned widgets (i.e. player coords and FPS counter) have been moved a step further to the right.

**Improvements**

 * Revealed secret sectors are now highlighted in gold on the Automap if both the "Extended Automap Colors" and "Show Revealed Secrets" features are enabled. Zodomaniac suggested this feature.
 * The `IDBEHOLDA` cheat can now disable the full automap again, pointed out by Zodomaniac.
 * Autocompletion of savegame slot name with [PWAD name in case it contains the map + map number] is introduced for all kinds of controllers, not only for the joystick as in Choco.
 * The demo timer widget is now also drawn on intermission screens (if enabled), thanks to Looper for the suggestion.
 * The `MF_DROPPED` flag is now checked for all ammo and weapon things. This has been contributed by NeuralStunner.
 * A "Crispy" color scheme has been introduced for Crispy Setup based on the suggestions by JNechaevsky and Zodomaniac.
 * Monster seesounds are now uninterruptible if the "Play Sounds In Full Length" feature is enabled, thanks to BCG2000 for pointing this out. Also, sounds "played in the player's head" (i.e. from origin `NULL`) don't interrupt each other anymore, thanks to BCG2000's remark.
 * The `IDMYPOS` cheat now yields extra high precision coordinates updating for 10 seconds and discarding after that instead of going static (the latter caught by Zodomaniac).
 * If the "Walk Over/Under Monsters" feature is enabled, the usual 24 units step-up is now allowed even across monsters' heads, thanks to BCG2000's suggestion. However, jumping on a monster's head straight from the floor by means of "low" jumping is disallowed.
 * A map's default music isn't loaded anymore if MUSINFO data is available and the game gets loaded from a savegame, thanks to zstephens for filing the issue.
 * ExM0 maps are now supported, reachable either through the `-warp x0` command line argument or the `IDCLEVx0` cheat, as suggested by StasBFG for the "No End In Sight" megawad (neis.wad).

**Bug Fixes**

 * Crashes or black screens that occurred when switching specific rendering options have been fixed by a complete overhaul of the rendering stack re-initialization code.
 * The initialization value of `floor->crush` in `EV_BuildStairs()` has been fixed, inherited from Chocolate Doom. This has caused a rare and obscure demo desyncing bug on TNT map 22, reported by Dime.
 * Direct aiming is now applied to the Beta BFG code as well, thanks to NeuralStunner for drawing attention to this.
 * Screenshots without the "screen shot" message have (hopefully!) been fixed again for all platforms and all rendering options.
 * Pickup messages for weapons that are already owned have been brought back as Zodomaniac spotted their absence.
 * All additional player properties are now reset when finishing a level, e.g. you'll now never start a new level with your view in the sky.
 * The things' actual height is now calculated from the spawnstate's first sprite (for shootable solid things only). This mitigates the issue JNechaevsky once reported when both "Mouselook" and "Direct Aiming" are enabled and you miss some obvious targets, like e.g. Romero's head on a stick.
 * The priority for the "Ouch Face" has been raised so that it actually shows up, thanks to BCG2000's and JNechaevsky's carefulness.
 * The default color of HUD level stat names for Hacx is now blue.
 * MUSINFO support has been repaired after it was accidentally destroyed in 5.4 by not setting the `lumpname` variable anymore in `P_SetupLevel()`.

**Other Games**

 * Crispy Heretic now catches intercepts overflows which fixes a crash in E1M2 of "Lost and Forgotten".
 * Optional level stats for Crispy Heretic can now be enabled, see the commit message to [`11e6091a`](https://github.com/fabiangreffrath/crispy-doom/commit/11e6091ac13906b5c79238a0a7f49abe60e2c7c9).

**Errata**

 * A thing height clipping issue when standing on a monster's head on a moving platform has been vastly improved, but not entirely fixed yet. Monsters may still get stuck in walls occasionally, but players won't anymore.

Crispy Doom 5.5 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`25ae4973`](https://github.com/chocolate-doom/chocolate-doom/commit/25ae4973fab0cfffe47fbc8373dae8a8715786d7).

### Changes of Crispy Doom 5.4 from Crispy Doom 5.3

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

Crispy Doom 5.2 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch up to commit [`5329fb5d`](https://github.com/chocolate-doom/chocolate-doom/commit/5329fb5d75971138b20abf940ed63635bd2861e0).

### Changes of Crispy Doom 5.1 from Crispy Doom 5.0

Crispy Doom 5.1 has been released on January 13, 2018.

Crispy Doom 5.1 offers some new features and improvements, most of them requested by JNechaevsky, and is accompanied by a new Music Pack adapted to the SDL2 version and tested by Zodomaniac (if you are experiencing problems with playing music remove crispy-midiproc.exe from the game folder).

**Features**

 * Optional Brightmaps for select textures and flats ("walls") and/or pick-up and weapon HUD sprites ("items") have been introduced for all supported IWADs. Thanks to JNechaevsky for this contribution!
 * Extended automap colors are now optional, the setting is in the new 'Navigational' section of the Crispness menu.
 * A new setting for squatting down the weapon when falling has been introduced to the 'Tactical' section of the Crispness menu.
 * A new setting for miscellaneous sound fixes has been introduced to the Audible section of the Crispness menu.
 * Demo de-syncing cheats are now disallowed during demo recording or playback as Zodomaniac requested.
 * An option for weapon bobbing with half amplitude during firing has been added.
 * Combined card and skull keys are now supported in the status bar (if provided by a PWAD).
 * The `VERSION` cheat code showing the engine version, build date and SDL version has been introduced.

**Improvements**

 * The GPL-licensed `P_CreateBlockMap()` implementation from MBF has been re-introduced.
 * The "power up" sound is not played anymore when selecting the berserk fist, according to Paar's request.
 * Fake contrast logic is now also applied to mid-textured lines.
 * Demo version mismatch has been made non-fatal for in-game demos, the demo just doesn't play if its version differs from the IWAD version. On deliberate launch of a wrong versioned demo the game still exits with an error message.
 * The 'A secret is revealed!' message isn't shown anymore when the menu is active.
 * The current music now restarts if IDMUS00 is entered.
 * If the D_INTRO music is from a PWAD, and D_INTROA is from a different WAD file, the former is played, as galileo31dos01 pointed out.
 * The INTERCEPTS limit has been removed (i.e. the overflow of Vanilla INTERCEPTS limit = 128 is still emulated like in Chocolate Doom to maintain network and demo compatibility, but Crispy won't crash unlike Chocolate if the value goes beyond Choco's extended limit = 189).
 * The inverted palette effect is now disabled when the player dies (e.g. when telefragged while having the Invulnerability power-up).
 * The yellow bonus palette is now reset and never shown if the player dies.

**Bug Fixes**

 * The player's looking direction is restored from savegames only if either keyboard-look or mouse-look is enabled.
 * Binding of the crispy_smoothscaling config variable has been fixed, reported by Paar.
 * The missing line not drawn at the bottom of fuzzed sprites has been fixed.
 * VSYNC is now disabled if software rendering is enforced.
 * Support for compressed ZDBSP nodes has been brought back, it was accidentally removed in 5.0 by the switch to the SDL2-based configure script.

Crispy Doom 5.1 is based on Chocolate Doom 3.0.0 and has merged all changes to the Chocolate Doom master branch, which has meanwhile merged the sdl2-branch, up to commit [`55a1c1c9`](https://github.com/chocolate-doom/chocolate-doom/commit/55a1c1c9e5ddc3c4e4e27240860d48bdb5701798).

### Changes of Crispy Doom 5.0 from Crispy Doom 4.3

Crispy Doom 5.0 has been released on November 04, 2017.

This is the first release based on SDL2, merging Chocolate Doom's SDL2-branch which has yet to see its own release.

Besides this major transition, it features the following changes and fixes:

 * The `NUMCMAPS` variable is now initialized to 32 to fix the missing "Finished" and "Now Entering" lines on the intermission screens for Doom 1 (bug introduced in Crispy 4.2).
 * Joystick look up/down (which has been introduced for the three non-Doom games in Choco's SDL2-branch) has been adapted to Doom.
 * Smooth scaling of the framebuffer content to the game window (which is the only choice in Choco's SDL2-branch) has been made optional with plain nearest interpolation as the other alternative.
 * The 60FPS and 70FPS rendering modes have been removed in favor of proper VSYNC.
 * DOS references in the game quit confirmation dialogs have been replaced with the name of the actual operating system.
 * A new `-monosfx` command line parameter has been introduced to play all sound effects in mono. Inspired by other Choco derivatives like Russian Doom and Riscos Doom.
 * Resolutions above 1920x1200 are now properly supported, thanks to SDL2-based hardware scaling.

Crispy Doom 5.0 has merged all changes to the Chocolate Doom sdl2-branch up to commit [`413f4ecd`](https://github.com/chocolate-doom/chocolate-doom/commit/413f4ecd7aae41229cc7b40ea3275970dc18cdb4).

### Changes of Crispy Doom 4.3 from Crispy Doom 4.2

Crispy Doom 4.3 has been released on September 19, 2017.

This quick fix release introduces improvements requested by ZeroMaster010 and is supposed to be the last release based on SDL1.

**Improvements**

 * The `sector->soundtarget` element is now saved in extended savegames.
 * The `-shorttics` command line option has been added to force low-resolution turning even when not recording a demo which is useful for demo run practice.
 * Smoother fake contrasts are disabled again as they disturb pixel perfect alignment for speedrunners.

Crispy Doom 4.3 and 4.2 have merged all changes to the Chocolate Doom master branch up to commit [`83a71dd8`](https://github.com/chocolate-doom/chocolate-doom/commit/83a71dd850fb02e571e2b5117f234395f8869a11).

### Changes of Crispy Doom 4.2 from Crispy Doom 4.1

Crispy Doom 4.2 has been released on September 15, 2017.

The changes since 4.1 are mostly polish of previously introduced features.

**Improvements**

 * DOS references have been removed from the game quit confirmation dialogs on Zodomaniac's request inspired by Doom Retro, now they say:<br>`Press Y to quit.`<br>`I wouldn't leave if i were you. Your desktop is much worse.`<br>`You're trying to say you like your desktop better than me, right?` and<br>`Don't go now, there's a dimensional shambler waiting at the command prompt!`<br>Note: When switching to SDL2, the desktop references will be replaced by the platform name detected by SDL2 ;)
 * "Screen shot" message doesn't get burned into the screenshots anymore when they are taken one after another on Zodomaniac's report.
 * Map coordinates enabled by the `IDMYPOS` cheat are now persistent, i.e. they don't automatically disappear anymore after a few seconds.
 * The (`sprframe->rotate == -1`) case is handled since it has been made non-fatal.
 * Interpolation of player missiles for the first tic is suppressed.
 * Different spellings of similar cheats in other engines are now supported:<br>Kill all monsters and disable all cube spitters: `TNTEM` (PrBoom+), `KILLEM` (MBF), `FHHALL` (Doom95)<br>Toggle monsters being deaf and blind until attacked: `NOTARGET` (PrBoom+), `FHSHH` (Doom95)<br>Display framerate: `SHOWFPS`, `IDRATE` (PrBoom+)
 * Higher scaling video modes up to 3200x2400/3200x2000 are supported on Calinou's request.
 * Crashes with maps with unusual map numbers are prevented.
 * Music number under- and overflows for both Doom 1 and 2 are prevented.
 * Negative ammo values are not allowed anymore thanks to the report by Zodomaniac and AnotherLife.
 * Evil grin is now triggered when `IDFA/IDKFA` is used which is 1) rather reasonable by itself as `ID(K)FA` gives weapons and items and 2) prevents the grin triggering when the next random item is picked up that AnotherLife noticed.
 * The first extra state is explicitly set to 1089 and the first extra mapthing to 150 to remain in sync with Doom Retro.
 * Weapon sprite is removed from clean screenshots only in maximized screen (no HUD) mode now, it was awkward before when the status bar was rendered but the weapon wasn't.
 * Calling `pspr` action pointers from `mobj` states and vice versa won't crash the game anymore after Zodomaniac's and JNechaevsky's investigation.
 * Menu messages are always printed with shadows regardless of which menu is the current one. Thanks to JNechaevsly for spotting this.

Crispy Doom 4.3 and 4.2 have merged all changes to the Chocolate Doom master branch up to commit [`83a71dd8`](https://github.com/chocolate-doom/chocolate-doom/commit/83a71dd850fb02e571e2b5117f234395f8869a11).

### Changes of Crispy Doom 4.1 from Crispy Doom 4.0

Crispy Doom 4.1 has been released on June 14, 2017.

This release adds some further polish to the engine both in terms of Vanilla bug fixing and Crispy-specific features.

**Features**

 * A "Deathmatch 3.0" mode has been added by AXDOOMER.
 * Dedicated music tracks for the 4th episode are now supported as lumps following the D_E4Mx name schema. Thanks to Death Egg for the feature request.
 * If the player is gibbed or crushed, the death animation is drawn in the Health widget of the Crispy HUD, keeping in sync with and applying the color translation of the actual player sprite. Thanks Zodomaniac for the suggestion.
 * Doomguy's mugshot never changes to evil grin or rampage face in god mode, because they don't have the glowing eyes as Zodomaniac and JNechaevsky pointed out.
 * Multi-resolution icon files for windows have been added by JNechaevsky and replace the icons formerly contributed by Zodomaniac.

**Improvements**

 * Game parameters (i.e. -fast, nomonsters, -respawn) are now re-read from command line if and only if a new game is started or a savegame is loaded. This is not performed if playing a demo is involved. Thanks to Zodomaniac for spotting this.
 * Savegame names in the Quick Load and Quick Save dialogs are now printed in golden letters, thanks to Zodomaniac for the suggestion.
 * Menu items are now drawn with a shadow instead of shading the background when the game is paused ot the menu is active, based on an idea by JNechaevsky.
 * Weapon recoil pitch has been refined with the help of JNechaevsky.
 * The Weapon Attack Alignment feature now has 4 options (Off, Centered, Horizontal and Bobbing) that have been shaped out in discussion with Im TPhentr, JNechaevsky and Zodomaniac.
 * The Zombieman's firing frames are now rendered full-bright. Thanks to JNechaevsky for pointing out this inconsistency.
 * Missing sounds are not fatal anymore.
 * The "angle" crosshair has been renamed "chevron" and got its vertical offset fixed. Thanks to Woolie Wool for pointing this out.

**Bug Fixes**

 * Players who left the game are no longer interpolated. Thanks to 38_ViTa_38 for the bug report.
 * Line breaks are now dynamically added for lines exceeding the screen width on the intermission screen. Thanks riderr3 for the bug report.
 * Weapons are not drawn 1 pixel too high anymore when player is idle. this bug has been re-fixed after being reported by JNechaevsky.
 * Weapons aren't changed anymore upon music change with the IDMUS cheat.
 * Restoring savegames with flipped levels has been fixed, following the April 1st incident.
 * Par times are now drawn conditionally on the intermission screen, as suggested by AXDOOMER.
 * A potential crash that would accur when using the IDBEHOLDS cheat with the Crispy HUD in the Shareware version has been eliminated.
 * Weapon sound sources have been implemented as requested by JNechaevsky. Sounds from these sources do not interfere with the sounds emitted by the player (e.g. "oof"), except for deliberately preserving the silent BFG trick from Vanilla.
 * The Ammo widget now switches colors at correct values, after Zodomaniac's report.
 * The "fast" parameters are now really only applied once, thanks to report by Pete-Lawrence.
 * Sound clipping in non-DOOM1 MAP08 has been fixed thanks to JNechaevsky and AXDOOMER.
 * Button states are now saved in extended savegames as suggested by JNechaevsky and Jeff Doggett.
 * The view cannot be changed anymore when playing back demos, resolving the ambiguity reported by Zodomaniac.
 * Crispy Doom now supports the SMOOTHED mod, thanks to a missing comma and an extra sprite from DR in the sprnames[] array. Thanks to AnotherLife aka VGA for the patient replies on the forums. Also, A_RandomJump() is now allowed in deaths in the cast sequence and in player weapon states, thanks to the code provided by Brad Harding and Jeff Doggett.
 * The Inability to load savegames while a netgame demo is playing has been resolved. Thanks to AXDOOMER for reporting this for Memento Mori PWAD where all in-game demos are netgame ones and JNechaevsky for providing the fix in the code.
 * A bug that has caused Revenants firing to desync in-game demos has been fixed, allowing for multi-hour stress tests by loading a PWAD that provides DEMOx lumps containing entire speed-runs.
 * A bug has been fixed that caused the demo sequence to crash for The Ultimate Doom and Final Doom if the DEMO4 lump was missing.

Crispy Doom 4.1 has merged all changes to the Chocolate Doom master branch up to commit [`429fa571`](https://github.com/chocolate-doom/chocolate-doom/commit/429fa571c476bf8ffb8aa9994e63db0ef79efe23).

### Changes of Crispy Doom 4.0 from Crispy Doom 3.5

Crispy Doom 4.0 has been released on Mar 11, 2017. It is based on Chocolate Doom 2.3.0.

This release introduces some new features and numerous improvements and bug fixes.

**Features**

 * It is now possible to play demos from savegames, thanks Zodomaniac for the idea.
 * It is now possible to delete savegames.
 * Different crosshair types have been introduced: cross, angle and dot.
 * Additional BOOM and MBF states, sprites and code pointers (supported through BEX format) have been added.
 * DEHACKED states up to 4000 and 100 additional sprites and mapthings from Doom Retro have been added.

**Improvements**

 * Dead players falling off an edge don't make an "oof" sound anymore.
 * Extended savegames have been improved to fix restoring of "fireflicker" sectors.
 * Fake contrast has been smoothened.
 * The Cyberdemon's firing frames are now rendered full-bright.
 * Sound is now properly clipped in non-Doom1 MAP08.
 * Status bar face hysteresis has been eliminated.
 * Toggling "always run" now plays a sound.
 * 16 sprite rotations are now supported.
 * Negative player health that can be shown optionally.
   Thanks to JNechaevsky for suggesting all of the above!
 * Smooth freelook and weapon recoil has been implemented by AlexMax, thanks for that!
 * DMENUPIC is now used instead of TITLEPIC for the BFG Edition of DOOM2.WAD.
 * The screenshot filename limit has been increased according to bzzrak's suggestion.
 * E2M9 is shown on the E2 intermission screen if it has been completed.
 * The MAXBUTTONS value is doubled to allow for using the "SPECHITS" cheat on E2M5.

**Code Clean-Up**

 * The "Crispness" menu has been disabled in the Setup tool because of its size and functionality being unable to cover all the options.
 * The "Invert Vertical Axis" option in the Setup tool has been replaced with "Permanent Mouse Look".
 * DoomBSP's original BLOCKMAP builder is now used instead of MBF's one as Crispy's internal BLOCKMAP builder (with removed limits and support for flipped levels and BLOCKMAP compression).
 * Linedefs without tags don't apply locally  anymore even in singleplayer (they send the wrong signal, maps that have this must be considered broken).

**Bug Fixes**

 * In the BFG Edition the correct music track is played when starting "Hell on Earth" after the "NRFTL" episode. Thanks to Zodomaniac for the bug report.
 * Blood spawned by monsters that are being harmed/killed by crushers has been fixed. Thanks to JNechaevsky for the report.
 * Flats don't get more distorted the closer they are to the right in Low-Res mode anymore, thanks to JNechaevsky's report.
 * Attacking Lost Souls flying over the player now don't bump into him (being meters above ;) and deal damage as Zodomaniac reported.
 * While in menus player view is not changed in singleplayer mode and rotated horizontally only in multiplayer mode as pointed out by Zodomaniac.
 * Music number overflows at MAP >= 36 are prevented after Zodomaniac's report.
 * The game doesn't crash anymore on the intermission screen with maps > 33 which was spotted by Zodomaniac.
 * Game parameters are now re-read from the command line when starting a new game, because an in-game demo could change them in between as Zodomaniac noticed.
 * When playing demos, NERVE.WAD levels are now skipped only if map <= 9 (the game errored out on demo's MAP10 before).

Crispy Doom 4.0 has merged all changes to the Chocolate Doom master branch up to commit [`9a257cdf78`](https://github.com/chocolate-doom/chocolate-doom/commit/9a257cdf78ee4fa7d3de5bbb5be3ca733b06077f).

### Changes of Crispy Doom 3.5 from Crispy Doom 3.4

Crispy Doom 3.5 has been released on Dec 27, 2016.
This release introduces some major new features as well as refinements and bug fixes.

**Features**

 * Total time for all completed levels is now counted, mostly taken from PrBoom+.
 * Extended savegames have been implemented that store the name of the WAD the current map has been loaded from. It can be toggled in the Crispness->Tactical menu and is enabled by default. When attempting to load a savegame, it is checked whether the map is restored from the same WAD file and if this check fails, the player is informed and given the choice to cancel or continue loading the savegame.
 * While at it, total level times, extra kills (i.e. respawned or resurrected monsters), [sectors with fireflicker](https://doomwiki.org/wiki/Sector_type_17_is_disabled_after_loading_a_saved_game), Automap markers and the players' lookdirs are also stored in extended savegames.
 * Furthermore, [plats in stasis](https://doomwiki.org/wiki/Moving_platform_heights_not_preserved_in_saved_games) are now properly saved and restored.

**Improvements**

 * New 128x128 icons by Zodomaniac.
 * Playing sounds in full length is now optional. Thanks to mason1729 and Danfun64 for the suggestion.
 * Overflow simulation into the frame table has been dropped, so that e.g. the `vbatman.deh` patch for Batman Doom is not necessary anymore.
 * Bullet puffs are now drawn in outdoor areas. Thanks to Ptoing for the idea.
 * Certain projectiles (e.g. the player's plasma gun and some enemies' weaponry) aren't absorbed by outdoor walls anymore. Instead, missiles are set into the last safe state in their death state sequence. Thanks to plumsinus for the suggestion.
 * In Chex Quest, the kill stats are not called "kills" anymore, as in this game you don't "kill monsters" but "return Flemoids" back to where they came from.
 * Weapon sprites may still be rendered centered, but their actual coordinates will be left untouched.
 * Sprite offsets for mirrored sprites have been fixed.
 * Brain spitters are now disabled when the TNTEM cheat kicks in, by setting `numbraintargets` to `-1`
 * Blood and bullet puff sprites are now randomly flipped.

**Bug Fixes**

 * Proper music is played if a map is reloaded from a savegame during the intermission screen. Thanks to Zodomaniac for the bug report.
 * The rendering of the "Loading Disk Icon" has been repaired.
 * The "Tutti Frutti Effect" is also fixed in Low Resolution mode now. Thanks to Vesperas for noticing.
 * MBF sky transfers with tag 0 have been fixed. Linedefs 271 & 272 are allowed to use tag 0 to affect the whole level. Thanks to Jeffdoggett for the heads-up and the code example.
 * The yellow key in TNT MAP31 isn't marked as multi-player only anymore, arranged alongside the fix for the Wolf SS with the BFG-Edition IWADs. Thanks to Andrewj for the suggestion and Jeffdoggett for the code example.
 * The SMMU swirling flats implementation has been repaired, thanks to Bzzrak for noticing.
 * A crash is fixed when attempting to play music in a map > 9 (e.g. during demo playback) when NERVE.WAD is loaded. Thanks to nbmrjuhneibkr for the report.

Crispy Doom 3.5 has merged all changes to the Chocolate Doom master branch up to commit [`5d3c0e0a82`](https://github.com/chocolate-doom/chocolate-doom/commit/5d3c0e0a824abd076bacffcfd01a6df5765435fe).

### Changes of Crispy Doom 3.4 from Crispy Doom 3.3

Crispy Doom 3.4 has been released on Jun 11, 2016.
This is another bug-fix release with only few new features.

**Features**

 * Support for for SMMU swirling flats has been added. However, this feature must be explicitely enabled by adding an ANIMATED lump and setting either `anim->speed` to some value higher than `65535` or by setting `anim->numpics` to `1` for the flat in question. So, currently no Vanilla flat will make use of it, but I hope that some future MegaWADs will ever adopt this feature. Thanks to plumsinus for the discussion.

**Improvements**

 * The music is not changed anymore when changing the map. This preserves the choice expressed by using the IDMUS cheat. Thanks to plumsinus for the feature request.
 * Crispy Doom now masquerades itself as "Chocolate Doom 2.2.1" in network games towards both clients and servers. This means it is now possible to play mixed network games with either Choco or Crispy as servers and clients. Thanks to AlexMax for requesting this feature.
 * Sprites are not visible behind closed doors or in sectors with no headroom anymore. Based on a similar fix found in MBF.
 * It is now possible to bind the "180 turn" key to a mouse button. Thanks to Danfun64 for the feature request.
 * Inputs are now flipped when recording or playing back demos with `-fliplevels`. Demos with monsters will still desync, though, because of the way the flipped BLOCKMAP is processed. Thanks to Danfun64 for reporting this issue and to Jon and Linguica for nagging me to fix it.
 * The GOOBERS cheat now leaves DR-type closed doors intact.
 * An Archvile fire's `floorz` and `ceilingz` values are now updated to prevent it from jumping back and forth between the floor heights of its (faulty) spawn sector and the target's actual sector. Thanks to buvk and vadrig4r for discussing this issue and to Quasar for his excellent analysis in [this thread](https://www.doomworld.com/vb/post/1297952).
 * Removed map objects may now finish playing their sounds.  When a map object is removed from the map by P_RemoveMobj(), instead of stopping its sounds, its coordinates are transfered to a "sound object" so stereo positioning and distance calculations continue to work even after the corresponding map object has already disappeared. Thanks to Megamur for requesting and jeff-d and kb1 for discussing this feature and especially jeff-d for the original implementation idea.
 * The "power up" sound is now played when selecting the Berserk fist, but only if it is not playing already. This brings back a feature that was removed more than a year ago, because I considered it "useless" back then.
 * The "saw up" sound is now played to finish when the Chainsaw is selected.
 * Empty demo lumps are now ignored. That is, it is now possible to load empty `DEMO1.lmp`, `DEMO2.lmp` and `DEMO3.lmp` files to prevent the game from running any demos during the title screen.
 * The lumps needed to render the Doom 2 "Which Expansion" menu may now be provided by a PWAD. Thanks to Doomer1999 and Zodomaniac for requesting this feature.
 * The map names of John Romero's new E1M8B and E1M4B maps as well as the individual maps of the Master Levels for Doom 2 are now shown in the Automap.
 * If the "Player may walk under/over monsters" feature is enabled, the monsters' melee attacks do now include a height check. That is, it is not possible anymore to get bitten by a monsters several stories beneath or above. Thanks to Zodomaniac for reporting this issue.
 * Strings are not colorized anymore if they have been Dehacked. Thanks to plumsinus for reporting the issue.
 * The Automap colors have been improved once again and are now basically PrBoom+'s default colors. Thanks to plumsinus and ptoing for reporting this issue.

**Bug Fixes**

 * The incorrect vertical offset for stretched MBF sky-transfer skies has been fixed. Thanks to plumsinus for the bug report.
 * The game missions pack_nerve and pack_master have been added as valid game mode/mission combinations. This should fix network games with these PWADs. Thanks  to Danfun64 for the bug report.
 * The resolution choice in the setup tool has been adapted to only show resolutions actually supported by Crispy Doom. Thanks to Zodomaniac for the discussion.
 * The music lump format detection has been fixed. Technically, this means that the MUS format header check has been enabled again. With the variety of music formats supported by Crispy Doom (through SDL_Mixer) it was not sufficient anymore to assume that any file which isn't in MIDI format must be in MUS format, except when the format conversion from MUS to MIDI fails. Thanks to Zodomaniac for the bug report and for providing some lumps in FLAC format which were reported as successfully converted by the mus2mid() function. This will break playback of the D_DM2TTL lump of DECA.WAD which erroneously has its format header read "PWAD" instead of "MUS".
 * The checks for "FRAME" keys in [CODEPTR] sections in BEX files are now case-insensitive.
 * A bug in the ordering of targets upon saving and loading a game has been fixed. Thanks to jeff-d for noticing.
 * The `lookdir` variable is only changed when processing a tic. Thanks to jmtd for noticing that mouselook up/down still works in single player games even when the game is paused or in a menu.

Crispy Doom 3.4 has merged all changes to the Chocolate Doom master branch up to commit [`010e52f99e`](https://github.com/chocolate-doom/chocolate-doom/commit/010e52f99e74a6f85a808f3c79504d96841e2e11).

### Changes of Crispy Doom 3.3 from Crispy Doom 3.2

Crispy Doom 3.3 has been released on Mar 10, 2016.

This is mostly a bug-fix release with only few new features.

**The Good, the Bad, the Ugly**

 * For the first time, this release of Crispy Doom is accompanied by some kind of a "Music Pack". It contains the fluidsynth library and all the libraries that it depends on, bundled with a freely-available soundfont. In order to use this "Music Pack", download the ZIP archive, extract its content into your Crispy Doom directory, select the "Native MIDI" music device and start Crispy Doom from the `crispy-doom-music-pack.bat` batch file. This file sets some environment variables that are required to convince the SDL_Mixer library to use fluidsynth with the bundled soundfont for music rendering. The Music Pack is available for download here:
   http://www.greffrath.com/~fabian/crispy-doom-music-pack_2.zip
 * From this release on, Crispy Doom will get released without Crispy Heretic, Hexen and Strife. These three games haven't seen any serious development in the past two years and have only been carried as dead freight. This situation will not change until someone volunteers to take over active maintenance of these ports.
 * From this release on, the official Win32 binaries will be build on MSYS2. The supplemental shared libraries bundled with the release will also be taken from this distribution. Since ABI-compatibility with the libraries bundled with former releases cannot be guaranteed, it is recommended to install this release into a clean directory and not mix up the libraries with other releases.

**Features**

 * A **variable frame rate** mode has been added. In addition to 35 fps mode which is the coarse, though original, frame rate on the one hand and completely uncapped frame rate, which burns up CPU cycles and battery, this adds two new rendering modes:
   * 60 fps may be ideal for those playing Doom on an LCD screen which tend to refresh with that rate,
   * 70 fps which means each tick is rendered twice (with interpolation) which may look much better on CRT screens.
   Thanks to plumsinus, Zodomaniac and Danfun64 for testing this feature.
 * Support for **ANIMATED and SWITCHES lumps** has been added. Likewise, support for **MBF sky transfer** linedefs has been added. All the code has been blatantly taken from MBF. Thanks to plumsinus for suggesting these features and to Breeder for reporting a char signedness issue and confirming the fix on the Doomworld forums.

**Improvements**

 * Space Marine corpses scattered around the maps are now randomly colorized. Thanks to Mr.Unsmiley for this suggestion!
 * There are now separate options for Colored Blood and Colored Corpses.
 * The original GREEN and BLUE2 color translation tables from BOOM have been re-introduced and are now used for Colored Blood and Pools of Gore. They keep some shades of red untouched and thus provide for a more balanced coloring.
 * Map Things are now counted on each map to provide for an additional source of pseudo-randomization (e.g. used for randomly flipping Space Marine corpses) and for more precise reporting of SPECHITS and INTERCEPTS overflows.
 * The loading of digital music from lumps has been simplified to support OGG, FLAC, MP3 and whatever format SDL_Mixer supports alike.
 * The "Secret Revealed!" message is always printed in gold, regardless of Colorized HUD texts being enabled or not. Likewise, the Crispness menu is now always rendered colorized.
 * The `-mergedump` parameter now reports the number of merged lumps and errors out if called without an additional argument.
 * The duration of the SSG muzzle flash has been reduced by one tic. While at it, the SSG flash frames have been turned full bright. Both fixes have been imported from MBF.
 * The weapon sprite is not centered anymore if its firing state's `misc1` variable is set. Fix imported from Doom Retro.
 * The game's skill level is now logged in plain text.
 * The SPECHITS cheat does not change the direction of doors anymore if they are already moving.
 * Sound effects are now played when DR type doors are manually closed while still open or re-opened while closing. Thanks to Megamur for the original suggestion.
 * The flashing HOM indicator is now pulsating.
 * On the Automap, the map title is now printed in gray letters from the first colon onward.
 * One-sided walls on the Automap are now drawn in a more desaturated red to better distinguish them from red-keyed linedefs. Thanks to plumsinus for this suggestion!
 * It is now forbidden to start a New Game or (Quick) Load Game while recording a demo. Thanks again to plumsinus for reporting this issue!
 * End Game will now quit if recording or playing back a demo, just like MBF.

**Bug Fixes**

 * A nasty bug has been fixed that caused multiplayer games to crash as soon as the second player joined the game. Thanks to Zodomaniac for initially reporting this bug and to Danfun64 for helping to track it down.
 * A graphical glitch caused by lines longer than 32767 units has been fixed. Thanks to entryway of PrBoom+ fame!
 * The sky offset is now correctly set if sky stretching is switched in-game (i.e. by toggling freelook, mouselook or recoil pitch). Thanks to plumsinus for reporting this bug!
 * The size of demo lumps recorded with the `-longtics` option is now calculated correctly. This bug caused the demo progress bar at the bottom of the screen to overflow beyond the frame buffer and eventually crash the game. Thanks again to plumsinus for reporting this bug!
 * The game is now paused if a message is shown (e.g. Quick Save confirmation) while recording a demo. Thanks to plumsinus for reporting this issue.
 * Movement of the projected laser crosshair is now smooth even with uncapped frame rate enabled. While at it, the crosshair is now vertically centered when idle, just like the static one. Thanks to Zodomaniac for reporting this issue.
 * An infinite loop caused by the Demon Speed bug has been fixed. Thanks to Zodomaniac for the bug report and the savegames that helped to reproduce the issue.
 * Wording in the Crispness menu has been changed from "Missiles" to "Projectiles". Thanks again to Zodomaniac for this suggestion.

Crispy Doom 3.3 has merged all changes to the Chocolate Doom master branch up to commit [`4077f339e7`](https://github.com/chocolate-doom/chocolate-doom/commit/4077f339e7a70fea357b047d8b6fd722b00f283b).

### Changes of Crispy Doom 3.2 from Crispy Doom 3.1

Crispy Doom 3.2 has been released on Nov 4, 2015.
This is merely a bug-fix release to fix some nasty bugs (missing icons in executables and crispy-*-setup.exe requiring admin rights) in the Windows version of Crispy Doom 3.1.

Crispy Doom 3.2 has merged all changes to the Chocolate Doom master branch up to commit [`fe32800392`](https://github.com/chocolate-doom/chocolate-doom/commit/2efd8ce2217cfbb81b9b5532228febfe32800392).

### Changes of Crispy Doom 3.1 from Crispy Doom 3.0

Crispy Doom 3.1 has been released on Nov 3, 2015.
This isn't considered a major Crispy Doom release, although it is based upon the Chocolate Doom 2.2.1 point release.

**Features**

 * **Colorized** Status Bar numbers and HUD texts can now be enabled separately.
 * If Colorized HUD texts are enabled, the colors of the keycards and skull keys are now emphasized in HUD messages.
 * Pools of gore left behind from crushing Cacodemons or Hell Nobles are now colorized in their respective blood colors if Colorized Blood is enabled. Thanks to Megamur for this suggestion.
 * Player messaged in multiplayer games are now colorized, i.e. the "indigo" player writes in grey and the "brown" player writes in gold.
 * The Status Bar face background is now properly colored when playing back a multiplayer demo that was recorded by another player than Player 1. Thanks to Zodomaniac for the bug report and demo.

 * Optional **Look Spring** has been implemented for both keyboard and mouse look, leaving the former default "Locked View" as another alternative (feature originally requested by user VGA in May 2015).

 * A **Direct Vertical Aiming** mode has been added. Formerly, auto-aiming would still kick in if a monster is aimed at and thus suppress direct aiming. This mode is now optional.

 * It is now possible to enable **Translucency** separately for Missiles and Items.
 * It is now possible to use the Translucent Crispy HUD even if the Translucency feature is generally disabled.
 * Tranclucency for Health Potions has been disabled, it just looked too crappy.
 * To compensate for this, the BFG spray explosions are now rendered translucently. ;)
 * Translucent rendering in the Low Resolution mode has been fixed.

 * **Screenshots** in PNG format are now a reproduction of the actual screen content, i.e. they are saved with the same dimensions as the rendered game.
 * The "Clean Screenshot" feature now got its own dedicated key binding. Previously, this feature was either overlooked or triggered accidently. Thanks again to Megamur for this suggestion.
 * In the course of this, the "Clean Screenshot" feature has actually been fixed. It relied on the engine rendering each frame twice which it doesn't do anymore since Chocolate Doom 2.2.0.
 * When taking a "Clean Screenshot" the background isn't shaded and neither the Pause picture nor the Menu are drawn anymore.

 * The system's native `qsort()` implementation with a comparison function retaining stable sorting is now used for ordering the `vissprites[]` array.
 * The default OPL emulation mode is now OPL3 with correctly reversed stereo channels.
 * Music lumps are now properly checked for being MP3 files. This was somehow forgotten when support for music lumps in OGG and FLAC formats has been implemented in Crispy Doom 2.3.
 * No nukage damage is applied anymore if the NOCLIP cheat is active.
 * The used Screen Mode is now always reported at start-up, Thanks to [FlaterCo](http://www.flaterco.com/kb/DOOM/ChocolateDOOM.html) for the idea.
 * Some information is printed to `stderr` when saving a game.

**Bug Fixes**

 * Lost Souls aren't counted as Kills anymore, not even in the separate `extrakills` counter.
 * When a vertex would need to move more then 8 map units for fixing a slime trail in `P_RemoveSlimeTrails()`, it is probably misplaced on purpose so its original coordinate is used instead. This fixes the rendering of [Linguortals](https://www.doomworld.com/vb/doom-editing/74354-stupid-bsp-tricks/).
 * Single-patched columns are normally not composited but directly read from the patch lump ignoring their `originy` value. This fixes an oddly rendered door in E3M5, thanks to ptoing and esselfortium for reporting.
 * Barrel explosions aren't randomly mirrored anymore. They have very distinct sideways shading on them and that flipping around just looks weird. Thanks to ptoing for reporting.
 * If a linedef with an untagged DR special is pushed from the wrong side, print a warning instead of crashing or exiting the game.
 * If the game is paused in Automap mode (but not in Automap Overlay mode) the screen isn't shaded anymore. Thanks to Megamur for this suggestion.
 * The blinking health indicator has been disregarded as distracting and has thus been disabled. Thanks to Megamur for the report.
 * Rendering glitches caused by integer overflows in the `rw_distance` variable have been fixed. This used to cause strange artifacts in e.g. `planisf2.wad` where sprites have been overridden by segs from far behind. Thanks to Andrey Budko for his help with fixing this.
 * Also, overflows in `R_PointToAngle()` for very long distances have been fixed. This should fix even more rendering glitches, e.g. the vertical HOM stripes at the horizon in `planisf2.wad`. Thanks again to Andrey Budko for helping me to fix this and taking my approach over into PrBoom+.
 * A bug has been fixed that caused mid-textures with out-of-screen coordinates to be entirely emitted from rendering. Thanks to vesperas for reporting this rendering glitch.
 * A bug reported by Valgrind as been fixed which caused reading of uninitialized memory from the `postcount[]` array. Since the values read from this array determine the amount of memory allocated for each column in a texture, this bug was quite a memory hog.

**Code Cleanup**

 * The `V_DrawPatch()` function has been simplified and now uses the same code path for opaque, translucent and colorized patches.
 * By now, running `cppcheck` on the Crispy Doom source code doesn't emit any more singnificant warnings than running it over the Chocolate Doom source code.
 * The Berserk Pack patch is only displayed in the Ammo widget if it is actually available (i.e. not in the Shareware version).
 * If the `DSSECRET` lump is available, e.g. from a PWAD, play this instead of `DSITMBK` when a secret is revealed and the corresponding feature is enabled.

Crispy Doom 3.1 has merged all changes to the Chocolate Doom master branch up to commit [`9084d870c4`](https://github.com/chocolate-doom/chocolate-doom/commit/ff61aa8695321e21b349583b4b43f19084d870c4).

### Changes of Crispy Doom 3.0 from Crispy Doom 2.3

Crispy Doom 3.0 has been released on July 14, 2015.
This is a major Crispy Doom release, because it is based upon the Chocolate Doom 2.2.0 release.

**Major Features**

 * Add optional rendering with **Uncapped Framerate**. Thank you very much to AlexMax for this major code contribution! In the course of this, add an FPS counter that can get activated with the new `SHOWFPS` cheat.
 * Allow for a **variable jump height**, either "low" (i.e. 8 units of vertical momentum as in ZDoom) or "high" (i.e. the previous value of 9 units as in Hexen or current PrBoom+).
 * Support the **`masterlevels.wad` all-in-one PWAD** file from the PSN version. Thanks to quakeguy123 for the suggestion.
 * If `-warp` is used with `-playdemo`, **fast-forward the demo** up to the requested map.
 * Fix rendering of **single-patched transparent textures** used as an upper or lower texture, or as a mid texture on a single-sided wall. Finally found a fix nearly one year after this has been requested by plumsinus. \o/

**Other Enhancements**

 * Add a third "Crispness" in-game menu page and get rid of some of the lesser used items in the "Cripsness" menu in the `crispy-doom-setup` tool.
 * Render missing flats as sky.
 * Permit binding mouse button to jump. Thanks to Jonathan Dowland for the PR.
 * Prevent frame buffer overflows in `V_DrawPatch()`. It should now be impossible to trigger a "Bad V_DrawPatch" error anymore. Also, prevent frame buffer overflows in `V_CopyRect()`.
 * Never override savegames during demo playback anymore.
 * Do not reset `-respawn`, `-fast` and `-nomonsters` parameters in `G_DoNewGame()` anymore.
 * The `NOTARGET` cheat makes all monsters forget their current target. Thanks to bradharding for the idea.
 * In the Automap, initialize the zoomlevel on huge maps so that a 4096 units square map would just fit in. Somehow inspired by Doom Retro.
 * Make "bad texturecolumn" errors non-fatal.
 * Colorize the confusing `IDBEHOLD` power-up menu. Inspired by Doom Retro.
 * Make the DEH/BEX parser more tolerant. If a DEHACKED file does not contain a valid header signature, print a warning but do not error out. If a DEHACKED file contains BEX sections which contain non-escaped newlines and there is no new valid section marker after the newline, try to proceed parsing with the previous line parser. Fixes loading the completely messed-up `Jptr_fix.bex` file provided on https://www.doomworld.com/pageofdoom/lostdoom.html.
 * Differentiate between Weapon Recoil Thrust and Pitch options.
 * When a linedef is missing a first sidedef (aka. right side), replace with a dummy sidedef instead of crashing.
 * Allow Chocolate Doom 2.2.0 clients to connect to Crispy Doom servers.

**Bug fixes**

 * In the Automap, do not calculate player coordinates for players not in game. Fixes a crash in multiplayer games reported by Marscaleb.
 * In Automap Overlay mode, draw the Automap beneath the bezel for smaller view sizes. Thanks to Ronald Lasmanowicz for reporting.
 * Also in Automap Overlay mode, for full view sizes, move the map title line to the bottom and remove the obtrusive map origin line.
 * Catch overflows in the `SlopeDiv()` function only during rendering. Fixes the single last remaining demo desync in Choco's statcheck test.
 * In the Crispy HUD, properly center the Berserk Pack patch in the Ammo widget.
 * Only interpret the second argument to the `-warp` parameter as "startmap" if it is not yet another parameter (i.e. begins with '`-`').
 * Restrict conditions for recognizing E1M10 and MAP33 and fix Par Times for MAP10 in "commercial" game mode.
 * The demo progress bar is now better recognizable.
 * Draw the Berserk Pack in the Ammo widget and blink the Health indicator also in Automap Overlay mode.
 * Force redrawing the status bar when switching Automap Overlay mode back and forth.
 * No Rest for the Living was not special-cased if the `nerve.wad` PWAD file name was in upper case. Thanks chungy for the report.
 * The map name for MAP33 now always defaults to "Betray" if it has not been dehacked to another name. Thanks to quakeguy123 for the suggestion.
 * The highest and lowest viewing angles can now be reached with the keyboard without needing to press the corresponding button a second time. Deviating from Heretic and Hexen behaviour here.
 * When resurrecting dead players with the `IDDQD` cheat, they now face the same direction again as before (not exactly, but closer).
 * Fix rendering glitches caused by segs that had their vertices moved in order to prevent slime trails. The shorter a segs is, the more affected is its angle by moving its vertices. So re-calculate seg angles after moving vertices in `P_RemoveSlimeTrails()` and use these angles during rendering only.
 * Remove the limits on merged `PNAMES` and `TEXTURE1/2` lumps.
 * Disable auto-loading of PWADs in the "shareware" game mode.

**Code clean-up**

 * Reformat `R_DrawColumnInCache()`, `R_GenerateComposite()`, `R_GenerateLookup()` and `R_DrawColumn()` to closer match their Chocolate Doom pendants. These functions were previously replaced with code from MBF to fix the Medusa and Tutti-Frutti effects, respectively. Now, these fixes have been merged into the Vanilla functions.
 * Do the slow linear search in `R_FlatNumForName()` only if the initially returned lump number is not within the "flats" range.
 * In the Crispness menu, the "Jumping" and "Crosshair" items now offer multiple choices.
 * Get rid of the `player2_t` type.

**Other ports**

 * Remove `MAXVISPLANES`, `MAXDRAWSEGS`, `MAXSEGS` and `MAXVISSPRITES` limits for Crispy Heretic.

Crispy Doom 3.0 has merged all changes to the Chocolate Doom master branch up to commit [`b81ba59d66`](https://github.com/chocolate-doom/chocolate-doom/commit/b81ba59d6649794cfba735d1ffa0e9458a6ee486).

### Changes of Crispy Doom 2.3 from Crispy Doom 2.2

Crispy Doom 2.3 has been released on March 4, 2015.

**Automap improvements**

 * The Automap now has an **Overlay Mode** that draws the map directly onto the player view and that can be toggled by pressing '`O`'.
 * The Automap now has a **Rotate Mode** that -- as the name suggests -- keeps the player arrow oriented upwards and rotates the entire map instead. It can be toggled by pressing the '`R`' key and fits nicely into the Automap Overlay Mode.
 * If Follow Mode is disabled and Overlay Mode is enabled, instead of panning through the map, it will remain static.
 * The extra triangle for the player is not drawn anymore if the `IDDT` cheat is enabled.
 * If Follow Mode is enabled an actual crosshair is drawn instead of a single point.

**Cheats improvements**

 * A new `NOMOMENTUM` cheat has been added -- that is, merely enabled, all the code has already been there before -- that avoids the player from gaining momentum. It is called a "debug aid" in the source code and is pretty useless unless you want to position the player at an exact position.
 * A "gibs" sound is now played if a dead player is resurrected by the `IDDQD` cheat.
 * Players are now given full in-air control in `IDCLIP` mode. Thanks Linguica for the idea.

**New features**

 * The **secret E1M10 map** present exclusively in the XBOX variant of The Ultimate Doom is now supported. The map is entered by triggering a secret, erm, "wall" in E1M1 and exits to E1M2. The map name is right in both the Automap and the Intermission screen and the blood splat is drawn in the latter, though only once. To enter the map with a cheat code, type `IDCLEV10`; to warp there from the command line, type `-warp 1 A` (or any other letter for the map). Thanks to Ronald Lasmanowicz of Wii-Doom for some suggestions regarding the implementation. If you want to try out this feature but do not own the XBOX variant of `DOOM.WAD`, try the patch posted here: https://www.doomworld.com/vb/wads-mods/71374-patch-ultimate-doom-1-9-xbox-doom/ .
 * The **texture files** (`TEXTURE1/2` lumps) and **patch lookup tables** (`PNAMES` lumps) from PWADs do not override those of the IWAD anymore. Instead, up to 8 `TEXTURE1/2` lumps and up to 4 PNAMES lumps **get merged**, so that all textures from all loaded WADs are available. This makes it possible to e.g. run `crispy-doom -iwad freedoom2.wad -file doom1.wad`, which loads the *Freedoom: Phase 2* IWAD and replaces all textures which are also present in the *Doom 1 Shareware* WAD but leaves all others intact.
 * **Weapon Pitch** has been added as a new feature and fits nicely if combined with Weapon Recoil.
 * The **static crosshair** has been added back. It is now drawn as the HU font's '`+`' character in the center of the screen and is rendered translucent if that is globally enabled. It has become the default, the crosshair rendered into the game world has to get activated by a new dedicated switch.
 * Support for **music in OGG or FLAC formats from lumps in PWADs** has been added. This means, it is possible to run e.g. DoomMetalVol4 by simply calling `crispy-doom -file DoomMetalVol4.wad`. Please note that this feature is only available if Music is set to "Native MIDI".
 * PWAD files and DEHCAKED patches that are located in the config directory (i.e. the same directory that holds the `crispy-doom.cfg` file) and that are named following the `preloadN.{wad,deh,bex}` naming scheme (with `N=[0..9]`) are **automatically pre-loaded at startup**. For example, Linux users may call `ln -s /path/to/DoomMetalVol4.wad ~/.crispy-doom/preload0.wad` to automate the call cited in the previous bullet point.
 * It is now possible to **dump the result of the `-merge` parameter into a file** by means of the `-mergedump <filename>.wad` parameter. This is meant as a convenient "DEUSF replacement" for Vanilla Doom or other exotic source ports that still do not support the "-merge" feature. It is now possible to e.g create a portable variant of Requiem by calling `crispy-doom -iwad doom2.wad -file requiem.wad reqmus.wad req21fix.wad -mergedump req4all.wad`. Please note that no duplicates are removed, so the resulting file gets rather big (about as big as the sum of all input files).
 * Crispy Doom is now able to (re-)create **BLOCKMAP lumps** if they are either too small or too big or if requested by the user by the `-blockmap` parameter. The actual BLOCKMAP creation routine has been taken from Lee Killough's MBF source port.
 * Crispy Doom is now able to load maps with **NODES lumps in either compressed or uncompressed ZDBSP format or DeePBSP format** and/or LINEDEFS and THINGS lumps in Hexen map format. The code is mostly adapted from PrBoom+ and Chocolate Hexen, though especially the ZDBSP nodes loading routine has been heavily modified, condensed and simplified. Please note that although it is now possible to load and explore maps in Hexen format, all interactions with their environment are most probably broken. Let me state that this feature has only been added because of a rejected patch of mine (tsts...) to add support for compressed ZDBSP nodes to PrBoom+. ;)
 * If running out of zone memory, Crispy Doom now allocates another zone of twice the size instead of crashing with a **Z_Malloc error** message. Please note that this turns Crispy Doom into a memory hog quite quickly, but it should only happen in very huge maps anyway and is still better than crashing.

**Rendering improvements**

 * Wobbling long walls have been fixed, using e6y's beautiful fixed-point math formula which is also used in PrBoom+.
 * Visplanes with the same flats now match up far better than before. Fixed using code adapted from PrBoom+, converted to fixed point math (and already merge back into PrBoom+).
 * Some rendering glitches introduced by the Wiggle Fix (feature introduced in Crispy Doom 2.0) have been fixed, thanks to e6y and PrBoom+.

**Minor improvements**

 * Lost Souls and spawned monsters are now counted in an extra variable that is displayed in the Automap stats next to the regular kills count, separated by a '`+`' sign. They are not added to the regular kills variable anymore which means one less (demo-critical) deviation from Vanilla behavior and one less switch in the Crispness menu. As a collateral damage, the special-casing of the Keen monsters in `-nomonsters` mode had to go as well, but that's to cope with.
 * In the Crispy HUD, the health indicator now only blinks if below 10%.
 * Weapon recoil is now applied after trajectories have been calculated.
 * Warnings are now printed whenever `SPECHITS` or `INTERCEPTS` overflows are triggered.
 * The previously selected savegame in the Load Game menu is now also pre-selected in the Save Game menu and the other way round.
 * The `braintargets` limit has been removed. It was previously set to `32` and broke e.g. Speed of Doom's MAP30 which has `40 braintargets`.

Crispy Doom 2.3 has merged all changes to the Chocolate Doom master branch up to commit [`7d6f26f3a4`](https://github.com/chocolate-doom/chocolate-doom/commit/622653dde922a486a056b63d37dc6d7d6f26f3a4).

### Changes of Crispy Doom 2.2 from Crispy Doom 2.1

Crispy Doom 2.2 has been released on January 2, 2015.

**Laser Pointer improvements**

 * The laser spot now points to the center coordinates of the target.
 * Laser spots are now solid instead of translucent. They became practically invisible too quickly, which was exactly not their purpose.

**Cheats improvements**

 * New cheat: IDBEHOLD0 will reset all player powers.
 * When the IDDQD cheat is activated after a player has died, his body is respawned at the current position.

**Feature review**

 * The "center weapon sprite when firing" feature is now optional.
 * The "colored blood" feature as well as the "fix lost soul and spectre blood" features are now optional.
 * The "randomly mirrored corpses" feature is now optional.
 * A second page has been added to the in-game "Crispness" menu to account for the added options.
 * New optional feature: The "Kills" statistics are fixed, i.e. the "Kills" ratios shown after finishing a map or in the automap stats are adjusted each time a new monster is spawned on the map (e.g. in Nightmare mode, a Lost Soul spawned by a Pain Elemental, a monster resurrected by an Archiville or a monster spawned by a cube spitter). Furthermore, in "-nomonsters" games, Keens are now preserved but don't count towards the "Kills" statistics.
 * If the "player may walk over/under monsters" feature is enabled, the player is now also able to walk under solid hanging corpses.

**Further improvements**

 * In the Crispy HUD, the health indicator now blinks if it is below 10%. Somehow inspired by Doom Retro.
 * Logging to stderr has been generally improved. Especially, the map slot, the WAD file and the skill are now printed whenever a map is loaded. Furthermore, absurd texture names in error messages have been fixed.
 * Linedefs with the two-sided flag but without second sidedef are now fixed in a demo-compatible way, i.e. they are rendered as one-sided walls when a mid-texture is set and transparent else.
 * Missing textures and flats do not lead to crashes anymore. Missing textures are rendered as HOM, i.e. black in Crispy Doom, and missing flats are replaced with the first flat available. In a very limited scope, this feature makes it possible to intermix resources and maps from different Doom missions.
 * Most Slime Trails are removed when loading a map. This feature has been mostly taken from Lee Killough's implementation in MBF, but has been modified for demo-compatibility to not modify actual vertex coordinates, but instead dummy "pseudovertexes" that are only used in rendering.

**Additional Bug fixes**

 * The patch color translations are now cleared after drawing the "Crispness" menu cursor. This could lead to the menu picture or the status bar being rendered too dark. Thanks to fistmarine for the bug report.
 * Crashes in huge maps that occured due to the Wiggle-Fix (introduced in Crispy Doom 2,0) have been fixed. This has been done by upgrading the entire renderer to use 32-bit integer math. Thanks to kb1, RjY and Quasar for pointers to the relevant code changes!
 * Crashes caused by the laser spot getting drawn behind the view plane have been fixed.
 * The laser spot is not able to trigger an intercepts overflow anymore.
 * A crash in maps with a cube spitter but without spawn spots (e.g. MAP04 in DV.WAD) has been fixed.
 * A crash during map transitions has been fixed if a PWAD contains a stray MAP33 lump (e.g. INTIME.WAD). MAP33 is now only considered a regular map if the CWILV32 lump is also present.

Crispy Doom 2.2 has merged all changes to the Chocolate Doom master branch up to commit [`3c6f436997`](https://github.com/chocolate-doom/chocolate-doom/commit/80bf45c902f78e3cd0212938ee176a3c6f436997).

### Changes of Crispy Doom 2.1 from Crispy Doom 2.0

Crispy Doom 2.1 has been released on November 12, 2014. This is mostly a bug fix release to treat a bug that reproducibly crashed the setup program on Windows systems.

Further changes that have accumulated in the short time frame include:

**Color translation improvements**

 * Color space translations are now also applied to gray shades. This means that now really **any** status bar can be fully colorized. The only exception are the gray drop shadows of the original IWAD status bar numbers, which are intentionally left untouched.

**Menu improvements**

 * In menus, numerical values are now always shown next to sliders.
 * Setting mouse sensitivity to 0 now disables the corresponding axis entirely.
 * The Crispness menu has got a complete overhaul:
   * The in-game "Crispness" menu now has a solid background. There was too much text displayed and the game graphics in the background were too distracting for it to remain legible.
   * The menu items are now ordered into "Visual", "Tactical" and "Physical" categories.
   * Disabled menu items are now indicated by darker colors.

**More victims of the feature clean-up**

 * The "power up" sound is not played anymore when selecting the Berserker fist. This feature was hardly noticable at all and did not justify all the code that it required. Also, speaking about random non-Vanilla featuritis...
 * Common mapping errors (e.g. missing sidedefs) are not fixed upon level setup anymore. After all, they are errors and should be fixed during mapping and not by the engine. But, more importantly, some mapping errors are demo and network game critical. However, due to the order in which variables are juggled around in p_setup.c, it is impossible to conditionally enable the fixed based on certain variables. Instead, I attempted to implement a separate patch, but failed and thus decided to drop that feature entirely.

**Further improvements**

 * The screen wiping speed and amplitude have been adjusted to be closer to Vanilla. This involved removing a work-around that has been introduced in Crispy Doom 1.0.
 * During demo playback, a thin (2 px) bar indicating demo progress is now printed at the bottom of the screen (similar to PrBoom+).
 * A warning will be printed when loading a level if it contains unknown linedef types (i.e. line->special > 141), e.g. Boom linedefs or generalized linedef types.

Crispy Doom 2.1 has merged all changes to the Chocolate Doom master branch up to commit [`804a666728`](https://github.com/chocolate-doom/chocolate-doom/commit/66b295d461789f204d19b7181b1a11804a666728).

### Changes of Crispy Doom 2.0 from Crispy Doom 1.5

Crispy Doom 2.0 "Back to the Roots" has been released on October 27, 2014.
This is a major Crispy Doom release, because it has merged the Chocolate Doom 2.1.0 release.

**"Back to the Roots" campaign**

 * For the current release, Crispy Doom has undergone a strict feature revision. Features that are considered demo or netgame critical but are not user-visible enough to justify a dedicated switch, have been removed. This has been done to avoid development two separate feature sets, one for demos and net games and one for regular play, and to generally remain closer to actual Vanilla Doom behaviour. As a rule of thumb, features which affect the BFG Edition support, especially the No Rest For The Living expansion, and features which affect level progression in general have been left intact. Furthermore, using cheats is considered at own risk (they are disabled in netgames and screw up demo recording anyway), so they are no longer restricted to single player games.
   * It is not possible anymore to switch between the regular fist and the chainsaw.
   * A new level will not automatically start with the chainsaw anymore if the previous one was quit with the fist.
   * The "restart level" and "go to next level" keys are considered as shortcuts to the IDCLEV cheat and are thus treated as such, i.e. they are only disabled in net games.
   * The monster corpse flipping feature (based on randomization of monster corpse health values) is now unconditionally enabled, it is considered harmless.
   * The vertical bobbing of ammo released by killed monsters has been removed, it has been shown to desync demos.
   * The ability to gib a monster with the SSG has been removed, it has caused demos to instantly desync whenever it occured. However, that feature is not lost forever, it has already been merged into Doom Retro in the mean time.
   * The weapon sprite is now unconditionally centered during shooting, but only horizontally. The vertical position affects the weapon lowering and raising times which in turn affect demo sync.
   * The behaviour of the "Run" key when resurrecting a dead player has been switched. If it is held down, the most current savegame is now reloaded. If it is not pressed, the level now starts from scratch, just as in Vanilla Doom.
   * Furthermore, for a cleaner diff relative to Chocolate Doom, the code added or changed by Crispy Doom has been extensively documented, commented, restructured and cleaned up where appropriate.

**Support for BEX files and lumps**

 * Support for DEHACKED lumps or files in the BEX format (established in Boom and MBF) has been added. The implementation is considered complete and supports the following features:
   * BITS mnemonics in regular "Things" sections,
   * [CODEPTR] sections,
   * [PARS] sections,
   * INCLUDE directives for which the following additional rules apply:
     * DEHACKED lumps loaded from PWADs may not include files,
     * files that have already been included may not include other files,
     * files included with the INCLUDE NOTEXT directive will have their regular "Text" sections ignored;
   * [STRINGS] sections. Support for the latter has been merged from Chocolate Doom, but is enabled in Crispy Doom without any further restrictions.

**Menu improvements**

 * The in-game menu now has a "Crispness" item that resembles the same item from the crispy-setup tool. It allows to enable or disable most of Crispy Doom's features from inside the game without the need to open an external application and restart the game.
 * The "permanent mouse look" switch has been added to the "Mouse Sensitivity" menu.
 * During demo recording or net games, it will be impossible to enable certain "Crispness" features for compatibility reasons. These will appear as grayed out in the menu.
 * Since there are no graphics for the word "Crispness" available in regular Doom resources, the menu item has to be renderd in the HU font. To avoid optical clashes with the other menu entries, these are now rendered in the HU font as well.

**Automap improvements**

 * If the "Automap stats" feature is enabled, the current map coordinates will be shown in the upper right corner of the Automap in a human readable form.
 * Also, if the "Automap stats" feature is enabled and the current map is loaded from a PWAD, the file name of this PWAD and the map are displayed in the bottom left corner of the Automap.

**Cheat improvements**

 * The IDMYPOS cheat has been improved to show the current map coordinates in the upper right corner in a human-readable form. This is the same widget that is also shown in the Automap when the "Automap stats" feature is enabled. It is shown twice as long as the text line printed by the original implementation, i.e. long enough to take a decent screenshot.
 * The SPECHITS cheat now also triggers tag 666/667 sectors. On maps, where specific monsters are expected to trigger that tag, their actions take precedence. On any other map, the tags are triggered as killing all "Keen" monsters would do.
 * The TNTHOM cheat has been introduced to toggle the flasing HOM detector on and off in-game.
 * IDMUS0x or IDMUSx0 will not lead to crashes anymore.
 * IDCLEV00 now reloads the current level and, in Doom 1, IDCLEV0x now warps to map x in the current episode.

**Color translation improvements**

 * The former static color translation tables have been replaced by dynamically generated ones based on actual color space translations, thanks Paul Haeberli. Formerly, the color translation tables were identical to the ones found in Boom and MBF and only covered the red range (palette indices 176-191, 44, 45, 47, 67) so that other colors were not translated at all. With the new tables, **any** color can be converted to any other color in the Status Bar or the HU font. Due to limitations in the color space translation procedure, though, gray tones will remain gray. Furthermore, due to limitations in the original Doom palette, light blue tones are often mapped to grays.
 * The translucency filter table has been improved. First, the entire algorithm to calculate that table has been re-implemented from scratch, replacing the former implementation by Lee Killough found in Boom and MBF. Second, for the same reason mentioned before, the algorithm has been tuned by plums to emphasize the blue tones in the results.
 * The translucency filter initialization routine now properly indicates if the TRANMAP lump has been generated or loaded from a file at startup.

**Transparent Crispy HUD**

 * When the display size is expanded beyond the regular Crispy HUD, the HUD is now rendered as translucent. Of course, this only works if transparency is generally enabled.
 * When the translucent Crispy HUD is enabled, all HU messages will be printed as translucent, too.

**Laser Pointer improvements**

 * The static laser pointer introduced in Crispy Doom 1.1 has been removed. Its implementation has always been considered emberrassing, since it was merely four red pixels hard-coded into the center of the frame buffer.
 * The new laser pointer works like a "real" laser vision spot and is now an actual sprite (i.e. the '+' character of the HU font) that is projected into the game world and shows **exactly** where the next shot will hit. This also means that it will lock itself to a monster sprite if the next shot is going to hit that monster.
 * Also, the ability to change the color of the spot when a monster is targeted has been removed, because laser vision spots really just don't do that.
 * It is not yet decided if the old static laser spot or the color changing feature will ever return in one form or another.

**Further improvements**

 * The per-line "WiggleFix" developed by kb1 and e6y has been applied. In the course of this, Lee Killough's int64 sprtopscreen fix has also been applied.
 * The number of supported savegames has been raised to 8.
 * In the cast shown after beating Doom 2, the monsters can now be rotated using the left/right keys. Furthermore, they can get skipped back and forth by using the strafe left/right keys and turned into gibs by pressing the "Run" key.
 * Colored blood is now enabled on a per-monster basis. That is, if a monster's death sprites have been replaced by a PWAD, this monster's blood will not get colorized anymore. This allows for disabling of colored blood for specific monsters by loading a PWAD that merely needs to include a single lump: BOSSI0 for the Baron of Hell, BOS2I0 for the Hell Knight or HEADG0 for the Cacodemon. Furthermore, Lost souls bleeding Puffs can be disabled by replacing the SKULG0 lump and Spectres bleeding Spectre blood can be disabled by replacing the SARGI0 lump. Colored blood is generally disabled in Chex Quest (where monsters do not bleed, anyway) and Hacx, with the exception of the Thorn Thing in the latter, which now bleeds green blood.
 * Lost Souls spawned by Pain Elementals now also bleed Puffs.
 * In Deathmatch games, frags are now colorized in the status bar. Positive frags are shown in green, negative ones in red and zero flags appear golden.
 * The red palette which is applied when the player is hurt is toned down a bit when the menu is active, so the latter remains legible.

**Further Bug and Compatibility fixes**

 * Since Crispy Doom 1.5, PWADs loaded with the "-file" parameter are treated as if they were loaded with the "-merge" parameter. This has lead to issues with PWADs that contain duplicate lumps once inside the "flats" range and once outside. To overcome this issue, lumps returned by the FlatNumForName() function are now restricted forcefully into the "flats" range (as an exception, this rule does not apply to PWADs which have been merged using one of the NWT-style merging parameters). Please note that this may cause graphical glitches in the flats rendering for savegames that are saved in Crispy Doom and afterwards loaded in Chocolate Doom. These can be avoided by loading the PWAD with the "-merge" parameter in Chocolate Doom as well.
 * The rude extra quit messages containing profanity (feature introduced in Crispy Doom 1.5) have been disabled again. Their reception was mostly negative, and since they did not win the game anything and did not justify another dedicated switch, they had to go. Maybe they were disabled for a reason in the first place...
 * The checks for the NERVE.WAD PWAD file have been simplified. First, the special-casing for this PWAD is not restricted to the Doom 2 IWAD from the Doom 3 BFG Edition anymore but is now also applied if it is loaded alongside the regular Doom 2 IWAD. Second, the special-casing is not restricted to the PWAD being loaded with the "-file" command line parameter anymore but is now applied independent of the way the PWAD has been loaded.
 * The check for the SSG resources has been simplified and does not lead to a crash anymore if a PWAD is loaded that contains the SSG sprites, but not its sounds (e.g. Freedoom's leftover.wad). It may not work when "PC Speaker" is selected for sound effects, though.
 * Text lines that exceed the width of the screen will no longer lead to crashes. Thanks Dragonsbrethren for the bug report.
 * The fix for the common mapping error which clears the ML_TWOSIDED flag when a linedef is missing a second sidedef has been fixed and is now only applied in single player games.
 * The hackish code that checked for the "Run" key being pressed in the menus when selecting to quit the game has been removed. Instead, to speed up the exit sequence, the exit sounds are now omitted if it has been chosen to not show the ENDOOM screen. However, it is still possible to instantly quit from in-game by pressing "Run"+F10.
 * Since Crispy Doom 1.1, the default movement keys were mapped to W, A, S and D and the keys to control the menu have served as a secondary mapping. However, this led to conflicts with specific key setups in which the menu keys were given a different meaning. Therefore, the original key mapping has now been reset and an alternative key set for forward, backward, strafe left and right has been introduced to which the W, A, S, D keys have been mapped. Thanks Average for reporting this issue.
 * Unpausing in the menus during demo recording does not lead to desyncing anymore.
 * The original Doom 2 bug which leads to the sky not changing between episodes has been fixed.
 * The bug which caused flats getting more distorted the closer they are to the right of the screen has been fixed. Thanks to manny for reporting the bug and to bradharding for pointing me to the right code change.
 * The newly allocated memory regions when raising the static limits are now initialized to please Valgrind.
 * Holding the ESC key no longer causes the menu to repeatedly flicker on and off. Thanks joe-ilya for reporting the issue.
 * Upon map initialization, unknown map things (e.g. players 5 to 8 starts) are now ignored and will not lead to crashes anymore.

Crispy Doom 2.0 has merged all changes to the Chocolate Doom master branch up to commit [`76b6d1a239`](https://github.com/chocolate-doom/chocolate-doom/commit/76b6d1a239ddda25a8645ac0e94d45884b71fa5a).

### Changes of Crispy Doom 1.5 from Crispy Doom 1.4

Crispy Doom 1.5 "Supernova" has been released on August 18, 2014.

**Gameplay**

 * Automap improvements:
   * Exit lines are now drawn in white.
   * Corpses are now shown as gray rectangles when the `IDDT` cheat is active.
   * The disappearance of map and grid lines when zooming far out in huge maps has been fixed.

 * A new, unique `SPECHITS` cheat has been added that triggers (by either using, shooting or crossing) all special lines available in the map **at once** -- including lines that require a key to get activated but excluding teleporters and level exits. *While this cheat may appear completely pointless at first, it has proven useful for debugging purposes and allows map designers to "just see what happens if...". Be warned, however, that using this cheat may leave the map in a completely inconsistent state and may cause several overflows and unpredictable behaviour. Furthermore, please note that doors which can be activated from both sides will not move at all, because they will be opened by one `linedef` and closed again by the other one in the same gametic.*
 * The `ID[K]FA` cheats now give the backpack to the player.

 * Optional weapon recoil has been added for all weapons except the fist and the chainsaw, using the recoil thrust values found in MBF and PrBoom+. This feature has to get enabled in *crispy-doom-setup*, it is disabled by default and only generally available in regular single player games.
 * The extra quit messages (**containing profanity!**) found in the source code have been enabled and are now shown with the same probability as the original messages.
 * The weapon sprite now gets centered upon firing.
 * When the "Quick Load" button (`F9`) is pressed before the game has been saved via "Quick Save" (`F6`), the regular "Load Game" menu is now shown and the selected slot is taken as the "Quick Save" slot. Also, the "Quick Save" slot is now cleared once the game is ended via "End Game" in the "Options" menu.

 * If the player exits a map with the Berserker fist active and is equiped with the chaisaw, he will start the next map with the chainsaw instead of the normal fist. This feature is only available in regular single player games.
 * The "power up" sound is now played (but only audible to the `consoleplayer`) each time the Berserker fist is selected.

**Technical**

 * Compatibility improvements:
   * The default amount of RAM that is allocated for the game has been doubled to `32 MB`, the minimal amount has been quadrupled to `16 MB`. 
   * The "Medusa" effect from multi-patch textures has been fixed by lazily replacing (and slightly adapting) `R_DrawColumnInCache()`, `R_GenerateComposite()` and `R_GenerateLookup()` in `src/doom/r_data.c` with Lee Killough's implementations from MBF.
   * The "Tutti Frutti" effect from short textures has been fixed by (again lazily) replacing `R_DrawColumn()` in `src/doom/r_draw.c` with Lee Killough's implementation from MBF. *However, this fix has not yet been applied to the other column drawing functions, namely `R_Draw{Fuzz,Translated,TL}Column[Low]()`.*
   * The `MAXVISSPRITES`, `MAXVISPLANES` and `MAXDRAWSEGS` limits have been entirely removed (*i.e. the formerly static arrays now get resized dynamically*), using no Boom-derived code at all. *However, for performance reasons (mostly sprite ordering in `R_SortVisSprites()`), the `MAXVISSPRITES` limit is currently capped at 4096 (i.e. `32 x` Vanilla `MAXVISSPRITES` limit).*
   * The `MAXSEGS` limit has been raised from `32` to `(SCREENWIDTH/2+1)`, i.e. the same value found in MBF.
   * The `BLOCKMAP` limit has been removed, using code from PrBoom+.
   * Support for extended nodes has been implemented, again using code from PrBoom+.
   * The `MAX_ADJOINING_SECTORS` limit (*formerly up to `20` adjoining sectors*) has been removed, while keeping the one overflow (*i.e. if `h == 21`*) that could possibly get emulated intact -- based on code adapted from PrBoom+.
   * The Vanilla savegame and demo limits are now unconditionally disabled.
   * When entering an unknown special sector, the game will not crash with an error message anymore but merely print a warning.
   * The file size limit (`96 kB`) for MID files inside PWADs has been disabled.
   * Common mapping errors (*e.g. missing `sidedefs`*) are now fixed upon loading the maps, using code adapted from PrBoom+.
   * A [HOM](https://doomwiki.org/wiki/Hall_of_mirrors_effect) is fixed if both `ceilingplane` and `floorplane` are the same visplane (e.g. both are skies).
   * An integer overflow in `SlopeDiv()` has been fixed that formerly lead to an open subsector and thus a giant slime trail in `nuts.wad`.
   All of the above changes make it possible to properly play (and save, and exit, and record and playback demos in) *Fraken*maps like e.g. `nuts.wad`, `arcadia.wad` and all of `NOVA.wad` -- gosh, what broken mess. :p

 * Enemy targets and tracers are now preserved when saving and loading a game. *This is achieved by enumerating all thinker pointers upon saving a game and storing the corresponding indices in the `mobj->target` and `mobj->tracers` fields instead of the actual pointers. When loading a game, after all the thinkers have been restored, the reverse process is applied and all indices in the `mobj->target` and `mobj->tracers` fields are replaced by the corresponding current pointers again. This process is completely Vanilla compatible, as Vanilla will ignore the contents of the `mobj->target` and `mobj->tracers` fields anyway and overwrite them with `NULL` when loading a game. Otherwise, when loading a game saved in Vanilla, the contents of the `mobj->target` and `mobj->tracers` fields will not match any of the indices expected by Crispy Doom and will thus get overwritten with `NULL`, just as in Vanilla.*
 * Games saved in a map with an active cube spitter will not cause a crash anymore when loaded again. *When awakened, the cube spitter counts the number of spawn spots and saves them in the `numbraintargets` variable. However, in Vanilla its value is not recalculated when loading a game. This has been fixed by calling `A_BrainAwake()` again if `numbraintargets == 0`.*

 * Mouse look improvements:
   * Vertical mouse movement has been vastly smoothened by adjusting the scale instead of the mouse delta. *In the former implementation the vertical mouse delta was scaled down by `1/8`, i.e. the lower three bits were discarded. So by moving the mouse really slowly, it was possible to not change `lookdir` at all when the delta on every tic was less than `8`. This has been fixed by moving the division into the rendering and slope calculations and scaling all the constants up by factor 8.* Thanks to clarry of the doomworld forum for this highly appreciated patch!
   * Centering the view with the mouse button assigned to mouse look is now more lenient. Any click with that button that is shorter than `6 tics` will now center the view.
   * The view is now smoothly centered after teleporting.
   * *The `yslope[i]` array is not recalculated anymore in `R_SetupFrame()` for each frame when `lookdir` changes; instead a lookup table is calculated once in `R_ExecuteSetViewSize`.*

 * All PWADs given as arguments to the `-file` parameter will now get merged as if they were passed to the `-merge` parameter.
 * It is now possible to scroll through the menus with the same mouse buttons that are assigned for changing to the previous/next weapon -- which are usually mouse wheel down/up, respectively.
 * If the "Run" key is pressed while the game window is closed (e.g. by clicking on the `X` button in the upper right corner) or during confirmation of the quit message, the game now exits instantly (in addition to the feature introduced in Crispy Doom 1.4).
 * The background now slowly fades out when a menu is activated or the game is paused.

**Additional bug fixes**
 * The `tranmap.dat` file will not get saved in the root directory on Windows systems anymore, instead it will now get saved in the same directory as e.g. `crispy-doom.cfg`. *This was caused by an extra leading path separator in the `tranmap.dat` file name string.* Thanks to plums of the doomworld forums for the bug report.
 * Visual glitches with transparent sprites caused by palette changes have now been fixed. *The cached translucency map in the `tranmap.dat` file is checked for palette changes at startup. However, in the original code derived from MBF, only the first `256` bytes of the palette were compared, whereas the base palette in the `PLAYPAL` lump has `768` bytes. So, changes in later palette indices went completely unnoticed.*
 * The state of the "always run" toggle (introduced in Crispy Doom 1.3) after loading and saving a game has been fixed. *Enabling "always run" makes the `joybspeed` variable greater than the size of the array which holds the button states, which caused an out-of-bounds read in the part of the expression that determines whether `speed` should be `true`.* Thanks to clarry of the doomworld forums for the bug report and the patch!
 * If things are stuck together vertically because of moving sectors, they are now allowed to move further apart. The fix for this bug has been taken from Doom Retro.

Crispy Doom 1.5 has merged all changes to the Chocolate Doom master branch up to commit [`97f1de6649`](https://github.com/chocolate-doom/chocolate-doom/commit/97f1de66497c1071ac138e0c63152267b9d2978f).

### Changes of Crispy Doom 1.4 from Crispy Doom 1.3

Crispy Doom 1.4 has been released on June 12, 2014.

**Gameplay**

 * Players are now allowed to walk over or underneath shootable objects, i.e. monsters and barrels. This needs to be explicitely enabled in *crispy-doom-setup*, but is disabled by default and generally only available in single-player games.
   Some restrictions still apply to this feature:
   * Only the player can walk over or underneath other objects, monsters can not. This prevents multiple monsters from piling up and avoids the need for more complicated measures in the code that would potentially break compatibility.
   * It is only allowed to walk over or underneath shootable objects. Most other objects in Doom have arbitrary heights hard-coded into the engine which do not necessarily match the actual appereance in the game.
   * Melee attacks across differences heights are entirely unaffected by this feature.
 * Automap improvements: 
   * Keyed doors are now drawn in their respective colors.
   * Thus, in order to distinguish red-keyed doors from regular walls (which were formerly drawn in red) and yellow-keyed doors from walls with ceiling level changes (which were formerly drawn in yellow), these are now drawn in darker red and yellow colors, respectively.
   * Furthermore, teleporters (which were formerly drawn in just that darker red color) are now drawn in green. Additionally, WR teleporters (*linedef type #97*) are now also drawn in green if they are not secret.
   * When the `IDDT` cheats are active:
     * Secret sector boundaries are now drawn in purple until they are revealed (*i.e. as long as `sector->special == 9`*).
     * Keys are now shown as crosses in their respective colors.
     * The triangle size now represents the actual thing size.
     ** Countable kills are now shown as red and countable items as yellow triangles.
   * Episode and Map are now explicitely shown in the automap if the map title string has been modified by means of dehacked.
 * HUD improvements:
   * When either the God Mode cheat or the Invincibility powerup are active, Health and Armor values on the status bar are now printed in gray.
   * When the fist is selected and the Berserk Pack is active, its sprite is now drawn into the previously empty Ammo field in the Crispy HUD.

**Technical**

 * Gun flash sprites are now rendered translucent.
 * Holding down the "Run" key while taking a screen shot now takes the picture without the weapon sprite or any other HUD elements.
 * Holding down the "Run" key while choosing "Quit Game" now exits instantly. This actually turns `Shift+F10` into an emergency "Boss Key". ;)
 * The presence of MAP33 is not hard-coded to the `doom2.wad` IWAD file of the BFG Edition anymore. Instead, it is now checked at startup and the level progression (i.e. *MAP02-(secret exit)->MAP33->MAP03*) is adapted accordingly.
 * Once a savegame has been rejected to continue from after player death (by holding down the "Run" key during resurrection, feature introduced in Crispy Doom 1.3) it is not considered anymore until loaded or saved again.
 * The `IDCLEVxy` cheat now eats key presses, i.e. the second digit of the level number is not interpreted as a weapon selection anymore.
 * When using the `-warp` command line parameter for Doom 1, the episode and map numbers do not have to be separated by spaces anymore.
 * The flashing HOM indicator (introduced in Crispy Doom 1.3) has been turned into a command line option `-flashinghom` and now flashes even when the `NOCLIP` cheat is active. Without this option, though, HOMs are still drawn in black but do not flash in red anymore.

Crispy Doom 1.4 and Crispy Doom 1.3 have merged all changes to the Chocolate Doom master branch up to commit [`e0331a0174`](https://github.com/chocolate-doom/chocolate-doom/commit/e0331a01741905ac665adce1da0fb04e86db4b05).

### Changes of Crispy Doom 1.3 from Crispy Doom 1.2

Crispy Doom 1.3 has been released on May 19, 2014.

 * Crispy Doom now has its own icon! It is is composed of the [Chocolate Doom icon](https://www.chocolate-doom.org/wiki/images/7/77/Chocolate-logo.png) and a [photo](https://commons.wikimedia.org/wiki/File:Potato-Chips.jpg) of potato crisps (Utz-brand, grandma's kettle-cooked style) by [Evan-Amos](https://commons.wikimedia.org/wiki/User:Evan-Amos) who kindly released it into the [public domain](https://en.wikipedia.org/wiki/en:public_domain).

**New features**

 * A "quick reverse" key has been added.
 * A key to toggle "always run" has been added, set to `Caps Lock` by default.
 * Keys have been added to "go to the next level" and to "reload the current level", but left unset by default (based on code taken from PrBoom+).
 * Support to invert the mouse on the vertical axis has been added.
 * It is now possible to toggle between Fist and Chainsaw even without the Berserk pack applied, not available in demo recording/playback and netgame mode.
 * Boom's `TNTEM` and `TNTWEAPx` cheats as well as PrBoom+'s/ZDoom's `NOTARGET` cheat have been implemented.
 * The Automap is now updated while playing.
 * The Automap markers are now centerd on the map. 
 * Vertical aiming (introduced in Crispy Doom 1.2) is now optional, since it may have some severe undesired effects on gameplay. It can be enabled in *crispy-doom-setup*, but is disabled by default and in demo recording/playback and netgame mode.
 * The corpse flipping randomization (introduced in Crispy Doom 1.2) has been improved by improving corpse health randomization. *The flipping decision is based on the monster health after death. However, since some weapons do only apply damage in multiples of (multiples of) 2, the distribution was uneven. In Crispy Doom 1.3, moster health is now reduced by `target->tics & 1`, which itself is randomized by `target->tics -= P_Random()&3` before. Thus, a more even distribution is achived across all weapons and monster types.* Furthermore, the Barrel has been added as an exception that should not have its death animation flipped (thanks Doom Retro).
 * The lethal pellet of a point-blank SSG blast may now get an additional damage boost to achieve an occasional gib chance, disabled in demo recording/playback and netgame mode. *This only happens if the target is within melee range (i.e. if `P_CheckMeleeRange(target)` returns `true`) and if `damage >= 10` for the lethal pellet, which roughly corresponds to a 2/3 chance.*
 * If the player dies and the game has been loaded or saved before in the current level, that savegame is now reloaded instead of restarting the entire level from scratch. However, since this behaviour may be undesired, it is possible to suppress it by holding the "Run" key during resurrection.
 * The player view is now centered when the player dies or hits hard ground (the latter only when mouse look is disabled).
 * Existing demo files with the same file name are now saved from getting overwritten by adding a file name suffix. That is, if you run e.g. `crispy-doom -record demo` and a file called `demo.lmp` does already exist, Crispy Doom will now save the new demo as `demo-000.lmp`, or `demo-001.lmp` if that does also exist, etc.
 * Entering menus while recording demos will now pause the game to prevent [instant desyncing](https://doomwiki.org/wiki/Demo_desyncing_caused_by_menu_access).
 * A [HOM](https://doomwiki.org/wiki/Hall_of_mirrors_effect) has been added that flashes between black and red (but remains black if the *noclip* cheat is active).
 * Chocolate Doom 2.0.0 clients are now allowed to connect to Crispy Doom servers.

**Experimental features**

 * The "flipped levels" feature has been ported over (i.e. "stolen") from Strawberry Doom. To try this out, add the `-fliplevels` parameter to the command line.
 * The SSG is now also available in Doom 1 if the required resources are made available. To try this out, run the game via e.g. `crispy-doom -iwad doom.wad -merge doom2.wad doom.wad` and type `IDKFA` or `TNTWEAP9`.

**Other bug fixes**

 * The tutti-frutti effect which appeared around the weapon sprite under certain circumstances has been fixed.
 * A crash has been fixed when an Automap marker reaches the border of the map.
 * A bug that made the status bar and the face visible under certain circumstances when using the *noclip* cheat has been fixed.

**Non-Doom Ports**

 * Due to popular demand, the non-Doom games that are part of Chocolate Doom have now been added back into the release. However, they have not seen any development since the Crispy Doom 1.0 release and are strictly **unsupported**. Feature requests are accepted if accompanied by patches. ;-)

Crispy Doom 1.4 and Crispy Doom 1.3 have merged all changes to the Chocolate Doom master branch up to commit [`e0331a0174`](https://github.com/chocolate-doom/chocolate-doom/commit/e0331a01741905ac665adce1da0fb04e86db4b05).

### Changes of Crispy Doom 1.2 from Crispy Doom 1.1

Crispy Doom 1.2 has been released on April 10, 2014.
It is mostly a bug fix release to fix the jerky mouse look in Crispy Doom 1.1. If you were unsatisfied with the mouse look performance of Crispy Doom 1.1 it is highly recommended to upgrade to Crispy Doom 1.2:

 * Unjerkify mouse look.
   * In Crispy Doom 1.1 vertical mouse movement changed the `look` variable which decided how the actual `player->lookdir` variable will be changed in the next game tic. However, the `look` variable is intended for keyboard use and changes the lookdir in rather coarse steps. The code has been changed to now act directly on the `player->lookdir` variable which provides for a much smoother mouse look.
   * Mouse sensitivity selection is moved into a separate sub-menu of the Options menu.
   * Allow to set separate values for horizontal and vertical mouse sensitivity in both the Options menu and in *crispy-doom-setup*.
   * Increase mouse sensitivity thermometer range up to 20.
   * Fix a crash in the Options menu when mouse sensitivity exceeds the maximum value. Instead, allow to exceed the thermometer range and print values next to it.
   * Fix slopes for bullets and missiles that would not have hit a target anyway. This means that missed shots will now go into the actual vertical direction looked into. However, auto-aim will still work and guide shots to their aim. This feature is only enabled in single-player games (though probably harmless).

 * Monster death sprites and corpses are now flipped randomly. While the idea is stolen from Doom Retro, the implementation is completely different. The flipping decision is based on the value of `thing->health`, which is randomized in Doom, since all damage done by weapons is already randomized. Once a monster is dead (i.e. `thing->flags & MF_CORPSE`), this value remains constant. Although probably harmless, this feature is also only available in single-player games.
 * Fix different view frames for normal fullscreen mode and Crispy HUD.
 * Fix a crash when attempting to test settings from *crispy-doom-setup*, merged from Chocolate Doom.
 * Fix "fast doors make two closing sounds" and "fast doors reopening with wrong sound" engine bugs.

Crispy Doom 1.2 has merged all changes to the Chocolate Doom master branch up to commit [`a1b066a0eb`](https://github.com/chocolate-doom/chocolate-doom/commit/a1b066a0eb0d2cf5d17ad68ca833fcdd21f80725).

### Changes of Crispy Doom 1.1 from Crispy Doom 1.0

Crispy Doom 1.1 has been released on April 5, 2014.

**Visual Enhancements**

 * **Minimal HUD:** When the view size is increased one step beyond fullscreen, a minimal HUD is displayed which only shows the numbers of the status bar without its background.
 * **Colored Patches and Sprites:** Color translation lookup tables have been ported over from Boom 2.02. They can be used to (optionally) colorize the numbers on the status bar, the HUD font and also the blood of certain monsters.
   * **Colored Status Bar Number:** The numbers displayed in the status bar or the minimal HUD can get colorized depending on their value. If enabled, health is colored red, yellow, green or blue for values up to 25%, 50%, 100% and above 100% respectively. Ammo is colored red, yellow and green for values up to 25%, 50%, 100% of the *initial* maximum ammo, respectively. Excess ammo, that can only be carried when a backpack has been found, gets colored blue. Armor is colored green or blue depending on the type of armor the player currently wears, red means no armor. Colored numbers in the status bar can be enabled in the *crispy-doom-setup* tool and are disabled by default.
   * **Colored Blood:** Blood is now colored depending on the monster class. Spectres and invisible players will now bleed spectre blood, Lost Souls will now bleed Puffs (*since spawning Puffs demands the RNG differently than spawning Blood, this feature is only enabled in single player games*), Cacodemons bleed blue blood and Hell Knights and Barons of Hell bleed green blood. *This feature required pointing the `target` field of the blood `mobj_t struct` to the monster `mobj_t struct` and furthermore the addition of another field to the `vissprite_t struct` to keep track of the object that a sprite belongs to. This is considered [harmless](https://www.doomworld.com/vb/post/1254800), though).
 * **Translucency:** Translucency support has been ported over from Boom 2.02. As in Boom, the nececssary color tinting lookup table is created once at startup and then cached on the hard disk for subsequent uses, alternatively it can be supplied as a lump or in a PWAD file. The same objects as in Boom 2.02 have been tagged as translucent in `mobjinfo[]`. Additionally, rocket and barrel explosions, Lost Soul and Pain Elemental explosions, and the explosions of the Boss Brain (Icon of Sin) have been tagged translucent. Also, if a monster gets resurrected by an Arch Vile from a pool of blood and turns into a "[ghost monster](https://doomwiki.org/wiki/Ghost_monster)" it will rise from the dead as translucent. Translucency can generally be enabled in the *crispy-doom-setup* tool and is disabled by default.
 * **Shaded Menus:** The background is shaded when a menu is active or when the game is paused. Furthermore, menu entries that currently make no sense, e.g. *Save Game* or *End Game* when no game is active, are shaded. Also, empty savegame slots are shaded in the *Load Game* menu.

**Enhancements affecting gameplay**

 * **Free Look:** Free look support has been ported over from Chocolate Hexen. It is now possible to look up and down up to a certain degree using the keyboard or mouse. Using the latter, it is possible to change the viewing angle by pressing a certain key and moving the mouse vertically. Single-clicking that key without moving the mouse will center the view again. It is furthermore possible to activate permanent mouse look. Mouse viewing may feel a bit jerky, because the view angle is only increased in integers. To enable mouse viewing without also enabling vertical mouse movement, the `-novert` command line parameter had to be modified to only apply to vertical *translational* movement. Since both free look and permanent mouse look require stretching the sky texture, they need to get enabled in the *crispy-doom-setup* tool and are disabled by default.
 * **Jumping:** Jumping support has been ported over from Chocolate Hexen. It needs to get enabled in the *crispy-doom-setup* tool and is of course disabled by default. Since jumping involves in-air movement which may cause demos or network games to desync, this feature is entirely disabled when not in a single player game.
   To achieve the above two features without sacrificing savegame compatibility with Chocolate Doom, a new `player2_t struct` has been introduced into the code to hold the additional player-speficic variables. This means that viewing angle and jumping state are not saved in savegames.*
 * **Autorun:** When the autorun feature is active, using the "run" key will result in walking at normal speed.
 * **Laser Pointer:** A red laser pointer dot can be shown in the center of the screen to help for aiming. Optionally, the color of the dot can change from red to yellow if a target is within reach. Both options need to be enabled in the *crispy-doom-setup* tool and are disabled by default.
 * **Secret Message:** A centered "A secret is revealed!" message is printed in a golden font and a sound is played when a secret is found. This can be enabled in the *crispy-doom-setup* tool and is disabled by default.
 * **Automap stats:** Additional level statistics can be shown in the automap, including the number of kills, items and secrets as well as the level time. This feature needs to be enabled in the *crispy-doom-setup* tool and is disabled by default.
 * **Ammo Bob:** Ammo or weapons released by killed enemies will slightly bob vertically (*this feature is only available in a single player game*).
 * **Engine Bugs:** The "Ouch Face" and the "Picked up a Medikit that you really need" messages are shown as intended.

**Technical Enhancements**

 * **Vertical Mouse Movement:** Separate values for mouse acceleration and threshold can now be applied for the vertical axis in the *crispy-doom-setup* tool.
 * **PNG Screenshots:** Screenshots can now be taken in PNG format. This feature has been merged from Chocolate Doom and is now the default in Crispy Doom. Also, a dedicated key can be set for taking screenshots, including the "Print Screen" key.
 * **Automatic loading of DEHACKED lumps:** Chocolate Doom has got the ability to load DEHACKED lumps embedded in PWAD files via the `-dehlump` command line parameter. This feature has been merged into Crispy Doom and is now the default. It can be disabled, though, via the `-nodehlump` or `-nodeh` command line parameters. The latter does additionally disable the special treatment of the `chex.wad` and `hacx.wad` IWAD files and the `nerve.wad` PWAD file. Furthermore, error handling has been made more tolerant for embedded and automatically loaded DEHACKED lumps: If errors are detected, error messages are printed, but the game does not abort.
 * **Wolf SS:** Although all resources of the Wolf SS enemy have been entirely removed from the BFG Edition `doom2.wad` IWAD file, there is still one present in the included [MAP33](https://doomwiki.org/wiki/MAP33:_Betray_(Xbox_Doom_II)). Therefore, all Wolf SS monsters are replaced with Former Humans in single player games when the BFG Edition `doom2.wad` IWAD is in use.
 * **No-clip Cheats:** Both `idspispopd` and `idclip` cheats are allowed in both Doom and Doom II.
 * **Par Times:** Par times for Episode 4 of The Ultimate Doom have been taken over from the BFG Edition. Also, the par time for MAP33 of the Doom II BFG Edition now reads "SUCKS!" as in the Xbox Doom II version. Both were added in a manner that does not interfere with `statdump`'s output.
 * **Sensible Defaults:** Some default configuration values have been changed. For example, the ENDOOM screen is not displayed, the `vanilla_savegame_limit` and `vanilla_demo_limit` size limits are disabled, autorun is enabled and the game is started in full width (`screenblocks = 10`) by default.
 * **Default Keys:** The default keys for forward, backward, strafe left and right have been changed to **W**, **S**, **A** and **D**, respectively. However, it is still possible to control the game with the cursor keys.

**Bug Fixes**

 * The weapon sprite was one pixel too high when the player was idle: https://github.com/fabiangreffrath/crispy-doom/issues/1 .
 * The "No rule to make target crispy-server.6, needed by all-am" Make error was fixed: https://github.com/fabiangreffrath/crispy-doom/issues/2 .

**Crispy-Doom-Setup**

Crispy Doom now comes with its own dedicated setup tool called *crispy-doom-setup*. Instead of `chocolate-doom-setup`'s "Compatibility" section it has one called "Crispness" which allows to selectively activate the following features:
 * Enable translucency
 * Show colored numbers in status bar
 * Show level stats in automap
 * Show secrets revealed message
 * Show laser pointer
 * Change laser pointer color on target
 * Enable jumping [*]
 * Enable free look [*]
 * Enable permanent mouse look

   The items marked with an asterisk require the setting of additional keys in the "Keyboard" or "Mouse" sections, respectively.

**Non-Doom Ports**

The non-Doom games that are part of Chocolate Doom have fallen behind in terms of development and haven't seen any significant changes since Crispy Doom 1.0. Thus, only Doom is included in the 1.1 release and will be from now on.

Crispy Doom 1.1 has merged all changes to the Chocolate Doom master branch up to commit [`a80aa343a5`](https://github.com/chocolate-doom/chocolate-doom/commit/a80aa343a53c3dd3db2434a52d70de72ec4df955).

### Changes of Crispy Doom 1.0 from Chocolate Doom 2.0.0

Crispy Doom 1.0 has been released on March 10, 2014.

**Display Resolution**

 * The display resolution in all four games (Doom, Heretic, Hexen and Strife) has been doubled in both horizontal and vertical direction, resulting in a 640x400 native resolution.
 * Additionally, in Crispy Doom, the *Graphic Detail: Low* mode has been modified to halve the display resolution also in the vertical direction (unlike Vanilla and Chocolate Doom, which only halve the resolution in the horizontal direction in this mode). As a result, in Crispy Doom in *Graphic Resolution: Low* mode the screen is rendered **exactly** identical to Chocolate Doom in *Graphic Resolution: High* mode.
 * An additional *mode_squash_1p5x* video mode has been introduced which stretches the screen by factor 1.25 horizontally and by factor 1.5 vertically. This results in a 800x600 display resolution to provide for a intermediate resolution suitable for vintage monitors.

Due to the increased display resolution, the supported video modes differ from Chocolate Doom. The following table gives an overview over the available video modes in Crispy Doom 1.0 and Chocolate Doom 2.0.0. A struck through video mode has been disabled in fullscreen mode for quality concerns. Additionally, all video modes that would result in a display resolution exceeding [WUXGA](https://en.wikipedia.org/wiki/Graphics_display_resolution#WUXGA_(1920%C3%971200)) (1920x1200) have been disabled in Crispy Doom.

Video Mode | Chocolate Doom 2.0.0 | Crispy Doom 1.0
:---|---:|---:
mode_scale_1x    | **320x200**     | **640x400**
mode_scale_2x    | **640x400**     | **1280x800**
mode_scale_3x    | **960x600**     | **1920x1200**
mode_scale_4x    | **1280x800**    | ~~**2560x1600**~~
mode_scale_5x    | **1600x1000**   | ~~**3200x2000**~~
mode_stretch_1x  | ~~**320x240**~~ | **640x480**
mode_stretch_2x  | **640x480**     | **1280x960**
mode_stretch_3x  | **960x720**     | ~~**1920x1440**~~
mode_stretch_4x  | **1280x960**    | ~~**2560x1920**~~
mode_stretch_5x  | **1600x1200**   | ~~**3200x2400**~~
mode_squash_1x   | ~~**256x200**~~ | ~~**512x400**~~
mode_squash_1p5x |                 | **800x600**
mode_squash_2x   | ~~**512x400**~~ | **1024x800**
mode_squash_3x   | **800x600**     | **1600x1200**
mode_squash_4x   | **1024x800**    | ~~**2048x1600**~~
mode_squash_5x   | **1280x1000**   | ~~**2560x2000**~~

**Raised Limits**

The static engine limits in all four games have been raised in line with [Doom+](http://prboom-plus.sourceforge.net/doom-plus.features.html). For the four included games this results in the following limits:

Limit | Chocolate Doom 2.0.0 | Chocolate Heretic 2.0.0 | Chocolate Hexen 2.0.0 | Chocolate Strife 2.0.0 | Crispy Doom 1.0 | Crispy Heretic 1.0 | Crispy Hexen 1.0 | Crispy Strife 1.0
:---|---:|---:|---:|---:|---:|---:|---:|---:
MAXVISPLANES  | 128    | 128    | 160    | 200    | 128\*8    | 128\*8     | 160\*8    | 200\*8
MAXVISSPRITES | 128    | 128    | 192    | 128    | 128\*8    | 128\*8     | 192\*8    | 128\*8
MAXDRAWSEGS   | 256    | 256    | 256    | 256    | 256\*8    | 256\*8     | 256\*8    | 256\*8
MAXPLATS      | 30     | 30     | 30     | 30     | 30\*256   | 30\*256    | 30\*256   | 30\*256
SAVEGAMESIZE  |        | 196608 |        |        |          | 196608\*16 |          |   
MAXLINEANIMS  | 64     | 64     | 64     | 96     | 64\*256   | 64\*256    | 64\*256   | 96\*256
MAXOPENINGS   | 320\*64 | 320\*64 | 320\*64 | 320\*64 | 640\*64\*4 | 640\*64\*4  | 640\*64\*4 | 640\*64\*4

**Improved Support for the BFG Edition**

Chocolade Doom has no support for *No Rest for the Living*, because its maps require a limit-removing source port. However, with its raised static limits there is no more reason to not add full support for this expansion to Crispy Doom.

If the IWAD loaded with Crispy Doom is recognized as `doom2.wad` shipped with the Doom 3: BFG Edition, the following changes apply:
 * The maximum number of levels (`NUMCMAPS`) is raised to 33, the secret exit in MAP02 leads to MAP33 and the exit there leads back to MAP03.

If furthermore the `nerve.wad` PWAD file can be found and no other PWAD files are loaded, the following additional changes apply:
 * The `nerve.wad` PWAD file is automatically loaded at startup, the level name patch lumps (`CWILVxx`) are renamed to not collide with regular Doom 2 ones.
 * An additional submenu as added to the New Game menu that allows for selection of either Hell on Earth (i.e. regular Doom 2) of No Rest For The Living.

If otherwise the nerve.wad PWAD file is loaded via the `-file nerve.wad` parameter, the following additional changes apply:
 * The name of the expansion is shown in the window title and the game banner.
 * The `TITLEPIC` lump is replaced with the `DMENUPIC` lump.

If either "No Rest For The Living" is selected from the New Game menu or if the `nerve.wad` PWAD file is loaded via the `-file nerve.wad` parameter, the following changes apply:
 * The level names in the automap and on the interlevel screen are adjusted.
 * The level music and par times are adjusted.
 * The secret exit in MAP04 leads to MAP09 and the exit there leads back to MAP05.
 * It is not possible to enter levels after MAP09 via *IDCLEVxx* cheat.
 * There is no "*Now entering...*" screen shown after level 8.
 * The intermission screen after level 6 is replaced by another one after level 8 with adjusted text.
 * The Doom 2 cast sequence is shown after level 8.

Please note that all of the above changes do **only** apply for the `doom2.wad` IWAD file from the BFG Edition; they do **not** apply for the regular `doom2.wad` IWAD file. Consequently, it is still possible to play the `nerve.wad` PWAD file as a regular PWAD without special treatment if loaded alongside the regular `doom2.wad` IWAD file.

Crispy Doom 1.0 is based on Chocolate Doom 2.0.0 with merged changes from the GIT master branch up to commit [`ecf457ddcb`](https://github.com/chocolate-doom/chocolate-doom/commit/ecf457ddcbf481b451f37474b057a06e7d843b66).
