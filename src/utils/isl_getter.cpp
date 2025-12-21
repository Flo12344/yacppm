#include "isl_getter.hpp"
#include "core/builder.hpp"
#include "core/manifest.hpp"
#include "fmt/color.h"
#include "logger.hpp"
#include "utils/command_helper.hpp"
#include "utils/git_utils.hpp"
#include <filesystem>
#include <stdexcept>

using namespace yacppm;
std::string getenv_or(const char *name, const std::string &fallback) {
  const char *val = std::getenv(name);
  return val ? std::string(val) : fallback;
}

std::string get_global_cache_dir() {
#if defined(_WIN32)
  std::filesystem::path base = getenv_or("LOCALAPPDATA", ".");
  return base.string() + "/.yacppm";

#else
  std::filesystem::path home = getenv_or("HOME", ".");
  return home.string() + "/.yacppm";
#endif
}
void yacppm::ISL_Getter::parse_src_folder(const std::string &path) {
  auto files = std::filesystem::directory_iterator(path);
  for (const auto &entry : files) {
    if (entry.is_regular_file() && (entry.path().extension() == ".c" || entry.path().extension() == ".cpp")) {
      sources.push_back(entry.path().relative_path());
    } else if (entry.is_directory()) {
      parse_src_folder(path + "/" + entry.path().filename().string());
    }
  }
}

std::vector<std::string> yacppm::ISL_Getter::find_libs(const std::string &path) {
  std::vector<std::string> libs;
  const std::vector<std::string> lib_exts = {".dll", ".so", ".a", ".lib"};

  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file()) {
      std::string ext = entry.path().extension().string();
      for (const auto &lib_ext : lib_exts) {
        if (ext == lib_ext)
          libs.push_back(entry.path().filename().string());
      }
    }
  }
  return libs;
}
std::vector<std::string> yacppm::ISL_Getter::clean_lib_names(const std::vector<std::string> &libs) {
  std::vector<std::string> cleaned;

  for (const auto &lib : libs) {
    std::string name = lib;
    size_t ext_pos = name.find_last_of(".");
    if (ext_pos != std::string::npos) {
      name = name.substr(0, ext_pos);
    }

    if (name.size() >= 3 && name.starts_with("lib")) {
      name = name.substr(3);
    }
    if (std::find(cleaned.begin(), cleaned.end(), name) == cleaned.end())
      cleaned.push_back(name);
  }
  return cleaned;
}
void yacppm::ISL_Getter::get_project_isl() {

  parse_src_folder("src/");

  std::string cache_dir = get_global_cache_dir();
  Manifest &m = Manifest::instance();
  for (auto &dep : m.get_deps()) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB) {
      local_libs.push_back({dep.first, dep.second.version});
      continue;
    }
    auto rep = git::get_user_repo(dep.second.git);
    std::string lib_file_path = cache_dir + "/libs/" + rep->first + "_" + rep->second + "/" + dep.second.version + "/" +
                                Builder::instance().get_build_hash();

    switch (pkg_type(dep.second.type)) {
    case CMAKE: {
      cmake_isl(lib_file_path);
    } break;
    case HEADER:
      header_isl(lib_file_path);
      break;
    case LLIB:

    case PKG_TYPE_MAX:
      break;
    }
  }
}

void yacppm::ISL_Getter::build_deps() {
  std::string cache_dir = get_global_cache_dir();
  Manifest &m = Manifest::instance();

  std::shared_ptr<ProgressBar> lib_build_bar = ProgressBarManager::instance().get_bar(
      ProgressBarManager::instance().create("Building libs :", m.get_deps().size()));

  int count = 0;
  count = 0;
  for (auto &dep : m.get_deps()) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB) {
      local_libs.push_back({dep.first, dep.second.version});
      continue;
    }
    auto rep = git::get_user_repo(dep.second.git);
    lib_build_bar->set_label(rep->first + "/" + rep->second);
    std::string git_file_path = cache_dir + "/git/" + rep->first + "_" + rep->second;
    std::string lib_file_path = cache_dir + "/libs/" + rep->first + "_" + rep->second + "/" + dep.second.version + "/" +
                                Builder::instance().get_build_hash();
    if (!std::filesystem::exists(git_file_path)) {
      continue;
    }

    // TODO: will need proper check
    bool already_built = false;
    if (!std::filesystem::exists(lib_file_path)) {
      std::filesystem::create_directories(lib_file_path);
    } else {
      already_built = true;
    }

    current_repo = rep->first + "/" + rep->second;
    if (!already_built)
      switch (pkg_type(dep.second.type)) {
      case CMAKE: {
        build_cmake(git_file_path, lib_file_path);
      } break;
      case HEADER:
        build_header(git_file_path, lib_file_path);
      case LLIB:

      case PKG_TYPE_MAX:
        break;
      }

    count++;
    lib_build_bar->set_progress(count);
    ProgressBarManager::instance().render();
  }
}
void yacppm::ISL_Getter::retrieve_deps() {
  std::string cache_dir = get_global_cache_dir();
  if (!std::filesystem::exists(cache_dir))
    std::filesystem::create_directory(cache_dir);
  if (!std::filesystem::exists(cache_dir + "/git"))
    std::filesystem::create_directory(cache_dir + "/git");
  if (!std::filesystem::exists(cache_dir + "/libs"))
    std::filesystem::create_directory(cache_dir + "/libs");

  Manifest &m = Manifest::instance();
  std::shared_ptr<ProgressBar> lib_get_bar = ProgressBarManager::instance().get_bar(
      ProgressBarManager::instance().create("Getting libs :", m.get_deps().size()));
  int count = 0;
  git_libgit2_init();
  for (auto &dep : m.get_deps()) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB)
      continue;

    auto rep = git::get_user_repo(dep.second.git);
    if (!std::filesystem::exists(cache_dir + "/git/" + rep->first + "_" + rep->second)) {
      git::Repository repo;
      git_clone(&repo.ptr, dep.second.git.c_str(), (cache_dir + "/git/" + rep->first + "_" + rep->second).c_str(),
                NULL);
    }

    git::Repository repo;
    git_repository_open(&repo.ptr, (cache_dir + "/git/" + rep->first + "_" + rep->second).c_str());

    if (dep.second.version == "latest") {
      git::checkout_default_branch(repo.ptr);
    } else {
      git::switch_to(repo.ptr, dep.second.version);
    }
  }

  git_libgit2_shutdown();
}

