# Chocolate Doom on Sysop-64

This port adapts Chocolate Doom to run on a real C64 display path through the
Sysop-64 cartridge.  The ARM/Linux side runs the Chocolate Doom game logic,
converts each 320x200 Doom frame into C64 multicolor bitmap data, and uses the
Sysop-64 DMA, audio, keyboard, mouse, framebuffer, and palette APIs to drive the
cartridge hardware.

The goal is not to make a new Doom renderer.  The game still renders normally
into Chocolate Doom's 8-bit screen buffer.  The sysop backend then treats that
frame as source material and produces the best C64-compatible presentation it
can each frame.

## Important Files

- `Makefile.sysop64` builds the sysop target without SDL.  It links against
  `/root/Sysop-64/linux-code/build/lib/libsysop64.a` and includes
  `/root/Sysop-64/linux-code/libsysop64/include`.
- `src/sysop64/i_sysop64_main.c` is the sysop executable entry point.  It provides
  `--help`, expands `--sysop-clean-all`, handles response files, and then calls
  `D_DoomMain()`.
- `src/sysop64/i_sysop64.c` is the main backend: C64 bitmap upload scheduling,
  palette control, framebuffer/clean C64 overlays, and Chocolate Doom video glue live here.
- `src/sysop64/sysop64_keyboard.c` owns C64 keyboard matrix scanning, C64 key
  translation, key queueing, and Chocolate Doom key event posting.
- `src/sysop64/sysop64_mouse.c` owns Sysop mouse discovery, `/dev/input`
  event reading, and Chocolate Doom mouse event posting.
- `src/sysop64/sysop64_joystick.c` owns optional C64 joystick port 2 polling
  and maps it onto Doom movement/fire/use key events.
- `src/sysop64/sysop64_sound.c` owns Sysop PCM SFX playback and Chocolate Doom
  sound/music glue.
- `src/sysop64/sysop64_image.c` owns shared RGB-to-C64 palette state, helper
  palette scoring routines, and the shared Koala frame buffer.
- `src/sysop64/sysop64_image_mega.c` owns indexed framebuffer conversion and
  its live-tunable tone/color/dither/scoring options.
- `src/sysop64/sysop64_tune_http.c` owns the optional HTTP listener for mega converter controls.
- `src/sysop64/sysop64_tune_http.html` is the tuner page loaded at runtime.
- `src/sysop64/sysop64_backend.h` is the small shared interface between the
  main backend and the Sysop helper modules.
- `src/sysop64/sid_player_bridge.cpp` wraps the SID player provided by
  `libsysop64` and exposes a small C API to `i_sysop64.c`.
- `src/sysop64/SDL*.h` are tiny SDL compatibility headers used only so shared
  Chocolate Doom headers compile cleanly for this target.
- `src/doom/m_menu.c`, `src/doom/hu_stuff.c`, `src/doom/st_stuff.c`, and
  `src/doom/wi_stuff.c` contain small sysop-facing hooks used to snapshot menu,
  message, status bar, and intermission state for clean C64 rendering.
- `src/sysop64/net_sysop64.c` and `src/sysop64/net_sysop64_gui.c` are included
  by the sysop makefile so Chocolate Doom's network code can be built for this target.

## Building

On the sysop build machine:

```sh
make -f Makefile.sysop64
```


The output is `chocolate-doom-sysop64`.

The sysop makefile intentionally does not build or link SDL.  A couple of tiny
SDL compatibility headers under `src/sysop64` exist only so shared Chocolate
Doom headers compile cleanly for this target.

## Runtime Files

At minimum, provide a Doom IWAD:

```sh
./chocolate-doom-sysop64 -iwad doom.wad
```

PWADs should be passed with standard Chocolate Doom options:

```sh
./chocolate-doom-sysop64 -iwad doom.wad -file mylevel.wad
```

SID music defaults to `At_Dooms_Gate.sid` in the same directory as the
executable. Use `--sysop-sid path/to/tune.sid` or
`--sysop-sid=path/to/tune.sid` to choose a different file, or `--sysop-no-sid`
to disable SID music. If the selected file is missing, the game still runs
without SID music.

## Video Architecture

Chocolate Doom renders into `I_VideoBuffer` using its normal 320x200 indexed
framebuffer.  `I_FinishUpdate()` converts that indexed image into an RGBA
scratch buffer, then `Sysop_BackendDrawFrame()` performs the sysop frame work:

