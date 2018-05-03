foreach(SUBDIR doom heretic hexen strife setup)
    add_subdirectory("${SUBDIR}")
endforeach()

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
target_include_directories("${PROGRAM_PREFIX}server"
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
target_link_libraries("${PROGRAM_PREFIX}server" SDL2::SDL2main SDL2::net)
