#pragma once
#include "core/builder.hpp"
#include "core/manifest.hpp"
#include "fmt/color.h"
#include "utils/constant.hpp"
#include "utils/logger.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#include <toml++/toml.hpp>
namespace yacppm {

inline void build(bool is_release, std::string target = "", std::string arch = "") {
  if (!std::filesystem::exists("yacppm.toml")) {
    throw std::runtime_error("Failed, not in a yacppm Project");
  }

  if (target.empty()) {
    target = Constant::get_current_os();
  }
  if (arch.empty()) {
    arch = Constant::get_current_arch();
  }

  if (Constant::get_enum_os(target) == Constant::OS::UNKNOWN) {
    throw std::invalid_argument(fmt::format("Invalid OS target {}", target));
  }
  if (Constant::get_enum_arch(arch) == Constant::ARCH::UNKNOWN) {
    throw std::invalid_argument(fmt::format("Invalid architecture target {}", target));
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
