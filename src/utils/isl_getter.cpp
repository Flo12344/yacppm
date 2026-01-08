#include "isl_getter.hpp"

#include "core/builder.hpp"
#include "core/manifest.hpp"
#include "generator/cmake_generator.hpp"
#include "utils/command_helper.hpp"
#include "utils/git_utils.hpp"
#include "utils/link_utils.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace yacppm;
std::string getenv_or(const char *name, const std::string &fallback) {
  const char *val = std::getenv(name);
  return val ? std::string(val) : fallback;
}

std::string get_global_cache_dir() {
#if defined(_WIN32)
  std::string path = getenv_or("LOCALAPPDATA", ".");

  std::replace(path.begin(), path.end(), '\\', '/');
  return path + "/.yacppm";

#else
  std::filesystem::path home = getenv_or("HOME", ".");
  return home.string() + "/.yacppm";
#endif
}
void yacppm::ISL_Getter::parse_src_folder(const std::string &path) {
  auto files = std::filesystem::directory_iterator(path);
  for (const auto &entry : files) {
    if (entry.is_regular_file() && (entry.path().extension() == ".c" || entry.path().extension() == ".cpp")) {
      sources.push_back(entry.path().relative_path().string());
    } else if (entry.is_directory()) {
      parse_src_folder(path + "/" + entry.path().filename().string());
    }
  }
}

void parse_build_folder(const std::string &path, std::vector<std::string> &libs) {
  const std::vector<std::string> lib_exts = {".dll", ".so", ".a", ".lib"};
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file()) {
      std::string ext = entry.path().extension().string();
      for (const auto &lib_ext : lib_exts) {
        if (ext == lib_ext)
          libs.push_back(entry.path().string());
      }
    } else if (entry.is_directory()) {
      std::string fname = entry.path().filename().string();
      if (fname == (Builder::instance().is_release ? "Release" : "Debug") || fname == "bin") {
        parse_build_folder(entry.path().string(), libs);
      }
    }
  }
}

std::vector<std::string> yacppm::ISL_Getter::find_libs(const std::string &path) {
  std::vector<std::string> libs;

  std::string _path = path;
  if (auto tmp = path + "/" + ::to_camel_case(path); std::filesystem::exists(tmp))
    _path = tmp;

  parse_build_folder(_path, libs);

  return libs;
}
std::vector<std::string> yacppm::ISL_Getter::clean_lib_names(const std::vector<std::string> &libs) {
  std::vector<std::string> cleaned;

  for (const auto &lib : libs) {
    std::string name = lib;
    std::replace(name.begin(), name.end(), '\\', '/');
    size_t last_slash = name.find_last_of("/") + 1;
    name = name.substr(last_slash);
    size_t ext_pos = name.find_last_of(".");
    name = name.substr(0, ext_pos);

    if (name.size() >= 3 && name.starts_with("lib")) {
      name = name.substr(3);
    }
    if (std::find(cleaned.begin(), cleaned.end(), name) == cleaned.end())
      cleaned.push_back(name);
  }
  return cleaned;
}
void yacppm::ISL_Getter::get_project_isl() {
  current_git_path.clear();

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

    current_lib_path = lib_file_path;
    switch (pkg_type(dep.second.type)) {
    case CMAKE: {
      cmake_isl();
    } break;
    case HEADER:
      header_isl();
      break;
    case LLIB:

    case PKG_TYPE_MAX:
      break;
    }
  }
}

void yacppm::ISL_Getter::create_bars() {
  global_progress = 0;
  auto deps = Manifest::instance().get_deps();
  main_status = barkeep::Status({
      .show = false,
  });
  auto lib_build_bar =
      barkeep::ProgressBar(&global_progress, {
                                                 .total = deps.size(),
                                                 .format = " {cyan}{percent:.2f}%{reset} {bar} {value}/{total}",
                                                 .speed = std::nullopt,
                                                 .style = barkeep::ProgressBarStyle::Rich,
                                                 .show = false,
                                             });

  for (auto &dep : deps) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB) {
      continue;
    }

    auto rep = git::get_user_repo(dep.second.git);
    auto key = rep->first + "/" + rep->second;
    bars_progress[key] = 0;

    auto bar =
        barkeep::ProgressBar(&bars_progress[key], {
                                                      .total = 100,
                                                      .format = "   " + key + " {cyan}{percent:.2f}%{reset} {bar}",
                                                      .speed = std::nullopt,
                                                      .style = barkeep::ProgressBarStyle::Rich,
                                                      .show = false,
                                                  });
    bars.push_back(bar);
  }
  main_comp = barkeep::Composite({main_status, lib_build_bar}, " ");
  bars.push_back(main_comp);
  bars_comp = barkeep::Composite(bars, "\n");
  bars_comp->show();
}

