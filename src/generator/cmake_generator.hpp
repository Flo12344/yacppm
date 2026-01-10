#pragma once

#include "utils/constant.hpp"
#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace yacppm {
class CmakeGenerator {
public:
  static void gen_build_cmake();

  static std::string get_cmd_args();
  static std::string parse_settings_for(const std::unordered_map<std::string, std::string> &settings, bool is_cmd);

private:
  static std::string get_windows_args(Constant::ARCH arch);
  static std::string get_linux_args(Constant::ARCH arch);
};
} // namespace yacppm
