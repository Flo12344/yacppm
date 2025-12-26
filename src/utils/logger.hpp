#pragma once

#include <fmt/base.h>
#include <fmt/color.h>
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
  template <typename... T> static void info(std::string info, T &&...args) {
    std::string msg = "[INFO] " + info;
#ifdef _WIN32
    fmt::print(fmt::fg(fmt::terminal_color::green) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
#else
    fmt::print(fmt::fg(fmt::color::yellow_green) | fmt::emphasis::bold, fmt::runtime(msg), std::forward<T>(args)...);
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
};
struct ProgressBar {
  std::string label;
  int total = 0;
  int current = 0;
  int width = 0;

  std::string render() const {
    float progress = current / (float)total;
    int filled = static_cast<int>(progress * width);
    std::string bar = "[" + std::string(filled, '=') + std::string(width - filled, ' ') + "]";
    int pct = static_cast<int>(progress * 100);
    return label + " " + bar + " " + std::to_string(pct) + "%                        ";
  }

  void set_progress(int value) { current = value; }
  void set_label(std::string value) { label = value; }
  void set_total(int value) { total = value; }
};
class ProgressBarManager {
private:
  std::vector<std::shared_ptr<ProgressBar>> bars;

  ProgressBarManager() = default;

public:
  static ProgressBarManager &instance() {
    static ProgressBarManager inst;
    return inst;
  }

  int create(const std::string &label, int total, int width = 40) {

    bars.emplace_back(std::make_shared<ProgressBar>(ProgressBar{.label = label, .total = total, .width = width}));
    return bars.size() - 1;
  }

  void render() {
    std::cout << "\033[?25l";                   // hide cursor
    std::cout << "\033[" << bars.size() << "A"; // move up
    for (auto &bar : bars) {
      std::cout << bar->render() << "\n";
    }
    std::cout.flush();
  }

  std::shared_ptr<ProgressBar> get_bar(int id) { return bars.at(id); }

  ~ProgressBarManager() { std::cout << "\033[?25h"; }
};
