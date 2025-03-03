add_library(doom STATIC
            am_map.c        am_map.h
            deh_ammo.c
            deh_bexincl.c
            deh_bexpars.c   deh_bexpars.h
            deh_bexptr.c
            deh_bexstr.c
            deh_cheat.c
            deh_doom.c
            deh_frame.c
            deh_misc.c      deh_misc.h
            deh_ptr.c
            deh_sound.c
            deh_thing.c
            deh_weapon.c
                            d_englsh.h
            d_items.c       d_items.h
            d_main.c        d_main.h
            d_net.c
            d_pwad.c        d_pwad.h
                            doomdata.h
            doomdef.c       doomdef.h
            doomstat.c      doomstat.h
                            d_player.h
            dstrings.c      dstrings.h
                            d_textur.h
                            d_think.h
            f_finale.c      f_finale.h
            f_wipe.c        f_wipe.h
            g_game.c        g_game.h
            hu_lib.c        hu_lib.h
            hu_stuff.c      hu_stuff.h
            info.c          info.h
                            m_background.h
            m_crispy.c      m_crispy.h
            m_menu.c        m_menu.h
            m_random.c      m_random.h
            p_bexptr.c
            p_blockmap.c
            p_ceilng.c
            p_doors.c
            p_enemy.c
            p_extnodes.c    p_extnodes.h
            p_extsaveg.c    p_extsaveg.h
            p_floor.c
            p_inter.c       p_inter.h
            p_lights.c
                            p_local.h
            p_map.c
            p_maputl.c
            p_mobj.c        p_mobj.h
            p_plats.c
            p_pspr.c        p_pspr.h
            p_saveg.c       p_saveg.h
            p_setup.c       p_setup.h
            p_sight.c
            p_spec.c        p_spec.h
            p_switch.c
            p_telept.c
            p_tick.c        p_tick.h
            p_user.c
            r_bmaps.c       r_bmaps.h
            r_bsp.c         r_bsp.h
            r_data.c        r_data.h
                            r_defs.h
            r_draw.c        r_draw.h
                            r_local.h
            r_main.c        r_main.h
            r_plane.c       r_plane.h
            r_segs.c        r_segs.h
            r_sky.c         r_sky.h
                            r_state.h
            r_swirl.c       r_swirl.h
            r_things.c      r_things.h
            s_musinfo.c     s_musinfo.h
            s_sound.c       s_sound.h
            sounds.c        sounds.h
            statdump.c      statdump.h
            st_lib.c        st_lib.h
            st_stuff.c      st_stuff.h
            v_snow.c        v_snow.h
            wi_stuff.c      wi_stuff.h)

target_include_directories(doom PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")

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
target_link_libraries(doom ${SDL_LINK_LIBS})
set_target_properties(doom PROPERTIES COMPILE_FLAGS "${SDL_FLAGS}")
set_target_properties(doom PROPERTIES LINK_FLAGS "${SDL_FLAGS}")
