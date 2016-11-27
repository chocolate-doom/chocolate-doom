Chocolate Doom has been designed around a careful and deliberate
philosophy that attempts to recreate the original (“Vanilla”) DOS
executables for Doom, Heretic, Hexen and Strife. This document
describes some of that philosophy and the reasoning behind it.

This document is descriptive, not proscriptive.

# Vanilla behavior

Ideally Chocolate Doom aims to recreate the behavior of the Vanilla
binaries, but different aspects of Vanilla behavior are held to
varying degrees of importance. It can be imagined as different “tiers”
of compatibility:

 * The game and gameplay itself is of central importance. Here, the
   Vanilla behavior ought to be maintained as accurately as possible.
   This includes the look, feel and sound, and things like demo
   compatibility.
 * The surrounding aspects of the game that aren’t part of the central
   gameplay experience can be extended as long as there’s a good
   reason and Vanilla behavior is respected.
 * The setup tool is not required to reproduce the behavior of the
   Vanilla setup tool, even though it reproduces its look and feel.

“Vanilla” is defined as:

 * DOS Doom 1.9 (although there are actually multiple “1.9”s).
 * DOS Heretic 1.3.
 * DOS Hexen 1.1.
 * DOS Strife 1.31.
 * DOS Chex Quest.

Compatibility with older versions of the DOS binaries is also a
secondary goal (though not pre-release versions). Other ports (either
official or unofficial) are out of scope: this includes console ports,
non-DOS ports, Doom 95 and Doom 3: BFG Edition.

# Compatibility

Chocolate Doom aims to be compatible with Vanilla Doom in several
different ways. Examples are:

 * Bug compatibility: the aim is to emulate compatibility of the
   original game down to bugs that were present in the DOS
   executables. This includes maintaining the limitations of the
   original engine: for example, the infamous “visplane overflow” bug
   is intentionally still present, where other source ports have
   removed it; this allows mappers targeting Vanilla Doom to use
   Chocolate Doom as a faithful substitute.
 * Demo compatibility: Doom was one of the first games to develop a
   ‘speedrunning’ community, and thousands of recordings of Doom
   gameplay (`.lmp` demo files) exist in the Compet-N archive. Chocolate
   Doom aims for the highest standard of demo compatibility with
   Vanilla Doom, a goal that is often complicated by obscure behavior
   that can be difficult to reproduce.
 * Configuration file compatibility: Chocolate Doom uses the same
   configuration file format as Vanilla Doom, such that a user should
   be able to reuse their existing Vanilla configuration file without
   making any changes. Extra non-Vanilla options are stored in a
   separate configuration file.
 * Savegame file compatibility: Chocolate Doom uses the same savegame
   file format as Vanilla, such that it should be possible to import
   and use existing savegame files.

# DOS tools

Chocolate Doom includes some features that aren’t part of Vanilla Doom
but exist for compatibility with DOS tools that interact with it.
These are considered part of the Vanilla experience and ought to be
treated as such. Some examples are:

 * The novert setting, which reproduces the functionality of
   `novert.exe`.
 * The `-deh` parameter, which loads Dehacked patches like dehacked.exe
   does under DOS. Chocolate Doom imposes the same limitations that
   Vanilla Dehacked does.

# Exceptions

