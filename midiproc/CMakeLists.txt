if(WIN32)
    add_executable("${PROGRAM_PREFIX}midiproc" WIN32 buffer.c buffer.h main.c proto.h)
    target_include_directories("${PROGRAM_PREFIX}midiproc"
                               PRIVATE "../src/" "${CMAKE_CURRENT_BINARY_DIR}/../")
    target_link_libraries("${PROGRAM_PREFIX}midiproc" SDL2::SDL2main SDL2::mixer)
endif()
