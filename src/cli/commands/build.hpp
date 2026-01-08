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

inline void build() {
  if (!std::filesystem::exists("yacppm.toml")) {
    throw std::runtime_error("Failed, not in a yacppm Project");
  }

  if (Builder::instance().target.empty()) {
    Builder::instance().target = Constant::get_current_os();
  }
  if (Builder::instance().arch.empty()) {
    Builder::instance().arch = Constant::get_current_arch();
  }

  if (Constant::get_enum_os(Builder::instance().target) == Constant::OS::UNKNOWN) {
    throw std::invalid_argument(fmt::format("Invalid OS target {}", Builder::instance().target));
  }
  if (Constant::get_enum_arch(Builder::instance().arch) == Constant::ARCH::UNKNOWN) {
    throw std::invalid_argument(fmt::format("Invalid architecture target {}", Builder::instance().target));
  }

  Manifest::instance().parse(toml::parse_file("yacppm.toml"));

  Builder::instance().setup();
  Builder::instance().build();

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

  if (std::filesystem::exists("build/compile_commands.json"))
    std::filesystem::copy("build/compile_commands.json", "compile_commands.json", opt);
}
} // namespace yacppm
