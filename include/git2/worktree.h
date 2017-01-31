/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_git_worktree_h__
#define INCLUDE_git_worktree_h__

#include "common.h"
#include "types.h"
#include "strarray.h"

/**
 * @file git2/worktrees.h
 * @brief Git worktree related functions
 * @defgroup git_commit Git worktree related functions
 * @ingroup Git
 * @{
 */
GIT_BEGIN_DECL

/**
 * List names of linked working trees
 *
 * The returned list should be released with `git_strarray_free`
 * when no longer needed.
 *
 * @param out pointer to the array of working tree names
 * @param repo the repo to use when listing working trees
 * @return 0 or an error code
 */
GIT_EXTERN(int) git_worktree_list(git_strarray *out, git_repository *repo);

/**
 * Lookup a working tree by its name for a given repository
 *
 * @param out Output pointer to looked up worktree or `NULL`
 * @param repo The repository containing worktrees
 * @param name Name of the working tree to look up
 * @return 0 or an error code
 */
GIT_EXTERN(int) git_worktree_lookup(git_worktree **out, git_repository *repo, const char *name);

/**
 * Open working tree as a repository
 *
 * Open the working directory of the working tree as a normal
 * repository that can then be worked on.
 *
 * @param out Output pointer containing opened repository
 * @param wt Working tree to open
 * @return 0 or an error code
 */
GIT_EXTERN(int) git_worktree_open(git_repository **out, git_worktree *wt);

/**
 * Free a previously allocated worktree
 *
 * @param wt worktree handle to close. If NULL nothing occurs.
 */
GIT_EXTERN(void) git_worktree_free(git_worktree *wt);

/**
 * Check if worktree is valid
 *
 * A valid worktree requires both the git data structures inside
 * the linked parent repository and the linked working copy to be
 * present.
 *
 * @param wt Worktree to check
 * @return 0 when worktree is valid, error-code otherwise
 */
GIT_EXTERN(int) git_worktree_validate(const git_worktree *wt);

/**
 * Initialize a new working tree
 *
 * Initialize a new working tree for the repository, that is
 * create the required data structures inside the repository and
 * check out the current HEAD at `path`
 *
 * @param out Output pointer containing new working tree
 * @param repo Repository to create working tree for
 * @param name Name of the working tree
 * @param path Path to create working tree at
 * @return 0 or an error code
 */
GIT_EXTERN(int) git_worktree_init(git_worktree **out, git_repository *repo, const char *name, const char *path);

/** @} */
GIT_END_DECL
#endif