1. Update any adaptive HDMI/C64 palette state.
2. Convert the Doom frame to C64 multicolor bitmap format via `src/sysop64/sysop64_image.c`.
3. Draw optional clean C64 overlays for messages, menus, status, and
   intermission screens.
4. Schedule palette split effects if enabled.
5. Begin a DMA tag and upload bitmap, screen RAM, color RAM, and VIC register
   updates through the sysop poke scheduler.
6. Poll keyboard, joystick, and mouse input.

The C64 display uses a Koala-like layout:

- bitmap data: 8000 bytes
- screen RAM/color pair data: 1000 bytes
- color RAM data: 1000 bytes
- background color: one byte

The backend configures C64 multicolor bitmap mode with the expected `D011`,
`D016`, `D018`, CIA bank, background, and border values.  VIC timing is detected
with `sysop_get_vic_info()`, so PAL and NTSC systems get the correct line and
cycle counts for scheduling.

For debugging the conversion quality, the C64 conversion/upload path can be
bypassed entirely:

```sh
--sysop-framebuffer-debug
--sysop-framebuffer-split
--sysop-framebuffer-split-demo
--sysop-display=framebuffer|split|split-demo|c64
```

`--sysop-framebuffer-debug` renders Chocolate Doom's normal 320x200 indexed
framebuffer with the current Doom palette directly into the Sysop-64 ARGB
framebuffer at 4x nearest neighbor scale. It draws into one mapped buffer while
the other is visible, calls `sysop_framebuffer_flip()`, advances the draw buffer, and
waits for `sysop_wait_hdmi_vblank()` before drawing the next frame, matching the
steady-state double-buffered pattern used by the sysop menu program. It is intended
as a reference image for comparing against the C64 dither/conversion paths, so
clean C64 overlays and C64 bitmap uploads are not applied while this mode is
active.

`--sysop-framebuffer-split` keeps the normal C64 conversion/upload path active
and overlays only the left half of the raw Doom framebuffer. The right half of
the ARGB framebuffer remains transparent, which gives a live side-by-side
comparison between the raw source and the C64 output on the same display. The
split starts in the middle. If `--sysop-joystick` is not enabled, joystick port
2 left/right moves the split line so you can choose how much of the screen uses
the raw framebuffer overlay. Use `--sysop-display=c64` or
`--sysop-no-framebuffer-debug` to return to normal C64 output.

`--sysop-framebuffer-split-demo` enables the same split overlay and automatically
sweeps the split line from left to right, then right to left, using the same
step size as the joystick adjustment.

Frame uploads are scheduled rather than blasted blindly.  The scheduler spreads
the more-than-10K pokes needed for a complete frame update across safe raster
time, tracks overflow, and uses `sysop_dma_tag_data()` /
`sysop_dma_write_tag()` to keep frame pacing stable.

## C64 Image Conversion

The C64 display path reads Chocolate Doom's active 8-bit paletted framebuffer,
averages physical RGB values from the current `I_SetPalette()` palette, and
maps pairs of horizontal Doom pixels into one C64 multicolor pixel. Each 8x8
C64 cell must choose a shared background plus three local colors, so the
converter:

- applies configurable tone/color preprocessing and local detail boost,
- uses selectable ordered or hash dithering before quantization,
- quantizes Doom RGB colors toward the selected Sysop-64/C64 target palette,
- builds candidate color sets per cell,
- scores candidate palettes against the 32 multicolor samples in the cell,
- chooses stable screen/color RAM roles to reduce flicker, and
- writes packed two-bit bitmap bytes into the Koala buffer.

Adaptive HDMI palette updates are enabled by default. When adaptive updates are
enabled, the RGBA scratch buffer is refreshed for the sampler; otherwise C64
conversion works directly from `I_VideoBuffer`.

The compiled image-conversion defaults are:

