cmake_minimum_required(VERSION 3.18)
project(yacppm LANGUAGES C CXX)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
endif()
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(INCLUDES
src/
/home/flo_12344/.yacppm/libs/marzer_tomlplusplus/v3.4.0/da6ff9e6f9fb7dcd
/home/flo_12344/.yacppm/libs/libgit2_libgit2/v1.9.2/da6ff9e6f9fb7dcd/include
/home/flo_12344/.yacppm/libs/fmtlib_fmt/12.1.0/da6ff9e6f9fb7dcd/include
)
set(SOURCES
src//cli/parser.cpp
src//core/manifest.cpp
src//generator/cmake_generator.cpp
src/main.cpp
src//utils/git_utils.cpp
src//utils/isl_getter.cpp
)
set(LIBRARIES
git2
fmt
ws2_32
winhttp
rpcrt4
crypt32
)

link_directories(
/usr/x86_64-w64-mingw32/lib/

/home/flo_12344/.yacppm/libs/libgit2_libgit2/v1.9.2/da6ff9e6f9fb7dcd
/home/flo_12344/.yacppm/libs/fmtlib_fmt/12.1.0/da6ff9e6f9fb7dcd
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC VERSION="0.1.0")
include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
