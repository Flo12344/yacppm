#include "parser.hpp"
#include "../utils/logger.hpp"
#include "commands/add.hpp"
#include "commands/build.hpp"
#include "commands/new.hpp"
#include "commands/run.hpp"
#include <algorithm>
#include <optional>
#include <string>

void yacppm::parse_cli_args(int argc, char *argv[]) {
  std::vector<CLI_Argument> command;
  for (int i = 1; i < argc; i++) {
    std::string name = argv[i];
    std::string option;
    auto pos = name.find("=");
    if (pos != std::string::npos) {
      option = name.substr(pos + 1);
      name = name.substr(0, pos);
    }
    bool dashed = false;
    if (name.starts_with("-")) {
      name = name.substr(1);
      dashed = true;
    }
    command.push_back({name, option, dashed});
  }

  check_command(command);
}

#define is_arg_present(value, error)                                           \
  {                                                                            \
    if (!value.has_value()) {                                                  \
      Loggger::err(error);                                                     \
      return;                                                                  \
    }                                                                          \
  }

void yacppm::check_command(std::vector<yacppm::CLI_Argument> args) {
  std::reverse(args.begin(), args.end());

  auto consume = [&]() -> std::optional<CLI_Argument> {
    if (args.empty())
      return std::nullopt;
    auto arg = args.back();
    args.pop_back();
    return arg;
  };

  auto is_positional = [](CLI_Argument arg) -> bool {
    return !arg.is_dash && arg.value.empty();
  };

  auto current = consume();
  is_arg_present(current, "Missing command");
  CLI_Argument current_value = current.value();
  if (current_value.name == "run") {
    run();
    return;
  }
  if (current_value.name == "build") {
    current = consume();
    if (current.has_value()) {
      if (!is_positional(current.value())) {
        Loggger::err("Target strats with a '-' or contains a '='");
        return;
      }
      current = consume();
      if (current.has_value()) {
        if (!is_positional(current.value())) {
          Loggger::err("Architecture strats with a '-' or contains a '='");
          return;
        }
        CLI_Argument previous = current_value;
        current_value = current.value();

        build(previous.name, current_value.name);
      } else {
        build(current_value.name);
      }
    } else {
      build();
    }
    return;
  }
  if (current_value.name == "add") {
    current = consume();
    is_arg_present(current, "Missing type after add");
    current_value = current.value();
    std::string type = current_value.name;
    if (args.size() < 1) {
      Loggger::err("Missing repo link");
      return;
    }

    current = consume();
    current_value = current.value();
    std::string repo = current_value.name;

    std::string version = "latest";
    if (args.size() > 0) {
      current = consume();
      current_value = current.value();
      version = current_value.name;
    }

    if (type == "c") {
      add_cmake(repo, version);
    } else if (type == "h") {
      add_header_only(repo, version);
    } else if (type == "llib") {
      add_local_lib(repo, version);
    }
    return;
  }
  if (current_value.name == "remove") {
    Loggger::info(current_value.name + "\n");
    return;
  }
  if (current_value.name == "new") {
    current = consume();
    is_arg_present(current, "Missing project name");
    current_value = current.value();

    std::string name = current_value.name;
    std::string _template = "default";
    std::string _type = "exec";

    while (args.size() > 0) {
      current = consume();
      if (current.has_value()) {
        current_value = current.value();
        if (current_value.is_dash && !current_value.value.empty()) {
          if (current_value.name == "template")
            _template = current_value.value;
          if (current_value.name == "type")
            _type = current_value.value;
        }
      }
    }

    create(name, _template, _type);
    return;
  }

  Loggger::err("Unknown command");
  return;
}
