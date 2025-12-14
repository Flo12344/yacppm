#pragma once
#include "../utils/link_utils.hpp"
#include "../utils/logger.hpp"
#include "toml.hpp"
#include <fstream>
#include <string>
#include <unordered_map>
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
};

struct Manifest {
  Package package;
  std::unordered_map<std::string, Dependency> dependencies;

  void add_dep(const std::string &repo, const std::string &version,
               const std::string &type,
               std::unordered_map<std::string, std::string> settings = {}) {

    if (type == "llib") {
      dependencies.insert_or_assign(repo,
                                    Dependency{version, type, "", settings});
      return;
    }
    std::string checked = git::get_git_link(repo);
    auto repo_info = git::get_user_repo(checked);
    if (!repo_info) {
      Loggger::err("failed to add: {}", repo);
      return;
    }
    Loggger::info("Added to project: {} -> {}", repo, checked);

    dependencies.insert_or_assign(repo_info->second,
                                  Dependency{version, type, checked, settings});
  }
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

inline Manifest create_manifest(const std::string &project_name) {
  Manifest m;
  m.package.name = project_name;
  m.package.version = "0.1.0";
  return m;
}

inline Manifest parse_manifest(const toml::table &tbl) {
  Manifest m;

  if (auto *pkg = tbl["package"].as_table()) {
    m.package.name = pkg->at("name").value_or("");
    m.package.version = pkg->at("version").value_or("");
  }

  if (auto *deps = tbl["dependencies"].as_table()) {
    for (auto &&[name, val] : *deps) {
      Dependency dep;

      if (auto *d = val.as_table()) {
        dep.version = d->at("version").value_or("");

        if (auto *type = d->at("type").as_string()) {
          dep.type = type->value_or("");
        }
        if (auto *git = d->at("git").as_string()) {
          dep.git = git->value_or("");
        }
        if (d->contains("settings"))
          if (auto *feats = d->at("settings").as_table()) {
            for (auto &&[iname, ival] : *feats)
              dep.settings.insert_or_assign(iname.data(), ival.value_or(""));
          }
      }

      m.dependencies.emplace(name, dep);
    }
  }
  return m;
}

inline toml::table to_toml(const Manifest &m) {
  toml::table root;

  root.insert("package", toml::table{{"name", m.package.name},
                                     {"version", m.package.version}});

  toml::table deps;

  for (auto &[name, dep] : m.dependencies) {
    toml::table dep_tbl;
    dep_tbl.insert("version", dep.version);
    dep_tbl.insert("type", dep.type);
    dep_tbl.insert("git", dep.git);

    if (!dep.settings.empty()) {
      toml::table feat_arr;
      for (auto &f : dep.settings)
        feat_arr.insert_or_assign(f.first, f.second);

      dep_tbl.insert("settings", std::move(feat_arr));
    }
    deps.insert(name, std::move(dep_tbl));
  }

  root.insert("dependencies", std::move(deps));

  return root;
}

inline void save_manifest(const Manifest &manifest, const std::string &path) {
  std::ofstream out(path);
  out << to_toml(manifest);
}

} // namespace yacppm
