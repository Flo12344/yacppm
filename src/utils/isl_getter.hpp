#pragma once

#include "barkeep.h"
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

  void build_cmake(std::string git_file_path, std::string lib_file_path);
  void cmake_isl(std::string lib_file_path);

  void build_header(std::string git_file_path, std::string lib_file_path);
  void header_isl(std::string lib_file_path);

  void create_bars();
  void reset_bars();

public:
  void retrieve_deps();
  void build_deps();
  void get_project_isl();

public:
  std::vector<std::string> libs_paths;
  std::vector<std::string> libs_names;
  std::vector<std::string> libs_include_paths;
  std::vector<std::string> sources;
  std::vector<std::pair<std::string, std::string>> local_libs;

private:
  std::string current_repo;
  static inline std::string current_repo_key;

  std::shared_ptr<barkeep::CompositeDisplay> main_comp;
  std::shared_ptr<barkeep::CompositeDisplay> bars_comp;
  std::shared_ptr<barkeep::StatusDisplay> main_status;
  std::vector<std::shared_ptr<barkeep::BaseDisplay>> bars = {};
  static inline std::unordered_map<std::string, int> bars_progress = {};
  size_t global_progress = 0;
};
} // namespace yacppm
