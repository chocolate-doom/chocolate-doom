The aim of Chocolate Doom is to behave as closely to Vanilla Doom as
possible.  As a result, you may experience problems that you would
also experience when using Vanilla Doom.  These are not “bugs” as
Chocolate Doom is behaving as intended.

This is not intended to be a comprehensive list of Vanilla Doom bugs.
For more information, consult the “engine bugs” page of the Doom Wiki.

## Game exits after title screen with message about game version

The game may exit after the title screen is shown, with a message like
the following:

    Demo is from a different game version!
    (read 2, should be 109)

    *** You may need to upgrade your version of Doom to v1.9. ***
        See: https://www.doomworld.com/classicdoom/info/patches.php
        This appears to be v1.0/v1.1/v1.2.

This usually indicates that your IWAD file that you are using to play
the game (usually named doom.wad or doom2.wad) is out of date.
Chocolate Doom only supports versions 1.666 through 1.9.

To fix the problem, you must upgrade your IWAD file, preferably
to 1.9.  The URL in the message has downloadable upgrade patches that
you can use to upgrade.

## Game exits in demo loop when playing Final Doom

When playing with the Final Doom IWAD files (tnt.wad, plutonia.wad),
if you leave the game at the title screen to play through the demo
loop, it will eventually exit with the following error message:

    W_GetNumForName: demo4 not found!

This is the same behavior as the Vanilla executables that were
bundled with Final Doom.  When Ultimate Doom was developed, a fourth
demo was added to the demo loop, and this change was retained in the
Final Doom version of the executable.  However, the Final Doom IWADs
do not include a fourth demo, so the game crashes.

An alternate version of Final Doom was included in the Id Anthology
boxed set, and this version of the game fixed this bug. However, this
version also changes the teleport behavior, so the default is to
emulate the most commonly distributed version of the game. To use
the alternate version, run with:

    chocolate-doom -gameversion final2

## Game exits when accessing the options menu

The game may exit with the message “Bad V_DrawPatch” when accessing
the options menu, if you have your mouse sensitivity set high.

The Doom options menu has a slider that allows the mouse sensitivity
to be controlled; however, it has only a very limited range. It is
common for experienced players to set a mouse sensitivity that is much
higher than what can be set via the options menu. The setup program
allows a larger range of values to be set.

However, setting very high sensitivity values causes the game to exit
when accessing the options menu under Vanilla Doom. Because Chocolate
Doom aims to emulate Vanilla Doom as closely as possible, it does the
same thing.

One solution to the problem is to set a lower mouse sensitivity.
Alternatively, all of the settings in the options menu can be
controlled through Doom’s key bindings anyway:

Option                     | Key
---------------------------|-----
End game                   | F7
Messages on/off            | F8
Graphic detail high/low    | F5
Screen size smaller/larger | -/+
Sound volume menu          | F4

## Game exits with “Savegame buffer overrun” when saving the game

If you are playing on a particularly large level, it is possible that
when you save the game, the game will quit with the message “Savegame
buffer overrun”.

Vanilla Doom has a limited size memory buffer that it uses for saving
games.  If you are playing on a large level, the buffer may be too
small for the entire savegame to fit.  Chocolate Doom allows the limit
to be disabled: in the setup tool, go to the “compatibility” menu and
disable the “Vanilla savegame limit” option.

If this error happens to you, your game has not been lost!  A file
named temp.dsg is saved; rename this to doomsav0.dsg to make it appear
in the first slot in the “load game” menu.  (On Unix systems, you will
need to look in the .chocolate-doom/savegames directory in your home
directory)

## Game ends suddenly when recording a demo

If you are recording a very long demo, the game may exit suddenly.
Vanilla Doom has a limited size memory buffer that it uses to save the
demo into.  When the buffer is full, the game exits.  You can tell if
this happens, as the demo file will be around 131,072 bytes in size.

You can work around this by using the -maxdemo command line parameter
to specify a larger buffer size.  Alternatively, the limit can be
disabled: in the setup tool, go to the compatibility menu and disable
the “Vanilla demo limit” option.

## Game exits with a message about “visplanes”

The game may exit with one of these messages:

    R_FindPlane: no more visplanes
    R_DrawPlanes: visplane overflow (129)

This is known as the “visplane overflow” limit and is one of the most
well-known Vanilla Doom engine limits.  You should only ever experience
this when trying to play an add-on level.  The level you are trying to
play is too complex; it was most likely designed to work with a limit
removing source port.

More information can be found here (archived link): https://archive.is/s6h7V

## IDMUS## cheat doesn’t work with shareware/registered Doom IWADs

The IDMUS cheat allows the in-game music to be changed.  However, in
the original v1.9 this cheat didn’t work properly when playing with
the Doom 1 (shareware and registered) IWADs.  This bug was fixed in
the Ultimate Doom and Final Doom executables.

Chocolate Doom emulates this bug.  When playing with the shareware or
registered Doom IWADs, the IDMUS cheat therefore does not work
properly.  If you are playing with the Ultimate Doom IWAD, the
Ultimate Doom executable is emulated by default, so the cheat works
properly.

## Some graphics are wrong when playing with BFG Edition IWADs

If you are playing using the IWAD files from Doom 3: BFG Edition, you
may notice that certain graphics appear strange or wrong. This
includes the title screen, and parts of the options menu.

The IWAD files in the new BFG Edition have had some significant
changes from the IWAD files in older releases. Some of the graphics
lumps have been removed or changed, and the Doom 2 secret levels are
also censored. Chocolate Doom includes some small workarounds that
allow the game to run, but for the best experience, it’s best to get a
copy of the classic versions of the IWADs. These are still available
to buy from Steam or GOG.com.