void yacppm::ISL_Getter::reset_bars() {
  global_progress = 0;
  auto deps = Manifest::instance().get_deps();

  for (auto &dep : deps) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB) {
      continue;
    }

    auto rep = git::get_user_repo(dep.second.git);
    auto key = rep->first + "/" + rep->second;
    bars_progress[key] = 0;
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

  create_bars();
  main_status->message("Fetching/Checkout deps");

  git_libgit2_init();
  auto checkout_progress = [](const char *path, size_t cur, size_t tot, void *payload) {
    bars_progress[current_repo_key] = cur / (float)tot * 100;
  };
  auto fetch_progress = [](const git_indexer_progress *stats, void *payload) -> int {
    bars_progress[current_repo_key] = stats->received_objects / (float)stats->total_objects * 100;
    return 0;
  };
  git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
  clone_opts.checkout_opts.progress_cb = checkout_progress;
  clone_opts.fetch_opts.callbacks.transfer_progress = fetch_progress;

  for (auto &dep : Manifest::instance().get_deps()) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB)
      continue;

    auto rep = git::get_user_repo(dep.second.git);
    current_repo_key = rep->first + "/" + rep->second;

    if (!std::filesystem::exists(cache_dir + "/git/" + rep->first + "_" + rep->second) || Builder::instance().clean) {
      git::Repository repo;
      git_clone(&repo.ptr, dep.second.git.c_str(), (cache_dir + "/git/" + rep->first + "_" + rep->second).c_str(),
                &clone_opts);
    }

    git::Repository repo;
    git_repository_open(&repo.ptr, (cache_dir + "/git/" + rep->first + "_" + rep->second).c_str());

    if (dep.second.version == "latest") {
      git::checkout_default_branch(repo.ptr);
    } else {
      git::switch_to(repo.ptr, dep.second.version);
    }
    global_progress++;
  }

  git_libgit2_shutdown();
}

void yacppm::ISL_Getter::build_deps() {
  std::string cache_dir = get_global_cache_dir();
  Manifest &m = Manifest::instance();

  reset_bars();
  main_status->message("Building deps");

  for (auto &dep : m.get_deps()) {
    if (dep.second.git.empty() && pkg_type(dep.second.type) == LLIB) {
      local_libs.push_back({dep.first, dep.second.version});
      continue;
    }
    auto rep = git::get_user_repo(dep.second.git);
    current_repo_key = rep->first + "/" + rep->second;
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

    current_repo = rep->second;
    current_git_path = git_file_path;
    current_lib_path = lib_file_path;
    copy_license();
    if (!already_built || Builder::instance().clean)
      switch (pkg_type(dep.second.type)) {
      case CMAKE: {
        build_cmake();
      } break;
      case HEADER:
        build_header();
      case LLIB:

      case PKG_TYPE_MAX:
        break;
      }
    else {
      auto key = rep->first + "/" + rep->second;
      bars_progress[key] = 100;
    }

    global_progress++;
  }
  main_status->message("Deps Built");
  main_status->done();
}

