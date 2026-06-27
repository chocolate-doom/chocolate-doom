//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Sysop-64 platform backend. Owns C64 bitmap upload scheduling, VIC
//     setup, palette control, framebuffer overlays, SID startup, and the
//     Chocolate Doom video/system glue for this target.
//

#include "config.h"
#include "deh_misc.h"
#include "d_englsh.h"
#include "deh_str.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "doomtype.h"
#include "i_joystick.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_controls.h"
#include "m_menu.h"
#include "m_misc.h"
#include "r_state.h"
#include "s_sound.h"
#include "tables.h"
#include "v_diskicon.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

#include "sysop64.h"
#include "sysop64/sysop64_backend.h"
#include "sysop64/sysop64_image.h"
#include "sysop64/sysop64_tune_http.h"
#include "sid_player_bridge.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Chocolate Doom still renders into an 8-bit indexed frame. The Sysop path
// keeps both the source palette and an optional RGBA scratch copy because the
// C64 converter wants indexed pixels, while HDMI palette effects need sampled
// RGB values from the same frame.
static uint32_t g_sysop_rgba_screen[SCREENWIDTH * SCREENHEIGHT] __attribute__((aligned(16)));
static byte sysop_choco_palette[256][3];
static uint8_t sysop_choco_palette_locks[256];

#define C64_HUD_MESSAGE_MAX 80
char hu_current_message[C64_HUD_MESSAGE_MAX + 1];
int hu_current_message_counter = 0;

#define SYSOP_MENU_MAX_ITEMS 12
#define SYSOP_MENU_TITLE_MAX 32
#define SYSOP_MENU_ITEM_MAX 40
#define SYSOP_MENU_MESSAGE_MAX 256

typedef struct
{
    int active;
    int is_message;
    int selected;
    int item_count;
    char title[SYSOP_MENU_TITLE_MAX];
    char items[SYSOP_MENU_MAX_ITEMS][SYSOP_MENU_ITEM_MAX];
    int item_status[SYSOP_MENU_MAX_ITEMS];
    char message[SYSOP_MENU_MESSAGE_MAX];
} sysop_menu_snapshot_t;

int sysop_clean_menu_enabled = 0;
int sysop_hud_messages_enabled = 0;
void M_GetSysopMenuSnapshot(sysop_menu_snapshot_t *snapshot);

static void play_sid_frame(void);
static void Sysop_UpdateIndexedLockTable(void);
static void start_sid_thread(void);
static void stop_sid_thread(void);
uint32_t sysop_dma_tag_data(void);
void sysop_dma_write_tag(uint32_t tag);
uint8_t sysop_get_vic_info(void);
void sysop_audio_set_sid_volume_left(uint32_t volume);
void sysop_audio_set_sid_volume_right(uint32_t volume);
void sysop_audio_set_sid_volume(uint32_t left, uint32_t right);
void sysop_wait_hdmi_vblank(void);

// Audio, video, and input can initialize at different points in Chocolate
// Doom's startup sequence. Keep sysop_init/sysop_uninit refcounted so the
// first subsystem brings the cartridge API up and the last one releases it.
static int sysop_library_refcount = 0;

// Acquire the shared Sysop library handle for a subsystem that is about to
// issue cartridge commands.
int Sysop_AcquireLibrary(const char *owner)
{
    int result;

    if (sysop_library_refcount == 0) {
        result = sysop_init();

        if (result != 0) {
            fprintf(stderr, "sysop_init failed for %s: %d\n",
                    owner != NULL ? owner : "driver", result);
            return 0;
        }
    }

    ++sysop_library_refcount;

    return 1;
}

// Release one subsystem's hold on the Sysop library and shut it down when the
// last user is gone.
void Sysop_ReleaseLibrary(void)
{
    if (sysop_library_refcount <= 0) {
        return;
    }

    --sysop_library_refcount;

    if (sysop_library_refcount == 0) {
        sysop_uninit();
    }
}

// Report whether sysop_init() has succeeded and hardware calls are currently
// permitted.
int Sysop_LibraryIsInitialized(void)
{
    return sysop_library_refcount > 0;
}

// Force a hardware shutdown during fatal paths where normal refcounted cleanup
// may not run.
void Sysop_ForceReleaseLibrary(void)
{
    if (sysop_library_refcount > 0) {
        sysop_uninit();
        sysop_library_refcount = 0;
    }
}

#define C64_HUD_MESSAGE_MAX 80
extern char hu_current_message[C64_HUD_MESSAGE_MAX + 1];
extern int hu_current_message_counter;

#define SYSOP_HUD_MODE_BITMAP 0
#define SYSOP_HUD_MODE_FRAMEBUFFER 1
#define SYSOP_HUD_MODE_OFF 2
#define SYSOP_HUD_MODE_CLEAN_BITMAP 3

static int g_sysop_hud_mode = SYSOP_HUD_MODE_OFF;

#define SYSOP_PALETTE_EFFECT_ADAPTIVE     0x01
#define SYSOP_PALETTE_EFFECT_STATUS_SPLIT 0x02
#define SYSOP_PALETTE_UPDATE_DEFAULT      8
#define SYSOP_ADAPTIVE_SAMPLE_X_STEP      4
#define SYSOP_ADAPTIVE_SAMPLE_Y_STEP      4
#define SYSOP_ADAPTIVE_MIN_COLOR_SAMPLES  8

static int g_sysop_palette_effects = SYSOP_PALETTE_EFFECT_ADAPTIVE;
static int g_sysop_palette_update_interval = SYSOP_PALETTE_UPDATE_DEFAULT;
static uint64_t g_sysop_adaptive_palette_sum[16][3];
static uint32_t g_sysop_adaptive_palette_count[16];
static int g_sysop_adaptive_palette_next_y = 0;
static const int (*g_sysop_adaptive_palette_base)[3] = NULL;
static int g_sysop_startup_palette[16][3];
static int g_sysop_startup_palette_saved = 0;
static int g_sysop_framebuffer_debug_enabled = 0;
static int g_sysop_framebuffer_split_enabled = 0;
static int g_sysop_framebuffer_split_demo_enabled = 0;
static int g_sysop_framebuffer_split_width = SCREENWIDTH / 2;
static int g_sysop_framebuffer_split_demo_direction = 1;

uint8_t sysop_read_joystick(uint8_t joystick_number);

#ifndef VIC_CHIP_6567R56A
#define VIC_CHIP_6567R56A 0
#endif
#ifndef VIC_CHIP_6567R8
#define VIC_CHIP_6567R8 1
#endif
#ifndef VIC_CHIP_6569
#define VIC_CHIP_6569 2
#endif
#ifndef VIC_CHIP_6572RO_DREAN
#define VIC_CHIP_6572RO_DREAN 3
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Shadow C64 memory to avoid sending redundant pokes. Most frame bandwidth is
// spent on bitmap/screen/color RAM, so skipping identical bytes matters.
uint8_t shadow_buffer[65536];

// Write a C64 byte only when it differs from the shadow copy, saving tagged
// DMA bandwidth on repeated frames.
void Poke(uint16_t address, uint8_t value) {
    if (shadow_buffer[address] != value)
    {
        sysop_poke(address, value);
        shadow_buffer[address] = value;
    }
}

// Keep raster waits explicit so the frame scheduler can place them in the
// tagged DMA stream at the point the C64 should observe them.
void Wait_vic2(uint16_t address, uint8_t value) {
    sysop_wait_vic2(address, value);
}

// The FPGA consumes a tagged DMA stream. Wait for the previous frame tag before
// emitting the next one so the C64 side sees stable pacing even if Linux gets
// ahead for a moment.
static void BeginFrameDmaTag(void) {
    static uint32_t frame_tag_data = 0;

    if (frame_tag_data != 0) {
        uint32_t tag = sysop_dma_tag_data();
        while (tag != frame_tag_data) {
            tag = sysop_dma_tag_data();
            usleep(50);
        }
    }

    frame_tag_data++;
    sysop_dma_write_tag(frame_tag_data);
}

#include <memory.h>
#include <sys/mman.h>

// C64 memory layout used by the converted Koala-style frame. D011/D016/D018
// are written every frame so the real VIC output is kept in bitmap multicolor
// mode even if prior code or a reset path disturbed the registers.
#define VIC_CTRL_REG1   0xD011 // Vertical scroll and control
#define VIC_CTRL_REG2   0xD016 // Horizontal scroll and control
#define VIC_MEM_CTRL    0xD018 // Memory control for screen and bitmap pointers

// Screen and Border Colors
#define BORDER_COLOR_REG 0xD020 // Border color
#define BG_COLOR_REG0    0xD021 // Background color 0

// Bitmap data and color RAM locations
#define BITMAP_MEMORY   0x2000 // Start of bitmap data (configurable)
#define SCREEN_RAM      0x0C00 // Start of screen RAM (for bitmap color data)
//#define SCREEN_RAM      0x0400 // Start of screen RAM (for bitmap color data)
//#define BITMAP_MEMORY   0x4000 // Start of bitmap data (configurable)
//#define SCREEN_RAM      0x6000 // Start of screen RAM (for bitmap color data)
#define COLOR_RAM       0xD800 // Color RAM for text/bitmap modes

#define C64_CIA2_BANK0          0x97
#define C64_BITMAP_D011         0x3B
#define C64_MULTICOLOR_D016     0xD8
#define C64_BITMAP_D018         0x38
#define C64_BITMAP_FIRST_BADLINE 51
#define C64_POKE_SAFE_CYCLE_START 11
#define C64_POKE_SAFE_CYCLE_END   56
#define C64_BITMAP_CHAR_ROWS 25
#define C64_BITMAP_CHAR_COLS 40
#define C64_BITMAP_BYTES_PER_CELL 8
#define C64_BITMAP_ROW_BYTES (C64_BITMAP_CHAR_COLS * C64_BITMAP_BYTES_PER_CELL)
#define DEFAULT_SID_PATH "At_Dooms_Gate.sid"
#define MAX_SID_WRITES_PER_FRAME 512
#define SID_PAL_FRAME_RATE 50.125
#define SID_NTSC_FRAME_RATE 59.826

// VIC timing drives both the bitmap poke scheduler and SID frame cadence. Start
// with PAL-safe defaults and replace them once the FPGA reports a VIC model.
static int g_cyclesPerLine = 63;
static int g_vic_lines = 312;
static int g_vic_model = VIC_CHIP_6569;
static int g_sid_is_pal_machine = 1;
int g_sysop_sid_music_volume = SYSOP_DOOM_VOLUME_MAX;
static int g_sysop_sid_enabled = 1;
static int g_sysop_sid_path_from_arg = 0;
static char g_default_sid_path[PATH_MAX] = DEFAULT_SID_PATH;
static double g_sid_frame_interval_us = 1000000.0 / SID_PAL_FRAME_RATE;
static pthread_t g_sid_thread;
static volatile int g_sid_thread_running = 0;
static int g_sid_thread_started = 0;
int g_sysop_key_debug = 0;
int g_sysop_mouse_enabled = 0;
int g_sysop_joystick_enabled = 0;
static int g_sysop_mouse_wasd_enabled = -1;
static int g_sysop_mouse_novert = -1;
int g_sysop_display_tune_enabled = 0;
int g_sysop_pcm_sfx_enabled = 1;
static int g_sysop_backend_args_parsed = 0;
static int g_sysop_start_with_idkfa = 0;
static mobj_t *g_sysop_idkfa_applied_mo = NULL;

static int g_sysop_menu_dither_mode = SYSOP_MENU_DITHER_SHARP;

#define SYSOP_MENU_PALETTE_OFF 0
#define SYSOP_MENU_PALETTE_SHARP 1
static int g_sysop_menu_palette_mode = SYSOP_MENU_PALETTE_SHARP;
static int g_sysop_menu_palette_active = -1;
static unsigned int g_sysop_palette_revision = 1;

// Apply the Sysop startup cheat once per live player object when --idkfa was
// requested.
static void Sysop_MaybeApplyIDKFA(void)
{
    player_t *player;

    if (!g_sysop_start_with_idkfa || netgame || demoplayback || demorecording
        || gamestate != GS_LEVEL || !playeringame[consoleplayer]) {
        g_sysop_idkfa_applied_mo = NULL;
        return;
    }

    player = &players[consoleplayer];
    if (player->playerstate != PST_LIVE || player->mo == NULL) {
        g_sysop_idkfa_applied_mo = NULL;
        return;
    }

    if (g_sysop_idkfa_applied_mo == player->mo) {
        return;
    }

    player->armorpoints = deh_idkfa_armor;
    player->armortype = deh_idkfa_armor_class;

    for (int i = 0; i < NUMWEAPONS; ++i) {
        player->weaponowned[i] = true;
    }

    for (int i = 0; i < NUMAMMO; ++i) {
        player->ammo[i] = player->maxammo[i];
    }

    for (int i = 0; i < NUMCARDS; ++i) {
        player->cards[i] = true;
    }

    player->message = DEH_String(STSTR_KFAADDED);
    g_sysop_idkfa_applied_mo = player->mo;
    printf("Sysop --idkfa: granted all weapons, ammo, armor, and keys.\n");
}

// Clamp tuner values before they are passed into the image-conversion option
// parser.
static int Sysop_TuneClampInt(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

// Set an integer mega-converter option through the same string API used by the
// HTTP tuner and command line.
static void Sysop_SetMegaIntOption(const char *name, int value)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%d", value);
    Sysop_ImageMegaSetOption(name, buffer);
}

// Print a compact summary of the currently hot-tuned converter settings.
static void Sysop_PrintMegaTuneState(void)
{
    SysopMegaOptions options;

    Sysop_ImageMegaGetOptions(&options);
    printf("Mega tune: dither=%s strength=%d brightness=%d contrast=%d fast_tables=%s\n",
           Sysop_ImageMegaDitherName(options.dither_pattern),
           options.dither_strength,
           options.brightness,
           options.contrast,
           options.fast_tables ? "on" : "off");
}

// Consume optional keyboard shortcuts for live display tuning before they reach
// Doom's normal input handling.
int Sysop_HandleDisplayTuneKey(uint8_t raw_char)
{
    SysopMegaOptions options;

    if (!g_sysop_display_tune_enabled) {
        return 0;
    }

    Sysop_ImageMegaGetOptions(&options);

    switch (raw_char) {
        case 't':
            Sysop_SetMegaIntOption("dither", (options.dither_pattern + 1) % (SYSOP_MEGA_DITHER_HASH + 1));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'a':
            Sysop_SetMegaIntOption("dither",
                                   Sysop_TuneClampInt(options.dither_pattern - 1,
                                                      SYSOP_MEGA_DITHER_OFF,
                                                      SYSOP_MEGA_DITHER_HASH));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'q':
            Sysop_SetMegaIntOption("dither",
                                   Sysop_TuneClampInt(options.dither_pattern + 1,
                                                      SYSOP_MEGA_DITHER_OFF,
                                                      SYSOP_MEGA_DITHER_HASH));
            Sysop_PrintMegaTuneState();
            return 1;
        case 's':
            Sysop_SetMegaIntOption("contrast", Sysop_TuneClampInt(options.contrast - 5, 25, 250));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'w':
            Sysop_SetMegaIntOption("contrast", Sysop_TuneClampInt(options.contrast + 5, 25, 250));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'd':
            Sysop_SetMegaIntOption("brightness", Sysop_TuneClampInt(options.brightness - 2, -96, 96));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'e':
            Sysop_SetMegaIntOption("brightness", Sysop_TuneClampInt(options.brightness + 2, -96, 96));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'z':
            Sysop_SetMegaIntOption("dither_strength", Sysop_TuneClampInt(options.dither_strength - 5, 0, 150));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'x':
            Sysop_SetMegaIntOption("dither_strength", Sysop_TuneClampInt(options.dither_strength + 5, 0, 150));
            Sysop_PrintMegaTuneState();
            return 1;
        case 'r':
            Sysop_ImageMegaResetOptions();
            Sysop_PrintMegaTuneState();
            return 1;
        case 'f':
            Sysop_SetMegaIntOption("fast_tables", options.fast_tables ? 0 : 1);
            Sysop_PrintMegaTuneState();
            return 1;
        default:
            break;
    }

    return 0;
}

// Clamp Chocolate Doom's menu volume range to the bounds expected by the Sysop
// audio helpers.
int Sysop_ClampDoomVolume(int volume)
{
    if (volume < 0) {
        return 0;
    }

    if (volume > SYSOP_DOOM_VOLUME_MAX) {
        return SYSOP_DOOM_VOLUME_MAX;
    }

    return volume;
}

// Convert Chocolate Doom's 0-15 style volume scale into the Sysop hardware
// volume range.
uint32_t Sysop_DoomVolumeToSysopVolume(int volume)
{
    volume = Sysop_ClampDoomVolume(volume);

    if (volume >= SYSOP_DOOM_MENU_VOLUME_MAX) {
        return SYSOP_AUDIO_VOLUME_MAX;
    }

    return (uint32_t)((volume * SYSOP_AUDIO_VOLUME_MAX
                     + (SYSOP_DOOM_MENU_VOLUME_MAX / 2))
                    / SYSOP_DOOM_MENU_VOLUME_MAX);
}

// Push the current music volume slider value to the Sysop SID mixer.
void Sysop_ApplySidMusicVolume(void)
{
    uint32_t volume;

    if (!Sysop_LibraryIsInitialized()) {
        return;
    }

    volume = Sysop_DoomVolumeToSysopVolume(g_sysop_sid_music_volume);
    sysop_audio_set_sid_volume(volume, volume);
}

static unsigned int g_sysop_runtime_palette_revision = 0;
static int g_sysop_clean_status_enabled = 0;
static int g_sysop_clean_intermission_enabled = 0;

// Enable every C64-native readability overlay controlled by --sysop-clean-all.
static void enable_sysop_clean_all(void)
{
    g_sysop_hud_mode = SYSOP_HUD_MODE_CLEAN_BITMAP;
    sysop_hud_messages_enabled = 1;
    sysop_clean_menu_enabled = 1;
    g_sysop_clean_status_enabled = 1;
    g_sysop_clean_intermission_enabled = 1;
}

// Detect clean-all aliases that may be queried after the first argument pass.
static int sysop_clean_all_arg_present(void)
{
    return M_CheckParm("--sysop-clean-all") > 0
        || M_CheckParm("-sysop-clean-all") > 0
        || M_CheckParm("--sysop-all-clean") > 0
        || M_CheckParm("-sysop-all-clean") > 0
        || M_CheckParm("--sysop-clean=all") > 0
        || M_CheckParm("-sysop-clean=all") > 0;
}

// Treat explicit clean-menu and clean-all requests as the same menu overlay
// decision.
static int sysop_clean_menu_requested(void)
{
    return sysop_clean_menu_enabled || sysop_clean_all_arg_present();
}

// Command-line SID paths must survive the later executable-relative default
// path setup. Mark explicit paths so configure_default_sid_path() cannot
// accidentally replace them with At_Dooms_Gate.sid.
static void set_sysop_sid_path(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        return;
    }

    snprintf(g_default_sid_path, sizeof(g_default_sid_path), "%s", path);
    g_sysop_sid_enabled = 1;
    g_sysop_sid_path_from_arg = 1;
}

// Parse a name=value mega-converter option and forward it to the image module.
static void apply_sysop_mega_option_pair(const char *pair)
{
    char buffer[128];
    char *equals;

    if (pair == NULL || pair[0] == '\0') {
        return;
    }

    snprintf(buffer, sizeof(buffer), "%s", pair);
    equals = strchr(buffer, '=');
    if (equals == NULL) {
        return;
    }

    *equals++ = '\0';
    Sysop_ImageMegaSetOption(buffer, equals);
}

