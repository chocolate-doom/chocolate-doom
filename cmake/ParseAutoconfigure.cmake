# Set variables set by configure.ac

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" CONFIGURE_AC_CONTENTS)
string(REGEX MATCH "AC_INIT\\(([a-zA-Z ]+),[ \t\r\n]*([a-zA-Z0-9.]+),[ \t\r\n]*([a-zA-Z0-9@.-]+),[ \t\r\n]*([a-zA-Z0-9@-]+)\\)" AUTOCONF_REGEX "${CONFIGURE_AC_CONTENTS}")
if(NOT AUTOCONF_REGEX)
    message(FATAL_ERROR "Cannot parse version from configure.ac")
endif()
set(PACKAGE_NAME "${CMAKE_MATCH_1}")
set(PACKAGE_VERSION "${CMAKE_MATCH_2}")
set(PACKAGE_EMAIL "${CMAKE_MATCH_3}")
set(PACKAGE_TARNAME "${CMAKE_MATCH_4}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

set(PACKAGE_SHORTNAME "${PACKAGE_NAME} Doom")

foreach(package_var
    PACKAGE_SHORTDESC
    PACKAGE_COPYRIGHT
    PACKAGE_LICENSE
    PACKAGE_MAINTAINER
    PACKAGE_URL
    PACKAGE_RDNS
    PACKAGE_ISSUES
)
    string(REGEX MATCH "${package_var}[ \t\r\n]*=[ \t\r\n]*\"([a-zA-Z0-9 _/():,.-]+)\"" AUTOCONF_REGEX "${CONFIGURE_AC_CONTENTS}")
    if(NOT AUTOCONF_REGEX)
        message(FATAL_ERROR "Cannot parse ${package_var} from configure.ac")
    endif()
    set("${package_var}" "${CMAKE_MATCH_1}")
endforeach()

string(REGEX REPLACE " Doom$" "" PACKAGE_SHORTNAME "${PACKAGE_NAME}")

# Without a hyphen. This is used for the bash-completion scripts.
string(TOLOWER "${PACKAGE_SHORTNAME}" PROGRAM_SPREFIX)

# With a hyphen, used almost everywhere else.
set(PROGRAM_PREFIX "${PROGRAM_SPREFIX}-" CACHE STRING "Program prefix")

string(REGEX MATCH "([a-zA-Z0-9.]+)-?.*" WINDOWS_RE_VERSION_REGEX "${PACKAGE_VERSION}")
if(NOT WINDOWS_RE_VERSION_REGEX)
    message(FATAL_ERROR "Cannot create WINDOWS_RC_VERSION from PACKAGE_VERSION")
endif()

string(REPLACE "." ", " WINDOWS_RC_VERSION "${CMAKE_MATCH_1}")
set(WINDOWS_RC_VERSION "${WINDOWS_RC_VERSION}, 0")

set(top_srcdir "${PROJECT_SOURCE_DIR}")
set(top_builddir "${PROJECT_BINARY_DIR}")
