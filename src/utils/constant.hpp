#pragma once

#include <stdexcept>
#ifndef BARKEEP_ENABLE_FMT_FORMAT
#define BARKEEP_ENABLE_FMT_FORMAT
#endif
#include "barkeep.h"

#include <string>
namespace yacppm {
class Constant {
public:
  enum class OS { WINDOWS, LINUX, MACOS, UNKNOWN };
  enum class ARCH { X86_64, I686, AARCH64, ARM, RISCV, UNKNOWN };

  static OS get_enum_os(const std::string &value) {
    if (value == "windows") {
      return OS::WINDOWS;
    } else if (value == "linux") {
      return OS::LINUX;
    } else if (value == "macos") {
      return OS::MACOS;
    } else {
      return OS::UNKNOWN;
    }
  }
  static ARCH get_enum_arch(const std::string &value) {
    if (value == "x86_64" || value == "x64") {
      return ARCH::X86_64;
    } else if (value == "x32" || value == "x86" || value == "i386" || value == "i686") {
      return ARCH::I686;
    } else if (value == "aarch64" || value == "arm64") {
      return ARCH::AARCH64;
    } else if (value == "arm" || value == "arm32") {
      return ARCH::ARM;
    } else if (value == "riscv" || value == "riscv64" || value == "rv64") {
      return ARCH::RISCV;
    } else {
      return ARCH::UNKNOWN;
    }
  }

  static std::string get_str_os(OS value) {
    switch (value) {
    case OS::WINDOWS:
      return "windows";
    case OS::LINUX:
      return "linux";
    case OS::MACOS:
      return "macos";
    case OS::UNKNOWN:
      throw std::invalid_argument("UNKNOWN OS");
    }
    return "";
  }
  static std::string get_str_arch(ARCH value) {
    switch (value) {
    case ARCH::X86_64:
      return "x86_64";
    case ARCH::I686:
      return "i686";
    case ARCH::AARCH64:
      return "aarch64";
    case ARCH::ARM:
      return "arm";
    case ARCH::RISCV:
      return "riscv64";
    case ARCH::UNKNOWN:
      throw std::invalid_argument("UNKNOWN ARCH");
    }
    return "";
  }

  static ARCH get_current_arch() {
#if defined(__x86_64__) || defined(_M_X64)
    return ARCH::X86_64;
#elif defined(__i386__) || defined(_M_IX86)
    return ARCH::I686;
#elif defined(__arm__) || defined(_M_ARM)
    return ARCH::ARM;
#elif defined(__aarch64__)
    return ARCH::AARCH64;
#elif defined(RV64)
    return ARCH::RISCV;
#else
    return ARCH::UNKNOWN;
#endif
  }

  static OS get_current_os() {
#if defined(_WIN32)
    return OS::WINDOWS;
#elif defined(__APPLE__)
    return OS::MACOS;
#elif defined(__linux__)
    return OS::LINUX;
#else
    return OS::UNKNOWN;
#endif
  }
};
} // namespace yacppm