// Parse Sysop-owned command line switches without consuming Chocolate Doom's
// normal game, WAD, and configuration options.
static void parse_sysop_backend_args(int argc, char **argv) {
    int clean_all_requested = 0;

    // Parse only options owned by the sysop backend. Unrecognized arguments are
    // intentionally left alone for Chocolate Doom's normal option handling.
    for (int i = 1; i < argc; i++) {
        const char *hud_arg = NULL;
        const char *menu_arg = NULL;
        const char *palette_arg = NULL;
        const char *display_arg = NULL;
        const char *menu_dither_arg = NULL;
        const char *menu_palette_arg = NULL;
        const char *status_arg = NULL;
        const char *intermission_arg = NULL;
        const char *mega_http_arg = NULL;
        const char *mega_option_name = NULL;
        const char *mega_option_value = NULL;

        if (!strcmp(argv[i], "--idkfa") || !strcmp(argv[i], "-idkfa")
            || !strcmp(argv[i], "--sysop-idkfa") || !strcmp(argv[i], "-sysop-idkfa")) {
            g_sysop_start_with_idkfa = 1;
            continue;
        }
        if (!strcmp(argv[i], "--sysop-key-debug") || !strcmp(argv[i], "-sysop-key-debug")) {
            g_sysop_key_debug = 1;
            continue;
        }

        if ((!strcmp(argv[i], "--sysop-sid")
             || !strcmp(argv[i], "-sysop-sid")) && i + 1 < argc) {
            set_sysop_sid_path(argv[++i]);
            continue;
        }

        if (!strncmp(argv[i], "--sysop-sid=", 12)) {
            set_sysop_sid_path(argv[i] + 12);
            continue;
        }

        if (!strncmp(argv[i], "-sysop-sid=", 11)) {
            set_sysop_sid_path(argv[i] + 11);
            continue;
        }

        if (!strcmp(argv[i], "--sysop-no-sid")
            || !strcmp(argv[i], "-sysop-no-sid")
            || !strcmp(argv[i], "--sysop-sid-off")
            || !strcmp(argv[i], "-sysop-sid-off")) {
            g_sysop_sid_enabled = 0;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-mouse") || !strcmp(argv[i], "-sysop-mouse")) {
            g_sysop_mouse_enabled = 1;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-joystick")
            || !strcmp(argv[i], "--sysop-c64-joystick")
            || !strcmp(argv[i], "--sysop-joy")
            || !strcmp(argv[i], "-sysop-joystick")
            || !strcmp(argv[i], "-sysop-c64-joystick")
            || !strcmp(argv[i], "-sysop-joy")) {
            g_sysop_joystick_enabled = 1;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-no-joystick")
            || !strcmp(argv[i], "--sysop-joystick-off")
            || !strcmp(argv[i], "--sysop-no-c64-joystick")
            || !strcmp(argv[i], "--sysop-c64-joystick-off")
            || !strcmp(argv[i], "-sysop-no-joystick")
            || !strcmp(argv[i], "-sysop-joystick-off")
            || !strcmp(argv[i], "-sysop-no-c64-joystick")
            || !strcmp(argv[i], "-sysop-c64-joystick-off")) {
            g_sysop_joystick_enabled = 0;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-no-mouse") || !strcmp(argv[i], "--sysop-mouse-off")
            || !strcmp(argv[i], "-sysop-no-mouse") || !strcmp(argv[i], "-sysop-mouse-off")) {
            g_sysop_mouse_enabled = 0;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-mouse-turn-only")
            || !strcmp(argv[i], "--sysop-mouse-novert")
            || !strcmp(argv[i], "--sysop-no-mouse-y")
            || !strcmp(argv[i], "-sysop-mouse-turn-only")
            || !strcmp(argv[i], "-sysop-mouse-novert")
            || !strcmp(argv[i], "-sysop-no-mouse-y")) {
            g_sysop_mouse_novert = 1;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-mouse-vertical")
            || !strcmp(argv[i], "--sysop-mouse-forward")
            || !strcmp(argv[i], "--sysop-mouse-y")
            || !strcmp(argv[i], "-sysop-mouse-vertical")
            || !strcmp(argv[i], "-sysop-mouse-forward")
            || !strcmp(argv[i], "-sysop-mouse-y")) {
            g_sysop_mouse_novert = 0;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-mouse-wasd") || !strcmp(argv[i], "--sysop-wasd")
            || !strcmp(argv[i], "-sysop-mouse-wasd") || !strcmp(argv[i], "-sysop-wasd")) {
            g_sysop_mouse_wasd_enabled = 1;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-no-mouse-wasd") || !strcmp(argv[i], "--sysop-no-wasd")
            || !strcmp(argv[i], "-sysop-no-mouse-wasd") || !strcmp(argv[i], "-sysop-no-wasd")) {
            g_sysop_mouse_wasd_enabled = 0;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-display-tune") || !strcmp(argv[i], "-sysop-display-tune")) {
            g_sysop_display_tune_enabled = 1;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-no-display-tune") || !strcmp(argv[i], "-sysop-no-display-tune")) {
            g_sysop_display_tune_enabled = 0;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-mega-http")
            || !strcmp(argv[i], "-sysop-mega-http")
            || !strcmp(argv[i], "--sysop-tune-http")
            || !strcmp(argv[i], "-sysop-tune-http")) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                mega_http_arg = argv[++i];
            } else {
                mega_http_arg = NULL;
            }
        } else if (!strncmp(argv[i], "--sysop-mega-http=", 18)) {
            mega_http_arg = argv[i] + 18;
        } else if (!strncmp(argv[i], "-sysop-mega-http=", 17)) {
            mega_http_arg = argv[i] + 17;
        } else if (!strncmp(argv[i], "--sysop-tune-http=", 18)) {
            mega_http_arg = argv[i] + 18;
        } else if (!strncmp(argv[i], "-sysop-tune-http=", 17)) {
            mega_http_arg = argv[i] + 17;
        } else if (!strcmp(argv[i], "--sysop-no-mega-http")
                   || !strcmp(argv[i], "-sysop-no-mega-http")
                   || !strcmp(argv[i], "--sysop-no-tune-http")
                   || !strcmp(argv[i], "-sysop-no-tune-http")) {
            Sysop_TuneHttpConfigure("off");
            continue;
        }

        if (mega_http_arg != NULL || !strcmp(argv[i], "--sysop-mega-http")
            || !strcmp(argv[i], "-sysop-mega-http")
            || !strcmp(argv[i], "--sysop-tune-http")
            || !strcmp(argv[i], "-sysop-tune-http")) {
            Sysop_TuneHttpConfigure(mega_http_arg);
            continue;
        }

        if ((!strcmp(argv[i], "--sysop-mega-set")
             || !strcmp(argv[i], "-sysop-mega-set")) && i + 1 < argc) {
            apply_sysop_mega_option_pair(argv[++i]);
            continue;
        } else if (!strncmp(argv[i], "--sysop-mega-set=", 17)) {
            apply_sysop_mega_option_pair(argv[i] + 17);
            continue;
        } else if (!strncmp(argv[i], "-sysop-mega-set=", 16)) {
            apply_sysop_mega_option_pair(argv[i] + 16);
            continue;
        }

        if ((!strcmp(argv[i], "--sysop-mega-dither")
             || !strcmp(argv[i], "-sysop-mega-dither")) && i + 1 < argc) {
            mega_option_name = "dither";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-dither=", 20)) {
            mega_option_name = "dither";
            mega_option_value = argv[i] + 20;
        } else if ((!strcmp(argv[i], "--sysop-mega-palette")
                    || !strcmp(argv[i], "-sysop-mega-palette")) && i + 1 < argc) {
            mega_option_name = "palette";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-palette=", 21)) {
            mega_option_name = "palette";
            mega_option_value = argv[i] + 21;
        } else if (!strncmp(argv[i], "-sysop-mega-palette=", 20)) {
            mega_option_name = "palette";
            mega_option_value = argv[i] + 20;
        } else if ((!strcmp(argv[i], "--sysop-mega-strength")
                    || !strcmp(argv[i], "-sysop-mega-strength")) && i + 1 < argc) {
            mega_option_name = "dither_strength";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-strength=", 22)) {
            mega_option_name = "dither_strength";
            mega_option_value = argv[i] + 22;
        } else if ((!strcmp(argv[i], "--sysop-mega-brightness")
                    || !strcmp(argv[i], "-sysop-mega-brightness")) && i + 1 < argc) {
            mega_option_name = "brightness";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-brightness=", 24)) {
            mega_option_name = "brightness";
            mega_option_value = argv[i] + 24;
        } else if ((!strcmp(argv[i], "--sysop-mega-contrast")
                    || !strcmp(argv[i], "-sysop-mega-contrast")) && i + 1 < argc) {
            mega_option_name = "contrast";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-contrast=", 22)) {
            mega_option_name = "contrast";
            mega_option_value = argv[i] + 22;
        } else if ((!strcmp(argv[i], "--sysop-mega-gamma")
                    || !strcmp(argv[i], "-sysop-mega-gamma")) && i + 1 < argc) {
            mega_option_name = "gamma";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-gamma=", 19)) {
            mega_option_name = "gamma";
            mega_option_value = argv[i] + 19;
        } else if ((!strcmp(argv[i], "--sysop-mega-saturation")
                    || !strcmp(argv[i], "-sysop-mega-saturation")) && i + 1 < argc) {
            mega_option_name = "saturation";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-saturation=", 24)) {
            mega_option_name = "saturation";
            mega_option_value = argv[i] + 24;
        } else if ((!strcmp(argv[i], "--sysop-mega-vibrance")
                    || !strcmp(argv[i], "-sysop-mega-vibrance")) && i + 1 < argc) {
            mega_option_name = "vibrance";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-vibrance=", 22)) {
            mega_option_name = "vibrance";
            mega_option_value = argv[i] + 22;
        } else if ((!strcmp(argv[i], "--sysop-mega-detail")
                    || !strcmp(argv[i], "-sysop-mega-detail")) && i + 1 < argc) {
            mega_option_name = "detail";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-detail=", 20)) {
            mega_option_name = "detail";
            mega_option_value = argv[i] + 20;
        } else if ((!strcmp(argv[i], "--sysop-mega-surface")
                    || !strcmp(argv[i], "-sysop-mega-surface")) && i + 1 < argc) {
            mega_option_name = "surface";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-surface=", 21)) {
            mega_option_name = "surface";
            mega_option_value = argv[i] + 21;
        } else if ((!strcmp(argv[i], "--sysop-mega-black")
                    || !strcmp(argv[i], "-sysop-mega-black")) && i + 1 < argc) {
            mega_option_name = "black";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-black=", 19)) {
            mega_option_name = "black";
            mega_option_value = argv[i] + 19;
        } else if ((!strcmp(argv[i], "--sysop-mega-yellow")
                    || !strcmp(argv[i], "-sysop-mega-yellow")) && i + 1 < argc) {
            mega_option_name = "yellow";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-yellow=", 20)) {
            mega_option_name = "yellow";
            mega_option_value = argv[i] + 20;
        } else if ((!strcmp(argv[i], "--sysop-mega-neutral")
                    || !strcmp(argv[i], "-sysop-mega-neutral")) && i + 1 < argc) {
            mega_option_name = "neutral";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-neutral=", 21)) {
            mega_option_name = "neutral";
            mega_option_value = argv[i] + 21;
        } else if ((!strcmp(argv[i], "--sysop-mega-luma")
                    || !strcmp(argv[i], "-sysop-mega-luma")) && i + 1 < argc) {
            mega_option_name = "luma";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-luma=", 18)) {
            mega_option_name = "luma";
            mega_option_value = argv[i] + 18;
        } else if ((!strcmp(argv[i], "--sysop-mega-chroma")
                    || !strcmp(argv[i], "-sysop-mega-chroma")) && i + 1 < argc) {
            mega_option_name = "chroma";
            mega_option_value = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-mega-chroma=", 20)) {
            mega_option_name = "chroma";
            mega_option_value = argv[i] + 20;
        }

        if (mega_option_name != NULL) {
            Sysop_ImageMegaSetOption(mega_option_name, mega_option_value);
            continue;
        }

        if ((!strcmp(argv[i], "--sysop-display")
             || !strcmp(argv[i], "-sysop-display")) && i + 1 < argc) {
            display_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-display=", 16)) {
            display_arg = argv[i] + 16;
        } else if (!strncmp(argv[i], "-sysop-display=", 15)) {
            display_arg = argv[i] + 15;
        } else if (!strcmp(argv[i], "--sysop-framebuffer-debug")
                   || !strcmp(argv[i], "-sysop-framebuffer-debug")
                   || !strcmp(argv[i], "--sysop-fb-debug")
                   || !strcmp(argv[i], "-sysop-fb-debug")) {
            display_arg = "framebuffer";
        } else if (!strcmp(argv[i], "--sysop-framebuffer-split")
                   || !strcmp(argv[i], "-sysop-framebuffer-split")
                   || !strcmp(argv[i], "--sysop-fb-split")
                   || !strcmp(argv[i], "-sysop-fb-split")) {
            display_arg = "split";
        } else if (!strcmp(argv[i], "--sysop-framebuffer-split-demo")
                   || !strcmp(argv[i], "-sysop-framebuffer-split-demo")
                   || !strcmp(argv[i], "--sysop-fb-split-demo")
                   || !strcmp(argv[i], "-sysop-fb-split-demo")) {
            display_arg = "split-demo";
        } else if (!strcmp(argv[i], "--sysop-no-framebuffer-debug")
                   || !strcmp(argv[i], "-sysop-no-framebuffer-debug")
                   || !strcmp(argv[i], "--sysop-c64-display")
                   || !strcmp(argv[i], "-sysop-c64-display")) {
            display_arg = "c64";
        }

        if (display_arg) {
            if (!strcmp(display_arg, "framebuffer")
                || !strcmp(display_arg, "fb")
                || !strcmp(display_arg, "debug")
                || !strcmp(display_arg, "raw")) {
                g_sysop_framebuffer_debug_enabled = 1;
                g_sysop_framebuffer_split_enabled = 0;
                g_sysop_framebuffer_split_demo_enabled = 0;
                g_sysop_hud_mode = SYSOP_HUD_MODE_OFF;
                sysop_hud_messages_enabled = 0;
            } else if (!strcmp(display_arg, "split-demo")
                       || !strcmp(display_arg, "framebuffer-split-demo")
                       || !strcmp(display_arg, "fb-split-demo")
                       || !strcmp(display_arg, "compare-demo")) {
                g_sysop_framebuffer_debug_enabled = 0;
                g_sysop_framebuffer_split_enabled = 1;
                g_sysop_framebuffer_split_demo_enabled = 1;
                g_sysop_framebuffer_split_width = 0;
                g_sysop_framebuffer_split_demo_direction = 1;
                g_sysop_hud_mode = SYSOP_HUD_MODE_OFF;
                sysop_hud_messages_enabled = 0;
            } else if (!strcmp(display_arg, "split")
                       || !strcmp(display_arg, "framebuffer-split")
                       || !strcmp(display_arg, "fb-split")
                       || !strcmp(display_arg, "compare")) {
                g_sysop_framebuffer_debug_enabled = 0;
                g_sysop_framebuffer_split_enabled = 1;
                g_sysop_framebuffer_split_demo_enabled = 0;
                g_sysop_framebuffer_split_width = SCREENWIDTH / 2;
                g_sysop_hud_mode = SYSOP_HUD_MODE_OFF;
                sysop_hud_messages_enabled = 0;
            } else if (!strcmp(display_arg, "c64")
                       || !strcmp(display_arg, "vic")
                       || !strcmp(display_arg, "bitmap")
                       || !strcmp(display_arg, "koala")
                       || !strcmp(display_arg, "normal")
                       || !strcmp(display_arg, "off")) {
                g_sysop_framebuffer_debug_enabled = 0;
                g_sysop_framebuffer_split_enabled = 0;
                g_sysop_framebuffer_split_demo_enabled = 0;
            }
            continue;
        }

        if (!strcmp(argv[i], "--sysop-pcm-sfx") || !strcmp(argv[i], "-sysop-pcm-sfx")
            || !strcmp(argv[i], "--sysop-audio-sfx") || !strcmp(argv[i], "-sysop-audio-sfx")) {
            g_sysop_pcm_sfx_enabled = 1;
            printf("Sysop PCM SFX: on\n");
            continue;
        }

        if (!strcmp(argv[i], "--sysop-no-pcm-sfx") || !strcmp(argv[i], "-sysop-no-pcm-sfx")
            || !strcmp(argv[i], "--sysop-pcm-sfx-off") || !strcmp(argv[i], "-sysop-pcm-sfx-off")
            || !strcmp(argv[i], "--sysop-no-audio-sfx") || !strcmp(argv[i], "-sysop-no-audio-sfx")) {
            g_sysop_pcm_sfx_enabled = 0;
            printf("Sysop PCM SFX: off; SID playback remains enabled\n");
            continue;
        }

        if (!strcmp(argv[i], "--sysop-clean-all")
            || !strcmp(argv[i], "-sysop-clean-all")
            || !strcmp(argv[i], "--sysop-all-clean")
            || !strcmp(argv[i], "-sysop-all-clean")
            || !strcmp(argv[i], "--sysop-clean=all")
            || !strcmp(argv[i], "-sysop-clean=all")) {
            enable_sysop_clean_all();
            clean_all_requested = 1;
            continue;
        }

        if (!strcmp(argv[i], "--sysop-menu-dither") && i + 1 < argc) {
            menu_dither_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-menu-dither=", 20)) {
            menu_dither_arg = argv[i] + 20;
        } else if (!strcmp(argv[i], "--sysop-menu-dither-sharp")
                   || !strcmp(argv[i], "--sysop-sharp-menu-dither")) {
            menu_dither_arg = "sharp";
        } else if (!strcmp(argv[i], "--sysop-menu-dither-off")) {
            menu_dither_arg = "off";
        }

        if (menu_dither_arg) {
            if (!strcmp(menu_dither_arg, "sharp") || !strcmp(menu_dither_arg, "lock")
                || !strcmp(menu_dither_arg, "locked") || !strcmp(menu_dither_arg, "c64")) {
                g_sysop_menu_dither_mode = SYSOP_MENU_DITHER_SHARP;
                printf("Sysop menu/intermission dither: soft C64 color locks\n");
            } else if (!strcmp(menu_dither_arg, "poster") || !strcmp(menu_dither_arg, "posterized")
                       || !strcmp(menu_dither_arg, "hard")) {
                g_sysop_menu_dither_mode = SYSOP_MENU_DITHER_POSTER;
                printf("Sysop menu/intermission dither: hard posterized C64 color locks\n");
            } else if (!strcmp(menu_dither_arg, "off") || !strcmp(menu_dither_arg, "normal")
                       || !strcmp(menu_dither_arg, "original") || !strcmp(menu_dither_arg, "default")) {
                g_sysop_menu_dither_mode = SYSOP_MENU_DITHER_OFF;
                printf("Sysop menu/intermission dither: normal frame dithering\n");
            }
            continue;
        }
        if (!strcmp(argv[i], "--sysop-menu-palette") && i + 1 < argc) {
            menu_palette_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-menu-palette=", 21)) {
            menu_palette_arg = argv[i] + 21;
        } else if (!strcmp(argv[i], "--sysop-menu-palette-sharp")
                   || !strcmp(argv[i], "--sysop-sharp-menu-palette")) {
            menu_palette_arg = "sharp";
        } else if (!strcmp(argv[i], "--sysop-menu-palette-off")) {
            menu_palette_arg = "off";
        }

        if (menu_palette_arg) {
            if (!strcmp(menu_palette_arg, "sharp") || !strcmp(menu_palette_arg, "menu")
                || !strcmp(menu_palette_arg, "tuned") || !strcmp(menu_palette_arg, "on")) {
                g_sysop_menu_palette_mode = SYSOP_MENU_PALETTE_SHARP;
                printf("Sysop menu/intermission palette: sharp red/gold tuning\n");
            } else if (!strcmp(menu_palette_arg, "off") || !strcmp(menu_palette_arg, "normal")
                       || !strcmp(menu_palette_arg, "original") || !strcmp(menu_palette_arg, "default")) {
                g_sysop_menu_palette_mode = SYSOP_MENU_PALETTE_OFF;
                printf("Sysop menu/intermission palette: normal frame palette\n");
            }
            continue;
        }
        if (!strcmp(argv[i], "--sysop-hud") && i + 1 < argc) {
            hud_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-hud=", 12)) {
            hud_arg = argv[i] + 12;
        } else if (!strcmp(argv[i], "--sysop-hud-fb")) {
            hud_arg = "fb";
        } else if (!strcmp(argv[i], "--sysop-clean-messages")
                   || !strcmp(argv[i], "--sysop-hud-clean")
                   || !strcmp(argv[i], "--sysop-messages-clean")) {
            hud_arg = "clean";
        } else if (!strcmp(argv[i], "--sysop-hud-bitmap")) {
            hud_arg = "bitmap";
        } else if (!strcmp(argv[i], "--sysop-hud-off")) {
            hud_arg = "off";
        }

        if (hud_arg) {
            if (!strcmp(hud_arg, "fb") || !strcmp(hud_arg, "framebuffer")) {
                g_sysop_hud_mode = SYSOP_HUD_MODE_FRAMEBUFFER;
            } else if (!strcmp(hud_arg, "clean") || !strcmp(hud_arg, "c64")
                       || !strcmp(hud_arg, "menu")) {
                g_sysop_hud_mode = SYSOP_HUD_MODE_CLEAN_BITMAP;
            } else if (!strcmp(hud_arg, "bitmap")) {
                g_sysop_hud_mode = SYSOP_HUD_MODE_BITMAP;
            } else if (!strcmp(hud_arg, "off") || !strcmp(hud_arg, "none")) {
                g_sysop_hud_mode = SYSOP_HUD_MODE_OFF;
            }
            sysop_hud_messages_enabled = (g_sysop_hud_mode != SYSOP_HUD_MODE_OFF);
            continue;
        }

        if (!strcmp(argv[i], "--sysop-status") && i + 1 < argc) {
            status_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-status=", 15)) {
            status_arg = argv[i] + 15;
        } else if (!strcmp(argv[i], "--sysop-clean-status")
                   || !strcmp(argv[i], "--sysop-status-clean")) {
            status_arg = "clean";
        } else if (!strcmp(argv[i], "--sysop-status-off")
                   || !strcmp(argv[i], "--sysop-doom-status")) {
            status_arg = "off";
        }

        if (status_arg) {
            if (!strcmp(status_arg, "clean") || !strcmp(status_arg, "c64")
                || !strcmp(status_arg, "native")) {
                g_sysop_clean_status_enabled = 1;
            } else if (!strcmp(status_arg, "off") || !strcmp(status_arg, "doom")
                       || !strcmp(status_arg, "original")
                       || !strcmp(status_arg, "default")) {
                g_sysop_clean_status_enabled = 0;
            }
            continue;
        }

        if ((!strcmp(argv[i], "--sysop-intermission")
             || !strcmp(argv[i], "-sysop-intermission")) && i + 1 < argc) {
            intermission_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-intermission=", 21)) {
            intermission_arg = argv[i] + 21;
        } else if (!strncmp(argv[i], "-sysop-intermission=", 20)) {
            intermission_arg = argv[i] + 20;
        } else if (!strcmp(argv[i], "--sysop-clean-intermission")
                   || !strcmp(argv[i], "--sysop-intermission-clean")
                   || !strcmp(argv[i], "-sysop-clean-intermission")
                   || !strcmp(argv[i], "-sysop-intermission-clean")) {
            intermission_arg = "clean";
        } else if (!strcmp(argv[i], "--sysop-intermission-off")
                   || !strcmp(argv[i], "--sysop-doom-intermission")
                   || !strcmp(argv[i], "-sysop-intermission-off")
                   || !strcmp(argv[i], "-sysop-doom-intermission")) {
            intermission_arg = "off";
        }

        if (intermission_arg) {
            if (!strcmp(intermission_arg, "clean") || !strcmp(intermission_arg, "c64")
                || !strcmp(intermission_arg, "native")) {
                g_sysop_clean_intermission_enabled = 1;
            } else if (!strcmp(intermission_arg, "off") || !strcmp(intermission_arg, "doom")
                       || !strcmp(intermission_arg, "original")
                       || !strcmp(intermission_arg, "default")) {
                g_sysop_clean_intermission_enabled = 0;
            }
            continue;
        }

        if (!strcmp(argv[i], "--sysop-menu") && i + 1 < argc) {
            menu_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-menu=", 13)) {
            menu_arg = argv[i] + 13;
        } else if (!strcmp(argv[i], "--sysop-clean-menu")
                   || !strcmp(argv[i], "--sysop-menu-clean")) {
            menu_arg = "clean";
        }

        if (menu_arg) {
            if (!strcmp(menu_arg, "clean") || !strcmp(menu_arg, "c64")
                || !strcmp(menu_arg, "simple")) {
                sysop_clean_menu_enabled = 1;
            } else if (!strcmp(menu_arg, "doom") || !strcmp(menu_arg, "original")
                       || !strcmp(menu_arg, "off") || !strcmp(menu_arg, "none")) {
                sysop_clean_menu_enabled = 0;
            }
            continue;
        }

        if (!strcmp(argv[i], "--sysop-palette") && i + 1 < argc) {
            palette_arg = argv[++i];
        } else if (!strncmp(argv[i], "--sysop-palette=", 16)) {
            palette_arg = argv[i] + 16;
        } else if (!strcmp(argv[i], "--sysop-palette-adaptive")) {
            palette_arg = "adaptive";
        } else if (!strcmp(argv[i], "--sysop-palette-status-split")
                   || !strcmp(argv[i], "--sysop-palette-split")) {
            palette_arg = "status-split";
        } else if (!strcmp(argv[i], "--sysop-palette-static")
                   || !strcmp(argv[i], "--sysop-palette-tuned")) {
            palette_arg = "default";
        } else if ((!strcmp(argv[i], "--sysop-palette-rate")
                    || !strcmp(argv[i], "--sysop-palette-interval")) && i + 1 < argc) {
            int interval = atoi(argv[++i]);
            if (interval > 0) {
                g_sysop_palette_update_interval = interval;
            }
            continue;
        } else if (!strncmp(argv[i], "--sysop-palette-rate=", 21)) {
            int interval = atoi(argv[i] + 21);
            if (interval > 0) {
                g_sysop_palette_update_interval = interval;
            }
            continue;
        } else if (!strncmp(argv[i], "--sysop-palette-interval=", 25)) {
            int interval = atoi(argv[i] + 25);
            if (interval > 0) {
                g_sysop_palette_update_interval = interval;
            }
            continue;
        }

        if (palette_arg) {
            if (!strcmp(palette_arg, "static")
                || !strcmp(palette_arg, "tuned")
                || !strcmp(palette_arg, "default")
                || !strcmp(palette_arg, "sysop")
                || !strcmp(palette_arg, "off")) {
                g_sysop_palette_effects = 0;
            } else if (!strcmp(palette_arg, "adaptive")) {
                g_sysop_palette_effects = SYSOP_PALETTE_EFFECT_ADAPTIVE;
            } else if (!strcmp(palette_arg, "status-split")
                       || !strcmp(palette_arg, "split")
                       || !strcmp(palette_arg, "statusbar")) {
                g_sysop_palette_effects = SYSOP_PALETTE_EFFECT_STATUS_SPLIT;
            } else if (!strcmp(palette_arg, "adaptive-status")
                       || !strcmp(palette_arg, "adaptive-split")
                       || !strcmp(palette_arg, "adaptive-status-split")) {
                g_sysop_palette_effects = SYSOP_PALETTE_EFFECT_ADAPTIVE | SYSOP_PALETTE_EFFECT_STATUS_SPLIT;
            }
        }
    }

    if (clean_all_requested) {
        printf("Sysop clean overlays: messages=%s menu=%s status=%s intermission=%s\n",
               g_sysop_hud_mode == SYSOP_HUD_MODE_CLEAN_BITMAP ? "clean" : "not-clean",
               sysop_clean_menu_requested() ? "clean" : "doom",
               g_sysop_clean_status_enabled ? "clean" : "doom",
               g_sysop_clean_intermission_enabled ? "clean" : "doom");
    }
}

