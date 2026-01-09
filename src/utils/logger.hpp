#pragma once

#include "barkeep.h"
#include <cstdio>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
class Loggger {
public:
  template <typename... T> static void err(std::string err, T &&...args) {
    std::string msg = "[ERROR] " + err;
#ifdef _WIN32
    fmt::print(fmt::fg(fmt::terminal_color::red) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
#else
    fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
#endif
  }
  template <typename... T> static void warn(std::string err, T &&...args) {
    std::string msg = "[WARN] " + err;
#ifdef _WIN32
    fmt::print(fmt::fg(fmt::terminal_color::yellow) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
#else
    fmt::print(fmt::fg(fmt::color::light_golden_rod_yellow) | fmt::emphasis::bold, fmt::runtime(msg),
               std::forward<T>(args)...);
#endif
  }
  template <typename... T> static void info(std::string info, T &&...args) {
#ifdef _WIN32
    fmt::print(fmt::fg(fmt::terminal_color::white) | fmt::emphasis::bold, fmt::runtime(info), std::forward<T>(args)...);
#else
    fmt::print(fmt::fg(fmt::color::light_gray) | fmt::emphasis::bold, fmt::runtime(info), std::forward<T>(args)...);
#endif
  }
  template <typename... T> static void verbose(std::string info, T &&...args) {
    std::string msg = "[INFO] " + info;
#ifdef _WIN32
    fmt::print(fmt::fg(fmt::terminal_color::white) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
#else
    fmt::print(fmt::fg(fmt::color::white) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
#endif
  }

  static void log_to_file(std::string s) {
    static std::ofstream file("log.log");
    file << s;
  }

  template <typename... T>
  static void print_indent(int indent, const std::string &colored, const std::string &text, T &&...args) {
    std::string sindent;
    for (int i = 0; i < indent; i++) {
      sindent += ' ';
    }
    sindent += colored + ' ';
    fmt::print(fmt::fg(current_color), fmt::runtime(sindent));
    fmt::print(fmt::runtime(text + "\n"), std::forward<T>(args)...);
  }

  static void set_print_color(fmt::detail::color_type color) { current_color = color; }

private:
  inline static fmt::detail::color_type current_color = fmt::terminal_color::green;
};
class ProgressBarManager {
private:
  std::vector<std::shared_ptr<barkeep::BaseDisplay>> bars{};
  std::shared_ptr<barkeep::CompositeDisplay> bar_comp;

  ProgressBarManager() = default;

public:
  static ProgressBarManager &instance() {
    static ProgressBarManager inst;
    return inst;
  }

  void init() { bar_comp = barkeep::Composite(bars, "\n"); }
  void add(std::shared_ptr<barkeep::BaseDisplay> display) {
    bars.push_back(display);
    bar_comp = barkeep::Composite(bars, "\n");
  }
};
