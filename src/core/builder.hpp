#pragma once

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

  std::string target = "";
  std::string arch = "";
  bool is_release = false;
  bool clean = false;

  bool build_success = false;
};
} // namespace yacppm
