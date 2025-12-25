#include "template_manager.hpp"
#include "core/manifest.hpp"
#include "utils/logger.hpp"
#include <string>
#include <vector>

std::string yacppm::TemplateManager::trim(const std::string &str) {
  size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos)
    return {};
  size_t end = str.find_last_not_of(whitespace);
  return str.substr(start, end - start + 1);
}

std::string yacppm::TemplateManager::parse_var(const std::string &var) {
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

std::string yacppm::TemplateManager::parse_element(const std::string &line, const std::string &start_separator,
                                                   const std::string &end_separator) {
  std::string val;
  if (auto start = line.find_first_not_of(start_separator, pos); start != std::string::npos) {
    auto end = line.find_first_of(end_separator, start + 1);
    if (end == std::string::npos)
      end = line.size();
    val = trim(line.substr(start, end - start));
    val = parse_var(val);
    pos = end;
  } else {
    pos = line.size();
  }
  return val;
}

void yacppm::TemplateManager::parse_var_dec(const std::string &line) {
  // add var size
  pos = 4;
  std::string var_name = parse_element(line, whitespace, "=");
  if (vtable.contains(var_name) && !vtable[var_name].empty())
    return;

  std::string var_value = parse_element(line, "=");
  vtable.insert({var_name, var_value});
}

void yacppm::TemplateManager::parse_lib(const std::string &line) {
  std::string type = parse_element(line);
  std::string repo = parse_element(line);
  std::string version = parse_element(line);
  if (version.empty())
    version = "latest";

  // WARN: unused for now
  std::unordered_map<std::string, std::string> options;

  libs.push_back({type, repo, version, options});
}

void yacppm::TemplateManager::parse_target_settings(const std::string &line) {
  pos = line.find_first_of(whitespace) + 1;
  std::string target = parse_element(line);
  if (target.empty())
    return;
  std::string option = parse_element(line);
  if (option.empty())
    return;
  std::vector<std::string> options;
  while (pos < line.size()) {
    std::string o = parse_element(line);
    if (o.size() > 0)
      options.push_back(o);
  }
  tmp_target_option[target][option] = options;
}

void yacppm::TemplateManager::parse(const std::string &path) {
  std::ifstream file(path);
  std::string line;
  while (getline(file, line)) {
    if (line.empty()) {
      current_line++;
      continue;
    }
    pos = 0;
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

void yacppm::TemplateManager::print_variables(const std::string &path) {
  parse(path);
  for (const auto &var : vtable) {
    Loggger::info("{} : {}", var.first, (var.second.empty() ? "required" : "optional"));
  }
}

void yacppm::TemplateManager::use_template(const std::string &path,
                                           const std::unordered_map<std::string, std::string> &template_settings) {
  vtable = template_settings;
  parse(path);
  check_for_empty_var();

  auto package = Manifest::instance().get_info();
  std::string project_path = package.name + "/";
  for (const auto &lib : libs) {
    auto t = pkg_type(lib.type);
    switch (t) {
    case HEADER:
      add_header_only(lib.repo, lib.version, project_path);
      break;
    case CMAKE:
      add_cmake(lib.repo, lib.version, project_path);
      break;
    case LLIB:
      add_local_lib(lib.repo, lib.version, project_path);
      break;
    case PKG_TYPE_MAX:
      break;
    }
  }

  Manifest::instance().set_targets_options(tmp_target_option);
  Manifest::instance().save(project_path);
}
void yacppm::TemplateManager::check_for_empty_var() {
  for (const auto &var : vtable) {
    if (var.second.empty())
      throw std::invalid_argument(fmt::format("Missing parameter: {}", var.first));
  }
}
