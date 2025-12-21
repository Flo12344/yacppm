#include "cmake_generator.hpp"
#include "core/builder.hpp"
#include "core/manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/isl_getter.hpp"
#include "utils/logger.hpp"
#include <sstream>

void yacppm::CmakeGenerator::gen_build_cmake() {
  ISL_Getter isl;
  isl.retrieve_deps();
  isl.build_deps();
  isl.get_project_isl();
  Package package = Manifest::instance().get_info();

  std::fstream cmake_file("CMakeLists.txt", std::ios::out);
  cmake_file << "cmake_minimum_required(VERSION 3.18)\n";
  cmake_file << "project(" << package.name << " LANGUAGES C CXX)\n";
  cmake_file << "SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)\n";
  cmake_file << "if(DEFINED CMAKE_TOOLCHAIN_FILE)\n";
  cmake_file << " include(${CMAKE_TOOLCHAIN_FILE})\n endif()\n";

  if (Builder::instance().is_release) {
    cmake_file << "set(CMAKE_BUILD_TYPE \"Release\")\n";
  } else {
    cmake_file << "set(CMAKE_BUILD_TYPE \"Debug\")\n";
  }

  if (package.settings.contains("cpp")) {
    cmake_file << "set(CMAKE_CXX_STANDARD " << package.settings.at("cpp") << ")\n";
    cmake_file << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    cmake_file << "set(CMAKE_CXX_EXTENSIONS OFF)\n";
  }
  cmake_file << "set(INCLUDES\n";
  cmake_file << "src/\n";
  for (const auto &inc : isl.libs_include_paths) {
    cmake_file << inc << "\n";
  }
  cmake_file << ")\n";
  cmake_file << "set(SOURCES\n";
  for (const auto &src : isl.sources) {
    cmake_file << src << "\n";
  }
  cmake_file << ")\n";

  for (const auto &llib : isl.local_libs) {
    cmake_file << "find_package(" << llib.first << (llib.second.empty() ? "" : " " + llib.second) << " REQUIRED)\n";
  }

  cmake_file << "set(LIBRARIES\n";
  for (const auto &lib : isl.libs_names) {
    cmake_file << lib << "\n";
  }
  cmake_file << ")\n";
  cmake_file << "\n";
  cmake_file << "link_directories(\n";
  for (const auto &lib : isl.libs_paths) {
    cmake_file << lib << "\n";
  }
  for (const auto &llib : isl.local_libs) {
    cmake_file << llib.first << "\n";
  }
  cmake_file << ")\n";
  cmake_file << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
  if (package.type == "exec")
    cmake_file << "add_executable(${PROJECT_NAME} ${SOURCES})\n";
  else if (package.type == "static")
    cmake_file << "add_library(${PROJECT_NAME} STATIC ${SOURCES})\n";
  else if (package.type == "shared")
    cmake_file << "add_library(${PROJECT_NAME} STATIC ${SOURCES})\n";
  cmake_file << "target_compile_definitions(${PROJECT_NAME} PUBLIC VERSION=\"" << package.version << "\")\n";
  cmake_file << "include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})\n";
  cmake_file << "target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})";

  cmake_file.close();
}

void yacppm::CmakeGenerator::gen_windows_toolchain(const std::string &architecture) {
  std::ofstream toolchain_file("toolchain.cmake");
  toolchain_file << "set(CMAKE_SYSTEM_NAME Windows)\n";
  if (architecture == "x86_64" || architecture == "x64") {
    toolchain_file << "set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)\n";
    toolchain_file << "set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)\n";
    toolchain_file << "set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)\n";
  } else if (architecture == "i386" || architecture == "x32") {
    toolchain_file << "set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)\n";
    toolchain_file << "set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)\n";
    toolchain_file << "set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)\n";
  } else {
    throw std::invalid_argument(fmt::format("Unsupported architecture for Windows target: {}\n", architecture));
  }
  toolchain_file << "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n";
  toolchain_file << "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)\n";
  toolchain_file << "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)\n";
  toolchain_file << "set(CMAKE_EXE_LINKER_FLAGS \"-static-libstdc++ "
                    "-static-libgcc -static\")\n";
  toolchain_file.close();
}
std::string yacppm::CmakeGenerator::get_windows_args(const std::string &architecture) {
  std::ostringstream out;
  out << "-DCMAKE_SYSTEM_NAME=Windows";
  if (architecture == "x86_64" || architecture == "x64") {
    out << "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc ";
    out << "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ ";
    out << "-DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres ";
  } else if (architecture == "i386" || architecture == "x32") {
    out << "-DCMAKE_C_COMPILER=i686-w64-mingw32-gcc ";
    out << "-DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ ";
    out << "-DCMAKE_RC_COMPILER=i686-w64-mingw32-windres ";
  } else {
    throw std::invalid_argument(fmt::format("Unsupported architecture for Windows target: {}\n", architecture));
  }
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER ";
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY ";
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY ";
  out << "-DCMAKE_EXE_LINKER_FLAGS=\"-static-libstdc++ "
         "-static-libgcc -static\" ";
  return out.str();
}
