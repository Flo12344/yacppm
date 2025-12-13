#pragma once

#include <optional>
#include <regex>
#include <string>
#include <utility>
namespace yacppm {
namespace git {
inline bool is_valid_repo_url(std::string link) {
  static const std::regex re(
      R"(^(https?|git)://[A-Za-z0-9._\-:]+/[A-Za-z0-9._\-/]+(\.git)?$)",
      std::regex::icase);
  return std::regex_match(link, re);
};

inline std::string get_git_link(const std::string &text) {
  std::string link = text;

  while (!link.empty() && isspace(link.back()))
    link.pop_back();
  while (!link.empty() && isspace(link.front()))
    link.erase(link.begin());

  if (is_valid_repo_url(link))
    return link;

  static const std::regex shortForm(R"(^[A-Za-z0-9._\-]+/[A-Za-z0-9._\-]+$)");
  if (std::regex_match(link, shortForm)) {
    return "https://github.com/" + link + ".git";
  }
  return link;
}

inline std::optional<std::pair<std::string, std::string>>
get_user_repo(std::string url) {
  if (url.size() > 4 && url.ends_with(".git"))
    url = url.substr(0, url.size() - 4);

  static const std::regex re(
      R"(^(?:https?|git)://[A-Za-z0-9._\-:]+/([A-Za-z0-9._\-]+)/([A-Za-z0-9._\-./]+)$)",
      std::regex::icase);
  std::smatch m;
  if (std::regex_match(url, m, re)) {
    return std::pair{m[1].str(), m[2].str()};
  }
  return std::nullopt;
}
} // namespace git
} // namespace yacppm