```sh
--sysop-palette=adaptive \
  --sysop-mega-set=fast_tables=on \
  --sysop-mega-set=dither=bayer8 \
  --sysop-mega-set=palette=pepto-ntsc-sony \
  --sysop-mega-set=dither_strength=75 \
  --sysop-mega-set=brightness=23 \
  --sysop-mega-set=contrast=150 \
  --sysop-mega-set=gamma=65 \
  --sysop-mega-set=saturation=100 \
  --sysop-mega-set=vibrance=0 \
  --sysop-mega-set=detail_pop=0 \
  --sysop-mega-set=surface_detail=0 \
  --sysop-mega-set=luma_weight=0 \
  --sysop-mega-set=chroma_weight=0 \
  --sysop-mega-set=red_weight=3 \
  --sysop-mega-set=green_weight=4 \
  --sysop-mega-set=blue_weight=4 \
  --sysop-mega-set=black_penalty=0 \
  --sysop-mega-set=yellow_penalty=0 \
  --sysop-mega-set=neutral_guard=0 \
  --sysop-mega-set=candidate_budget=6 \
  --sysop-mega-set=background_color=-1
```

Image-conversion settings can be changed from the command line:

```sh
./chocolate-doom-sysop64 -iwad doom.wad \
  --sysop-mega-set=dither_strength=60 \
  --sysop-mega-set=contrast=135
```

Or with the small HTTP tuner:

```sh
./chocolate-doom-sysop64 -iwad doom.wad --sysop-mega-http
```

By default this listens on `0.0.0.0:6464`, so open
`http://<sysop-ip>:6464/` from another machine.  A specific listener can be
selected with `--sysop-mega-http=127.0.0.1:6464` or
`--sysop-mega-http=8080`.  The page exposes sliders/selectors for dither
pattern, conversion palette model, strength, brightness, contrast, gamma,
saturation, vibrance, local detail, surface-detail weighting, RGB/luma/chroma
scoring weights, black/yellow penalties, neutral guard, candidate budget, and
background color.  It also includes an adaptive palette toggle, equivalent to
enabling or disabling `--sysop-palette=adaptive` while the game is running.
The same values can be set with
`--sysop-mega-set=name=value`; `--sysop-mega-palette=current|custom|VICE_NAME`
is a shortcut for the palette model.  The page also shows a 16-color swatch
preview of the currently selected mega target palette.  Editing a swatch clones
the active palette into `custom`, changes that one color, and uses the custom
palette for both mega conversion scoring and the Sysop HDMI base palette.
Browser presets and `Copy settings` include custom colors when the custom
palette is active; the command-line form is
`--sysop-mega-set=palette_color_N=#RRGGBB`.

The tuner HTML is not compiled into the executable.  At startup, the HTTP listener loads `sysop64_tune_http.html` from `SYSOP_TUNE_HTTP_PAGE`, `./sysop64_tune_http.html`, `./src/sysop64/sysop64_tune_http.html`, or matching paths beside the executable.  Keep that file with the executable or source tree when copying a sysop build.  The root page is reloaded from disk on browser refresh, so UI edits do not require rebuilding the executable.

Mega uses cached lookup tables by default.  `fast_tables=on` precomputes the
current tone curve, dither noise, C64 color ordering, and all 65,536 possible
horizontal Doom palette-pair samples.  It also maintains a lazy exact RGB
distance cache for colors encountered while scoring cells.  Those tables are
rebuilt only when mega options, the Doom palette, or the active C64 palette
change.  Disable it with `--sysop-mega-set=fast_tables=off` from the command
line or the HTTP tuner if you need to compare against the fully direct math path
while tuning.

The HTTP tuner keeps ten browser-local preset slots named `Preset 1` through
`Preset 10`.  Saving or loading a preset marks it as the active preset, and that
preset is automatically applied again when the page is refreshed.  `Reset
default` restores the compiled mega defaults listed above on the running game
and clears the auto-load marker, but leaves the saved preset slots in browser local storage.
`Copy settings` copies a shareable command-line fragment using
`--sysop-mega-set=name=value` pairs.

Mega dither should normally be configured with
`--sysop-mega-dither=PATTERN` or
`--sysop-mega-set=dither=PATTERN`. Supported patterns are `off`, `bayer2`,
`bayer4`, `bayer8`, `bayer8x16`, `checker`, `diagonal`, `dot`, and `hash`.
`--sysop-mega-strength=N` or `--sysop-mega-set=dither_strength=N` controls
amplitude from `0` to `150`.

Display tuning hotkeys can be enabled with `--sysop-display-tune`.  The useful
ones are:

- `T`: cycle mega dither pattern
- `A` / `Q`: step mega dither pattern down/up
- `S` / `W`: adjust mega contrast
- `D` / `E`: adjust mega brightness
- `Z` / `X`: adjust mega dither strength
- `F`: toggle mega fast tables
- `R`: reset mega options to compiled defaults

