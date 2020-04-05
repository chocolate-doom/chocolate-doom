if(NOT PYTHON_EXECUTABLE)
    message("Python not available. Building mans disabled.")
    return()
endif()

set(PROJECT_MANDIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "mandir")

function(add_docgen OUT)
    cmake_parse_arguments(MAN "WIKI_OUTPUT" "GAME;MANPAGE_TEMPLATE;MARKDOWN_TEMPLATE;PLAINTEXT_TEMPLATE;COMPLETION_TEMPLATE;CONFIG_FILE" "EXTRA" ${ARGN})
    set(DOCGEN_ARGS -n "${PROGRAM_SPREFIX}" -s "${PACKAGE_NAME}" -z "${PACKAGE_SHORTNAME}")
    set(DEPENDS)
    if(MAN_WIKI_OUTPUT)
        list(APPEND DOCGEN_ARGS -w)
    endif()
    if(MAN_GAME)
        list(APPEND DOCGEN_ARGS -g "${MAN_GAME}")
    endif()
    if(MAN_MANPAGE_TEMPLATE)
        list(APPEND DOCGEN_ARGS -m "${MAN_MANPAGE_TEMPLATE}")
        list(APPEND DEPENDS "${MAN_MANPAGE_TEMPLATE}")
    endif()
    if(MAN_MARKDOWN_TEMPLATE)
        list(APPEND DOCGEN_ARGS -M "${MAN_MARKDOWN_TEMPLATE}")
        list(APPEND DEPENDS "${MAN_MARKDOWN_TEMPLATE}")
    endif()
    if(MAN_PLAINTEXT_TEMPLATE)
        list(APPEND DOCGEN_ARGS -p "${MAN_PLAINTEXT_TEMPLATE}")
        list(APPEND DEPENDS "${MAN_PLAINTEXT_TEMPLATE}")
    endif()
    if(MAN_COMPLETION_TEMPLATE)
        list(APPEND DOCGEN_ARGS -b "${MAN_COMPLETION_TEMPLATE}")
        list(APPEND DEPENDS "${MAN_COMPLETION_TEMPLATE}")
    endif()
    if(MAN_CONFIG_FILE)
        list(APPEND DOCGEN_ARGS -c "${MAN_CONFIG_FILE}")
    endif()
    list(APPEND DOCGEN_ARGS ${MAN_EXTRA})
    if(NOT IS_ABSOLUTE "${OUT}")
        set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${OUT}")
    endif()
    add_custom_command(
        OUTPUT "${OUT}"
        COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_MANDIR}/docgen" ${DOCGEN_ARGS} > "${OUT}"
        DEPENDS "${DEPENDS}" "${PROJECT_MANDIR}/docgen"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )
endfunction()

function(simplecpp OUT IN)
    cmake_parse_arguments(SPP "" "" "DEFINES" ${ARGN})
    set(SIMPLEPP_ARGS)
    if(WIN32)
        list(APPEND SPP_DEFINES _WIN32)
    endif()
    foreach(SPP_ARG ${SPP_DEFINES})
        list(APPEND SIMPLEPP_ARGS "-D${SPP_ARG}")
    endforeach()
    if(NOT IS_ABSOLUTE "${OUT}")
        set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${OUT}")
    endif()
    add_custom_command(
        OUTPUT "${OUT}"
        COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_MANDIR}/simplecpp" ${SIMPLEPP_ARGS} > "${OUT}" < "${IN}"
        DEPENDS "${IN}" "${PROJECT_MANDIR}/simplecpp"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )
endfunction()

