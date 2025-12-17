#pragma once
#include "generator/cmake_generator.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
namespace yacppm {
inline void build(std::string target = "", std::string arch = "") {
  std::fstream yacppm_file("yacppm.toml", std::ios::in);
  if (yacppm_file.fail()) {
    throw std::runtime_error("Failed, not in a yacppm Project");
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

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive |
      std::filesystem::copy_options::overwrite_existing;

  if (std::filesystem::exists("build/compile_commands.json"))
    std::filesystem::copy("build/compile_commands.json",
                          "compile_commands.json", opt);
}
} // namespace yacppm
