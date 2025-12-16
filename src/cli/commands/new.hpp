#pragma once

#include "add.hpp"
#include "core/manifest.hpp"
#include "logger.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
namespace yacppm {
inline void create(std::string name, std::string _template, std::string type) {
  std::filesystem::create_directory(name);
  Manifest m = create_manifest(name);
  m.set_type(type);
  save_manifest(m, name + "/yacppm.toml");

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive |
      std::filesystem::copy_options::overwrite_existing;
  if (!std::filesystem::exists("templates/" + _template)) {
    Loggger::err("Unknown template : {}", _template);
    return;
  }
  std::filesystem::copy("templates/" + _template, name, opt);
  if (std::filesystem::exists(name + "/template.deps")) {
    std::ifstream file(name + "/template.deps");
    std::string line;
    while (getline(file, line)) {
      if (line.empty()) {
        continue;
      }
      std::string type = line.substr(0, line.find_first_of(" "));
      line = line.substr(line.find_first_of(" ") + 1);
      std::string git = line.substr(0, line.find_first_of(" "));
      line = line.substr(line.find_first_of(" ") + 1);
      std::string version = line;
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
