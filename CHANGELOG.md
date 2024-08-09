***This is a major release built upon on the fresh Chocolate Doom 3.1.0 code base. Its main new feature is the addition of True color rendering support as a compile-time option for all four games. Thanks to all contributors!***

**General Fixes and Improvements**
* Don't apply framecap when using `-timedemo` (@JNechaevsky).
* Make demo footer compatible with PrBoom+/DSDA-Doom demo autoplay (@rfomin).
* Fix visual distortion of drawing planes near the screen edges in wide screen modes (@rfomin and @JNechaevsky).
* Support savegame loading for PWADs with spaces in their name.
* Aspect ratio now matches screen by default (@SoDOOManiac).
* Only update sounds once per gametic (@rfomin).
* Improve green color translation (@fabiangreffrath and @JNechaevsky).
* Always draw borders of fullscreen patches using black found in PLAYPAL.
* Add support for negative gamma levels (@JNechaevsky).
* Apply palette changes every gametic, not every frame (@JNechaevsky).
* Apply brightmaps to translucent and translated columns (@JNechaevsky).
* Fix bug where non-midi music would play at max volume (@mikeday0).
* Make max volume `libsamplerate` output the default (@mikeday0).
* Fix memory leak on light tables recalculation (@JNechaevsky).
* Introduce fast mouse polling. Reduces input lag when running uncapped
  (@mikeday0).
* Analog gamepad controls are enabled by default. (@mikeday0)

**Crispy Doom**
* Improve display of secret sector coloring in automap when custom PLAYPALs are
  used (@SoDOOManiac).
* Use PWAD-provided color translation tables for colored blood.
* "Status Bar" level stats are now left-aligned with HUD (@SoDOOManiac).
* Fix visual glitch on door in Doom 2 MAP19 when running uncapped (@rfomin and
  @JNechaevsky).
* Correctly report complevel 3 in demo footer (@rfomin).
* Armor bonus brightmap improvement (@JNechaevsky).
* Improve detection of improper patches.
* Only stretch short skies.
* Fix bug where reviving with IDDQD can possibly leave player in a "zombie"
  state (@tomas7770).
* Add reload, next level warp keys support and IDCLEV warp for demo playback
  (@JNechaevsky).
* Enter key in Crispness menu behaves same as other games (@mikeday0).
* Add support for `-coop_spawns` parameter (@JNechaevsky).
* Allow non-power-of-2 wide sky textures.
* Add support for dedicated No Rest for the Living music tracks.
* Fix automap marks disappearing near left side of screen (@JNechaevsky).
* Add support for Sigil II (@mikeday0).
* Apply sideloading lump renaming to associated autoloaded WADs (@mikeday0).
* Give Master Levels its own intermission picture MASTRINT (@mikeday0).
* "Widescreen Aspect Ratio" -> "Aspect Ratio" in Crispness menu (@SoDOOManiac).
* Improved True color code for better compatibility of drawing graphical patches
  when PLAYPAL and COLORMAP lumps have different color indexes.
* Resolution-tuned weapon sprite "1 pixel too high" fix for compatibility with bit-shifting hires forks (@SoDOOManiac).
* Small optimizations on toggling gamma-correction levels in True color mode
  (@JNechaevsky).
* Fix blinking of drawing Tower of Babel on intermission screen (@JNechaevsky).
* Show date and time of savegame in Save/Load menu (@fabiangreffrath and @JNechaevsky).
* Suppress pause when a new game is started (@fabiangreffrath).
* Fix sector's sound origin for large levels (@kitchen-ace and @rfomin).

**Crispy Heretic**
* Add support for MBF sky transfers (@SilverMiner).
* Display correct active artifact when loading game or entering a new level
  (@mikeday0).
* Add support for SWITCHES lump (@SilverMiner).
* Add support for True color rendering as compile-time option (@fabiangreffrath
  and @JNechaevsky).
* Fix missing one pixel line at top and bottom of screen in transparent
  drawing functions (@JNechaevsky).
* Add support for `-coop_spawns` parameter (@JNechaevsky).
* Fix scrolling floor textures bleeding through static floor textures
  (@JNechaevsky).
* Unknown wall textures, map things and sector specials are no longer fatal
  (@JNechaevsky).
* Removed MAXBUTTONS vanilla limit (@JNechaevsky)
* Apply brightmaps to D'Sparil teleportation frames (@JNechaevsky).
* Improve load times for complex levels (@JNechaevsky).
* Add support for `-blockmap` parameter (@JNechaevsky).
* Create blockmap if WAD-provided blockmap is bad (@JNechaevsky).
* Fix hitscan/missile puffs disappearing in certain areas (@JNechaevsky).
* Fix crash if save is attempted when game has not started (@JNechaevsky).
* Allow for flat scrolling in all cardinal directions (@kitchen-ace).
* Add support for demo fast-forward (@kitchen-ace).
* Fix medusa effect (@mikeday0).
* Improve brightmap for serpent torch (@kitchen-ace).
* Allow multiple jumps over menu items with same first letters (@JNechaevsky).
* Show date and time of savegame in Save/Load menu (@fabiangreffrath and @JNechaevsky).
* Suppress pause when a new game is started (@fabiangreffrath).

**Crispy Hexen**
* Fix animation glitches seen on swiping weapons when Weapon Attack Alignment is
  enabled. (@mikeday0).
* Display correct active artifact when loading game or entering a new level
  (@mikeday0).
* Fix crash if save is attempted when game has not started (@JNechaevsky).
* Fix desync when running uncapped with polyobjects in level (@mikeday0).
* Add support for demo fast-forward (@kitchen-ace).
* Add support for True color rendering as compile-time option (@fabiangreffrath
  and @JNechaevsky).
* Fix medusa effect (@mikeday0).
* Fix wall rendering visual glitch due to overflow (@JNechaevsky).
* Fix teleportation visual glitches when running uncapped (@JNechaevsky).
* Improve line drawing in automap overlay mode (@JNechaevsky).
* Apply brightmaps to Twined Torch, Skull with Flame and Brazier (@JNechaevsky).
* Interpolated sky scrolling in uncapped frame rate (@JNechaevsky).
* Allow multiple jumps over menu items with same first letters (@JNechaevsky).
* Show date and time of savegame in Save/Load menu (@fabiangreffrath and @JNechaevsky).
* Fix missing one pixel line at top and bottom of screen in transparent
  drawing functions (@JNechaevsky).

**Crispy Strife**
* Add flipped weapon sprites (@ceski-1).
* Misc Sound Fixes: Restore assault rifle PC speaker sound (@ceski-1).
* Fix automap marks disappearing near left side of screen (@JNechaevsky).
* Add support for True color rendering as compile-time option (@fabiangreffrath
  and @JNechaevsky).
* Show date and time of savegame in Save/Load menu (@fabiangreffrath and @JNechaevsky).

