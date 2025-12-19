#pragma once
#include "core/manifest.hpp"
#include "toml++/toml.hpp"
#include <string>
namespace yacppm {
inline void add_header_only(std::string repo, std::string version, std::string path = "") {
  Manifest::instance().parse(toml::parse_file(path + "yacppm.toml"));
  Manifest::instance().add_dep(repo, version == "" ? "latest" : version, "header", {});
  Manifest::instance().save(path + "yacppm.toml");
}
inline void add_cmake(std::string repo, std::string version, std::string path = "") {
  Manifest::instance().parse(toml::parse_file(path + "yacppm.toml"));
  Manifest::instance().add_dep(repo, version == "" ? "latest" : version, "cmake", {});
  Manifest::instance().save(path + "yacppm.toml");
}

inline void add_local_lib(std::string repo, std::string version, std::string path = "") {
  Manifest::instance().parse(toml::parse_file(path + "yacppm.toml"));
  Manifest::instance().add_dep(repo, version, "llib", {});
  Manifest::instance().save(path + "yacppm.toml");
}

inline void add(std::string repo) {}
} // namespace yacppm
