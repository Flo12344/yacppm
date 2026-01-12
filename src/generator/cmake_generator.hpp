#pragma once

#include "utils/constant.hpp"
#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace yacppm {
class Package;
class ISL_Getter;
class BuildSettings;
class CmakeGenerator {
public:
  static void gen_build_cmake(Package package, const ISL_Getter &isl, const BuildSettings &settings);

  static std::string get_cmd_args(yacppm::Constant::OS target, yacppm::Constant::ARCH arch);
  static std::string parse_settings_for(const std::unordered_map<std::string, std::string> &settings, bool is_cmd);

private:
  static std::string get_windows_args(Constant::ARCH arch);
  static std::string get_linux_args(Constant::ARCH arch);
};
} // namespace yacppm
