#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace yacppm {
// @param command : command to execute
// @param func : function to use to process command output
void run_command(const std::string &command, std::function<void(std::string)> func = nullptr);
bool has_program(const std::string &program_name);
void throw_if_missing(const std::string &prog);
void throw_if_missing(const std::vector<std::string> &progs);
void throw_if_both_missing(const std::vector<std::pair<std::string, std::string>> &progs);
std::string to_camel_case(const std::string &convert);
std::string get_bin_path();
} // namespace yacppm
