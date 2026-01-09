#pragma once

#include "utils/constant.hpp"
#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>

namespace yacppm {
class CmakeGenerator {
public:
  static void gen_build_cmake();

  static std::string get_cmd_args();

private:
  static std::string get_windows_args(Constant::ARCH arch);
  static std::string get_linux_args(Constant::ARCH arch);
};
} // namespace yacppm