Those hotkeys are disabled by default because several of those keys are useful
in-game.

## Palette Work

The Sysop-64 default C64 palette is installed through the Sysop-64 HDMI palette
APIs. Mega uses one target palette for both C64 color scoring and the fixed
Sysop HDMI base palette. `current` means the Sysop-64 default C64 palette,
`custom` means the editable tuner palette, and VICE palette names select one of
the embedded `.vpl` palettes.

The mega target palette list also includes the C64 `.vpl` palettes from VICE's
`data/C64` directory.  The HTTP tuner shows their VICE `NAME:` labels and the
command-line value is the lowercase `.vpl` filename without the extension, for
example:

```sh
--sysop-mega-palette=pepto-pal
--sysop-mega-palette=colodore
--sysop-mega-palette=palette_6569r5_v1r
```

Selecting one of these palettes changes both mega conversion scoring and the
Sysop HDMI base palette, just like `current` and `custom`.

Optional HDMI palette effects can adjust the displayed Sysop HDMI palette:

- `--sysop-palette=adaptive` is the default. It incrementally samples the
  current Doom frame over several frames and nudges active HDMI colors toward
  the sampled averages.
- `--sysop-palette=static` disables adaptive/status effects and uses the fixed
  base palette selected by the active converter.
- `--sysop-palette=status-split` applies a different palette slice around the
  status bar region.
- `--sysop-palette=adaptive-status` combines adaptive frame color and the status
  split.
- `--sysop-palette-rate=N` controls the adaptive update interval.

Adaptive palette updates are intentionally treated as an HDMI display effect.
The RGB-to-C64 converter keeps using the stable mega target palette, so adaptive
color nudges do not force expensive per-update table rebuilds or alter native
C64 color-index choices.

Menu and intermission screens can also use a sharp red/gold palette treatment:

- `--sysop-menu-palette=sharp`
- `--sysop-menu-palette=off`

## Clean C64 Overlays

Doom's original patch text often becomes hard to read after conversion to C64
multicolor bitmap constraints.  Several optional clean overlays draw important
UI directly in the C64 bitmap using simple high-contrast C64 text and boxes.

The umbrella option is:

```sh
--sysop-clean-all
```

It expands to:

- `--sysop-clean-messages`
- `--sysop-clean-menu`
- `--sysop-clean-status`
- `--sysop-clean-intermission`

Individual options:

- `--sysop-hud=fb|clean|bitmap|off`
- `--sysop-hud-fb`
- `--sysop-clean-messages`
- `--sysop-hud-bitmap`
- `--sysop-hud-off`
- `--sysop-menu=clean|doom|off`
- `--sysop-clean-menu`
- `--sysop-status=clean|off`
- `--sysop-clean-status`
- `--sysop-intermission=clean|off`
- `--sysop-clean-intermission`

`--sysop-hud-fb` uses the Sysop-64 framebuffer overlay mechanism for in-game
messages.  The other clean modes draw into the converted C64 bitmap.

When keeping Doom's original menu/intermission patches, the backend can lock
selected source palette colors into clearer C64 colors:

- `--sysop-menu-dither=sharp`
- `--sysop-menu-dither=poster`
- `--sysop-menu-dither=off`

## Audio

There are two audio paths.

### SID Music

The backend loads `At_Dooms_Gate.sid` by default and plays it from a separate
thread. Use `--sysop-sid path/to/tune.sid` or
`--sysop-sid=path/to/tune.sid` to choose a different SID file. Use
`--sysop-no-sid` to disable SID music entirely while leaving the rest of the
audio backend active. The C++ SID player emits SID register writes each frame;
the sysop backend pokes those writes to `$D400-$D41F`.

Music volume must be controlled only through the Sysop-64 SID mixer APIs:

```c
sysop_audio_set_sid_volume_left(volume);
sysop_audio_set_sid_volume_right(volume);
sysop_audio_set_sid_volume(left, right);
```

`I_SetMusicVolume()` maps Chocolate Doom's `0..127` volume range to the sysop
`0..255` range and calls `sysop_audio_set_sid_volume(volume, volume)`.  Do not
scale `$D418` or otherwise modify the SID register stream for menu volume.

### PCM Sound Effects

PCM SFX are enabled by default and can be disabled with:

```sh
--sysop-no-pcm-sfx
```

