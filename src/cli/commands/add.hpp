#pragma once
#include "../../core/manifest.hpp"
#include "toml.hpp"
#include <string>
namespace yacppm {
inline void add_header_only(std::string repo, std::string version) {
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));
  m.add_dep(repo, version == "" ? "latest" : version, "header", {});
  save_manifest(m, "yacppm.toml");
}
inline void add_cmake(std::string repo, std::string version) {
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));
  m.add_dep(repo, version == "" ? "latest" : version, "cmake", {});
  save_manifest(m, "yacppm.toml");
}

inline void add_local_lib(std::string repo, std::string version) {
  Manifest m = parse_manifest(toml::parse_file("yacppm.toml"));
  m.add_dep(repo, version, "llib", {});
  save_manifest(m, "yacppm.toml");
}

inline void add(std::string repo) {}
} // namespace yacppm
