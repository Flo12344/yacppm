#include "builder.hpp"

#include "generator/cmake_generator.hpp"
#include "manifest.hpp"
#include "utils/command_helper.hpp"
#include "utils/constant.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

void yacppm::Builder::build() {
  int progress = 0;
  std::string cmd = "cmake -S . -B build/" + build_dir_name + " ";

  cmd += CmakeGenerator::get_cmd_args();

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

  std::vector<std::string> other;
  auto process = [&](std::string sbuf) {
    std::smatch m;
    if (sbuf.starts_with("--")) {
    } else if (std::regex_search(sbuf, m, percentage)) {
      auto str = m.str();
      progress = (std::stoi(str.substr(1, str.size() - 3)));
    } else {
      other.push_back(sbuf);
    }
  };

  cmd = "cmake --build build/" + build_dir_name;
  const auto processor_count = std::thread::hardware_concurrency();
  if (processor_count != 0) {
    cmd += fmt::format(" -j{} ", processor_count - 1);
  }
  cmd += " 2>&1";
  run_command(cmd, process);
  global_bar->done();

  static const std::regex error(R"(: error( [A-Z0-9]+)?:)");
  static const std::regex warning(R"(: warning( [A-Z0-9]+)?:)");
  static const std::regex file(R"(([a-zA-Z0-9./\-\(\)\\\_]+)(\([0-9]+\)|:[0-9]+:))");
  static bool error_found = false;
  std::for_each(other.begin(), other.end(), [](const std::string &s) {
    std::smatch m;
    std::smatch f;
    std::string affected;
    if (s.starts_with("make")) {
      return;
    }
    if (std::regex_search(s, f, file)) {
      affected = f.str().substr(std::filesystem::current_path().string().size());
    }

    if (std::regex_search(s, m, error)) {
      Loggger::err("{}{}", affected, s.substr(m.position() + m.str().size()));
      error_found = true;
    } else if (std::regex_search(s, m, warning)) {
      Loggger::warn("{}{}", affected, s.substr(m.position() + m.str().size()));
    } else if (!affected.empty()) {
      Loggger::info("{}{}", affected, s.substr(f.str().size()));
    } else {
      Loggger::info("{}", s);
    }
    // Loggger::info("{}", s);
  });
  if (error_found) {
    Loggger::err("Failed to build {}\n", Manifest::instance().get_info().name);
    return;
  }

  if (std::filesystem::exists("build/" + build_dir_name + "/compile_commands.json")) {
    std::filesystem::copy_options opt = std::filesystem::copy_options::overwrite_existing;
    std::filesystem::copy_file("build/" + build_dir_name + "/compile_commands.json", "compile_commands.json", opt);
  }

  std::string combined_license = "build/THIRDPARTY_LICENSES";
  if (std::filesystem::exists(combined_license)) {
    if (std::filesystem::exists("build/" + build_dir_name + "/bin"))
      std::filesystem::copy(combined_license, "build/" + build_dir_name + "/bin/THIRDPARTY_LICENSES",
                            std::filesystem::copy_options::overwrite_existing);
    else
      std::filesystem::copy(combined_license,
                            "build/" + build_dir_name + "/bin/" + (is_release ? "Release" : "Debug") +
                                "/THIRDPARTY_LICENSES",
                            std::filesystem::copy_options::overwrite_existing);
  }

  for (const auto &lib : isl.libs_to_copy) {
    if (std::filesystem::exists("build/" + build_dir_name + "/bin"))
      std::filesystem::copy(lib, "build/" + build_dir_name + "/bin/",
                            std::filesystem::copy_options::overwrite_existing);
    else
      std::filesystem::copy(lib, "build/" + build_dir_name + "/bin/" + (is_release ? "Release" : "Debug") + "/",
                            std::filesystem::copy_options::overwrite_existing);
  }

  Loggger::info("Built {} Successfully\n", Manifest::instance().get_info().name);
  build_success = true;
}

void yacppm::Builder::setup() {
  build_dir_name = Constant::get_str_os(target) + "_" + Constant::get_str_arch(arch);
  isl.retrieve_deps();
  isl.build_deps();
  isl.get_project_isl();
  CmakeGenerator::gen_build_cmake();
}

std::string yacppm::Builder::get_build_hash() {
  std::ostringstream key;
  key << Constant::get_str_os(target) << "+" << Constant::get_str_arch(arch) << "+"
      << (is_release ? "Release" : "Debug");
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
