#pragma once

#include "manifest.hpp"
#include <string>
#include <unordered_map>
namespace yacppm {
class TemplateParser {
private:
  void parse_var(std::string var) {}
  void parse_add(std::string line) {}

public:
  void parse(std::string path, Manifest &m) {}
  void pass_options(std::string options) {}

private:
  std::string path;
  std::unordered_map<std::string, std::string> vars;
};
} // namespace yacppm