// Lazily parse backend arguments once, since audio can initialize before
// graphics in Chocolate Doom's startup order.
void ensure_sysop_backend_args_parsed(int argc, char **argv) {
    if (!g_sysop_backend_args_parsed) {
        // Sound can initialize before graphics, so backend option parsing must
        // be idempotent and available to every sysop subsystem.
        parse_sysop_backend_args(argc, argv);
        g_sysop_backend_args_parsed = 1;
    }
}

// Recompute SID frame duration after PAL/NTSC detection changes the tune rate.
static void update_sid_frame_interval(void) {
    double frame_rate = g_sid_is_pal_machine ? SID_PAL_FRAME_RATE : SID_NTSC_FRAME_RATE;
    g_sid_frame_interval_us = 1000000.0 / frame_rate;
}

// Return a monotonic-enough microsecond timestamp for pacing SID frames.
static uint64_t get_ticks_us(void) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((uint64_t)tp.tv_sec * 1000000ULL) + (uint64_t)tp.tv_usec;
}

// Query the FPGA's VIC model and set raster timing values used by video and
// SID scheduling.
static int configure_vic_timing(void) {
    uint8_t vic_info = sysop_get_vic_info();

    // The same executable runs on PAL and NTSC machines. The FPGA reports the
    // VIC model after probing the real C64, and that result determines raster
    // line counts, cycles-per-line, and SID playback rate.
    if (!(vic_info & 0x80)) {
        printf("FPGA has not yet determined VIC model. Using PAL timing defaults.\n");
        g_cyclesPerLine = 63;
        g_vic_lines = 312;
        g_vic_model = VIC_CHIP_6569;
        g_sid_is_pal_machine = 1;
        update_sid_frame_interval();
        return -1;
    }

    printf("VIC Model: ");
    g_vic_model = vic_info & 0x7;
    switch (g_vic_model)
    {
        case VIC_CHIP_6567R56A:
            printf("NTSC OLD");
            g_cyclesPerLine = 64;
            g_vic_lines = 262;
            g_sid_is_pal_machine = 0;
            break;
        case VIC_CHIP_6567R8:
            printf("NTSC NEW");
            g_cyclesPerLine = 65;
            g_vic_lines = 263;
            g_sid_is_pal_machine = 0;
            break;
        case VIC_CHIP_6572RO_DREAN:
            printf("DREAN");
            g_cyclesPerLine = 65;
            g_vic_lines = 312;
            g_sid_is_pal_machine = 1;
            break;
        case VIC_CHIP_6569:
            printf("PAL");
            g_cyclesPerLine = 63;
            g_vic_lines = 312;
            g_sid_is_pal_machine = 1;
            break;
        default:
            printf("Unknown");
            g_cyclesPerLine = 63;
            g_vic_lines = 312;
            g_vic_model = VIC_CHIP_6569;
            g_sid_is_pal_machine = 1;
            break;
    }
    update_sid_frame_interval();
    printf(" (%d cycles/line, %d lines)\n", g_cyclesPerLine, g_vic_lines);

    return 0;
}

// Choose the default SID path beside the executable unless --sysop-sid already
// supplied an explicit file.
static void configure_default_sid_path(const char *argv0) {
    const char *slash;
    const char *backslash;
    const char *last_separator;
    size_t dir_len;

    if (g_sysop_sid_path_from_arg) {
        return;
    }

    // Without an explicit --sysop-sid, prefer a tune beside the executable so
    // copied deployments can keep the binary, WAD, and SID in one directory.
    if (!argv0 || !argv0[0]) {
        return;
    }

    slash = strrchr(argv0, '/');
    backslash = strrchr(argv0, '\\');
    last_separator = slash;
    if (backslash && (!last_separator || backslash > last_separator)) {
        last_separator = backslash;
    }

    if (!last_separator) {
        return;
    }

    dir_len = (size_t)(last_separator - argv0) + 1;
    if (dir_len + strlen(DEFAULT_SID_PATH) >= sizeof(g_default_sid_path)) {
        return;
    }

    memcpy(g_default_sid_path, argv0, dir_len);
    memcpy(g_default_sid_path + dir_len, DEFAULT_SID_PATH, sizeof(DEFAULT_SID_PATH));
}

// Load the configured SID tune and start asynchronous SID register playback.
static void init_doom_sid_player(void) {
    if (!g_sysop_sid_enabled) {
        printf("[SID] Playback disabled by --sysop-no-sid\n");
        return;
    }

    doom_sid_set_machine(g_sid_is_pal_machine);
    printf("[SID] Selected %s\n", g_default_sid_path);
    if (doom_sid_init(g_default_sid_path, g_sid_is_pal_machine)) {
        Sysop_ApplySidMusicVolume();
        start_sid_thread();
    }
}

// Clear SID voices before shutdown so a stuck note does not survive process
// exit.
static void silence_sid_registers(void) {
    for (uint16_t i = 0; i <= 0x18; ++i) {
        sysop_poke((uint16_t)(0xD400 + i), 0);
    }
}

// Generate one SID emulation frame and write the resulting SID register deltas
// to the C64.
static void play_sid_frame(void) {
    DoomSidWrite writes[MAX_SID_WRITES_PER_FRAME];
    int write_count;

    if (!doom_sid_is_loaded()) {
        return;
    }

    write_count = doom_sid_play_frame(writes,
                                      MAX_SID_WRITES_PER_FRAME,
                                      NULL,
                                      0,
                                      NULL);

    for (int i = 0; i < write_count; ++i) {
        if (writes[i].addr >= 0xD400 && writes[i].addr <= 0xD41F) {
            sysop_poke(writes[i].addr, writes[i].val);
        }
    }
}

// Worker thread that keeps SID playback at music cadence independent of video
// frame time.
static void *sid_thread_function(void *arg) {
    double next_sid_frame_us = (double)get_ticks_us();

    (void)arg;

    // SID writes are generated outside the video frame upload path. Keeping
    // them on their own cadence avoids tying music speed to render load.
    while (g_sid_thread_running) {
        uint64_t now_us = get_ticks_us();

        if ((double)now_us >= next_sid_frame_us) {
            play_sid_frame();
            next_sid_frame_us += g_sid_frame_interval_us;

            if (((double)now_us - next_sid_frame_us) > (g_sid_frame_interval_us * 4.0)) {
                next_sid_frame_us = (double)now_us + g_sid_frame_interval_us;
            }
        } else {
            uint64_t sleep_us = (uint64_t)(next_sid_frame_us - (double)now_us);
            if (sleep_us > 1000) {
                sleep_us = 1000;
            }
            usleep((useconds_t)sleep_us);
        }
    }

    return NULL;
}

// Start the SID worker after a tune has loaded successfully.
static void start_sid_thread(void) {
    if (g_sid_thread_started || !doom_sid_is_loaded()) {
        return;
    }

    g_sid_thread_running = 1;
    if (pthread_create(&g_sid_thread, NULL, sid_thread_function, NULL) != 0) {
        perror("Failed to create SID thread");
        g_sid_thread_running = 0;
        return;
    }

    g_sid_thread_started = 1;
    printf("[SID] Playback thread running at %.3f Hz\n", 1000000.0 / g_sid_frame_interval_us);
}

// Stop the SID worker and join it before releasing hardware resources.
static void stop_sid_thread(void) {
    if (!g_sid_thread_started) {
        return;
    }

    g_sid_thread_running = 0;
    pthread_join(g_sid_thread, NULL);
    g_sid_thread_started = 0;
}

// Put the real VIC into the fixed multicolor bitmap layout expected by the
// converter: bank 0, bitmap at $2000, screen RAM at $0c00, color RAM at $d800.
void setup_multicolor_bitmap_mode() {
    // Bank 0, screen RAM at $0c00, bitmap RAM at $2000.
    sysop_poke(VIC_CTRL_REG1, C64_BITMAP_D011 & ~0x10);
    sysop_poke(0xdd00, C64_CIA2_BANK0);
    sysop_poke(VIC_CTRL_REG2, C64_MULTICOLOR_D016);
    sysop_poke(VIC_MEM_CTRL, C64_BITMAP_D018);

    // Clear the bitmap and color RAM
    for (uint16_t i = 0; i < 8000; i++) {
        sysop_poke(BITMAP_MEMORY + i, 0x00);
    }
    for (uint16_t i = 0; i < 1000; i++) {
        sysop_poke(SCREEN_RAM + i, 0x00);
        sysop_poke(COLOR_RAM + i, 0x00);
    }

    sysop_poke(BG_COLOR_REG0, 0x00);
    sysop_poke(BORDER_COLOR_REG, 0x00);
    sysop_poke(VIC_CTRL_REG1, C64_BITMAP_D011);
}

uint8_t *charset;

static int first = 1;

static int sysop_active_palette[16][3];
static int sysop_status_palette[16][3];
static int sysop_menu_palette[16][3];
static int sysop_menu_status_palette[16][3];
static const int (*g_sysop_installed_base_palette)[3] = NULL;

// Clamp intermediate palette math back to an 8-bit HDMI color component.
static uint8_t clamp_u8_int(int v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

// Blend one palette component using integer math so runtime palette effects
// stay cheap.
static int mix_component(int a, int b, int b_weight, int denom)
{
    return (a * (denom - b_weight) + b * b_weight + denom / 2) / denom;
}

// Send a complete 16-color palette to the Sysop HDMI palette registers.
static void set_sysop_palette_from(const int palette[16][3])
{
    for (int i = 0; i < 16; i++) {
        sysop_set_palette_entry((uint8_t)i,
                          clamp_u8_int(palette[i][0]),
                          clamp_u8_int(palette[i][1]),
                          clamp_u8_int(palette[i][2]));
    }
}

// Snapshot the HDMI/C64 palette that was active before the Doom backend changes
// it, so shutdown can put the cartridge display state back where it started.
static void capture_sysop_startup_palette(void)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    for (int i = 0; i < 16; i++) {
        sysop_get_palette_entry((uint8_t)i, &r, &g, &b);
        g_sysop_startup_palette[i][0] = r;
        g_sysop_startup_palette[i][1] = g;
        g_sysop_startup_palette[i][2] = b;
    }

    g_sysop_startup_palette_saved = 1;
}

// Restore the palette captured at startup. This is intentionally independent
// from restore_sysop_palette_effects(), which restores Doom's own base palette.
static void restore_sysop_startup_palette(void)
{
    if (!g_sysop_startup_palette_saved || !Sysop_LibraryIsInitialized()) {
        return;
    }

    set_sysop_palette_from(g_sysop_startup_palette);
    g_sysop_startup_palette_saved = 0;
}

// Build a slightly brighter status-area palette for raster-sliced HDMI output.
static void build_status_split_palette(const int src[16][3], int dst[16][3])
{
    Sysop_ImageCopyPalette(dst, src);

    dst[2][0] = clamp_u8_int(src[2][0] + 32);
    dst[2][1] = clamp_u8_int(src[2][1] + 4);
    dst[2][2] = clamp_u8_int(src[2][2] - 6);

    dst[7][0] = clamp_u8_int(src[7][0] + 16);
    dst[7][1] = clamp_u8_int(src[7][1] + 14);
    dst[7][2] = clamp_u8_int(src[7][2] - 18);

    dst[8][0] = clamp_u8_int(src[8][0] + 30);
    dst[8][1] = clamp_u8_int(src[8][1] + 14);
    dst[8][2] = clamp_u8_int(src[8][2] - 4);

    dst[9][0] = clamp_u8_int(src[9][0] + 20);
    dst[9][1] = clamp_u8_int(src[9][1] + 10);
    dst[9][2] = clamp_u8_int(src[9][2]);

    dst[10][0] = clamp_u8_int(src[10][0] + 36);
    dst[10][1] = clamp_u8_int(src[10][1] + 18);
    dst[10][2] = clamp_u8_int(src[10][2] + 6);

    dst[11][0] = clamp_u8_int(src[11][0] + 8);
    dst[11][1] = clamp_u8_int(src[11][1] + 8);
    dst[11][2] = clamp_u8_int(src[11][2] + 8);

    dst[12][0] = clamp_u8_int(src[12][0] + 16);
    dst[12][1] = clamp_u8_int(src[12][1] + 16);
    dst[12][2] = clamp_u8_int(src[12][2] + 16);

    dst[15][0] = clamp_u8_int(src[15][0] + 22);
    dst[15][1] = clamp_u8_int(src[15][1] + 22);
    dst[15][2] = clamp_u8_int(src[15][2] + 22);
}

// Reinstall the active palette; kept as a small compatibility hook for code
// paths that expect a video palette refresh function.
void update_palette()
{
    set_sysop_palette_from(sysop_active_palette);
}


#define C64_HUD_TEXT_START_ROW 0
#define C64_HUD_TEXT_ROWS 2
#define C64_HUD_TEXT_RIGHT_MARGIN_COLS 0
#define C64_HUD_TEXT_FG_COLOR 1
#define C64_HUD_TEXT_BG_COLOR 0

// Return one 4-pixel-wide row from the compact HUD message font.
static uint8_t c64_hud_glyph_row(char ch, int row) {
    static const uint8_t letters[26][7] = {
        {0x6, 0x9, 0x9, 0xf, 0x9, 0x9, 0x9}, // A
        {0xe, 0x9, 0x9, 0xe, 0x9, 0x9, 0xe}, // B
        {0x7, 0x8, 0x8, 0x8, 0x8, 0x8, 0x7}, // C
        {0xe, 0x9, 0x9, 0x9, 0x9, 0x9, 0xe}, // D
        {0xf, 0x8, 0x8, 0xe, 0x8, 0x8, 0xf}, // E
        {0xf, 0x8, 0x8, 0xe, 0x8, 0x8, 0x8}, // F
        {0x7, 0x8, 0x8, 0xb, 0x9, 0x9, 0x7}, // G
        {0x9, 0x9, 0x9, 0xf, 0x9, 0x9, 0x9}, // H
        {0xe, 0x4, 0x4, 0x4, 0x4, 0x4, 0xe}, // I
        {0x1, 0x1, 0x1, 0x1, 0x9, 0x9, 0x6}, // J
        {0x9, 0xa, 0xc, 0x8, 0xc, 0xa, 0x9}, // K
        {0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0xf}, // L
        {0x9, 0xf, 0xf, 0x9, 0x9, 0x9, 0x9}, // M
        {0x9, 0xd, 0xd, 0xb, 0xb, 0x9, 0x9}, // N
        {0x6, 0x9, 0x9, 0x9, 0x9, 0x9, 0x6}, // O
        {0xe, 0x9, 0x9, 0xe, 0x8, 0x8, 0x8}, // P
        {0x6, 0x9, 0x9, 0x9, 0xb, 0xa, 0x5}, // Q
        {0xe, 0x9, 0x9, 0xe, 0xa, 0x9, 0x9}, // R
        {0x7, 0x8, 0x8, 0x6, 0x1, 0x1, 0xe}, // S
        {0xf, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4}, // T
        {0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0xf}, // U
        {0x9, 0x9, 0x9, 0x9, 0x9, 0x6, 0x6}, // V
        {0x9, 0x9, 0x9, 0x9, 0xf, 0xf, 0x9}, // W
        {0x9, 0x9, 0x6, 0x6, 0x6, 0x9, 0x9}, // X
        {0x9, 0x9, 0x9, 0x6, 0x4, 0x4, 0x4}, // Y
        {0xf, 0x1, 0x2, 0x4, 0x8, 0x8, 0xf}  // Z
    };
    static const uint8_t digits[10][7] = {
        {0x6, 0x9, 0xb, 0xd, 0x9, 0x9, 0x6},
        {0x4, 0xc, 0x4, 0x4, 0x4, 0x4, 0xe},
        {0xe, 0x1, 0x1, 0x6, 0x8, 0x8, 0xf},
        {0xe, 0x1, 0x1, 0x6, 0x1, 0x1, 0xe},
        {0x9, 0x9, 0x9, 0xf, 0x1, 0x1, 0x1},
        {0xf, 0x8, 0x8, 0xe, 0x1, 0x1, 0xe},
        {0x7, 0x8, 0x8, 0xe, 0x9, 0x9, 0x6},
        {0xf, 0x1, 0x2, 0x4, 0x4, 0x4, 0x4},
        {0x6, 0x9, 0x9, 0x6, 0x9, 0x9, 0x6},
        {0x6, 0x9, 0x9, 0x7, 0x1, 0x1, 0xe}
    };

    if (row < 0 || row >= 7) {
        return 0;
    }

    ch = (char)toupper((unsigned char)ch);
    if (ch >= 'A' && ch <= 'Z') {
        return letters[ch - 'A'][row];
    }
    if (ch >= '0' && ch <= '9') {
        return digits[ch - '0'][row];
    }

    switch (ch) {
        case '!': return ((const uint8_t[]){0x4, 0x4, 0x4, 0x4, 0x4, 0x0, 0x4})[row];
        case '?': return ((const uint8_t[]){0x6, 0x9, 0x1, 0x2, 0x4, 0x0, 0x4})[row];
        case '.': return ((const uint8_t[]){0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4})[row];
        case ',': return ((const uint8_t[]){0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x8})[row];
        case ':': return ((const uint8_t[]){0x0, 0x4, 0x0, 0x0, 0x4, 0x0, 0x0})[row];
        case ';': return ((const uint8_t[]){0x0, 0x4, 0x0, 0x0, 0x4, 0x4, 0x8})[row];
        case '\'': return ((const uint8_t[]){0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0})[row];
        case '"': return ((const uint8_t[]){0xa, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0})[row];
        case '-': return ((const uint8_t[]){0x0, 0x0, 0x0, 0xf, 0x0, 0x0, 0x0})[row];
        case '_': return ((const uint8_t[]){0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf})[row];
        case '+': return ((const uint8_t[]){0x0, 0x4, 0x4, 0xe, 0x4, 0x4, 0x0})[row];
        case '/': return ((const uint8_t[]){0x1, 0x1, 0x2, 0x4, 0x8, 0x8, 0x0})[row];
        case '<': return ((const uint8_t[]){0x1, 0x2, 0x4, 0x8, 0x4, 0x2, 0x1})[row];
        case '>': return ((const uint8_t[]){0x8, 0x4, 0x2, 0x1, 0x2, 0x4, 0x8})[row];
        case '(': return ((const uint8_t[]){0x2, 0x4, 0x8, 0x8, 0x8, 0x4, 0x2})[row];
        case ')': return ((const uint8_t[]){0x8, 0x4, 0x2, 0x2, 0x2, 0x4, 0x8})[row];
        case '[': return ((const uint8_t[]){0xe, 0x8, 0x8, 0x8, 0x8, 0x8, 0xe})[row];
        case ']': return ((const uint8_t[]){0xe, 0x2, 0x2, 0x2, 0x2, 0x2, 0xe})[row];
        case '=': return ((const uint8_t[]){0x0, 0x0, 0xf, 0x0, 0xf, 0x0, 0x0})[row];
        case '%': return ((const uint8_t[]){0x9, 0x1, 0x2, 0x4, 0x8, 0x9, 0x0})[row];
        default: return 0;
    }
}

