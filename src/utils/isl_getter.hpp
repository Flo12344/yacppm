#pragma once
#include "utils/constant.hpp"
#include <cstddef>
#include <git2/checkout.h>
#include <git2/clone.h>
#include <git2/common.h>
#include <git2/global.h>
#include <git2/object.h>
#include <git2/refs.h>
#include <git2/remote.h>
#include <git2/repository.h>
#include <git2/revparse.h>
#include <git2/tag.h>
#include <git2/types.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
namespace yacppm {
class ISL_Getter {

  void parse_src_folder(const std::string &path);

  std::vector<std::string> find_libs(const std::string &path);
  std::vector<std::string> clean_lib_names(const std::vector<std::string> &libs);
  void copy_license();

  void build_cmake();
  void cmake_isl();

  void build_header();
  void header_isl();

  void create_bars();
  void reset_bars();

public:
  void retrieve_deps();
  void build_deps();
  void get_project_isl();

public:
  std::vector<std::string> libs_to_copy;
  std::vector<std::string> libs_paths;
  std::vector<std::string> libs_names;
  std::vector<std::string> libs_include_paths;
  std::vector<std::string> sources;
  std::vector<std::pair<std::string, std::string>> local_libs;
  std::vector<std::string> licenses;

private:
  std::string current_repo;
  static inline std::string current_repo_key;
  std::string current_git_path;
  std::string current_lib_path;

  std::shared_ptr<barkeep::CompositeDisplay> main_comp;
  std::shared_ptr<barkeep::CompositeDisplay> bars_comp;
  std::shared_ptr<barkeep::StatusDisplay> main_status;
  std::vector<std::shared_ptr<barkeep::BaseDisplay>> bars = {};
  static inline std::unordered_map<std::string, int> bars_progress = {};
  size_t global_progress = 0;
};
} // namespace yacppm
