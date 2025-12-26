#pragma once

#include <string>
namespace yacppm {
class Constant {
public:
  enum class OS { WINDOWS, LINUX, UNKNOWN };
  enum class ARCH { X86_64, I386, UNKNOWN };

  static OS get_enum_os(const std::string &value) {
    if (value == "windows") {
      return OS::WINDOWS;
    } else if (value == "linux") {
      return OS::LINUX;
    } else {
      return OS::UNKNOWN;
    }
  }
  static ARCH get_enum_arch(const std::string &value) {
    if (value == "x86_64" || value == "x64") {
      return ARCH::X86_64;
    } else if (value == "x32" || value == "i386") {
      return ARCH::I386;
    } else {
      return ARCH::UNKNOWN;
    }
  }

  static std::string get_current_arch() {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    return "i386";
#else
    return "unknown";
#endif
  }

  static std::string get_current_os() {
#if defined(_WIN32)
    return "windows";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown";
#endif
  }
};
} // namespace yacppm
