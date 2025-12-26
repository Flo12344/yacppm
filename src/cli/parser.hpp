#pragma once

#include <optional>
#include <string>
#include <vector>
namespace yacppm {
struct CLI_Argument {
  std::string name;
  std::string value;
  bool is_dash;
};
class Parser {
private:
  std::vector<CLI_Argument> args;
  int pos;

public:
  void parse_cli_args(int argc, char *argv[]);

private:
  std::optional<CLI_Argument> consume();
  bool check(bool dash, std::string name = "", int offset = 0);
  std::optional<CLI_Argument> next(int offset = 0);
  CLI_Argument expect(bool dash, const std::string &name = "", const std::string &type = "");
  void check_command();
};

} // namespace yacppm