function(generate_man)
    cmake_parse_arguments(AD "" "GAME_NAME;DEFAULT_CFG_NAME" "" ${ARGV})

    if(NOT AD_DEFAULT_CFG_NAME)
        set(AD_DEFAULT_CFG_NAME "${AD_GAME_NAME}")
    endif()

    set(GENERATED_DOCS)
    set(GENERATED_MAN5)
    set(GENERATED_MAN6)

    add_docgen("${PROGRAM_PREFIX}${AD_GAME_NAME}.6"
        GAME "${AD_GAME_NAME}"
        MANPAGE_TEMPLATE "${AD_GAME_NAME}.template"
        EXTRA
            "${PROJECT_SOURCE_DIR}/src"
            "${PROJECT_SOURCE_DIR}/src/${AD_GAME_NAME}"
    )
    list(APPEND GENERATED_MAN6 "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}${AD_GAME_NAME}.6")

    add_docgen("${AD_DEFAULT_CFG_NAME}.cfg.5"
        GAME "${AD_GAME_NAME}"
        MANPAGE_TEMPLATE default.cfg.template
        CONFIG_FILE default
        EXTRA "${PROJECT_SOURCE_DIR}/src/m_config.c"
    )
    list(APPEND GENERATED_MAN5 "${CMAKE_CURRENT_BINARY_DIR}/${AD_DEFAULT_CFG_NAME}.cfg.5")

    add_docgen("${PROGRAM_PREFIX}${AD_GAME_NAME}.cfg.5"
        GAME "${AD_GAME_NAME}"
        MANPAGE_TEMPLATE extra.cfg.template
        CONFIG_FILE extended
        EXTRA "${PROJECT_SOURCE_DIR}/src/m_config.c"
    )
    list(APPEND GENERATED_MAN5 "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}${AD_GAME_NAME}.cfg.5")

    add_docgen("CMDLINE.${AD_GAME_NAME}"
        PLAINTEXT_TEMPLATE CMDLINE.template
        EXTRA "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/${AD_GAME_NAME}"
    )
    list(APPEND GENERATED_DOCS "${CMAKE_CURRENT_BINARY_DIR}/CMDLINE.${AD_GAME_NAME}")

    add_docgen("${PROGRAM_PREFIX}${AD_GAME_NAME}-setup.6"
        GAME "${AD_GAME_NAME}"
        MANPAGE_TEMPLATE setup.template
        EXTRA "${PROJECT_SOURCE_DIR}/src"
    )
    list(APPEND GENERATED_MAN6 "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}${AD_GAME_NAME}-setup.6")

    add_docgen("CMDLINE.${AD_GAME_NAME}.md"
        MARKDOWN_TEMPLATE CMDLINE.template.md
        EXTRA "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/${AD_GAME_NAME}"
    )
    #list(APPEND GENERATED_DOCS "${CMAKE_CURRENT_BINARY_DIR}/CMDLINE.${AD_GAME_NAME}.md")

    string(SUBSTRING "${AD_GAME_NAME}" 0 1  GAME_NAME_FIRST_LETTER)
    string(TOUPPER "${GAME_NAME_FIRST_LETTER}" GAME_NAME_FIRST_LETTER)
    string(SUBSTRING "${AD_GAME_NAME}" 1 -1 GAME_NAME_REST)
    set(GAME_NAME_CAPITALIZED "${GAME_NAME_FIRST_LETTER}${GAME_NAME_REST}")

    string(TOUPPER "${AD_GAME_NAME}" NAME_UPPER)
    simplecpp("INSTALL.${AD_GAME_NAME}" "INSTALL.template"
        DEFINES
            "${NAME_UPPER}"
            "LONG_GAME_NAME=${PACKAGE_SHORTNAME} ${GAME_NAME_CAPITALIZED}"
            "LONG_EXE_NAME=$<TARGET_PROPERTY:${AD_GAME_NAME},OUTPUT_NAME>"
            PRECOMPILED
    )
    list(APPEND GENERATED_DOCS "${CMAKE_CURRENT_BINARY_DIR}/INSTALL.${AD_GAME_NAME}")

    doom_install(FILES ${GENERATED_MAN5}
        DESTINATION "${CMAKE_INSTALL_MANDIR}/man5"
    )

    doom_install(FILES ${GENERATED_MAN6}
        DESTINATION "${CMAKE_INSTALL_MANDIR}/man6"
    )

    doom_install(
        FILES
            ${GENERATED_DOCS}
            "${PROJECT_SOURCE_DIR}/ChangeLog"
            "${PROJECT_SOURCE_DIR}/COPYING.md"
            "${PROJECT_SOURCE_DIR}/NEWS.md"
            "${PROJECT_SOURCE_DIR}/PHILOSOPHY.md"
            "${PROJECT_SOURCE_DIR}/README.md"
            "${PROJECT_SOURCE_DIR}/README.Music.md"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}/${PROGRAM_PREFIX}${AD_GAME_NAME}"
    )

    add_custom_target("mans_${AD_GAME_NAME}"
        DEPENDS ${GENERATED_DOCS} ${GENERATED_MAN5} ${GENERATED_MAN6}
    )

    add_dependencies(mans "mans_${AD_GAME_NAME}")
endfunction()

set(DOOM_INSTALL_DEFINES)

add_custom_target(mans ALL)

if(DOOM_BUILD_SERVER)
    add_docgen("${PROGRAM_PREFIX}server.6"
        GAME server
        MANPAGE_TEMPLATE server.template
        EXTRA
            "${PROJECT_SOURCE_DIR}/src"
    )
    doom_install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}server.6"
        DESTINATION "${CMAKE_INSTALL_MANDIR}/man6"
    )
    add_custom_target(mans_server
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${PROGRAM_PREFIX}server.6"
    )
    add_dependencies(mans mans_server)
endif()

if(DOOM_BUILD_DOOM)
    generate_man(GAME_NAME doom DEFAULT_CFG_NAME default)
    list(APPEND DOOM_INSTALL_DEFINES doom)
    doom_install(FILES "${PROJECT_SOURCE_DIR}/NOT-BUGS.md"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}/${PROGRAM_PREFIX}doom"
    )
endif()

if(DOOM_BUILD_HERETIC)
    generate_man(GAME_NAME heretic)
    list(APPEND DOOM_INSTALL_DEFINES heretic)
endif()

if(DOOM_BUILD_HEXEN)
    generate_man(GAME_NAME hexen)
    list(APPEND DOOM_INSTALL_DEFINES hexen)
endif()

if(DOOM_BUILD_STRIFE)
    generate_man(GAME_NAME strife)
    list(APPEND DOOM_INSTALL_DEFINES strife)
    doom_install(FILES "${PROJECT_SOURCE_DIR}/README.Strife.md"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}/${PROGRAM_PREFIX}strife"
    )
endif()

set(EXTRA_MANS)
add_docgen(CMDLINE.doom.wiki
    WIKI_OUTPUT
    EXTRA "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/doom"
)
#list(APPEND EXTRA_MANS "${CMAKE_CURRENT_BINARY_DIR}/CMDLINE.doom.wiki")

simplecpp(INSTALL INSTALL.template
    DEFINES
        ${DOOM_INSTALL_DEFINES}
        "LONG_EXE_NAME=${PROGRAM_PREFIX}doom"
        PRECOMPILED
    EXTRA "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/doom"
)
#list(APPEND EXTRA_MANS "${CMAKE_CURRENT_BINARY_DIR}/INSTALL")

doom_install(FILES "${EXTRA_MANS}"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
)

add_custom_target(mans_extra
    DEPENDS ${EXTRA_MANS}
)

add_dependencies(mans mans_extra)

add_subdirectory(bash-completion)