// Expand a 4-bit HUD glyph row into one C64 multicolor bitmap byte.
static uint8_t c64_hud_bitmap_byte(uint8_t glyph_row) {
    uint8_t out = 0;

    for (int x = 0; x < 4; x++) {
        out <<= 2;
        out |= (glyph_row & (1 << (3 - x))) ? 0x01 : 0x02;
    }

    return out;
}

// Draw one clean HUD message character into the converted C64 bitmap frame.
static void c64_hud_draw_cell(int row, int col, char ch) {
    int char_row = C64_HUD_TEXT_START_ROW + row;
    int cell = char_row * C64_BITMAP_CHAR_COLS + col;
    int bitmap_offset = char_row * C64_BITMAP_ROW_BYTES + col * C64_BITMAP_BYTES_PER_CELL;

    sysop_c64_frame[8000 + cell] = (C64_HUD_TEXT_FG_COLOR << 4) | C64_HUD_TEXT_BG_COLOR;
    sysop_c64_frame[9000 + cell] = C64_HUD_TEXT_BG_COLOR;

    for (int y = 0; y < C64_BITMAP_BYTES_PER_CELL; y++) {
        sysop_c64_frame[bitmap_offset + y] = c64_hud_bitmap_byte(c64_hud_glyph_row(ch, y));
    }
}

// Wrap the current Doom message into the small top-of-bitmap HUD area.
static int c64_hud_layout_message(char lines[C64_HUD_TEXT_ROWS][C64_BITMAP_CHAR_COLS + 1],
                                  int line_lengths[C64_HUD_TEXT_ROWS]) {
    int row = 0;
    int col = 0;
    int used_rows = 0;

    for (int i = 0; i < C64_HUD_TEXT_ROWS; i++) {
        lines[i][0] = '\0';
        line_lengths[i] = 0;
    }

    if (hu_current_message_counter <= 0 || hu_current_message[0] == '\0') {
        return 0;
    }

    for (int i = 0; hu_current_message[i] && row < C64_HUD_TEXT_ROWS; i++) {
        char ch = hu_current_message[i];

        if (ch == '\n') {
            lines[row][col] = '\0';
            line_lengths[row] = col;
            used_rows = row + 1;
            row++;
            col = 0;
            continue;
        }

        if (col >= C64_BITMAP_CHAR_COLS) {
            lines[row][col] = '\0';
            line_lengths[row] = col;
            used_rows = row + 1;
            row++;
            col = 0;
            if (row >= C64_HUD_TEXT_ROWS) {
                break;
            }
        }

        lines[row][col++] = ch;
        lines[row][col] = '\0';
        line_lengths[row] = col;
        used_rows = row + 1;
    }

    return used_rows;
}

// Render Doom's message text using the older two-row bitmap overlay path.
static void draw_c64_legacy_hud_message_overlay(void) {
    char lines[C64_HUD_TEXT_ROWS][C64_BITMAP_CHAR_COLS + 1];
    int line_lengths[C64_HUD_TEXT_ROWS];
    int used_rows = c64_hud_layout_message(lines, line_lengths);

    if (used_rows <= 0) {
        return;
    }

    for (int row = 0; row < used_rows; row++) {
        int len = line_lengths[row];
        int start_col;

        if (len <= 0) {
            continue;
        }

        start_col = C64_BITMAP_CHAR_COLS - C64_HUD_TEXT_RIGHT_MARGIN_COLS - len;
        if (start_col < 0) {
            start_col = 0;
        }

        for (int col = 0; col < len && start_col + col < C64_BITMAP_CHAR_COLS; col++) {
            c64_hud_draw_cell(row, start_col + col, lines[row][col]);
        }
    }
}

#define SYSOP_MENU_BOX_MIN_COLS 18
#define SYSOP_MENU_BOX_MAX_COLS 38
#define SYSOP_MENU_MESSAGE_WRAP_COLS 34
#define SYSOP_MENU_MAX_TEXT_LINES 18
#define SYSOP_MENU_BORDER_COLOR 11
#define SYSOP_MENU_BG_COLOR 0
#define SYSOP_MENU_FG_COLOR 1
#define SYSOP_MENU_DISABLED_COLOR 12
#define SYSOP_MENU_SELECTED_FG_COLOR 0
#define SYSOP_MENU_SELECTED_BG_COLOR 7

// Clamp menu overlay geometry to the C64 bitmap's character-grid limits.
static int sysop_menu_clamp_int(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

// Measure menu text while enforcing the overlay's fixed line length.
static int sysop_menu_text_len(const char *text, int max_len) {
    int len = 0;

    if (!text) {
        return 0;
    }

    while (text[len] && len < max_len) {
        len++;
    }

    return len;
}

// Return one row from the larger clean menu/intermission/status font.
static uint8_t c64_menu_glyph_row(char ch, int row) {
    static const uint8_t letters[26][7] = {
        {0x4, 0xa, 0xa, 0xe, 0xa, 0xa, 0xa}, // A
        {0xc, 0xa, 0xa, 0xc, 0xa, 0xa, 0xc}, // B
        {0xe, 0x8, 0x8, 0x8, 0x8, 0x8, 0xe}, // C
        {0xc, 0xa, 0xa, 0xa, 0xa, 0xa, 0xc}, // D
        {0xe, 0x8, 0x8, 0xc, 0x8, 0x8, 0xe}, // E
        {0xe, 0x8, 0x8, 0xc, 0x8, 0x8, 0x8}, // F
        {0xe, 0x8, 0x8, 0xa, 0xa, 0xa, 0xe}, // G
        {0xa, 0xa, 0xa, 0xe, 0xa, 0xa, 0xa}, // H
        {0xe, 0x4, 0x4, 0x4, 0x4, 0x4, 0xe}, // I
        {0x2, 0x2, 0x2, 0x2, 0xa, 0xa, 0x4}, // J
        {0xa, 0xa, 0xc, 0x8, 0xc, 0xa, 0xa}, // K
        {0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0xe}, // L
        {0xa, 0xe, 0xe, 0xa, 0xa, 0xa, 0xa}, // M
        {0xa, 0xe, 0xe, 0xa, 0xa, 0xa, 0xa}, // N
        {0xe, 0xa, 0xa, 0xa, 0xa, 0xa, 0xe}, // O
        {0xc, 0xa, 0xa, 0xc, 0x8, 0x8, 0x8}, // P
        {0xe, 0xa, 0xa, 0xa, 0xa, 0xc, 0x6}, // Q
        {0xc, 0xa, 0xa, 0xc, 0xc, 0xa, 0xa}, // R
        {0xe, 0x8, 0x8, 0xe, 0x2, 0x2, 0xe}, // S
        {0xe, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4}, // T
        {0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xe}, // U
        {0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x4}, // V
        {0xa, 0xa, 0xa, 0xa, 0xe, 0xe, 0xa}, // W
        {0xa, 0xa, 0x4, 0x4, 0x4, 0xa, 0xa}, // X
        {0xa, 0xa, 0xa, 0x4, 0x4, 0x4, 0x4}, // Y
        {0xe, 0x2, 0x2, 0x4, 0x8, 0x8, 0xe}  // Z
    };
    static const uint8_t digits[10][7] = {
        {0xe, 0xa, 0xa, 0xa, 0xa, 0xa, 0xe},
        {0x4, 0xc, 0x4, 0x4, 0x4, 0x4, 0xe},
        {0xe, 0x2, 0x2, 0xe, 0x8, 0x8, 0xe},
        {0xe, 0x2, 0x2, 0x6, 0x2, 0x2, 0xe},
        {0xa, 0xa, 0xa, 0xe, 0x2, 0x2, 0x2},
        {0xe, 0x8, 0x8, 0xe, 0x2, 0x2, 0xe},
        {0xe, 0x8, 0x8, 0xe, 0xa, 0xa, 0xe},
        {0xe, 0x2, 0x2, 0x4, 0x4, 0x4, 0x4},
        {0xe, 0xa, 0xa, 0xe, 0xa, 0xa, 0xe},
        {0xe, 0xa, 0xa, 0xe, 0x2, 0x2, 0xe}
    };

    if (row < 0 || row >= 7) {
        return 0;
    }

    ch = (char)toupper((unsigned char)ch);
    if (ch >= 'A' && ch <= 'Z') {
        return letters[ch - 'A'][row];
    }
    if (ch >= '0' && ch <= '9') {
        return digits[ch - '0'][row];
    }

    switch (ch) {
        case '!': return ((const uint8_t[]){0x4, 0x4, 0x4, 0x4, 0x4, 0x0, 0x4})[row];
        case '?': return ((const uint8_t[]){0xc, 0x2, 0x2, 0x4, 0x4, 0x0, 0x4})[row];
        case '.': return ((const uint8_t[]){0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4})[row];
        case ',': return ((const uint8_t[]){0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x8})[row];
        case ':': return ((const uint8_t[]){0x0, 0x4, 0x0, 0x0, 0x4, 0x0, 0x0})[row];
        case ';': return ((const uint8_t[]){0x0, 0x4, 0x0, 0x0, 0x4, 0x4, 0x8})[row];
        case '\'': return ((const uint8_t[]){0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0})[row];
        case '"': return ((const uint8_t[]){0xa, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0})[row];
        case '-': return ((const uint8_t[]){0x0, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0})[row];
        case '_': return ((const uint8_t[]){0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe})[row];
        case '+': return ((const uint8_t[]){0x0, 0x4, 0x4, 0xe, 0x4, 0x4, 0x0})[row];
        case '/': return ((const uint8_t[]){0x2, 0x2, 0x4, 0x4, 0x8, 0x8, 0x0})[row];
        case '<': return ((const uint8_t[]){0x2, 0x4, 0x8, 0x4, 0x2, 0x0, 0x0})[row];
        case '>': return ((const uint8_t[]){0x8, 0x4, 0x2, 0x4, 0x8, 0x0, 0x0})[row];
        case '(': return ((const uint8_t[]){0x4, 0x8, 0x8, 0x8, 0x8, 0x4, 0x0})[row];
        case ')': return ((const uint8_t[]){0x4, 0x2, 0x2, 0x2, 0x2, 0x4, 0x0})[row];
        case '[': return ((const uint8_t[]){0xc, 0x8, 0x8, 0x8, 0x8, 0x8, 0xc})[row];
        case ']': return ((const uint8_t[]){0x6, 0x2, 0x2, 0x2, 0x2, 0x2, 0x6})[row];
        case '=': return ((const uint8_t[]){0x0, 0x0, 0xe, 0x0, 0xe, 0x0, 0x0})[row];
        case '%': return ((const uint8_t[]){0xa, 0x2, 0x4, 0x4, 0x8, 0xa, 0x0})[row];
        default: return 0;
    }
}
// Draw one character cell with explicit foreground/background C64 colors.
static void c64_menu_draw_cell(int row, int col, char ch, uint8_t fg, uint8_t bg) {
    int cell;
    int bitmap_offset;

    if (row < 0 || row >= C64_BITMAP_CHAR_ROWS
        || col < 0 || col >= C64_BITMAP_CHAR_COLS) {
        return;
    }

    fg &= 0x0f;
    bg &= 0x0f;
    cell = row * C64_BITMAP_CHAR_COLS + col;
    bitmap_offset = row * C64_BITMAP_ROW_BYTES + col * C64_BITMAP_BYTES_PER_CELL;

    sysop_c64_frame[8000 + cell] = (fg << 4) | bg;
    sysop_c64_frame[9000 + cell] = bg;

    for (int y = 0; y < C64_BITMAP_BYTES_PER_CELL; y++) {
        sysop_c64_frame[bitmap_offset + y] = c64_hud_bitmap_byte(c64_menu_glyph_row(ch, y));
    }
}

// Fill a character-cell rectangle in the C64 bitmap frame.
static void c64_menu_fill_rect(int x, int y, int width, int height, uint8_t bg) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            c64_menu_draw_cell(y + row, x + col, ' ', SYSOP_MENU_FG_COLOR, bg);
        }
    }
}

// Draw a clipped string using the clean C64 menu font.
static void c64_menu_draw_text(int row,
                               int col,
                               const char *text,
                               uint8_t fg,
                               uint8_t bg,
                               int max_chars) {
    if (!text || max_chars <= 0) {
        return;
    }

    for (int i = 0; text[i] && i < max_chars; i++) {
        c64_menu_draw_cell(row, col + i, text[i], fg, bg);
    }
}

// Render pickup/status messages with the clean C64 font rather than the
// dithered Doom framebuffer text.
static void draw_c64_clean_hud_message_overlay(void) {
    char lines[C64_HUD_TEXT_ROWS][C64_BITMAP_CHAR_COLS + 1];
    int line_lengths[C64_HUD_TEXT_ROWS];
    int used_rows = c64_hud_layout_message(lines, line_lengths);

    if (used_rows <= 0) {
        return;
    }

    for (int row = 0; row < used_rows; row++) {
        int len = line_lengths[row];
        int start_col;

        if (len <= 0) {
            continue;
        }

        start_col = C64_BITMAP_CHAR_COLS - C64_HUD_TEXT_RIGHT_MARGIN_COLS - len;
        if (start_col < 0) {
            start_col = 0;
        }

        for (int col = 0; col < len && start_col + col < C64_BITMAP_CHAR_COLS; col++) {
            c64_menu_draw_cell(row, start_col + col, lines[row][col],
                               SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR);
        }
    }
}

// Draw a simple framed box for clean menu and intermission overlays.
static void c64_menu_draw_box(int x, int y, int width, int height) {
    c64_menu_fill_rect(x, y, width, height, SYSOP_MENU_BORDER_COLOR);

    if (width > 2 && height > 2) {
        c64_menu_fill_rect(x + 1, y + 1, width - 2, height - 2,
                           SYSOP_MENU_BG_COLOR);
    }
}

// Wrap a long menu message into fixed-width C64 text lines.
static int sysop_menu_wrap_text(const char *text,
                                char lines[SYSOP_MENU_MAX_TEXT_LINES][SYSOP_MENU_MESSAGE_WRAP_COLS + 1],
                                int max_lines,
                                int width) {
    int line = 0;
    int col = 0;
    int wrote_any = 0;

    if (width > SYSOP_MENU_MESSAGE_WRAP_COLS) {
        width = SYSOP_MENU_MESSAGE_WRAP_COLS;
    }

    for (int i = 0; i < max_lines; i++) {
        lines[i][0] = '\0';
    }

    if (!text || !text[0]) {
        return 0;
    }

    for (int i = 0; text[i] && line < max_lines; i++) {
        char ch = text[i];

        if (ch == '\r') {
            continue;
        }

        if (ch == '\n') {
            lines[line][col] = '\0';
            line++;
            col = 0;
            wrote_any = 1;
            continue;
        }

        if (col >= width) {
            lines[line][col] = '\0';
            line++;
            col = 0;
            if (line >= max_lines) {
                break;
            }
        }

        lines[line][col++] = ch;
        lines[line][col] = '\0';
        wrote_any = 1;
    }

    if (line >= max_lines) {
        return max_lines;
    }

    return wrote_any ? line + 1 : 0;
}

// Draw Doom's modal menu message text using fixed-width C64 bitmap cells.
static void draw_c64_clean_menu_message(const sysop_menu_snapshot_t *snapshot) {
    char lines[SYSOP_MENU_MAX_TEXT_LINES][SYSOP_MENU_MESSAGE_WRAP_COLS + 1];
    int line_count = sysop_menu_wrap_text(snapshot->message, lines,
                                          SYSOP_MENU_MAX_TEXT_LINES,
                                          SYSOP_MENU_MESSAGE_WRAP_COLS);
    int max_len = sysop_menu_text_len(snapshot->title, SYSOP_MENU_TITLE_MAX);
    int width;
    int height;
    int box_x;
    int box_y;
    int title_x;
    int title_len = sysop_menu_text_len(snapshot->title, SYSOP_MENU_TITLE_MAX);

    for (int i = 0; i < line_count; i++) {
        int len = sysop_menu_text_len(lines[i], SYSOP_MENU_MESSAGE_WRAP_COLS);
        if (len > max_len) {
            max_len = len;
        }
    }

    width = sysop_menu_clamp_int(max_len + 4,
                                 SYSOP_MENU_BOX_MIN_COLS,
                                 SYSOP_MENU_BOX_MAX_COLS);
    height = sysop_menu_clamp_int(line_count + 4, 5, C64_BITMAP_CHAR_ROWS - 2);
    box_x = (C64_BITMAP_CHAR_COLS - width) / 2;
    box_y = (C64_BITMAP_CHAR_ROWS - height) / 2;
    title_x = box_x + (width - title_len) / 2;

    c64_menu_draw_box(box_x, box_y, width, height);
    c64_menu_draw_text(box_y + 1, title_x, snapshot->title,
                       SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                       width - 2);

    for (int i = 0; i < line_count && i + 3 < height - 1; i++) {
        c64_menu_draw_text(box_y + 3 + i, box_x + 2, lines[i],
                           SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                           width - 4);
    }
}

// Draw selectable menu entries and highlight the current item.
static void draw_c64_clean_menu_items(const sysop_menu_snapshot_t *snapshot) {
    int max_len = sysop_menu_text_len(snapshot->title, SYSOP_MENU_TITLE_MAX);
    int width;
    int height;
    int box_x;
    int box_y;
    int title_x;

    for (int i = 0; i < snapshot->item_count; i++) {
        int len = sysop_menu_text_len(snapshot->items[i], SYSOP_MENU_ITEM_MAX) + 2;
        if (len > max_len) {
            max_len = len;
        }
    }

    width = sysop_menu_clamp_int(max_len + 4,
                                 SYSOP_MENU_BOX_MIN_COLS,
                                 SYSOP_MENU_BOX_MAX_COLS);
    height = sysop_menu_clamp_int(snapshot->item_count + 4,
                                  5,
                                  C64_BITMAP_CHAR_ROWS - 2);
    box_x = (C64_BITMAP_CHAR_COLS - width) / 2;
    box_y = (C64_BITMAP_CHAR_ROWS - height) / 2;
    title_x = box_x + (width - sysop_menu_text_len(snapshot->title, SYSOP_MENU_TITLE_MAX)) / 2;

    c64_menu_draw_box(box_x, box_y, width, height);
    c64_menu_draw_text(box_y + 1, title_x, snapshot->title,
                       SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                       width - 2);

    for (int i = 0; i < snapshot->item_count && i + 3 < height - 1; i++) {
        int row = box_y + 3 + i;
        int selected = (i == snapshot->selected);
        uint8_t fg = snapshot->item_status[i] ? SYSOP_MENU_FG_COLOR : SYSOP_MENU_DISABLED_COLOR;
        uint8_t bg = SYSOP_MENU_BG_COLOR;

        if (selected) {
            fg = SYSOP_MENU_SELECTED_FG_COLOR;
            bg = SYSOP_MENU_SELECTED_BG_COLOR;
            c64_menu_fill_rect(box_x + 1, row, width - 2, 1, bg);
        }

        c64_menu_draw_cell(row, box_x + 2, selected ? '>' : ' ', fg, bg);
        c64_menu_draw_text(row, box_x + 4, snapshot->items[i],
                           fg, bg, width - 6);
    }
}

