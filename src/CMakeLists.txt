add_subdirectory(doom)
add_subdirectory(heretic)
add_subdirectory(hexen)
add_subdirectory(strife)
add_subdirectory(setup)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    "../opl" "../pcsound" "../textscreen"
    ${SDL_INCLUDE_DIR}
    ${SDL_MIXER_INCLUDE_DIR}
    ${SDL_NET_INCLUDE_DIR})

# Common source files used by absolutely everything:

set(COMMON_SOURCE_FILES
    i_main.c
    i_system.c           i_system.h
    m_argv.c             m_argv.h
    m_misc.c             m_misc.h)

# Dedicated server (chocolate-server):

set(DEDSERV_FILES
    d_dedicated.c
    d_mode.c             d_mode.h
    i_timer.c            i_timer.h
    net_common.c         net_common.h
    net_dedicated.c      net_dedicated.h
    net_io.c             net_io.h
    net_packet.c         net_packet.h
    net_sdl.c            net_sdl.h
    net_query.c          net_query.h
    net_server.c         net_server.h
    net_structrw.c       net_structrw.h
    z_native.c           z_zone.h)

add_executable("${PROGRAM_PREFIX}server" ${COMMON_SOURCE_FILES} ${DEDSERV_FILES})
target_link_libraries("${PROGRAM_PREFIX}server" ${SDL_LIBRARY} ${SDL_NET_LIBRARY})

# Source files used by the game binaries (chocolate-doom, etc.)

set(GAME_SOURCE_FILES
    d_event.c            d_event.h
                         doomkeys.h
                         doomfeatures.h
                         doomtype.h
    d_iwad.c             d_iwad.h
    d_loop.c             d_loop.h
    d_mode.c             d_mode.h
                         d_ticcmd.h
    deh_str.c            deh_str.h
    i_cdmus.c            i_cdmus.h
    i_endoom.c           i_endoom.h
    i_joystick.c         i_joystick.h
    i_scale.c            i_scale.h
                         i_swap.h
    i_sound.c            i_sound.h
    i_timer.c            i_timer.h
    i_video.c            i_video.h
    i_videohr.c          i_videohr.h
    m_bbox.c             m_bbox.h
    m_cheat.c            m_cheat.h
    m_config.c           m_config.h
    m_controls.c         m_controls.h
    m_fixed.c            m_fixed.h
    sha1.c               sha1.h
    memio.c              memio.h
    tables.c             tables.h
    v_video.c            v_video.h
                         v_patch.h
    w_checksum.c         w_checksum.h
    w_main.c             w_main.h
    w_wad.c              w_wad.h
    w_file.c             w_file.h
    w_file_stdc.c
    w_file_posix.c
    w_file_win32.c
    z_zone.c             z_zone.h)

# source files needed for FEATURE_DEHACKED

set(FEATURE_DEHACKED_SOURCE_FILES
    deh_defs.h
    deh_io.c             deh_io.h
    deh_main.c           deh_main.h
    deh_mapping.c        deh_mapping.h
    deh_text.c)

# source files needed for FEATURE_MULTIPLAYER

set(FEATURE_MULTIPLAYER_SOURCE_FILES
    aes_prng.c           aes_prng.h
    net_client.c         net_client.h
    net_common.c         net_common.h
    net_dedicated.c      net_dedicated.h
    net_defs.h
    net_gui.c            net_gui.h
    net_io.c             net_io.h
    net_loop.c           net_loop.h
    net_packet.c         net_packet.h
    net_query.c          net_query.h
    net_sdl.c            net_sdl.h
    net_server.c         net_server.h
    net_structrw.c       net_structrw.h)

# source files needed for FEATURE_WAD_MERGE

set(FEATURE_WAD_MERGE_SOURCE_FILES
    w_merge.c            w_merge.h)

# source files needed for FEATURE_SOUND

set(FEATURE_SOUND_SOURCE_FILES
    gusconf.c            gusconf.h
    i_pcsound.c
    i_sdlsound.c
    i_sdlmusic.c
    i_oplmusic.c
    midifile.c           midifile.h
    mus2mid.c            mus2mid.h)

# Some games support dehacked patches, some don't:

set(SOURCE_FILES
    ${COMMON_SOURCE_FILES}
    ${GAME_SOURCE_FILES}
    ${FEATURE_WAD_MERGE_SOURCE_FILES}
    ${FEATURE_SOUND_SOURCE_FILES}
    ${FEATURE_MULTIPLAYER_SOURCE_FILES})

set(SOURCE_FILES_WITH_DEH
    ${SOURCE_FILES}
    ${FEATURE_DEHACKED_SOURCE_FILES})

set(EXTRA_LIBS
    opl pcsound textscreen
    ${SDL_LIBRARY}
    ${SDL_MIXER_LIBRARY}
    ${SDL_NET_LIBRARY})

add_executable("${PROGRAM_PREFIX}doom" ${SOURCE_FILES_WITH_DEH} resource.rc)
target_link_libraries("${PROGRAM_PREFIX}doom" doom ${EXTRA_LIBS} m)

add_executable("${PROGRAM_PREFIX}heretic" ${SOURCE_FILES_WITH_DEH} resource.rc)
target_link_libraries("${PROGRAM_PREFIX}heretic" heretic ${EXTRA_LIBS} m)

add_executable("${PROGRAM_PREFIX}hexen" ${SOURCE_FILES} resource.rc)
target_link_libraries("${PROGRAM_PREFIX}hexen" hexen ${EXTRA_LIBS} m)

add_executable("${PROGRAM_PREFIX}strife" ${SOURCE_FILES_WITH_DEH} resource.rc)
target_link_libraries("${PROGRAM_PREFIX}strife" strife ${EXTRA_LIBS} m)

# Source files needed for chocolate-setup:

set(SETUP_FILES
    deh_str.c            deh_str.h
    d_mode.c             d_mode.h
    d_iwad.c             d_iwad.h
    i_timer.c            i_timer.h
    m_config.c           m_config.h
    m_controls.c         m_controls.h
    net_io.c             net_io.h
    net_packet.c         net_packet.h
    net_sdl.c            net_sdl.h
    net_query.c          net_query.h
    net_structrw.c       net_structrw.h
    z_native.c           z_zone.h)

add_executable("${PROGRAM_PREFIX}setup"
               ${SETUP_FILES} ${COMMON_SOURCE_FILES} setup-res.rc)
target_link_libraries("${PROGRAM_PREFIX}setup"
                      setup textscreen ${SDL_LIBRARY} ${SDL_NET_LIBRARY} m)