void yacppm::ISL_Getter::build_cmake() {
  if (std::filesystem::exists(current_git_path + "/CMakeLists.txt")) {
    std::string cmd = "cmake -S " + current_git_path + "/ -B " + current_git_path + "/build ";
    if (auto settings = Manifest::instance().get_info().settings; settings.contains("cpp")) {
      cmd += "-DCMAKE_CXX_STANDARD=" + settings["cpp"] + " ";
    }
    if (Constant::get_current_os() != Builder::instance().target) {
      if (Constant::get_current_os() == "linux" && Builder::instance().target == "windows") {
        cmd += CmakeGenerator::get_windows_args(Builder::instance().arch);
      } else {
        throw std::invalid_argument("Currently not supported");
      }
      // WARN: will be needed when adding lib options
      // if (Manifest::instance().get_deps()[current_repo].settings.contains("cross_libs")) {
      // }
    }
    cmd += "2>&1";
    run_command(cmd, nullptr);

    cmd = "cmake --build " + current_git_path + "/build 2>&1";
    static const std::regex percentage(R"(\[ {0,2}[0-9]{1,3}%\])");

    auto process = [&](std::string sbuf) {
      std::smatch m;
      if (sbuf.starts_with("--")) {
      } else if (std::regex_search(sbuf, m, percentage)) {
        auto str = m.str();
        bars_progress[current_repo_key] = (std::stoi(str.substr(1, str.size() - 3)));
      } else {
      }
    };
    run_command(cmd, process);
  } else {
    throw std::invalid_argument(fmt::format("Unable to find CMakeLists.txt for : {}", current_repo));
  }

  auto libs = find_libs(current_git_path + "/build");
  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;
  for (const auto &lib : libs) {
    std::filesystem::copy(lib, current_lib_path, opt);
  }

  std::string include_file_path = current_git_path + "/include";
  if (libs.empty()) {
    std::string _path;
    for (const auto &entry : std::filesystem::directory_iterator(current_git_path + "/build")) {
      if (entry.is_directory() && entry.path().filename() != "CMakeFiles") {
        _path = entry.path().string();
        libs = find_libs(entry.path().string());
        if (std::filesystem::exists(entry.path().string() + "/include")) {
          include_file_path = entry.path().string() + "/include";
        }
        if (!libs.empty())
          break;
      }
    }

    for (const auto &lib : libs) {
      std::filesystem::copy(lib, current_lib_path, opt);
    }
  }

  if (std::filesystem::exists(include_file_path)) {
    std::filesystem::copy(include_file_path, current_lib_path + "/include", opt);
  }

  // INFO: remove build dir to avoid issue when cross building
  if (std::filesystem::exists(current_git_path + "/build/"))
    std::filesystem::remove_all(current_git_path + "/build/");
}

void yacppm::ISL_Getter::build_header() {
  std::filesystem::copy_options opt =
      std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

  if (std::filesystem::exists(current_git_path + "/single_include")) {
    std::filesystem::copy(current_git_path + "/single_include", current_lib_path, opt);
  } else if (std::filesystem::exists(current_git_path + "/include")) {
    std::filesystem::copy(current_git_path + "/include", current_lib_path, opt);
  } else if (std::filesystem::exists(current_git_path + "/" + current_repo)) {
    std::filesystem::copy(current_git_path + "/" + current_repo, current_lib_path, opt);
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(current_git_path)) {
      auto ext = entry.path().filename().extension().string();
      if (ext == "hpp" || ext == "h") {
        std::filesystem::copy(entry.path(), current_lib_path, opt);
      }
    }
  }
}

void yacppm::ISL_Getter::header_isl() {
  if (std::filesystem::exists(current_lib_path + "/single_include")) {
    libs_include_paths.push_back(current_lib_path + "/single_include");
  } else if (std::filesystem::exists(current_lib_path + "/include")) {
    libs_include_paths.push_back(current_lib_path + "/include");
  } else {
    libs_include_paths.push_back(current_lib_path);
  }
}
void yacppm::ISL_Getter::cmake_isl() {
  std::string include_file_path = current_lib_path + "/include";
  auto libs = find_libs(current_lib_path);
  std::vector<std::string> lib_names = clean_lib_names(libs);

  libs_paths.push_back(current_lib_path);
  for (const auto &lib : lib_names) {
    libs_names.push_back(lib);
  }

  if (std::filesystem::exists(include_file_path)) {
    libs_include_paths.push_back(current_lib_path + "/include");
  }
}

void yacppm::ISL_Getter::copy_license() {
  static const auto possible_name = {"LICENSE", "COPYING"};
  std::for_each(possible_name.begin(), possible_name.end(), [&](const std::string &s) {
    if (std::filesystem::exists(current_git_path + "/" + s)) {
      std::filesystem::copy(current_git_path + "/" + s, current_lib_path + "/LICENSE",
                            std::filesystem::copy_options::overwrite_existing);
      licenses.push_back(current_lib_path + "/LICENSE");
    }
  });
}
