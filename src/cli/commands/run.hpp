#pragma once
#include "build.hpp"

#include "core/manifest.hpp"
#include "utils/command_helper.hpp"
namespace yacppm {
inline void run(bool is_release) {
  Manifest::instance().parse(toml::parse_file("yacppm.toml"));
  build(is_release);
  run_command("cd build/bin/ && ./" + Manifest::instance().get_info().name);
}
} // namespace yacppm
