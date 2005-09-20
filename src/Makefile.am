
bindir = $(prefix)/bin

CFLAGS = @CFLAGS@ @SDL_CFLAGS@ @SDLMIXER_CFLAGS@ @SDLNET_CFLAGS@ -Wall
chocolate_doom_LDADD = @LDFLAGS@                \
                       @SDL_LIBS@               \
                       @SDLMIXER_LIBS@          \
                       @SDLNET_LIBS@ 

SOURCE_FILES=\
am_map.c    d_think.h   i_video.c   p_floor.c   p_tick.c    r_things.h	\
am_map.h    d_ticcmd.h  i_video.h   p_inter.c   p_tick.h    sounds.c	\
d_englsh.h  f_finale.c  m_argv.c    p_inter.h   p_user.c    sounds.h	\
d_event.h   f_finale.h  m_argv.h    p_lights.c  r_bsp.c     s_sound.c	\
d_french.h  f_wipe.c    m_bbox.c    p_local.h   r_bsp.h     s_sound.h	\
d_items.c   f_wipe.h    m_bbox.h    p_map.c     r_data.c    st_lib.c	\
d_items.h   g_game.c    m_cheat.c   p_maputl.c  r_data.h    st_lib.h	\
d_main.c    g_game.h    m_cheat.h   p_mobj.c    r_defs.h    st_stuff.c	\
d_main.h    hu_lib.c    m_fixed.c   p_mobj.h    r_draw.c    st_stuff.h	\
d_net.c     hu_lib.h    m_fixed.h   p_plats.c   r_draw.h    tables.c	\
d_net.h     hu_stuff.c  m_menu.c    p_pspr.c    r_local.h   tables.h	\
doomdata.h  hu_stuff.h  m_menu.h    p_pspr.h    r_main.c    v_video.c	\
doomdef.c   i_main.c    m_misc.c    p_saveg.c   r_main.h    v_video.h	\
doomdef.h   i_net.c     m_misc.h    p_saveg.h   r_plane.c   wi_stuff.c	\
doomstat.c  i_net.h     m_random.c  p_setup.c   r_plane.h   wi_stuff.h	\
doomstat.h  info.c      m_random.h  p_setup.h   r_segs.c    w_wad.c	\
doomtype.h  info.h      m_swap.c    p_sight.c   r_segs.h    w_wad.h	\
d_player.h  i_sound.c   m_swap.h    p_spec.c    r_sky.c     z_zone.c	\
dstrings.c  i_sound.h   p_ceilng.c  p_spec.h    r_sky.h     z_zone.h	\
dstrings.h  i_system.c  p_doors.c   p_switch.c  r_state.h   mmus2mid.c  \
d_textur.h  i_system.h  p_enemy.c   p_telept.c  r_things.c  mmus2mid.h  

if HAVE_WINDRES
chocolate_doom_SOURCES=$(SOURCE_FILES) chocolate-doom-res.rc
else
chocolate_doom_SOURCES=$(SOURCE_FILES)
endif

bin_PROGRAMS = chocolate-doom

.rc.o:
	windres $^ -o $@

