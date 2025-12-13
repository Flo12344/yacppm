#pragma once

#include "core/manifest.hpp"
#include <filesystem>
#include <string>
namespace yacppm {
inline void create(std::string name = "new_project") {
  std::filesystem::create_directory(name);
  Manifest m = create_manifest(name);
  save_manifest(m, name + "/yacppm.toml");

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive |
      std::filesystem::copy_options::overwrite_existing;
  std::filesystem::copy("templates/default", name, opt);
}
} // namespace yacppm
