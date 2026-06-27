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
//     Shared declarations for Sysop-64 video, audio, keyboard, mouse, and
//     joystick helper modules.
//

#ifndef SYSOP64_BACKEND_H
#define SYSOP64_BACKEND_H

#include <stdint.h>

#define SYSOP_DOOM_VOLUME_MAX 127
#define SYSOP_DOOM_MENU_VOLUME_MAX 120
#define SYSOP_AUDIO_VOLUME_MAX 255

extern int g_sysop_mouse_enabled;
extern int g_sysop_joystick_enabled;
extern int g_sysop_pcm_sfx_enabled;
extern int g_sysop_sid_music_volume;
extern int g_sysop_key_debug;
extern int g_sysop_display_tune_enabled;

extern float mouse_acceleration;
extern int mouse_threshold;
extern int novert;

// Acquire the shared Sysop library connection for one subsystem.
int Sysop_AcquireLibrary(const char *owner);
// Release one subsystem's hold on the shared Sysop library connection.
void Sysop_ReleaseLibrary(void);
// Return nonzero when sysop_init() has completed and hardware calls are safe.
int Sysop_LibraryIsInitialized(void);
// Force the Sysop library connection closed during fatal/error cleanup.
void Sysop_ForceReleaseLibrary(void);

// Clamp a Chocolate Doom volume value to the range accepted by Sysop helpers.
int Sysop_ClampDoomVolume(int volume);
// Convert Chocolate Doom's menu volume scale into the Sysop audio scale.
uint32_t Sysop_DoomVolumeToSysopVolume(int volume);
// Push the current music volume value to the Sysop SID mixer.
void Sysop_ApplySidMusicVolume(void);
// Parse Sysop-specific command line options once, even if audio initializes
// before video.
void ensure_sysop_backend_args_parsed(int argc, char **argv);
// Return nonzero when adaptive HDMI palette updates are enabled.
int Sysop_AdaptivePaletteEnabled(void);
// Enable or disable adaptive HDMI palette updates at runtime.
void Sysop_SetAdaptivePaletteEnabled(int enabled);

// Initialize the Sysop mouse driver and any state needed by the Doom input path.
void Sysop_MouseInit(void);
// Shut down the Sysop mouse driver.
void Sysop_MouseShutdown(void);
// Poll the Sysop mouse and post movement/buttons into Chocolate Doom input.
void Sysop_MouseRead(void);

// Poll the C64 joystick and post the configured Doom movement/fire events.
void Sysop_JoystickRead(void);

// Scan the C64 keyboard matrix and queue Chocolate Doom key events.
void Sysop_KeyboardScan(void);
// Flush queued C64 keyboard events into Chocolate Doom's event system.
void Sysop_PostQueuedKeyEvents(void);
// Let display-tuning hotkeys consume a raw key before Doom sees it.
int Sysop_HandleDisplayTuneKey(uint8_t raw_char);

#endif
