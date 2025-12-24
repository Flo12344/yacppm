#include "parser.hpp"
#include "cli/commands/version.hpp"
#include "commands/add.hpp"
#include "commands/build.hpp"
#include "commands/new.hpp"
#include "commands/run.hpp"
#include "commands/set.hpp"
#include "commands/symlink.hpp"
#include "utils/logger.hpp"
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

void yacppm::Parser::parse_cli_args(int argc, char *argv[]) {
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
    args.push_back({name, option, dashed});
  }

  check_command();
}
void yacppm::Parser::check_command() {
  if (args.size() == 0)
    throw std::invalid_argument("Missing command");

  if (check(false, "run")) {
    consume();
    bool is_release = false;
    if (check(false)) {
      if (check(false, "Release")) {
        consume();
        is_release = true;
      } else if (check(false, "Debug")) {
        consume();
      } else {
        throw std::invalid_argument(fmt::format("Unknown argument {}", consume()->name));
      }
    }
    run(is_release);
    return;
  }
  if (check(false, "build")) {
    consume();

    bool is_release = false;
    if (check(false)) {
      if (check(false, "Release")) {
        consume();
        is_release = true;
      } else if (check(false, "Debug")) {
        consume();
      } else {
        throw std::invalid_argument(fmt::format("Unknown argument {}", consume()->name));
      }
    }
    if (check(true, "target")) {
      std::string target = consume()->value;
      if (check(true, "arch")) {
        std::string arch = consume()->value;
        build(is_release, target, arch);
      } else {
        build(is_release, target);
      }
    } else {
      build(is_release);
    }
    return;
  }
  if (check(false, "add")) {
    consume();

    std::string type = expect(true, "", "Repo type").name;
    std::string repo = expect(false, "", "Repo link").name;
    std::string version = "latest";
    if (check(false)) {
      version = consume()->name;
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
  if (check(false, "remove")) {
    Loggger::info(consume()->name + "\n");
    return;
  }

  if (check(false, "new")) {
    consume();

    std::string name = expect(false, "", "Project name").name;
    std::string _template = "default";
    std::string _type = "exec";
    std::unordered_map<std::string, std::string> template_settings;

    while (pos < args.size()) {
      if (check(true, "template"))
        _template = args[pos].value;
      if (check(true, "type"))
        _type = args[pos].value;
      if (!_template.empty() && check(false)) {
        template_settings.insert_or_assign(args[pos].name, args[pos].value);
      }
      consume();
    }

    create(name, _type, _template, template_settings);
    return;
  }

  if (check(false, "set")) {
    consume();
    while (check(true)) {
      if (check(true, "cpp")) {
        if (args[pos].value.empty())
          throw std::invalid_argument("Missing cpp version");
        set_cxx(std::stoi(args[pos].value));
      }
      consume();
    }
    return;
  }

  if (check(false, "symlink")) {
    consume();
    symlink();
    return;
  }

  if (check(true, "v")) {
    consume();
    version();
    return;
  }

  throw std::invalid_argument("Unknown command");
}
std::optional<yacppm::CLI_Argument> yacppm::Parser::consume() {
  if (args.size() > pos) {
    int cur = pos;
    pos++;
    return args[cur];
  }
  return std::nullopt;
}
bool yacppm::Parser::check(bool dash, std::string name, int offset) {
  if (pos + offset >= args.size()) {
    return false;
  }
  CLI_Argument next = args[pos + offset];
  if (next.is_dash != dash) {
    return false;
  }

  if (name.empty())
    return true;

  if (name != next.name)
    return false;
  else
    return true;
}
std::optional<yacppm::CLI_Argument> yacppm::Parser::next(int offset) {
  if (args.size() > pos + 1 + offset) {
    pos++;
    return args[pos + offset];
  }
  return std::nullopt;
};
yacppm::CLI_Argument yacppm::Parser::expect(bool dash, const std::string &name, const std::string &type) {
  auto c = consume();
  if (!c.has_value())
    throw std::invalid_argument(fmt::format("{} missing after {}", type, args[pos].name));

  CLI_Argument next = c.value();
  if (next.is_dash == dash) {
    if (name.empty())
      return next;
    if (name != next.name)
      throw std::invalid_argument(fmt::format("Wrong argumment name {} expected {}", next.name, name));
    else
      return next;
  }
  if (dash)
    throw std::invalid_argument(fmt::format("Argumment missing a '-' {}", next.name));
  else
    throw std::invalid_argument(fmt::format("Argumment shouldn't start with a '-' {}", next.name));
}
