#pragma once

#include <exception>
#include <git2/branch.h>
#include <git2/checkout.h>
#include <git2/commit.h>
#include <git2/deprecated.h>
#include <git2/errors.h>
#include <git2/object.h>
#include <git2/oid.h>
#include <git2/repository.h>
#include <git2/revparse.h>
#include <git2/types.h>
#include <optional>
#include <string>
namespace yacppm {
namespace git {

class GitError : public std::exception {
private:
  std::string msg;

public:
  GitError(const std::string &msg) : msg(msg) {};
  const char *what() const noexcept override { return msg.c_str(); }
};

struct Ref {
  git_reference *ptr = nullptr;
  ~Ref() { git_reference_free(ptr); }
};

struct Obj {
  git_object *ptr = nullptr;
  ~Obj() { git_object_free(ptr); }
};

struct StrArray {
  git_strarray arr = {0};
  ~StrArray() { git_strarray_dispose(&arr); }
};

struct Commit {
  git_commit *ptr = nullptr;
  ~Commit() { git_commit_free(ptr); }
};

struct Tree {
  git_tree *ptr = nullptr;
  ~Tree() { git_tree_free(ptr); }
};

struct Repository {
  git_repository *ptr = nullptr;
  ~Repository() { git_repository_free(ptr); }
};

void git_print_error(const std::string &msg = "libgit2 error");
std::optional<git_oid> resolve_to_commit_oid(git_repository *repo, const std::string &refspec);
void switch_to_commit(git_repository *repo, const git_oid &commit_oid);
void switch_to(git_repository *repo, const std::string &refspec);
void checkout_default_branch(git_repository *repo);
void init_git_project(const std::string &path);
} // namespace git

} // namespace yacppm
