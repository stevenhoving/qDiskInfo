cmake_minimum_required(VERSION 3.16)
project(qDiskInfo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED
  COMPONENTS
    Core
    Widgets
    Quick
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# for now we just do msvc
add_compile_options(
    /W3
    /diagnostics:caret
)

#include(CTest)
#enable_testing()

add_subdirectory(libdiskinfo)
add_subdirectory(qDiskInfo)

