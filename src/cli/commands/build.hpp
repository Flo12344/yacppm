#pragma once
#include "generator/cmake_generator.hpp"
#include "logger.hpp"
#include <string>
namespace yacppm {
inline void build(std::string target = "", std::string arch = "") {
  std::fstream yacppm_file("yacppm.toml", std::ios::in);
  if (yacppm_file.fail()) {
    Loggger::err("Failed, not in a yacppm Project");
    return;
  }
  yacppm_file.close();
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));

  CmakeGenerator::gen_build_cmake(m);
  if (target.empty())
    CmakeGenerator::build();
  else if (arch.empty())
    CmakeGenerator::build_toolchain(target);
  else
    CmakeGenerator::build_toolchain(target, arch);
}
} // namespace yacppm
