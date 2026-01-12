#include "cmake_generator.hpp"
#include "core/builder.hpp"
#include "core/manifest.hpp"
#include "fmt/format.h"
#include "utils/command_helper.hpp"
#include "utils/constant.hpp"
#include "utils/isl_getter.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

void yacppm::CmakeGenerator::gen_build_cmake(Package package, const ISL_Getter &isl, const BuildSettings &settings) {
  std::string target = Constant::get_str_os(settings.target);
  std::string extended_target = settings.build_dir_name;
  Logger::log_to_file(extended_target + "\n");

  std::fstream cmake_file("CMakeLists.txt", std::ios::out);

  auto add_if_extra_build = [&](const std::string &target, const std::string &name) {
    if (package.build_extra_options.contains(target) && package.build_extra_options[target].contains(name)) {
      for (const auto &lib : package.build_extra_options[target][name]) {
        cmake_file << lib << "\n";
      }
    }
  };

  cmake_file << "cmake_minimum_required(VERSION 3.18)\n";
  cmake_file << "project(" << package.name << " LANGUAGES C CXX)\n";
  cmake_file << "SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)\n";
  cmake_file << "SET(CMAKE_LIBRARY_PATH ${PROJECT_BINARY_DIR}/bin ${CMAKE_LIBRARY_PATH})\n";

  if (settings.is_release) {
    cmake_file << "set(CMAKE_BUILD_TYPE \"Release\")\n";
  } else {
    cmake_file << "set(CMAKE_BUILD_TYPE \"Debug\")\n";
  }

  cmake_file << parse_settings_for(package.settings, false);

  cmake_file << "set(INCLUDES\n";
  cmake_file << "src/\n";
  for (const auto &inc : isl.libs_include_paths) {
    cmake_file << inc << "\n";
  }

  cmake_file << ")\n";
  cmake_file << "set(SOURCES\n";
  for (const auto &src : isl.sources) {
    cmake_file << src << "\n";
  }
  cmake_file << ")\n";

  for (const auto &llib : isl.local_libs) {
    cmake_file << "find_package(" << llib.first << (llib.second.empty() ? "" : " " + llib.second) << " REQUIRED)\n";
  }

  cmake_file << "set(LIBRARIES\n";
  for (const auto &lib : isl.libs_names) {
    cmake_file << lib << "\n";
  }
  add_if_extra_build(target, "cross_libs");
  add_if_extra_build(extended_target, "cross_libs");

  add_if_extra_build(target, "libs");
  add_if_extra_build(extended_target, "libs");

  cmake_file << ")\n";
  cmake_file << "\n";
  cmake_file << "link_directories(\n";
  for (const auto &lib : isl.libs_paths) {
    cmake_file << lib << "\n";
  }
  for (const auto &llib : isl.local_libs) {
    cmake_file << llib.first << "\n";
  }

  add_if_extra_build(target, "cross_libs_path");
  add_if_extra_build(extended_target, "cross_libs_path");

  cmake_file << ")\n";
  cmake_file << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
  if (package.type == "exec")
    cmake_file << "add_executable(${PROJECT_NAME} ${SOURCES})\n";
  else if (package.type == "static")
    cmake_file << "add_library(${PROJECT_NAME} STATIC ${SOURCES})\n";
  else if (package.type == "shared")
    cmake_file << "add_library(${PROJECT_NAME} STATIC ${SOURCES})\n";
  cmake_file << "target_compile_definitions(${PROJECT_NAME} PUBLIC VERSION=\"" << package.version << "\")\n";
  cmake_file << "include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})\n";
  cmake_file << "target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})\n";
  cmake_file << "set_target_properties(${PROJECT_NAME} PROPERTIES \nBUILD_WITH_INSTALL_RPATH FALSE \nLINK_FLAGS "
                "\"-Wl,-rpath,$ORIGIN/\")\n";

  cmake_file.close();

  if (!std::filesystem::exists("build"))
    std::filesystem::create_directory("build");
  if (isl.licenses.size() == 1) {
    std::filesystem::copy(isl.licenses[0], "build/THIRDPARTY_LICENSES",
                          std::filesystem::copy_options::overwrite_existing);
    return;
  }
  std::fstream license_file("build/THIRDPARTY_LICENSES", std::ios::out);
  for (const auto &l : isl.licenses) {
    std::ifstream lf(l);
    std::string line;
    while (std::getline(lf, line)) {
      license_file << line << "\n";
    }
    lf.close();
  }
  license_file.close();
}

