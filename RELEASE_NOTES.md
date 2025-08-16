## 3.1.1 (2025-08-14)

### General
  * Fix compilation on GCC 15 (thanks Fabian).
  * Hide public IP addresses for privacy.
  * Use native OpenGL texture format for better performance (thanks Fabian).
  * Improved directory handling on Windows (thank Roman Fomin):
      * Fixes problems with postfix "." for non-relative paths.
      * Support for long NTFS paths (up to 32768 characters).
  * Fixed metainfo files for latest AppStream spec compliance (thanks Fabian).
  * Add option to enable or disable smooth pixel scaling (thanks StevenSYS).
  * Allow spaces in GUS patch folder path (thanks Michael Day).
  * Add support for non-US backslash key (thanks Michael Day).
  * Never let SDL Mixer use native midi on Windows (thanks Michael Day).
  * Use $TMPDIR to find tempdir on Unix (thanks Mr. Myth).
  * Improve man page formatting (thanks g-branden-robinson).

### Build systems
  * Can now be built with Emscripten (thanks James Baicoianu).
  * Misc. improvements (thanks suve, Emmanuel Ferdman, Gabriele Simoni, et al.)

### Setup
  * Use correct EGA palette colors (thanks Henrique Jorge).

### Doom
  * Initial support for Doom version 1.5 (thanks Michael Francis).

### Heretic
  * Fix crash after loading too many saved games (thanks Noseey).

### Hexen
  * Fix crash with Heresiarch fireballs if player has never touched a wall (thank Noseey).

### Strife
  * Initialize floor->type and floor->crush fields in EV_BuildStairs() (thanks Fabian).
