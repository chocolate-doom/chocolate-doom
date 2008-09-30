
SUBDIRS=doom heretic hexen

gamesdir = $(prefix)/games

EXTRA_PROGRAMS = chocolate-heretic chocolate-hexen
games_PROGRAMS = chocolate-doom chocolate-server

AM_CFLAGS = -Idoom -I../textscreen -I../pcsound @SDL_CFLAGS@ @SDLMIXER_CFLAGS@ @SDLNET_CFLAGS@ -Werror

DEDSERV_FILES=\
d_dedicated.c                              \
d_mode.c             d_mode.h              \
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
chocolate_server_LDADD = @LDFLAGS@ @SDL_LIBS@ @SDLNET_LIBS@ 

MAIN_SOURCE_FILES=\
d_event.c            d_event.h             \
                     doomkeys.h            \
                     doomfeatures.h        \
                     doomtype.h            \
d_iwad.c             d_iwad.h              \
d_mode.c             d_mode.h              \
                     d_ticcmd.h            \
i_cdmus.c            i_cdmus.h             \
i_main.c                                   \
i_joystick.c         i_joystick.h          \
i_scale.c            i_scale.h             \
                     i_swap.h              \
i_sound.c            i_sound.h             \
i_system.c           i_system.h            \
i_timer.c            i_timer.h             \
i_video.c            i_video.h             \
m_argv.c             m_argv.h              \
m_bbox.c             m_bbox.h              \
m_cheat.c            m_cheat.h             \
m_config.c           m_config.h            \
m_controls.c         m_controls.h          \
m_fixed.c            m_fixed.h             \
m_misc.c             m_misc.h              \
md5.c                md5.h                 \
memio.c              memio.h               \
tables.c             tables.h              \
v_video.c            v_video.h             \
                     v_patch.h             \
w_checksum.c         w_checksum.h          \
w_wad.c              w_wad.h               \
w_file.c             w_file.h              \
w_file_stdc.c                              \
w_file_posix.c                             \
w_file_win32.c                             \
z_zone.c             z_zone.h   

# source files needed for FEATURE_DEHACKED

FEATURE_DEHACKED_SOURCE_FILES=             \
deh_str.c            deh_str.h

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
mus2mid.c            mus2mid.h

SOURCE_FILES = $(MAIN_SOURCE_FILES)                \
               $(FEATURE_DEHACKED_SOURCE_FILES)    \
               $(FEATURE_WAD_MERGE_SOURCE_FILES)   \
               $(FEATURE_SOUND_SOURCE_FILES)

EXTRA_LIBS =                                       \
	       ../textscreen/libtextscreen.a       \
	       ../pcsound/libpcsound.a             \
	       @LDFLAGS@                           \
	       @SDL_LIBS@                          \
	       @SDLMIXER_LIBS@                     \
	       @SDLNET_LIBS@ 

if HAVE_WINDRES
chocolate_doom_SOURCES=$(SOURCE_FILES) resource.rc
else
chocolate_doom_SOURCES=$(SOURCE_FILES)
endif

chocolate_doom_LDADD = doom/libdoom.a $(EXTRA_LIBS)

if HAVE_WINDRES
chocolate_heretic_SOURCES=$(SOURCE_FILES) resource.rc
else
chocolate_heretic_SOURCES=$(SOURCE_FILES)
endif

chocolate_heretic_LDADD = heretic/libheretic.a $(EXTRA_LIBS)

if HAVE_WINDRES
chocolate_hexen_SOURCES=$(SOURCE_FILES) resource.rc
else
chocolate_hexen_SOURCES=$(SOURCE_FILES)
endif

chocolate_hexen_LDADD = hexen/libhexen.a $(EXTRA_LIBS)

EXTRA_DIST =                        \
        icon.c                      \
        doom-screensaver.desktop.in

.rc.o:
	$(WINDRES) $^ -o $@
%.o : %.rc
	$(WINDRES) $^ -o $@

if HAVE_PYTHON

icon.c : ../data/doom.ico
	../data/convert-icon $^ $@

endif

