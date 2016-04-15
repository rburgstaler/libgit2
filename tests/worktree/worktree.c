#include "clar_libgit2.h"
#include "worktree_helpers.h"

#include "repository.h"
#include "worktree.h"

#define COMMON_REPO "testrepo"
#define WORKTREE_REPO "testrepo-worktree"

static worktree_fixture fixture =
	WORKTREE_FIXTURE_INIT(COMMON_REPO, WORKTREE_REPO);

void test_worktree_worktree__initialize(void)
{
	setup_fixture_worktree(&fixture);
}

void test_worktree_worktree__cleanup(void)
{
	cleanup_fixture_worktree(&fixture);
}

void test_worktree_worktree__list(void)
{
	git_strarray wts;

	cl_git_pass(git_worktree_list(&wts, fixture.repo));
	cl_assert_equal_i(wts.count, 1);
	cl_assert_equal_s(wts.strings[0], "testrepo-worktree");

	git_strarray_free(&wts);
}

void test_worktree_worktree__list_with_invalid_worktree_dirs(void)
{
	const char *filesets[3][2] = {
		{ "gitdir", "commondir" },
		{ "gitdir", "HEAD" },
		{ "HEAD", "commondir" },
	};
	git_buf path = GIT_BUF_INIT;
	git_strarray wts;
	unsigned i, j, len;

	cl_git_pass(git_buf_printf(&path, "%s/worktrees/invalid",
		    fixture.repo->commondir));
	cl_git_pass(p_mkdir(path.ptr, 0755));

	len = path.size;

	for (i = 0; i < ARRAY_SIZE(filesets); i++) {

		for (j = 0; j < ARRAY_SIZE(filesets[i]); j++) {
			git_buf_truncate(&path, len);
			cl_git_pass(git_buf_joinpath(&path, path.ptr, filesets[i][j]));
			cl_git_pass(p_close(p_creat(path.ptr, 0644)));
		}

		cl_git_pass(git_worktree_list(&wts, fixture.worktree));
		cl_assert_equal_i(wts.count, 1);
		cl_assert_equal_s(wts.strings[0], "testrepo-worktree");
		git_strarray_free(&wts);

		for (j = 0; j < ARRAY_SIZE(filesets[i]); j++) {
			git_buf_truncate(&path, len);
			cl_git_pass(git_buf_joinpath(&path, path.ptr, filesets[i][j]));
			p_unlink(path.ptr);
		}
	}

	git_buf_free(&path);
}

void test_worktree_worktree__list_in_worktree_repo(void)
{
	git_strarray wts;

	cl_git_pass(git_worktree_list(&wts, fixture.worktree));
	cl_assert_equal_i(wts.count, 1);
	cl_assert_equal_s(wts.strings[0], "testrepo-worktree");

	git_strarray_free(&wts);
}

void test_worktree_worktree__list_bare(void)
{
	git_repository *repo;
	git_strarray wts;

	repo = cl_git_sandbox_init("testrepo.git");
	cl_git_pass(git_worktree_list(&wts, repo));
	cl_assert_equal_i(wts.count, 0);

	git_repository_free(repo);
}

void test_worktree_worktree__list_without_worktrees(void)
{
	git_repository *repo;
	git_strarray wts;

	repo = cl_git_sandbox_init("testrepo2");
	cl_git_pass(git_worktree_list(&wts, repo));
	cl_assert_equal_i(wts.count, 0);

	git_repository_free(repo);
}

void test_worktree_worktree__lookup(void)
{
	git_worktree *wt;
	git_buf gitdir_path = GIT_BUF_INIT;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));

	git_buf_printf(&gitdir_path, "%s/worktrees/%s", fixture.repo->commondir, "testrepo-worktree");

	cl_assert(git_path_equal(wt->gitdir_path, gitdir_path.ptr));
	cl_assert(git_path_equal(wt->parent_path, fixture.repo->path_repository));
	cl_assert(git_path_equal(wt->gitlink_path, fixture.worktree->path_gitlink));
	cl_assert(git_path_equal(wt->commondir_path, fixture.repo->commondir));

	git_buf_free(&gitdir_path);
	git_worktree_free(wt);
}

void test_worktree_worktree__lookup_nonexistent_worktree(void)
{
	git_worktree *wt;

	cl_git_fail(git_worktree_lookup(&wt, fixture.repo, "nonexistent"));
	cl_assert_equal_p(wt, NULL);
}

void test_worktree_worktree__open(void)
{
	git_worktree *wt;
	git_repository *repo;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));

	cl_git_pass(git_worktree_open(&repo, wt));
	cl_assert_equal_s(git_repository_workdir(repo),
		git_repository_workdir(fixture.worktree));

	git_repository_free(repo);
	git_worktree_free(wt);
}

void test_worktree_worktree__open_invalid_commondir(void)
{
	git_worktree *wt;
	git_repository *repo;
	git_buf buf = GIT_BUF_INIT, path = GIT_BUF_INIT;

	cl_git_pass(git_buf_sets(&buf, "/path/to/nonexistent/commondir"));
	cl_git_pass(git_buf_printf(&path,
		    "%s/worktrees/testrepo-worktree/commondir",
		    fixture.repo->commondir));
	cl_git_pass(git_futils_writebuffer(&buf, path.ptr, O_RDWR, 0644));

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_fail(git_worktree_open(&repo, wt));

	git_buf_free(&buf);
	git_buf_free(&path);
	git_worktree_free(wt);
}

