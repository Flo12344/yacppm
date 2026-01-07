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
/home/flo_12344/.yacppm/libs/marzer_tomlplusplus/v3.4.0/7bb0cb9e0c598422
/home/flo_12344/.yacppm/libs/libgit2_libgit2/v1.9.2/7bb0cb9e0c598422/include
/home/flo_12344/.yacppm/libs/fmtlib_fmt/12.1.0/7bb0cb9e0c598422/include
/home/flo_12344/.yacppm/libs/oir_barkeep/latest/7bb0cb9e0c598422
)
set(SOURCES
src//cli/parser.cpp
src//core/builder.cpp
src//core/manifest.cpp
src//core/template_parser/template_manager.cpp
src//generator/cmake_generator.cpp
src/main.cpp
src//utils/git_utils.cpp
src//utils/isl_getter.cpp
)
set(LIBRARIES
git2
fmt
)

link_directories(
/home/flo_12344/.yacppm/libs/libgit2_libgit2/v1.9.2/7bb0cb9e0c598422
/home/flo_12344/.yacppm/libs/fmtlib_fmt/12.1.0/7bb0cb9e0c598422
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC VERSION="0.1.5")
include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})