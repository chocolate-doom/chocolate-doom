if(WIN32)
    add_executable(midiproc WIN32
        buffer.c        buffer.h
        main.c
        proto.h
    )

    target_include_directories(midiproc PRIVATE
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/src"
    )

    target_link_libraries(midiproc PUBLIC
        SDL2::mixer
    )

    set_target_properties(midiproc PROPERTIES
        OUTPUT_NAME "${PROGRAM_PREFIX}midiproc"
    )
endif()