std::string yacppm::CmakeGenerator::get_windows_args(Constant::ARCH arch) {
  std::ostringstream out;
  out << "-DCMAKE_SYSTEM_NAME=Windows ";
  if (arch == Constant::ARCH::X86_64) {
    throw_if_missing({"x86_64-w64-mingw32-gcc", "x86_64-w64-mingw32-g++", "x86_64-w64-mingw32-windres",
                      "x86_64-w64-mingw32-dlltool"});
    out << "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc ";
    out << "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ ";
    out << "-DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres ";
    out << "-DDLLTOOL=x86_64-w64-mingw32-dlltool ";
    out << "-DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 ";
  } else if (arch == Constant::ARCH::I686) {
    throw_if_missing(
        {"i686-w64-mingw32-gcc", "i686-w64-mingw32-g++", "i686-w64-mingw32-windres", "i686-w64-mingw32-dlltool"});
    out << "-DCMAKE_C_COMPILER=i686-w64-mingw32-gcc ";
    out << "-DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ ";
    out << "-DCMAKE_RC_COMPILER=i686-w64-mingw32-windres ";
    out << "-DDLLTOOL=i686-w64-mingw32-dlltool ";
    out << "-DCMAKE_FIND_ROOT_PATH=/usr/i686-w64-mingw32 ";
  } else {
    throw std::invalid_argument(fmt::format("Unsupported architecture for Windows\n"));
  }
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER ";
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY ";
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY ";
  out << "-DCMAKE_EXE_LINKER_FLAGS=\"-static-libstdc++ "
         "-static-libgcc -static\" ";
  return out.str();
}
std::string yacppm::CmakeGenerator::get_linux_args(Constant::ARCH arch) {
  auto s_arch = Constant::get_str_arch(arch);
  auto gnu = arch == Constant::ARCH::ARM ? "gnueabi" : "gnu";
  std::ostringstream out;
  out << "-DCMAKE_SYSTEM_NAME=Linux ";
  if (arch == Constant::ARCH::I686 && Constant::get_current_arch() == Constant::ARCH::X86_64) {
    throw_if_missing(std::vector<std::string>{"gcc", "g++"});
    out << "-DCMAKE_SYSTEM_PROCESSOR=i686 ";
    out << "-DCMAKE_C_COMPILER=gcc ";
    out << "-DCMAKE_CXX_COMPILER=g++ ";
    out << "-DCMAKE_C_FLAGS=-m32 ";
    out << "-DCMAKE_CXX_FLAGS=-m32 ";
    out << "-DCMAKE_EXE_LINKER_FLAGS=-m32 ";
    out << "-DCMAKE_EXE_LINKER_FLAGS=-m32 ";
  } else if (arch == Constant::ARCH::I686) {
    throw_if_missing(std::vector<std::string>{"gcc-i686-linux-gnu", "g++-i686-linux-gnu"});
    out << "-DCMAKE_SYSTEM_PROCESSOR=i686 ";
    out << "-DCMAKE_C_COMPILER=gcc-i686-linux-gnu ";
    out << "-DCMAKE_CXX_COMPILER=g++-i686-linux-gnu ";
  } else if (arch != Constant::ARCH::UNKNOWN) {
    auto c1 = s_arch + "-linux-" + gnu + "-gcc";
    auto c2 = "gcc-" + s_arch + "-linux-" + gnu;
    auto cxx1 = s_arch + "-linux-" + gnu + "-g++";
    auto cxx2 = "g++-" + s_arch + "-linux-" + gnu;
    throw_if_both_missing({{c1, c2}, {cxx1, cxx2}});
    out << "-DCMAKE_SYSTEM_PROCESSOR=" << s_arch << " ";
    out << "-DCMAKE_C_COMPILER=";
    out << (has_program(c1) ? c1 : c2) << " ";
    out << "-DCMAKE_CXX_COMPILER=";
    out << (has_program(cxx1) ? cxx1 : cxx2) << " ";
  } else {
    throw std::invalid_argument(fmt::format("Unsupported architecture for Linux\n"));
  }
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER ";
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY ";
  out << "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY ";
  return out.str();
}
std::string yacppm::CmakeGenerator::get_cmd_args(yacppm::Constant::OS target, yacppm::Constant::ARCH arch) {
  std::string out{};
  auto current_os = Constant::get_current_os();
  auto current_arch = Constant::get_current_arch();
  if (target != current_os) {
    if (target == Constant::OS::WINDOWS) {
      out = CmakeGenerator::get_windows_args(arch);
    } else if (target == Constant::OS::LINUX && target != Constant::OS::WINDOWS) {
      out = CmakeGenerator::get_linux_args(arch);
    } else {
      throw std::invalid_argument("Unsupported target");
    }

  } else if (arch != current_arch) {
    if (target == Constant::OS::WINDOWS) {
      out = CmakeGenerator::get_windows_args(arch);
    } else if (target == Constant::OS::LINUX || target == Constant::OS::MACOS /*Not sure if it'll work*/) {
      out = CmakeGenerator::get_linux_args(arch);
    } else {
      throw std::invalid_argument("Unsupported target");
    }
  }

  return out;
}

const std::unordered_map<std::string, std::string> cmake_file_settings_map = {
    {"cpp", "set(CMAKE_CXX_STANDARD {})\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\nset(CMAKE_CXX_EXTENSIONS OFF)\n"},
    {"linking", "set(BUILD_SHARED_LIBS {})\n"},
};

const std::unordered_map<std::string, std::string> cmake_cmd_settings_map = {
    {"linking", "-DBUILD_SHARED_LIBS={}"},
};

std::string yacppm::CmakeGenerator::parse_settings_for(const std::unordered_map<std::string, std::string> &settings,
                                                       bool is_cmd) {
  std::ostringstream out;

  for (const auto &s : is_cmd ? cmake_cmd_settings_map : cmake_file_settings_map) {
    if (settings.contains(s.first)) {
      if (s.first == "linking")
        out << fmt::format(fmt::runtime(s.second), settings.at(s.first) == "static" ? "OFF" : "ON");
      else
        out << fmt::format(fmt::runtime(s.second), settings.at(s.first));
      if (is_cmd)
        out << " ";
    }
  }

  return out.str();
}
