Crispy Doom provides many optional features. Most of them are disabled by default, but can get enabled in the Crispness menu.

## Rendering

These options affect the rendering of the internal frame buffer to the game window.

* High Resolution Rendering

  Increase the internal frame buffer's rendering resolution from Doom's original 320x200 to 640x400 pixels (default: on).

* Widescreen Rendering

  Extend the rendered game scene horizontally to match the screen's aspect ratio (default: off).

  This feature is only available if "Aspect Ratio Correction" is enabled in the setup tool. Without preserving an aspect ratio, the window could just be resized arbitrarily wide.

* Uncapped Framerate

  Update the frame buffer to the game window as soon as possible and interpolate between frames, instead of updating at Doom's original fixed rate of 35 FPS (default: off).

* Framerate Limit

  When Uncapped Framerate is on, limit the framerate to the value specified here. Press Enter to type in a number directly. A value of "0" corresponds to no frame limit (default: none).

* Enable VSync

  Synchronize the updating of the frame buffer with the screen's own vertical refresh rate (default: on).

* Smooth Pixel Scaling

  Upscale the frame buffer to the game window's resolution by means of linear interpolation (default: on).

## Visual

These features change the visual appearance of the game.

* Colorize HUD Elements

  * Status Bar: Colorize Status Bar widgets (i.e. Ammo, Health and Armor) by their respective value, from blue (> 100%) over green (normal) and gold (< 50%) to red (critical).
  * HUD Texts: Highlight specific parts of HUD messages, e.g. when trying to open a key-locked door, by their respective color. Furthermore, colorize widgets like the map name on the Automap, or level statistics and player coordinates if enabled.
  * Both.
  * Off: Disabled (default).
  
* Enable Translucency

  * Projectiles: Render certain projectiles, e.g. plasma balls and monsters' fireballs, translucently.
  * Items: Render certain items, e.g. power-up spheres and teleport fog, translucently.
  * Both.
  * Off: Disabled (default).
  
* Smooth Diminishing Lighting
  
  Apply smoother diminishing lighting of the game scene with increasing distance from the player by using 32 light levels instead of 16 (default: off).

* Apply Brightmaps to

  * Walls: Highlight certain parts of wall textures and ceiling/floor flats that resemble light sources, e.g. switches and computer panels.
  * Items: Highlight certain parts of pick-up and weapon sprites as well as explosive barrels that resemble light sources, e.g. status lights on weapons and ammo.
  * Both.
  * None: Disabled (default).
  
* Colored Blood and Corpses

  Colorize the blood sprites and the sprites of crushed corpses for certain monsters (e.g. green for Barons of Hell and Hell Knights, blue for Cacodemons). Furthermore, render Spectres' blood sprites with the invisibility blur and turn Lost Souls' blood into puffs.

  * None: Disabled (default).
  * Blood: Colorize blood for Barons, Hell Knights and Cacodemons.
  * All: In addition, make Spectres' blood invisible and turn Lost Souls' blood into puffs.

* Randomly Mirrored Corpses

  Randomly flip death animations and corpse sprites for certain monsters as well as decorative marine corpses (default: off). 

## Audible

These options change how sound effects are played back.

* Play Sounds in Full Length

  Prevent one sound effect cutting off the previous one, e.g the chainsaw's idle sound interrupting its own start-up sound. Also, continue playing a sound effect even if the corresponding thing was removed from the map, e.g. an exploding barrel or a rocket impacting on a wall (default: off).

* Misc. Sound Fixes

  Miscellaneous small fixes to sound quirks considered to be bugs in Vanilla Doom, e.g. fast doors making two closing sounds (default: on).

* Sound Channels

  The number of sound effects that can be played back simultaneously.

  * 8 (vanilla, default)
  * 16
  * 32

* Mono SFX

  Play all sound effects in mono, i.e. without stereo separation (default: off). While this may limit orientation and enemy localization, it reportedly helps players who quickly feel nauseous.

## Navigational

These features help with navigation within the maps.

* Extended Automap Colors

  Apply a more diverse and convenient color scheme to the Automap, e.g. doors and switches locked by keys are drawn in their respective colors, teleports are drawn in green and unrevealed secret sectors are drawn in purple (default: on).
  
* Show Level Stats

  Show Kills (K), Items (I) and Secrets (S) statistics in the top left corner of the screen (current/total).
  
  * Always.
  * In Automap.
  * Never (default).
  
* Level Stats Format

  * Ratio: Show stats as x/y, where x is the current number achieved and y is the total (default).
  * Remaining: Show the number remaining, counting down to 0.
  * Percent: Show percentage completion, similar to the stats screen at the end of the map.
  * Boolean: Show "Yes" if you've found/killed everything for that particular stat, or "No" otherwise.

