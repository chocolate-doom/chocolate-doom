function(add_completion NAME)
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}${NAME}")
    add_docgen("${OUT}"
        GAME doom
        COMPLETION_TEMPLATE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.template"
        EXTRA
            "${PROJECT_SOURCE_DIR}/src"
            "${PROJECT_SOURCE_DIR}/src/${NAME}"
    )

    doom_install(FILES "${OUT}"
        DESTINATION "${CMAKE_INSTALL_COMPLETIONDIR}"
    )

    add_custom_target("completion_${NAME}"
        DEPENDS "${OUT}"
    )

    add_dependencies(completions "completion_${NAME}")
endfunction()

foreach(GAME doom heretic hexen strife)
    configure_file("${GAME}.template.in" "${GAME}.template" @ONLY)
endforeach()

set(BASH_SOURCES)

add_custom_target(completions ALL)

if(DOOM_BUILD_DOOM)
    add_completion(doom)
endif()

if(DOOM_BUILD_HERETIC)
    add_completion(heretic)
endif()

if(DOOM_BUILD_HEXEN)
    add_completion(hexen)
endif()

if(DOOM_BUILD_STRIFE)
    add_completion(strife)
endif()
