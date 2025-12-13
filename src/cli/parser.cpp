#include "parser.hpp"
#include "../utils/logger.hpp"
#include "commands/add.hpp"
#include "commands/build.hpp"
#include "commands/new.hpp"
#include "commands/remove.hpp"
#include "commands/run.hpp"

void yacppm::parse_cli_args(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "run") {
      run();
    }
    if (arg == "build") {
      if (argc > 2) {
        build(argv[i + 1], argc == 4 ? argv[i + 2] : "");
      } else {
        build();
      }
    }
    if (arg == "remove") {
      if (argc != 3) {
        Loggger::err("Missing repo link/name");
        return;
      }
      remove(argv[i + 2]);
    }
    if (arg == "new") {
      if (argc < i + 1)
        create();
      else
        create(argv[i + 1]);
    }
    if (arg == "add") {
      if (argc < 4) {
        Loggger::err("Missing repo link");
        return;
      }
      if (std::string(argv[i + 1]) == "-h") {
        add_header_only(argv[i + 2], argc == 5 ? argv[i + 3] : "");
      } else if (std::string(argv[i + 1]) == "-c") {
        add_cmake(argv[i + 2], argc == 5 ? argv[i + 3] : "");
      }
    }
  }
}
