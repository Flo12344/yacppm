#pragma once

#include "cli/commands/add.hpp"
#include "core/manifest.hpp"
#include "fmt/format.h"
#include "utils/logger.hpp"
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

  std::string trim(const std::string &str);
  std::string parse_element(const std::string &line, const std::string &start_separator = whitespace,
                            const std::string &end_separator = whitespace);

  void parse_var_dec(const std::string &line);
  std::string parse_var(const std::string &var);
  void parse_lib(const std::string &line);
  void parse_target_settings(const std::string &line);
  void parse_type(const std::string &line);

  void parse(const std::string &path);
  void check_for_empty_var();

public:
  void print_variables(const std::string &path);
  void use_template(const std::string &path, const std::unordered_map<std::string, std::string> &template_settings);

private:
  std::unordered_map<std::string, std::string> vtable;
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> tmp_target_option;
  std::vector<tmpDeps> libs;
  std::string project_type;
  int current_line = 0;
  size_t pos = 0;
  inline static const char *whitespace = " \t\n\r\f\v";
};
} // namespace yacppm