void yacppm::ISL_Getter::build_cmake(std::string git_file_path, std::string lib_file_path) {
  if (std::filesystem::exists(git_file_path + "/CMakeLists.txt")) {
    std::string cmd = "cmake -S " + git_file_path + "/ -B " + git_file_path + "/build ";
    if (auto settings = Manifest::instance().get_info().settings; settings.contains("cpp")) {
      cmd += "-DCMAKE_CXX_STANDARD=" + settings["cpp"] + " ";
    }
    cmd += "2>&1";
    run_command(cmd);

    cmd = "cmake --build " + git_file_path + "/build 2>&1";
    run_command(cmd);
  } else {
    throw std::invalid_argument(fmt::format("Unable to find CMakeLists.txt for : {}", current_repo));
  }

  auto libs = find_libs(git_file_path + "/build");
  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;
  for (const auto &lib : libs) {
    std::filesystem::copy(git_file_path + "/build/" + lib, lib_file_path, opt);
  }

  std::string include_file_path = git_file_path + "/include";
  if (libs.empty()) {
    std::string _path;
    for (const auto &entry : std::filesystem::directory_iterator(git_file_path + "/build")) {
      if (entry.is_directory() && entry.path().filename() != "CMakeFiles") {
        _path = entry.path();
        libs = find_libs(entry.path());
        if (std::filesystem::exists(entry.path().string() + "/include")) {
          include_file_path = entry.path().string() + "/include";
        }
        if (!libs.empty())
          break;
      }
    }

    for (const auto &lib : libs) {
      std::filesystem::copy(_path + "/" + lib, lib_file_path, opt);
    }
  }

  if (std::filesystem::exists(include_file_path)) {
    std::filesystem::copy(include_file_path, lib_file_path + "/include", opt);
  }
}

void yacppm::ISL_Getter::build_header(std::string git_file_path, std::string lib_file_path) {
  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

  if (std::filesystem::exists(git_file_path + "/single_include")) {
    std::filesystem::copy(git_file_path + "/single_include", lib_file_path, opt);
  } else if (std::filesystem::exists(git_file_path + "/include")) {
    std::filesystem::copy(git_file_path + "/include", lib_file_path, opt);
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(git_file_path)) {
      auto ext = entry.path().filename().extension().string();
      if (ext == "hpp" || ext == "h") {
        std::filesystem::copy(entry.path(), lib_file_path, opt);
      }
    }
  }
}
void yacppm::ISL_Getter::header_isl(std::string lib_file_path) {
  if (std::filesystem::exists(lib_file_path + "/single_include")) {
    libs_include_paths.push_back(lib_file_path + "/single_include");
  } else if (std::filesystem::exists(lib_file_path + "/include")) {
    libs_include_paths.push_back(lib_file_path + "/include");
  }
  libs_include_paths.push_back(lib_file_path);
}
void yacppm::ISL_Getter::cmake_isl(std::string lib_file_path) {

  std::string include_file_path = lib_file_path + "/include";
  auto libs = find_libs(lib_file_path);
  std::vector<std::string> lib_names = clean_lib_names(libs);

  libs_paths.push_back(lib_file_path);
  for (const auto &lib : lib_names) {
    libs_names.push_back(lib);
  }

  if (std::filesystem::exists(include_file_path)) {
    libs_include_paths.push_back(lib_file_path + "/include");
  }
}
