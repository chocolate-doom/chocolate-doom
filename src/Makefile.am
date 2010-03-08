
gamesdir = $(prefix)/games

games_PROGRAMS = chocolate-doom chocolate-server

AM_CFLAGS = -I../opl -I../textscreen -I../pcsound @SDLMIXER_CFLAGS@ @SDLNET_CFLAGS@

DEDSERV_FILES=\
d_dedicated.c                              \
i_main.c                                   \
i_timer.c            i_timer.h             \
m_argv.c             m_argv.h              \
m_misc.c             m_misc.h              \
net_common.c         net_common.h          \
net_dedicated.c      net_dedicated.h       \
net_io.c             net_io.h              \
net_packet.c         net_packet.h          \
net_sdl.c            net_sdl.h             \
net_server.c         net_server.h          \
net_structrw.c       net_structrw.h        \
z_native.c           z_zone.h

chocolate_server_SOURCES=$(DEDSERV_FILES)
chocolate_server_LDADD = ../wince/libc_wince.a @LDFLAGS@ @SDLNET_LIBS@ 

MAIN_SOURCE_FILES=\
am_map.c             am_map.h              \
d_englsh.h                                 \
d_event.h                                  \
d_items.c            d_items.h             \
d_iwad.c             d_iwad.h              \
d_main.c             d_main.h              \
d_net.c              d_net.h               \
doomdata.h                                 \
doomdef.c            doomdef.h             \
doomkeys.h                                 \
doomfeatures.h                             \
doomstat.c           doomstat.h            \
doomtype.h                                 \
d_player.h                                 \
dstrings.c           dstrings.h            \
d_textur.h                                 \
d_think.h                                  \
d_ticcmd.h                                 \
f_finale.c           f_finale.h            \
f_wipe.c             f_wipe.h              \
g_game.c             g_game.h              \
hu_lib.c             hu_lib.h              \
hu_stuff.c           hu_stuff.h            \
i_main.c                                   \
info.c               info.h                \
i_joystick.c         i_joystick.h          \
i_scale.c            i_scale.h             \
                     i_swap.h              \
i_system.c           i_system.h            \
i_timer.c            i_timer.h             \
i_video.c            i_video.h             \
m_argv.c             m_argv.h              \
m_bbox.c             m_bbox.h              \
m_cheat.c            m_cheat.h             \
m_config.c           m_config.h            \
m_fixed.c            m_fixed.h             \
m_menu.c             m_menu.h              \
m_misc.c             m_misc.h              \
md5.c                md5.h                 \
memio.c              memio.h               \
m_random.c           m_random.h            \
p_ceilng.c                                 \
p_doors.c                                  \
p_enemy.c                                  \
p_floor.c                                  \
p_inter.c            p_inter.h             \
p_lights.c                                 \
p_local.h                                  \
p_map.c                                    \
p_maputl.c                                 \
p_mobj.c             p_mobj.h              \
p_plats.c                                  \
p_pspr.c             p_pspr.h              \
p_saveg.c            p_saveg.h             \
p_setup.c            p_setup.h             \
p_sight.c                                  \
p_spec.c             p_spec.h              \
p_switch.c                                 \
p_telept.c                                 \
p_tick.c             p_tick.h              \
p_user.c                                   \
r_bsp.c              r_bsp.h               \
r_data.c             r_data.h              \
r_defs.h                                   \
r_draw.c             r_draw.h              \
r_local.h                                  \
r_main.c             r_main.h              \
r_plane.c            r_plane.h             \
r_segs.c             r_segs.h              \
r_sky.c              r_sky.h               \
r_state.h                                  \
r_things.c           r_things.h            \
sounds.c             sounds.h              \
s_sound.c            s_sound.h             \
st_lib.c             st_lib.h              \
st_stuff.c           st_stuff.h            \
tables.c             tables.h              \
v_video.c            v_video.h             \
wi_stuff.c           wi_stuff.h            \
w_checksum.c         w_checksum.h          \
w_wad.c              w_wad.h               \
w_file.c             w_file.h              \
w_file_stdc.c                              \
w_file_posix.c                             \
w_file_win32.c                             \
z_zone.c             z_zone.h   

# source files needed for FEATURE_DEHACKED

FEATURE_DEHACKED_SOURCE_FILES=             \
deh_ammo.c                                 \
deh_cheat.c                                \
deh_defs.h                                 \
deh_frame.c                                \
deh_io.c             deh_io.h              \
deh_main.c           deh_main.h            \
deh_mapping.c        deh_mapping.h         \
deh_misc.c           deh_misc.h            \
deh_ptr.c                                  \
deh_sound.c                                \
deh_text.c                                 \
deh_thing.c                                \
deh_weapon.c

# source files needed for FEATURE_MULTIPLAYER

FEATURE_MULTIPLAYER_SOURCE_FILES=          \
net_client.c         net_client.h          \
net_common.c         net_common.h          \
net_dedicated.c      net_dedicated.h       \
net_defs.h                                 \
net_gui.c            net_gui.h             \
net_io.c             net_io.h              \
net_loop.c           net_loop.h            \
net_packet.c         net_packet.h          \
net_query.c          net_query.h           \
net_sdl.c            net_sdl.h             \
net_server.c         net_server.h          \
net_structrw.c       net_structrw.h

# source files needed for FEATURE_WAD_MERGE

FEATURE_WAD_MERGE_SOURCE_FILES =           \
w_merge.c            w_merge.h

# source files needed for FEATURE_SOUND

FEATURE_SOUND_SOURCE_FILES =               \
i_pcsound.c                                \
i_sdlsound.c                               \
i_sdlmusic.c                               \
i_oplmusic.c                               \
midifile.c           midifile.h            \
mus2mid.c            mus2mid.h

SOURCE_FILES = $(MAIN_SOURCE_FILES)                \
               $(FEATURE_DEHACKED_SOURCE_FILES)    \
               $(FEATURE_MULTIPLAYER_SOURCE_FILES) \
               $(FEATURE_WAD_MERGE_SOURCE_FILES)   \
               $(FEATURE_SOUND_SOURCE_FILES)

if HAVE_WINDRES
chocolate_doom_SOURCES=$(SOURCE_FILES) resource.rc
else
chocolate_doom_SOURCES=$(SOURCE_FILES)
endif

chocolate_doom_LDADD =                               \
                       ../wince/libc_wince.a         \
                       ../textscreen/libtextscreen.a \
                       ../pcsound/libpcsound.a       \
                       ../opl/libopl.a               \
                       @LDFLAGS@                     \
                       @SDLMIXER_LIBS@               \
                       @SDLNET_LIBS@

EXTRA_DIST =                        \
        icon.c                      \
        doom-screensaver.desktop.in

.rc.o:
	$(WINDRES) $^ -o $@
%.o : %.rc
	$(WINDRES) $^ -o $@

if HAVE_PYTHON

icon.c : ../data/doom8.ico
	../data/convert-icon $^ $@

endif

midiread : midifile.c
	$(CC) -DTEST $(CFLAGS) @LDFLAGS@ $^ -o $@

