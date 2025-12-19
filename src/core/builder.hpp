#pragma once

#include "generator/cmake_generator.hpp"
#include "utils/command_helper.hpp"
#include <string>
namespace yacppm {
class Builder {
private:
  // used for dependency build dir name
  std::string get_build_hash() {}

public:
  static Builder &instance() {
    static Builder inst{};
    return inst;
  }

  void setup(std::string target, std::string arch, bool is_release) {
    if (target == "windows") {
      CmakeGenerator::gen_windows_toolchain(arch);
    } else if (target != "") {
      throw std::invalid_argument("Unsupported target");
    }
    this->is_release = is_release;
    this->target = target;
    this->arch = arch;

    CmakeGenerator::gen_build_cmake();
  }
  void build() {
    if (target != "") {

      std::string cmd = "cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=Toolchain.cmake 2>&1";
      run_command(cmd);

      cmd = "cmake --build build 2>&1";
      run_command(cmd);
    } else {

      std::string cmd = "cmake -S . -B build 2>&1";
      run_command(cmd);

      cmd = "cmake --build build 2>&1";
      run_command(cmd);
    }
  }

  std::string target = "";
  std::string arch = "";
  bool is_release = false;
};
} // namespace yacppm
