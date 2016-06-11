This is Chocolate Doom’s “to do” list. Note that this is kind of an arbitrary
and unstructured wish list of features and improvements. The bug tracker
(http://chocolate-doom.org/bugs) has more feature requests.

* Multiplayer:
  - Use UPnP to automatically configure port forwarding for NATed
    networks.
  - Multiplayer options and configuration file (server name, etc)
* Improve multiplayer startup:
  - Select an IWAD automatically from the server’s game type rather than
    all players having to specify -iwad.
  - Send list of WADs to load instead of all clients having to specify -file.
  - Same applies to dehacked patches and wad merging parameters.
* Portability improvements:
  - Test on and fix for architectures where `((-2) >> 1) != -1`
  - Use size-specific types (eg. `int32_t` instead of int)
  - Don’t make structure packing assumptions when loading levels.
  - Port to every OS and architecture under the sun
  - Port to Emscripten and release a web-based version.
* Video capture mode
  - Real-time recording of gameplay
  - Batch conversion of demos into videos
* Heretic/Hexen/Strife:
  - Merge r_draw.c to common version and delete duplicates
  - Heretic v1.2 emulation (if possible)
  - Hexen v1.0 emulation (if possible/necessary)
  - Strife v1.1 emulation (for demo IWAD support)
  - Screensaver mode

Crazy pie in the sky ideas:

* Automatic WAD installer - download and run TCs from a list automatically
  (automating the current “instructions on wiki” system).
* Textscreen interface to the Compet-N database: menu driven system to
  automatically download and play speedruns.
* DWANGO-like interface for finding players and setting up games.
