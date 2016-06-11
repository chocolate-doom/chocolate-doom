````````````````````````````````````````````````````````````````````````

          Samuel ‘Kaiser’ Villarreal and James ‘Quasar’ Haley Present

           C      H      O      C      O      L      A      T      E
            ______________________________._________________________
           /   _____/\__    ___/\______   \   \_   _____/\_   _____/
           \_____  \   |    |    |       _/   ||    __)   |    __)_
           /        \  |    |    |    |   \   ||     \    |        \
          /_______  /  |____|    |____|_  /___|\___  /   /_______  /
                  \/                    \/         \/            \/

````````````````````````````````````````````````````````````````````````

## What is it?

Chocolate Strife is the most accurate and complete recreation of Rogue
Entertainment’s “Strife: Quest for the Sigil.”  It was created through more
than four years of reverse engineering effort with the blessings of the
original programmers of the game.


## Why?

The source code for Strife was lost, which means, unlike the code for all the
other commercial DOOM-engine games, it cannot be released. The only access we
have to the code is the binary executable file. Tools such as IDA Pro have
been employed to disassemble and decompile the executable, which was cross-
referenced against the Linux DOOM and DOS Heretic sources and painstakingly
combed over multiple times, instruction-by-instruction, to ensure that the
resulting Chocolate Doom-based executable is as close as possible to the
original.


## Is it Legal?

Chocolate Strife was originally reverse-engineered from the DOS Strife
binaries. Although reverse engineering is legally a protected activity, this
nonetheless left some open questions about its legal status.

In 2014, a new commercial release of Strife was published (Strife: Veteran
Edition) based on the Chocolate Strife code, and developed by the authors of
Chocolate Strife under commercial license. The release of Strife: Veteran
Edition, along with its GPL-licensed source code, constitutes tacit approval
for the legal status of Chocolate Strife by its current copyright holder.


## Is it Perfect?

Almost, but not entirely! That’s where you come in. Help us by reporting any
discrepancies you may notice between this executable and the vanilla DOS
program!

However, do *not* report any glitch that you can replicate in the vanilla EXE
as a bug. The point of Chocolate Strife, like Chocolate Doom before it, is to
be as bug-compatible with the original game as possible. Also be aware that
some glitches are impossible to compatibly recreate, and wherever this is the
case, Chocolate Strife has erred on the side of not crashing the program,
for example by initializing pointers to NULL rather than using them without
setting a value first.


## What are some known problems?

The demo version is *not* supported, and there are not any current plans to
support it in the future, due to the vast number of differences (the demo
version of Strife is based on an earlier version of Rogue’s
codebase).

The commercial Strife IWAD version 1.1 may run, but also exhibit issues.  Like
the demo version, there are no current plans to fully support it.  Make sure
your copy is updated to at least 1.2.  Strife: Veteran Edition already
includes the required version.


## How do I use it?

From the Run box or a command line, issue a command to Chocolate Strife just
like you would run Chocolate Doom. Most of the same command line parameters
are supported.

voices.wad will be read from the same directory as the IWAD, if it can be
found. If it is not found, then voices will be disabled just as would happen
with the vanilla executable. Do not add voices.wad using -file, as that is
redundant and unnecessary.

Some new command-line parameters in Chocolate Strife include the following:

  - -nograph
    - Disables the graphical introduction sequence. -devparm implies this.

  - -novoice
    - Disables voices even if voices.wad can be found.

  - -work
    - Enables Rogue’s playtesting mode. Automatic godmode, and pressing the
      inventory drop key will toggle noclipping.

  - -flip
    - Flips player gun graphics. This is buggy, however, because it does not
      reverse the graphics’ x offsets (this is an accurate emulation of the
      vanilla engine’s behavior).

  - -random
    - Randomizes the timing and location of item respawns in deathmatch, when
      item respawning is enabled.


## Copyright

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

See the “COPYING” file for the full license text. The source code for this
program is available from the same location where you downloaded this package.

Aside from Chocolate Doom, portions of the code are derived from the Eternity
Engine, Copyright 2011 Team Eternity, as published under the GNU GPL.
