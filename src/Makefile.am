
SUBDIRS = doom heretic hexen strife setup

execgamesdir = ${bindir}

execgames_PROGRAMS = @PROGRAM_PREFIX@doom     \
                     @PROGRAM_PREFIX@heretic  \
                     @PROGRAM_PREFIX@hexen    \
                     @PROGRAM_PREFIX@strife   \
                     @PROGRAM_PREFIX@server

noinst_PROGRAMS = @PROGRAM_PREFIX@setup

SETUP_BINARIES = @PROGRAM_PREFIX@doom-setup$(EXEEXT)    \
                 @PROGRAM_PREFIX@heretic-setup$(EXEEXT) \
                 @PROGRAM_PREFIX@hexen-setup$(EXEEXT)   \
                 @PROGRAM_PREFIX@strife-setup$(EXEEXT)

execgames_SCRIPTS = $(SETUP_BINARIES)

AM_CFLAGS = -I$(top_srcdir)/textscreen            \
            -I$(top_srcdir)/opl                   \
            -I$(top_srcdir)/pcsound               \
            -I$(top_srcdir)/win32                 \
            @SDLMIXER_CFLAGS@ @SDLNET_CFLAGS@

# Common source files used by absolutely everything:

COMMON_SOURCE_FILES=\
crispy.c             crispy.h              \
i_main.c                                   \
i_system.c           i_system.h            \
m_argv.c             m_argv.h              \
m_misc.c             m_misc.h

# Dedicated server (chocolate-server):

DEDSERV_FILES=\
d_dedicated.c                              \
d_iwad.c             d_iwad.h              \
d_mode.c             d_mode.h              \
deh_str.c            deh_str.h             \
i_timer.c            i_timer.h             \
m_config.c           m_config.h            \
net_common.c         net_common.h          \
net_dedicated.c      net_dedicated.h       \
net_io.c             net_io.h              \
net_packet.c         net_packet.h          \
net_sdl.c            net_sdl.h             \
net_query.c          net_query.h           \
net_server.c         net_server.h          \
net_structrw.c       net_structrw.h        \
z_native.c           z_zone.h

@PROGRAM_PREFIX@server_SOURCES=$(COMMON_SOURCE_FILES) $(DEDSERV_FILES)
@PROGRAM_PREFIX@server_LDADD = @LDFLAGS@ @SDLNET_LIBS@

# Source files used by the game binaries (chocolate-doom, etc.)

GAME_BASE_FILES=\
a11y.c               a11y.h                \
aes_prng.c           aes_prng.h            \
d_event.c            d_event.h             \
                     doomkeys.h            \
                     doomtype.h            \
d_iwad.c             d_iwad.h              \
d_loop.c             d_loop.h              \
d_mode.c             d_mode.h              \
                     d_ticcmd.h            \
deh_str.c            deh_str.h             \
gusconf.c            gusconf.h             \
i_cdmus.c            i_cdmus.h             \
i_endoom.c           i_endoom.h            \
i_flmusic.c                                \
i_glob.c             i_glob.h              \
i_input.c            i_input.h             \
i_joystick.c         i_joystick.h          \
                     i_swap.h              \
i_musicpack.c                              \
i_oplmusic.c                               \
i_pcsound.c                                \
i_sdlmusic.c                               \
i_sdlsound.c                               \
i_sound.c            i_sound.h             \
i_timer.c            i_timer.h             \
i_truecolor.c        i_truecolor.h         \
i_video.c            i_video.h             \
i_videohr.c          i_videohr.h           \
i_winmusic.c                               \
midifallback.c       midifallback.h        \
midifile.c           midifile.h            \
mus2mid.c            mus2mid.h             \
m_bbox.c             m_bbox.h              \
m_cheat.c            m_cheat.h             \
m_config.c           m_config.h            \
m_controls.c         m_controls.h          \
m_fixed.c            m_fixed.h             \
net_client.c         net_client.h          \
net_common.c         net_common.h          \
net_dedicated.c      net_dedicated.h       \
net_defs.h                                 \
net_gui.c            net_gui.h             \
net_io.c             net_io.h              \
net_loop.c           net_loop.h            \
net_packet.c         net_packet.h          \
net_petname.c        net_petname.h         \
net_query.c          net_query.h           \
net_sdl.c            net_sdl.h             \
net_server.c         net_server.h          \
net_structrw.c       net_structrw.h        \
sha1.c               sha1.h                \
memio.c              memio.h               \
tables.c             tables.h              \
v_diskicon.c         v_diskicon.h          \
v_video.c            v_video.h             \
                     v_patch.h             \
