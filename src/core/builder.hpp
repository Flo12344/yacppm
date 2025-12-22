#pragma once

#include "fmt/color.h"
#include "generator/cmake_generator.hpp"
#include "manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/constant.hpp"
#include <filesystem>
#include <sstream>
#include <string>
namespace yacppm {
class Builder {
public:
  // used for dependency build dir name
  std::string get_build_hash() {
    std::ostringstream key;
    key << target << "+" << arch << "+" << (is_release ? "Release" : "Debug");
    for (const auto &[k, v] : Manifest::instance().get_info().settings) {
      key << "+" << k << "=" << v;
    }

    // FNV-1a
    uint64_t hash = 0xcbf29ce484222325;
    uint64_t prime = 0x100000001b3;

    for (char c : key.str()) {
      hash ^= c;
      hash *= prime;
    }

    return fmt::format("{:x}", hash);
  }

public:
  static Builder &instance() {
    static Builder inst{};
    return inst;
  }

  void setup(std::string target, std::string arch, bool is_release) {

    if (target != Constant::get_current_os()) {
      if (target == "windows") {
        CmakeGenerator::gen_windows_toolchain(arch);
      } else if (target != Constant::get_current_os()) {
        throw std::invalid_argument("Unsupported target");
      }
    } else {
      std::filesystem::remove("toolchain.cmake");
    }
    this->is_release = is_release;
    this->target = target;
    this->arch = arch;

    CmakeGenerator::gen_build_cmake();
  }
  void build() {
    std::string cmd = "cmake -S . -B build/" + target + " ";
    if (target != Constant::get_current_os()) {
      cmd += "-DCMAKE_TOOLCHAIN_FILE=toolchain.cmake";
    }

    cmd += " 2>&1";
    run_command(cmd);

    cmd = "cmake --build build/" + target + " 2>&1";
    run_command(cmd);
  }

  std::string target = "";
  std::string arch = "";
  bool is_release = false;
};
} // namespace yacppm