void test_worktree_worktree__open_invalid_gitdir(void)
{
	git_worktree *wt;
	git_repository *repo;
	git_buf buf = GIT_BUF_INIT, path = GIT_BUF_INIT;

	cl_git_pass(git_buf_sets(&buf, "/path/to/nonexistent/gitdir"));
	cl_git_pass(git_buf_printf(&path,
		    "%s/worktrees/testrepo-worktree/gitdir",
		    fixture.repo->commondir));
	cl_git_pass(git_futils_writebuffer(&buf, path.ptr, O_RDWR, 0644));

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_fail(git_worktree_open(&repo, wt));

	git_buf_free(&buf);
	git_buf_free(&path);
	git_worktree_free(wt);
}

void test_worktree_worktree__open_invalid_parent(void)
{
	git_worktree *wt;
	git_repository *repo;
	git_buf buf = GIT_BUF_INIT;

	cl_git_pass(git_buf_sets(&buf, "/path/to/nonexistent/gitdir"));
	cl_git_pass(git_futils_writebuffer(&buf,
		    fixture.worktree->path_gitlink, O_RDWR, 0644));

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_fail(git_worktree_open(&repo, wt));

	git_buf_free(&buf);
	git_worktree_free(wt);
}

void test_worktree_worktree__init(void)
{
	git_worktree *wt;
	git_repository *repo;
	git_reference *branch;
	git_buf path = GIT_BUF_INIT;

	cl_git_pass(git_buf_joinpath(&path, fixture.repo->workdir, "../worktree-new"));
	cl_git_pass(git_worktree_init(&wt, fixture.repo, "worktree-new", path.ptr));

	/* Open and verify created repo */
	cl_git_pass(git_repository_open(&repo, path.ptr));
	cl_git_pass(git_branch_lookup(&branch, repo, "worktree-new", GIT_BRANCH_LOCAL));

	git_buf_free(&path);
	git_worktree_free(wt);
	git_reference_free(branch);
	git_repository_free(repo);
}

void test_worktree_worktree__init_existing_worktree(void)
{
	git_worktree *wt;
	git_buf path = GIT_BUF_INIT;

	cl_git_pass(git_buf_joinpath(&path, fixture.repo->workdir, "../worktree-new"));
	cl_git_fail(git_worktree_init(&wt, fixture.repo, "testrepo-worktree", path.ptr));

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_assert(git_path_equal(wt->gitlink_path, fixture.worktree->path_gitlink));

	git_buf_free(&path);
	git_worktree_free(wt);
}

void test_worktree_worktree__init_existing_path(void)
{
	const char *wtfiles[] = { "HEAD", "commondir", "gitdir", "index" };
	git_worktree *wt;
	git_buf path = GIT_BUF_INIT;
	unsigned i;

	/* Delete files to verify they have not been created by
	 * the init call */
	for (i = 0; i < ARRAY_SIZE(wtfiles); i++) {
		cl_git_pass(git_buf_joinpath(&path,
			    fixture.worktree->path_repository, wtfiles[i]));
		cl_git_pass(p_unlink(path.ptr));
	}

	cl_git_pass(git_buf_joinpath(&path, fixture.repo->workdir, "../testrepo-worktree"));
	cl_git_fail(git_worktree_init(&wt, fixture.repo, "worktree-new", path.ptr));

	/* Verify files have not been re-created */
	for (i = 0; i < ARRAY_SIZE(wtfiles); i++) {
		cl_git_pass(git_buf_joinpath(&path,
			    fixture.worktree->path_repository, wtfiles[i]));
		cl_assert(!git_path_exists(path.ptr));
	}

	git_buf_free(&path);
}

void test_worktree_worktree__validate(void)
{
	git_worktree *wt;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	cl_git_pass(git_worktree_validate(wt));

	git_worktree_free(wt);
}

void test_worktree_worktree__validate_invalid_commondir(void)
{
	git_worktree *wt;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	git__free(wt->commondir_path);
	wt->commondir_path = "/path/to/invalid/commondir";

	cl_git_fail(git_worktree_validate(wt));

	wt->commondir_path = NULL;
	git_worktree_free(wt);
}

void test_worktree_worktree__validate_invalid_gitdir(void)
{
	git_worktree *wt;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	git__free(wt->gitdir_path);
	wt->gitdir_path = "/path/to/invalid/gitdir";
	cl_git_fail(git_worktree_validate(wt));

	wt->gitdir_path = NULL;
	git_worktree_free(wt);
}

void test_worktree_worktree__validate_invalid_parent(void)
{
	git_worktree *wt;

	cl_git_pass(git_worktree_lookup(&wt, fixture.repo, "testrepo-worktree"));
	git__free(wt->parent_path);
	wt->parent_path = "/path/to/invalid/parent";
	cl_git_fail(git_worktree_validate(wt));

	wt->parent_path = NULL;
	git_worktree_free(wt);
}
