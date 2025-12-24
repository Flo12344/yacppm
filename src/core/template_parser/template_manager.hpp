#pragma once

#include "cli/commands/add.hpp"
#include "core/manifest.hpp"
#include "fmt/format.h"
#include "utils/logger.hpp"
#include <cctype>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
namespace yacppm {

class TemplateManager {
private:
  struct tmpDeps {
    std::string type;
    std::string repo;
    std::string version;
    std::unordered_map<std::string, std::string> options;
  };

  std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
      return {};
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
  }

  void parse_var_dec(const std::string &line) {
    std::string var_name;
    size_t vname_s = line.find_first_of(' ');
    size_t vname_e = line.find_first_of('=');
    if (vname_e == std::string::npos) {
      vname_e = line.length();
    }
    var_name = trim(line.substr(vname_s, vname_e - vname_s));
    if (vtable.contains(var_name) && !vtable[var_name].empty())
      return;

    size_t vvalue_s = line.find_first_of('=');
    if (vvalue_s == std::string::npos) {
      vtable.insert({var_name, {}});
      return;
    }
    std::string var_value = trim(line.substr(vvalue_s + 1));
    vtable.insert({var_name, var_value});
  }
  std::string parse_var(const std::string &var) {
    if (var.starts_with('{')) {
      if (!var.ends_with('}')) {
        throw std::invalid_argument(fmt::format("Missing '{}' after : {}", "}", var));
      }
      auto v = trim(var.substr(1, var.size() - 2));
      if (vtable.contains(v)) {
        return vtable[v];
      } else {
        throw std::invalid_argument(fmt::format("Unknown var : {}", v));
      }
    }
    return var;
  }
  void parse_lib(const std::string &line) {
    size_t pos = 0;
    auto parse_element = [&](std::string &val) {
      if (auto start = line.find_first_not_of(whitespace, pos); start != std::string::npos) {
        auto end = line.find_first_of(whitespace, start + 1);
        val = trim(line.substr(pos, end - start + 1));
        val = parse_var(val);
        pos = end;
      }
    };

    std::string type;
    parse_element(type);

    std::string repo;
    parse_element(repo);

    std::string version;
    parse_element(version);
    if (version.empty())
      version = "latest";

    // INFO: unused for now
    std::unordered_map<std::string, std::string> options;
    // if (auto _pos = line.find_first_not_of(whitespace, pos); _pos != std::string::npos) {
    //   // version = trim(line.substr(pos + 1));
    // }

    libs.push_back({type, repo, version, options});
  }
  void parse_target_settings(const std::string &line) {}

  void parse(const std::string &path) {
    std::ifstream file(path);
    std::string line;
    while (getline(file, line)) {
      if (line.empty()) {
        current_line++;
        continue;
      }
      if (line.starts_with("cmake") || line.starts_with("llib") || line.starts_with("header")) {
        parse_lib(line);
      } else if (line.starts_with("var")) {
        parse_var_dec(line);
      } else if (line.starts_with("target")) {
        parse_target_settings(line);
      } else if (line.starts_with("type")) {
      }
      current_line++;
    }
  }

public:
  void print_variables(const std::string &path) {
    parse(path);
    for (const auto &var : vtable) {
      Loggger::info("{} : {}", var.first, (var.second.empty() ? "required" : "optional"));
    }
  }
  void use_template(const std::string &path, const std::unordered_map<std::string, std::string> &template_settings) {
    vtable = template_settings;
    parse(path);
    for (const auto &var : vtable) {
      if (var.second.empty())
        throw std::invalid_argument(fmt::format("Missing parameter: {}", var.first));
    }
    std::string project = Manifest::instance().get_info().name;
    for (const auto &lib : libs) {
      auto t = pkg_type(lib.type);
      switch (t) {
      case HEADER:
        add_header_only(lib.repo, lib.version, project + "/");
        break;
      case CMAKE:
        add_cmake(lib.repo, lib.version, project + "/");
        break;
      case LLIB:
        add_local_lib(lib.repo, lib.version, project + "/");
        break;
      case PKG_TYPE_MAX:
        break;
      }
    }
  }

private:
  std::unordered_map<std::string, std::string> vtable;
  std::vector<tmpDeps> libs;
  int current_line = 0;
  const char *whitespace = " \t\n\r\f\v";
};
} // namespace yacppm
