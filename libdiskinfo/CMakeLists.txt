file(GLOB_RECURSE PRIVATE_SOURCES
  CONFIGURE_DEPENDS
    "private/*.cpp"
    "private/*.h"
)

file(GLOB_RECURSE PUBLIC_HEADERS
  CONFIGURE_DEPENDS
    "public/*.h"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${PRIVATE_SOURCES}
    ${PUBLIC_HEADERS}
)

# although we aim for a library lets just make our life easier by creating an executable for now
add_library(libdiskinfo
    ${PRIVATE_SOURCES}
    ${PUBLIC_HEADERS}
)

target_include_directories(libdiskinfo
    PUBLIC
        public
    PRIVATE
        private
)

target_link_libraries(libdiskinfo
    PRIVATE
        wbemuuid.lib # for the ata stuff
)

target_compile_definitions(libdiskinfo
    PRIVATE
        UNICODE
        _UNICODE
        NOMINMAX
        #WIN32_LEAN_AND_MEAN
)

target_compile_options(libdiskinfo
    PRIVATE
        /utf-8
        /Zc:__cplusplus
)

target_precompile_headers(libdiskinfo
    PRIVATE
        <windows.h>
        <string>
        <vector>
        <iostream>
)