// Overlay the current Doom menu with a C64-native readable menu when enabled.
static void draw_c64_clean_menu_overlay(void) {
    sysop_menu_snapshot_t snapshot;

    if (!sysop_clean_menu_requested()) {
        return;
    }

    M_GetSysopMenuSnapshot(&snapshot);
    if (!snapshot.active) {
        return;
    }

    if (snapshot.is_message) {
        draw_c64_clean_menu_message(&snapshot);
    } else {
        draw_c64_clean_menu_items(&snapshot);
    }
}

#define SYSOP_INTERMISSION_BOX_X 1
#define SYSOP_INTERMISSION_BOX_Y 1
#define SYSOP_INTERMISSION_BOX_W 38
#define SYSOP_INTERMISSION_BOX_H 22
#define SYSOP_INTERMISSION_LABEL_COLOR 7
#define SYSOP_INTERMISSION_OK_COLOR 5
#define SYSOP_INTERMISSION_WARN_COLOR 7
#define SYSOP_INTERMISSION_BAD_COLOR 2

// Draw centered clean text inside an intermission box.
static void c64_intermission_draw_centered(int row,
                                           const char *text,
                                           uint8_t fg,
                                           uint8_t bg,
                                           int box_x,
                                           int width)
{
    int len = sysop_menu_text_len(text, C64_BITMAP_CHAR_COLS);
    int col = box_x + (width - len) / 2;

    if (col < box_x + 1) {
        col = box_x + 1;
    }

    c64_menu_draw_text(row, col, text, fg, bg, width - 2);
}

// Format a Doom map number as MAPxx or ExMx for clean intermission text.
static void c64_intermission_format_map(const sysop_intermission_snapshot_t *snapshot,
                                        int map,
                                        char *buffer,
                                        size_t buffer_size)
{
    if (map < 0) {
        M_snprintf(buffer, buffer_size, "LEVEL");
    } else if (gamemode == commercial) {
        M_snprintf(buffer, buffer_size, "MAP%02d", map + 1);
    } else {
        M_snprintf(buffer, buffer_size, "E%dM%d", snapshot->episode + 1, map + 1);
    }
}

// Format optional percentage values, preserving Doom's "not available" cases.
static void c64_intermission_format_percent(int value,
                                            char *buffer,
                                            size_t buffer_size)
{
    if (value < 0) {
        M_snprintf(buffer, buffer_size, "---");
    } else {
        M_snprintf(buffer, buffer_size, "%d%%", value);
    }
}

// Format intermission times using MM:SS or H:MM:SS as needed.
static void c64_intermission_format_time(int seconds,
                                         char *buffer,
                                         size_t buffer_size)
{
    if (seconds < 0) {
        M_snprintf(buffer, buffer_size, "--:--");
    } else if (seconds >= 3600) {
        M_snprintf(buffer, buffer_size, "%d:%02d:%02d",
                   seconds / 3600, (seconds / 60) % 60, seconds % 60);
    } else {
        M_snprintf(buffer, buffer_size, "%d:%02d", seconds / 60, seconds % 60);
    }
}

// Pick a C64 color that gives quick feedback on completion percentages.
static uint8_t c64_intermission_percent_color(int value)
{
    if (value < 0) {
        return SYSOP_MENU_DISABLED_COLOR;
    }
    if (value >= 100) {
        return SYSOP_INTERMISSION_OK_COLOR;
    }
    if (value >= 50) {
        return SYSOP_INTERMISSION_WARN_COLOR;
    }
    return SYSOP_INTERMISSION_BAD_COLOR;
}

// Draw one label/value row in the clean intermission stats panel.
static void c64_intermission_draw_stat_row(int row,
                                           const char *label,
                                           const char *value,
                                           uint8_t value_color)
{
    int value_len = sysop_menu_text_len(value, 16);
    int value_col = SYSOP_INTERMISSION_BOX_X + SYSOP_INTERMISSION_BOX_W - 4 - value_len;

    c64_menu_draw_text(row, SYSOP_INTERMISSION_BOX_X + 5, label,
                       SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 14);
    c64_menu_draw_text(row, value_col, value,
                       value_color, SYSOP_MENU_BG_COLOR, 16);
}

// Draw the single-player level-complete stats screen in the clean C64 style.
static void draw_c64_clean_intermission_single(const sysop_intermission_snapshot_t *snapshot)
{
    int player = snapshot->me;
    char map_name[16];
    char value[24];

    if (player < 0 || player >= MAXPLAYERS) {
        player = 0;
    }

    c64_intermission_format_map(snapshot, snapshot->last,
                                map_name, sizeof(map_name));

    c64_menu_draw_box(SYSOP_INTERMISSION_BOX_X,
                      SYSOP_INTERMISSION_BOX_Y,
                      SYSOP_INTERMISSION_BOX_W,
                      SYSOP_INTERMISSION_BOX_H);

    c64_intermission_draw_centered(3, "LEVEL COMPLETE",
                                   SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                                   SYSOP_INTERMISSION_BOX_X,
                                   SYSOP_INTERMISSION_BOX_W);
    c64_intermission_draw_centered(5, map_name,
                                   SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR,
                                   SYSOP_INTERMISSION_BOX_X,
                                   SYSOP_INTERMISSION_BOX_W);

    c64_intermission_format_percent(snapshot->kills[player], value, sizeof(value));
    c64_intermission_draw_stat_row(8, "KILLS", value,
                                   c64_intermission_percent_color(snapshot->kills[player]));

    c64_intermission_format_percent(snapshot->items[player], value, sizeof(value));
    c64_intermission_draw_stat_row(10, "ITEMS", value,
                                   c64_intermission_percent_color(snapshot->items[player]));

    c64_intermission_format_percent(snapshot->secrets[player], value, sizeof(value));
    c64_intermission_draw_stat_row(12, "SECRETS", value,
                                   c64_intermission_percent_color(snapshot->secrets[player]));

    c64_intermission_format_time(snapshot->time, value, sizeof(value));
    c64_intermission_draw_stat_row(15, "TIME", value, SYSOP_MENU_FG_COLOR);

    if (snapshot->show_par) {
        c64_intermission_format_time(snapshot->par, value, sizeof(value));
        c64_intermission_draw_stat_row(17, "PAR", value, SYSOP_MENU_DISABLED_COLOR);
    }
}

// Draw cooperative/netgame intermission stats for all active players.
static void draw_c64_clean_intermission_net(const sysop_intermission_snapshot_t *snapshot)
{
    char value[16];
    int row = 7;

    c64_menu_draw_box(SYSOP_INTERMISSION_BOX_X,
                      SYSOP_INTERMISSION_BOX_Y,
                      SYSOP_INTERMISSION_BOX_W,
                      SYSOP_INTERMISSION_BOX_H);
    c64_intermission_draw_centered(3, "NETGAME STATS",
                                   SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                                   SYSOP_INTERMISSION_BOX_X,
                                   SYSOP_INTERMISSION_BOX_W);

    c64_menu_draw_text(5, 4, "PL", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 3);
    c64_menu_draw_text(5, 9, "KILL", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 5);
    c64_menu_draw_text(5, 16, "ITEM", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 5);
    c64_menu_draw_text(5, 23, "SECR", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 5);
    if (snapshot->show_frags) {
        c64_menu_draw_text(5, 31, "FRAG", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 5);
    }

    for (int i = 0; i < MAXPLAYERS && row < 20; ++i) {
        if (!snapshot->player_in_game[i]) {
            continue;
        }

        M_snprintf(value, sizeof(value), "%cP%d",
                   i == snapshot->me ? '>' : ' ', i + 1);
        c64_menu_draw_text(row, 4, value, SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR, 4);

        c64_intermission_format_percent(snapshot->kills[i], value, sizeof(value));
        c64_menu_draw_text(row, 9, value,
                           c64_intermission_percent_color(snapshot->kills[i]),
                           SYSOP_MENU_BG_COLOR, 6);

        c64_intermission_format_percent(snapshot->items[i], value, sizeof(value));
        c64_menu_draw_text(row, 16, value,
                           c64_intermission_percent_color(snapshot->items[i]),
                           SYSOP_MENU_BG_COLOR, 6);

        c64_intermission_format_percent(snapshot->secrets[i], value, sizeof(value));
        c64_menu_draw_text(row, 23, value,
                           c64_intermission_percent_color(snapshot->secrets[i]),
                           SYSOP_MENU_BG_COLOR, 6);

        if (snapshot->show_frags) {
            M_snprintf(value, sizeof(value), "%d", snapshot->frags[i]);
            c64_menu_draw_text(row, 32, value,
                               SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR, 5);
        }

        row += 2;
    }
}

// Draw the clean deathmatch frag table.
static void draw_c64_clean_intermission_deathmatch(const sysop_intermission_snapshot_t *snapshot)
{
    char value[16];
    int row = 7;

    c64_menu_draw_box(SYSOP_INTERMISSION_BOX_X,
                      SYSOP_INTERMISSION_BOX_Y,
                      SYSOP_INTERMISSION_BOX_W,
                      SYSOP_INTERMISSION_BOX_H);
    c64_intermission_draw_centered(3, "DEATHMATCH",
                                   SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                                   SYSOP_INTERMISSION_BOX_X,
                                   SYSOP_INTERMISSION_BOX_W);

    c64_menu_draw_text(5, 4, "PL", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 3);
    for (int i = 0; i < MAXPLAYERS; ++i) {
        if (snapshot->player_in_game[i]) {
            M_snprintf(value, sizeof(value), "P%d", i + 1);
            c64_menu_draw_text(5, 9 + i * 5, value,
                               SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 3);
        }
    }
    c64_menu_draw_text(5, 31, "TOT", SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR, 4);

    for (int i = 0; i < MAXPLAYERS && row < 20; ++i) {
        if (!snapshot->player_in_game[i]) {
            continue;
        }

        M_snprintf(value, sizeof(value), "%cP%d",
                   i == snapshot->me ? '>' : ' ', i + 1);
        c64_menu_draw_text(row, 4, value, SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR, 4);

        for (int j = 0; j < MAXPLAYERS; ++j) {
            if (snapshot->player_in_game[j]) {
                M_snprintf(value, sizeof(value), "%d", snapshot->dm_frags[i][j]);
                c64_menu_draw_text(row, 9 + j * 5, value,
                                   SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR, 4);
            }
        }

        M_snprintf(value, sizeof(value), "%d", snapshot->dm_totals[i]);
        c64_menu_draw_text(row, 31, value, SYSOP_INTERMISSION_LABEL_COLOR,
                           SYSOP_MENU_BG_COLOR, 5);
        row += 2;
    }
}

// Draw the short "entering next map" intermission screen.
static void draw_c64_clean_intermission_next(const sysop_intermission_snapshot_t *snapshot)
{
    char map_name[16];

    c64_intermission_format_map(snapshot, snapshot->next,
                                map_name, sizeof(map_name));

    c64_menu_draw_box(SYSOP_INTERMISSION_BOX_X,
                      SYSOP_INTERMISSION_BOX_Y,
                      SYSOP_INTERMISSION_BOX_W,
                      SYSOP_INTERMISSION_BOX_H);
    c64_intermission_draw_centered(10, "ENTERING",
                                   SYSOP_MENU_FG_COLOR, SYSOP_MENU_BG_COLOR,
                                   SYSOP_INTERMISSION_BOX_X,
                                   SYSOP_INTERMISSION_BOX_W);
    c64_intermission_draw_centered(12, map_name,
                                   SYSOP_INTERMISSION_LABEL_COLOR, SYSOP_MENU_BG_COLOR,
                                   SYSOP_INTERMISSION_BOX_X,
                                   SYSOP_INTERMISSION_BOX_W);
}

// Replace Doom's dithered intermission art with the clean C64 intermission
// overlay when requested.
static void draw_c64_clean_intermission_overlay(void)
{
    sysop_intermission_snapshot_t snapshot;

    if (!g_sysop_clean_intermission_enabled) {
        return;
    }

    WI_GetSysopIntermissionSnapshot(&snapshot);
    if (!snapshot.active) {
        return;
    }

    if (snapshot.mode == sysop_intermission_stats) {
        if (snapshot.deathmatch) {
            draw_c64_clean_intermission_deathmatch(&snapshot);
        } else if (snapshot.netgame) {
            draw_c64_clean_intermission_net(&snapshot);
        } else {
            draw_c64_clean_intermission_single(&snapshot);
        }
    } else if (snapshot.mode == sysop_intermission_next) {
        draw_c64_clean_intermission_next(&snapshot);
    }
}

#define SYSOP_STATUS_START_ROW (C64_BITMAP_CHAR_ROWS - 4)
#define SYSOP_STATUS_BG_COLOR 0
#define SYSOP_STATUS_LABEL_COLOR 7
#define SYSOP_STATUS_FG_COLOR 1
#define SYSOP_STATUS_DIM_COLOR 12
#define SYSOP_STATUS_BAD_COLOR 2
#define SYSOP_STATUS_WARN_COLOR 7
#define SYSOP_STATUS_OK_COLOR 5
#define SYSOP_STATUS_BLUE_COLOR 14
#define SYSOP_STATUS_RED_COLOR 2
#define SYSOP_STATUS_SELECTED_FG_COLOR 0
#define SYSOP_STATUS_SELECTED_BG_COLOR 7

// Decide whether the clean C64 status strip should replace or supplement the
// current Doom view.
static int sysop_status_should_draw(void) {
    return g_sysop_clean_status_enabled
        && gamestate == GS_LEVEL
        && consoleplayer >= 0
        && consoleplayer < MAXPLAYERS
        && playeringame[consoleplayer]
        && (screenblocks < 11 || automapactive || viewheight != SCREENHEIGHT);
}

// Color health values by danger level for the clean status strip.
static uint8_t sysop_status_health_color(int value) {
    if (value <= 25) {
        return SYSOP_STATUS_BAD_COLOR;
    }
    if (value <= 50) {
        return SYSOP_STATUS_WARN_COLOR;
    }
    return SYSOP_STATUS_OK_COLOR;
}

// Color ammo values by availability for the clean status strip.
static uint8_t sysop_status_ammo_color(int value) {
    if (value < 0) {
        return SYSOP_STATUS_DIM_COLOR;
    }
    if (value == 0) {
        return SYSOP_STATUS_BAD_COLOR;
    }
    if (value <= 10) {
        return SYSOP_STATUS_WARN_COLOR;
    }
    return SYSOP_STATUS_FG_COLOR;
}

// Return the ammo count for the currently readied weapon, or -1 for fists and
// other no-ammo weapons.
static int sysop_status_current_ammo(const player_t *player) {
    ammotype_t ammo_type;

    if (!player || player->readyweapon < 0 || player->readyweapon >= NUMWEAPONS) {
        return -1;
    }

    ammo_type = weaponinfo[player->readyweapon].ammo;
    if (ammo_type == am_noammo || ammo_type < 0 || ammo_type >= NUMAMMO) {
        return -1;
    }

    return player->ammo[ammo_type];
}

// Sum a player's frags for the deathmatch status readout.
static int sysop_status_frag_count(const player_t *player) {
    int frags = 0;

    if (!player) {
        return 0;
    }

    for (int i = 0; i < MAXPLAYERS; i++) {
        frags += player->frags[i];
    }

    return frags;
}

// Draw status-strip text with the status background color.
static void c64_status_draw_text(int row, int col, const char *text, uint8_t fg) {
    c64_menu_draw_text(row, col, text, fg, SYSOP_STATUS_BG_COLOR,
                       C64_BITMAP_CHAR_COLS - col);
}

// Draw a label/value pair in the clean status strip.
static void c64_status_draw_field(int row,
                                  int col,
                                  const char *label,
                                  const char *value,
                                  uint8_t value_color) {
    c64_status_draw_text(row, col, label, SYSOP_STATUS_LABEL_COLOR);
    c64_status_draw_text(row, col + (int)strlen(label) + 1,
                         value, value_color);
}

// Draw current/max ammo for one ammo type.
static void c64_status_draw_ammo_pair(int row,
                                      int col,
                                      const char *label,
                                      int ammo,
                                      int max_ammo) {
    char value[12];

    M_snprintf(value, sizeof(value), "%3d/%3d", ammo, max_ammo);
    c64_status_draw_field(row, col, label, value, sysop_status_ammo_color(ammo));
}

// Draw one key indicator, dimmed when the player does not have that key color.
static void c64_status_draw_key(int row,
                                int col,
                                char label,
                                int owned,
                                uint8_t key_color) {
    c64_menu_draw_cell(row, col, owned ? label : '-',
                       owned ? key_color : SYSOP_STATUS_DIM_COLOR,
                       SYSOP_STATUS_BG_COLOR);
}

// Draw blue/yellow/red key ownership in the clean status strip.
static void c64_status_draw_keys(int row, int col, const player_t *player) {
    int has_blue = player->cards[it_bluecard] || player->cards[it_blueskull];
    int has_yellow = player->cards[it_yellowcard] || player->cards[it_yellowskull];
    int has_red = player->cards[it_redcard] || player->cards[it_redskull];

    c64_status_draw_text(row, col, "KEY", SYSOP_STATUS_LABEL_COLOR);
    c64_status_draw_key(row, col + 4, 'B', has_blue, SYSOP_STATUS_BLUE_COLOR);
    c64_status_draw_key(row, col + 6, 'Y', has_yellow, SYSOP_STATUS_WARN_COLOR);
    c64_status_draw_key(row, col + 8, 'R', has_red, SYSOP_STATUS_RED_COLOR);
}

// Draw weapon ownership and highlight the currently readied weapon.
static void c64_status_draw_weapons(int row, const player_t *player) {
    static const weapontype_t weapon_order[NUMWEAPONS] = {
        wp_fist, wp_pistol, wp_shotgun, wp_chaingun, wp_missile,
        wp_plasma, wp_bfg, wp_chainsaw, wp_supershotgun
    };
    static const char weapon_labels[NUMWEAPONS] = {
        'F', 'P', 'S', 'G', 'R', 'L', 'B', 'A', 'D'
    };
    int col = 4;

    c64_status_draw_text(row, 0, "WPN", SYSOP_STATUS_LABEL_COLOR);

    for (int i = 0; i < NUMWEAPONS; i++) {
        weapontype_t weapon = weapon_order[i];
        int owned = player->weaponowned[weapon];
        int ready = player->readyweapon == weapon;
        uint8_t fg = owned ? SYSOP_STATUS_FG_COLOR : SYSOP_STATUS_DIM_COLOR;
        uint8_t bg = SYSOP_STATUS_BG_COLOR;

        if (ready) {
            fg = SYSOP_STATUS_SELECTED_FG_COLOR;
            bg = SYSOP_STATUS_SELECTED_BG_COLOR;
        }

        c64_menu_draw_cell(row, col, owned ? weapon_labels[i] : '-',
                           fg, bg);
        col += 2;
    }

    if (deathmatch) {
        char frags[10];
        M_snprintf(frags, sizeof(frags), "FRG %d",
                   sysop_status_frag_count(player));
        c64_status_draw_text(row, 26, frags, SYSOP_STATUS_LABEL_COLOR);
    }
}

// Replace the lower Doom status area with a compact C64-native status strip.
static void draw_c64_clean_status_overlay(void) {
    const player_t *player;
    int current_ammo;
    char value[12];
    int row = SYSOP_STATUS_START_ROW;

    if (!sysop_status_should_draw()) {
        return;
    }

    player = &players[consoleplayer];
    current_ammo = sysop_status_current_ammo(player);

    c64_menu_fill_rect(0, SYSOP_STATUS_START_ROW,
                       C64_BITMAP_CHAR_COLS, 4,
                       SYSOP_STATUS_BG_COLOR);

    if (current_ammo >= 0) {
        M_snprintf(value, sizeof(value), "%3d", current_ammo);
    } else {
        M_snprintf(value, sizeof(value), "---");
    }
    c64_status_draw_field(row, 0, "AM", value,
                          sysop_status_ammo_color(current_ammo));

    M_snprintf(value, sizeof(value), "%3d", player->health);
    c64_status_draw_field(row, 8, "HP", value,
                          sysop_status_health_color(player->health));

    M_snprintf(value, sizeof(value), "%3d", player->armorpoints);
    c64_status_draw_field(row, 16, "AR", value,
                          player->armorpoints > 0
                              ? SYSOP_STATUS_OK_COLOR
                              : SYSOP_STATUS_DIM_COLOR);

    c64_status_draw_keys(row, 24, player);

    c64_status_draw_ammo_pair(row + 1, 0, "BUL",
                              player->ammo[am_clip],
                              player->maxammo[am_clip]);
    c64_status_draw_ammo_pair(row + 1, 14, "SHL",
                              player->ammo[am_shell],
                              player->maxammo[am_shell]);
    if (player->backpack) {
        c64_status_draw_text(row + 1, 29, "PACK", SYSOP_STATUS_OK_COLOR);
    }

    c64_status_draw_ammo_pair(row + 2, 0, "RKT",
                              player->ammo[am_misl],
                              player->maxammo[am_misl]);
    c64_status_draw_ammo_pair(row + 2, 14, "CEL",
                              player->ammo[am_cell],
                              player->maxammo[am_cell]);

    c64_status_draw_weapons(row + 3, player);
}

