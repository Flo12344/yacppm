#pragma once

#include "constant.hpp"
#include <string>

namespace yacppm {

struct BuildSettings {
  Constant::OS target = Constant::OS::UNKNOWN;
  Constant::ARCH arch = Constant::ARCH::UNKNOWN;
  std::string build_dir_name;
  bool is_release = false;
  bool clean = false;
};
} // namespace yacppm
