#pragma once

#include "core/manifest.hpp"
#include "core/template_parser/template_manager.hpp"
#include "fmt/format.h"
#include "utils/command_helper.hpp"
#include "utils/git_utils.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
namespace yacppm {
inline void create(std::string name, std::string type, std::string _template,
                   std::unordered_map<std::string, std::string> template_settings) {
  if (std::filesystem::exists(name) && std::filesystem::is_directory(name) && !std::filesystem::is_empty(name)) {
    throw std::invalid_argument(fmt::format("{} is not empty", name));
  }
  std::filesystem::create_directory(name);
  git::init_git_project(name);

  Manifest::instance().create(name);
  Manifest::instance().set_type(type);
  Manifest::instance().save(name);

  std::string template_path = get_bin_path() + "/templates/" + _template;
  if (!std::filesystem::exists(template_path)) {
    throw std::invalid_argument(fmt::format("Unknown template : {}", _template));
  }

  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

  std::filesystem::copy(template_path, name, opt);
  TemplateManager templates;
  templates.use_template(name + "/template.deps", template_settings);
}
} // namespace yacppm