The PCM path uses eight sysop audio channels.  Each channel gets a fixed 1 MiB
region in audio memory starting at `0x28000000`.  WAD sound effects are cached
as unsigned 8-bit mono samples and copied into the selected channel memory when
played.

For each PCM channel the backend uses this pattern:

1. `sysop_audio_select_channel(channel)`
2. configure format, base address, length, loop flag, and phase step
3. set left/right volume with `sysop_audio_set_volume(left, right)`
4. start playback

Chocolate Doom's spatial separation and SFX volume are converted to per-channel
left/right volumes.  Active channels are updated when positional sound
parameters change.

## Input

Keyboard input uses the newer Sysop-64 keyboard scan data path.  Raw C64 keys
are translated to Chocolate Doom keys before posting events.

Useful sysop input options:

- `--sysop-key-debug` prints raw key translation details.
- `--idkfa` starts each single-player spawn with the classic all-weapons, ammo, armor, and keys loadout.
- `--sysop-mouse` enables the Sysop-64 mouse driver.
- `--sysop-no-mouse` disables it.
- `--sysop-mouse-turn-only` ignores mouse Y movement.
- `--sysop-mouse-vertical` allows mouse Y forward/back movement.
- `--sysop-mouse-wasd` sets W/S move, A/D strafe, and Shift run.
- `--sysop-no-mouse-wasd` disables that automatic binding.
- `--sysop-joystick` enables C64 joystick port 2 controls.
- `--sysop-no-joystick` disables C64 joystick input.

The mouse code scans `/dev/input` for a likely mouse-like event device, reads
relative motion/buttons, and posts Chocolate Doom mouse events.  With
`--sysop-mouse`, vertical mouse movement is disabled by default so the mouse is
used for turning/aiming while W/S handle forward/back.  `--sysop-mouse-vertical`
restores Doom-style mouse Y forward/back movement.

The C64 joystick path reads `sysop_read_joystick(2)` and interprets the normal
active-low C64 joystick bits.  With `--sysop-joystick`, directions map to the
current Doom movement bindings.  Fire is treated as a modifier:

- tap fire: shoot once
- hold fire and push up/down: run forward/back
- hold fire and push left/right: strafe left/right
- hold fire with no direction: use/open after a short hold

Some C64-oriented bindings added by the sysop backend:

- C64 `CTRL` opens the automap.
- Shift plus the single C64 cursor key can navigate upward in menus.
- Weapon cycling defaults to `9` previous and `0` next if those bindings are
  otherwise unset.

## Help and Common Launches

The sysop executable has a focused help page:

```sh
./chocolate-doom-sysop64 --help
```

Common examples:

```sh
./chocolate-doom-sysop64 -iwad doom.wad --sysop-clean-all --sysop-mouse
./chocolate-doom-sysop64 -iwad doom.wad --sysop-clean-menu --sysop-clean-status
./chocolate-doom-sysop64 -iwad doom.wad --sysop-no-pcm-sfx
./chocolate-doom-sysop64 -iwad doom.wad --sysop-sid my_tune.sid
./chocolate-doom-sysop64 -iwad doom.wad --sysop-no-sid
./chocolate-doom-sysop64 -iwad doom.wad --idkfa
./chocolate-doom-sysop64 -iwad doom.wad --sysop-display-tune
./chocolate-doom-sysop64 -iwad doom.wad --sysop-framebuffer-debug
./chocolate-doom-sysop64 -iwad doom.wad --sysop-framebuffer-split
./chocolate-doom-sysop64 -iwad doom.wad --sysop-framebuffer-split-demo
```

## Development Notes

- Keep the sysop backend initialized before issuing sysop commands.  The helper
  refcount around `sysop_init()`/`sysop_uninit()` exists to prevent audio and
  video from racing library lifetime.
- Avoid touching the SID register stream for user volume.  Use
  `sysop_audio_set_sid_volume*()` for music and `sysop_audio_select_channel()`
  plus `sysop_audio_set_volume*()` for PCM SFX.
- `--sysop-clean-all` is expanded in `src/sysop64/i_sysop64_main.c` before Doom parses
  options.  There are also defensive checks in menu/rendering code so the alias
  still behaves like the explicit clean options.
- The port currently targets the Doom executable path.  Chocolate Doom's other
  game frontends have not been given equivalent sysop render/audio/input
  integration.
