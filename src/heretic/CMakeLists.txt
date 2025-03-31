add_library(heretic STATIC
            am_data.h
            am_map.c            am_map.h
            ct_chat.c           ct_chat.h
            deh_ammo.c
            deh_frame.c
            deh_htext.c
            deh_htic.c          deh_htic.h
            deh_sound.c
            deh_thing.c
            deh_weapon.c
            d_main.c
            d_net.c
                                doomdata.h
                                doomdef.h
                                dstrings.h
            f_finale.c
            g_game.c
            info.c              info.h
            in_lude.c
            m_random.c          m_random.h
            mn_menu.c
                                p_action.h
            p_blockmap.c
            p_ceilng.c
            p_doors.c
            p_enemy.c
            p_extnodes.c        p_extnodes.h
            p_extsaveg.c        p_extsaveg.h
            p_floor.c
            p_inter.c
            p_lights.c
            p_local.h
            p_map.c
            p_maputl.c
            p_mobj.c
            p_plats.c
            p_pspr.c
            p_saveg.c
            p_setup.c
            p_sight.c
            p_spec.c            p_spec.h
            p_switch.c
            p_telept.c
            p_tick.c
            p_user.c
            r_bmaps.c           r_bmaps.h
            r_bsp.c
            r_data.c
            r_draw.c
                                r_local.h
            r_main.c
            r_plane.c
            r_segs.c
            r_swirl.c           r_swirl.h
            r_things.c
            sb_bar.c
            sounds.c            sounds.h
            s_sound.c           s_sound.h)

target_include_directories(heretic PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")

if (DEFINED EMSCRIPTEN)
    set(SDL_FLAGS "-s USE_SDL=2")
    if(ENABLE_SDL2_MIXER)
        set(SDL_FLAGS "${SDL_FLAGS} -s USE_SDL_MIXER=2")
    endif()
    if(ENABLE_SDL2_NET)
        set(SDL_FLAGS "${SDL_FLAGS} -s USE_SDL_NET=2")
    endif()
else()
    set(SDL_LINK_LIBS SDL2::SDL2)
    if(ENABLE_SDL2_MIXER)
        list(APPEND SDL_LINK_LIBS SDL2_mixer::SDL2_mixer)
    endif()
    if(ENABLE_SDL2_NET)
        list(APPEND SDL_LINK_LIBS SDL2_net::SDL2_net)
    endif()
endif()
target_link_libraries(heretic textscreen ${SDL_LINK_LIBS})
set_target_properties(heretic PROPERTIES COMPILE_FLAGS "${SDL_FLAGS}")
set_target_properties(heretic PROPERTIES LINK_FLAGS "${SDL_FLAGS}")
