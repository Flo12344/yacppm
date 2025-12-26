#pragma once

#include "core/manifest.hpp"
#include "fmt/color.h"
#include <cstdlib>
#include <string>
namespace yacppm {
inline void set_cxx(int version) {
  Manifest::instance().parse(toml::parse_file("yacppm.toml"));
  Manifest::instance().set_settings("cpp", std::to_string(version));
  Manifest::instance().save("yacppm.toml");
}
inline void set_pkg_version(std::string version) {}
} // namespace yacppm