// Compute how many C64 byte writes fit in the configured safe cycle window for
// one raster line.
static int safe_poke_capacity_for_line(void) {
    int safe_end = C64_POKE_SAFE_CYCLE_END;
    if (safe_end > g_cyclesPerLine) {
        safe_end = g_cyclesPerLine;
    }

    if (safe_end < C64_POKE_SAFE_CYCLE_START) {
        return 1;
    }

    return safe_end - C64_POKE_SAFE_CYCLE_START + 1;
}

// Multicolor bitmap badlines consume memory bandwidth on the real VIC. The
// scheduler avoids those lines and only spends cycles in the configured safe
// window so complete bitmap/screen/color updates remain PAL/NTSC friendly.
static int is_bitmap_active_badline(int line) {
    if (line < 0 || line >= g_vic_lines) {
        return 0;
    }

    return (line >= 0x30 && line <= 0xf7)
        && ((line & 7) == (C64_BITMAP_D011 & 7));
}

// Move the scheduler to a usable raster line before emitting the next poke.
static void prepare_scheduled_poke_line(int *line, int *used_on_line, int deadline, int *overflow_count) {
    int capacity = safe_poke_capacity_for_line();

    while (*line < deadline && is_bitmap_active_badline(*line)) {
        (*line)++;
        *used_on_line = 0;
    }

    while (*used_on_line >= capacity) {
        int next_line = *line + 1;
        while (next_line <= deadline && is_bitmap_active_badline(next_line)) {
            next_line++;
        }

        if (next_line > deadline) {
            (*overflow_count)++;
            break;
        }

        *line = next_line;
        *used_on_line = 0;
        capacity = safe_poke_capacity_for_line();
    }
}

// Advance scheduler bookkeeping after a poke has consumed one safe cycle slot.
static void advance_scheduled_poke_line(int *line, int *used_on_line, int deadline) {
    int capacity = safe_poke_capacity_for_line();

    (*used_on_line)++;
    if (*used_on_line < capacity) {
        return;
    }

    int next_line = *line + 1;
    while (next_line <= deadline && is_bitmap_active_badline(next_line)) {
        next_line++;
    }

    if (next_line > deadline) {
        return;
    }

    *line = next_line;
    *used_on_line = 0;
}

// Emit one scheduled C64 write, inserting a raster wait when moving to a new
// safe line.
static void scheduled_frame_poke(int *line,
                                 int *used_on_line,
                                 int *emitted_wait_line,
                                 int deadline,
                                 uint16_t address,
                                 uint8_t value,
                                 int *overflow_count) {
    if (deadline >= g_vic_lines) {
        deadline = g_vic_lines - 1;
    }
    if (deadline < 0) {
        deadline = 0;
    }

    if (*line > deadline) {
        *line = deadline;
        *used_on_line = 0;
    }

    prepare_scheduled_poke_line(line, used_on_line, deadline, overflow_count);

    if (*emitted_wait_line != *line) {
        Wait_vic2((uint16_t)*line, (uint8_t)C64_POKE_SAFE_CYCLE_START);
        *emitted_wait_line = *line;
    }

    Poke(address, value);
    advance_scheduled_poke_line(line, used_on_line, deadline);
}

// Upload the complete bitmap, screen RAM, color RAM, and VIC register frame
// through the PAL/NTSC-aware poke scheduler.
static void upload_koala_frame_scheduled(void) {
    int line = 0;
    int used_on_line = 0;
    int emitted_wait_line = -1;
    int overflow_count = 0;
    int init_deadline = C64_BITMAP_FIRST_BADLINE - 1;

    // Register setup must land before the first bitmap badline. The remaining
    // cell data is then scheduled row-by-row before each row's own badline.
    scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, init_deadline, VIC_CTRL_REG2, C64_MULTICOLOR_D016, &overflow_count);
    scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, init_deadline, VIC_MEM_CTRL, C64_BITMAP_D018, &overflow_count);
    scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, init_deadline, 0xdd00, C64_CIA2_BANK0, &overflow_count);
    scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, init_deadline, VIC_CTRL_REG1, C64_BITMAP_D011, &overflow_count);
    scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, init_deadline, BORDER_COLOR_REG, 0x00, &overflow_count);
    scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, init_deadline, BG_COLOR_REG0, sysop_c64_frame[10000], &overflow_count);

    for (int row = 0; row < C64_BITMAP_CHAR_ROWS; row++) {
        int badline = C64_BITMAP_FIRST_BADLINE + row * 8;
        int deadline = badline - 1;

        for (int col = 0; col < C64_BITMAP_CHAR_COLS; col++) {
            int cell = row * C64_BITMAP_CHAR_COLS + col;
            int bitmap_offset = row * C64_BITMAP_ROW_BYTES + col * C64_BITMAP_BYTES_PER_CELL;

            scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, deadline,
                                 SCREEN_RAM + cell, sysop_c64_frame[8000 + cell], &overflow_count);
            scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, deadline,
                                 COLOR_RAM + cell, sysop_c64_frame[9000 + cell], &overflow_count);

            for (int b = 0; b < C64_BITMAP_BYTES_PER_CELL; b++) {
                scheduled_frame_poke(&line, &used_on_line, &emitted_wait_line, deadline,
                                     BITMAP_MEMORY + bitmap_offset + b,
                                     sysop_c64_frame[bitmap_offset + b],
                                     &overflow_count);
            }
        }
    }

    if (overflow_count != 0) {
        printf("Warning: bitmap poke scheduler overflowed %d time(s)\n", overflow_count);
    }
}

// Build a menu-biased HDMI palette that preserves Doom's red/gold menu art.
static void build_sysop_menu_palette(const int src[16][3], int dst[16][3])
{
    Sysop_ImageCopyPalette(dst, src);

    dst[0][0] = 0;    dst[0][1] = 0;    dst[0][2] = 0;
    dst[1][0] = 255;  dst[1][1] = 255;  dst[1][2] = 255;
    dst[2][0] = 188;  dst[2][1] = 24;   dst[2][2] = 18;
    dst[7][0] = 255;  dst[7][1] = 236;  dst[7][2] = 76;
    dst[8][0] = 213;  dst[8][1] = 82;   dst[8][2] = 16;
    dst[9][0] = 88;   dst[9][1] = 40;   dst[9][2] = 0;
    dst[10][0] = 255; dst[10][1] = 82;  dst[10][2] = 62;
    dst[11][0] = 35;  dst[11][1] = 35;  dst[11][2] = 35;
    dst[12][0] = 88;  dst[12][1] = 88;  dst[12][2] = 88;
    dst[15][0] = 190; dst[15][1] = 190; dst[15][2] = 190;
}

// Decide when Doom's red patch graphics need the special menu/intermission
// treatment.
static int sysop_should_use_red_patch_treatment(void)
{
    return (menuactive && !sysop_clean_menu_requested())
           || (gamestate == GS_INTERMISSION && !g_sysop_clean_intermission_enabled);
}

// Decide whether the current frame should use the menu-focused HDMI palette.
static int sysop_should_use_menu_palette(void)
{
    return g_sysop_menu_palette_mode == SYSOP_MENU_PALETTE_SHARP
           && sysop_should_use_red_patch_treatment();
}

// Rebuild menu and menu-status palettes from the current active base palette.
static void refresh_sysop_menu_palette(void)
{
    build_sysop_menu_palette(sysop_active_palette, sysop_menu_palette);
    build_status_split_palette(sysop_menu_palette, sysop_menu_status_palette);
}

// Return the human-readable name of the palette currently selected by mega
// converter settings.
static const char *sysop_base_palette_name(void)
{
    SysopMegaOptions mega_options;

    Sysop_ImageMegaGetOptions(&mega_options);
    return Sysop_ImageMegaPaletteLabel(mega_options.palette_model);
}

// Reinstall palette state when the mega converter's target palette changes.
static void refresh_sysop_base_palette_if_needed(void)
{
    const int (*base_palette)[3] = Sysop_ImageBaseDisplayPalette();
    static unsigned int installed_mega_palette_revision = 0;
    unsigned int mega_palette_revision = Sysop_ImageMegaPaletteRevision();

    if (base_palette == g_sysop_installed_base_palette
        && mega_palette_revision == installed_mega_palette_revision) {
        return;
    }

    Sysop_ImageCopyPalette(sysop_active_palette, base_palette);
    build_status_split_palette(sysop_active_palette, sysop_status_palette);
    g_sysop_installed_base_palette = base_palette;
    installed_mega_palette_revision = mega_palette_revision;
    g_sysop_palette_revision++;
    g_sysop_menu_palette_active = -1;
    g_sysop_runtime_palette_revision = 0;
}

// Keep converter palette state and HDMI palette effects in sync for the next
// frame.
static void apply_sysop_runtime_palette_state(void)
{
    refresh_sysop_base_palette_if_needed();

    // The renderer has two palette concepts: the C64 palette used for
    // conversion/scoring, and the HDMI palette that Sysop-64 can retune on the
    // fly. Keep them separate so HDMI tricks do not constantly invalidate the
    // converter's fast lookup tables.
    int want_menu_palette = sysop_should_use_menu_palette();
    const int (*palette)[3] = sysop_active_palette;

    if (want_menu_palette) {
        refresh_sysop_menu_palette();
        palette = sysop_menu_palette;
    }

    if (want_menu_palette == g_sysop_menu_palette_active
        && g_sysop_runtime_palette_revision == g_sysop_palette_revision) {
        return;
    }

    unsigned int conversion_palette_revision = Sysop_ImageConversionPaletteRevision();

    if (want_menu_palette) {
        Sysop_ImageSetConversionPaletteFrom(palette);
    } else {
        // Adaptive palette updates are an HDMI display effect. Keep the
        // gameplay converter pointed at the stable base palette so palette
        // nudges do not force expensive quantization table rebuilds.
        Sysop_ImageSetGameplayConversionPaletteFrom(Sysop_ImageBaseDisplayPalette());
    }
    if (Sysop_ImageConversionPaletteRevision() != conversion_palette_revision) {
        Sysop_ImageRecomputeColorQuantizationTables();
    }

    if (!(g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_STATUS_SPLIT)) {
        set_sysop_palette_from(palette);
    }

    g_sysop_menu_palette_active = want_menu_palette;
    g_sysop_runtime_palette_revision = g_sysop_palette_revision;
}

// Install the startup palette and print the active Sysop display configuration.
static void install_sysop_base_palette(void)
{
    const int (*base_palette)[3] = Sysop_ImageBaseDisplayPalette();
    Sysop_ImageCopyPalette(sysop_active_palette, base_palette);
    build_status_split_palette(sysop_active_palette, sysop_status_palette);
    g_sysop_installed_base_palette = base_palette;
    g_sysop_palette_revision++;

    Sysop_ImageSetGameplayConversionPaletteFrom(sysop_active_palette);
    set_sysop_palette_from(sysop_active_palette);

    if (g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_ADAPTIVE) {
        printf("Sysop palette: adaptive incremental updates over about %d frame(s)\n",
               g_sysop_palette_update_interval);
    } else {
        printf("Sysop palette: fixed %s palette\n",
               sysop_base_palette_name());
    }

    if (g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_STATUS_SPLIT) {
        printf("Sysop palette: status-bar HDMI split enabled\n");
    }

    SysopMegaOptions mega_options;
    Sysop_ImageMegaGetOptions(&mega_options);
    printf("Sysop converter: mega indexed live-tunable path\n");
    printf("Mega C64 converter: dither=%s palette=%s strength=%d brightness=%d contrast=%d gamma=%d saturation=%d vibrance=%d detail=%d surface=%d%s\n",
           Sysop_ImageMegaDitherName(mega_options.dither_pattern),
           Sysop_ImageMegaPaletteName(mega_options.palette_model),
           mega_options.dither_strength,
           mega_options.brightness,
           mega_options.contrast,
           mega_options.gamma,
           mega_options.saturation,
           mega_options.vibrance,
           mega_options.detail_pop,
           mega_options.surface_detail,
           Sysop_TuneHttpEnabled() ? " HTTP tuner enabled" : "");
}

// Clear incremental sampling state for adaptive HDMI palette updates.
static void reset_adaptive_sysop_palette_sampler(void)
{
	memset(g_sysop_adaptive_palette_sum, 0, sizeof(g_sysop_adaptive_palette_sum));
	memset(g_sysop_adaptive_palette_count, 0, sizeof(g_sysop_adaptive_palette_count));
	g_sysop_adaptive_palette_next_y = 0;
	g_sysop_adaptive_palette_base = NULL;
}

// Sample the current frame over several updates and nudge HDMI palette entries
// toward the observed Doom colors.
static void update_adaptive_sysop_palette(void)
{
	int next_palette[16][3];
	const int (*base_palette)[3] = Sysop_ImageBaseDisplayPalette();

	if (!(g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_ADAPTIVE)) {
		return;
	}

	if (g_sysop_palette_update_interval <= 0) {
		g_sysop_palette_update_interval = SYSOP_PALETTE_UPDATE_DEFAULT;
	}

	if (base_palette != g_sysop_adaptive_palette_base) {
		reset_adaptive_sysop_palette_sampler();
		g_sysop_adaptive_palette_base = base_palette;
	}

	if (g_sysop_adaptive_palette_next_y == 0) {
		memset(g_sysop_adaptive_palette_sum, 0, sizeof(g_sysop_adaptive_palette_sum));
		memset(g_sysop_adaptive_palette_count, 0, sizeof(g_sysop_adaptive_palette_count));
	}

	int rows_per_frame = (SCREENHEIGHT + g_sysop_palette_update_interval - 1)
	                   / g_sysop_palette_update_interval;
	if (rows_per_frame < SYSOP_ADAPTIVE_SAMPLE_Y_STEP) {
		rows_per_frame = SYSOP_ADAPTIVE_SAMPLE_Y_STEP;
	}

	int start_y = g_sysop_adaptive_palette_next_y;
	int end_y = start_y + rows_per_frame;
	if (end_y > SCREENHEIGHT) {
		end_y = SCREENHEIGHT;
	}

	for (int y = start_y; y < end_y; y += SYSOP_ADAPTIVE_SAMPLE_Y_STEP) {
		int x_phase = (y >> 1) & (SYSOP_ADAPTIVE_SAMPLE_X_STEP - 1);

		for (int x = x_phase; x < SCREENWIDTH; x += SYSOP_ADAPTIVE_SAMPLE_X_STEP) {
			const uint8_t *p = (const uint8_t *)&g_sysop_rgba_screen[x + y * SCREENWIDTH];
			int color;

			if (p[3] > 0 && p[3] < 16) {
				continue;
			}

			color = Sysop_ImageNearestC64PaletteColorRgb(p[2], p[1], p[0]);
			g_sysop_adaptive_palette_sum[color][0] += p[2];
			g_sysop_adaptive_palette_sum[color][1] += p[1];
			g_sysop_adaptive_palette_sum[color][2] += p[0];
			g_sysop_adaptive_palette_count[color]++;
		}
	}

	g_sysop_adaptive_palette_next_y = end_y;

	if (g_sysop_adaptive_palette_next_y < SCREENHEIGHT) {
		return;
	}

	Sysop_ImageCopyPalette(next_palette, sysop_active_palette);

	for (int color = 0; color < 16; color++) {
		if (color == 0 || color == 1) {
			next_palette[color][0] = base_palette[color][0];
			next_palette[color][1] = base_palette[color][1];
			next_palette[color][2] = base_palette[color][2];
			continue;
		}

		if (g_sysop_adaptive_palette_count[color] >= SYSOP_ADAPTIVE_MIN_COLOR_SAMPLES) {
			int avg_r = (int)(g_sysop_adaptive_palette_sum[color][0] / g_sysop_adaptive_palette_count[color]);
			int avg_g = (int)(g_sysop_adaptive_palette_sum[color][1] / g_sysop_adaptive_palette_count[color]);
			int avg_b = (int)(g_sysop_adaptive_palette_sum[color][2] / g_sysop_adaptive_palette_count[color]);

			next_palette[color][0] = mix_component(sysop_active_palette[color][0], avg_r, 1, 4);
			next_palette[color][1] = mix_component(sysop_active_palette[color][1], avg_g, 1, 4);
			next_palette[color][2] = mix_component(sysop_active_palette[color][2], avg_b, 1, 4);

			next_palette[color][0] = mix_component(next_palette[color][0], base_palette[color][0], 1, 12);
			next_palette[color][1] = mix_component(next_palette[color][1], base_palette[color][1], 1, 12);
			next_palette[color][2] = mix_component(next_palette[color][2], base_palette[color][2], 1, 12);
		} else {
			next_palette[color][0] = mix_component(sysop_active_palette[color][0], base_palette[color][0], 1, 16);
			next_palette[color][1] = mix_component(sysop_active_palette[color][1], base_palette[color][1], 1, 16);
			next_palette[color][2] = mix_component(sysop_active_palette[color][2], base_palette[color][2], 1, 16);
		}
	}

	Sysop_ImageCopyPalette(sysop_active_palette, next_palette);
	build_status_split_palette(sysop_active_palette, sysop_status_palette);
	g_sysop_palette_revision++;

	g_sysop_adaptive_palette_next_y = 0;
}

// Queue a complete HDMI palette change at a specific display scanline.
static void queue_palette_slice(uint16_t hdmi_y, const int palette[16][3])
{
	sysop_wait_set_palette_entry(1,
								 hdmi_y,
								 0,
								 255,
								 clamp_u8_int(palette[0][0]),
								 clamp_u8_int(palette[0][1]),
								 clamp_u8_int(palette[0][2]));

	for (int color = 1; color < 16; color++) {
		sysop_queue_set_palette_entry((uint8_t)color,
									  255,
									  clamp_u8_int(palette[color][0]),
									  clamp_u8_int(palette[color][1]),
									  clamp_u8_int(palette[color][2]));
	}
}

// Queue per-frame HDMI palette slices such as the status-bar split.
static void schedule_sysop_palette_effects(void)
{
    const uint16_t hdmi_frame_reset_y = 1;
    const uint16_t hdmi_c64_y_offset = 136;
    const uint16_t status_split_y = hdmi_c64_y_offset + (uint16_t)((SCREENHEIGHT - 32) * 4);

    if (!(g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_STATUS_SPLIT)) {
        return;
    }

    if (sysop_should_use_menu_palette()) {
        queue_palette_slice(hdmi_frame_reset_y, sysop_menu_palette);
        queue_palette_slice(status_split_y, sysop_menu_status_palette);
    } else {
        queue_palette_slice(hdmi_frame_reset_y, sysop_active_palette);
        queue_palette_slice(status_split_y, sysop_status_palette);
    }
}

// Return HDMI and converter palette state to the stable base palette.
static void restore_sysop_palette_effects(void)
{
    const int (*base_palette)[3] = Sysop_ImageBaseDisplayPalette();

    Sysop_ImageCopyPalette(sysop_active_palette, base_palette);
    build_status_split_palette(sysop_active_palette, sysop_status_palette);
    g_sysop_installed_base_palette = base_palette;
    g_sysop_palette_revision++;
    Sysop_ImageSetGameplayConversionPaletteFrom(sysop_active_palette);
    Sysop_ImageRecomputeColorQuantizationTables();
    set_sysop_palette_from(sysop_active_palette);
    g_sysop_menu_palette_active = -1;
    g_sysop_runtime_palette_revision = 0;
}

// Expose adaptive palette state to the HTTP tuner.
int Sysop_AdaptivePaletteEnabled(void)
{
    return (g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_ADAPTIVE) != 0;
}

// Let the HTTP tuner enable or disable adaptive HDMI palette updates at
// runtime.
void Sysop_SetAdaptivePaletteEnabled(int enabled)
{
    int was_enabled = Sysop_AdaptivePaletteEnabled();

    reset_adaptive_sysop_palette_sampler();

    if (enabled) {
        g_sysop_palette_effects |= SYSOP_PALETTE_EFFECT_ADAPTIVE;
    } else {
        g_sysop_palette_effects &= ~SYSOP_PALETTE_EFFECT_ADAPTIVE;
    }

    if (was_enabled && !enabled) {
        restore_sysop_palette_effects();
    } else if (!was_enabled && enabled) {
        g_sysop_runtime_palette_revision = 0;
    }
}

// The ARGB framebuffer overlay is separate from the C64 VIC output. It is used
// for debug displays, HUD text overlays, and split-screen comparison against
// the converted C64 image.
// 512MB is the start
#define MEM_ADDRESS1 0x20000000
#define MEM_ADDRESS2 0x207e9000

// One 1920x1080 ARGB8888 buffer is 0x7e9000 bytes; map enough for either
// sysop-fb's exported buffers or the older /dev/mem fallback.
#define MEM_SIZE (16*1024*1024)
#define SYSOP_FB_WIDTH 1920
#define SYSOP_FB_HEIGHT 1080
#define SYSOP_FB_BYTES (SYSOP_FB_WIDTH * SYSOP_FB_HEIGHT * 4)
#define SYSOP_FB_C64_X_OFFSET 192
#define SYSOP_FB_C64_PIXEL_SCALE 4
#define SYSOP_FB_HUD_VISIBLE_FIRST_VIC_LINE 24
#define SYSOP_FB_HUD_VISIBLE_X_OFFSET 0
#define SYSOP_FB_SPLIT_PAL_Y_OFFSET_LINES 1
#define SYSOP_FB_SPLIT_NTSC_EXTRA_Y_OFFSET_LINES 4
#define SYSOP_FRAMEBUFFER_SPLIT_JOYSTICK_PORT 2
#define SYSOP_FRAMEBUFFER_SPLIT_STEP 10
#define SYSOP_FRAMEBUFFER_SPLIT_LINE_COLOR 0xffffffffu

