set(DOOM_SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Doom srcdir")

add_library(common STATIC EXCLUDE_FROM_ALL
    i_main.c
    i_system.c           i_system.h
    m_argv.c             m_argv.h
    m_misc.c             m_misc.h
)

target_include_directories(common PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/utils"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(common PUBLIC
    SDL2::SDL2
)

# Dedicated server (chocolate-server):

option(DOOM_BUILD_SERVER "Build server" ON)
if(DOOM_BUILD_SERVER)
    add_executable(server WIN32
    d_dedicated.c
    d_iwad.c            d_iwad.h
    d_mode.c            d_mode.h
    deh_str.c           deh_str.h
    i_timer.c           i_timer.h
    m_config.c          m_config.h
    net_common.c        net_common.h
    net_dedicated.c     net_dedicated.h
    net_io.c            net_io.h
    net_packet.c        net_packet.h
    net_sdl.c           net_sdl.h
    net_query.c         net_query.h
    net_server.c        net_server.h
    net_structrw.c      net_structrw.h
    z_native.c          z_zone.h
    )

    target_link_libraries(server
        common
        SDL2::net
    )

    set_target_properties(server
        PROPERTIES
            OUTPUT_NAME "${PROGRAM_PREFIX}server"
    )

    doom_install(TARGETS server)
endif()

# Source files used by the game binaries (chocolate-doom, etc.)
add_library(gamecommon STATIC EXCLUDE_FROM_ALL
    aes_prng.c          aes_prng.h
    d_event.c           d_event.h
                        doomkeys.h
                        doomtype.h
    d_iwad.c            d_iwad.h
    d_loop.c            d_loop.h
    d_mode.c            d_mode.h
                        d_ticcmd.h
    deh_str.c           deh_str.h
    gusconf.c           gusconf.h
    i_cdmus.c           i_cdmus.h
    i_endoom.c          i_endoom.h
    i_glob.c            i_glob.h
    i_input.c           i_input.h
    i_joystick.c        i_joystick.h
                        i_swap.h
    i_midipipe.c        i_midipipe.h
    i_musicpack.c
    i_oplmusic.c
    i_pcsound.c
    i_sdlmusic.c
    i_sdlsound.c
    icon.c
    i_sound.c           i_sound.h
    i_timer.c           i_timer.h
    i_video.c           i_video.h
    i_videohr.c         i_videohr.h
    midifile.c          midifile.h
    mus2mid.c           mus2mid.h
    m_bbox.c            m_bbox.h
    m_cheat.c           m_cheat.h
    m_config.c          m_config.h
    m_controls.c        m_controls.h
    m_fixed.c           m_fixed.h
    net_client.c        net_client.h
    net_common.c        net_common.h
    net_dedicated.c     net_dedicated.h
    net_defs.h
    net_gui.c           net_gui.h
    net_io.c            net_io.h
    net_loop.c          net_loop.h
    net_packet.c        net_packet.h
    net_petname.c       net_petname.h
    net_query.c         net_query.h
    net_sdl.c           net_sdl.h
    net_server.c        net_server.h
    net_structrw.c      net_structrw.h
    sha1.c              sha1.h
    memio.c             memio.h
    tables.c            tables.h
    v_diskicon.c        v_diskicon.h
    v_video.c           v_video.h
                        v_patch.h
    w_checksum.c        w_checksum.h
    w_main.c            w_main.h
    w_wad.c             w_wad.h
    w_file.c            w_file.h
    w_file_stdc.c
    w_file_posix.c
    w_file_win32.c
    w_merge.c           w_merge.h
    z_zone.c            z_zone.h
)
convert_icon("${PROJECT_DATADIR}/doom.png" "icon.c" COPY "${CMAKE_CURRENT_SOURCE_DIR}/icon.c")

function(generate_install_desktop NAME_CAP)
    cmake_parse_arguments(GID "" "SRCDIR;INSTALL_SUBDIR" "" ${ARGV})

    set(SRCDIR "${DOOM_SRCDIR}")
    if(GID_SRCDIR)
        set(SRCDIR "${GID_SRCDIR}")
    endif()
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_RDNS}.${NAME_CAP}.desktop")
    configure_file("${SRCDIR}/${NAME_CAP}.desktop.in" "${OUT}" @ONLY)
    doom_install(FILES "${OUT}"
        DESTINATION "${CMAKE_INSTALL_DESKTOPDIR}/${GID_INSTALL_SUBDIR}"
    )
