#include "git_utils.hpp"
#include "git2/global.h"
#include "git2/repository.h"
#include "logger.hpp"
#include <fstream>

void yacppm::git::git_print_error(const std::string &msg) {
  const git_error *e = git_error_last();
  throw GitError(msg + (e ? (": " + std::string(e->message)) : ""));
}
std::optional<git_oid> yacppm::git::resolve_to_commit_oid(git_repository *repo, const std::string &refspec) {

  Obj obj;
  if (git_revparse_single(&obj.ptr, repo, refspec.c_str()) < 0) {
    git_print_error("Cannot resolve '" + refspec + "'");
    return std::nullopt;
  }

  Commit commit;
  if (git_object_peel((git_object **)&commit.ptr, obj.ptr, GIT_OBJECT_COMMIT) < 0) {
    git_print_error("'" + refspec + "' does not point to a commit");
    return std::nullopt;
  }

  git_oid oid = *git_commit_id(commit.ptr);

  return oid;
}
void yacppm::git::switch_to_commit(git_repository *repo, const git_oid &commit_oid) {
  Commit commit;
  if (git_commit_lookup(&commit.ptr, repo, &commit_oid) < 0) {
    git_print_error("Commit lookup failed");
    return;
  }

  Tree tree;
  if (git_commit_tree(&tree.ptr, commit.ptr)) {
    git_print_error("Tree lookup failed");
    return;
  }
  git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
  opts.checkout_strategy = GIT_CHECKOUT_SAFE | GIT_CHECKOUT_RECREATE_MISSING;
  if (git_checkout_tree(repo, (git_object *)tree.ptr, &opts) < 0) {
    git_print_error("Checkout failed");
    return;
  }

  if (git_repository_set_head_detached(repo, &commit_oid) < 0) {
    git_print_error("Set detached HEAD failed");
    return;
  }
}
void yacppm::git::switch_to(git_repository *repo, const std::string &refspec) {
  auto res_oid = resolve_to_commit_oid(repo, refspec);
  if (!res_oid)
    return;
  git_oid oid = res_oid.value();
  switch_to_commit(repo, oid);

  char oid_str[GIT_OID_HEXSZ + 1] = {};
  git_oid_tostr(oid_str, sizeof(oid_str), &oid);
}
void yacppm::git::checkout_default_branch(git_repository *repo) {

  StrArray remotes;
  if (git_remote_list(&remotes.arr, repo) < 0 || remotes.arr.count == 0) {
    git_print_error();
    return;
  }

  const char *remote_name = remotes.arr.strings[0];

  std::string remote_head = std::string("refs/remotes/") + remote_name + "/HEAD";

  Ref sym;
  if (git_reference_lookup(&sym.ptr, repo, remote_head.c_str()) < 0) {
    git_print_error();
    return;
  }

  Ref resolved;
  if (git_reference_resolve(&resolved.ptr, sym.ptr) < 0) {
    git_print_error();
    return;
  }
  const char *resolved_name = git_reference_name(resolved.ptr);

  std::string prefix = std::string("refs/remotes/") + remote_name + "/";
  if (strncmp(resolved_name, prefix.c_str(), prefix.size()) != 0) {
    git_print_error();
    return;
  }
  std::string branch = resolved_name + prefix.size();

  Ref local;
  int error = git_branch_lookup(&local.ptr, repo, branch.c_str(), GIT_BRANCH_LOCAL);
  if (error == GIT_ENOTFOUND) {
    Ref remote_ref;
    if (git_reference_lookup(&remote_ref.ptr, repo, resolved_name) < 0) {
      git_print_error();
      return;
    }
  } else if (error < 0) {
    {
      git_print_error();
      return;
    }
  }

  Obj commit;
  if (git_reference_peel(&commit.ptr, local.ptr, GIT_OBJECT_COMMIT) < 0) {
    git_print_error();
    return;
  }
  git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
  opts.checkout_strategy = GIT_CHECKOUT_SAFE;

  if (git_checkout_tree(repo, commit.ptr, &opts) < 0) {
    git_print_error();
    return;
  }
  std::string head = "refs/heads/" + branch;
  if (git_repository_set_head(repo, head.c_str()) < 0) {
    git_print_error();
    return;
  }
  return;
}

void yacppm::git::init_git_project(const std::string &path) {
  git_libgit2_init();
  Repository repo;
  git_repository_init(&repo.ptr, path.c_str(), false);

  std::fstream out(path + "/.gitignore", std::ios::out);
  out <<
      R"(# Prerequisites
*.d

# Compiled Object files
*.slo
*.lo
*.o
*.obj

# Precompiled Headers
*.gch
*.pch

# Linker files
*.ilk

# Debugger Files
*.pdb

# Compiled Dynamic libraries
*.so
*.dylib
*.dll

# Fortran module files
*.mod
*.smod

# Compiled Static libraries
*.lai
*.la
*.a
*.lib

# Executables
*.exe
*.out
*.app

# debug information files
*.dwo
*.github
*.devcontainer
*.vscode

build/
.cache/
compile_commands.json
CmakeFiles.txt
)";
  out.close();
  git_libgit2_shutdown();
}
