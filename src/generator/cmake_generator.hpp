#pragma once

#include <fmt/base.h>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>

namespace yacppm {
class CmakeGenerator {
public:
  static void gen_build_cmake();
  static void gen_windows_toolchain(std::string architecture);
};
} // namespace yacppm
