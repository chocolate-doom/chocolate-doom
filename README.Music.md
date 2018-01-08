Doom has a memorable and atmospheric soundtrack. Like many games of
the era, it is MIDI-based. Chocolate Doom includes a number of
different options for music playback, detailed below.

# Native MIDI playback

Most modern operating systems have some kind of built-in support for
MIDI playback; some have very good quality MIDI playback (Mac OS X for
example). To use this, choose “Native MIDI” in the sound configuration
dialog in the setup tool.

# Timidity

Timidity is a software-based MIDI synthesizer, and a version of it is
included in the SDL2_mixer library used by Chocolate Doom. To use
Timidity for MIDI playback, first download a sound font. An example of
a good quality sound font is the eawpats font, which can be downloaded
from the idgames archive as sounds/eawpats.zip:

  https://www.doomworld.com/idgames/sounds/eawpats

Having installed a sound font, select “Native MIDI” in the sound
configuration dialog in the setup tool, and use the “Timidity
configuration file” widget below to enter the path to the Timidity
configuration file (normally named timidity.cfg).

# Gravis Ultrasound (GUS)

The Gravis Ultrasound (GUS) was a PC sound card popular in the ’90s,
notable for having wavetable synthesis that provided MIDI playback
that was superior to most other cards of the era. Chocolate Doom
includes a “pseudo-GUS emulation” feature that simulates the GUS
(using Timidity, under the hood).

To use this feature you need a copy of the GUS patch files that were
distributed with the original GUS patches. If you have Doom 3: BFG
Edition, these patches are included with its version of classic Doom,
and are automatically detected. Otherwise, they can be downloaded
from the idgames archive as music/dgguspat.zip:

  https://www.doomworld.com/idgames/music/dgguspat

Having downloaded the patches, select “GUS (emulated)” in the sound
configuration dialog in the setup tool, and use the “GUS patch path”
widget to enter the path to the directory containing the patch files.

By default a GUS card with 1024KB is simulated; to simulate a 256KB,
512KB or 768KB card instead, change the gus_ram_kb option in
chocolate-doom.cfg.

# OPL (Soundblaster / Adlib)

Most people playing Doom in the ’90s had Soundblaster-compatible sound
cards, which used the Yamaha OPL series of chips for FM-based MIDI
synthesis. Chocolate Doom includes the ability to emulate these chips
for a retro experience. OPL emulation is the default MIDI playback,
but can be selected in the setup tool as “OPL (Adlib/SB)”.

Most modern computers do not include an OPL chip any more, as CPUs are
fast enough to do decent software MIDI synthesis. However, no software
emulator sounds exactly like a real (hardware) OPL chip, and a few
cards do have real hardware OPL. If you have such a card, here’s how
to configure Chocolate Doom to use it.

## Sound cards with OPL chips

If you have an ISA sound card, it almost certainly includes an OPL
chip. Modern computers don’t have slots for ISA cards though, so you
must be running a pretty old machine.

If you have a PCI sound card, you probably don’t have an OPL chip.
However, there are some exceptions to this. The following cards are
known to include “legacy” OPL support:

  * C-Media CMI8738 (*)
  * Forte Media FM801
  * Cards based on the Yamaha YMF724 (*)

Other cards that apparently have OPL support but have not been tested:

  * S3 SonicVibes
  * AZTech PCI 168 (AZT 3328 chipset)
  * ESS Solo-1 sound cards (ES1938, ES1946, ES1969 chipset)
  * Conexant Riptide Audio/Modem combo cards
  * Cards based on the Crystal Semiconductors CS4281
  * Cards based on the Avance Logic ALS300
  * Cards based on the Avance Logic ALS4000

If you desperately want hardware OPL music, you may be able to find
one of these cards for sale cheap on eBay.

For the cards listed above with (\*) next to them, OPL support is
disabled by default and must be explictly enabled in software. See
sections below for operating system-specific instructions on how you
may be able to do this.

If your machine is not a PC, you don’t have an OPL chip, and you will
have to use the software OPL.

## Operating System support

If you’re certain that you have a sound card with hardware OPL, you
may need to take extra steps to configure your operating system to
allow access to it. To do hardware OPL, Chocolate Doom must access
the chip directly, which is usually not possible in modern operating
systems unless you are running as the superuser (root/Administrator).

### Microsoft Windows

On modern Windows systems it is not possible to directly access the
OPL chip, even when running as Administrator. Fortunately, it is
possible to use the “ioperm.sys” driver developed for Cygwin (look for
iopermsys-0.2.1.tar.bz2):

  http://openwince.sourceforge.net/ioperm/

It is not necessary to have Cygwin installed to use this. Copy the
ioperm.sys file into the same directory as the Chocolate Doom
executable and it should be automatically loaded.

You can confirm that hardware OPL is working by checking for this
message in stdout.txt:

    OPL_Init: Using driver 'Win32'.

If you have a C-Media CMI8738, you may need to enable the `FM_EN`
flag in Windows Device Manager to access hardware OPL output. See
[this](http://www.vogons.org/viewtopic.php?f=46&t=36445) thread on
vogons.org for some more information.

### Linux

If you are using a system based on the Linux kernel, you can access
the OPL chip directly, but you must be running as root. You can
confirm that hardware OPL is working, by checking for this message on
startup:

    OPL_Init: Using driver 'Linux'.

If you are using one of the PCI cards in the list above with a (*)
next to it, you may need to manually enable FM legacy support. Add
the following to your /etc/modprobe.conf file to do this:

    options snd-ymfpci fm_port=0x388
    options snd-cmipci fm_port=0x388

### OpenBSD/NetBSD

You must be running as root to access the hardware OPL directly. You
can confirm that hardware OPL is working by checking for this message
on startup:

    OPL_Init: Using driver 'OpenBSD'.

There is no native OPL backend for FreeBSD yet. Sorry!

# Other options

If you have some other favorite MIDI playback option that isn’t
listed above, you can set a hook to invoke an external command for
MIDI playback using the ‘snd_musiccmd’ configuration file option. For
example, set:

    snd_musiccmd    "aplaymidi -p 128:0"

in your chocolate-doom.cfg file.
