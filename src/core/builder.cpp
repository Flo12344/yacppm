#include "builder.hpp"

#include "generator/cmake_generator.hpp"
#include "manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/constant.hpp"
#include <filesystem>
#include <regex>
#include <sstream>

void yacppm::Builder::build() {
  int progress = 0;
  std::string cmd = "cmake -S . -B build/" + target + " ";
  if (target != Constant::get_current_os()) {
    cmd += "-DCMAKE_TOOLCHAIN_FILE=toolchain.cmake";
  }

  cmd += " 2>&1";
  run_command(cmd);

  auto global_bar =
      barkeep::ProgressBar(&progress, {
                                          .total = 100,
                                          .format = "{green}Building{reset} " + Manifest::instance().get_info().name +
                                                    " {cyan}{percent:.2f}%{reset} {bar}",
                                          .speed = std::nullopt,
                                          .style = barkeep::ProgressBarStyle::Rich,
                                      });
  static const std::regex percentage(R"(\[ {0,2}[0-9]{1,3}%\])");

  auto process = [&](std::string sbuf) {
    std::smatch m;
    if (sbuf.starts_with("--")) {
    } else if (std::regex_search(sbuf, m, percentage)) {
      auto str = m.str();
      progress = (std::stoi(str.substr(1, str.size() - 3)));
    } else {
    }
  };

  cmd = "cmake --build build/" + target + " 2>&1";
  run_command(cmd, process);
}

void yacppm::Builder::setup(std::string target, std::string arch, bool is_release, bool clean) {

  if (target != Constant::get_current_os()) {
    if (target == "windows") {
      CmakeGenerator::gen_windows_toolchain(arch);
    } else if (target != Constant::get_current_os()) {
      throw std::invalid_argument("Unsupported target");
    }
  } else {
    std::filesystem::remove("toolchain.cmake");
  }
  this->is_release = is_release;
  this->target = target;
  this->arch = arch;
  this->clean = clean;

  CmakeGenerator::gen_build_cmake();
}

std::string yacppm::Builder::get_build_hash() {
  std::ostringstream key;
  key << target << "+" << arch << "+" << (is_release ? "Release" : "Debug");
  for (const auto &[k, v] : Manifest::instance().get_info().settings) {
    key << "+" << k << "=" << v;
  }

  // FNV-1a
  uint64_t hash = 0xcbf29ce484222325;
  uint64_t prime = 0x100000001b3;

  for (char c : key.str()) {
    hash ^= c;
    hash *= prime;
  }

  return fmt::format("{:x}", hash);
}
