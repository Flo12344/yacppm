#pragma once
#include "core/builder.hpp"
#include "utils/constant.hpp"

#include "build.hpp"
#include "core/manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/logger.hpp"
namespace yacppm {
inline void run(Builder builder) {
  build(builder);
  if (!builder.build_success) {
    return;
  }

  auto process = [](const std::string &s) { Logger::info("{}", s); };
  std::string cmd;
  if (std::filesystem::exists("build/" + builder.settings.build_dir_name + "/bin/" +
                              (builder.settings.is_release ? "Release" : "Debug")))
    cmd = "cd build/" + builder.settings.build_dir_name + "/bin/" +
          (builder.settings.is_release ? "Release" : "Debug") + " && " + builder.manifest.get_info().name;
  else
    cmd = "cd build/" + builder.settings.build_dir_name + "/bin/ && ./" + builder.manifest.get_info().name;
  run_command(cmd, process);
}
} // namespace yacppm