v_trans.c            v_trans.h             \
w_checksum.c         w_checksum.h          \
w_main.c             w_main.h              \
w_wad.c              w_wad.h               \
w_file.c             w_file.h              \
w_file_stdc.c                              \
w_file_posix.c                             \
w_file_win32.c                             \
w_merge.c            w_merge.h             \
$(top_builddir)/win32/win_opendir.c        \
$(top_builddir)/win32/win_opendir.h


MEMORY_NATIVE_SOURCE_FILES=\
z_native.c           z_zone.h

MEMORY_ZONE_SOURCE_FILES=\
z_zone.c             z_zone.h

if HAVE_ZPOOL
GAME_SOURCE_FILES=$(GAME_BASE_FILES) $(MEMORY_ZONE_SOURCE_FILES)
else
GAME_SOURCE_FILES=$(GAME_BASE_FILES) $(MEMORY_NATIVE_SOURCE_FILES)
endif

DEHACKED_SOURCE_FILES =                    \
deh_defs.h                                 \
deh_io.c             deh_io.h              \
deh_main.c           deh_main.h            \
deh_mapping.c        deh_mapping.h         \
deh_text.c

# Some games support dehacked patches, some don't:

SOURCE_FILES = $(COMMON_SOURCE_FILES) $(GAME_SOURCE_FILES)
SOURCE_FILES_WITH_DEH = $(SOURCE_FILES) $(DEHACKED_SOURCE_FILES)

EXTRA_LIBS =                                               \
               $(top_builddir)/textscreen/libtextscreen.a  \
               $(top_builddir)/pcsound/libpcsound.a        \
               $(top_builddir)/opl/libopl.a                \
               @LDFLAGS@                                   \
               @EM_LDFLAGS@                                \
               @SDL_LIBS@                                  \
               @SDLMIXER_LIBS@                             \
               @SDLNET_LIBS@

if HAVE_WINDRES
@PROGRAM_PREFIX@doom_SOURCES=$(SOURCE_FILES_WITH_DEH) doom-res.rc
else
@PROGRAM_PREFIX@doom_SOURCES=$(SOURCE_FILES_WITH_DEH)
endif

@PROGRAM_PREFIX@doom_LDADD = doom/libdoom.a $(EXTRA_LIBS)

if HAVE_WINDRES
@PROGRAM_PREFIX@heretic_SOURCES=$(SOURCE_FILES_WITH_DEH) heretic-res.rc
else
@PROGRAM_PREFIX@heretic_SOURCES=$(SOURCE_FILES_WITH_DEH)
endif

@PROGRAM_PREFIX@heretic_LDADD = heretic/libheretic.a $(EXTRA_LIBS)

if HAVE_WINDRES
@PROGRAM_PREFIX@hexen_SOURCES=$(SOURCE_FILES) hexen-res.rc
else
@PROGRAM_PREFIX@hexen_SOURCES=$(SOURCE_FILES)
endif

@PROGRAM_PREFIX@hexen_LDADD = hexen/libhexen.a $(EXTRA_LIBS)

if HAVE_WINDRES
@PROGRAM_PREFIX@strife_SOURCES=$(SOURCE_FILES_WITH_DEH) strife-res.rc
else
@PROGRAM_PREFIX@strife_SOURCES=$(SOURCE_FILES_WITH_DEH)
endif

@PROGRAM_PREFIX@strife_LDADD = strife/libstrife.a $(EXTRA_LIBS)

$(SETUP_BINARIES): @PROGRAM_PREFIX@setup$(EXEEXT)
	cp @PROGRAM_PREFIX@setup$(EXEEXT) $@

# Make "make" aware of convenience libraries in subdirectories

doom/libdoom.a:
	$(MAKE) -C doom

