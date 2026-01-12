#pragma once

#include "toml++/toml.hpp"
#include "utils/constant.hpp"
#include "utils/logger.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
namespace yacppm {

struct Dependency {
  std::string version;
  std::string type;
  std::string git;
  std::unordered_map<std::string, std::string> settings;
};

struct Package {
  std::string name;
  std::string version;
  std::string type;
  std::unordered_map<std::string, std::string> settings;
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> build_extra_options;
};

class Manifest {

public:
  Manifest(const std::string &path = "") {
    if (std::filesystem::exists("yacppm.toml")) {
      parse(toml::parse_file("yacppm.toml"));
    } else if (!path.empty()) {
      create(path);
    } else {
      return;
    }

    project_toml_path = path.empty() ? "" : (path + "/") + "yacppm.toml";
  }
  ~Manifest() { save(); }

  void create(const std::string &project_name);
  void save();

  void set_settings(std::string name, std::string value);
  void set_type(const std::string &type);
  void add_target_option(const std::string &target, const std::string &name, std::vector<std::string> value) {
    if (package.build_extra_options.contains(target) && package.build_extra_options[target].contains(name)) {
      package.build_extra_options[target][name].insert(package.build_extra_options[target][name].end(), value.begin(),
                                                       value.end());
    } else
      package.build_extra_options[target].insert_or_assign(name, value);
  }
  void set_targets_options(
      const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &options) {
    package.build_extra_options = options;
  }

  Package get_info() const { return package; }
  std::unordered_map<std::string, Dependency> get_deps() const { return dependencies; }
  void add_dep(const std::string &repo, const std::string &version, const std::string &type,
               std::unordered_map<std::string, std::string> settings = {});

private:
  void parse(const toml::table &tbl);
  toml::table to_table();

  std::string project_toml_path;
  Package package;
  std::unordered_map<std::string, Dependency> dependencies;
};

enum PkgType { HEADER, CMAKE, LLIB, PKG_TYPE_MAX };
inline PkgType pkg_type(std::string type) {
  if (type == "header")
    return HEADER;
  if (type == "cmake")
    return CMAKE;
  if (type == "llib")
    return LLIB;
  return PKG_TYPE_MAX;
}
} // namespace yacppm