int framebuffer_visible = 0;
unsigned char* pFrameBuffer = NULL;
unsigned char* pFrameBuffer1 = NULL;
unsigned char* pFrameBuffer2 = NULL;

int g_framebuffer_width = SYSOP_FB_WIDTH;
int g_framebuffer_height = SYSOP_FB_HEIGHT;
int font_size = 30;

int fd_sysop = -1;
static pthread_t g_framebuffer_hud_thread;
static volatile int g_framebuffer_hud_thread_running = 0;
static int g_framebuffer_hud_thread_started = 0;
static int g_framebuffer_debug_started = 0;

// Map both Sysop ARGB overlay buffers and clear them before first use.
static int map_framebuffer_pair(int fd, off_t offset1, off_t offset2)
{
    pFrameBuffer1 = (unsigned char*)mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset1);
    pFrameBuffer2 = (unsigned char*)mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset2);

    if (pFrameBuffer1 == MAP_FAILED || pFrameBuffer2 == MAP_FAILED) {
        if (pFrameBuffer1 != MAP_FAILED) {
            munmap(pFrameBuffer1, MEM_SIZE);
        }
        if (pFrameBuffer2 != MAP_FAILED) {
            munmap(pFrameBuffer2, MEM_SIZE);
        }
        pFrameBuffer1 = NULL;
        pFrameBuffer2 = NULL;
        return 0;
    }

    memset(pFrameBuffer1, 0x0, SYSOP_FB_BYTES);
    memset(pFrameBuffer2, 0x0, SYSOP_FB_BYTES);
    pFrameBuffer = pFrameBuffer1;
    return 1;
}

// Prefer the sysop framebuffer device when available. The /dev/mem fallback is
// kept for older images where the two overlay buffers are exposed only as raw
// physical memory.
int init_framebuffer()
{
    if (pFrameBuffer1 && pFrameBuffer2) {
        return 1;
    }

    fd_sysop = open("/dev/sysop-fb", O_RDWR | O_SYNC);
    if (fd_sysop >= 0) {
        if (map_framebuffer_pair(fd_sysop, 0, MEM_ADDRESS2 - MEM_ADDRESS1)) {
            printf("Mapped sysop framebuffer overlay via /dev/sysop-fb at %p and %p\n",
                   pFrameBuffer1, pFrameBuffer2);
            return 1;
        }

        perror("Couldn't mmap /dev/sysop-fb");
        close(fd_sysop);
        fd_sysop = -1;
    }

    fd_sysop = open("/dev/mem", O_RDWR | O_SYNC);

    if (fd_sysop < 0) {
        perror("Couldn't open /dev/sysop-fb or /dev/mem");
        return 0;
    }

    if (!map_framebuffer_pair(fd_sysop, MEM_ADDRESS1, MEM_ADDRESS2)) {
        perror("Couldn't mmap /dev/mem framebuffers");
        close(fd_sysop);
        fd_sysop = -1;
        return 0;
    }

    printf("Mapped sysop framebuffer overlay via /dev/mem at %p and %p\n",
           pFrameBuffer1, pFrameBuffer2);
    return 1;
}

// Fill a rectangle in an ARGB overlay buffer, clipping to the 1080p surface.
static void framebuffer_hud_fill_rect(uint32_t *buffer,
                                      int left,
                                      int top,
                                      int right,
                                      int bottom,
                                      uint32_t argb) {
    if (!buffer) {
        return;
    }

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right >= SYSOP_FB_WIDTH) right = SYSOP_FB_WIDTH - 1;
    if (bottom >= SYSOP_FB_HEIGHT) bottom = SYSOP_FB_HEIGHT - 1;
    if (right < left || bottom < top) {
        return;
    }

    for (int y = top; y <= bottom; y++) {
        uint32_t *row = buffer + y * SYSOP_FB_WIDTH;
        for (int x = left; x <= right; x++) {
            row[x] = argb;
        }
    }
}

// Draw one compact HUD glyph into the ARGB overlay buffer.
static void framebuffer_hud_draw_char(uint32_t *buffer,
                                      int x,
                                      int y,
                                      char ch,
                                      int scale,
                                      uint32_t argb) {
    for (int row = 0; row < 7; row++) {
        uint8_t bits = c64_hud_glyph_row(ch, row);
        for (int col = 0; col < 4; col++) {
            if (!(bits & (1 << (3 - col)))) {
                continue;
            }

            framebuffer_hud_fill_rect(buffer,
                                      x + col * scale,
                                      y + row * scale,
                                      x + col * scale + scale - 1,
                                      y + row * scale + scale - 1,
                                      argb);
        }
    }
}

// Render the framebuffer-overlay HUD message layer for the current frame.
static void framebuffer_hud_render_to_buffer(uint32_t *buffer) {
    const int scale = SYSOP_FB_C64_PIXEL_SCALE;
    const int gap = scale;
    const int glyph_width = 4 * scale;
    const int glyph_height = 7 * scale;
    const int line_height = glyph_height + 2 * scale;
    const int pad = 2 * scale;
    const int bg_left = SYSOP_FB_HUD_VISIBLE_X_OFFSET + 2 * scale;
    const int text_left = bg_left + pad;
    const int bitmap_top_y = (C64_BITMAP_FIRST_BADLINE - SYSOP_FB_HUD_VISIBLE_FIRST_VIC_LINE) * SYSOP_FB_C64_PIXEL_SCALE;
    char lines[C64_HUD_TEXT_ROWS][C64_BITMAP_CHAR_COLS + 1];
    int line_lengths[C64_HUD_TEXT_ROWS];
    int used_rows;
    int max_line_width = 0;
    int text_height;
    int origin_y;

    if (!buffer) {
        return;
    }

    // This overlay is intentionally independent of the C64 bitmap upload; it
    // draws readable Doom messages above the VIC image without spending C64
    // poke budget.
    memset(buffer, 0, SYSOP_FB_BYTES);

    used_rows = c64_hud_layout_message(lines, line_lengths);
    if (used_rows <= 0) {
        return;
    }

    for (int row = 0; row < used_rows; row++) {
        int len = line_lengths[row];
        int line_width;

        if (len <= 0) {
            continue;
        }

        line_width = len * glyph_width + (len - 1) * gap;
        if (line_width > max_line_width) {
            max_line_width = line_width;
        }
    }

    if (max_line_width <= 0) {
        return;
    }

    text_height = (used_rows - 1) * line_height + glyph_height;
    origin_y = bitmap_top_y - text_height;
    if (origin_y < pad) {
        origin_y = pad;
    }

    framebuffer_hud_fill_rect(buffer,
                              bg_left,
                              origin_y - pad,
                              bg_left + max_line_width + 2 * pad - 1,
                              bitmap_top_y - 1,
                              0xC0000000u);

    for (int row = 0; row < used_rows; row++) {
        int len = line_lengths[row];

        if (len <= 0) {
            continue;
        }

        for (int col = 0; col < len; col++) {
            framebuffer_hud_draw_char(buffer,
                                      text_left + col * (glyph_width + gap),
                                      origin_y + row * line_height,
                                      lines[row][col],
                                      scale,
                                      0xFFFFFFFFu);
        }
    }
}

// Worker thread that refreshes the independent ARGB HUD overlay at video rate.
static void *framebuffer_hud_thread_function(void *arg) {
    (void)arg;

    pFrameBuffer = pFrameBuffer1;
    sysop_framebuffer_show();
    framebuffer_visible = 1;

    while (g_framebuffer_hud_thread_running) {
        sysop_framebuffer_flip();
        framebuffer_hud_render_to_buffer((uint32_t*)pFrameBuffer);
        pFrameBuffer = (pFrameBuffer == pFrameBuffer1) ? pFrameBuffer2 : pFrameBuffer1;
        usleep(16666);
    }

    if (pFrameBuffer1) {
        memset(pFrameBuffer1, 0, SYSOP_FB_BYTES);
    }
    if (pFrameBuffer2) {
        memset(pFrameBuffer2, 0, SYSOP_FB_BYTES);
    }
    sysop_framebuffer_flip();
    sysop_framebuffer_hide();
    framebuffer_visible = 0;

    return NULL;
}

// Start the ARGB HUD overlay path or fall back to bitmap messages if unavailable.
static void start_framebuffer_hud_overlay(void) {
    if (g_framebuffer_hud_thread_started) {
        return;
    }

    if (!init_framebuffer()) {
        printf("Unable to initialize sysop framebuffer HUD overlay; using bitmap HUD messages.\n");
        g_sysop_hud_mode = SYSOP_HUD_MODE_BITMAP;
        return;
    }

    g_framebuffer_hud_thread_running = 1;
    if (pthread_create(&g_framebuffer_hud_thread, NULL, framebuffer_hud_thread_function, NULL) != 0) {
        perror("Failed to create sysop framebuffer HUD thread");
        g_framebuffer_hud_thread_running = 0;
        g_sysop_hud_mode = SYSOP_HUD_MODE_BITMAP;
        return;
    }

    g_framebuffer_hud_thread_started = 1;
    printf("HUD messages: sysop framebuffer overlay\n");
}

// Stop the ARGB HUD overlay thread and clear its buffers.
static void stop_framebuffer_hud_overlay(void) {
    if (!g_framebuffer_hud_thread_started) {
        return;
    }

    g_framebuffer_hud_thread_running = 0;
    pthread_join(g_framebuffer_hud_thread, NULL);
    g_framebuffer_hud_thread_started = 0;
}

// Clear both ARGB overlay buffers used by framebuffer debug modes.
static void framebuffer_debug_clear_buffers(void)
{
    if (pFrameBuffer1) {
        memset(pFrameBuffer1, 0, SYSOP_FB_BYTES);
    }
    if (pFrameBuffer2) {
        memset(pFrameBuffer2, 0, SYSOP_FB_BYTES);
    }
}

// Start raw-framebuffer or split-comparison display modes.
static void start_framebuffer_debug_display(void)
{
    if ((!g_sysop_framebuffer_debug_enabled && !g_sysop_framebuffer_split_enabled)
        || g_framebuffer_debug_started) {
        return;
    }

    if (!init_framebuffer()) {
        printf("Unable to initialize sysop framebuffer debug display; falling back to C64 conversion.\n");
        g_sysop_framebuffer_debug_enabled = 0;
        g_sysop_framebuffer_split_enabled = 0;
        return;
    }

    framebuffer_debug_clear_buffers();
    sysop_framebuffer_show();
    pFrameBuffer = pFrameBuffer2;
    framebuffer_visible = 1;
    g_framebuffer_debug_started = 1;
    if (g_sysop_framebuffer_split_enabled) {
        printf("Sysop framebuffer split compare: left half raw Doom framebuffer overlay, right half transparent over C64 output.\n");
    } else {
        printf("Sysop framebuffer debug display: showing raw Chocolate Doom 320x200 buffer at 4x scale with vblanked double buffering.\n");
    }
}

// Stop framebuffer debug output and hide the ARGB overlay.
static void stop_framebuffer_debug_display(void)
{
    if (!g_framebuffer_debug_started) {
        return;
    }

    framebuffer_debug_clear_buffers();
    sysop_framebuffer_hide();
    framebuffer_visible = 0;
    g_framebuffer_debug_started = 0;
}

// Present the current ARGB debug buffer and select the other buffer for drawing.
static void framebuffer_debug_flip_buffers(void)
{
    sysop_framebuffer_flip();
    pFrameBuffer = (pFrameBuffer == pFrameBuffer1) ? pFrameBuffer2 : pFrameBuffer1;
    sysop_wait_hdmi_vblank();
}

// Let joystick port 2 adjust the split-comparison line when it is not being
// used for gameplay.
static void update_framebuffer_split_position_from_joystick(void)
{
    uint8_t joy;
    int left;
    int right;

    // Split adjustment borrows joystick port 2 only when gameplay joystick
    // control is disabled, so calibration never steals active player input.
    if (!g_sysop_framebuffer_split_enabled
        || g_sysop_joystick_enabled
        || !Sysop_LibraryIsInitialized()) {
        return;
    }

    joy = sysop_read_joystick(SYSOP_FRAMEBUFFER_SPLIT_JOYSTICK_PORT);
    if (joy == 0) {
        joy = 0xff;
    }

    left = (joy & 0x04) == 0;
    right = (joy & 0x08) == 0;

    if (left && !right) {
        g_sysop_framebuffer_split_width -= SYSOP_FRAMEBUFFER_SPLIT_STEP;
    } else if (right && !left) {
        g_sysop_framebuffer_split_width += SYSOP_FRAMEBUFFER_SPLIT_STEP;
    }

    if (g_sysop_framebuffer_split_width < 0) {
        g_sysop_framebuffer_split_width = 0;
    } else if (g_sysop_framebuffer_split_width > SCREENWIDTH) {
        g_sysop_framebuffer_split_width = SCREENWIDTH;
    }
}

// Animate the split-comparison line for unattended display demos.
static void update_framebuffer_split_position_demo(void)
{
    if (!g_sysop_framebuffer_split_enabled || !g_sysop_framebuffer_split_demo_enabled) {
        return;
    }

    g_sysop_framebuffer_split_width +=
        g_sysop_framebuffer_split_demo_direction * SYSOP_FRAMEBUFFER_SPLIT_STEP;

    if (g_sysop_framebuffer_split_width >= SCREENWIDTH) {
        g_sysop_framebuffer_split_width = SCREENWIDTH;
        g_sysop_framebuffer_split_demo_direction = -1;
    } else if (g_sysop_framebuffer_split_width <= 0) {
        g_sysop_framebuffer_split_width = 0;
        g_sysop_framebuffer_split_demo_direction = 1;
    }
}

// Return the raw-framebuffer overlay's split-mode Y correction in framebuffer
// pixels. PAL lines already align with the one-line offset; NTSC needs the
// overlay raised further because the visible C64 image lands lower in HDMI
// space on those VIC models.
static int framebuffer_split_y_offset_pixels(int scale)
{
    int offset_lines = SYSOP_FB_SPLIT_PAL_Y_OFFSET_LINES;

    if (!g_sysop_framebuffer_split_enabled) {
        return 0;
    }

    if (g_vic_model == VIC_CHIP_6567R56A || g_vic_model == VIC_CHIP_6567R8) {
        offset_lines += SYSOP_FB_SPLIT_NTSC_EXTRA_Y_OFFSET_LINES;
    }

    return offset_lines * scale;
}

// Draw Chocolate Doom's raw indexed framebuffer into the ARGB overlay for
// visual comparison with the converted C64 output.
static void draw_framebuffer_debug_image(void)
{
    const int scale = 4;
    const int dst_width = SCREENWIDTH * scale;
    const int dst_height = SCREENHEIGHT * scale;
    const int dst_x = (SYSOP_FB_WIDTH - dst_width) / 2;
    const int dst_y = ((SYSOP_FB_HEIGHT - dst_height) / 2)
                      - framebuffer_split_y_offset_pixels(scale);
    uint32_t *buffer = (uint32_t *)pFrameBuffer;

    if (!g_framebuffer_debug_started || buffer == NULL || I_VideoBuffer == NULL) {
        return;
    }

    memset(buffer, 0, SYSOP_FB_BYTES);

    // Full framebuffer debug bypasses the C64 converter. Split mode keeps the
    // C64 upload active and draws only the left portion of the raw Doom frame
    // so both outputs can be compared on the same HDMI display.
    const int src_width = g_sysop_framebuffer_split_enabled
                          ? g_sysop_framebuffer_split_width
                          : SCREENWIDTH;

    for (int y = 0; y < SCREENHEIGHT; y++) {
        uint32_t *first_row = buffer + (dst_y + y * scale) * SYSOP_FB_WIDTH + dst_x;

        for (int x = 0; x < src_width; x++) {
            byte index = I_VideoBuffer[y * SCREENWIDTH + x];
            uint32_t r = sysop_choco_palette[index][0];
            uint32_t g = sysop_choco_palette[index][1];
            uint32_t b = sysop_choco_palette[index][2];
            uint32_t argb = 0xff000000u | (r << 16) | (g << 8) | b;

            for (int sx = 0; sx < scale; sx++) {
                first_row[x * scale + sx] = argb;
            }
        }

        for (int sy = 1; sy < scale; sy++) {
            memcpy(first_row + sy * SYSOP_FB_WIDTH,
                   first_row,
                   (size_t)dst_width * sizeof(*first_row));
        }
    }

    if (g_sysop_framebuffer_split_enabled) {
        const int split_x = dst_x + g_sysop_framebuffer_split_width * scale;

        if (split_x >= 0 && split_x < SYSOP_FB_WIDTH) {
            for (int y = 0; y < SYSOP_FB_HEIGHT; ++y) {
                buffer[y * SYSOP_FB_WIDTH + split_x] = SYSOP_FRAMEBUFFER_SPLIT_LINE_COLOR;
            }
        }
    }

    framebuffer_debug_flip_buffers();
}

// Initialize Sysop video, palette, input, overlays, and SID pieces after
// Chocolate Doom has allocated its video buffers.
static void Sysop_BackendInit(void)
{
    capture_sysop_startup_palette();
    install_sysop_base_palette();

    for (uint16_t i=0;i<0xFFFF;i++)
    {
        shadow_buffer[i] = 0xFF;
    }


    sysop_poke(0xd020, 0);
    configure_vic_timing();
    setup_multicolor_bitmap_mode();

    // Framebuffer debug modes need the ARGB overlay, but only the full debug
    // mode bypasses the C64 upload path. Split mode starts the overlay here and
    // still runs the normal conversion/upload loop every frame.
    if (g_sysop_framebuffer_debug_enabled || g_sysop_framebuffer_split_enabled) {
        start_framebuffer_debug_display();
        if (g_sysop_framebuffer_debug_enabled || g_sysop_framebuffer_split_enabled) {
            g_sysop_hud_mode = SYSOP_HUD_MODE_OFF;
        }
    }

    if (g_sysop_hud_mode == SYSOP_HUD_MODE_FRAMEBUFFER
        && !g_sysop_framebuffer_debug_enabled
        && !g_sysop_framebuffer_split_enabled) {
        start_framebuffer_hud_overlay();
    } else if (g_sysop_hud_mode == SYSOP_HUD_MODE_CLEAN_BITMAP) {
        printf("HUD messages: clean C64 bitmap overlay\n");
    } else if (g_sysop_hud_mode == SYSOP_HUD_MODE_BITMAP) {
        printf("HUD messages: legacy C64 bitmap overlay\n");
    } else {
        printf("HUD messages: off\n");
    }

    printf("Menu rendering: %s\n",
           sysop_clean_menu_requested() ? "clean C64 bitmap overlay" : "DOOM patches");

    printf("Status bar rendering: %s\n",
           g_sysop_clean_status_enabled ? "clean C64 bitmap overlay" : "DOOM patches");

    printf("Intermission rendering: %s\n",
           g_sysop_clean_intermission_enabled ? "clean C64 bitmap overlay" : "DOOM patches");

    printf("Menu/intermission palette mode: %s\n",
           g_sysop_menu_palette_mode == SYSOP_MENU_PALETTE_SHARP ? "sharp red/gold tuning" : "normal frame palette");

    if (g_sysop_framebuffer_debug_enabled) {
        printf("C64 bitmap conversion/upload: bypassed for framebuffer debug display\n");
    } else if (g_sysop_framebuffer_split_enabled) {
        printf("C64 bitmap conversion/upload: active under framebuffer split compare overlay\n");
        if (g_sysop_framebuffer_split_demo_enabled) {
            printf("Framebuffer split demo: auto-sweeping split line left/right\n");
        } else if (!g_sysop_joystick_enabled) {
            printf("Framebuffer split adjust: joystick port 2 left/right moves split line\n");
        }
    }

    if (g_sysop_mouse_enabled) {
        Sysop_MouseInit();
    } else {
        printf("Sysop mouse: off\n");
    }

    printf("Sysop C64 joystick port 2: %s\n",
           g_sysop_joystick_enabled
           ? "on (tap fire shoots; hold fire modifies run/strafe/use)"
           : "off");

    init_doom_sid_player();

    printf("init complete\n");
}

// Convert and upload one frame, then scan Sysop input for the next game tic.
static void Sysop_BackendDrawFrame(void)
{
    // Full framebuffer debug is a raw-source reference path. It intentionally
    // skips converter work and C64 DMA so timing can be compared against the
    // normal path without changing the source image.
    if (g_sysop_framebuffer_debug_enabled && !g_sysop_framebuffer_split_enabled) {
        draw_framebuffer_debug_image();
        Sysop_KeyboardScan();
        return;
    }

	if ( first )
	{
		first = 0;
		Sysop_ImagePrecomputeColorQuantization();
	}

    // Keep the expensive frame work ordered: first tune any HDMI palette state,
    // then convert indexed Doom pixels to the C64 frame, draw clean overlays,
    // and finally emit the scheduled C64 writes.
    update_adaptive_sysop_palette();
    apply_sysop_runtime_palette_state();
    Sysop_UpdateIndexedLockTable();
    Sysop_ImageConvertMegaIndexed(I_VideoBuffer, sysop_choco_palette,
                                  sysop_choco_palette_locks,
                                  g_sysop_menu_dither_mode);
    draw_c64_clean_status_overlay();
    if (g_sysop_hud_mode == SYSOP_HUD_MODE_CLEAN_BITMAP) {
        draw_c64_clean_hud_message_overlay();
    } else if (g_sysop_hud_mode == SYSOP_HUD_MODE_BITMAP) {
        draw_c64_legacy_hud_message_overlay();
    }
    draw_c64_clean_intermission_overlay();
    draw_c64_clean_menu_overlay();

    schedule_sysop_palette_effects();
    BeginFrameDmaTag();
    upload_koala_frame_scheduled();

    if (g_sysop_framebuffer_split_enabled) {
        if (g_sysop_framebuffer_split_demo_enabled) {
            draw_framebuffer_debug_image();
            update_framebuffer_split_position_demo();
        } else {
            update_framebuffer_split_position_from_joystick();
            draw_framebuffer_debug_image();
        }
    }

    Sysop_KeyboardScan();
}

