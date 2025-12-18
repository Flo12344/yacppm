#pragma once

#include "core/manifest.hpp"
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
#include <string>
#include <utility>
#include <vector>
namespace yacppm {
class ISL_Getter {

  void parse_src_folder(const std::string &path);

  std::vector<std::string> find_libs(const std::string &path);
  std::vector<std::string> clean_lib_names(const std::vector<std::string> &libs);

  void process_cmake(std::string git_file_path, std::string lib_file_path, bool already_built);

  void process_header(std::string git_file_path, std::string lib_file_path, bool already_built);

public:
  void get_project_isl(const Manifest &m);

public:
  std::vector<std::string> libs_paths;
  std::vector<std::string> libs_names;
  std::vector<std::string> libs_include_paths;
  std::vector<std::string> sources;
  std::vector<std::pair<std::string, std::string>> local_libs;
};
} // namespace yacppm
