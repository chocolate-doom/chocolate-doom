function(convert_icon PNG SRC)
    set(ACC_EXTRA)
    cmake_parse_arguments(ACC "" "COPY" "" ${ARGN})
    if(ACC_COPY)
        list(APPEND ACC_EXTRA COMMAND "${CMAKE_COMMAND}" -E copy "${SRC}" "${ACC_COPY}")
    endif()
    if(PYTHON_EXECUTABLE)
        add_custom_command(OUTPUT "${SRC}"
            COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_DATADIR}/convert-icon" "${PNG}" "${SRC}"
            ${ACC_EXTRA}
        )
    endif()
endfunction()
