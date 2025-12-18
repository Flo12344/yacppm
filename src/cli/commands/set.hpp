#pragma once

#include "core/manifest.hpp"
#include "fmt/color.h"
#include <cstdlib>
#include <string>
namespace yacppm {
inline void set_cxx(int version) {
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));
  m.set_settings("cpp", std::to_string(version));
  save_manifest(m, "yacppm.toml");
}
inline void set_pkg_version(std::string version) {}
} // namespace yacppm