endfunction()

function(generate_install_metainfo NAME_CAP)
    cmake_parse_arguments(GIM "" "SRCDIR" "" ${ARGV})

    set(SRCDIR "${DOOM_SRCDIR}")
    if(GIM_SRCDIR)
        set(SRCDIR "${GIM_SRCDIR}")
    endif()
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_RDNS}.${NAME_CAP}.metainfo.xml")
    configure_file("${SRCDIR}/${NAME_CAP}.metainfo.xml.in" "${OUT}" @ONLY)
    doom_install(FILES "${OUT}"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
    )
endfunction()

if(MSVC)
    target_sources(gamecommon PRIVATE
         ../win32/win_opendir.c ../win32/win_opendir.h
     )
endif()

target_include_directories(gamecommon INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(gamecommon PUBLIC
    common
    SDL2::mixer
    SDL2::net
    textscreen
    pcsound
    opl
)
if(SAMPLERATE_FOUND)
    target_link_libraries(gamecommon PUBLIC samplerate::samplerate)
endif()
if(PNG_FOUND)
    target_link_libraries(gamecommon PUBLIC PNG::PNG)
endif()

# Some games support dehacked patches, some don't:
add_library(gamecommon_dehacked STATIC
    deh_defs.h
    deh_io.c            deh_io.h
    deh_main.c          deh_main.h
    deh_mapping.c       deh_mapping.h
    deh_text.c
)
target_link_libraries(gamecommon_dehacked PUBLIC
    gamecommon
)

# Source files needed for chocolate-setup:

option(DOOM_BUILD_SETUP "Build setup" ON)
if(DOOM_BUILD_SETUP)
    add_library(setupcommon
        deh_str.c             deh_str.h
        d_mode.c              d_mode.h
        d_iwad.c              d_iwad.h
        i_timer.c             i_timer.h
        m_config.c            m_config.h
        m_controls.c          m_controls.h
        net_io.c              net_io.h
        net_packet.c          net_packet.h
        net_petname.c         net_petname.h
        net_sdl.c             net_sdl.h
        net_query.c           net_query.h
        net_structrw.c        net_structrw.h
        z_native.c            z_zone.h
    )

    target_link_libraries(setupcommon PUBLIC
        common
        SDL2::net
    )

    add_subdirectory(setup)
endif()

option(DOOM_BUILD_DOOM "Build doom" ON)
if(DOOM_BUILD_DOOM)
    add_subdirectory(doom)
endif()

option(DOOM_BUILD_HERETIC "Build heretic" ON)
if(DOOM_BUILD_HERETIC)
    add_subdirectory(heretic)
endif()

option(DOOM_BUILD_HEXEN "Build hexen" ON)
if(DOOM_BUILD_HEXEN)
    add_subdirectory(hexen)
endif()

option(DOOM_BUILD_STRIFE "Build strife" ON)
if(DOOM_BUILD_STRIFE)
    add_subdirectory(strife)
endif()

add_executable(midiread
    deh_str.c             deh_str.h
    d_iwad.c              d_iwad.h
    i_system.c            i_system.h
    midifile.c            midifile.h
    m_argv.c              m_argv.h
    m_config.c            m_config.h
    m_misc.c              m_misc.h
    z_native.c
)
target_compile_definitions(midiread PRIVATE
    TEST
)
target_include_directories(midiread PRIVATE
    "${PROJECT_BINARY_DIR}"
)
target_link_libraries(midiread PRIVATE
    SDL2::SDL2
)

add_executable(mus2mid
    mus2mid.c             mus2mid.h
    memio.c               memio.h
    z_native.c
    i_system.c            i_system.h
    m_argv.c              m_argv.h
    m_misc.c              m_misc.h
    d_iwad.c              d_iwad.h
    deh_str.c             deh_str.h
    m_config.c            m_config.h
)
target_compile_definitions(mus2mid PRIVATE
    STANDALONE
)
target_include_directories(mus2mid PRIVATE
    "${PROJECT_BINARY_DIR}"
)
target_link_libraries(mus2mid PRIVATE
    SDL2::SDL2
)
