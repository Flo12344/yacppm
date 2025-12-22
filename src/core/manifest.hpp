#pragma once
#include "../utils/link_utils.hpp"
#include "../utils/logger.hpp"
#include "toml++/toml.hpp"
#include <fstream>
#include <memory>
#include <stdexcept>
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
  static Manifest &instance() {
    static Manifest inst;
    return inst;
  }
  void create(const std::string &project_name);
  void parse(const toml::table &tbl);
  void save(const std::string &path);

  void add_dep(const std::string &repo, const std::string &version, const std::string &type,
               std::unordered_map<std::string, std::string> settings = {});
  void set_settings(std::string name, std::string value);
  void set_type(const std::string &type);
  void add_target_option(const std::string &target, const std::string &name, std::vector<std::string> value) {
    if (package.build_extra_options.contains(target) && package.build_extra_options[target].contains(name)) {
      package.build_extra_options[target][name].insert(package.build_extra_options[target][name].end(), value.begin(),
                                                       value.end());
    } else
      package.build_extra_options[target].insert_or_assign(name, value);
  }

  Package &get_info() { return package; }
  std::unordered_map<std::string, Dependency> &get_deps() { return dependencies; }

private:
  toml::table to_table();

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
