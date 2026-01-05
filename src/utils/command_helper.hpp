#pragma once
#include "barkeep.h"
#include "logger.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#elif defined __linux__
#include <limits.h>
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif

namespace yacppm {
inline void run_command(const std::string &command) {
  std::array<char, 256> buf;
  std::unique_ptr<FILE, int (*)(FILE *)> pipe(POPEN(command.c_str(), "r"), PCLOSE);
  if (!pipe) {
    throw std::runtime_error("popen failed!");
  }

  // TODO: Parse cmake outputs
  while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
    Loggger::verbose("{}", buf.data());
  }
}

inline void run_cmake(const std::string &command, bool is_build = false) {
  std::array<char, 256> buf;
  std::unique_ptr<FILE, int (*)(FILE *)> pipe(POPEN(command.c_str(), "r"), PCLOSE);
  if (!pipe) {
    throw std::runtime_error("popen failed!");
  }

  // TODO: Parse cmake outputs
  static const std::regex percentage(R"(\[ {0,2}[0-9]{1,3}%\])");
  int progress = 0;
  // auto bar = barkeep::ProgressBar(&progress, {
  //                                                .speed = std::nullopt,
  //                                                .style = barkeep::ProgressBarStyle::Rich,
  //                                                .show = false,
  //                                            });

  // indicators::BlockProgressBar progress{
  //     indicators::option::BarWidth{50},
  //     indicators::option::Start{"Building ["},
  //     indicators::option::End{"]"},
  //     indicators::option::ForegroundColor{indicators::Color::white},
  //     indicators::option::ShowPercentage{true},
  //     indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};
  // if (is_build) {
  //   progress.set_progress(0);
  // }
  //
  // indicators::ProgressSpinner spinner{
  //     indicators::option::PostfixText{"Setting-up cmake"},
  //     indicators::option::ForegroundColor{indicators::Color::yellow}, indicators::option::ShowPercentage{false},
  //     indicators::option::SpinnerStates{std::vector<std::string>{"⠈", "⠐", "⠠", "⢀", "⡀", "⠄", "⠂", "⠁"}},
  //     indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

  std::smatch m;
  while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
    // Loggger::verbose("{}", buf.data());
    std::string sbuf = buf.data();
    if (sbuf.starts_with("--")) {
      // spinner.set_option(indicators::option::PostfixText{sbuf.substr(3)});
      // spinner.tick();
      // Loggger::info("{}\n", sbuf);
    } else
      // get percent
      if (is_build && std::regex_search(sbuf, m, percentage)) {
        auto str = m.str();
        // Loggger::info("{}\n", str.substr(1, str.size() - 3));
        // progress.set_progress(std::stoi(str.substr(1, str.size() - 3)));
        // if (!bar->running())
        // bar->show();
        progress = (std::stoi(str.substr(1, str.size() - 3)));
        // Loggger::info("{}\n", sbuf);
      } else {
        // Loggger::info("{}\n", sbuf);
      }
  }
  // bar->done();
}

inline std::string to_camel_case(const std::string &convert) {
  bool white_space = true;
  std::string out;
  for (const auto &c : convert) {
    if (c == ' ' || c == '_') {
      white_space = true;
      continue;
    }
    if (white_space && ::isalpha(c)) {
      out += ::toupper(c);
    } else {
      out += c;
    }
  }
  return out;
}

inline std::string get_bin_path() {
  std::string path;
#ifdef _WIN32
  char _path[MAX_PATH] = {0};
  GetModuleFileName(NULL, _path, MAX_PATH);
  path = std::string(_path);
  std::replace(path.begin(), path.end(), '\\', '/');

#elif defined __linux__
  path = std::filesystem::canonical("/proc/self/exe");
#endif
  if (path.find_last_of('/') != std::string::npos) {
    path = path.substr(0, path.find_last_of('/'));
  }
  return path;
}
} // namespace yacppm
