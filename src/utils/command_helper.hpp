#pragma once
#include "logger.hpp"
#include <array>
#include <memory>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif

inline void run_command(std::string command) {
  std::array<char, 256> buf;
  std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(command.c_str(), "r"),
                                                PCLOSE);
  if (!pipe) {
    Loggger::err("popen failed!");
    return;
  }

  // TODO: Parse cmake outputs
  while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
    Loggger::verbose(buf.data());
  }
}
