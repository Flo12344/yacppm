#include "command_helper.hpp"

#include "utils/logger.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <stdexcept>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#elif defined(__APPLE__)
#include <limits.h>
#include <mach-o/dyld.h>
#define POPEN popen
#define PCLOSE pclose
#elif defined __linux__
#include <limits.h>
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif

void yacppm::run_command(const std::string &command, std::function<void(std::string)> func) {
  std::array<char, 256> buf;
  std::unique_ptr<FILE, int (*)(FILE *)> pipe(POPEN(command.c_str(), "r"), PCLOSE);
  if (!pipe) {
    throw std::runtime_error("popen failed!");
  }

  while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
    std::string sbuf = buf.data();

    Logger::log_to_file(sbuf);
    if (func)
      func(sbuf);
  }
}

bool yacppm::has_program(const std::string &program_name) {
#if defined(_WIN32)
  std::string cmd = "where " + program_name + " 2>nul";
#else
  std::string cmd = "command -v " + program_name + " 2>/dev/null";
#endif

  std::array<char, 256> buf;
  FILE *pipe = POPEN(cmd.c_str(), "r");
  if (!pipe) {
    throw std::runtime_error("popen failed");
  }

  while (fgets(buf.data(), buf.size(), pipe)) {
  }

  int status = PCLOSE(pipe);
  return status == 0;
}
void yacppm::throw_if_missing(const std::string &prog) {
  if (!yacppm::has_program(prog))
    throw std::invalid_argument(fmt::format("Environement doesn't have {} available through commands", prog));
}
void yacppm::throw_if_missing(const std::vector<std::string> &progs) {
  std::string missings;
  for (const auto &prog : progs) {
    if (!yacppm::has_program(prog)) {
      if (!missings.empty())
        missings += ", ";
      missings += prog;
    }
  }
  if (!missings.empty())
    throw std::invalid_argument(fmt::format("Environement doesn't have {} available through commands", missings));
}
void yacppm::throw_if_both_missing(const std::vector<std::pair<std::string, std::string>> &progs) {
  std::string missings;
  for (const auto &prog : progs) {
    if (!yacppm::has_program(prog.first) && !yacppm::has_program(prog.second)) {
      if (!missings.empty())
        missings += ", ";
      missings += "(" + prog.first + "or" + prog.second + ")";
    }
  }
  if (!missings.empty())
    throw std::invalid_argument(fmt::format("Environement doesn't have {} available through commands", missings));
}
std::string yacppm::to_camel_case(const std::string &convert) {
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
std::string yacppm::get_bin_path() {
  std::string path;
#ifdef _WIN32
  char _path[MAX_PATH] = {0};
  GetModuleFileName(NULL, _path, MAX_PATH);
  path = std::string(_path);
#elif defined(__APPLE__)
  char _path[PATH_MAX];
  uint32_t size = sizeof(_path);
  if (_NSGetExecutablePath(_path, &size) == 0) {
    path = std::string(_path);
  }
#elif defined __linux__
  path = std::filesystem::canonical("/proc/self/exe");
#endif
  std::replace(path.begin(), path.end(), '\\', '/');
  if (path.find_last_of('/') != std::string::npos) {
    path = path.substr(0, path.find_last_of('/'));
  }
  return path;
}