heretic/libheretic.a:
	$(MAKE) -C heretic

hexen/libhexen.a:
	$(MAKE) -C hexen

strife/libstrife.a:
	$(MAKE) -C strife

# Source files needed for chocolate-setup:

SETUP_FILES=                               \
deh_str.c            deh_str.h             \
d_mode.c             d_mode.h              \
d_iwad.c             d_iwad.h              \
i_timer.c            i_timer.h             \
m_config.c           m_config.h            \
m_controls.c         m_controls.h          \
net_io.c             net_io.h              \
net_packet.c         net_packet.h          \
net_petname.c        net_petname.h         \
net_sdl.c            net_sdl.h             \
net_query.c          net_query.h           \
net_structrw.c       net_structrw.h        \
z_native.c           z_zone.h

if HAVE_WINDRES
@PROGRAM_PREFIX@setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES) setup-res.rc
else
@PROGRAM_PREFIX@setup_SOURCES=$(SETUP_FILES) $(COMMON_SOURCE_FILES)
endif
@PROGRAM_PREFIX@setup_LDADD = setup/libsetup.a                             \
                        $(top_builddir)/textscreen/libtextscreen.a   \
                        @LDFLAGS@ @SDL_LIBS@ @SDLMIXER_LIBS@ @SDLNET_LIBS@

EXTRA_DIST =                        \
        CMakeLists.txt              \
        Doom_Screensaver.desktop.in \
        manifest.xml

metainfodir = $(prefix)/share/metainfo
metainfo_DATA =                             \
        @PACKAGE_RDNS@.Doom.metainfo.xml    \
        @PACKAGE_RDNS@.Heretic.metainfo.xml \
        @PACKAGE_RDNS@.Hexen.metainfo.xml   \
        @PACKAGE_RDNS@.Strife.metainfo.xml

@PACKAGE_RDNS@.Doom.metainfo.xml : Doom.metainfo.xml
	cp Doom.metainfo.xml $@

@PACKAGE_RDNS@.Heretic.metainfo.xml : Heretic.metainfo.xml
	cp Heretic.metainfo.xml $@

@PACKAGE_RDNS@.Hexen.metainfo.xml : Hexen.metainfo.xml
	cp Hexen.metainfo.xml $@

@PACKAGE_RDNS@.Strife.metainfo.xml : Strife.metainfo.xml
	cp Strife.metainfo.xml $@

appdir = $(prefix)/share/applications
app_DATA =                                 \
           @PACKAGE_RDNS@.Doom.desktop    \
           @PACKAGE_RDNS@.Heretic.desktop \
           @PACKAGE_RDNS@.Hexen.desktop   \
           @PACKAGE_RDNS@.Strife.desktop

@PACKAGE_RDNS@.Doom.desktop : Doom.desktop
	cp Doom.desktop $@

@PACKAGE_RDNS@.Heretic.desktop : Heretic.desktop
	cp Heretic.desktop $@

@PACKAGE_RDNS@.Hexen.desktop : Hexen.desktop
	cp Hexen.desktop $@

@PACKAGE_RDNS@.Strife.desktop : Strife.desktop
	cp Strife.desktop $@

screensaverdir = $(prefix)/share/applications/screensavers
screensaver_DATA = @PACKAGE_RDNS@.Doom_Screensaver.desktop

@PACKAGE_RDNS@.Doom_Screensaver.desktop: Doom_Screensaver.desktop
	cp Doom_Screensaver.desktop $@

CLEANFILES = $(execgames_SCRIPTS) $(metainfo_DATA) $(app_DATA) $(screensaver_DATA)

.rc.o:
	$(WINDRES) $< -o $@
%.o : %.rc
	$(WINDRES) $< -o $@

midiread : midifile.c
	$(CC) -DTEST $(CFLAGS) @LDFLAGS@ midifile.c -o $@

MUS2MID_SRC_FILES = mus2mid.c memio.c z_native.c i_system.c m_argv.c m_misc.c
mus2mid : $(MUS2MID_SRC_FILES)
	$(CC) -DSTANDALONE -I$(top_builddir) $(CFLAGS) @LDFLAGS@ \
              $(MUS2MID_SRC_FILES) -o $@