// Sleep helper used by Chocolate Doom's platform timer wrappers.
static void Sysop_SleepMs(uint32_t ms)
{
    usleep (ms * 1000);
}

// Return a millisecond timer value for Chocolate Doom's I_GetTimeMS hook.
static uint32_t Sysop_GetTicksMs(void)
{
    struct timeval  tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);

    // Return milliseconds.
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

// Handle Ctrl-C by stopping helper threads and returning Sysop hardware to a
// quiet state.
void sigintHandler(int signal)
{
    // SIGINT can arrive while audio/video/helper threads are active. Tear down
    // hardware-facing pieces in the same order as normal shutdown, then force
    // the sysop library closed in case the usual atexit path does not run.
    if (Sysop_LibraryIsInitialized()) {
        sysop_poke(0xd020, 11);
    }
    Sysop_TuneHttpStop();
    stop_framebuffer_debug_display();
    stop_framebuffer_hud_overlay();
    Sysop_MouseShutdown();
    stop_sid_thread();
    silence_sid_registers();
    doom_sid_shutdown();
    restore_sysop_startup_palette();
    //dma_disable();
    sysop_server_dma_unlock();
    sysop_server_disconnect();
    Sysop_ForceReleaseLibrary();
    exit(signal);
}


// -----------------------------------------------------------------------------
// Chocolate Doom platform glue
// -----------------------------------------------------------------------------

static boolean sysop_graphics_initialized = false;
static const char *sysop_window_title = "Chocolate Doom";
static boolean sysop_display_fps_dots = false;

pixel_t *I_VideoBuffer = NULL;
char *video_driver = "sysop64";
boolean screenvisible = true;
int vanilla_keyboard_mapping = true;
boolean screensaver_mode = false;
int usegamma = 0;
int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;
int fullscreen = true;
int aspect_ratio_correct = true;
int integer_scaling = false;
int smooth_pixel_scaling = false;
int vga_porch_flash = false;
int force_software_renderer = true;
int png_screenshots = 0;
char *window_position = "center";
unsigned int joywait = 0;
int use_analog = 0;
int joystick_turn_sensitivity = 10;
int joystick_move_sensitivity = 10;
int joystick_look_sensitivity = 10;
int usemouse = 0;
float mouse_acceleration = 2.0f;
int mouse_threshold = 10;
int novert = 0;

// Force obvious menu reds/golds to stable C64 colors for the hard posterized
// menu dither mode.
static uint8_t Sysop_MenuDitherPosterizedLockedColor(byte palette_index, uint32_t r, uint32_t g, uint32_t b)
{
    int luma;
    int red_dominant;
    int warm_gold;

    if (palette_index == 0) {
        return 0;
    }

    luma = (int)((r * 30 + g * 59 + b * 11) / 100);
    red_dominant = r >= 72 && r > g + 18 && r > b + 20;
    warm_gold = r >= 112 && g >= 54 && b <= 110 && r > b + 32 && g > b + 12;

    if (!red_dominant && !warm_gold) {
        return 0;
    }

    if (luma < 70 || (red_dominant && r < 112)) {
        return 11;
    }

    if (warm_gold && g >= 124 && r >= 150) {
        return 7;
    }

    if (r >= 176 || (red_dominant && luma >= 102)) {
        return 10;
    }

    return 2;
}

// Optionally lock red/gold Doom menu pixels to a small stable C64 color set.
static uint8_t Sysop_MenuDitherLockedColor(byte palette_index, uint32_t r, uint32_t g, uint32_t b)
{
    if (g_sysop_menu_dither_mode == SYSOP_MENU_DITHER_POSTER) {
        return Sysop_MenuDitherPosterizedLockedColor(palette_index, r, g, b);
    }

    if (palette_index == 0) {
        return 0;
    }

    if (r >= 84 && r > g + 24 && r > b + 24) {
        if (r >= 190 || (r >= 168 && g >= 44)) {
            return 10;
        }
        if (g >= 48 && b <= 64) {
            return 8;
        }
        if (r >= 104) {
            return 2;
        }
        return 9;
    }

    if (r >= 136 && g >= 76 && b <= 96 && r > b + 40 && g > b + 24) {
        if (g >= 138 || r >= 190) {
            return 7;
        }
        return 8;
    }

    return 0;
}

// Refresh per-PLAYPAL-index color locks used by menu/intermission readability
// tuning.
static void Sysop_UpdateIndexedLockTable(void)
{
    int use_menu_locks = g_sysop_menu_dither_mode != SYSOP_MENU_DITHER_OFF
                         && sysop_should_use_red_patch_treatment();

    if (!use_menu_locks) {
        memset(sysop_choco_palette_locks, 0, sizeof(sysop_choco_palette_locks));
        return;
    }

    for (int i = 0; i < 256; ++i) {
        uint32_t r = sysop_choco_palette[i][0];
        uint32_t g = sysop_choco_palette[i][1];
        uint32_t b = sysop_choco_palette[i][2];
        sysop_choco_palette_locks[i] = Sysop_MenuDitherLockedColor((byte)i, r, g, b);
    }
}

// Decide whether the RGBA scratch buffer is needed for this frame.
static int Sysop_ShouldRefreshRGBABuffer(void)
{
    return (g_sysop_palette_effects & SYSOP_PALETTE_EFFECT_ADAPTIVE) != 0;
}

// Expand Doom's indexed framebuffer to RGBA for adaptive palette sampling.
static void Sysop_UpdateRGBABufferFromIndexed(void)
{
    int use_menu_locks = g_sysop_menu_dither_mode != SYSOP_MENU_DITHER_OFF
                         && sysop_should_use_red_patch_treatment();

    if (I_VideoBuffer == NULL) {
        return;
    }

    // Alpha doubles as a tiny side channel for the adaptive palette sampler:
    // 0xff means ordinary pixel, while small nonzero values mark menu colors
    // that should stay locked during red-patch readability tuning.
    for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i) {
        byte index = I_VideoBuffer[i];
        uint32_t r = sysop_choco_palette[index][0];
        uint32_t g = sysop_choco_palette[index][1];
        uint32_t b = sysop_choco_palette[index][2];
        uint8_t lock = use_menu_locks ? Sysop_MenuDitherLockedColor(index, r, g, b) : 0;
        uint32_t alpha = lock ? lock : 0xffu;
        g_sysop_rgba_screen[i] = (alpha << 24) | (r << 16) | (g << 8) | b;
    }
}

// Apply Sysop-friendly default controls after command line parsing but before
// gameplay starts.
static void configure_sysop_default_controls(void)
{
    int changed = 0;
    int use_mouse_wasd = g_sysop_mouse_wasd_enabled;

    if (key_prevweapon == 0) {
        key_prevweapon = '9';
        changed = 1;
    }

    if (key_nextweapon == 0) {
        key_nextweapon = '0';
        changed = 1;
    }

    if (changed) {
        printf("Sysop weapon cycling: 9 previous, 0 next. Direct weapons remain 1-8.\n");
    }

    if (use_mouse_wasd < 0) {
        use_mouse_wasd = g_sysop_mouse_enabled;
    }

    if (use_mouse_wasd) {
        key_up = 'w';
        key_down = 's';
        key_strafeleft = 'a';
        key_straferight = 'd';
        key_speed = KEY_RSHIFT;
        printf("Sysop mouse controls: W/S move, A/D strafe, Shift run.\n");
    }

    if (g_sysop_mouse_enabled) {
        int use_mouse_novert = g_sysop_mouse_novert;

        if (use_mouse_novert < 0) {
            use_mouse_novert = 1;
        }

        novert = use_mouse_novert ? 1 : 0;

        printf("Sysop mouse vertical movement: %s.\n",
               novert ? "off, turn only" : "on, forward/back");
    }

    printf("Sysop display tuning hotkeys: %s\n",
           g_sysop_display_tune_enabled ? "on" : "off");
}

// Initialize Chocolate Doom's video backend and the Sysop hardware path.
void I_InitGraphics(void)
{
    if (sysop_graphics_initialized) {
        return;
    }

    signal(SIGINT, sigintHandler);
    // Keep this order: graphics derives the default executable-relative SID
    // path, then the shared sysop parser may override it with --sysop-sid. The
    // parser may also have run earlier from sound init, so it is idempotent.
    configure_default_sid_path(myargc > 0 ? myargv[0] : NULL);
    ensure_sysop_backend_args_parsed(myargc, myargv);
    if (g_sysop_mouse_enabled) {
        usemouse = 1;
    }
    configure_sysop_default_controls();

    I_VideoBuffer = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
    if (I_VideoBuffer == NULL) {
        I_Error("I_InitGraphics: failed to allocate video buffer");
    }
    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
    V_RestoreBuffer();

    if (!Sysop_AcquireLibrary("graphics")) {
        I_Error("I_InitGraphics: sysop_init failed");
    }

    if (sysop_server_connect() != 0) {
        printf("Unable to connect to sysop server\n");
    }
    sysop_server_dma_lock();

    Sysop_BackendInit();
    Sysop_TuneHttpStart();
    sysop_graphics_initialized = true;
    I_AtExit(I_ShutdownGraphics, true);
}

// Shut down graphics, helper threads, SID playback, and the Sysop connection.
void I_ShutdownGraphics(void)
{
    if (!sysop_graphics_initialized) {
        return;
    }

    Sysop_TuneHttpStop();
    stop_framebuffer_debug_display();
    stop_framebuffer_hud_overlay();
    Sysop_MouseShutdown();
    stop_sid_thread();
    silence_sid_registers();
    doom_sid_shutdown();
    restore_sysop_startup_palette();
    sysop_server_dma_unlock();
    sysop_server_disconnect();
    Sysop_ReleaseLibrary();

    free(I_VideoBuffer);
    I_VideoBuffer = NULL;
    sysop_graphics_initialized = false;
}

// Present one completed Chocolate Doom frame through the Sysop backend.
void I_FinishUpdate(void)
{
    if (!sysop_graphics_initialized || I_VideoBuffer == NULL) {
        return;
    }

    if (sysop_display_fps_dots) {
        static int lasttic;
        int now = I_GetTime();
        int tics = now - lasttic;
        lasttic = now;
        if (tics > 20) {
            tics = 20;
        }
        for (int i = 0; i < tics * 4; i += 4) {
            I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0xff;
        }
        for (int i = tics * 4; i < 20 * 4; i += 4) {
            I_VideoBuffer[(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0;
        }
    }

    V_DrawDiskIcon();
    // The main converter reads indexed pixels directly. The RGBA copy is only
    // refreshed when an HDMI/adaptive palette feature needs RGB sampling.
    if (Sysop_ShouldRefreshRGBABuffer()) {
        Sysop_UpdateRGBABufferFromIndexed();
    }
    Sysop_BackendDrawFrame();
    V_RestoreDiskBackground();
}

// Store the active Doom palette for indexed conversion and framebuffer debug
// output.
void I_SetPalette(byte *doompalette)
{
    // Chocolate Doom supplies the active PLAYPAL-derived palette here whenever
    // gamma or game state changes. Store it for indexed conversion and raw
    // framebuffer debug output.
    for (int i = 0; i < 256; ++i) {
        sysop_choco_palette[i][0] = gammatable[usegamma][*doompalette++] & ~3;
        sysop_choco_palette[i][1] = gammatable[usegamma][*doompalette++] & ~3;
        sysop_choco_palette[i][2] = gammatable[usegamma][*doompalette++] & ~3;
    }
}

// Find the nearest current Doom palette index for code that needs reverse
// palette lookup.
int I_GetPaletteIndex(int r, int g, int b)
{
    int best = 0;
    int best_diff = INT_MAX;

    for (int i = 0; i < 256; ++i) {
        int dr = r - sysop_choco_palette[i][0];
        int dg = g - sysop_choco_palette[i][1];
        int db = b - sysop_choco_palette[i][2];
        int diff = dr * dr + dg * dg + db * db;
        if (diff < best_diff) {
            best = i;
            best_diff = diff;
            if (diff == 0) {
                break;
            }
        }
    }

    return best;
}

// SDL backends use this to select video drivers; Sysop parses its options
// earlier and has no extra check here.
void I_GraphicsCheckCommandLine(void) {}
// No-blit updates are unnecessary because the Sysop path presents in
// I_FinishUpdate().
void I_UpdateNoBlit(void) {}
// Copy the current indexed framebuffer for Chocolate Doom screenshot/readback
// callers.
void I_ReadScreen(pixel_t *scr) { memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT * sizeof(*scr)); }
// Sysop has no streamed readback setup step.
void I_BeginRead(void) {}
// Per-frame work is handled at tic and finish-update boundaries.
void I_StartFrame(void) {}
// Poll C64 keyboard/joystick/mouse from the tic path so input latency is tied
// to Chocolate Doom's normal event cadence rather than the video upload rate.
void I_StartTic(void) { Sysop_MaybeApplyIDKFA(); Sysop_PostQueuedKeyEvents(); Sysop_JoystickRead(); Sysop_MouseRead(); }
// Retain the title string for callers even though Sysop has no desktop window.
void I_SetWindowTitle(const char *title) { sysop_window_title = title ? title : ""; }
// Sysop never runs as a screensaver, so keep the shared flag false.
void I_CheckIsScreensaver(void) { screensaver_mode = false; }
// Mouse grab callbacks are desktop-window behavior and are ignored on Sysop.
void I_SetGrabMouseCallback(grabmouse_callback_t func) { (void)func; }
// Toggle Chocolate Doom's FPS dot diagnostic in the indexed framebuffer.
void I_DisplayFPSDots(boolean dots_on) { sysop_display_fps_dots = dots_on; }
// Window title initialization is a no-op for the C64/Sysop display.
void I_InitWindowTitle(void) { (void)sysop_window_title; }
// Window icons are unused because there is no host window.
void I_RegisterWindowIcon(const unsigned int *icon, int width, int height) { (void)icon; (void)width; (void)height; }
// Window icon initialization is unused on Sysop.
void I_InitWindowIcon(void) {}
// Report a fixed origin for code that asks about host window placement.
void I_GetWindowPosition(int *x, int *y, int w, int h) { (void)w; (void)h; if (x) *x = 0; if (y) *y = 0; }

// Bind video-related config variables that Chocolate Doom expects to exist.
void I_BindVideoVariables(void)
{
    M_BindIntVariable("fullscreen", &fullscreen);
    M_BindIntVariable("aspect_ratio_correct", &aspect_ratio_correct);
}

// Bind mouse and keyboard config variables used by Chocolate Doom menus.
void I_BindInputVariables(void)
{
    M_BindFloatVariable("mouse_acceleration", &mouse_acceleration);
    M_BindIntVariable("mouse_threshold", &mouse_threshold);
    M_BindIntVariable("vanilla_keyboard_mapping", &vanilla_keyboard_mapping);
    M_BindIntVariable("novert", &novert);
}

// Built-in SDL joystick variables are unused; Sysop joystick handling lives in
// sysop64_joystick.c.
void I_BindJoystickVariables(void) {}
// SDL joystick initialization is unused by the Sysop joystick path.
void I_InitJoystick(void) {}
// SDL joystick shutdown is unused by the Sysop joystick path.
void I_ShutdownJoystick(void) {}
// SDL joystick polling is unused; Sysop polling happens in I_StartTic().
void I_UpdateJoystick(void) {}
// Text input rectangles are desktop-window behavior and are ignored on Sysop.
void I_StartTextInput(int x1, int y1, int x2, int y2) { (void)x1; (void)y1; (void)x2; (void)y2; }
// Text input shutdown is a no-op for the Sysop backend.
void I_StopTextInput(void) {}
// Poll the Sysop mouse driver when callers explicitly request a mouse read.
void I_ReadMouse(void) { Sysop_MouseRead(); }

// Return elapsed milliseconds since Chocolate Doom's Sysop timer was first
// queried.
int I_GetTimeMS(void)
{
    static uint32_t basetime = 0;
    uint32_t ticks = Sysop_GetTicksMs();

    if (basetime == 0) {
        basetime = ticks;
    }

    return (int)(ticks - basetime);
}

// Convert milliseconds to Chocolate Doom's 35 Hz tic clock.
int I_GetTime(void)
{
    return (I_GetTimeMS() * TICRATE) / 1000;
}
// Sleep for the requested number of milliseconds.
void I_Sleep(int ms) { Sysop_SleepMs((uint32_t)ms); }
// Approximate a vertical blank wait in Chocolate Doom's expected 70 Hz units.
void I_WaitVBL(int count) { I_Sleep((count * 1000) / 70); }
// Timer initialization is implicit in the Sysop timer helpers.
void I_InitTimer(void) {}

// -----------------------------------------------------------------------------
// Minimal system layer copied away from SDL dependencies
// -----------------------------------------------------------------------------

typedef struct atexit_listentry_s atexit_listentry_t;
struct atexit_listentry_s
{
    atexit_func_t func;
    boolean run_on_error;
    atexit_listentry_t *next;
};

static atexit_listentry_t *exit_funcs = NULL;
static boolean already_quitting = false;

// Register shutdown callbacks for normal and error exits without SDL.
void I_AtExit(atexit_func_t func, boolean run_on_error)
{
    atexit_listentry_t *entry = malloc(sizeof(*entry));
    if (entry == NULL) {
        fprintf(stderr, "I_AtExit: out of memory\n");
        exit(-1);
    }
    entry->func = func;
    entry->run_on_error = run_on_error;
    entry->next = exit_funcs;
    exit_funcs = entry;
}

// Force-feedback/tactile output is unsupported on Sysop.
void I_Tactile(int on, int off, int total) { (void)on; (void)off; (void)total; }

// Allocate Chocolate Doom's zone memory, honoring the standard -mb option.
byte *I_ZoneBase(int *size)
{
    int default_ram = sizeof(void *) == 8 ? 32 : 16;
    int p = M_CheckParmWithArgs("-mb", 1);
    byte *zonemem;

    if (p > 0) {
        default_ram = atoi(myargv[p + 1]);
    }

    *size = default_ram * 1024 * 1024;
    zonemem = malloc(*size);
    if (zonemem == NULL) {
        I_Error("Unable to allocate %i MiB of RAM for zone", default_ram);
    }

    printf("zone memory: %p, %x allocated for zone\n", zonemem, *size);
    return zonemem;
}

// Sysop runs from a console, so startup/status output should go to stdout.
boolean I_ConsoleStdout(void) { return true; }

// Print a centered startup banner line.
void I_PrintBanner(const char *msg)
{
    int spaces = 35 - ((int)strlen(msg) / 2);
    for (int i = 0; i < spaces; ++i) putchar(' ');
    puts(msg);
}

// Print the standard Chocolate Doom startup divider.
void I_PrintDivider(void)
{
    for (int i = 0; i < 75; ++i) putchar('=');
    putchar('\n');
}

// Print the Sysop-flavored Chocolate Doom startup title.
void I_PrintStartupBanner(const char *gamedescription)
{
    I_PrintDivider();
    I_PrintBanner(gamedescription);
    I_PrintDivider();
    printf(" " PACKAGE_NAME " is free software, covered by the GNU General Public\n"
           " License. There is NO warranty.\n");
    I_PrintDivider();
}

// Bind shared platform variables during Chocolate Doom configuration setup.
void I_BindVariables(void)
{
    I_BindInputVariables();
    I_BindVideoVariables();
    I_BindJoystickVariables();
    I_BindSoundVariables();
}

// Run registered shutdown callbacks and terminate the process cleanly.
void I_Quit(void)
{
    atexit_listentry_t *entry = exit_funcs;
    while (entry != NULL) {
        entry->func();
        entry = entry->next;
    }
    exit(0);
}

// Report a fatal error, run error-enabled cleanup callbacks, and exit.
void I_Error(const char *error, ...)
{
    va_list argptr;
    atexit_listentry_t *entry;

    if (already_quitting) {
        fprintf(stderr, "Warning: recursive call to I_Error detected.\n");
        exit(-1);
    }
    already_quitting = true;

    va_start(argptr, error);
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n\n");
    va_end(argptr);
    fflush(stderr);

    entry = exit_funcs;
    while (entry != NULL) {
        if (entry->run_on_error) {
            entry->func();
        }
        entry = entry->next;
    }

    exit(-1);
}

// Reallocate memory for callers that use Chocolate Doom's platform allocator
// wrapper.
void *I_Realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (size != 0 && new_ptr == NULL) {
        I_Error("I_Realloc: failed on reallocation of %zu bytes", size);
    }
    return new_ptr;
}

// Sysop does not emulate direct low-memory DOS reads, so this compatibility
// hook always fails.
boolean I_GetMemoryValue(unsigned int offset, void *value, int size)
{
    (void)offset;
    memset(value, 0, size);
    return true;
}