* Show Level Time

  Show the time that has ticked away since the level started in the top left corner of the screen (MM:SS).
  
  * Always.
  * In Automap.
  * Never (default).
  
* Show Player Coords

  Show the player's X and Y coordinates and the horizontal view angle (A) in the top right corner of the screen.

  * In Automap.
  * Never (default).

  There is no "Always" option for this feature as this is considered to be cheating during demo recordings.

* Show Revealed Secrets

  Show a centered golden message and play a sound effect whenever a secret is revealed.
  
  * On: Show a plain "A secret is revealed!" message.
  * Count: Show a "Secret M of N revealed!" message, where N is the total number of secrets in the map.
  * Off (default).

## Tactical

These features provide some tactical advantage by adding to the realism of the game.

* Allow Free Look

  Allow looking up and down either by pushing keyboard keys (usually bound to the <kbd>PgDn</kbd> and <kbd>Del</kbd> keys, as in Heretic) or by vertical mouse movement while holding a dedicated mouse button.

  * Spring: Center the view angle once the key or mouse button is released.
  * Lock: Keep the view angle, even after the key or mouse button is released.
  * Off (default).

  **Enabling this feature will cause the sky texture to become vertically stretched.**
  
* Permanent Mouse Look

  Allow looking up and down by vertical mouse movement without the need to hold a dedicated key or mouse button (default: off).

  **Enabling this feature will cause the sky texture to become vertically stretched.**

* Player View/Weapon Bobbing

  Set the player view and weapon sprite bobbing amplitude.

  * Full: Set player view and weapon sprite bobbing to full amplitude, as in vanilla Doom (default).
  * 75%: Reduce player view and weapon sprite bobbing amplitude to 75%, as in the Doom Classic engine of the Doom 3: BFG Edition.
  * Off: Disable player view and weapon sprite bobbing. Weapon sprites remain centered, as in e.g. Wolfenstein 3-D. 

  **Obviously, disabling weapon sprite bobbing will render the "Weapon Attack Alignment" feature useless.**

* Weapon Attack Alignment

  When attacking, align the weapon sprite relative to the center of the screen.
  
  * Off: The weapon sprite's position remains unchanged when attacking, as in vanilla Doom (default).
  * Centered: The weapon sprite gets centered horizontally and pushed up vertically into its idle position, as in e.g. Jaguar Doom.
  * Bobbing: The weapon sprite continues to bob as it does while the player is moving.

* Weapon Recoil Pitch

  When firing, give an impression of the weapon pushing up by slightly changing the player's vertical view angle without affecting the trajectory (default: off).

  **Enabling this feature will cause the sky texture to become vertically stretched.**

* Negative Player Health

  Allow the player's health to go below 0, to give an impression of the lethal hit's damage (default: off).

* Default Difficulty

  Set default difficulty when starting a new game (default: Hurt Me Plenty).

## Crosshair

These options change the appearance of the "crosshair" aiming aid.

* Draw Crosshair

  Draw the crosshair aiming aid.

  * Static: The crosshair is drawn in the center of the screen.
  * Projected: The crosshair is projected into the game scene, moving with the player and showing exactly where the player's shot would hit.
  * Off (default).

* Crosshair Shape

  Set the shape of the crosshair.

  * Cross: + (default).
  * Chevron: ^
  * Dot: .

* Color Indicates Health

  Colorize the crosshair depending on the player's health value, using the same color scheme as the Status Bar's Health widget (default: off). 

* Highlight on Target

  Highlight the crosshair if the shot would hit an enemy (default: off).

## Physical

These features change the game physics. **They are forcefully disabled during demo recording and playback as well as network games.**

* Vertical Aiming

  * Autoaim: Help the player aiming by guiding the shot trajectory towards monsters, as in Vanilla Doom (default).
  * Direct: Aim the shot trajectory exactly into the player's looking direction.
  * Both: As above, but enable "Autoaim" if a shot would miss a monster otherwise.

* Allow Jumping

  * Allow the player to jump (usually bound to the <kbd>/</kbd> key) either "high", as in Hexen, or "low", as in Strife (default: off).

* Walk over/under Monsters

  * Allow the player to walk across monsters' heads or beneath flying monsters (default: off).

## Demos

These features provide some additional information when recording or playing back demos.

* Show Demo Timer

  Show a small timer widget in the upper right corner of the screen during demo recording and/or playback (default: off).

* Playback Timer Direction

  If the "Demo Timer" widget is enabled during demo playback, decide whether it counts up the elapsed time since the demo started (forward, default) or the remaining time until it ends (backward).

* Show Demo Progress Bar

  Show a black/white progress bar at the bottom of the screen that fills up from left to right during demo playback (default: off).

* "Use" Button Timer

  Whenever the "Use" button is pressed, momentarily display a timestamp in the upper left of the screen (default: off).
