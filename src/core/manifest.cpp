#include "manifest.hpp"
#include <unordered_map>
#include <vector>

using namespace yacppm;

void Manifest::add_dep(const std::string &repo, const std::string &version, const std::string &type,
                       std::unordered_map<std::string, std::string> settings) {

  if (type == "llib") {
    dependencies.insert_or_assign(repo, Dependency{version, type, "", settings});
    return;
  }
  std::string checked = git::get_git_link(repo);
  auto repo_info = git::get_user_repo(checked);
  if (!repo_info) {
    throw std::runtime_error(fmt::format("failed to add: {}", repo));
  }
  Loggger::info("Added to project: {} -> {}", repo, checked);

  dependencies.insert_or_assign(repo_info->second, Dependency{version, type, checked, settings});
}

void Manifest::set_settings(std::string name, std::string value) { package.settings.insert_or_assign(name, value); }

void Manifest::set_type(const std::string &type) {
  if (type == "exec" || type == "static" || type == "shared")
    package.type = type;
}
void Manifest::create(const std::string &project_name) {
  package.name = project_name;
  package.version = "0.1.0";
  package.settings = {};
}
void Manifest::parse(const toml::table &tbl) {

  if (auto *pkg = tbl["package"].as_table()) {
    package.name = pkg->at("name").value_or("");
    package.version = pkg->at("version").value_or("");
    package.type = pkg->at("type").value_or("exec");

    if (auto *settings = pkg->at("settings").as_table()) {
      for (auto &&[name, val] : *settings) {
        package.settings.insert_or_assign(name.data(), val.value_or(""));
      }
    }
  }

  if (auto *pkg = tbl["target"].as_table()) {
    for (auto &&[target, map] : *pkg) {

      std::unordered_map<std::string, std::vector<std::string>> target_opt;
      for (auto &&[vname, arr] : *map.as_table()) {
        std::vector<std::string> _arr;
        for (auto &&val : *arr.as_array()) {
          _arr.push_back(val.value_or(""));
        }
        target_opt.insert_or_assign(vname.data(), _arr);
      }
      package.build_extra_options[target.data()] = target_opt;
    }
  }

  if (auto *deps = tbl["dependencies"].as_table()) {
    for (auto &&[name, val] : *deps) {
      Dependency dep;

      if (auto *d = val.as_table()) {
        dep.version = d->at("version").value_or("latest");

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

      dependencies.emplace(name, dep);
    }
  }
}
toml::table Manifest::to_table() {
  toml::table root;

  toml::table pkg_settings;
  for (auto &[name, set] : package.settings) {
    pkg_settings.insert(name, set);
  }

  root.insert("package", toml::table{{"name", package.name},
                                     {"version", package.version},
                                     {"type", package.type},
                                     {"settings", pkg_settings}});

  if (!package.build_extra_options.empty()) {
    toml::table extra_options;
    for (auto &&[target, map] : package.build_extra_options) {
      if (map.empty())
        continue;
      toml::table opt;
      for (auto &&[name, arr] : map) {
        toml::array _arr;
        for (auto &&val : arr)
          _arr.push_back(val);
        opt.insert(name, _arr);
      }
      if (opt.empty())
        continue;
      extra_options.insert(target, opt);
    }
    if (!extra_options.empty()) {
      root.insert("target", extra_options);
    }
  }

  toml::table deps;

  for (auto &[name, dep] : dependencies) {
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

void Manifest::save(const std::string &path) {
  std::ofstream out(path);
  out << to_table();
}
