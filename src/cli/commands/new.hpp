#pragma once

#include "add.hpp"
#include "core/manifest.hpp"
#include "fmt/color.h"
#include "utils/command_helper.hpp"
#include "utils/logger.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
namespace yacppm {
inline void create(std::string name, std::string _template, std::string type) {
  std::filesystem::create_directory(name);
  Manifest::instance().create(name);
  Manifest::instance().set_type(type);
  Manifest::instance().save(name + "/yacppm.toml");

  std::string template_path = get_bin_path() + "/templates/" + _template;
  if (!std::filesystem::exists(template_path)) {
    throw std::invalid_argument(fmt::format("Unknown template : {}", _template));
  }

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

  std::filesystem::copy(template_path, name, opt);
  if (std::filesystem::exists(name + "/template.deps")) {
    std::ifstream file(name + "/template.deps");
    std::string line;
    int lpos = 0;
    while (getline(file, line)) {
      lpos++;
      if (line.empty()) {
        continue;
      }
      if (line.starts_with("type")) {
        Manifest::instance().set_type(line.substr(line.find_first_of(" ") + 1));
        Manifest::instance().save(name + "/yacppm.toml");
        continue;
      }

      std::string type = line.substr(0, line.find_first_of(" "));
      line = line.substr(line.find_first_of(" ") + 1);
      std::string git = line.substr(0, line.find_first_of(" "));
      line = line.substr(line.find_first_of(" ") + 1);
      std::string version = line;

      if (type.empty() || git.empty() || version.empty()) {
        throw std::invalid_argument(
            fmt::format("Error with package in template dependencies at line: {} for template: {}", lpos, _template));
      }

      auto t = pkg_type(type);
      switch (t) {
      case HEADER:
        add_header_only(git, version, name + "/");
        break;
      case CMAKE:
        add_cmake(git, version, name + "/");
        break;
      case LLIB:
        add_local_lib(git, version, name + "/");
        break;
      case PKG_TYPE_MAX:
        break;
      }
    }
    file.close();
  }
  std::filesystem::remove(name + "/template.deps");
}
} // namespace yacppm
