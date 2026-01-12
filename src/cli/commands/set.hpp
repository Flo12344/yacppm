#pragma once

#include "core/manifest.hpp"
#include "fmt/color.h"
#include <cstdlib>
#include <string>
namespace yacppm {
inline void set_cxx(int version) {
  auto manifest = Manifest();
  manifest.set_settings("cpp", std::to_string(version));
}
inline void set_pkg_version(std::string version) {}
} // namespace yacppm
