 * Savegames saved by Crispy Doom are still **compatible with Vanilla Doom**, but are **not identical** anymore to the files that Vanilla Doom would have saved. This is caused by several reasons:
   * Crispy Doom writes savegames in an **extended format** that stores additional game state information past the end-of-file marker of the original savegame format. Vanilla and Chocolate Doom are still able to read savegames in this format but are not able to write it.
   * In order to distinguish NRFTL and Masterlevel maps from regular Doom 2 levels, Crispy Doom uses the `gameepisode` variable, which is used in Doom 1 to distinguish between the game episodes but is hitherto unused in Doom 2.
   * Crispy Doom preserves the `mobj->target` and `mobj->tracers` fields of map objects when saving a game by replacing their contents with specific indices. These indices are then converted back to the corresponding pointers when the game is restored again. In Vanilla and Chocolate Doom, however, the contents of the `mobj->target` and `mobj->tracers` fields are disregarded and overwritten with `NULL`.
   * Furthermore, Crispy Doom saves thinkers for active platforms in stasis.

 * If you are going to share savegames between Crispy Doom and Chocolate Doom, make sure to load all PWADs with the `-merge` parameter in the latter.

 * The Crispy HUD (i.e. fullscreen rendering with only the Status Bar numbers) is displayed when `screenblocks >= 12`, which isn't supported by Chocolate Doom. To retain config file compatibility, quit the game with any lower view size.

 * Crispy Doom features intermediate Gamma levels which aren't supported by Chocolate Doom. In order to share config files between both ports, make sure to quit Crispy Doom with a Gamma level up to 2 (i.e. `usegamma <= 4`), which corresponds to Gamma level 4 in Chocolate Doom.

 * Crispy Doom saves screenshots as exact replicas of the rendered screen content, i.e. with Gamma correction and pixel scaling applied, whereas Chocolate Doom saves images derived directly from the framebuffer.

 * The "flipped levels" and "SSG available in Doom 1" features introduced in Crispy Doom 1.3 are considered strictly **experimental**! They may produce savegames, demo files or netgames that are not compatible with Chocolate Doom, Vanilla Doom or previous versions of Crispy Doom at all. Furthermore, the `SPECHITS` cheat introduced in Crispy Doom 1.5 may leave a map in a completely inconsistent state and games saved after using it may even cause Vanilla to crash by exceeding static limits.
