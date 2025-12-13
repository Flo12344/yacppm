cmake_minimum_required(VERSION 3.11.0)
project(cpp-manager)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(SOURCE_FILES
    src/main.cpp
    src/utils/git_utils.cpp
    src/utils/isl_getter.cpp
    src/cli/parser.cpp
    )

include_directories(cpp-manager PRIVATE
    external/
    src/
    src/utils
    src/cli
    src/cli/commands
    src/core
    external/libgit2/include
    )

# switch to build libgit2
# add_subdirectory(external/libgit2)
add_subdirectory(external/fmt)
find_library(LIBGIT_LIBRARY
    NAMES git2
    HINTS "${CMAKE_ROOT}/external/libgit2/build")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(cpp-manager ${SOURCE_FILES})

target_link_libraries(cpp-manager PRIVATE git2 fmt)