Chocolate Doom differs from Vanilla Doom in a number of ways. In most
cases these are subtle, minor differences. Nonetheless they deserve
some explanation and justification. Here are some examples of
situations where changes are considered acceptable:

 1. Vanilla behavior can be broken that is harmful, eg. can damage the
    player’s computer, or is just outright misleading. For example:

    - Vanilla uses unbounded sprintf and strcpy (security issue).
    - Vanilla has crashes that force the user to reboot the machine.
    - Vanilla has an out of memory message that recommends tweaking
      CONFIG.SYS. As Chocolate Doom doesn’t run under DOS, reproducing
      this message would not be helpful.

 2. Subtly extending behavior is okay where it’s not clear what the
    Vanilla behavior is anyway. For example:

    - Opening the menu releases mouse grab in Chocolate Doom.
    - Chocolate Hexen’s graphical startup screen runs in a window.

 3. Supporting obsolete technology is not a goal: it’s considered
    acceptable that Chocolate Doom does not support every type of
    hardware from 1993. However, Chocolate Doom should aim to recreate
    the functionality in a modern way. Examples of technologies that
    aren’t supported are:

    - No support for IPX networks, but TCP/IP is supported instead.
    - No support for dial-up/serial connections; modern operating
      systems have features that do that for you.
    - No MS-DOS version.

 4. Changes are acceptable that allow the player to be able play the
    game. For example:

    - There are new key bindings for actions that can’t be rebound with
      Vanilla Doom, because it’s useful for portability to machines
      that don’t have a full keyboard.
    - There are additional mouse and joystick button bindings that let
      you perform actions with them that can only be done with the
      keyboard in Vanilla Doom.
    - Chocolate Doom includes some hacks to support the Doom 3: BFG
      Edition IWAD files. The assumption is that being able to at least
      play is better than nothing, even if it’s not Vanilla behavior.
    - Chocolate Strife does not emulate the save bug from
      Vanilla 1.31, which could corrupt saves when overwriting a slot,
      if the old slot was not part of the current game’s progression.
      Vanilla behavior is unexpected and potentially devastating.

 5. Adding extra options to Vanilla functionality is acceptable as long
    as the defaults match Vanilla, it doesn’t change gameplay and
    there’s a good reason for it. For example:

    - PNG screenshots are supported because PCX is an obsolete format.
    - Chocolate Doom has the vanilla_keyboard_mapping option that
      allows the user to use the native keyboard mapping for their
      computer, rather than always assuming a US layout.

 6. Changing configuration file defaults is acceptable where there’s a
    very good reason. For example:

    - Vanilla Doom defaults to no sound or music if a configuration
      file is not found. Chocolate Doom defaults to having sound
      effects and music turned on by default, because modern computers
      support these; there’s no need to configure hardware IRQ settings
      to get sound working.

 7. Things can be changed if they’re really just inconsequential. For
    example:

    - The startup messages in Chocolate Doom are not identical to
      Vanilla Doom and are not necessarily in the same order.
    - Vanilla Doom has command line options named `-comdev`, `-shdev`
      and `-regdev` used by id internally for development; these have
      been removed.

 8. Expansions to the vanilla demo formats are allowed, to make
    recording and playback of vanilla gameplay more convenient, with
    the following restrictions:

    - Such expansions are not supported in WAD files (they are not
      an editing feature for WAD authors to use).
    - Support for these features can be completely disabled using the
      `-strictdemos` command line argument.
    - A warning is shown to the user on the console (stdout) when a
      demo using one of these features is recorded or played back.

A good litmus test of when it’s acceptable to break from Vanilla
behavior is to ask the question: “Although this is Vanilla behavior,
is there anyone who would want this?”

For example, emulating Vanilla bugs like the visplane overflow bug is
something that is useful for people making Vanilla format maps. On the
other hand, painstakingly emulating Vanilla Doom by starting with no
sound or music by default is not helpful to anyone.

# Minimalism

Chocolate Doom aims to be minimalist and straightforward to configure;
in particular, the setup tool should have a sane interface. Part of
the inspiration for Chocolate Doom came from Boom’s complicated maze
of options menus (and a desire to avoid them). Too many preferences
lead to a bad user interface; see Havoc Pennington’s essay on Free
Software UI:

  http://ometer.com/free-software-ui.html

Chocolate Doom has some options that are quite obscure and only really
of interest to a small number of people. In these cases, the options
are hidden away in configuration files and deliberately not exposed in
the setup tool. The assumption is that if you care enough about those
obscure features, editing a configuration file by hand should not be a
huge problem for you.

Also inspirational was the README file from Havoc’s Metacity window
manager, where the list of features begins:

  > Boring window manager for the adult in you. Many window managers
  > are like Marshmallow Froot Loops; Metacity is like Cheerios.

I’d like to think that Chocolate Doom’s philosophy towards features is
similar. The idea is for a source port that is boring. I find the best
software - both proprietary and open source - is software that is
“egoless”: it does a job well without pretentions about its importance
or delusions of grandeur. A couple of other notable examples of
software that I feel embody this spirit of design in a beautiful way
are Marco Pesenti Gritti’s Epiphany web browser and Evince PDF viewer.

# Other philosophical aspects

Chocolate Doom aims for maximal portability. That includes running on
many different CPUs, different operating systems and different devices
(ie. not just a desktop machine with a keyboard and mouse).

Chocolate Doom is and will always remain Free Software. It will never
include code that is not compatible with the GNU GPL.
