#pragma once
#include "core/builder.hpp"
#include "utils/constant.hpp"

#include "build.hpp"
#include "core/manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/logger.hpp"
namespace yacppm {
inline void run() {
  Manifest::instance().parse(toml::parse_file("yacppm.toml"));
  build();
  if (!Builder::instance().build_success) {
    return;
  }

  auto process = [](const std::string &s) { Loggger::info("{}", s); };
  std::string cmd;
  if (std::filesystem::exists("build/" + Constant::get_current_os() + "/bin/" +
                              (Builder::instance().is_release ? "Release" : "Debug")))
    cmd = "cd build/" + Constant::get_current_os() + "/bin/" + (Builder::instance().is_release ? "Release" : "Debug") +
          " && " + Manifest::instance().get_info().name;
  else
    cmd = "cd build/" + Constant::get_current_os() + "/bin/ && ./" + Manifest::instance().get_info().name;
  run_command(cmd, process);
}
} // namespace yacppm
