#pragma once
#include "core/builder.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
namespace yacppm {
inline void build(bool is_release, std::string target = "", std::string arch = "") {
  if (!std::filesystem::exists("yacppm.toml")) {
    throw std::runtime_error("Failed, not in a yacppm Project");
  }

  Manifest::instance().parse(toml::parse_file("yacppm.toml"));

  Builder::instance().setup(target, arch, is_release);
  Builder::instance().build();

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

  if (std::filesystem::exists("build/compile_commands.json"))
    std::filesystem::copy("build/compile_commands.json", "compile_commands.json", opt);
}
} // namespace yacppm
