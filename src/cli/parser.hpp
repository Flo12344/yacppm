#pragma once

#include <string>
#include <vector>
namespace yacppm {
struct CLI_Argument {
  std::string name;
  std::string value;
  bool is_dash;
};

void parse_cli_args(int argc, char *argv[]);
void check_command(std::vector<CLI_Argument> args);
} // namespace yacppm
