#pragma once

#include "core/manifest.hpp"
#include "utils/builder_settings.hpp"
#include "utils/constant.hpp"
#include "utils/isl_getter.hpp"
#include <memory>
#include <string>
#include <unordered_map>
namespace yacppm {

class Builder {
public:
  // used for dependency build dir name
  std::string get_build_hash(const std::string &repo);
  std::unordered_map<std::string, std::string> get_all_build_hash();

public:
  Builder() {

    Logger::verbose("test");
    manifest = Manifest();

    Logger::verbose("test");
  }

  void setup();
  void build();

  BuildSettings settings;

  bool build_success = false;
  ISL_Getter isl;
  Manifest manifest = {};
};
} // namespace yacppm
