**New Features and Improvements**
* Make MIDI device selection less ambiguous.
* Add default difficulty option (by kiwaph and @mikeday0).
* Improve smoothness of network games when running uncapped (thanks @rfomin).
* Disable smooth pixel scaling if software rendering is enforced.
* Add framerate limiting option (@mikeday0 and @rfomin).
* Updates of Native MIDI on Windows from Chocolate Doom (@ceski-1 and @rfomin)
  * SysEx messages.
  * Proper device reset (winmm_reset_type and winmm_reset_delay config options).
  * Loop points (Final Fantasy and RPG Maker).
  * Full support of EMIDI.
* All music formats now work when the OPL backend is selected (@rfomin).
* Implement demo footer for Doom and Heretic (@rfomin).

**Bug Fixes**
* Fix crosshair shifting position after changing resolution.
* Fix Automap line jitter in low res mode (@mikeday0).
* Widescreen rendering now requires aspect ratio correction.
* Fix widgets obscuring chat line (@JNechaevsky).
* Raven: Disable autoloading in shareware mode (thanks @rzhxiii and @JNechaevsky).
* Raven: Don't scroll map background when paused (@mikeday0).
* Fix glitchy drawing of instant moving sectors when running uncapped (@mikeday0).
* Fix bogus linedef numbers in P_LoadSegs warning (thanks @tpoppins).
* Hexen: Don't interpolate 180 degree polyobject rotations (@mikeday0).
* Hexen: Don't interpolate polyobjects when loading game (@mikeday0).

**Crispy Doom**
* Add support for loading REKKR as IWAD (@SoDOOManiac).
* Do not show the "WAD: map" Automap widget for IWAD levels.
* Improve brightmap for COMPUTE1 texture (@JNechaevsky).
* Allow L and R arrow keys in Crispness menu (@mikeday0).
* Add weapon bobbing interpolation from Woof! (@rrPKrr).
* Restore colored blood options from previous Crispy Doom versions (@rrPKrr).
* Add known hashes for Sigil music tracks (@SirYodaJedi).
* Colored blood setting to change in both directions (@SoDOOManiac).
* Minor HUD fixes (@SoDOOManiac).

**Crispy Heretic**
* Support 8 pages of saves (@mikeday0).
* Colorize Crispness menu (@mikeday0).
* Interpolate weapon bobbing (@mikeday0).
* Make Crispness menu behavior consistent with Doom (@mikeday0).
* Add Mono SFX option (@mikeday0).
* Add Sound Channels option (@mikeday0).
* Add Player Bob and Weapon Alignment options (@mikeday0).
* Fix long wall wobble (@mikeday0).
* Add WiggleFix (@mikeday0).
* Improve slime trail removal (@mikeday0).
* Fixed "Look forward" button not working properly while recording a demo (by @Dasperal).
* Fixed bug with minimal vertical movement of the mouse doesn't affect vertical look (by @Dasperal).
* DeePBSP and ZDBSP nodes support from Crispy Doom (@rfomin).
* Introduce a HEHACKED lump for embedded HHE files (@rfomin).
* Add support for ANIMATED lumps and swirling flats (@SilverMiner).

**Crispy Hexen**
* Generate default save slot name.
* Allowing deletion of savegames from menu (@mikeday0).
* Support 8 pages of saves (@mikeday0).
* Colorize Crispness menu (@mikeday0).
* Interpolate weapon bobbing (@mikeday0).
* Add SHOWFPS cheat (@mikeday0).
* Make Crispness menu behavior consistent with Doom (@mikeday0).
* Add player coordinates widget (@mikeday0).
* Add Mono SFX option (@mikeday0).
* Add Sound Channels option (@mikeday0).
* Add Player Bob and Weapon Alignment options (@mikeday0).
* Fix long wall wobble (@mikeday0).
* Add WiggleFix (@mikeday0).
* Fixed "Look forward" button not working properly while recording a demo (by @Dasperal).
* Fixed bug with minimal vertical movement of the mouse doesn't affect vertical look (by @Dasperal).

Crispy Strife has been made possible by porting Crispy Doom features developed by @fabiangreffrath and contributors including @mikeday0, @JNechaevsky, @rrPKrr, and @AlexMax. The fullscreen HUD is a modified version of the original HUD/statusbar and is influenced by Strife: Veteran Edition. The ambient sound fix was originally developed by @haleyjd for SVE.

**Crispy Strife** (@ceski-1)
* Add Crispness menu
* Expose high resolution mode
* Add widescreen support 
* Add uncapped framerate
* Add framerate limit
* Add vertical sync option
* Add smooth pixel scaling option
* Add view/weapon bobbing options
* Add attack alignment options
* Add default difficulty option
* Add fast exit (Shift + F10)
* Add Crispy automap features:
    * Interpolation
    * Smooth lines
    * Overlay mode
    * Rotation mode
    * Fix overflows
    * Mouse control
* Show exit screen only if showing ENDOOM (ENDSTRF)
* Add `showfps` cheat
* Add show player coords. option
* Add mono SFX option
* Add sound channels option
* Fix flat distortion
* Fix long wall wobble
* Add WiggleFix
* Upgrade renderer to 32-bit integer math
* Add slime trail fix
* Fix Tutti-Frutti effect
* Add smooth diminishing lighting option
* Draw missing lines for translucency effect
* Add always run toggle
* Add vertical mouse movement toggle
* Add "Run" centers view option (i.e. run key can be separated from view centering)
* Add center view key
* Add mouse sensitivity options
* Add freelook options (lock or spring)
* Add mouselook features (single-player only):
    * Permanent mouselook option
    * Hold button to mouselook or click to center view
    * Invert y-axis option
    * Don't auto-center view when landing with mouselook enabled
* Add crosshair options:
    * Static crosshair
    * Color indicates health
* Show subtitles by default
* Add mouse buttons for inventory
* Support wide status bars with zero offset
* Support widescreen graphics mods
* Add fullscreen HUD (modified version of original HUD/statusbar and SVE HUD)
* Fix mouse fire delay
* Add support for PWAD autoload directories
* Add clean screenshot key
* Add misc. sound fixes
    * Play ambient sounds consistently (@haleyjd for SVE)
    * Play door sounds correctly during various open/close states
    * Door "use" sound originates from player
* Add play sounds in full length option
* Preserve targets/tracers in saved games
* Preserve monster skill when loading a game

