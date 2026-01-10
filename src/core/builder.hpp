#pragma once

#include "utils/constant.hpp"
#include "utils/isl_getter.hpp"
#include <string>
namespace yacppm {
class Builder {
public:
  // used for dependency build dir name
  std::string get_build_hash();

public:
  static Builder &instance() {
    static Builder inst{};
    return inst;
  }

  void setup();
  void build();

  Constant::OS target = Constant::OS::UNKNOWN;
  Constant::ARCH arch = Constant::ARCH::UNKNOWN;
  std::string build_dir_name;
  bool is_release = false;
  bool clean = false;

  bool build_success = false;
  ISL_Getter isl;
};
} // namespace yacppm
