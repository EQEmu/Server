if (NOT MSVC)
include(FindPkgConfig)
pkg_check_modules(PC_SODIUM "libsodium")
if (NOT PC_SODIUM_FOUND)
    pkg_check_modules(PC_SODIUM "sodium")
endif (NOT PC_SODIUM_FOUND)
if (PC_SODIUM_FOUND)
  set(SODIUM_INCLUDE_HINTS ${PC_SODIUM_INCLUDE_DIRS} ${PC_SODIUM_INCLUDE_DIRS}/*)
  set(SODIUM_LIBRARY_HINTS ${PC_SODIUM_LIBRARY_DIRS} ${PC_SODIUM_LIBRARY_DIRS}/*)
endif()
endif (NOT MSVC)

# some libraries install the headers is a subdirectory of the include dir
# returned by pkg-config, so use a wildcard match to improve chances of finding
# headers and libraries.
find_path(
    SODIUM_INCLUDE_DIRS
    NAMES sodium.h
    HINTS ${SODIUM_INCLUDE_HINTS}
)

find_library(
    SODIUM_LIBRARIES
    NAMES libsodium sodium
    HINTS ${SODIUM_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SODIUM DEFAULT_MSG SODIUM_LIBRARIES SODIUM_INCLUDE_DIRS)
mark_as_advanced(SODIUM_FOUND SODIUM_LIBRARIES SODIUM_INCLUDE_DIRS)
