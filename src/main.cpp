#include "cli/parser.hpp"
#include "logger.hpp"
#include <exception>
#include <fmt/base.h>

int main(int arg_count, char *argv[]) {
  try {
    yacppm::Parser parser{};
    parser.parse_cli_args(arg_count, argv);
  } catch (const std::exception &e) {
    Loggger::err(e.what());
  }
  return 0;
}
