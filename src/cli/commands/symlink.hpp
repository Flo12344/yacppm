#pragma once
#include "utils/command_helper.hpp"
#include "utils/constant.hpp"
#include "utils/logger.hpp"
#include <filesystem>
namespace yacppm {
inline void symlink() {
  Constant::OS os = Constant::get_enum_os(Constant::get_current_os());
  switch (os) {

  case Constant::OS::WINDOWS:
    Loggger::info("Add yacppm folder to your PATH");
    break;
  case Constant::OS::LINUX:
    std::filesystem::create_symlink(get_bin_path() + "/yacppm", "/usr/local/bin/yacppm");
    break;
  case Constant::OS::UNKNOWN:
    break;
  }
}
} // namespace yacppm
