#include "cli/parser.hpp"
#include <fmt/base.h>

int main(int arg_count, char *argv[]) {
  yacppm::parse_cli_args(arg_count, argv);
  return 0;
}
