#pragma once
#include "logger.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <memory>
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
