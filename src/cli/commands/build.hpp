#pragma once
#include "generator/cmake_generator.hpp"
#include "logger.hpp"
namespace yacppm {
inline void build() {
  std::fstream yacppm_file("yacppm.toml", std::ios::in);
  if (yacppm_file.fail()) {
    Loggger::err("Failed, not in a yacppm Project");
    return;
  }
  yacppm_file.close();
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));

  CmakeGenerator::gen_build_cmake(m);
  CmakeGenerator::build();
}
} // namespace yacppm
