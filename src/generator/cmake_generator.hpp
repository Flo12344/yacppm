#pragma once

#include "manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/isl_getter.hpp"
#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <string>

namespace yacppm {
class CmakeGenerator {
public:
  static void gen_build_cmake(const Manifest &m) {
    ISL_Getter isl;
    isl.get_project_isl(m);

    std::fstream cmake_file("CMakeLists.txt", std::ios::out);
    cmake_file << "cmake_minimum_required(VERSION 3.18)\n";
    cmake_file << "project(" << m.package.name << " LANGUAGES C CXX)\n";
    cmake_file << "SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)\n";
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
    cmake_file << ")\n";
    cmake_file << "add_executable(${PROJECT_NAME} ${SOURCES})\n";
    cmake_file << "target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})";

    cmake_file.close();
  }

  static void build() {
    std::string cmd = "cmake -S . -B build 2>&1";
    run_command(cmd);

    cmd = "cmake --build build 2>&1";
    run_command(cmd);
  }
};
} // namespace yacppm
