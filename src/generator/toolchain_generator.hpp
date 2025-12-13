#pragma once

#include "logger.hpp"
#include <fstream>
#include <string>

namespace yacppm {
inline void gen_windows_toolchain(std::string architecture) {
  std::ofstream toolchain_file("toolchain.cmake");
  toolchain_file << "set(CMAKE_SYSTEM_NAME Windows)\n";
  if (architecture == "x86_64") {
    toolchain_file << "set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)\n";
    toolchain_file << "set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)\n";
  } else if (architecture == "i386") {
    toolchain_file << "set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)\n";
    toolchain_file << "set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)\n";
  } else {
    Loggger::err("Unsupported architecture for Windows target: {}\n",
                 architecture);
    return;
  }
  toolchain_file << "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n";
  toolchain_file << "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)\n";
  toolchain_file << "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)\n";
  toolchain_file << "set(CMAKE_EXE_LINKER_FLAGS \"-static-libstdc++ "
                    "-static-libgcc -static\")\n";
  toolchain_file.close();
}

} // namespace yacppm
