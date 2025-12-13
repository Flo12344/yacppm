#pragma once
#include "build.hpp"

#include "command_helper.hpp"
namespace yacppm {
inline void run() {
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));
  build();
  run_command("cd build/bin/ && ./" + m.package.name);
}
} // namespace yacppm
